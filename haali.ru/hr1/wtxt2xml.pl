#!/usr/bin/perl -w

use strict;
use T2X;

###############################################
# dialog box layout
my $D_width	= 620; # dialog width
my $D_height	= 486; # dialog height
my $B_width	= 60;  # button width
my $B_height	= 24;  # button height
my $S_height	= 20;  # status line height
my $E_height	= 22;  # text field height
my $L_width	= 60;  # label width
my $Space	= 10;  # vertical spacing
my $Margin	= 12;

###############################################
# construct the window
use Win32::GUI;
use Win32::API;

my $GetSysColor=new Win32::API('user32','GetSysColor','I','N');

my $MW=new Win32::GUI::DialogBox(
  -title  => 'Text to xml conversion',
  -width  => $D_width,
  -height => $D_height,
  -left	  => 200,
  -top	  => 200,
  -name	  => 'MainWin',
  -style  => WS_OVERLAPPEDWINDOW,
  -background => $GetSysColor->Call(15) # haha doesnt work
);

$D_width=$MW->ScaleWidth;
$D_height=$MW->ScaleHeight;

my @move_controls;
my @size_controls;

push(@move_controls,[$MW->AddButton(
  -title  => '&Load...',
  -name	  => 'Load',
  -top	  => $Margin,
  -left	  => $D_width-2*$Margin-2*$B_width,
  -width  => $B_width,
  -height => $B_height,
  -tabstop=> 1
), 1,-2*$Margin-2*$B_width,0,$Margin]);

push(@move_controls,[$MW->AddButton(
  -title  => '&Save...',
  -name	  => 'Save',
  -top	  => $Margin,
  -left	  => $D_width-$Margin-$B_width,
  -width  => $B_width,
  -height => $B_height,
  -tabstop=> 1
), 1,-$Margin-$B_width,0,$Margin]);

push(@size_controls,[$MW->AddLabel(
  -name	  => 'Filename',
  -top	  => $Margin+5,
  -left	  => $Margin,
  -width  => $D_width-4*$Margin-2*$B_width,
  -height => $B_height-5,
  -noprefix => 1
), 1, -4*$Margin-2*$B_width,0,$B_height-5]);

sub Field {
  my ($n,$name,$label,$h,$re)=@_;

  $h||=1;

  my $win;
  if ($re) {
    $MW->AddLabel(
      -name   => "${name}_Label",
      -top    => $Margin+$B_height+$n*($E_height+$Space)+$Space,
      -left   => $Margin,
      -width  => $L_width,
      -height => $E_height,
      -text   => $label
    );
    $win=$MW->AddRichEdit(
      -name   => $name,
      -style  => WS_BORDER|WS_VISIBLE|WS_CHILD|WS_TABSTOP|
	($h>1 ? ES_MULTILINE|ES_WANTRETURN|ES_AUTOVSCROLL|WS_VSCROLL : ES_AUTOHSCROLL),
      -top    => $Margin+$B_height+$n*($E_height+$Space)+$Space,
      -left   => $Margin+$L_width,
      -width  => $D_width-2*$Margin-$L_width,
      -height => $h*$E_height+($h-1)*$Margin,
    );
  } else {
    $win=$MW->AddTextfield(
      -name   => $name,
      -style  => WS_BORDER|WS_VISIBLE|WS_CHILD|WS_TABSTOP|
	($h>1 ? ES_MULTILINE|ES_WANTRETURN|ES_AUTOVSCROLL|WS_VSCROLL : ES_AUTOHSCROLL),
      -top    => $Margin+$B_height+$n*($E_height+$Space)+$Space,
      -left   => $Margin,
      -width  => $D_width-2*$Margin-$L_width,
      -height => $h*$E_height+($h-1)*$Margin,
      -prompt => [ $label, $L_width ]
    );
  }
  push(@size_controls,[$win,1,-2*$Margin-$L_width,0,
      $h*$E_height+($h-1)*$Margin]);
  $win;
}

Field(0,"Title","&Title:");
Field(1,"Author","&Author:");
Field(2,"Annotation","A&nnotation:",3);
Field(5,"Book","Te&xt:",7,1);
$size_controls[-1][3]=1;
$size_controls[-1][4]=-$D_height+$size_controls[-1][0]->Height;

$MW->AddButton(
  -title  => 'Toggle&Fmt',
  -name	  => 'ToggleFmt',
  -top	  => $Margin+$B_height+5*($E_height+$Space)+$Space+$Margin+$E_height,
  -left	  => $Margin,
  -width  => $B_width,
  -height => $B_height,
  -tabstop=> 1
);

my $SB=$MW->AddStatusBar(
  -name	  => 'Status',
  -top	  => $D_height-$S_height,
  -left	  => 0,
  -width  => $D_width,
  -height => $S_height,
  -title  => 'Ready'
);
push(@move_controls,[$SB,0,0,1,-$S_height]);
push(@size_controls,[$SB,1,0,0,$S_height]);

###############################################
# global variables
my $Book={ 'file' => '', 'fmt' => 0, 'info' => '', 'title' => '', 'author' => '', 'annotation' => '', html => 0, 'body' => [] };

###############################################
# utility functions
my $dchap=4000;

sub SetFields {
  $MW->Title->Text($Book->{title});
  $MW->Author->Text($Book->{author});
  my $tmp=$Book->{annotation};
  $tmp =~ s/\n/\r\n/sg;
  $MW->Annotation->Text($tmp);
  # fill in chapters from the book
  $tmp="";
  for (my $i=0;$i<$dchap && $i<=$#{$Book->{body}};++$i) {
    $tmp.="\x87\x87\x87\x87 ";
    $tmp.=$Book->{body}[$i]{title};
    $tmp.="\r\n";
    $tmp.="    $_\r\n" for @{$Book->{body}[$i]{text}};
  }
  $MW->Book->Text($tmp);
  my $file=$Book->{file};
  $file =~ s/.*[\/\\]//;
  $MW->Filename->Text("Source file: $file, F:$Book->{fmt}");
}

sub GetFields {
  $Book->{title}=$MW->Title->Text;
  $Book->{author}=$MW->Author->Text;
  $Book->{annotation}=$MW->Annotation->Text;
  $Book->{annotation} =~ s/\r\n/\n/sg;
  # parse back the book field
  my $title;
  my @para;
  my @chap;
  for my $l (split(/\r\n/,$MW->Book->Text)) {
    if ($l =~ /^\x87+ *(.*)/) { # new chapter
      push(@chap,{ "title" => $title, "text" => [ @para ] }) if @para || defined($title);
      $title=$1;
      T2X::remws_inp($title);
      @para=();
    } else {
      T2X::remws_inp($l);
      push(@para,$l);
    }
  }
  push(@chap,{ "title" => $title, "text" => [ @para ] }) if @para || defined($title);
  # replace original chapters
  splice(@{$Book->{body}},0,$dchap,@chap);
} 

###############################################
# event handlers
sub MainWin_Terminate {
  $MW->PostQuitMessage(0);
  1;
}

sub MainWin_Resize {
  return 1 unless $MW;
  my $h=$MW->ScaleHeight;
  my $w=$MW->ScaleWidth;
  $_->[0]->Move($w*$_->[1]+$_->[2],$h*$_->[3]+$_->[4]) for @move_controls;
  $_->[0]->Resize($w*$_->[1]+$_->[2],$h*$_->[3]+$_->[4]) for @size_controls;
  1;
}

sub Load_Click {
  my $file=Win32::GUI::GetOpenFileName(
    -owner => $MW, 
    -title => "Open text file",
    -filter=> [ "Text files", "*.txt;*.html;*.htm", "All files", "*.*"]
  );
  return unless defined($file) && length($file);
  $MW->Status->Text('Loading...');
  $MW->Disable();
  eval { $Book=T2X::load_file($file,undef) };
  my $error=$@;
  $MW->Enable();
  if ($error) {
    chop($error);
    Win32::GUI::MessageBox($MW,$error,"Error",MB_OK|MB_ICONERROR);
  } else {
    SetFields;
  }
  $MW->Status->Text('Ready');
  1;
}

sub ToggleFmt_Click {
  if (defined($Book) && $Book->{file}) {
    my $file=$Book->{file};
    $MW->Status->Text('Loading...');
    $MW->Disable();
    eval { $Book=T2X::load_file($file,($Book->{fmt}+1)%4) };
    my $error=$@;
    $MW->Enable();
    if ($error) {
      chop($error);
      Win32::GUI::MessageBox($MW,$error,"Error",MB_OK|MB_ICONERROR);
    } else {
      SetFields;
    }
    $MW->Status->Text('Ready');
  }
  1;
}

sub Save_Click {
  my $fn=$Book->{file};
  if ($fn) {
    $fn.=".xml" unless $fn =~ s/\.(txt|html|htm)$/.xml/i;
  }
  my $file=Win32::GUI::GetSaveFileName(
    -owner => $MW,
    -title => "Save XML file",
    -filter=> [ "XML files", "*.xml" ],
    -file  => $fn
  );
  return unless defined($file) && length($file);
  if (-f $file) {
    return unless 6==Win32::GUI::MessageBox($MW,"Overwrite $file?",
				"Save",MB_YESNO|MB_ICONQUESTION);
  }
  if (!open(FF,"> $file")) {
    Win32::GUI::MessageBox($MW,"Can't open $file: $!","Error",MB_OK|MB_ICONERROR);
    return;
  }
  binmode(FF);
  GetFields;
  my $tmp=T2X::make_xml($Book);
  #$tmp =~ s/>\s+</></sg;
  print FF $tmp;
  close(FF);
  1;
}

$MW->Show();
$MW->Load->SetFocus();
Win32::GUI::Dialog();

