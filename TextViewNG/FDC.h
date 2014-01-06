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
* $Id: FDC.h,v 1.14.2.1 2003/04/12 22:52:32 mike Exp $
*
*/

#if !defined(AFX_FDC_H__28915DD2_B307_42CF_9BF9_F27108F0EDC3__INCLUDED_)
#define AFX_FDC_H__28915DD2_B307_42CF_9BF9_F27108F0EDC3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFDC
{
    HDC  m_hDC;
    void  *m_hFont;
    HGDIOBJ m_hOrigFont, m_hOrigPen;
    HWND  m_hWnd;
    PAINTSTRUCT *m_ps;
    COLORREF m_fg, m_bg, m_pfg;
    int  m_lpx;
public:
    enum {
        FORCENORMALWEIGHT = 0x10000,
        FORCETAHOMA = 0x20000,
    };
    CFDC(HWND hWnd, PAINTSTRUCT *ps = NULL);
    ~CFDC();

    void   SelectFont(int size, unsigned flags, bool zesc = false);
    void   SelectFontAbs(int size, unsigned flags, bool zesc = false);
    HDC   DC() { return m_hDC; }
    void   GetFontSize(int& height, int& ascent);
    void   SetBkColor(COLORREF c) { if (c != m_bg) ::SetBkColor(m_hDC, m_bg = c); }
    void   SetTextColor(COLORREF c) { if (c != m_fg) ::SetTextColor(m_hDC, m_fg = c); }
    void   SetColor(COLORREF c);
    int   GetLPX();
    int   GetHypWidth();
    void   GetTextExtent(const wchar_t *str, int len, int width, int& nch, int *dx, SIZE& sz);

    static void SetDefaultFont(const TCHAR *face, int basesize, bool bold,
        int cleartype, int angle);
    static void SetCacheSize(int cs);
};

#endif // !defined(AFX_FDC_H__28915DD2_B307_42CF_9BF9_F27108F0EDC3__INCLUDED_)
