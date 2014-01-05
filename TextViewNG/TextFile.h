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
* $Id: TextFile.h,v 1.32.2.2 2003/09/21 11:40:17 mike Exp $
*
*/

#if !defined(AFX_TEXTFILE_H__37F37F43_6FC5_4C70_AFBB_1187E125D777__INCLUDED_)
#define AFX_TEXTFILE_H__37F37F43_6FC5_4C70_AFBB_1187E125D777__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TextParser.h"

class TextFile
{
protected:
    TextFile(RFile *fp, const CString& name);
    void		  SetFormatEncodingImp(int format, int enc, Bookmarks *bmk);
    TextParser	  *Parser(int docid) { return docid == -1 && m_dictp ? m_dictp : m_tp.get(); }
public:
    ~TextFile() { }

    int		  ByteLength() { return m_fp->size(); }

    // all these proxy functions switch to the dictionary when docid==-1

    // paragraphs
    int		  Length(int docid) { return Parser(docid)->Length(docid); }
    int		  GetPLength(int docid, int para);
    Paragraph	  GetParagraph(int docid, int para);
    int		  GetPStart(int docid, int para) { return Parser(docid)->GetPStart(docid, para); }
    int		  GetTotalLength(int docid) { return Parser(docid)->GetTotalLength(docid); }
    int		  LookupParagraph(int docid, int charpos) { return Parser(docid)->LookupParagraph(docid, charpos); }

    // documents
    int		  GetSubDocCount() { return m_tp->GetSubDocCount(); }
    CString	  GetSubDocName(int docid) { return m_tp->GetSubDocName(docid); }

    // links
    bool		  LookupReference(const wchar_t *name, FilePos& dest) { return m_tp->LookupReference(name, dest); }
    bool		  LookupDict(const wchar_t *name, FilePos& dest);
    bool		  GetImage(const wchar_t *name, HDC hDC,
        int maxwidth, int maxheight, int rotation, Image& img) {
        return m_tp->GetImage(name, hDC, maxwidth, maxheight, rotation, img);
    }
    void		  InvalidateImageCache() { m_tp->InvalidateImageCache(); }

    // a small helper
    int		  AbsPos(const FilePos& p) { return Parser(p.docid)->GetPStart(p.docid, p.para) + p.off; }

    int		  GetEncoding() { return m_enc; }
    int		  GetFormat() { return m_format; }
    int		  GetRealEncoding() { return m_tp->GetEncoding(); }
    int		  GetRealFormat() { return m_tp->GetFormat(); }
    void		  SetFormatEncoding(int format, int enc) {
        SetFormatEncodingImp(format, enc, NULL);
    }
    void		  Reparse();
    static const TCHAR *GetEncodingName(int enc);
    static const TCHAR *GetFormatName(int format);

    static TextFile *Open(const CString& filename);
    const CString&  Name() const { return m_name; }

    CString	  CompressionInfo() { return m_fp->CompressionInfo(); }

    bool		  Ok() { return m_fp.get() && m_tp.get(); }

    Bookmarks&	  bmk() { return m_bookmarks; }
    void		  SaveBookmarks(FilePos cur);

    void		  SetDict(TextParser *dp) { m_dictp = dp; }

    // images
    bool		  IsImage() { return m_tp->IsImage(); }
protected:
    auto_ptr<CBufFile>	m_fp;
    auto_ptr<TextParser>  m_tp;
    TextParser		*m_dictp;
    int			m_enc;
    int			m_format;
    CString		m_name;
    Bookmarks		m_bookmarks;
};

#endif // !defined(AFX_TEXTFILE_H__37F37F43_6FC5_4C70_AFBB_1187E125D777__INCLUDED_)
