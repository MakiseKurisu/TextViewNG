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
* $Id: TextParser.h,v 1.39.2.3 2004/01/08 11:06:06 mike Exp $
*
*/

#pragma warning(disable:4100)

#if !defined(AFX_TEXTPARSER_H__2F6237F5_E47E_45B8_9597_CB0AAAA1413C__INCLUDED_)
#define AFX_TEXTPARSER_H__2F6237F5_E47E_45B8_9597_CB0AAAA1413C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ptr.h"
#include "Attr.h"
#include "BufFile.h"
#include "Bookmarks.h"

struct Paragraph {
 Buffer<wchar_t> str;
 Buffer<Attr> cflags;
 int len;
 BYTE flags;
 int lindent, rindent, findent;
 struct Link {
 DWORD off; // offset into str
 DWORD len; // number of chars
 const wchar_t *target; // target footnote or paragraph, this is _borrowed_ from textparser
 };
 Buffer<Link> links;
 enum {
 right = 1,
 center = 2,
 justify = 4,
 align_mask = 7,
 header = 8,
 image = 64,
 hypdone = 128
 };
 Paragraph() : len(0), flags(0), lindent(0), rindent(0), findent(0) { }
 Paragraph(int l) : str(l), cflags(l), len(l), flags(0), lindent(0), rindent(0), findent(0) { }
 Paragraph(const Paragraph& p) : str(p.str), cflags(p.cflags), len(p.len),
 flags(p.flags), lindent(p.lindent), rindent(p.rindent), findent(p.findent),
 links(p.links) { }
 Paragraph& operator=(const Paragraph& p) {
 str = p.str;
 cflags = p.cflags;
 len = p.len;
 flags = p.flags;
 links = p.links;
 lindent = p.lindent;
 rindent = p.rindent;
 findent = p.findent;
 return *this;
 }
 void Hyphenate();
};

struct Image {
 HBITMAP hBmp;
 int width; // virutual/rotated
 int height;
};

class TextParser
{
public:
 class Meter {
 public:
 virtual ~Meter() { }
 virtual void SetMax(DWORD max) = 0;
 virtual void SetCur(DWORD cur) = 0;
 };

protected:
 CBufFile *m_fp;
 int m_format;
 int m_encoding;
 Meter *m_mm;
 Bookmarks *m_bmk;
 HANDLE m_heap;

 TextParser(Meter *m, CBufFile *fp, HANDLE heap, Bookmarks *bmk) :
 m_fp(fp), m_mm(m), m_bmk(bmk), m_heap(heap) {
 if (m_mm) m_mm->SetMax(m_fp->size());
 }

 void ProgSetCur(DWORD cur) { if (m_mm) m_mm->SetCur(cur); }

public:
 virtual ~TextParser();

 int GetFormat() { return m_format; }
 int GetEncoding() { return m_encoding; }

 // paragraphs, mandatory
 virtual Paragraph GetParagraph(int docid, int para) = 0;
 virtual int Length(int docid) = 0; // in paragraphs
 virtual int GetPLength(int docid, int para) = 0;
 virtual int GetPStart(int docid, int para) = 0;
 virtual int GetTotalLength(int docid) = 0;
 virtual int LookupParagraph(int docid, int charpos) = 0;

 // images, optional
 virtual bool GetImage(const wchar_t *name, HDC hDC, int maxwidth, int maxheight, int rotation, Image& img) { return false; }
 virtual void InvalidateImageCache() { }
 virtual bool IsImage() { return false; }

 // subdocuments, optional
 virtual int GetSubDocCount() { return 1; }
 virtual CString GetSubDocName(int docid) { return _T("Main"); }

 // links, optional
 virtual bool LookupReference(const wchar_t *rname, FilePos& dest) { return false; }

 // helper functions
 static int DetectFormat(CBufFile *fp);
 static int GetNumFormats();
 static const TCHAR *GetFormatName(int format);
 static TextParser *Create(Meter *m, CBufFile *fp, int format, int encoding, Bookmarks *bma);
};

#endif // !defined(AFX_TEXTPARSER_H__2F6237F5_E47E_45B8_9597_CB0AAAA1413C__INCLUDED_)
