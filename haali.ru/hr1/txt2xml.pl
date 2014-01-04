#!/usr/bin/perl -w

use T2X;
use Getopt::Std;

###############################################
# run the application
my %opts;
getopts('ils',\%opts);
my $fmt;
$fmt=0 if $opts{i};
$fmt=1 if $opts{l};
my $file=shift(@ARGV) if @ARGV;
my $book=T2X::load_file($file,$fmt);
my $txt=T2X::make_xml($book);
$txt =~ s/>\s*</></sg unless $opts{s};
if (@ARGV) {
  open(OUT,"> $ARGV[0]") || die "Can't open '$ARGV[0]': $!\n";
  binmode(OUT);
  print OUT $txt;
  close(OUT);
} else {
  print STDOUT $txt;
}

exit(0);
