@echo off
rem
rem Copyright (c) 2001 Mike Matsnev.  All Rights Reserved.
rem
rem Redistribution and use in source and binary forms, with or without
rem modification, are permitted provided that the following conditions
rem are met:
rem
rem 1. Redistributions of source code must retain the above copyright
rem    notice immediately at the beginning of the file, without modification,
rem    this list of conditions, and the following disclaimer.
rem 2. Redistributions in binary form must reproduce the above copyright
rem    notice, this list of conditions and the following disclaimer in the
rem    documentation and/or other materials provided with the distribution.
rem 3. Absolutely no warranty of function or purpose is made by the author
rem    Mike Matsnev.
rem
rem THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
rem IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
rem OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
rem IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
rem INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
rem NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
rem DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
rem THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
rem (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
rem THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
rem 
rem $Id: build.bat,v 1.88.2.33 2007/03/24 23:17:50 mike Exp $
rem 
rem


set WCEROOT=C:\Program Files\Microsoft eMbedded Tools
set WCE4ROOT=C:\Program Files\Microsoft eMbedded C++ 4.0
set SAVEPATH=%PATH%
set RECURSE=0
set NAME=HaaliReader

REM parse command line! gotta love this ugly shell
REM clean env first
set AFXDLL=
set HPC=
set PSPC=
set PPC=
set HPC2K=
set CABDIR=
set ARM=
set MIPS=
set SH3=
set BBE300=
set FORCEPSPC=
set NOCLEAN=
set MFCSTATIC=
set DX86U=
set XSCALE=
set DOTNET=
set arg=%1
if X%arg%==X set arg=p
:cmdloop
if X%arg%==X goto donecmdline
set opt=%arg:~0,1%
set arg=%arg:~1%
if %opt%==d (
  set CABDIR=..\..\cabs
  set PPC=1
  set ARM=1
  set MIPS=1
  set SH3=1
  set DX86U=1
)
if %opt%==h set HPC2K=1
if %opt%==p set PPC=1
if %opt%==l set PSPC=1
if %opt%==o set HPC=1
if %opt%==c set HPCPRO=1
if %opt%==m set MIPS=1
if %opt%==s set SH3=1
if %opt%==a set ARM=1
if %opt%==u set DX86U=1
if %opt%==b set BBE300=1
if %opt%==N set NOCLEAN=1
if %opt%==P set FORCEPSPC=1
if %opt%==S set MFCSTATIC=1
if %opt%==e set XSCALE=1
if %opt%==. set DOTNET=1
goto cmdloop
:donecmdline
if defined ARM goto okcpu
if defined MIPS goto okcpu
if defined SH3 goto okcpu
if defined DX86U goto okcpu
if defined XSCALE goto okcpu
echo "No cpus specified, aborting"
goto :eof
:okcpu

setlocal
pushd

if not defined CABDIR goto skipcvs
  REM delete all cabs
  rd /q /s cabs
  mkdir cabs


  REM create a fresh source tree
  rd /q /s tmp
  mkdir tmp
  cd tmp
  cvs -d :ext:mike@oin:/export/mike/ncvs co -r HR_2_0_STABLE TextViewNG
  if errorlevel 1 goto fail

  REM increase build number
  set /p build= < TextViewNG\buildnum.h > nul:
  set build=%build:* =%
  set build=%build:* =%
  set build=%build:*	=%
  set build=%build:*	=%
  set /a build=build+1
  set cdate=%DATE%
  set cdate=%cdate:* =%
  set ctime=%TIME%
  set ctime=%ctime:~0,5%
  echo #define BUILD_NUM %build% > TextViewNG\buildnum.h
  echo #define BUILD_DATE _T("%cdate: =% %ctime: =0%") >> TextViewNG\buildnum.h
  REM commit the new build number
  cd TextViewNG
  cvs ci -m "New build (%build%)" buildnum.h
  if errorlevel 1 goto fail
  cvs tag BUILD_2_0_%build%
  if errorlevel 1 goto fail
  cd ..

  REM create the source archive
  call :find TextViewNG TextViewNG > files.lst
  "C:\Program Files\7-Zip\7z.exe" a -tzip -mx ..\cabs\TextViewNG.zip @files.lst
  if errorlevel 1 goto fail

  cd TextViewNG
:skipcvs

REM build executables for all platforms
if defined DX86U (
  call :dx86u
  if errorlevel 1 goto fail
)
if defined DOTNET (
  call :dotnet
  if errorlevel 1 goto fail
)
if defined HPC2K (
  call :hpc2k
  if errorlevel 1 goto fail
)
if defined PSPC (
  call :pspc
 if errorlevel 1 goto fail
)
if defined PPC (
  call :pocketpc
  if errorlevel 1 goto fail
)
if defined HPC (
  call :hpc
  if errorlevel 1 goto fail
)
if defined HPCPRO (
  call :hpcpro
  if errorlevel 1 goto fail
)
if defined BBE300 (
  call :be300
  if errorlevel 1 goto fail
)
if defined XSCALE (
  call :xscale
  if errorlevel 1 goto fail
)

call :dict
if errorlevel 1 goto fail

REM build cabs
if defined CABDIR (
  pushd
  if defined BBE300 (
    rem copy be300 files
    mkdir %CABDIR%\be300
    copy /b be300\HaaliReader.BE300.txt %CABDIR%\be300
    copy /b be300\Install.inf %CABDIR%\be300
    copy /b be300\native.ina %CABDIR%\be300
    copy /b be300\UnHaaliReader.inf %CABDIR%\be300
    copy /b be300\setup.ini %CABDIR%
  )
  rem copy setup files
  copy /b HaaliReader.inf %CABDIR%
  copy /b HaaliReader.ini %CABDIR%
  copy /b HaaliReader.nsi %CABDIR%
  copy /b HaaliReaderBE300.nsi %CABDIR%
  copy /b HaaliReaderDesktop.nsi %CABDIR%
  copy /b LICENSE.txt %CABDIR%
  if defined BBE300 (
    copy /b "C:\Program Files\Windows CE Tools\casiosdk\BE300SDK_1012\Installer\English PC Tools\Setup.exe" %CABDIR%
  )
  cd %CABDIR%
  REM XXX sdk root should be here
  "C:\Program Files\Windows CE Tools\wce300\MS Pocket PC\support\ActiveSync\windows ce application installation\cabwiz\Cabwiz.exe" HaaliReader.inf /cpu MIPS.PPC ARM.PPC SH3.PPC
  del /q/f *.DAT
  del HaaliReader.inf
  del /q/f dict.*
  c:\progra~1\nsis\makensis.exe /V2 HaaliReader.nsi
  c:\progra~1\nsis\makensis.exe /V2 HaaliReaderDesktop.nsi
  if defined BBE300 (
    copy /b HaaliReader.BE300.exe be300
    "C:\Program Files\Windows CE Tools\casiosdk\BE300SDK_1012\Installer\English PC Tools\MkArch.exe" be300 HaaliReader.cbea -p 0000 -o c000 -c 0000
    c:\progra~1\nsis\makensis.exe /V2 HaaliReaderBE300.nsi
  )
  del /q/f *.CAB
  del /q/f HaaliReader.ini
  del /q/f HaaliReader*.nsi
  del /q/f LICENSE.txt
  if defined BBE300 (
    rd /q/s be300
    del /q/f setup.exe
    del /q/f setup.ini
  )
  for %%f in (HaaliReader.*.exe) do "C:\Program Files\7-Zip\7z.exe" a -tzip -mx "%%~dpnf.zip" "%%f"
  del /q/f HaaliReader.*.exe
  popd
)
:end
popd
endlocal
if defined CABDIR (
  rd /q /s tmp
  cvs update buildnum.h
)
echo ++++++++++++++++++++++++++++++++++++++++++++++++
echo +                BUILD COMPLETE                +
echo ++++++++++++++++++++++++++++++++++++++++++++++++
goto :eof

:fail
echo ************************************************
echo * BUILD FAILED while building %LASTBUILD%
echo ************************************************
popd
endlocal
if defined CABDIR (
  rd /q /s tmp
  rd /q /s cabs
)
goto :eof

:be300
REM *******************************************
REM CASIO BE300 (casio really supports 3.0,
REM but their mfc is a bit weird)
REM *******************************************
set SDKROOT=C:\Program Files\Windows CE Tools
set OSVERSION=WCE300
set COMPVERSION=WCE300
set PLATFORM=CASIO_BE300
set CESubsystem=windowsce,3.00
set CEVersion=300
set CE_PLATFORM=WIN32_PLATFORM_PSPC
set AFXDLL=
if not defined MFCSTATIC set AFXDLL=LL
set BE300=1
set EXTRACFLAGS=

if defined MIPS (
  set LASTBUILD=Building for BE300
  Title Building for BE300
  call :mips
  if errorlevel 1 goto :eof
)

REM copy exe files
if defined CABDIR (
  move /y MIPSRel\TextViewNG.exe %CABDIR%\%NAME%.BE300.exe
  if errorlevel 1 goto :eof
)
goto :eof

:dx86u
REM *******************************************
REM Normal desktop win32 build (unicode)
REM *******************************************

set LASTBUILD=Building for DX86U
Title Building for DX86U
"C:\Program Files\Microsoft Visual Studio 8\Common7\IDE\VCExpress.exe" tvngx86.sln /build "Unicode Release"
if errorlevel 1 goto :eof

REM copy exe files
if defined CABDIR (
  move /y DX86UnicodeRel\tvngx86.exe %CABDIR%\%NAME%.x86.U.exe
  if errorlevel 1 goto :eof
)
goto :eof

:hpc
REM *******************************************
REM HPC (WinCE 2.0)
REM *******************************************
set SDKROOT=C:\Program Files\Windows CE Tools
set OSVERSION=WCE200
set COMPVERSION=WCE200
set PLATFORM=hpc
set CESubsystem=windowsce,2.00
set CEVersion=200
set CE_PLATFORM=WIN32_PLATFORM_HPC
set AFXDLL=
set BE300=0
set EXTRACFLAGS=

if defined MIPS (
  set LASTBUILD=Building for HPC/MIPS
  Title Building for HPC/MIPS
  call :mips
  if errorlevel 1 goto :eof
)
if defined SH3 (
 set LASTBUILD=Building for HPC/SH3
  Title Building for HPC/SH3
 call :sh3
 if errorlevel 1 goto :eof
)

REM copy exe files
if defined CABDIR (
  move /y SH3Rel\TextViewNG.exe %CABDIR%\%NAME%.HPCold.SH3.exe
  if errorlevel 1 goto :eof
  move /y MIPSRel\TextViewNG.exe %CABDIR%\%NAME%.HPCold.MIPS.exe
  if errorlevel 1 goto :eof
)
goto :eof

:hpcpro
REM *******************************************
REM HPC Pro (WinCE 2.11)
REM *******************************************
set SDKROOT=C:\Program Files\Windows CE Tools
set OSVERSION=WCE211
set COMPVERSION=WCE211
set PLATFORM=MS HPC pro
set CESubsystem=windowsce,2.11
set CEVersion=211
set CE_PLATFORM=WIN32_PLATFORM_HPCPRO
set AFXDLL=
if not defined MFCSTATIC set AFXDLL=LL
set BE300=0
set EXTRACFLAGS=

rem if defined MIPS (
rem   set LASTBUILD=Building for HPC Pro/MIPS
rem   Title Building for HPC Pro/MIPS
rem   call :mips
rem   if errorlevel 1 goto :eof
rem )
if defined SH3 (
  set LASTBUILD=Building for HPC Pro/SH3
  Title Building for HPC Pro/SH3
  call :sh3
  if errorlevel 1 goto :eof
)
if defined ARM (
 set LASTBUILD=Building for HPC Pro/ARM
  Title Building for HPC Pro/ARM
 call :arm
 if errorlevel 1 goto :eof
)

REM copy exe files
if defined CABDIR (
  move /y SH3Rel\TextViewNG.exe %CABDIR%\%NAME%.HPCPro.SH3.exe
  if errorlevel 1 goto :eof
  rem move /y MIPSRel\TextViewNG.exe %CABDIR%\%NAME%.HPCPro.MIPS.exe
  rem if errorlevel 1 goto :eof
  move /y ARMRel\TextViewNG.exe %CABDIR%\%NAME%.HPCPro.ARM.exe
  if errorlevel 1 goto :eof
)
goto :eof

:hpc2k
REM *******************************************
REM HPC 2000
REM *******************************************
set SDKROOT=C:\Program Files\Windows CE Tools
set OSVERSION=WCE300
set COMPVERSION=WCE300
set PLATFORM=hpc2000
set CESubsystem=windowsce,3.00
set CEVersion=300
set CE_PLATFORM=WIN32_PLATFORM_HPC2000
set AFXDLL=
if not defined MFCSTATIC set AFXDLL=LL
set BE300=0
set EXTRACFLAGS=

if defined ARM (
  set LASTBUILD=Building for HPC 2000/ARM
  Title Building for HPC 2000/ARM
  call :arm
  if errorlevel 1 goto :eof
)
if defined MIPS (
  set LASTBUILD=Building for HPC 2000/MIPS
  Title Building for HPC 2000/MIPS
  call :mips
  if errorlevel 1 goto :eof
)

REM copy exe files
if defined CABDIR (
  move /y ARMRel\TextViewNG.exe %CABDIR%\%NAME%.HPC.ARM.exe
  if errorlevel 1 goto :eof
  move /y MIPSRel\TextViewNG.exe %CABDIR%\%NAME%.HPC.MIPS.exe
  if errorlevel 1 goto :eof
)
goto :eof

:pspc
REM *******************************************
REM PsPC
REM *******************************************
set OSVERSION=WCE211
set COMPVERSION=WCE211
set PLATFORM=ms palm size pc
set SDKROOT=C:\Program Files\Windows CE Tools
set CESubsystem=windowsce,2.11
set CEVersion=211
set CE_PLATFORM=WIN32_PLATFORM_PSPC
set AFXDLL=
if not defined MFCSTATIC set AFXDLL=LL
set BE300=0
set EXTRACFLAGS=

if defined SH3 (
  set LASTBUILD=Building for PsPC/SH3
  Title Building for PsPC/SH3
  call :sh3
  if errorlevel 1 goto :eof
)
if defined MIPS (
  set LASTBUILD=Building for PsPC/MIPS
  Title Building for PsPC/MIPS
  call :mips
  if errorlevel 1 goto :eof
)

REM copy exe files
if defined CABDIR (
  move /y MIPSRel\TextViewNG.exe %CABDIR%\%NAME%.PsPC.MIPS.exe
  if errorlevel 1 goto :eof
  move /y SH3Rel\TextViewNG.exe %CABDIR%\%NAME%.PsPC.SH3.exe
  if errorlevel 1 goto :eof
)
goto :eof

:pocketpc
REM *******************************************
REM Pocket PC
REM *******************************************
set OSVERSION=WCE300
set COMPVERSION=WCE300
set PLATFORM=ms pocket pc
set SDKROOT=C:\Program Files\Windows CE Tools
set CESubsystem=windowsce,3.0
set CEVersion=300
set CE_PLATFORM=WIN32_PLATFORM_PSPC
set AFXDLL=
if not defined MFCSTATIC set AFXDLL=LL
set BE300=0
set EXTRACFLAGS=

if defined ARM (
  set LASTBUILD=Building for PocketPC/ARM
  Title Building for PocketPC/ARM
  call :arm
  if errorlevel 1 goto :eof
)
if defined MIPS (
  set LASTBUILD=Building for PocketPC/MIPS
  Title Building for PocketPC/MIPS
  call :mips
  if errorlevel 1 goto :eof
)
set COMPVERSION=WCE300
if defined SH3 (
  set LASTBUILD=Building for PocketPC/SH3
  Title Building for PocketPC/SH3
  call :sh3
  if errorlevel 1 goto :eof
)

REM copy exe files
if defined CABDIR (
  move /y ARMRel\TextViewNG.exe %CABDIR%\%NAME%.PPC.ARM.exe
  if errorlevel 1 goto :eof
  move /y MIPSRel\TextViewNG.exe %CABDIR%\%NAME%.PPC.MIPS.exe
  if errorlevel 1 goto :eof
  move /y SH3Rel\TextViewNG.exe %CABDIR%\%NAME%.PPC.SH3.exe
  if errorlevel 1 goto :eof
)
goto :eof

:xscale
REM *******************************************
REM Pocket PC (xscale)
REM *******************************************
set OSVERSION=WCE300
set COMPVERSION=WCE300a
set PLATFORM=ms pocket pc
set SDKROOT=C:\Program Files\Windows CE Tools
set CESubsystem=windowsce,3.0
set CEVersion=300
set CE_PLATFORM=WIN32_PLATFORM_PSPC
set AFXDLL=
if not defined MFCSTATIC set AFXDLL=LL
set BE300=0
set EXTRACFLAGS=/QRxscale /QRxscalesched

set LASTBUILD=Building for PocketPC/ARM
Title Building for PocketPC/ARM
call :arm
if errorlevel 1 goto :eof

REM copy exe files
if defined CABDIR (
  move /y ARMRel\TextViewNG.exe %CABDIR%\%NAME%.PPC.XScale.exe
  if errorlevel 1 goto :eof
)
goto :eof

:dotnet
REM *******************************************
REM Build for CE .net (ARMV4,MIPS)
REM *******************************************
set SDK4ROOT=C:\Program Files\Windows CE Tools
set OSVERSION=WCE400
set COMPVERSION=WCE400
set PLATFORM=STANDARDSDK
set CESubsystem=windowsce,4.00
set CEVersion=400
set CE_PLATFORM=WIN32_PLATFORM_PSPC
set AFXDLL=
if not defined MFCSTATIC set AFXDLL=LL
set BE300=0
set EXTRACFLAGS=

if defined ARM (
  set LASTBUILD=Building for .NET/ARMV4
  Title Building for .NET/ARMV4
  call :armv4
  if errorlevel 1 goto :eof
)
if defined MIPS (
  set LASTBUILD=Building for .NET/MIPSII
  Title Building for .NET/MIPSII
  call :mipsii
  if errorlevel 1 goto :eof
)

REM copy exe files
if defined CABDIR (
  move /y ARMV4Rel\tvngnet.exe %CABDIR%\%NAME%.NET.ARMV4.exe
  move /y MIPSIIRel\tvngnet.exe %CABDIR%\%NAME%.NET.MIPSII.exe
  if errorlevel 1 goto :eof
)
goto :eof

:dict
REM *******************************************
REM Dict (all platforms)
REM *******************************************
set OSVERSION=WCE211
set COMPVERSION=WCE211
set PLATFORM=MS HPC Pro
set SDKROOT=C:\Program Files\Windows CE Tools
set CESubsystem=windowsce,2.11
set CEVersion=211
set CE_PLATFORM=WIN32_PLATFORM_HPC
set AFXDLL=
set BE300=0
set EXTRACFLAGS=
cmd /c echo 1 > nul:

if defined ARM (
  set LASTBUILD=Building dict for ARM
  Title %LASTBUILD%
  call :dictarm
  if errorlevel 1 goto :eof
)
if defined MIPS (
  set LASTBUILD=Building dict for MIPS
  Title %LASTBUILD%
  call :dictmips
  if errorlevel 1 goto :eof
)
if defined SH3 (
  set LASTBUILD=Building dict for SH3
  Title %LASTBUILD%
  call :dictsh3
  if errorlevel 1 goto :eof
)

REM copy exe files
if defined CABDIR (
  move /y dict_run\ARMRel\dict_run.exe %CABDIR%\dict.ARM.exe
  if errorlevel 1 goto :eof
  move /y dict_run\MIPSRel\dict_run.exe %CABDIR%\dict.MIPS.exe
  if errorlevel 1 goto :eof
  move /y dict_run\SH3Rel\dict_run.exe %CABDIR%\dict.SH3.exe
  if errorlevel 1 goto :eof
)
goto :eof

REM *******************************************
REM Build for ARMV4 cpu (.NET)
REM *******************************************
:armv4
set PATH=%WCE4ROOT%\COMMON\EVC\bin;%WCE4ROOT%\EVC\%COMPVERSION%\bin;%SAVEPATH%
set INCLUDE=%SDK4ROOT%\%OSVERSION%\%PLATFORM%\include\armv4;%SDK4ROOT%\%OSVERSION%\%PLATFORM%\MFC\include;%SDK4ROOT%\%OSVERSION%\%PLATFORM%\ATL\include
set LIB=%SDK4ROOT%\%OSVERSION%\%PLATFORM%\lib\armv4;%SDK4ROOT%\%OSVERSION%\%PLATFORM%\MFC\lib\armv4;%SDK4ROOT%\%OSVERSION%\%PLATFORM%\ATL\lib\armv4

set CC=clarm.exe

set TARGETCPU=ARM

set LASTBUILD=Building for .NET/ARMV4
Title %LASTBUILD%
set CFG=tvngnet - Win32 (WCE ARMV4) Release
if not defined NOCLEAN nmake /f tvngnet.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion% clean
nmake /f tvngnet.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion%
goto :eof

REM *******************************************
REM Build for MIPS cpu (.NET)
REM *******************************************
:mipsii
set PATH=%WCE4ROOT%\COMMON\EVC\bin;%WCE4ROOT%\EVC\%COMPVERSION%\bin;%SAVEPATH%
set INCLUDE=%SDK4ROOT%\%OSVERSION%\%PLATFORM%\include\mipsii;%SDK4ROOT%\%OSVERSION%\%PLATFORM%\MFC\include;%SDK4ROOT%\%OSVERSION%\%PLATFORM%\ATL\include
set LIB=%SDK4ROOT%\%OSVERSION%\%PLATFORM%\lib\mipsii;%SDK4ROOT%\%OSVERSION%\%PLATFORM%\MFC\lib\mipsii;%SDK4ROOT%\%OSVERSION%\%PLATFORM%\ATL\lib\mipsii

set CC=clarm.exe

set TARGETCPU=ARM

set LASTBUILD=Building for .NET/MIPSII
Title %LASTBUILD%
set CFG=tvngnet - Win32 (WCE MIPSII) Release
if not defined NOCLEAN nmake /f tvngnet.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion% clean
nmake /f tvngnet.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion%
goto :eof

REM *******************************************
REM Build for ARM cpu
REM *******************************************
:arm
set PATH=%WCEROOT%\COMMON\EVC\bin;%WCEROOT%\EVC\%COMPVERSION%\bin;%SAVEPATH%
set INCLUDE=%SDKROOT%\%OSVERSION%\%PLATFORM%\include;%SDKROOT%\%OSVERSION%\%PLATFORM%\MFC\include;%SDKROOT%\%OSVERSION%\%PLATFORM%\ATL\include;
set LIB=%SDKROOT%\%OSVERSION%\%PLATFORM%\lib\arm;%SDKROOT%\%OSVERSION%\%PLATFORM%\MFC\lib\arm;%SDKROOT%\%OSVERSION%\%PLATFORM%\ATL\lib\arm;

set CC=clarm.exe

set TARGETCPU=ARM

set CFG=zlib - Win32 (WCE ARM) Release
cd zlib
if not defined NOCLEAN nmake /f zlib.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion% clean
nmake /f zlib.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion%
cd ..
set CFG=expat - Win32 (WCE ARM) Release
cd expat
if not defined NOCLEAN nmake /f expat.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion% clean
nmake /f expat.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion%
cd ..
set CFG=libjpeg - Win32 (WCE ARM) Release
cd libjpeg
if not defined NOCLEAN nmake /f libjpeg.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion% clean
nmake /f libjpeg.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion%
cd ..
set CFG=libpng - Win32 (WCE ARM) Release
cd libpng
if not defined NOCLEAN nmake /f libpng.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion% clean
nmake /f libpng.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion%
cd ..
set CFG=TextViewNG - Win32 (WCE ARM) Release
if not defined NOCLEAN nmake /f TextViewNG.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion% clean
nmake /f TextViewNG.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion% FORCEPSPC=%FORCEPSPC%
goto :eof

REM *******************************************
REM Build for SH3 cpu
REM *******************************************
:sh3
set PATH=%WCEROOT%\COMMON\EVC\bin;%WCEROOT%\EVC\%COMPVERSION%\bin;%SAVEPATH%
set INCLUDE=%SDKROOT%\%OSVERSION%\%PLATFORM%\include;%SDKROOT%\%OSVERSION%\%PLATFORM%\MFC\include;%SDKROOT%\%OSVERSION%\%PLATFORM%\ATL\include;
set LIB=%SDKROOT%\%OSVERSION%\%PLATFORM%\lib\SH3;%SDKROOT%\%OSVERSION%\%PLATFORM%\MFC\lib\SH3;%SDKROOT%\%OSVERSION%\%PLATFORM%\ATL\lib\SH3;

set CC=shcl.exe

set TARGETCPU=SH3

set CFG=zlib - Win32 (WCE SH3) Release
cd zlib
if not defined NOCLEAN nmake /f zlib.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion% clean
nmake /f zlib.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion%
cd ..
set CFG=expat - Win32 (WCE SH3) Release
cd expat
if not defined NOCLEAN nmake /f expat.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion% clean
nmake /f expat.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion%
cd ..
set CFG=libjpeg - Win32 (WCE SH3) Release
cd libjpeg
if not defined NOCLEAN nmake /f libjpeg.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion% clean
nmake /f libjpeg.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion%
cd ..
set CFG=libpng - Win32 (WCE SH3) Release
cd libpng
if not defined NOCLEAN nmake /f libpng.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion% clean
nmake /f libpng.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion%
cd ..
set CFG=TextViewNG - Win32 (WCE SH3) Release
if not defined NOCLEAN nmake /f TextViewNG.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion% clean
nmake /f TextViewNG.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion% FORCEPSPC=%FORCEPSPC%
goto :eof

REM *******************************************
REM Build for MIPS cpu
REM *******************************************
:mips
set PATH=%WCEROOT%\COMMON\EVC\bin;%WCEROOT%\EVC\%COMPVERSION%\bin;%SAVEPATH%
set INCLUDE=%SDKROOT%\%OSVERSION%\%PLATFORM%\include;%SDKROOT%\%OSVERSION%\%PLATFORM%\MFC\include;%SDKROOT%\%OSVERSION%\%PLATFORM%\ATL\include;
set LIB=%SDKROOT%\%OSVERSION%\%PLATFORM%\lib\MIPS;%SDKROOT%\%OSVERSION%\%PLATFORM%\MFC\lib\MIPS;%SDKROOT%\%OSVERSION%\%PLATFORM%\ATL\lib\MIPS;

set CC=clmips.exe

set TARGETCPU=R4100

set CFG=zlib - Win32 (WCE MIPS) Release
cd zlib
if not defined NOCLEAN nmake /f zlib.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion% clean
nmake /f zlib.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion%
cd ..
set CFG=expat - Win32 (WCE MIPS) Release
cd expat
if not defined NOCLEAN nmake /f expat.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion% clean
nmake /f expat.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion%
cd ..
set CFG=libjpeg - Win32 (WCE MIPS) Release
cd libjpeg
if not defined NOCLEAN nmake /f libjpeg.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion% clean
nmake /f libjpeg.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion%
cd ..
set CFG=libpng - Win32 (WCE MIPS) Release
cd libpng
if not defined NOCLEAN nmake /f libpng.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion% clean
nmake /f libpng.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion%
cd ..
set CFG=TextViewNG - Win32 (WCE MIPS) Release
if not defined NOCLEAN nmake /f TextViewNG.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion% BE300=%BE300% clean
nmake /f TextViewNG.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion% BE300=%BE300% FORCEPSPC=%FORCEPSPC%
goto :eof

REM *******************************************
REM Build for ARM cpu (dict)
REM *******************************************
:dictarm
set PATH=%WCEROOT%\COMMON\EVC\bin;%WCEROOT%\EVC\%COMPVERSION%\bin;%SAVEPATH%
set INCLUDE=%SDKROOT%\%OSVERSION%\%PLATFORM%\include;%SDKROOT%\%OSVERSION%\%PLATFORM%\MFC\include;%SDKROOT%\%OSVERSION%\%PLATFORM%\ATL\include;
set LIB=%SDKROOT%\%OSVERSION%\%PLATFORM%\lib\arm;%SDKROOT%\%OSVERSION%\%PLATFORM%\MFC\lib\arm;%SDKROOT%\%OSVERSION%\%PLATFORM%\ATL\lib\arm;

set CC=clarm.exe

set TARGETCPU=ARM

set CFG=dict_run - Win32 (WCE ARM) Release
cd dict_run
if not defined NOCLEAN nmake /f dict_run.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion% clean
nmake /f dict_run.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion%
cd ..
goto :eof

REM *******************************************
REM Build for MIPS cpu (dict)
REM *******************************************
:dictmips
set PATH=%WCEROOT%\COMMON\EVC\bin;%WCEROOT%\EVC\%COMPVERSION%\bin;%SAVEPATH%
set INCLUDE=%SDKROOT%\%OSVERSION%\%PLATFORM%\include;%SDKROOT%\%OSVERSION%\%PLATFORM%\MFC\include;%SDKROOT%\%OSVERSION%\%PLATFORM%\ATL\include;
set LIB=%SDKROOT%\%OSVERSION%\%PLATFORM%\lib\mips;%SDKROOT%\%OSVERSION%\%PLATFORM%\MFC\lib\mips;%SDKROOT%\%OSVERSION%\%PLATFORM%\ATL\lib\mips;

set CC=clmips.exe

set TARGETCPU=R4100

set CFG=dict_run - Win32 (WCE MIPS) Release
cd dict_run
if not defined NOCLEAN nmake /f dict_run.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion% clean
nmake /f dict_run.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion%
cd ..
goto :eof

REM *******************************************
REM Build for SH3 cpu (dict)
REM *******************************************
:dictsh3
set PATH=%WCEROOT%\COMMON\EVC\bin;%WCEROOT%\EVC\%COMPVERSION%\bin;%SAVEPATH%
set INCLUDE=%SDKROOT%\%OSVERSION%\%PLATFORM%\include;%SDKROOT%\%OSVERSION%\%PLATFORM%\MFC\include;%SDKROOT%\%OSVERSION%\%PLATFORM%\ATL\include;
set LIB=%SDKROOT%\%OSVERSION%\%PLATFORM%\lib\sh3;%SDKROOT%\%OSVERSION%\%PLATFORM%\MFC\lib\sh3;%SDKROOT%\%OSVERSION%\%PLATFORM%\ATL\lib\sh3;

set CC=shcl.exe

set TARGETCPU=SH3

set CFG=dict_run - Win32 (WCE SH3) Release
cd dict_run
if not defined NOCLEAN nmake /f dict_run.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion% clean
nmake /f dict_run.vcn CESubsystem=%CESubsystem% CEVersion=%CEVersion%
cd ..
goto :eof

REM *******************************************
REM construct a list of files
REM *******************************************
:find
if %1==extra goto :eof
if %1==CVS goto :eof
if %1==. goto :eof
if %1==.. goto :eof
pushd %1
for /d %%d in (*) do call :find %%d %2/%%d
for %%f in (*) do echo %2/%%f
popd
goto :eof