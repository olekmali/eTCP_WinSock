#!PERL.EXE -w
#(C) Aleksander Malinowski 2011
use strict;

opendir(DIR, ".") or die "ERROR: Cannot read the current directory\n";
my @flist=readdir(DIR);
closedir(DIR);

open(OUT, "> html2c.c") or die "ERROR: Cannot write results to \"html2c.c\"\n";
print "Processing files:\n";

my $fname;
foreach $fname (sort(@flist))
{
    next if ($fname eq "." or $fname eq ".." or $fname eq "html2c.pl" or $fname eq "html2c.c");
    my $cname = $fname; $cname =~ s/\./_/g;

    my $fdata;
    if ($fname =~ m/\.(html?|css|js|txt)$/i) {
        $fdata = arrASC($fname);
        defined($fdata) or die "ERROR: Cannot read from $fname\n";
        $fdata =~ s/\n/\n  /g;
    } else {
        $fdata = arrBIN($fname);
        defined($fdata) or die "ERROR: Cannot read from $fname\n";
        $fdata =~ s/\n/\n  /g;
    }
    print OUT "code const unsigned char ${cname}[]=\n{\n  $fdata\n};\n\n";
    print "\t${fname}\tencoded as ${cname}\n";
}

close(OUT);
print "Done!\n";



###############################################################################
sub arrBIN
{
    my ($fname)=@_;
    my $bytes="";
    open(FILE, $fname) or return(undef); 
    binmode(FILE);
    read(FILE, $bytes, (stat($fname))[7]) or return(undef);
    close(FILE);
    $bytes = unpack("H*",$bytes);
    $bytes =~ s/(\w\w)/0x$1, /g;
    $bytes =~ s/((0x\w\w, ){12})/$1\n/g;
    return($bytes);
}

###############################################################################
sub arrASC
{
    my ($fname)=@_;
    my $fdata="";
    my $cnt=0;
    open(FILE, $fname) or return(undef); 
    my $line;
    while($line=<FILE>)
    {
        $fdata .= sprintf("\n// 0x%03x %s", $cnt, $line);
        my $bytes = unpack("H*",$line);
        $bytes =~ s/(\w\w)/0x$1, /g;
        while ($bytes =~ m/0x\w\w, /mg) { $cnt++; }
        $bytes =~ s/((0x\w\w, ){12})/$1\n/g;
        $fdata .= "${bytes}\n";
    }
    close(FILE);
        $fdata .= "0x00,\n";
    return($fdata);
}

