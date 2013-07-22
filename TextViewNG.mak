# Microsoft Developer Studio Generated NMAKE File, Based on tvngx86.dsp
!IF "$(CFG)" == ""
CFG=tvngx86 - Win32 Unicode Debug
!MESSAGE No configuration specified. Defaulting to tvngx86 - Win32 Unicode Debug.
!ENDIF 

!IF "$(CFG)" != "tvngx86 - Win32 Unicode Release" && "$(CFG)" != "tvngx86 - Win32 Unicode Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tvngx86.mak" CFG="tvngx86 - Win32 Unicode Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tvngx86 - Win32 Unicode Release" (based on "Win32 (x86) Application")
!MESSAGE "tvngx86 - Win32 Unicode Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"

OUTDIR=.\DX86UnicodeRel
INTDIR=.\DX86UnicodeRel
# Begin Custom Macros
OutDir=.\DX86UnicodeRel
# End Custom Macros

ALL : "$(OUTDIR)\tvngx86.exe"


CLEAN :
	-@erase "$(INTDIR)\adler32.obj"
	-@erase "$(INTDIR)\Bookmarks.obj"
	-@erase "$(INTDIR)\BufFile.obj"
	-@erase "$(INTDIR)\ColorSelector.obj"
	-@erase "$(INTDIR)\ContentsDlg.obj"
	-@erase "$(INTDIR)\crc32.obj"
	-@erase "$(INTDIR)\Dictionary.obj"
	-@erase "$(INTDIR)\DictSetupDlg.obj"
	-@erase "$(INTDIR)\DummyView.obj"
	-@erase "$(INTDIR)\FastArray.obj"
	-@erase "$(INTDIR)\FDC.obj"
	-@erase "$(INTDIR)\FileFormatDialog.obj"
	-@erase "$(INTDIR)\FileOpenDialog.obj"
	-@erase "$(INTDIR)\Image.obj"
	-@erase "$(INTDIR)\infblock.obj"
	-@erase "$(INTDIR)\infcodes.obj"
	-@erase "$(INTDIR)\inffast.obj"
	-@erase "$(INTDIR)\inflate.obj"
	-@erase "$(INTDIR)\inftrees.obj"
	-@erase "$(INTDIR)\infutil.obj"
	-@erase "$(INTDIR)\InputBox.obj"
	-@erase "$(INTDIR)\jcomapi.obj"
	-@erase "$(INTDIR)\jdapimin.obj"
	-@erase "$(INTDIR)\jdapistd.obj"
	-@erase "$(INTDIR)\jdcoefct.obj"
	-@erase "$(INTDIR)\jdcolor.obj"
	-@erase "$(INTDIR)\jddctmgr.obj"
	-@erase "$(INTDIR)\jdhuff.obj"
	-@erase "$(INTDIR)\jdinput.obj"
	-@erase "$(INTDIR)\jdmainct.obj"
	-@erase "$(INTDIR)\jdmarker.obj"
	-@erase "$(INTDIR)\jdmaster.obj"
	-@erase "$(INTDIR)\jdmerge.obj"
	-@erase "$(INTDIR)\jdphuff.obj"
	-@erase "$(INTDIR)\jdpostct.obj"
	-@erase "$(INTDIR)\jdsample.obj"
	-@erase "$(INTDIR)\jdtrans.obj"
	-@erase "$(INTDIR)\jerror.obj"
	-@erase "$(INTDIR)\jidctfst.obj"
	-@erase "$(INTDIR)\jidctint.obj"
	-@erase "$(INTDIR)\jidctred.obj"
	-@erase "$(INTDIR)\jmemmgr.obj"
	-@erase "$(INTDIR)\jmemnobs.obj"
	-@erase "$(INTDIR)\jutils.obj"
	-@erase "$(INTDIR)\Keys.obj"
	-@erase "$(INTDIR)\MiscOptDlg.obj"
	-@erase "$(INTDIR)\OptionsDialog.obj"
	-@erase "$(INTDIR)\PDBFile.obj"
	-@erase "$(INTDIR)\png.obj"
	-@erase "$(INTDIR)\pngerror.obj"
	-@erase "$(INTDIR)\pngget.obj"
	-@erase "$(INTDIR)\pngmem.obj"
	-@erase "$(INTDIR)\pngpread.obj"
	-@erase "$(INTDIR)\pngread.obj"
	-@erase "$(INTDIR)\pngrio.obj"
	-@erase "$(INTDIR)\pngrtran.obj"
	-@erase "$(INTDIR)\pngrutil.obj"
	-@erase "$(INTDIR)\pngset.obj"
	-@erase "$(INTDIR)\pngtrans.obj"
	-@erase "$(INTDIR)\ProgressDlg.obj"
	-@erase "$(INTDIR)\RFile.obj"
	-@erase "$(INTDIR)\StrBuf.obj"
	-@erase "$(INTDIR)\StylesDlg.obj"
	-@erase "$(INTDIR)\TCRFile.obj"
	-@erase "$(INTDIR)\TextFile.obj"
	-@erase "$(INTDIR)\TextFormatter.obj"
	-@erase "$(INTDIR)\TextParser.obj"
	-@erase "$(INTDIR)\TextViewNG.obj"
	-@erase "$(INTDIR)\TextViewNG.res"
	-@erase "$(INTDIR)\TVFrame.obj"
	-@erase "$(INTDIR)\TView.obj"
	-@erase "$(INTDIR)\uncompr.obj"
	-@erase "$(INTDIR)\Unicode.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\WMap.obj"
	-@erase "$(INTDIR)\XListBox.obj"
	-@erase "$(INTDIR)\xmlparse.obj"
	-@erase "$(INTDIR)\XMLParser.obj"
	-@erase "$(INTDIR)\xmlrole.obj"
	-@erase "$(INTDIR)\xmltok.obj"
	-@erase "$(INTDIR)\xscanf.obj"
	-@erase "$(INTDIR)\ZipFile.obj"
	-@erase "$(INTDIR)\zutil.obj"
	-@erase "$(OUTDIR)\tvngx86.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /Ox /Os /I "zlib" /I "expat" /I "libjpeg" /I "libpng" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /Fp"$(INTDIR)\tvngx86.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\TextViewNG.res" /i "win32-x86" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\tvngx86.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=unicows.lib kernel32.lib user32.lib gdi32.lib advapi32.lib comctl32.lib shell32.lib winspool.lib uuid.lib uafxcw.lib libcmt.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\tvngx86.pdb" /machine:I386 /nodefaultlib /out:"$(OUTDIR)\tvngx86.exe" /opt:nowin98 
LINK32_OBJS= \
	"$(INTDIR)\adler32.obj" \
	"$(INTDIR)\Bookmarks.obj" \
	"$(INTDIR)\BufFile.obj" \
	"$(INTDIR)\ColorSelector.obj" \
	"$(INTDIR)\ContentsDlg.obj" \
	"$(INTDIR)\crc32.obj" \
	"$(INTDIR)\Dictionary.obj" \
	"$(INTDIR)\DictSetupDlg.obj" \
	"$(INTDIR)\DummyView.obj" \
	"$(INTDIR)\FastArray.obj" \
	"$(INTDIR)\FDC.obj" \
	"$(INTDIR)\FileFormatDialog.obj" \
	"$(INTDIR)\FileOpenDialog.obj" \
	"$(INTDIR)\Image.obj" \
	"$(INTDIR)\infblock.obj" \
	"$(INTDIR)\infcodes.obj" \
	"$(INTDIR)\inffast.obj" \
	"$(INTDIR)\inflate.obj" \
	"$(INTDIR)\inftrees.obj" \
	"$(INTDIR)\infutil.obj" \
	"$(INTDIR)\InputBox.obj" \
	"$(INTDIR)\jcomapi.obj" \
	"$(INTDIR)\jdapimin.obj" \
	"$(INTDIR)\jdapistd.obj" \
	"$(INTDIR)\jdcoefct.obj" \
	"$(INTDIR)\jdcolor.obj" \
	"$(INTDIR)\jddctmgr.obj" \
	"$(INTDIR)\jdhuff.obj" \
	"$(INTDIR)\jdinput.obj" \
	"$(INTDIR)\jdmainct.obj" \
	"$(INTDIR)\jdmarker.obj" \
	"$(INTDIR)\jdmaster.obj" \
	"$(INTDIR)\jdmerge.obj" \
	"$(INTDIR)\jdphuff.obj" \
	"$(INTDIR)\jdpostct.obj" \
	"$(INTDIR)\jdsample.obj" \
	"$(INTDIR)\jdtrans.obj" \
	"$(INTDIR)\jerror.obj" \
	"$(INTDIR)\jidctfst.obj" \
	"$(INTDIR)\jidctint.obj" \
	"$(INTDIR)\jidctred.obj" \
	"$(INTDIR)\jmemmgr.obj" \
	"$(INTDIR)\jmemnobs.obj" \
	"$(INTDIR)\jutils.obj" \
	"$(INTDIR)\Keys.obj" \
	"$(INTDIR)\MiscOptDlg.obj" \
	"$(INTDIR)\OptionsDialog.obj" \
	"$(INTDIR)\PDBFile.obj" \
	"$(INTDIR)\png.obj" \
	"$(INTDIR)\pngerror.obj" \
	"$(INTDIR)\pngget.obj" \
	"$(INTDIR)\pngmem.obj" \
	"$(INTDIR)\pngpread.obj" \
	"$(INTDIR)\pngread.obj" \
	"$(INTDIR)\pngrio.obj" \
	"$(INTDIR)\pngrtran.obj" \
	"$(INTDIR)\pngrutil.obj" \
	"$(INTDIR)\pngset.obj" \
	"$(INTDIR)\pngtrans.obj" \
	"$(INTDIR)\ProgressDlg.obj" \
	"$(INTDIR)\RFile.obj" \
	"$(INTDIR)\StrBuf.obj" \
	"$(INTDIR)\StylesDlg.obj" \
	"$(INTDIR)\TextFile.obj" \
	"$(INTDIR)\TextFormatter.obj" \
	"$(INTDIR)\TextParser.obj" \
	"$(INTDIR)\TextViewNG.obj" \
	"$(INTDIR)\TVFrame.obj" \
	"$(INTDIR)\TView.obj" \
	"$(INTDIR)\uncompr.obj" \
	"$(INTDIR)\Unicode.obj" \
	"$(INTDIR)\WMap.obj" \
	"$(INTDIR)\XListBox.obj" \
	"$(INTDIR)\xmlparse.obj" \
	"$(INTDIR)\XMLParser.obj" \
	"$(INTDIR)\xmlrole.obj" \
	"$(INTDIR)\xmltok.obj" \
	"$(INTDIR)\xscanf.obj" \
	"$(INTDIR)\ZipFile.obj" \
	"$(INTDIR)\zutil.obj" \
	"$(INTDIR)\TextViewNG.res" \
	"$(INTDIR)\TCRFile.obj"

"$(OUTDIR)\tvngx86.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"

OUTDIR=.\DX86UnicodeDebug
INTDIR=.\DX86UnicodeDebug
# Begin Custom Macros
OutDir=.\DX86UnicodeDebug
# End Custom Macros

ALL : "$(OUTDIR)\tvngx86.exe" "$(OUTDIR)\tvngx86.bsc"


CLEAN :
	-@erase "$(INTDIR)\adler32.obj"
	-@erase "$(INTDIR)\adler32.sbr"
	-@erase "$(INTDIR)\Bookmarks.obj"
	-@erase "$(INTDIR)\Bookmarks.sbr"
	-@erase "$(INTDIR)\BufFile.obj"
	-@erase "$(INTDIR)\BufFile.sbr"
	-@erase "$(INTDIR)\ColorSelector.obj"
	-@erase "$(INTDIR)\ColorSelector.sbr"
	-@erase "$(INTDIR)\ContentsDlg.obj"
	-@erase "$(INTDIR)\ContentsDlg.sbr"
	-@erase "$(INTDIR)\crc32.obj"
	-@erase "$(INTDIR)\crc32.sbr"
	-@erase "$(INTDIR)\Dictionary.obj"
	-@erase "$(INTDIR)\Dictionary.sbr"
	-@erase "$(INTDIR)\DictSetupDlg.obj"
	-@erase "$(INTDIR)\DictSetupDlg.sbr"
	-@erase "$(INTDIR)\DummyView.obj"
	-@erase "$(INTDIR)\DummyView.sbr"
	-@erase "$(INTDIR)\FastArray.obj"
	-@erase "$(INTDIR)\FastArray.sbr"
	-@erase "$(INTDIR)\FDC.obj"
	-@erase "$(INTDIR)\FDC.sbr"
	-@erase "$(INTDIR)\FileFormatDialog.obj"
	-@erase "$(INTDIR)\FileFormatDialog.sbr"
	-@erase "$(INTDIR)\FileOpenDialog.obj"
	-@erase "$(INTDIR)\FileOpenDialog.sbr"
	-@erase "$(INTDIR)\Image.obj"
	-@erase "$(INTDIR)\Image.sbr"
	-@erase "$(INTDIR)\infblock.obj"
	-@erase "$(INTDIR)\infblock.sbr"
	-@erase "$(INTDIR)\infcodes.obj"
	-@erase "$(INTDIR)\infcodes.sbr"
	-@erase "$(INTDIR)\inffast.obj"
	-@erase "$(INTDIR)\inffast.sbr"
	-@erase "$(INTDIR)\inflate.obj"
	-@erase "$(INTDIR)\inflate.sbr"
	-@erase "$(INTDIR)\inftrees.obj"
	-@erase "$(INTDIR)\inftrees.sbr"
	-@erase "$(INTDIR)\infutil.obj"
	-@erase "$(INTDIR)\infutil.sbr"
	-@erase "$(INTDIR)\InputBox.obj"
	-@erase "$(INTDIR)\InputBox.sbr"
	-@erase "$(INTDIR)\jcomapi.obj"
	-@erase "$(INTDIR)\jcomapi.sbr"
	-@erase "$(INTDIR)\jdapimin.obj"
	-@erase "$(INTDIR)\jdapimin.sbr"
	-@erase "$(INTDIR)\jdapistd.obj"
	-@erase "$(INTDIR)\jdapistd.sbr"
	-@erase "$(INTDIR)\jdcoefct.obj"
	-@erase "$(INTDIR)\jdcoefct.sbr"
	-@erase "$(INTDIR)\jdcolor.obj"
	-@erase "$(INTDIR)\jdcolor.sbr"
	-@erase "$(INTDIR)\jddctmgr.obj"
	-@erase "$(INTDIR)\jddctmgr.sbr"
	-@erase "$(INTDIR)\jdhuff.obj"
	-@erase "$(INTDIR)\jdhuff.sbr"
	-@erase "$(INTDIR)\jdinput.obj"
	-@erase "$(INTDIR)\jdinput.sbr"
	-@erase "$(INTDIR)\jdmainct.obj"
	-@erase "$(INTDIR)\jdmainct.sbr"
	-@erase "$(INTDIR)\jdmarker.obj"
	-@erase "$(INTDIR)\jdmarker.sbr"
	-@erase "$(INTDIR)\jdmaster.obj"
	-@erase "$(INTDIR)\jdmaster.sbr"
	-@erase "$(INTDIR)\jdmerge.obj"
	-@erase "$(INTDIR)\jdmerge.sbr"
	-@erase "$(INTDIR)\jdphuff.obj"
	-@erase "$(INTDIR)\jdphuff.sbr"
	-@erase "$(INTDIR)\jdpostct.obj"
	-@erase "$(INTDIR)\jdpostct.sbr"
	-@erase "$(INTDIR)\jdsample.obj"
	-@erase "$(INTDIR)\jdsample.sbr"
	-@erase "$(INTDIR)\jdtrans.obj"
	-@erase "$(INTDIR)\jdtrans.sbr"
	-@erase "$(INTDIR)\jerror.obj"
	-@erase "$(INTDIR)\jerror.sbr"
	-@erase "$(INTDIR)\jidctfst.obj"
	-@erase "$(INTDIR)\jidctfst.sbr"
	-@erase "$(INTDIR)\jidctint.obj"
	-@erase "$(INTDIR)\jidctint.sbr"
	-@erase "$(INTDIR)\jidctred.obj"
	-@erase "$(INTDIR)\jidctred.sbr"
	-@erase "$(INTDIR)\jmemmgr.obj"
	-@erase "$(INTDIR)\jmemmgr.sbr"
	-@erase "$(INTDIR)\jmemnobs.obj"
	-@erase "$(INTDIR)\jmemnobs.sbr"
	-@erase "$(INTDIR)\jutils.obj"
	-@erase "$(INTDIR)\jutils.sbr"
	-@erase "$(INTDIR)\Keys.obj"
	-@erase "$(INTDIR)\Keys.sbr"
	-@erase "$(INTDIR)\MiscOptDlg.obj"
	-@erase "$(INTDIR)\MiscOptDlg.sbr"
	-@erase "$(INTDIR)\OptionsDialog.obj"
	-@erase "$(INTDIR)\OptionsDialog.sbr"
	-@erase "$(INTDIR)\PDBFile.obj"
	-@erase "$(INTDIR)\PDBFile.sbr"
	-@erase "$(INTDIR)\png.obj"
	-@erase "$(INTDIR)\png.sbr"
	-@erase "$(INTDIR)\pngerror.obj"
	-@erase "$(INTDIR)\pngerror.sbr"
	-@erase "$(INTDIR)\pngget.obj"
	-@erase "$(INTDIR)\pngget.sbr"
	-@erase "$(INTDIR)\pngmem.obj"
	-@erase "$(INTDIR)\pngmem.sbr"
	-@erase "$(INTDIR)\pngpread.obj"
	-@erase "$(INTDIR)\pngpread.sbr"
	-@erase "$(INTDIR)\pngread.obj"
	-@erase "$(INTDIR)\pngread.sbr"
	-@erase "$(INTDIR)\pngrio.obj"
	-@erase "$(INTDIR)\pngrio.sbr"
	-@erase "$(INTDIR)\pngrtran.obj"
	-@erase "$(INTDIR)\pngrtran.sbr"
	-@erase "$(INTDIR)\pngrutil.obj"
	-@erase "$(INTDIR)\pngrutil.sbr"
	-@erase "$(INTDIR)\pngset.obj"
	-@erase "$(INTDIR)\pngset.sbr"
	-@erase "$(INTDIR)\pngtrans.obj"
	-@erase "$(INTDIR)\pngtrans.sbr"
	-@erase "$(INTDIR)\ProgressDlg.obj"
	-@erase "$(INTDIR)\ProgressDlg.sbr"
	-@erase "$(INTDIR)\RFile.obj"
	-@erase "$(INTDIR)\RFile.sbr"
	-@erase "$(INTDIR)\StrBuf.obj"
	-@erase "$(INTDIR)\StrBuf.sbr"
	-@erase "$(INTDIR)\StylesDlg.obj"
	-@erase "$(INTDIR)\StylesDlg.sbr"
	-@erase "$(INTDIR)\TCRFile.obj"
	-@erase "$(INTDIR)\TCRFile.sbr"
	-@erase "$(INTDIR)\TextFile.obj"
	-@erase "$(INTDIR)\TextFile.sbr"
	-@erase "$(INTDIR)\TextFormatter.obj"
	-@erase "$(INTDIR)\TextFormatter.sbr"
	-@erase "$(INTDIR)\TextParser.obj"
	-@erase "$(INTDIR)\TextParser.sbr"
	-@erase "$(INTDIR)\TextViewNG.obj"
	-@erase "$(INTDIR)\TextViewNG.res"
	-@erase "$(INTDIR)\TextViewNG.sbr"
	-@erase "$(INTDIR)\TVFrame.obj"
	-@erase "$(INTDIR)\TVFrame.sbr"
	-@erase "$(INTDIR)\TView.obj"
	-@erase "$(INTDIR)\TView.sbr"
	-@erase "$(INTDIR)\uncompr.obj"
	-@erase "$(INTDIR)\uncompr.sbr"
	-@erase "$(INTDIR)\Unicode.obj"
	-@erase "$(INTDIR)\Unicode.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\WMap.obj"
	-@erase "$(INTDIR)\WMap.sbr"
	-@erase "$(INTDIR)\XListBox.obj"
	-@erase "$(INTDIR)\XListBox.sbr"
	-@erase "$(INTDIR)\xmlparse.obj"
	-@erase "$(INTDIR)\xmlparse.sbr"
	-@erase "$(INTDIR)\XMLParser.obj"
	-@erase "$(INTDIR)\XMLParser.sbr"
	-@erase "$(INTDIR)\xmlrole.obj"
	-@erase "$(INTDIR)\xmlrole.sbr"
	-@erase "$(INTDIR)\xmltok.obj"
	-@erase "$(INTDIR)\xmltok.sbr"
	-@erase "$(INTDIR)\xscanf.obj"
	-@erase "$(INTDIR)\xscanf.sbr"
	-@erase "$(INTDIR)\ZipFile.obj"
	-@erase "$(INTDIR)\ZipFile.sbr"
	-@erase "$(INTDIR)\zutil.obj"
	-@erase "$(INTDIR)\zutil.sbr"
	-@erase "$(OUTDIR)\tvngx86.bsc"
	-@erase "$(OUTDIR)\tvngx86.exe"
	-@erase "$(OUTDIR)\tvngx86.ilk"
	-@erase "$(OUTDIR)\tvngx86.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "zlib" /I "expat" /I "libjpeg" /I "libpng" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\tvngx86.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\TextViewNG.res" /i "win32-x86" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\tvngx86.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\adler32.sbr" \
	"$(INTDIR)\Bookmarks.sbr" \
	"$(INTDIR)\BufFile.sbr" \
	"$(INTDIR)\ColorSelector.sbr" \
	"$(INTDIR)\ContentsDlg.sbr" \
	"$(INTDIR)\crc32.sbr" \
	"$(INTDIR)\Dictionary.sbr" \
	"$(INTDIR)\DictSetupDlg.sbr" \
	"$(INTDIR)\DummyView.sbr" \
	"$(INTDIR)\FastArray.sbr" \
	"$(INTDIR)\FDC.sbr" \
	"$(INTDIR)\FileFormatDialog.sbr" \
	"$(INTDIR)\FileOpenDialog.sbr" \
	"$(INTDIR)\Image.sbr" \
	"$(INTDIR)\infblock.sbr" \
	"$(INTDIR)\infcodes.sbr" \
	"$(INTDIR)\inffast.sbr" \
	"$(INTDIR)\inflate.sbr" \
	"$(INTDIR)\inftrees.sbr" \
	"$(INTDIR)\infutil.sbr" \
	"$(INTDIR)\InputBox.sbr" \
	"$(INTDIR)\jcomapi.sbr" \
	"$(INTDIR)\jdapimin.sbr" \
	"$(INTDIR)\jdapistd.sbr" \
	"$(INTDIR)\jdcoefct.sbr" \
	"$(INTDIR)\jdcolor.sbr" \
	"$(INTDIR)\jddctmgr.sbr" \
	"$(INTDIR)\jdhuff.sbr" \
	"$(INTDIR)\jdinput.sbr" \
	"$(INTDIR)\jdmainct.sbr" \
	"$(INTDIR)\jdmarker.sbr" \
	"$(INTDIR)\jdmaster.sbr" \
	"$(INTDIR)\jdmerge.sbr" \
	"$(INTDIR)\jdphuff.sbr" \
	"$(INTDIR)\jdpostct.sbr" \
	"$(INTDIR)\jdsample.sbr" \
	"$(INTDIR)\jdtrans.sbr" \
	"$(INTDIR)\jerror.sbr" \
	"$(INTDIR)\jidctfst.sbr" \
	"$(INTDIR)\jidctint.sbr" \
	"$(INTDIR)\jidctred.sbr" \
	"$(INTDIR)\jmemmgr.sbr" \
	"$(INTDIR)\jmemnobs.sbr" \
	"$(INTDIR)\jutils.sbr" \
	"$(INTDIR)\Keys.sbr" \
	"$(INTDIR)\MiscOptDlg.sbr" \
	"$(INTDIR)\OptionsDialog.sbr" \
	"$(INTDIR)\PDBFile.sbr" \
	"$(INTDIR)\png.sbr" \
	"$(INTDIR)\pngerror.sbr" \
	"$(INTDIR)\pngget.sbr" \
	"$(INTDIR)\pngmem.sbr" \
	"$(INTDIR)\pngpread.sbr" \
	"$(INTDIR)\pngread.sbr" \
	"$(INTDIR)\pngrio.sbr" \
	"$(INTDIR)\pngrtran.sbr" \
	"$(INTDIR)\pngrutil.sbr" \
	"$(INTDIR)\pngset.sbr" \
	"$(INTDIR)\pngtrans.sbr" \
	"$(INTDIR)\ProgressDlg.sbr" \
	"$(INTDIR)\RFile.sbr" \
	"$(INTDIR)\StrBuf.sbr" \
	"$(INTDIR)\StylesDlg.sbr" \
	"$(INTDIR)\TextFile.sbr" \
	"$(INTDIR)\TextFormatter.sbr" \
	"$(INTDIR)\TextParser.sbr" \
	"$(INTDIR)\TextViewNG.sbr" \
	"$(INTDIR)\TVFrame.sbr" \
	"$(INTDIR)\TView.sbr" \
	"$(INTDIR)\uncompr.sbr" \
	"$(INTDIR)\Unicode.sbr" \
	"$(INTDIR)\WMap.sbr" \
	"$(INTDIR)\XListBox.sbr" \
	"$(INTDIR)\xmlparse.sbr" \
	"$(INTDIR)\XMLParser.sbr" \
	"$(INTDIR)\xmlrole.sbr" \
	"$(INTDIR)\xmltok.sbr" \
	"$(INTDIR)\xscanf.sbr" \
	"$(INTDIR)\ZipFile.sbr" \
	"$(INTDIR)\zutil.sbr" \
	"$(INTDIR)\TCRFile.sbr"

"$(OUTDIR)\tvngx86.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=unicows.lib kernel32.lib user32.lib gdi32.lib advapi32.lib comctl32.lib shell32.lib winspool.lib uuid.lib uafxcwd.lib libcmtd.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\tvngx86.pdb" /debug /machine:I386 /nodefaultlib /out:"$(OUTDIR)\tvngx86.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\adler32.obj" \
	"$(INTDIR)\Bookmarks.obj" \
	"$(INTDIR)\BufFile.obj" \
	"$(INTDIR)\ColorSelector.obj" \
	"$(INTDIR)\ContentsDlg.obj" \
	"$(INTDIR)\crc32.obj" \
	"$(INTDIR)\Dictionary.obj" \
	"$(INTDIR)\DictSetupDlg.obj" \
	"$(INTDIR)\DummyView.obj" \
	"$(INTDIR)\FastArray.obj" \
	"$(INTDIR)\FDC.obj" \
	"$(INTDIR)\FileFormatDialog.obj" \
	"$(INTDIR)\FileOpenDialog.obj" \
	"$(INTDIR)\Image.obj" \
	"$(INTDIR)\infblock.obj" \
	"$(INTDIR)\infcodes.obj" \
	"$(INTDIR)\inffast.obj" \
	"$(INTDIR)\inflate.obj" \
	"$(INTDIR)\inftrees.obj" \
	"$(INTDIR)\infutil.obj" \
	"$(INTDIR)\InputBox.obj" \
	"$(INTDIR)\jcomapi.obj" \
	"$(INTDIR)\jdapimin.obj" \
	"$(INTDIR)\jdapistd.obj" \
	"$(INTDIR)\jdcoefct.obj" \
	"$(INTDIR)\jdcolor.obj" \
	"$(INTDIR)\jddctmgr.obj" \
	"$(INTDIR)\jdhuff.obj" \
	"$(INTDIR)\jdinput.obj" \
	"$(INTDIR)\jdmainct.obj" \
	"$(INTDIR)\jdmarker.obj" \
	"$(INTDIR)\jdmaster.obj" \
	"$(INTDIR)\jdmerge.obj" \
	"$(INTDIR)\jdphuff.obj" \
	"$(INTDIR)\jdpostct.obj" \
	"$(INTDIR)\jdsample.obj" \
	"$(INTDIR)\jdtrans.obj" \
	"$(INTDIR)\jerror.obj" \
	"$(INTDIR)\jidctfst.obj" \
	"$(INTDIR)\jidctint.obj" \
	"$(INTDIR)\jidctred.obj" \
	"$(INTDIR)\jmemmgr.obj" \
	"$(INTDIR)\jmemnobs.obj" \
	"$(INTDIR)\jutils.obj" \
	"$(INTDIR)\Keys.obj" \
	"$(INTDIR)\MiscOptDlg.obj" \
	"$(INTDIR)\OptionsDialog.obj" \
	"$(INTDIR)\PDBFile.obj" \
	"$(INTDIR)\png.obj" \
	"$(INTDIR)\pngerror.obj" \
	"$(INTDIR)\pngget.obj" \
	"$(INTDIR)\pngmem.obj" \
	"$(INTDIR)\pngpread.obj" \
	"$(INTDIR)\pngread.obj" \
	"$(INTDIR)\pngrio.obj" \
	"$(INTDIR)\pngrtran.obj" \
	"$(INTDIR)\pngrutil.obj" \
	"$(INTDIR)\pngset.obj" \
	"$(INTDIR)\pngtrans.obj" \
	"$(INTDIR)\ProgressDlg.obj" \
	"$(INTDIR)\RFile.obj" \
	"$(INTDIR)\StrBuf.obj" \
	"$(INTDIR)\StylesDlg.obj" \
	"$(INTDIR)\TextFile.obj" \
	"$(INTDIR)\TextFormatter.obj" \
	"$(INTDIR)\TextParser.obj" \
	"$(INTDIR)\TextViewNG.obj" \
	"$(INTDIR)\TVFrame.obj" \
	"$(INTDIR)\TView.obj" \
	"$(INTDIR)\uncompr.obj" \
	"$(INTDIR)\Unicode.obj" \
	"$(INTDIR)\WMap.obj" \
	"$(INTDIR)\XListBox.obj" \
	"$(INTDIR)\xmlparse.obj" \
	"$(INTDIR)\XMLParser.obj" \
	"$(INTDIR)\xmlrole.obj" \
	"$(INTDIR)\xmltok.obj" \
	"$(INTDIR)\xscanf.obj" \
	"$(INTDIR)\ZipFile.obj" \
	"$(INTDIR)\zutil.obj" \
	"$(INTDIR)\TextViewNG.res" \
	"$(INTDIR)\TCRFile.obj"

"$(OUTDIR)\tvngx86.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("tvngx86.dep")
!INCLUDE "tvngx86.dep"
!ELSE 
!MESSAGE Warning: cannot find "tvngx86.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "tvngx86 - Win32 Unicode Release" || "$(CFG)" == "tvngx86 - Win32 Unicode Debug"
SOURCE=.\zlib\adler32.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\adler32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\adler32.obj"	"$(INTDIR)\adler32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Bookmarks.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\Bookmarks.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\Bookmarks.obj"	"$(INTDIR)\Bookmarks.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\BufFile.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\BufFile.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\BufFile.obj"	"$(INTDIR)\BufFile.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\ColorSelector.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\ColorSelector.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\ColorSelector.obj"	"$(INTDIR)\ColorSelector.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\ContentsDlg.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\ContentsDlg.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\ContentsDlg.obj"	"$(INTDIR)\ContentsDlg.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\zlib\crc32.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\crc32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\crc32.obj"	"$(INTDIR)\crc32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Dictionary.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\Dictionary.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\Dictionary.obj"	"$(INTDIR)\Dictionary.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\DictSetupDlg.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\DictSetupDlg.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\DictSetupDlg.obj"	"$(INTDIR)\DictSetupDlg.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\DummyView.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\DummyView.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\DummyView.obj"	"$(INTDIR)\DummyView.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\FastArray.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\FastArray.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\FastArray.obj"	"$(INTDIR)\FastArray.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\FDC.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\FDC.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\FDC.obj"	"$(INTDIR)\FDC.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\FileFormatDialog.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\FileFormatDialog.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\FileFormatDialog.obj"	"$(INTDIR)\FileFormatDialog.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\FileOpenDialog.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\FileOpenDialog.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\FileOpenDialog.obj"	"$(INTDIR)\FileOpenDialog.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\Image.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\Image.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\Image.obj"	"$(INTDIR)\Image.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\zlib\infblock.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\infblock.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\infblock.obj"	"$(INTDIR)\infblock.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\zlib\infcodes.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\infcodes.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\infcodes.obj"	"$(INTDIR)\infcodes.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\zlib\inffast.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\inffast.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\inffast.obj"	"$(INTDIR)\inffast.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\zlib\inflate.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\inflate.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\inflate.obj"	"$(INTDIR)\inflate.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\zlib\inftrees.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\inftrees.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\inftrees.obj"	"$(INTDIR)\inftrees.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\zlib\infutil.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\infutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\infutil.obj"	"$(INTDIR)\infutil.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\InputBox.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\InputBox.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\InputBox.obj"	"$(INTDIR)\InputBox.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\libjpeg\jcomapi.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\jcomapi.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\jcomapi.obj"	"$(INTDIR)\jcomapi.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libjpeg\jdapimin.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\jdapimin.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\jdapimin.obj"	"$(INTDIR)\jdapimin.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libjpeg\jdapistd.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\jdapistd.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\jdapistd.obj"	"$(INTDIR)\jdapistd.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libjpeg\jdcoefct.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\jdcoefct.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\jdcoefct.obj"	"$(INTDIR)\jdcoefct.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libjpeg\jdcolor.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\jdcolor.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\jdcolor.obj"	"$(INTDIR)\jdcolor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libjpeg\jddctmgr.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\jddctmgr.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\jddctmgr.obj"	"$(INTDIR)\jddctmgr.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libjpeg\jdhuff.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\jdhuff.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\jdhuff.obj"	"$(INTDIR)\jdhuff.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libjpeg\jdinput.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\jdinput.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\jdinput.obj"	"$(INTDIR)\jdinput.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libjpeg\jdmainct.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\jdmainct.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\jdmainct.obj"	"$(INTDIR)\jdmainct.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libjpeg\jdmarker.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\jdmarker.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\jdmarker.obj"	"$(INTDIR)\jdmarker.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libjpeg\jdmaster.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\jdmaster.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\jdmaster.obj"	"$(INTDIR)\jdmaster.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libjpeg\jdmerge.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\jdmerge.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\jdmerge.obj"	"$(INTDIR)\jdmerge.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libjpeg\jdphuff.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\jdphuff.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\jdphuff.obj"	"$(INTDIR)\jdphuff.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libjpeg\jdpostct.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\jdpostct.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\jdpostct.obj"	"$(INTDIR)\jdpostct.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libjpeg\jdsample.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\jdsample.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\jdsample.obj"	"$(INTDIR)\jdsample.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libjpeg\jdtrans.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\jdtrans.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\jdtrans.obj"	"$(INTDIR)\jdtrans.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libjpeg\jerror.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\jerror.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\jerror.obj"	"$(INTDIR)\jerror.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libjpeg\jidctfst.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\jidctfst.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\jidctfst.obj"	"$(INTDIR)\jidctfst.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libjpeg\jidctint.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\jidctint.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\jidctint.obj"	"$(INTDIR)\jidctint.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libjpeg\jidctred.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\jidctred.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\jidctred.obj"	"$(INTDIR)\jidctred.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libjpeg\jmemmgr.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\jmemmgr.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\jmemmgr.obj"	"$(INTDIR)\jmemmgr.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libjpeg\jmemnobs.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\jmemnobs.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\jmemnobs.obj"	"$(INTDIR)\jmemnobs.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libjpeg\jutils.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\jutils.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\jutils.obj"	"$(INTDIR)\jutils.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Keys.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\Keys.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\Keys.obj"	"$(INTDIR)\Keys.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\MiscOptDlg.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\MiscOptDlg.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\MiscOptDlg.obj"	"$(INTDIR)\MiscOptDlg.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\OptionsDialog.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\OptionsDialog.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\OptionsDialog.obj"	"$(INTDIR)\OptionsDialog.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\PDBFile.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\PDBFile.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\PDBFile.obj"	"$(INTDIR)\PDBFile.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\libpng\png.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\png.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\png.obj"	"$(INTDIR)\png.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libpng\pngerror.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\pngerror.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\pngerror.obj"	"$(INTDIR)\pngerror.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libpng\pngget.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\pngget.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\pngget.obj"	"$(INTDIR)\pngget.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libpng\pngmem.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\pngmem.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\pngmem.obj"	"$(INTDIR)\pngmem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libpng\pngpread.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\pngpread.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\pngpread.obj"	"$(INTDIR)\pngpread.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libpng\pngread.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\pngread.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\pngread.obj"	"$(INTDIR)\pngread.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libpng\pngrio.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\pngrio.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\pngrio.obj"	"$(INTDIR)\pngrio.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libpng\pngrtran.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\pngrtran.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\pngrtran.obj"	"$(INTDIR)\pngrtran.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libpng\pngrutil.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\pngrutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\pngrutil.obj"	"$(INTDIR)\pngrutil.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libpng\pngset.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\pngset.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\pngset.obj"	"$(INTDIR)\pngset.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\libpng\pngtrans.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\pngtrans.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\pngtrans.obj"	"$(INTDIR)\pngtrans.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ProgressDlg.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\ProgressDlg.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\ProgressDlg.obj"	"$(INTDIR)\ProgressDlg.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\RFile.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\RFile.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\RFile.obj"	"$(INTDIR)\RFile.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\StrBuf.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\StrBuf.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\StrBuf.obj"	"$(INTDIR)\StrBuf.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\StylesDlg.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\StylesDlg.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\StylesDlg.obj"	"$(INTDIR)\StylesDlg.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\TCRFile.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\TCRFile.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\TCRFile.obj"	"$(INTDIR)\TCRFile.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\TextFile.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\TextFile.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\TextFile.obj"	"$(INTDIR)\TextFile.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\TextFormatter.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\TextFormatter.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\TextFormatter.obj"	"$(INTDIR)\TextFormatter.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\TextParser.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\TextParser.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\TextParser.obj"	"$(INTDIR)\TextParser.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\TextViewNG.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\TextViewNG.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\TextViewNG.obj"	"$(INTDIR)\TextViewNG.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\TVFrame.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\TVFrame.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\TVFrame.obj"	"$(INTDIR)\TVFrame.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\TView.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\TView.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\TView.obj"	"$(INTDIR)\TView.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\zlib\uncompr.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\uncompr.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\uncompr.obj"	"$(INTDIR)\uncompr.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Unicode.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\Unicode.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\Unicode.obj"	"$(INTDIR)\Unicode.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\WMap.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\WMap.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\WMap.obj"	"$(INTDIR)\WMap.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\XListBox.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\XListBox.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\XListBox.obj"	"$(INTDIR)\XListBox.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\expat\xmlparse.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\xmlparse.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\xmlparse.obj"	"$(INTDIR)\xmlparse.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\XMLParser.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\XMLParser.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\XMLParser.obj"	"$(INTDIR)\XMLParser.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\expat\xmlrole.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\xmlrole.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\xmlrole.obj"	"$(INTDIR)\xmlrole.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\expat\xmltok.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\xmltok.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\xmltok.obj"	"$(INTDIR)\xmltok.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\xscanf.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\xscanf.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\xscanf.obj"	"$(INTDIR)\xscanf.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\ZipFile.cpp

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\ZipFile.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\ZipFile.obj"	"$(INTDIR)\ZipFile.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\zlib\zutil.c

!IF  "$(CFG)" == "tvngx86 - Win32 Unicode Release"


"$(INTDIR)\zutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tvngx86 - Win32 Unicode Debug"


"$(INTDIR)\zutil.obj"	"$(INTDIR)\zutil.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\TextViewNG.rc

"$(INTDIR)\TextViewNG.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

