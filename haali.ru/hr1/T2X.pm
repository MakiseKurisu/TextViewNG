package T2X;

use strict;

sub remws_inp { # remove whitespace
  $_[0] =~ tr/\000-\040/ /s;
  $_[0] =~ s/^ //;
  $_[0] =~ s/ $//;
}

sub remws { # remove whitespace
  no warnings;
  my $tmp=shift;
  $tmp =~ tr/\000-\040/ /s;
  $tmp =~ s/^ //;
  $tmp =~ s/ $//;
  $tmp;
}

sub escape { # xml-escape
  no warnings;
  my $p=shift;
  my $h=shift;
  $p =~ s/&/&amp;/g;
  if (!$h) {
    $p =~ s/</&lt;/g;
    $p =~ s/"/&quot;/g;
    $p =~ s/>/&gt;/g;
  }
  $p;
}

sub conv_quotes { # nicely convert possibly nested quotes
  local $^W=0;
  my ($l,$r)=("\x93","\x94");
  ($l,$r)=("\xab","\xbb") if $_[0] =~ tr/\200-\377//;
  my $t="";
  my $nest=0;
  my $last=0;
  while ($_[0] =~ /((?<=[^ ])")|(?:"(?=[^ ,.]))/sgc) {
    if (length($1)) { # close
      return if $nest<=0;
      $t.=substr($_[0],$last,pos($_[0])-$last-1);
      $t.=$r;
      --$nest;
    } else { #open
      return if $nest>1;
      $t.=substr($_[0],$last,pos($_[0])-$last-1);
      $t.=$l;
      ++$nest;
    }
    $last=pos($_[0]);
  }
  if ($nest==0) { # properly balanced
    $t.=substr($_[0],pos($_[0]));
    $_[0]=$t;
  }
}

sub beautify { # beautify a paragraph
  my $p=shift;
  my $h=shift;
  $p =~ s/^- /– /;
  conv_quotes($p);
  $p =~ s/\.\.\./…/g;
  if (!$h) {
    $p =~ s/&/&amp;/g;
    $p =~ s/</&lt;/g;
    $p =~ s/"/&quot;/g;
    $p =~ s/>/&gt;/g;
  }
  $p;
}

sub make_xml { # convert a book into xml
  no warnings;
  my $book=shift;
  my $max=shift;
  my $xml="";
  my @au=split(' ',escape(remws($book->{author})));
  if (1==@au) {
    unshift(@au,"","");
  } elsif (2==@au) {
    splice(@au,1,0,"");
  } elsif (3<@au) {
    @au=("","",escape(remws($book->{author})));
  }
  my $title=escape(remws($book->{title}));
  $xml .= qq(<?xml version="1.0" encoding="windows-1251"?>\n\n);
  $xml .= qq(<book>\n);
  $xml .= qq(  <description>\n);
  $xml .= qq(    <title>$title</title>\n);
  $xml .= qq(    <author>\n);
  $xml .= qq(      <first-name>$au[0]</first-name>\n);
  $xml .= qq(      <middle-name>$au[1]</middle-name>\n);
  $xml .= qq(      <last-name>$au[2]</last-name>\n);
  $xml .= qq(    </author>\n);
  $xml .= qq(    <annotation>);
  $xml .= escape(join("",map { "<p>" . remws($_) . "</p>" }
		split (/\n/,$book->{annotation})),1);
  $xml .= qq(</annotation>\n);
  $xml .= qq(  </description>\n);
  $xml .= qq(  <body>\n);
  for (@{$book->{body}}) {
    $xml .= qq(    <chapter title=");
    $xml .= escape($_->{title});
    $xml .= qq(">\n);
    for (@{$_->{text}}) {
      my $tmp=beautify($_,$book->{html});
      if (length($tmp)) {
	if ($tmp =~ /^\s*\*\s*\*\s*\*\s*$/) {
	  $xml .= qq(      <subheading>* * *</subheading>\n);
        } else {
          $xml .= qq(      <p>);
          $xml .= $tmp;
          $xml .= qq(</p>\n);
	}
      } else {
        $xml .= qq(      <emptyline/>\n);
      }
      last if defined($max) && length($xml)>=$max;
    }
    $xml .= qq(    </chapter>\n);
    last if defined($max) && length($xml)>=$max;
  }
  $xml .= qq(  </body>\n);
  $xml .= qq(</book>\n);
  $xml;
}

sub replace_tag {
  my ($tag,$end)=@_;
  return "<${end}emphasis>" if $tag eq "i";
  return "<${end}strong>" if $tag eq "b";
  if ($tag =~ /^h[0-9]$/ ) {
    return "\n <$tag>" if !length($end);
    return "</$tag>\n ";
  }
  return "<hr>" if $tag eq "hr";
  ""; # delete all other tags
}

my %html_entities=(
"iexcl" => "&#161;",
"cent" => "&#162;",
"pound" => "&#163;",
"curren" => "&#164;",
"yen" => "&#165;",
"brvbar" => "&#166;",
"sect" => "&#167;",
"uml" => "&#168;",
"copy" => "&#169;",
"ordf" => "&#170;",
"laquo" => "&#171;",
"not" => "&#172;",
"shy" => "&#173;",
"reg" => "&#174;",
"macr" => "&#175;",
"deg" => "&#176;",
"plusmn" => "&#177;",
"sup2" => "&#178;",
"sup3" => "&#179;",
"acute" => "&#180;",
"micro" => "&#181;",
"para" => "&#182;",
"middot" => "&#183;",
"cedil" => "&#184;",
"sup1" => "&#185;",
"ordm" => "&#186;",
"raquo" => "&#187;",
"frac14" => "&#188;",
"frac12" => "&#189;",
"frac34" => "&#190;",
"iquest" => "&#191;",
"Agrave" => "&#192;",
"Aacute" => "&#193;",
"Acirc" => "&#194;",
"Atilde" => "&#195;",
"Auml" => "&#196;",
"Aring" => "&#197;",
"AElig" => "&#198;",
"Ccedil" => "&#199;",
"Egrave" => "&#200;",
"Eacute" => "&#201;",
"Ecirc" => "&#202;",
"Euml" => "&#203;",
"Igrave" => "&#204;",
"Iacute" => "&#205;",
"Icirc" => "&#206;",
"Iuml" => "&#207;",
"ETH" => "&#208;",
"Ntilde" => "&#209;",
"Ograve" => "&#210;",
"Oacute" => "&#211;",
"Ocirc" => "&#212;",
"Otilde" => "&#213;",
"Ouml" => "&#214;",
"times" => "&#215;",
"Oslash" => "&#216;",
"Ugrave" => "&#217;",
"Uacute" => "&#218;",
"Ucirc" => "&#219;",
"Uuml" => "&#220;",
"Yacute" => "&#221;",
"THORN" => "&#222;",
"szlig" => "&#223;",
"agrave" => "&#224;",
"aacute" => "&#225;",
"acirc" => "&#226;",
"atilde" => "&#227;",
"auml" => "&#228;",
"aring" => "&#229;",
"aelig" => "&#230;",
"ccedil" => "&#231;",
"egrave" => "&#232;",
"eacute" => "&#233;",
"ecirc" => "&#234;",
"euml" => "&#235;",
"igrave" => "&#236;",
"iacute" => "&#237;",
"icirc" => "&#238;",
"iuml" => "&#239;",
"eth" => "&#240;",
"ntilde" => "&#241;",
"ograve" => "&#242;",
"oacute" => "&#243;",
"ocirc" => "&#244;",
"otilde" => "&#245;",
"ouml" => "&#246;",
"divide" => "&#247;",
"oslash" => "&#248;",
"ugrave" => "&#249;",
"uacute" => "&#250;",
"ucirc" => "&#251;",
"uuml" => "&#252;",
"yacute" => "&#253;",
"thorn" => "&#254;",
"yuml" => "&#255;",
"fnof" => "&#402;",
"Alpha" => "&#913;",
"Beta" => "&#914;",
"Gamma" => "&#915;",
"Delta" => "&#916;",
"Epsilon" => "&#917;",
"Zeta" => "&#918;",
"Eta" => "&#919;",
"Theta" => "&#920;",
"Iota" => "&#921;",
"Kappa" => "&#922;",
"Lambda" => "&#923;",
"Mu" => "&#924;",
"Nu" => "&#925;",
"Xi" => "&#926;",
"Omicron" => "&#927;",
"Pi" => "&#928;",
"Rho" => "&#929;",
"Sigma" => "&#931;",
"Tau" => "&#932;",
"Upsilon" => "&#933;",
"Phi" => "&#934;",
"Chi" => "&#935;",
"Psi" => "&#936;",
"Omega" => "&#937;",
"alpha" => "&#945;",
"beta" => "&#946;",
"gamma" => "&#947;",
"delta" => "&#948;",
"epsilon" => "&#949;",
"zeta" => "&#950;",
"eta" => "&#951;",
"theta" => "&#952;",
"iota" => "&#953;",
"kappa" => "&#954;",
"lambda" => "&#955;",
"mu" => "&#956;",
"nu" => "&#957;",
"xi" => "&#958;",
"omicron" => "&#959;",
"pi" => "&#960;",
"rho" => "&#961;",
"sigmaf" => "&#962;",
"sigma" => "&#963;",
"tau" => "&#964;",
"upsilon" => "&#965;",
"phi" => "&#966;",
"chi" => "&#967;",
"psi" => "&#968;",
"omega" => "&#969;",
"thetasym" => "&#977;",
"upsih" => "&#978;",
"piv" => "&#982;",
"bull" => "&#8226;",
"hellip" => "&#8230;",
"prime" => "&#8242;",
"Prime" => "&#8243;",
"oline" => "&#8254;",
"frasl" => "&#8260;",
"weierp" => "&#8472;",
"image" => "&#8465;",
"real" => "&#8476;",
"trade" => "&#8482;",
"alefsym" => "&#8501;",
"larr" => "&#8592;",
"uarr" => "&#8593;",
"rarr" => "&#8594;",
"darr" => "&#8595;",
"harr" => "&#8596;",
"crarr" => "&#8629;",
"lArr" => "&#8656;",
"uArr" => "&#8657;",
"rArr" => "&#8658;",
"dArr" => "&#8659;",
"hArr" => "&#8660;",
"forall" => "&#8704;",
"part" => "&#8706;",
"exist" => "&#8707;",
"empty" => "&#8709;",
"nabla" => "&#8711;",
"isin" => "&#8712;",
"notin" => "&#8713;",
"ni" => "&#8715;",
"prod" => "&#8719;",
"sum" => "&#8721;",
"minus" => "&#8722;",
"lowast" => "&#8727;",
"radic" => "&#8730;",
"prop" => "&#8733;",
"infin" => "&#8734;",
"ang" => "&#8736;",
"and" => "&#8743;",
"or" => "&#8744;",
"cap" => "&#8745;",
"cup" => "&#8746;",
"int" => "&#8747;",
"there4" => "&#8756;",
"sim" => "&#8764;",
"cong" => "&#8773;",
"asymp" => "&#8776;",
"ne" => "&#8800;",
"equiv" => "&#8801;",
"le" => "&#8804;",
"ge" => "&#8805;",
"sub" => "&#8834;",
"sup" => "&#8835;",
"nsub" => "&#8836;",
"sube" => "&#8838;",
"supe" => "&#8839;",
"oplus" => "&#8853;",
"otimes" => "&#8855;",
"perp" => "&#8869;",
"sdot" => "&#8901;",
"lceil" => "&#8968;",
"rceil" => "&#8969;",
"lfloor" => "&#8970;",
"rfloor" => "&#8971;",
"lang" => "&#9001;",
"rang" => "&#9002;",
"loz" => "&#9674;",
"spades" => "&#9824;",
"clubs" => "&#9827;",
"hearts" => "&#9829;",
"diams" => "&#9830;",
"quot" => "\"",
"amp" => "&amp;",
"lt" => "&lt;",
"gt" => "&gt;",
"OElig" => "&#338;",
"oelig" => "&#339;",
"Scaron" => "&#352;",
"scaron" => "&#353;",
"Yuml" => "&#376;",
"circ" => "&#710;",
"tilde" => "&#732;",
"ensp" => "&#8194;",
"emsp" => "&#8195;",
"thinsp" => "&#8201;",
"zwnj" => "&#8204;",
"zwj" => "&#8205;",
"lrm" => "&#8206;",
"rlm" => "&#8207;",
"ndash" => "&#8211;",
"mdash" => "&#8212;",
"lsquo" => "&#8216;",
"rsquo" => "&#8217;",
"sbquo" => "&#8218;",
"ldquo" => "&#8220;",
"rdquo" => "&#8221;",
"bdquo" => "&#8222;",
"dagger" => "&#8224;",
"Dagger" => "&#8225;",
"permil" => "&#8240;",
"lsaquo" => "&#8249;",
"rsaquo" => "&#8250;",
"euro" => "&#8364;",
"nbsp" => " "
);

sub replace_ent {
  my $ent=shift;
  return $html_entities{$ent} if $html_entities{$ent};
  return "&$ent;" if $ent =~ /^#\d+$/ || $ent =~ /^#x[\dA-Fa-f]+$/;
  return "&amp;" if !length($ent);
  "";
}

sub load_file {
  my $infile=shift;
  my $text;
  my $fmt=shift;
  my $info='';

  # load file
  if (defined($infile)) {
    open(SRC,"< $infile") || die "Can't open '$infile': $!\n";
    binmode(SRC);
    { local $/=undef; $text=<SRC>; }
    close(SRC);
  } else {
    binmode(STDIN);
    { local $/=undef; $text=<STDIN>; }
  }
  $info=length($text) . " byte(s)";

  conv_charset($text);
  my $html=0;
  # check moshkow's formatting
  if ($text =~ /\x14/) {
    $text =~ s|\x14 \*([^\n]*)\* \x15|<h1>$1</h1>|sg;
    $text =~ s|\x14([^\n]*)\x15|<h2>$1</h2>|sg;
    $html = 1;
  }

  # remove msdos garbage and trailing whitespace
  $text =~ tr/\r//d;
  $text =~ tr/\000-\011\013-\037\xa0/ /;
  $text =~ s/ +$//mg;

  # check if it is some crappy html
  $html||= $text =~ /<html>/i;
  my $anno="";
  my $title="";
  my $author="";
  if ($html) {
    # delete most of the garbage
    # all before title
    $text =~ s/^.*?(?=<h.>)//s;
    # all past the end of text
    my $tmp=rindex($text,"<hr");
    $text=substr($text,0,$tmp) if length($text)-$tmp<1024;
    # destroy all other tags
    $text =~ s=<(/?)([a-zA-Z0-9]*)[^>]*>=replace_tag($2,$1)=sge;
    $text =~ s/&(([^;]{1,15});)?/replace_ent($2)/sge;
    $text =~ s=<emphasis>\s+</emphasis>= =g; # remove empty space
    $text =~ s=<strong>\s+</strong>= =g; # remove empty space
    # convert runs of --- to hr
    $text =~ s/-{5,}/<hr>/sg;
    # stuff between first two hr is moved to annotation
    $anno=join("\n",map { remws($_) } grep { length } split(/\n/,$1))
	if $text =~ s/<hr>(.*?)<hr>//s;
    # first <h> becomes the title
    $title=$1 if $text =~ s/<h[0-9]>([^<]*)<\/h[0-9]>//s;
    $author=$1 if $title =~ s/(.*\.)(.*)/$2/;
    $author =~ s/(?<!\.[^.])\.$//;
    # delete all remaining <hr>s
    $text =~ s/<hr>//sg;
    remws_inp($title); remws_inp($author);
    $info.=", html";
  }

  # count total number of lines
  my $nlines= $text =~ tr/\n//;

  # check if the whole file is indented and remove that extra indentation
  my @indent;
  ++$indent[length($1)] while $text =~ /^(\s+)/mg;
  for (my $space=1;$space<@indent;++$space) {
    no warnings;
    if ($indent[$space]>$nlines*0.87) {
      $text =~ s/^\s{1-$space}//mg;
      splice(@indent,0,$space+1);
      last;
    }
  }

  # try to guess file format
  # check remaining indentation
  if (!defined($fmt)) {
    no warnings;
    my $ilines=0;
    $ilines+=$_ for @indent;
    $info.=", IL=$ilines, NL=$nlines";
    if ($ilines<$nlines*0.03) {
      $fmt=1;
    } elsif ($ilines>$nlines*0.08) {
      $fmt=0;
    } else { # shold really ask user here
      $fmt=1;
    }
  }
  $info.=", FMT=$fmt";

  # split the whole thing into paragraphs
  my @plist;
  if ($fmt==3) {
    @plist=split(/\n\s*\n\s*\n\s*\n/s,$text);
  } if ($fmt==2) {
    @plist=split(/\n\n/s,$text);
  } elsif ($fmt==1) {
    @plist=split(/\n/s,$text);
  } else {
    @plist=split(/(?:\n )|(?:\n(?=\n))/s,$text);
  }

  # delete unneeded whitespace
  remws_inp($_) for @plist;

  # try to find chapter headings
  my $max=$fmt ? 50 : 120;
  my @chlist;
  my $curch=[];
  my $curtitle="";
  push(@plist,"");
  if ($html) {
    for (my $i=0;$i<@plist-1;++$i) {
      if ($plist[$i] =~ m=^<h[0-9]>(.*)</h[0-9]>$=) {
        # this seems to be a chapter heading
        if (@$curch) {
	  push(@chlist,{ 'title' => $curtitle, 'text' => $curch});
	  $curch=[];
        }
        $curtitle=$1;
	$curtitle =~ s/<[^>]*>//g; # remove markup from titles
      } else {
        push(@$curch,$plist[$i]);
      }
    }
  } else {
    if ($fmt==2) {
      for (my $i=0;$i<@plist;++$i) {
	if ($plist[$i] !~ /^\s*\*\s*\*\s*\*\s*$/ && $plist[$i] =~ /^(chapter|section|book)/i) {
	  if (@$curch) {
	    push(@chlist,{ 'title' => $curtitle, 'text' => $curch});
	    $curch=[];
	  }
	  $curtitle=$plist[$i];
	} else {
	  push(@$curch,$plist[$i]);
	}
      }
    } else {
      for (my $i=0;$i<@plist-1;++$i) {
	if (!length($plist[$i-1]) && !length($plist[$i+1]) &&
	    length($plist[$i])>0 && length($plist[$i])<$max && $plist[$i] !~ /^\s*\*\s*\*\s*\*\s*$/)
	{
	  # this seems to be a chapter heading
	  if (@$curch) {
	    push(@chlist,{ 'title' => $curtitle, 'text' => $curch});
	    $curch=[];
	  }
	  $curtitle=$plist[$i];
	} else {
	  push(@$curch,$plist[$i]);
	}
      }
    }
  }
  push(@chlist,{ 'title' => $curtitle, 'text' => $curch});

  # remove empty paragraphs
  for my $chap (@chlist) {
    for (my $i=0;$i<@{$chap->{text}};++$i) {
      if (!length($chap->{text}[$i])) {
	my $j;
	for ($j=1;$i+$j<@{$chap->{text}} && !length($chap->{text}[$i+$j]);++$j) { }
	splice(@{$chap->{text}},$i,$j-1) if $j>1;
      }
    }
    shift(@{$chap->{text}}) if @{$chap->{text}} && !length($chap->{text}[0]);
    pop(@{$chap->{text}}) if @{$chap->{text}} && !length($chap->{text}[-1]);
  }
  # delete empty chapters
  @chlist=grep { length($_->{title}) || @{$_->{text}} } @chlist;
  $info.=", CH=" . ($#chlist+1);
  # return the whole thing now
  my %book;
  $book{title}=$title;
  $book{annotation}=$anno;
  $book{author}=$author;
  $book{body}=\@chlist;
  $book{file}=$infile;
  $book{html}=$html;
  $book{info}=$info;
  $book{fmt}=$fmt;
  \%book;
}

##########################################
# charset support
my @letter_maps=( ["WIN", [
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03,
0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
0x1e, 0x1f, 0x20, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
]], ["DOS" , [
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02,
0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c,
0x1d, 0x1e, 0x1f, 0x20, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,
0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x06, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
]], ["KOI", [
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x01, 0x02,
0x17, 0x05, 0x06, 0x15, 0x04, 0x16, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
0x10, 0x20, 0x11, 0x12, 0x13, 0x14, 0x07, 0x03, 0x1d, 0x1c, 0x08, 0x19, 0x1e,
0x1a, 0x18, 0x1b, 0x1f, 0x01, 0x02, 0x17, 0x05, 0x06, 0x15, 0x04, 0x16, 0x09,
0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x20, 0x11, 0x12, 0x13, 0x14, 0x07,
0x03, 0x1d, 0x1c, 0x08, 0x19, 0x1e, 0x1a, 0x18, 0x1b,
]], ["ISO", [
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
   17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
   17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    0,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
]]);
my @distrib=(
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 77, 259, 55, 298, 90, 111, 296, 12, 96, 384, 709,
284, 340, 1, 101, 381, 393, 422, 4, 1, 88, 6, 84, 49, 32, 0, 0, 0, 0, 86, 181,
0, 52, 0, 4, 0, 1, 180, 2, 0, 91, 0, 18, 86, 3, 45, 237, 0, 196, 13, 2, 73, 0,
3, 0, 0, 2, 18, 11, 271, 2, 0, 0, 36, 0, 416, 1, 7, 0, 20, 530, 0, 65, 283, 0,
12, 80, 9, 158, 548, 19, 63, 254, 15, 73, 0, 3, 2, 4, 93, 0, 0, 220, 16, 0, 0,
22, 0, 98, 0, 0, 0, 109, 30, 0, 0, 58, 0, 15, 194, 0, 59, 616, 0, 83, 0, 1, 49,
0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 412, 3, 76, 1, 5, 433, 40, 0, 214, 0, 29,
63, 2, 252, 358, 9, 186, 22, 10, 132, 0, 5, 7, 4, 3, 0, 0, 60, 42, 0, 2, 21, 0,
5, 102, 200, 260, 255, 140, 83, 113, 9, 281, 170, 544, 400, 776, 18, 84, 631,
427, 403, 10, 0, 43, 31, 105, 85, 55, 0, 0, 0, 0, 11, 15, 0, 126, 0, 0, 1, 96,
347, 1, 0, 109, 0, 10, 4, 0, 126, 8, 0, 0, 0, 0, 27, 0, 0, 0, 6, 0, 0, 0, 0, 3,
0, 0, 0, 0, 488, 17, 94, 45, 109, 49, 2, 2, 37, 0, 20, 24, 40, 154, 77, 0, 34,
0, 1, 33, 0, 0, 0, 0, 0, 0, 1, 27, 12, 0, 0, 16, 0, 4, 45, 233, 40, 120, 213,
33, 191, 33, 88, 217, 485, 226, 277, 10, 16, 85, 269, 433, 0, 0, 170, 76, 96,
64, 16, 0, 0, 0, 0, 18, 124, 0, 0, 0, 0, 8, 16, 0, 0, 0, 0, 0, 3, 0, 8, 21, 2,
0, 115, 32, 34, 0, 2, 0, 15, 17, 5, 0, 0, 0, 0, 0, 0, 0, 0, 540, 0, 6, 0, 0, 43,
1, 0, 328, 0, 55, 63, 0, 42, 800, 0, 160, 40, 39, 152, 0, 0, 3, 0, 1, 0, 0, 0,
0, 0, 0, 0, 0, 849, 3, 2, 34, 52, 501, 36, 1, 629, 0, 22, 25, 0, 37, 568, 5, 0,
166, 4, 125, 0, 0, 0, 6, 0, 0, 0, 69, 373, 0, 86, 164, 0, 248, 1, 0, 8, 0, 346,
0, 0, 234, 0, 12, 28, 1, 145, 330, 4, 6, 9, 0, 144, 0, 0, 1, 7, 0, 0, 0, 105,
7, 0, 0, 45, 0, 998, 0, 0, 1, 86, 878, 2, 30, 831, 0, 34, 0, 0, 258, 1016, 0,
6, 40, 139, 350, 1, 0, 20, 14, 0, 14, 0, 374, 77, 0, 8, 109, 0, 0, 395, 640,
446, 407, 157, 241, 123, 95, 310, 212, 633, 487, 707, 25, 108, 652, 587, 575,
6, 1, 71, 9, 157, 59, 35, 0, 0, 0, 1, 12, 61, 0, 122, 0, 0, 0, 0, 226, 0, 0,
63, 0, 6, 112, 0, 26, 909, 5, 636, 0, 4, 61, 0, 0, 0, 0, 0, 0, 0, 49, 1, 0, 0,
16, 0, 880, 4, 33, 12, 82, 654, 31, 5, 417, 0, 28, 11, 18, 111, 751, 9, 18, 29,
55, 260, 21, 21, 3, 8, 17, 2, 0, 159, 45, 0, 7, 98, 0, 239, 4, 131, 14, 21, 237,
7, 1, 111, 0, 278, 256, 86, 87, 279, 205, 17, 130, 960, 65, 3, 20, 2, 29, 4, 0,
1, 29, 317, 4, 10, 392, 0, 528, 4, 203, 0, 6, 510, 0, 0, 321, 0, 63, 36, 6, 90,
1026, 15, 248, 119, 3, 97, 0, 0, 1, 5, 1, 2, 0, 173, 553, 0, 1, 70, 0, 15, 51,
81, 80, 144, 13, 102, 32, 0, 9, 65, 227, 82, 26, 0, 65, 36, 117, 146, 0, 0, 39,
0, 77, 50, 24, 0, 0, 0, 1, 105, 6, 0, 50, 0, 0, 0, 0, 20, 0, 0, 29, 0, 0, 1, 0,
0, 27, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 28, 0, 20, 0, 0,
4, 0, 0, 16, 0, 0, 11, 11, 38, 212, 0, 15, 7, 1, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 50, 0, 7, 0, 0, 59, 0, 0, 24, 0, 0, 0, 0, 0, 27, 0, 0, 0, 0, 9, 0,
0, 0, 0, 0, 0, 0, 19, 0, 0, 0, 0, 0, 185, 0, 5, 0, 0, 226, 0, 0, 162, 0, 19, 0,
0, 94, 8, 0, 1, 0, 268, 44, 0, 0, 0, 0, 15, 0, 0, 0, 24, 0, 0, 0, 0, 66, 0, 2,
0, 0, 162, 0, 0, 155, 0, 53, 40, 0, 19, 19, 0, 1, 0, 2, 18, 0, 0, 0, 0, 0, 0,
0, 0, 58, 0, 0, 0, 0, 42, 0, 0, 0, 0, 137, 0, 0, 88, 0, 0, 0, 0, 10, 0, 0, 1,
0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 33, 75,
14, 10, 117, 6, 9, 0, 157, 11, 167, 130, 9, 0, 12, 25, 57, 76, 0, 0, 109, 0,
16, 34, 0, 0, 0, 0, 0, 0, 1, 0, 0, 7, 0, 1, 0, 27, 0, 22, 2, 0, 82, 0, 9, 129,
0, 0, 0, 104, 3, 0, 0, 0, 15, 13, 56, 0, 0, 0, 0, 0, 42, 27, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 1, 3, 0, 2, 6, 0, 0, 0, 0, 149, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 25, 0, 3, 46, 0, 26, 1, 0, 0, 0, 0, 2, 10, 0, 0, 1, 11, 43, 0, 0,
0, 0, 5, 0, 47, 0, 0, 0, 0, 2, 0, 0, 0, 0, 44, 14, 69, 10, 17, 23, 4, 19, 7,
80, 55, 57, 0, 0, 11, 63, 100, 0, 0, 21, 1, 6, 2, 39, 0, 0, 0, 0, 12, 9,
);

sub detect_charset {
  my ($msv,$enc)=(0,"WIN");
  my @chars=map { ord } split(//,$_[0]);
  return "utf-16" if $chars[0]==0xff && $chars[1]==0xfe;
  local $^W=0;
  for (@letter_maps) {
    my $map=$_->[1];
    my $last=0;
    my @dt;
    ++$dt[$last*33+$map->[$_]],$last=$map->[$_] for @chars;
    my $sum=0;
    $sum+=$dt[$_]*$distrib[$_] for 0..33*33-1;
    $enc=$_->[0],$msv=$sum if $sum>$msv;
  }
  $enc;
}

sub conv_charset {
  my $cs=detect_charset(substr($_[0],0,1024));
  if ($cs eq "KOI") {
    $_[0] =~ tr/\200-\377/\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217\220\221\222\223\224\225\226\227\230\231\232\233\234\235\236\237\240\241\242\250\272\245\263\277\250\251\252\253\254\255\256\257\260\261\262\250\252\265\262\257\270\271\272\273\274\275\276\277\376\340\341\366\344\345\364\343\365\350\351\352\353\354\355\356\357\377\360\361\362\363\346\342\374\373\347\370\375\371\367\372\336\300\301\326\304\305\324\303\325\310\311\312\313\314\315\316\317\337\320\321\322\323\306\302\334\333\307\330\335\331\327\332/;
  } elsif ($cs eq "DOS") {
    $_[0] =~ tr/\200-\377/\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356\357\260\261\262\263\264\265\266\267\270\271\272\273\274\275\276\277\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337\360\361\362\363\364\365\366\367\370\371\372\373\374\375\376\377\360\361\362\363\364\365\366\367\370\371\372\373\374\375\376\377/;
  } elsif ($cs eq "ISO") {
    $_[0] =~ tr/\200-\377/\260\261\262\263\264\265\266\267\270\271\272\273\274\275\276\277\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356\357\360\361\362\363\364\365\366\367\370\371\372\373\374\375\376\377\360\361\362\363\364\365\366\367\370\371\372\373\374\375\376\377/;
  } elsif ($cs eq "utf-16") {
    eval {
      require Unicode::String;
      $_[0]=Unicode::String::utf16($_[0])->latin1;
    };
  }
}

1;
