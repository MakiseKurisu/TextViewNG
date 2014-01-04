#!/usr/bin/perl -w

=head1 NAME

html2xml - convert HTML to FictioBook XML

=head1 SYNOPSIS

B<html2xml> [options] [source file [destination file]]

=head1 DESCRIPTION

I<html2xml> parses its HTML input and converts text and simple
formatting to FictionBook XML.  Source file can be omited or specified as
'-' to read standard input.  Destination file is produced by replacing any
extension of source file with .xml, if destination file is not specified and
source is stdin, the result is written to stdout.

=head1 OPTIONS

=over 4

=item B<-m> I<num>

treat I<num> consecutive paragraph end events as a single end
of paragraph, this can be used with files that add <br><br>
after each paragraph for example

=item B<-f> I<num>

minimum size in the C<font> element that will be converted to
a section heading, logical depth will be
clamp(I<num>+4-I<actual size>,1,4)*3

=item B<-s>

swap the meaning of C<b> and C<i> tags

=item B<-S>

use C<s> and C<em> elements instead of C<strong> and C<emphasis>

=item B<-c>

print the detected table of contents to stdout

=item B<-p> I<min[,max]>

look for C<span> elements with the I<font-size> style
specified and use the ones with I<min> <= I<font-size> <= I<max>
as section headings, logical depth will be
clamp(36-I<font-size>,1,20)

=item B<-e> I<charset>

set the charset in the xml declaration to I<charset>, if not specified
then the charset in html C<meta> tag will be used, or
I<iso-8859-1> if C<meta> tag is not present

=item B<-r>

look for classes used in C<p> elements and print the class usage
table, no conversion is done if this option is specified

=item B<-b> I<stylespec>

use the specified C<p> classes to detect section headings
I<stylespec> is like "class1,class2;class3;class4,class5",
groups delimited by semicolons specify the classes for the same
logical depth, if there are multiple classes for a single depth,
they can be separated by commas. No spaces are allowed in the
I<stylespec>

=back

=head1 EXAMPLES

html2xml -b head -p 14,20 text.html

=head1 AUTHOR

Mike Matsnev <mike@po.cs.msu.su>

=cut

use strict;
use warnings;
use HTML::TreeBuilder;
use Getopt::Std;

my $xmllib=$^O eq "MSWin32" ? "use Win32::OLE" : "use XML::LibXML";
eval $xmllib;
if ($@) {
  print STDERR "No XML support found.\n";
  undef $xmllib;
}

# global options
my $minempty=1;	    # minimum number of empty paragraphs that
		    # produces an <empty-line/>
my $minfont=0;	    # minimum <font> size to be interpreted as a section
my $ST="strong";    # strong tag
my $EM="emphasis";  # emphasis tag
my $print_toc=0;    # print table of contents in the end
my $minstyle=0;	    # minimum font-size style to be used as a section
my $maxstyle=10000; # maximum ....
my $Encoding; 	    # encoding
my $Title;	    # title
my $Author;
my $report_styles;  # styles
my %sect_styles;    # styles to use as sections

# fetch options
my %options;
getopts('m:f:sScp:e:rb:',\%options);
$minempty=$options{m}+0 if $options{m} && $options{m}>0;
$minfont=$options{f}+0 if $options{f};
($ST,$EM)=("s","em") if $options{S};
($ST,$EM)=($EM,$ST) if $options{s};
$print_toc=1 if $options{c};
if ($options{p} && $options{p} =~ /^(\d+)(?:,(\d+))?$/) {
  $minstyle=$1;
  $maxstyle=$2 if $2;
}
$Encoding=$options{e} if $options{e};
$report_styles=1 if $options{r};
if ($options{b}) {
  my $depth=3;
  for (split(/;/,$options{b})) {
    $sect_styles{lc($_)}=$depth for split(/,/,$_);
    $depth+=3;
  }
}

select(STDERR); $|=1; select(STDOUT);
my $infile=shift(@ARGV);
my $inf;
if (defined($infile) && $infile ne "-") {
  open(INF,"< $infile") || die "Can't open '$infile' for reading: $!\n";
  binmode(INF);
  $inf=\*INF;
}
my $tree=new HTML::TreeBuilder;
$tree->p_strict(1);
$tree->store_comments(1);
print STDERR "Loading file... ";
$tree->parse_file($inf ? $inf : \*STDIN);
print STDERR "done.\n";
close($inf) if $inf;

my %styles_stats;
sub stylecheck {
  no warnings;
  my $elem=shift;
  return unless ref($elem);
  $styles_stats{$_}++ if $elem->tag eq "p" && ($_=$elem->attr('class'));
  stylecheck($_) for $elem->content_list;
}

if ($report_styles) {
  stylecheck($tree);
  printf("%10d %s\n",$_->[0],$_->[1]) for sort { $b->[0] <=> $a->[0] }
    map { [$styles_stats{$_},$_] } keys %styles_stats;
  exit 0;
}

my $outfile;
if (@ARGV) {
  $outfile=shift(@ARGV);
} else {
  no warnings;
  $outfile=$infile;
  $outfile =~ s/\.[^\.\\\/]*$/.xml/;
  $outfile='-' unless $outfile;
}
if ($outfile eq "-") {
  $outfile=\*STDOUT;
} else {
  open(OUTF,"> $outfile") || die "Can't open '$outfile' for writing: $!\n";
  $outfile=\*OUTF;
}
binmode($outfile);

my $textbuf='';  # current paragraph contents
my $realtext=0;  # 1 if there is any real text in textbuf
my $space='';    # current trailing space
my $lastempty=0; # last paragraph was <emptyline>
my $strong=0;	 # <b> enabled/disabled in source
my $emphasis=0;  # <i> enabled/disabled in source
my $curstate=0;  # current state of <b> and <i> in XML output
my @elist;	 # list of all paragraphs/headers
my $body=0;
my $next_name=1; # footnotes
my %footnotes;
my $note_name=undef;
my $note_count=0;
my $poem=0;	 # we are parsing a poem

sub cleanstate {
  # reinit state
  $textbuf='';
  $realtext=0;
  $space='';
  $lastempty=0;
  $strong=0;
  $emphasis=0;
  $curstate=0;
  $body=0;
  undef(@elist);
}

sub cleanup {
  $_[0] =~ y/\t\r\n \xa0/ /s;
  $_[0] =~ s/^ //;
  $_[0] =~ s/ $//;
}

my %html_entities=( "iexcl" => "&#161;", "cent" => "&#162;", "pound" =>
"&#163;", "curren" => "&#164;", "yen" => "&#165;", "brvbar" => "&#166;", "sect" => "&#167;",
"uml" => "&#168;", "copy" => "&#169;", "ordf" => "&#170;", "laquo" => "&#171;",
"not" => "&#172;", "shy" => "&#173;", "reg" => "&#174;", "macr" => "&#175;",
"deg" => "&#176;", "plusmn" => "&#177;", "sup2" => "&#178;", "sup3" => "&#179;",
"acute" => "&#180;", "micro" => "&#181;", "para" => "&#182;",
"middot" => "&#183;", "cedil" => "&#184;", "sup1" => "&#185;", "ordm" =>
"&#186;", "raquo" => "&#187;", "frac14" => "&#188;", "frac12" => "&#189;",
"frac34" => "&#190;", "iquest" => "&#191;", "Agrave" => "&#192;", "Aacute" =>
"&#193;", "Acirc" => "&#194;", "Atilde" => "&#195;", "Auml" => "&#196;",
"Aring" => "&#197;", "AElig" => "&#198;", "Ccedil" => "&#199;", "Egrave" =>
"&#200;", "Eacute" => "&#201;", "Ecirc" => "&#202;", "Euml" => "&#203;",
"Igrave" => "&#204;", "Iacute" => "&#205;", "Icirc" => "&#206;", "Iuml" =>
"&#207;", "ETH" => "&#208;", "Ntilde" => "&#209;", "Ograve" => "&#210;",
"Oacute" => "&#211;", "Ocirc" => "&#212;", "Otilde" => "&#213;", "Ouml" =>
"&#214;", "times" => "&#215;", "Oslash" => "&#216;", "Ugrave" => "&#217;",
"Uacute" => "&#218;", "Ucirc" => "&#219;", "Uuml" => "&#220;", "Yacute" =>
"&#221;", "THORN" => "&#222;", "szlig" => "&#223;", "agrave" => "&#224;",
"aacute" => "&#225;", "acirc" => "&#226;", "atilde" => "&#227;", "auml" =>
"&#228;", "aring" => "&#229;", "aelig" => "&#230;", "ccedil" => "&#231;",
"egrave" => "&#232;", "eacute" => "&#233;", "ecirc" => "&#234;", "euml" =>
"&#235;", "igrave" => "&#236;", "iacute" => "&#237;", "icirc" => "&#238;",
"iuml" => "&#239;", "eth" => "&#240;", "ntilde" => "&#241;", "ograve" =>
"&#242;", "oacute" => "&#243;", "ocirc" => "&#244;", "otilde" => "&#245;",
"ouml" => "&#246;", "divide" => "&#247;", "oslash" => "&#248;", "ugrave" =>
"&#249;", "uacute" => "&#250;", "ucirc" => "&#251;", "uuml" => "&#252;",
"yacute" => "&#253;", "thorn" => "&#254;", "yuml" => "&#255;", "fnof" =>
"&#402;", "Alpha" => "&#913;", "Beta" => "&#914;", "Gamma" => "&#915;", "Delta" => "&#916;",
"Epsilon" => "&#917;", "Zeta" => "&#918;", "Eta" => "&#919;",
"Theta" => "&#920;", "Iota" => "&#921;", "Kappa" => "&#922;", "Lambda" =>
"&#923;", "Mu" => "&#924;", "Nu" => "&#925;", "Xi" => "&#926;", "Omicron" =>
"&#927;", "Pi" => "&#928;", "Rho" => "&#929;", "Sigma" => "&#931;", "Tau" =>
"&#932;", "Upsilon" => "&#933;", "Phi" => "&#934;", "Chi" => "&#935;", "Psi" =>
"&#936;", "Omega" => "&#937;", "alpha" => "&#945;", "beta" => "&#946;", "gamma" => "&#947;",
"delta" => "&#948;", "epsilon" => "&#949;", "zeta" => "&#950;",
"eta" => "&#951;", "theta" => "&#952;", "iota" => "&#953;", "kappa" =>
"&#954;", "lambda" => "&#955;", "mu" => "&#956;", "nu" => "&#957;", "xi" =>
"&#958;", "omicron" => "&#959;", "pi" => "&#960;", "rho" => "&#961;", "sigmaf" => "&#962;",
"sigma" => "&#963;", "tau" => "&#964;", "upsilon" => "&#965;",
"phi" => "&#966;", "chi" => "&#967;", "psi" => "&#968;", "omega" => "&#969;",
"thetasym" => "&#977;", "upsih" => "&#978;", "piv" => "&#982;", "bull" =>
"&#8226;", "hellip" => "&#8230;", "prime" => "&#8242;", "Prime" => "&#8243;",
"oline" => "&#8254;", "frasl" => "&#8260;", "weierp" => "&#8472;", "image" =>
"&#8465;", "real" => "&#8476;", "trade" => "&#8482;", "alefsym" => "&#8501;",
"larr" => "&#8592;", "uarr" => "&#8593;", "rarr" => "&#8594;", "darr" =>
"&#8595;", "harr" => "&#8596;", "crarr" => "&#8629;", "lArr" => "&#8656;",
"uArr" => "&#8657;", "rArr" => "&#8658;", "dArr" => "&#8659;", "hArr" =>
"&#8660;", "forall" => "&#8704;", "part" => "&#8706;", "exist" => "&#8707;",
"empty" => "&#8709;", "nabla" => "&#8711;", "isin" => "&#8712;", "notin" =>
"&#8713;", "ni" => "&#8715;", "prod" => "&#8719;", "sum" => "&#8721;", "minus" => "&#8722;",
"lowast" => "&#8727;", "radic" => "&#8730;", "prop" => "&#8733;",
"infin" => "&#8734;", "ang" => "&#8736;", "and" => "&#8743;", "or" =>
"&#8744;", "cap" => "&#8745;", "cup" => "&#8746;", "int" => "&#8747;", "there4" => "&#8756;",
"sim" => "&#8764;", "cong" => "&#8773;", "asymp" => "&#8776;",
"ne" => "&#8800;", "equiv" => "&#8801;", "le" => "&#8804;", "ge" => "&#8805;",
"sub" => "&#8834;", "sup" => "&#8835;", "nsub" => "&#8836;", "sube" =>
"&#8838;", "supe" => "&#8839;", "oplus" => "&#8853;", "otimes" => "&#8855;",
"perp" => "&#8869;", "sdot" => "&#8901;", "lceil" => "&#8968;", "rceil" =>
"&#8969;", "lfloor" => "&#8970;", "rfloor" => "&#8971;", "lang" => "&#9001;",
"rang" => "&#9002;", "loz" => "&#9674;", "spades" => "&#9824;", "clubs" =>
"&#9827;", "hearts" => "&#9829;", "diams" => "&#9830;", "quot" => "\"", "amp" => "&amp;",
"lt" => "&lt;", "gt" => "&gt;", "OElig" => "&#338;", "oelig" =>
"&#339;", "Scaron" => "&#352;", "scaron" => "&#353;", "Yuml" => "&#376;",
"circ" => "&#710;", "tilde" => "&#732;", "ensp" => "&#8194;", "emsp" =>
"&#8195;", "thinsp" => "&#8201;", "zwnj" => "&#8204;", "zwj" => "&#8205;",
"lrm" => "&#8206;", "rlm" => "&#8207;", "ndash" => "&#8211;", "mdash" =>
"&#8212;", "lsquo" => "&#8216;", "rsquo" => "&#8217;", "sbquo" => "&#8218;",
"ldquo" => "&#8220;", "rdquo" => "&#8221;", "bdquo" => "&#8222;", "dagger" =>
"&#8224;", "Dagger" => "&#8225;", "permil" => "&#8240;", "lsaquo" => "&#8249;",
"rsaquo" => "&#8250;", "euro" => "&#8364;", "nbsp" => " ");


sub moretext {
  my $txt=shift;
  { no warnings; $txt =~ s/&(?!#)(?:([a-zA-Z]+);)?/$1 ? $html_entities{$1} : "&amp;"/sge; }
  $txt =~ y/\t\r\n \xa0/ /s; # also destroys nbsp
  $space=' ' if $txt =~ /^\s/;
  my $msp='';
  $msp=' ' if $txt =~ /\s$/;
  $txt =~ s/^ //;
  $txt =~ s/ $//;
  $txt =~ s/ \. \. \./&#8230;/sg;
  #$txt =~ s/&(?!#|(?:[a-z]+;))/&amp;/g; # escape &, but don't touch entities
  if (!length($txt)) { # whitespace only???
    $space=(length($space) || length($msp)) && $realtext ? ' ' : '';
  } else {
    $textbuf.=$space if $realtext;
    $realtext=1,checkhl() unless $realtext;
    $textbuf.=$txt;
    $space=$msp;
  }
}

sub checkhl { # check <b> and <i> highlighting
  my $newstate=0;
  $newstate|=1 if @_ ? $_[0] : $strong;
  $newstate|=2 if @_ ? $_[1] : $emphasis;
  return if $curstate==$newstate || ($curstate==0 && !$realtext);
  # always close whatever is open
  $textbuf.="</$ST>" if $curstate&1;
  $textbuf.="</$EM>" if $curstate&2;
  # and now open new tags
  $textbuf.="<$EM>" if $newstate&2;
  $textbuf.="<$ST>" if $newstate&1;
  $curstate=$newstate;
}

sub pbreak {
  if (length($textbuf)) {
    # close highlighting
    checkhl(0,0);
    push(@elist,[ 0, undef, $poem ]) if $lastempty>$minempty;
    push(@elist,[ 0, $textbuf, $poem]);
    $lastempty=0;
    $textbuf='';
    $space='';
    $realtext=0;
  } else {
    ++$lastempty;
  }
}

sub add_section {
  my ($txt,$depth)=@_;
  cleanup($txt);
  pbreak();
  push(@elist,[ $depth, $txt ]);
}

sub get_styles {
  my $styles=$_[0]->attr('style');
  return () unless $styles;
  my @styles;
  for (split(/;/,$styles)) {
    push (@styles,$1,$2) if /^\s*(\S+)\s*:\s*(\S+)\s*$/;
  }
  @styles;
}

sub element {
  my $elem=shift;
  if ($body) {
    if (!ref($elem)) {
      moretext($elem);
      return;
    }
    my $t=$elem->tag;
    if ($t eq "p") {
      my $class=$elem->attr('class');
      if ($class) {
	if ($sect_styles{lc($class)}) { # MS Office stuff
	  add_section($elem->as_text,$sect_styles{lc($class)});
	  return;
	} elsif ($class eq "MsoFootnoteText" || $class eq "note") {
	  # skip it here
	  return;
	} elsif ($class eq "lyrics" && !$poem) {
	  pbreak();
	  $poem=1;
	  element($_) for $elem->content_list;
	  pbreak();
	  $poem=0;
	  return;
	} elsif ($class eq "intro") {
	  pbreak();
	  ++$emphasis;
	  checkhl;
	  element($_) for $elem->content_list;
	  --$emphasis;
	  checkhl;
	  pbreak();
	  return;
	}
      }
      pbreak();
    } elsif ($t eq "dd" || $t eq "br") {
      pbreak();
    } elsif ($t =~ /^h(\d)/) {
      add_section($elem->as_text,$1*3);
      return;
    } elsif ($minstyle && $t eq "span") {
      my %styles=get_styles($elem);
      if ($styles{'font-size'} &&
	  $styles{'font-size'} =~ /^(\d+(?:\.\d+)?)(?:pt)?$/ &&
	  $1>=$minstyle && $1<=$maxstyle)
      {
	my $depth=36-$1;
	$depth=1 if $depth<1;
	$depth=20 if $depth>20;
	add_section($elem->as_text,$depth);
	return;
      }
    } elsif ($minfont && $t eq "font") {
      my $size=$elem->attr('size');
      if ($size && $size>=$minfont) {
	my $depth=$minfont+4-$size;
	$depth=1 if $depth<1;
	$depth=4 if $depth>4;
	add_section($elem->as_text,$depth*3);
	return;
      }
    } elsif ($t eq "script" || $t eq "style") {
      return;
    } elsif ($t eq "i" || $t eq "em") {
      ++$emphasis;
      checkhl;
      element($_) for $elem->content_list;
      --$emphasis;
      checkhl;
      return;
    } elsif ($t eq "blockquote") {
      pbreak();
      ++$emphasis;
      checkhl;
      element($_) for $elem->content_list;
      --$emphasis;
      checkhl;
      return;
    } elsif ($t eq "b" || $t eq "strong") {
      ++$strong;
      checkhl;
      element($_) for $elem->content_list;
      --$strong;
      checkhl;
      return;
    } elsif ($t eq "a") {
      my $href=$elem->attr('href');
      if ($href && $href =~ /^#/) { # ms office footnote
	$href=substr($href,1);
	if ($footnotes{$href}) {
	  moretext("<note name=\"$next_name\">");
	  moretext("<p>$_</p>") for @{$footnotes{$href}};
	  moretext("</note>");
	  ++$next_name;
	  return;
	}
      }
    }
  } else {
    return if !ref($elem);
    if ($elem->tag eq "body") {
      $body=1;
      element($_) for $elem->content_list;
      $body=0;
      return;
    } elsif ($elem->tag eq "meta") {
      my $eq=$elem->attr('http-equiv');
      my $content=$elem->attr('content');
      $Encoding=$1 if $eq && $content && lc($eq) eq "content-type" &&
		      !$Encoding && $content =~ /charset=(\S+)/i;
    } elsif ($elem->tag eq "title") {
      $Title=$elem->as_text;
    }
  }
  element($_) for $elem->content_list;
}

sub findnotes_element {
  my $elem=shift;
  if ($body) {
    if (!ref($elem)) {
      moretext($elem);
      return;
    }
    my $t=$elem->tag;
    if ($t eq "p") {
      pbreak();
    } elsif ($t eq "dd" || $t eq "br") {
      pbreak();
    } elsif ($t =~ /^h(\d)/) {
      pbreak();
    } elsif ($t eq "script" || $t eq "style") {
      return;
    } elsif ($t eq "i") {
      ++$emphasis;
      checkhl;
      element($_) for $elem->content_list;
      --$emphasis;
      checkhl;
      return;
    } elsif ($t eq "b") {
      ++$strong;
      checkhl;
      element($_) for $elem->content_list;
      --$strong;
      checkhl;
      return;
    } elsif ($t eq "a") {
      $note_name=$elem->attr('name');
    }
  } else {
    return if !ref($elem);
    if ($elem->tag eq "p") {
      my $class=$elem->attr('class');
      if ($class && ($class eq "MsoFootnoteText" || $class eq "note")) {
	$body=1;
	findnotes_element($_) for $elem->content_list;
	pbreak();
	shift(@elist) while @elist && (!defined($elist[0][1]) || !length($elist[0][1]));
	pop(@elist) while @elist && (!defined($elist[-1][1])|| !length($elist[-1][1]));
	if ($note_name) {
	  $footnotes{$note_name}=[map { $_->[1] } @elist];
	  $note_name=undef;
	  ++$note_count;
	}
	cleanstate();
      }
    }
  }
  findnotes_element($_) for $elem->content_list;
}

print STDERR "Building footnotes list... ";
findnotes_element($tree);
print STDERR "done ($note_count found).\n";
cleanstate();
print STDERR "Building paragraph list... ";
element($tree); # process text
pbreak();
if ($xmllib) {
  # scavenge m$ pseudo xml
  my $dom;
  for ($tree->find_by_tag_name("~comment")) {
    my $cm=$_->attr('text');
    if ($cm =~ s/^\[if [^\]]*\]>//s && $cm =~ s/<!\[(?:(?:else\])|(?:endif\])).*//s &&
        $cm =~ s/<xml>/<xml xmlns:w="blah:blah" xmlns:dt="uff:uff">/s) {
      $cm =~ s/o:(\w)/$1/sg;
      # now parse this as an XML
      if ($dom=parse_xml($cm)) {
        my $eba=first_node($dom,"//eBook-author");
        my $pa=first_node($dom,"//Author");
        my $pt=first_node($dom,"//eBook-title");
        $pa=$eba if $eba;
        $Title=$pt if $pt;
        $Author=$pa if $pa;
      } else {
        #my $reason=$dom->{parseError}->{reason};
        #print STDERR "XML parse error: $reason" if $reason !~ /namespace/;
      }
    }
  }
}
$tree->delete;
$Encoding||="iso-8859-1";
$Title||="";
$Author||="";
cleanup($Title);
cleanup($Author);
print STDERR "done.\n";

# convert the list into a tree
my $root={ type => undef, chl => [ ], parent => undef, title => undef, level => 0 };
my $cur=$root;

print STDERR "Building sections tree... ";
# iterate over paragraphs
for my $p (@elist) {
  if ($p->[0] == 0) { # plain text
    if (!$cur->{type}) { # not defined yet
      $cur->{type}='p'; # text
    } elsif ($cur->{type} eq 's') { # section, create a new one
      my $new={ type => 'p', chl => [ ], parent => $cur, title => undef, level => $cur->{level}+1};
      $cur=$new;
    }
    push(@{$cur->{chl}},[$p->[1],$p->[2]]);
  } else { # header
    # back off to the proper level
    while ($p->[0]<=$cur->{level}) {
      $cur=$cur->{parent};
    }
    my $new={ type => undef, chl => [ ], parent => $cur, title => $p->[1], level => $p->[0] };
    if (!$cur->{type} || $cur->{type} eq 's') { # no contents yet
      $cur->{type}='s';
      push(@{$cur->{chl}},$new);
    } else { # convert to container
      # create an implicit section
      my $tmp={ type => 'p', chl => $cur->{chl}, parent => $cur, title => undef, level => $cur->{level} + 1 };
      $cur->{chl}=[$tmp,$new];
      $cur->{type}='s';
    }
    $cur=$new;
  }
}
undef(@elist);
print STDERR "done.\n";

print STDERR "Cleaning up... ";
# remove leading and trailing empty lines
sub remove_empty_lines {
  my $section=shift;
  if ($section->{type} && $section->{type} eq 'p') {
    my $n;
    for ($n=0;$n<=$#{$section->{chl}} && !defined($section->{chl}[$n][0]);++$n) {}
    splice(@{$section->{chl}},0,$n) if $n;
    for ($n=0;$n<=$#{$section->{chl}} && !defined($section->{chl}[-$n-1][0]);++$n) {}
    splice(@{$section->{chl}},-$n) if $n;
  } else {
    remove_empty_lines($_) for @{$section->{chl}};
  }
}
remove_empty_lines($root);
# remove completely empty sections
sub remove_empty_sections {
  no warnings;
  my $section=shift;
  return if $section->{type} && $section->{type} eq 'p';
  remove_empty_sections($_) for @{$section->{chl}};
  @{$section->{chl}}=grep { length($_->{title}) || @{$_->{chl}} } @{$section->{chl}};
  # eliminate 's' sections with empty titles
  my $chl=$section->{chl};
  for (my $i=0;$i<=$#$chl;++$i) {
    if ((!defined($chl->[$i]{type}) || $chl->[$i]{type} eq 's') &&
	!length($chl->[$i]{title}))
    {
      my $n=@{$chl->[$i]{chl}};
      splice(@$chl,$i,1,@{$chl->[$i]{chl}});
      for (my $k=0;$k<$n;++$k) {
	$chl->[$i+$k]{parent}=$section;
      }
      $i+=$n;
    } else {
      ++$i;
    }
  }
  # eliminate 'p' sections with empty titles
  if ($#$chl==0 && defined($chl->[0]{type}) && $chl->[0]{type} eq 'p' &&
      (!length($chl->[0]{title}) || !length($section->{title})))
  {
    $chl->[0]{parent}=undef;
    $section->{title}=$chl->[0]{title} if length($chl->[0]{title});
    $section->{type}=$chl->[0]{type};
    $section->{level}=$chl->[0]{level} if $chl->[0]{level}&&!$section->{level};
    @$chl=@{$chl->[0]{chl}};
  }
}
remove_empty_sections($root);
print STDERR "done.\n";

print STDERR "Writing XML... " unless $print_toc;
# print a standard header
my @auth=split(' ',$Author);
if (@auth==0) { # no author
  @auth=('','','');
} elsif (@auth==1) {
  @auth=('','',$auth[0]);
} elsif (@auth==2) {
  @auth=($auth[0],'',$auth[1]);
} elsif (@auth>3) {
  @auth=('','',join(' ',@auth));
}
print $outfile <<EOF ;
<?xml version="1.0" encoding="$Encoding"?>
<?xml-stylesheet type="text/xsl" href="o:\\bin\\fb2htm.xsl"?>
<FictionBook>
  <description>
    <title-info>
      <genre></genre>
      <author>
	<first-name>$auth[0]</first-name>
	<middle-name>$auth[1]</middle-name>
	<last-name>$auth[2]</last-name>
      </author>
      <book-title>$Title</book-title>
      <annotation>
	<p/>
      </annotation>
    </title-info>
  </description>
  <body>
EOF

# let's see what we've done
my $indent=3;
sub stopen($$$) {
  $_[0]=1,++$_[2],print $outfile " " x ($_[2]-1),"<poem>\n" if !$_[0];
  $_[1]=1,++$_[2],print $outfile " " x ($_[2]-1),"<stanza>\n" if !$_[1];
}

sub stclose($$) {
  $_[0]=0,--$_[1],print $outfile " " x $_[1],"</stanza>\n" if $_[0];
}

sub pstclose($$$) {
  $_[0]=0,--$_[2],print $outfile " " x $_[2],"</stanza>\n" if $_[0];
  $_[1]=0,--$_[2],print $outfile " " x $_[2],"</poem>\n" if $_[1];
}

sub section {
  my $sect=shift;
  my $st_open=0;
  my $poem_open=0;
  if ($sect->{title}) {
    print $outfile " " x $indent, "<section title=\"",$sect->{title},"\">\n";
    print " " x $indent, $sect->{title}, "\n" if $print_toc;
  } else {
    print $outfile " " x $indent, "<section title=\"\">\n";
  }
  ++$indent;
  if ($sect->{type} && $sect->{type} eq "p") {
    for (@{$sect->{chl}}) {
      if (defined($_->[0])) {
	if ($_->[1]) {
	  stopen($poem_open,$st_open,$indent);
	  print $outfile " " x $indent, "<v>",$_->[0],"</v>\n";
	} else {
	  pstclose($poem_open,$st_open,$indent);
          print $outfile " " x $indent, "<p>",$_->[0],"</p>\n"
	}
      } else {
	if ($_->[1]) {
	  stclose($st_open,$indent);
	} else {
	  pstclose($poem_open,$st_open,$indent);
	  print $outfile " " x $indent, "<empty-line/>\n";
	}
      }
    }
    pstclose($poem_open,$st_open,$indent);
  } else {
    section($_) for @{$sect->{chl}};
  }
  --$indent;
  print $outfile " " x $indent,"</section>\n";
}
if (!($root->{type} && $root->{type} eq 'p') && (!defined($root->{title}) ||
      !length($root->{title})))
{
  section($_) for @{$root->{chl}};
} else {
  section($root);
}

print $outfile <<EOF ;
  </body>
</FictionBook>
EOF
print STDERR "done.\n" unless $print_toc;

exit 0;

sub parse_xml {
  my $xml=shift;
  if ($^O eq "MSWin32") {
    my $dom=new Win32::OLE('Msxml2.DOMDocument.4.0');
    $dom->{async}=0;
    $dom->{validateOnParse}=0;
    return $dom if $dom->loadXML($xml);
  } else {
    my $parser=new XML::LibXML;
    my $dom=eval {
      $parser->parse_string($xml);
    };
    return $dom if $dom && !$@;
  }
  undef;
}

sub first_node {
  my ($dom,$xpe)=@_;
  if ($^O eq "MSWin32") {
    my $node=$dom->selectSingleNode($xpe);
    return $node->text if $node;
    undef;
  } else {
    $dom->findvalue($xpe);
  }
}
