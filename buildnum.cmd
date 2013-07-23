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
rem

pushd	%~dp0

REM increase build number
set /p build= <buildnum.h 2>nul
set build=%build:* =%
set build=%build:* =%
set build=%build:*	=%
set build=%build:*	=%
set /a build=build+1
set cdate=%DATE:~0,10%
set ctime=%TIME%
echo #define BUILD_NUM %build% > buildnum.h
echo #define BUILD_DATE _T("%cdate% %ctime%") >> buildnum.h

popd