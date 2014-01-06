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
* $Id: TextFormatter.h,v 1.40.2.2 2003/07/20 12:55:10 mike Exp $
*
*/

#if !defined(AFX_TEXTFORMATTER_H__F50D0A58_8D9D_4FD7_B2B5_747AD7ADE317__INCLUDED_)
#define AFX_TEXTFORMATTER_H__F50D0A58_8D9D_4FD7_B2B5_747AD7ADE317__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Attr.h"
#include "FDC.h"

struct Line {
    Buffer<wchar_t> str;
    Buffer<Attr> attr;
    Buffer<int> dx;
    FilePos pos;
    DWORD  flags;
    int  ispace;
    int  height;
    int  base;
    int  real_len; // not including last '-'
    const wchar_t *href; // only valid for images
    int  yoffset; // vertical offset in image
    int  imageheight;
    enum {
        first = 1,
        last = 2,
        defstyle = 4,
        image = 8
    };
    // not initialized at all
    Line() : flags(first | last | defstyle), real_len(0), ispace(0), height(1), base(0)
        // href and yoffset are only valid when image flag is set
    { }
    // not all fields initialized!!!
    Line(const wchar_t *s, int l, bool addhyp) : str(s, l + (int)addhyp),
        attr(l + (int)addhyp),
        dx(l + (int)addhyp),
        real_len(l)
    { }
    Line(const Line& l) : str(l.str),
        attr(l.attr),
        dx(l.dx),
        pos(l.pos),
        flags(l.flags),
        ispace(l.ispace),
        height(l.height),
        base(l.base),
        real_len(l.real_len),
        href(l.href),
        yoffset(l.yoffset),
        imageheight(l.imageheight) { }
    Line& operator=(const Line& l) {
        str = l.str;
        attr = l.attr;
        dx = l.dx;
        pos = l.pos;
        flags = l.flags;
        ispace = l.ispace;
        height = l.height;
        base = l.base;
        real_len = l.real_len;
        href = l.href;
        yoffset = l.yoffset;
        imageheight = l.imageheight;
        return *this;
    }
    ~Line() { }
    void CheckStyle();
};

typedef CArray<Line, Line&>    LineArray;

class TextFormatter
{
    FilePos     m_top;
    FilePos     m_bot;
    TextFile    *m_tf;
    int       m_width;
    int       m_total_width;
    int       m_margin;
    int       m_height;
    int       m_pages;
    CUIntArray  m_pagelen;
    LineArray   m_lines;
    bool       m_justified;
    bool       m_hyphenate;
    FilePos     m_hlstart;
    int       m_hllen;
    int       m_angle;

    Line       m_junk;

    void       GetTextExtent(CFDC& dc, Paragraph& line, int off, int width, int& nch, int *dx, int& lh, int& lbase);

    int       WrapLine(CFDC& dc, Paragraph& line, FilePos& pos, LineArray& la,
        int top, int maxl);
    int       WrapImage(CFDC& dc, Paragraph& line, FilePos& pos, LineArray& la,
        int top, int maxl);
    void       Highlight();
public:
    TextFormatter(TextFile *tf);
    ~TextFormatter() { }
    const Line& GetLine(int num) { return m_lines[num]; }
    bool       FormatFwd(CFDC& dc) { return FormatFwd(dc, m_bot); }
    bool       FormatFwd(CFDC& dc, FilePos start);
    bool       FormatFwdAdj(CFDC& dc);
    bool       FormatBack(CFDC& dc) { return FormatBack(dc, m_top, FilePos()); }
    bool       FormatBack(CFDC& dc, FilePos start, FilePos prev_top);
    void       FormatPlainText(CFDC& dc,
        int& width, int& height,
        int fontsize,
        const wchar_t *text, int len,
        LineArray& lines);
    bool       EnsureVisible(CFDC& dc, FilePos pos);
    void       AdjustPos(FilePos& p, bool back = false);
    void       SetJustified(bool j = false) { m_justified = j; }
    void       SetHyphenate(bool h = false) { m_hyphenate = h; }
    void       SetSize(int width, int margin, int height, int pages, int angle);
    FilePos     Top() { return m_top; }
    FilePos     Bottom() { return m_bot; }
    FilePos     Eof() { return FilePos(m_tf->Length(m_top.docid), 0, m_top.docid); }
    FilePos     Sof() { return FilePos(0, 0, m_top.docid); }
    int       DocId() { return m_top.docid; }
    bool       AtEof();
    bool       AtTop();
    void       SetTop(FilePos pos);
    void       Reformat(CFDC& dc) { FormatFwd(dc, m_top); }
    bool       SetHighlight(FilePos pos, int len);
    int       Length() { return m_lines.GetSize() - 1; } // number of formatted lines
    int       PageLength(int i) { return i >= 0 && i < m_pagelen.GetSize() ? m_pagelen[i] : 0; }
    int       Distance(const FilePos& a, const FilePos& b);
};

#endif // !defined(AFX_TEXTFORMATTER_H__F50D0A58_8D9D_4FD7_B2B5_747AD7ADE317__INCLUDED_)
