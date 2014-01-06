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
* $Id: TextFile.cpp,v 1.45.2.5 2004/07/07 12:04:47 mike Exp $
*
*/

#define _WIN32_WINNT _WIN32_WINNT_MAXVER

#include <afxwin.h>
#include <afxtempl.h>

#include "resource.h"

#include "TextFile.h"
#include "TextViewNG.h"
#include "ProgressDlg.h"
#include "Unicode.h"
#include "config.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

TextFile::TextFile(RFile *fp, const CString& name) :
m_name(name), m_bookmarks(name), m_dictp(NULL)
{
    m_format = m_bookmarks.Format();
    m_enc = m_bookmarks.Encoding();
    if (m_format >= TextParser::GetNumFormats())
        m_format = -1;
    if (m_enc >= Unicode::GetNumCodePages())
        m_enc = -1;
    // create a buffered file for it
    auto_ptr<RFile> rf(fp);
    m_fp = new CBufFile(rf);
    // and initialize parser
    m_bookmarks.LoadFromRegistry();
    SetFormatEncodingImp(m_format, m_enc, &m_bookmarks);
    m_bookmarks.NormalizeLevels();
}

void TextFile::SaveBookmarks(FilePos cur)
{
    m_bookmarks.SetStartPos(cur);
    if (m_bookmarks.SaveInfo())
        m_bookmarks.SaveToRegistry();
}

void TextFile::SetFormatEncodingImp(int format, int enc, Bookmarks *bmk)
{
    TextParser *np = NULL;
    CProgressDlg dlg(Name(), AfxGetMainWnd());

    dlg.SetMax(m_fp->size());
    if (format < 0)
    {
        m_fp->seek(0);
        int nf = TextParser::DetectFormat(m_fp.get());
        np = TextParser::Create(&dlg, m_fp.get(), nf, enc, bmk);
    }
    else
    {
        if (m_tp.get() && format == m_tp->GetFormat())
        {
            m_format = format;
            return;
        }
        np = TextParser::Create(&dlg, m_fp.get(), format, enc, bmk);
    }
    if (np == NULL)
        return;
    m_tp.reset(np);
    m_format = format;
    m_enc = enc;
    m_bookmarks.SetFormat(m_format);
    m_bookmarks.SetEncoding(m_enc);
}

void TextFile::Reparse()
{
    TextParser *np = NULL;
    CProgressDlg dlg(Name(), AfxGetMainWnd());

    dlg.SetMax(m_fp->size());
    int fmt = m_format < 0 ? m_tp->GetFormat() : m_format;
    np = TextParser::Create(&dlg, m_fp.get(), fmt, m_enc, NULL);
    if (np == NULL)
        return;
    m_tp.reset(np);
}

const TCHAR *TextFile::GetEncodingName(int enc)
{
    return enc < 0 ? _T("Auto") : Unicode::GetCodePageName(enc);
}

const TCHAR *TextFile::GetFormatName(int format)
{
    return format < 0 ? _T("Auto") : TextParser::GetFormatName(format);
}

class DummyRFile : public RFile
{
public:
    DummyRFile() : RFile(CString())
    {
    }

    // generic file operations
    virtual DWORD size()
    {
        return 0;
    }
    virtual DWORD read(void *buf)
    {
        return 0;
    }
    virtual void seek(DWORD pos)
    {
    }
};

TextFile *TextFile::Open(const CString& filename)
{
    auto_ptr<RFile> fp;
    CString cur(filename);
    RFile *rf = NULL;
    TextFile *tf = NULL;

    CString FILENAME;
    // normalize filename
    TCHAR buffer[MAX_PATH], *filepart;
    DWORD nc = GetFullPathName(cur, sizeof(buffer) / sizeof(TCHAR), buffer, &filepart);
    if (nc > 0 && nc < sizeof(buffer) / sizeof(buffer[0]))
        FILENAME = buffer;
    else
        FILENAME = filename;

    for (;;)
    {
        fp = new RFile(cur);
        if (fp->Reopen())
        {
            rf = fp.release();
            TRY{
                tf = new TextFile(rf, FILENAME);
            } CATCH_ALL(e)
            {
                tf = NULL;
            }
            END_CATCH_ALL;
            if (tf && !tf->Ok())
            {
                delete tf;
                tf = NULL;
            }
            else
            {
                CString ipath(FILENAME);
                int i = ipath.ReverseFind(_T('\\'));
                if (i != -1)
                {
                    CTVApp::SetStr(_T("OpenPath"), ipath.Left(i));
                }
            }
            break;
        }
        else
        {
            // unable to open, chop last piece
            int spos = cur.ReverseFind(_T('\\'));
            if (spos <= 0) // we failed
            {
                break;
            }
            cur.Delete(spos, cur.GetLength() - spos);
        }
    }
    // when we get here we failed to open any prefix of filename
    return tf;
}

// these tricks are needed to distinguish between a true end of paragraph
// and position 0, which is difficult when length is also 0
int TextFile::GetPLength(int docid, int para)
{
    int pl = Parser(docid)->GetPLength(docid, para);
    return pl == 0 ? 1 : pl;
}

Paragraph TextFile::GetParagraph(int docid, int para)
{
    Paragraph p(Parser(docid)->GetParagraph(docid, para));
    if (p.len == 0)
    {
        p.len = 1;
        p.str = Buffer<wchar_t>(1); p.str[0] = _T(' ');
        p.cflags = Buffer<Attr>(1);
        p.cflags[0].wa = 0;
    }

    // map ipa extensions to alt font
    for (int j = 0; j < p.len; ++j)
        if (p.str[j] >= 0x250 && p.str[j] < 0x2b0)
            p.cflags[j].xfont = 1;
    return p;
}

bool TextFile::LookupDict(const wchar_t *name, FilePos& dest)
{
    if (m_dictp && m_dictp->LookupReference(name, dest))
    {
        dest.docid = -1;
        return true;
    }
    return false;
}
