/*
* Copyright (c) 2001,2002,2003 Mike Matsnev.  All Rights Reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
*    notice immediately at the beginning of the file, without modification,
*    this list of conditions, and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 3. Absolutely no warranty of function or purpose is made by the author
*    Mike Matsnev.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* $Id: Unicode.h,v 1.8.2.2 2003/12/17 12:19:58 mike Exp $
*
*/

#ifndef	UNICODE_H
#define	UNICODE_H

#ifndef	CP_UTF8
#define CP_UTF8 65001
#endif

#ifndef	CP_UTF16
#define CP_UTF16 1200
#endif

#ifndef CP_1252
#define	CP_1252 1252
#endif

class Unicode {
public:
    // conversion to unicode
    static int				WCLength(int codepage, const char *mbstr, int mblen);
    static void				ToWC(int codepage, const char *mbstr, int mblen, wchar_t *wcstr, int wclen);

    // conversion to system default codepage
    static int				MBLength(const wchar_t *wcstr, int wclen);
    static void				ToMB(const wchar_t *wcstr, int wclen, char *mbstr, int mblen);

    // utf8
    static Buffer<char>		ToUtf8(const CString& cs);

    // convenience functions
    static Buffer<wchar_t>	ToWCbuf(int codepage, const char *mbstr, int mblen);
    static Buffer<char>		ToMBbuf(const wchar_t *wcstr, int wclen);

    // MFC interface
    static CString			ToCS(const wchar_t *wcstr, int wclen);
    static CString			ToCS(const Buffer<wchar_t>& wcbuf) {
        return ToCS(wcbuf, wcbuf.size());
    }
    static CString			ToCS(int codepage, const char *mbstr, int mblen) {
        return ToCS(ToWCbuf(codepage, mbstr, mblen));
    }

    static Buffer<wchar_t>	ToWCbuf(const CString& str);
    static Buffer<wchar_t>	ToWCbufZ(const CString& str);

    // codepages support
    static int				GetNumCodePages();
    static const TCHAR		*GetCodePageName(int num);
    static const wchar_t	*GetCodePageNameW(int num);
    static int				GetCodePage(int num);
    static int				GetIntCodePage(UINT mscp);
    static UINT				GetMSCodePage(int cp);
    static int				FindCodePage(const TCHAR *name);
    static int				DefaultCodePage();
    static const wchar_t	*GetTable(int cp); // only for builtin encodings

    // codepage detection
    static int				DetectCodePage(const char *mbstr, int mblen);

    // case conversion
    static Buffer<wchar_t>	Lower(const Buffer<wchar_t>& str);

    // sortkey generation
    static Buffer<char>		SortKey(LCID lcid, const wchar_t *str, int len);
    static Buffer<char>		SortKey(LCID lcid, const wchar_t *str) {
        return SortKey(lcid, str, wcslen(str));
    }
};

#endif
