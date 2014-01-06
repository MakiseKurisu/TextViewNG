/*
* Copyright (c) 2001,2002,2003 Mike Matsnev. All Rights Reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
* notice immediately at the beginning of the file, without modification,
* this list of conditions, and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* 3. Absolutely no warranty of function or purpose is made by the author
* Mike Matsnev.
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
* $Id: TextFormatter.cpp,v 1.66.2.6 2004/01/08 11:06:06 mike Exp $
*
*/

#define _WIN32_WINNT _WIN32_WINNT_MAXVER

#include <afxwin.h>
#include <afxtempl.h>

#include "ptr.h"
#include "FDC.h"
#include "FilePos.h"
#include "TextFile.h"
#include "TextFormatter.h"
#include "TextViewNG.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#ifndef MAXINT
#define MAXINT 0x7fffffff
#endif

#define FBIG_ADD 4
#define FSMALL_ADD -1

TextFormatter::TextFormatter(TextFile *tf) :
m_tf(tf),
m_width(1),
m_height(1),
m_pages(1),
m_justified(false),
m_hyphenate(false),
m_hllen(0),
m_angle(0)
{
 m_junk.flags = Line::first | Line::last;
}

void TextFormatter::GetTextExtent(CFDC& dc, Paragraph& line, int off,
 int width, int& nch, int *dx, int& lh, int& lb)
{
 const wchar_t *sp = line.str;
 Attr *att = line.cflags + off;
 int len = nch - off;
 int xoff = 0;
 sp += off;
 nch = 0;
 while (len > 0 && width > 0) {
 int i;
 int n = 0;
 SIZE sz = { 0, 0 };
 Attr curatt = att[0];
 if (curatt.img) {
 // inline image
 int idx = *sp;
 Image img;
 if (idx < line.links.size() &&
 m_tf->GetImage(line.links[idx].target, dc.DC(),
 width, m_height, m_angle, img))
 {
 int dh = img.height - lb;
 if (dh > 0) {
 lh += dh;
 lb += dh;
 }
 sz.cx = img.width;
 *dx = xoff + img.width;
 }
 n = 1;
 ++nch;
 if (len == 1)
 break;
 }
 else {
 for (i = 1; i < len && att[i].fontflags() == curatt.fontflags(); ++i);
 dc.SelectFont(curatt.fsize, curatt.fontattr(), true);
 int fh, fa;
 dc.GetFontSize(fh, fa);
 if (lh < fh) {
 lh = fh;
 lb = fa;
 }
 dc.GetTextExtent(sp, i, width, n, dx, sz);
 if (n == 0)
 break;
 for (int j = 0; j < n; ++j)
 dx[j] += xoff;
 nch += n;
 if (i == len)
 break;
 }
 xoff += sz.cx;
 width -= sz.cx;
 dx += n;
 sp += n;
 att += n;
 len -= n;
 }
}

void CopyAttr(Attr *dest, const Attr *src, DWORD len) {
 while (len--)
 (*dest++ = *src++).hyphen = false;
}

static void AdjustIndent(int& width, int& indent, int li, int ri, int fi, int lpx) {
 int pli = lpx*(li + fi) / 25;
 int pri = lpx*ri / 25;
 if (width - pli < 50)
 pli = width - 50;
 width -= pli;
 indent += pli;
 width -= pri;
 if (width < 50)
 width = 50;
}

// split image into strips
int TextFormatter::WrapImage(CFDC& dc, Paragraph& pp,
 FilePos& pos, LineArray& la,
 int top, int maxh)
{
 Image img;
#if 0
 int curwidth = m_width;
 int ispace = 0;
 AdjustIndent(curwidth, ispace, pp.lindent, pp.rindent, 0, dc.GetLPX());
#else
 int curwidth = m_total_width;
 int ispace = -m_margin;
#endif
 if (pp.links.size() <= 0 ||
 !m_tf->GetImage(pp.links[0].target, dc.DC(), curwidth, m_height, m_angle, img))
 {
 // image fetch failed, just skip the paragraph
 pos.off += pp.len;
 return 0;
 }
#if 0
 if (top && img.height > maxh)
 return -1;
#endif
 // calc strips, min strip height is 16
 int striph = (img.height + pp.str.size() - 1) / pp.str.size();
 if (striph < 16)
 striph = 16;
 if (striph > img.height)
 striph = img.height;
 // number of visible strips
 int vstrips = (img.height + striph - 1) / striph;
 int topstrip = vstrips;
 if (topstrip > pp.len)
 topstrip = pp.len;
 // all lines are the same
 Line line(L" ", 1, false);
 line.attr[0].wa = 0;
 line.dx[0] = 0;
 line.ispace = ispace + (curwidth - img.width) / 2;
 line.href = pp.links[0].target;
 line.pos = pos;
 line.flags = Line::image;
 line.height = striph;
 line.base = curwidth;
 line.imageheight = m_height;
 // take care of the strip offset
 int stripnum = pos.off;
 int yoffset = stripnum*striph;
 // add visible strips as lines
 int toth = 0;
 // add visible strips
 while (stripnum < topstrip) {
 line.yoffset = yoffset;
 line.pos = pos;
 if (stripnum == vstrips - 1) {
 // last line
 // assign all unsused spaces here
 int spcount = pp.len - pos.off;
 line.attr = Buffer<Attr>(spcount);
 line.str = Buffer<wchar_t>(spcount);
 line.dx = Buffer<int>(spcount);
 for (int i = 0; i < spcount; ++i) {
 line.attr[i].wa = 0;
 line.str[i] = L' ';
 line.dx[i] = 0;
 }
 line.real_len = spcount;
 // adjust line height
 line.height = img.height - striph*stripnum;
 }
 if (toth + line.height > maxh)
 break;
 la.Add(line);
 pos.off += line.real_len;
 ++stripnum;
 yoffset += striph;
 toth += line.height;
 }
 // ok, return processed height
 // if we didn't process anything, return a failure
 return toth ? toth : -1;
}


// the formatter's core, wraps the line and justifies it if needed
int TextFormatter::WrapLine(CFDC& dc, Paragraph& pp,
 FilePos& pos, LineArray& la,
 int top, int maxh)
{
 if (maxh <= 0)
 return 0;
 // process images separately
 if (pp.flags&Paragraph::image)
 return WrapImage(dc, pp, pos, la, top, maxh);
 if (pp.len == 0 || (pp.len == 1 && pp.str[0] == L' ')) {
 dc.SelectFont(0, 0);
 int fh, fa;
 dc.GetFontSize(fh, fa);
 if (fh > maxh)
 return -1;
 Line l;
 l.pos = pos;
 l.flags = Line::first | Line::last | Line::defstyle;
 l.height = fh;
 l.base = fa;
 la.Add(l);
 pos.off = pp.len;
 return l.height;
 }
 if (m_hyphenate)
 pp.Hyphenate();
 const wchar_t *str = pp.str;
 int len = pp.len;
 Buffer<int> dx(len + 1);
 int toth = 0;
 while (toth < maxh && pos.off < len) {
 // 1. get text size
 int nch = len;
 int curwidth = m_width;
 int ispace = 0;
 if (pos.off == 0 && (pp.flags&(Paragraph::center | Paragraph::right)) == 0)
 AdjustIndent(curwidth, ispace, pp.lindent, pp.rindent, pp.findent, dc.GetLPX());
 else
 AdjustIndent(curwidth, ispace, pp.lindent, pp.rindent, 0, dc.GetLPX());
 dx[0] = 0;
 int lh = 1, lbase = 1;
 GetTextExtent(dc, pp, pos.off, curwidth, nch, dx + 1, lh, lbase);
 if (toth + lh > maxh)
 return -1;
 if (nch == 0)
 nch = 1;
 // 2. do word wrap
 bool addhyp = false;
 if (nch + pos.off < pp.str.size()) {
 int i;
 for (i = nch; i > 0 && str[pos.off + i] != L' '; --i) {
 // wrap at existing dashes
 if (i < nch && (str[pos.off + i] == L'-' || str[pos.off + i] == 0x2013 ||
 str[pos.off + i] == 0x2014) && i < len - 1 && (str[pos.off + i + 1] == L' ' ||
 iswalpha(str[pos.off + i + 1])))
 {
 ++i;
 break;
 }
 // or at possible hyphenation points
 if (m_hyphenate && pp.cflags[pos.off + i].hyphen &&
 dx[i] + dc.GetHypWidth() <= curwidth) {
 addhyp = true;
 break;
 }
 }
 if (i > 0)
 nch = i;
 else
 addhyp = false;
 }
 // insert it into line list
 if (pos.off == 0 && nch == pp.str.size()) {
 // got full line
 Line l(str, len, false);
 l.pos = pos;
 l.flags = Line::first | Line::last;
 l.ispace = ispace;
 l.height = lh;
 l.base = lbase;
 if (dx[nch] < curwidth) {
 if (pp.flags&Paragraph::center)
 l.ispace += (curwidth - dx[nch]) / 2;
 else if (pp.flags&Paragraph::right)
 l.ispace += curwidth - dx[nch];
 }
 CopyAttr(l.attr, pp.cflags, len);
 for (int j = 0; j < len; ++j)
 l.dx[j] = dx[j + 1] - dx[j];
 la.Add(l);
 pos.off = len;
 }
 else {
 Line l(str + pos.off, nch, addhyp);
 if (addhyp)
 l.str[nch] = L'-';
 l.pos = pos;
 l.ispace = ispace;
 l.height = lh;
 l.base = lbase;
 l.flags = 0;
 if (pos.off == 0)
 l.flags |= Line::first;
 if (pos.off + nch == pp.str.size())
 l.flags |= Line::last;
 for (int j = 0; j < nch; ++j)
 l.dx[j] = dx[j + 1] - dx[j];
 int extra_width = 0;
 if (addhyp)
 l.dx[nch] = extra_width = dc.GetHypWidth();
 // 3. justify/center text if needed
 if (dx[nch] < curwidth) {
 if (addhyp)
 curwidth -= extra_width;
 if (pp.flags&Paragraph::center) {
 l.ispace += (curwidth - dx[nch]) / 2;
 }
 else if (pp.flags&Paragraph::right) {
 l.ispace += curwidth - dx[nch];
 }
 else if ((m_justified || pp.flags&Paragraph::justify) &&
 !(l.flags&Line::last))
 {
 // count spaces in string
 int nspc = 0, i;
 for (i = 0; i < nch; ++i)
 if (L' ' == str[pos.off + i])
 ++nspc;
 // and distribute extra width to them
 if (nspc > 0) {
 int addw = (curwidth - dx[nch]) / nspc;
 int extraddw = curwidth - dx[nch] - addw*nspc;
 for (i = 0; i < nch; ++i) {
 if (str[pos.off + i] == L' ') {
 l.dx[i] += addw;
 if (extraddw) {
 ++l.dx[i];
 --extraddw;
 }
 }
 }
 }
 }
 }
 CopyAttr(l.attr, pp.cflags + pos.off, nch);
 if (addhyp)
 l.attr[nch] = l.attr[nch - 1];
 la.Add(l);
 pos.off += nch;
 while (pos.off < len && str[pos.off] == L' ')
 ++pos.off;
 }
 toth += lh;
 }
 return toth;
}

bool TextFormatter::FormatFwd(CFDC& dc, FilePos start) {
 AdjustPos(start); // just to be safe
 if (start.para >= m_tf->Length(start.docid))
 return false; // at eof
 m_lines.RemoveAll();
 m_top = start;
 bool header = true;
 for (int page = 0; page < m_pages; ++page) {
 int h = 0;
 int beg = m_lines.GetSize();
 while (h < m_height && start.para < m_tf->Length(start.docid)) {
 Paragraph para(m_tf->GetParagraph(start.docid, start.para));
 bool empty = para.len == 0 || para.len == 1 && para.str[0] == _T(' ');
 if (h == 0 && empty) {
 start.off = para.len;
 AdjustPos(start);
 continue;
 }
 if (para.flags & Paragraph::header) {
 if (!header)
 break;
 }
 else
 if (!empty && !(para.flags & Paragraph::image))
 header = false;
 int lh = WrapLine(dc, para, start, m_lines, h, m_height - h);
 if (lh < 0)
 break;
 AdjustPos(start);
 h += lh;
 }
 m_pagelen.SetAtGrow(page, m_lines.GetSize() - beg);
 }
 m_bot = start;
 Highlight();
 // add one dummy line always
 Line l;
 l.pos = m_bot;
 m_lines.Add(l);
 return true;
}

bool TextFormatter::FormatBack(CFDC& dc, FilePos start, FilePos prev_top) {
 AdjustPos(start, true);
 if (start.para == 0 && start.off == 0)
 return false; // at the top
 m_lines.RemoveAll();
 m_bot = start;
 for (int page = m_pages - 1; page >= 0; --page) {
 LineArray tmp;
 FilePos pos = start;
 int h = 0;
 // while there are still paragrahs before
 while (h < m_height && (pos.para>0 || pos.off > 0)) {
 // format entire paragraph
 LineArray cp;
 Paragraph para(m_tf->GetParagraph(pos.docid, pos.para));
 if (pos.off < para.len) // double check args
 para.len = pos.off;
 else
 pos.off = para.len;
 FilePos fp = FilePos(pos.para, 0, pos.docid);
 WrapLine(dc, para, fp, cp, 0, 32768);
 // insert the formatted paragraph at start of list
 tmp.InsertAt(0, &cp);
 for (int i = 0; i < cp.GetSize(); ++i)
 h += cp[i].height;
 pos.off = 0;
 AdjustPos(pos, true);
 }
 // delete extra lines
 int j;
 // remove top lines
 for (h = 0, j = tmp.GetUpperBound(); j >= 0 && h + tmp[j].height <= m_height; --j)
 h += tmp[j].height;
 if (j < tmp.GetUpperBound()) {
 if (j >= 0 && prev_top != 0 && tmp[j + 1].pos >= prev_top) {
 --j;
 tmp.RemoveAt(0, j + 1);
 // now remove bottom lines
 for (h = j = 0; j < tmp.GetSize() && h + tmp[j].height <= m_height; ++j)
 h += tmp[j].height;
 if (j < tmp.GetSize())
 tmp.RemoveAt(j, tmp.GetSize() - j);
 }
 else
 tmp.RemoveAt(0, j + 1);
 }
 // save lines
 m_lines.InsertAt(0, &tmp);
 m_pagelen.SetAtGrow(page, tmp.GetSize());
 start = m_lines[0].pos;
 if (start.para == 0 && start.off == 0) // we reached the top of file
 return FormatFwd(dc, FilePos(0, 0, start.docid));
 }
 // save positions
 m_top = m_lines[0].pos;
 Highlight();
 // add one dummy line always
 Line l;
 l.pos = m_bot;
 m_lines.Add(l);
 return true;
}

void TextFormatter::AdjustPos(FilePos& p, bool back) {
 if (back) {
 if (p.para > 0) {
 if (p.para >= m_tf->Length(p.docid)) {
 if (m_tf->Length(p.docid) > 0) {
 p.para = m_tf->Length(p.docid) - 1;
 p.off = m_tf->GetPLength(p.docid, p.para);
 }
 }
 else {
 if (p.off == 0) {
 --p.para;
 p.off = m_tf->GetPLength(p.docid, p.para);
 }
 }
 }
 }
 else {
 if (p.para >= 0 && p.para < m_tf->Length(p.docid) && p.off >= m_tf->GetPLength(p.docid, p.para)) {
 p.off = 0;
 p.para++;
 }
 }
}

void TextFormatter::SetSize(int width, int margin, int height, int pages,
 int angle)
{
 if (height < 1)
 height = 1;
 if (pages < 1)
 pages = 1;
 m_total_width = width;
 m_margin = margin;
 m_width = m_total_width - 2 * m_margin;
 if (m_width < 1)
 m_width = 1;
 m_height = height;
 m_pages = pages;
 m_angle = angle;
}

bool TextFormatter::AtTop()
{
 FilePos p(m_top);
 AdjustPos(p, true);
 return p.off == 0 && p.para == 0;
}

bool TextFormatter::AtEof()
{
 FilePos p(m_bot);
 AdjustPos(p);
 return p.para >= m_tf->Length(p.docid);
}

static bool intersect(int a, int la, int b, int lb, int& i, int& li) {
 if (a >= b && a < b + lb) {
 i = a - b;
 if (la > b + lb - a)
 li = b + lb - a;
 else
 li = la;
 return true;
 }
 if (b >= a && b < a + la) {
 i = 0;
 if (lb > a + la - b)
 li = a + la - b;
 else
 li = lb;
 return true;
 }
 return false;
}

void TextFormatter::Highlight() {
 if (!m_hllen || m_hlstart.docid != DocId())
 return;

 FilePos hls(m_hlstart);
 int hll = m_hllen;
 for (int i = 0; i < m_lines.GetSize(); ++i)
 if (hls.para == m_lines[i].pos.para) {
 int beg, len;
 if (hls.off - m_lines[i].pos.off < m_lines[i].real_len &&
 intersect(hls.off, hll, m_lines[i].pos.off, m_lines[i].str.size(), beg, len))
 {
 int top = beg + len;
 while (beg < top)
 m_lines[i].attr[beg++].hibg = true;
 m_lines[i].flags &= ~Line::defstyle;
 }
 else
 m_lines[i].CheckStyle();
 // advance our pointer
 if (i<m_lines.GetSize() - 1 && m_lines[i + 1].pos.para>hls.para &&
 hls.off + hll > m_tf->GetPLength(hls.docid, hls.para))
 {
 hll -= m_tf->GetPLength(hls.docid, hls.para) - hls.off;
 ++hls.para;
 hls.off = 0;
 }
 }
 else
 m_lines[i].CheckStyle();
}

bool TextFormatter::SetHighlight(FilePos pos, int len) {
 if (m_hllen == len && (!len || pos == m_hlstart)) // avoid extra work
 return false;
 // remove highlighting
 for (int i = 0; i < m_lines.GetSize(); ++i) {
 for (int j = 0; j < m_lines[i].attr.size(); ++j)
 m_lines[i].attr[j].hibg = false;
 }
 m_hlstart = pos;
 m_hllen = len;
 Highlight();
 return true;
}

void Line::CheckStyle() {
 Attr *p = attr;
 Attr *e = p + attr.size();
 while (p < e)
 if ((*p++).wa) {
 flags &= ~defstyle;
 return;
 }
 flags |= defstyle;
}

int TextFormatter::Distance(const FilePos& a, const FilePos& b)
{
 FilePos start(a), end(b);
 bool sign = false;
 if (a > b) {
 start = b;
 end = a;
 sign = true;
 }
 // check bounds
 if (start.para < 0)
 start.para = 0;
 if (start.para > m_tf->Length(start.docid)) {
 start.para = m_tf->Length(start.docid);
 start.off = 0;
 }
 if (start.off < 0)
 start.off = 0;
 if (start.off > m_tf->GetPLength(start.docid, start.para))
 start.off = m_tf->GetPLength(start.docid, start.para);
 if (end.para < 0)
 end.para = 0;
 if (end.para > m_tf->Length(end.docid)) {
 end.para = m_tf->Length(end.docid);
 end.off = 0;
 }
 if (end.off < 0)
 end.off = 0;
 if (end.off > m_tf->GetPLength(end.docid, end.para))
 end.off = m_tf->GetPLength(end.docid, end.para);
 // calc distance now
 int dist;
 if (start.para == end.para)
 dist = end.off - start.off;
 else {
 dist = m_tf->GetPLength(start.docid, start.para) - start.off;
 ++start.para;
 while (start.para < end.para) {
 dist += m_tf->GetPLength(start.docid, start.para);
 ++start.para;
 }
 dist += end.off;
 }
 return sign ? -dist : dist;
}

bool TextFormatter::EnsureVisible(CFDC& dc, FilePos pos) {
 if (pos >= m_top && pos < m_bot)
 return false;
 FilePos ptop(pos);
 ptop.off = 0;
 if (!FormatFwd(dc, ptop))
 return true;
 while (m_top.para == pos.para && pos >= m_bot)
 if (!FormatFwd(dc))
 break;
 return true;
}

void TextFormatter::FormatPlainText(CFDC& dc,
 int& width, int& height,
 int fontsize,
 const wchar_t *text, int len,
 LineArray& lines)
{
 lines.RemoveAll();

 int save_width = m_width;
 bool save_justified = m_justified;
 m_width = width;
 m_justified = false;

 const wchar_t *top = text + len;

 Attr attr;
 attr.wa = 0;
 attr.fsize = fontsize;

 int curh = 0;

 while (text < top && curh < height) {
 const wchar_t *p_end = text;
 while (p_end < top && *p_end != '\r' && *p_end != '\n')
 ++p_end;
 Paragraph p(p_end - text);
 memcpy(p.str, text, (p_end - text)*sizeof(wchar_t));
 for (int i = 0; i < p.cflags.size(); ++i)
 p.cflags[i].wa = attr.wa;
 p.findent = 3; // XXX
 int last = lines.GetSize();
 FilePos fp = FilePos();
 int lh = WrapLine(dc, p, fp, lines, curh, height - curh);
 if (lh < 0) {
 // it still might add something
 while (last < lines.GetSize())
 curh += lines[last++].height;
 break;
 }
 curh += lh;
 while (p_end < top && (*p_end == '\r' || *p_end == '\n'))
 ++p_end;
 text = p_end;
 }

 m_width = save_width;
 m_justified = save_justified;

 // deduct ispace
 int min_ispace = -1, max_width = 0;
 for (int i = 0; i < lines.GetSize(); ++i) {
 const Line& ll = lines[i];
 int w = 0;
 for (int j = 0; j < ll.dx.size(); ++j)
 w += ll.dx[j];
 w += ll.ispace;
 if (max_width < w)
 max_width = w;
 if (min_ispace<0 || min_ispace>ll.ispace)
 min_ispace = ll.ispace;
 }

 if (min_ispace > 0) {
 for (int i = 0; i < lines.GetSize(); ++i)
 lines[i].ispace -= min_ispace;
 max_width -= min_ispace;
 }

 height = curh;
 width = max_width;
}

bool TextFormatter::FormatFwdAdj(CFDC& dc) {
 // if an image crosses the bottom of the window, and does not start
 // at the top of window, then move down only until the image is
 // fully visible
 AdjustPos(m_bot);
 Paragraph p(m_tf->GetParagraph(m_bot.docid, m_bot.para));
 if (p.flags&Paragraph::image && // image
 m_bot.off > 0 && // crosses window border
 (m_top.docid != m_bot.docid || m_top.para != m_bot.para)) // is not visible at top
 // then do a partial move forward to show a whole image
 m_bot.off = 0;
 return FormatFwd(dc, m_bot);
}

void TextFormatter::SetTop(FilePos pos) {
 if (pos.docid >= m_tf->GetSubDocCount()) {
 pos.docid = m_tf->GetSubDocCount() - 1;
 if (pos.docid < 0)
 pos.docid = 0;
 }
 if (pos.para >= m_tf->Length(pos.docid)) {
 pos.para = m_tf->Length(pos.docid) - 1;
 if (pos.para < 0)
 pos.para = 0;
 }
 if (pos.off >= m_tf->GetPLength(pos.docid, pos.para)) {
 pos.off = m_tf->GetPLength(pos.docid, pos.para) - 1;
 if (pos.off < 0)
 pos.off = 0;
 }
 m_top = pos;
}
