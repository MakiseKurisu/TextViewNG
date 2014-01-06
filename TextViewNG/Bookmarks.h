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
* $Id: Bookmarks.h,v 1.23.2.2 2003/12/17 12:20:39 mike Exp $
*
*/

#pragma warning(disable:4100)

#if !defined(AFX_BOOKMARKS_H__BEA3E1C9_F320_44CA_A735_A38A811FD7CB__INCLUDED_)
#define AFX_BOOKMARKS_H__BEA3E1C9_F320_44CA_A735_A38A811FD7CB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FilePos.h"
class TextFile;

class Bookmarks
{
public:
    enum
    {
        BMK = 1,
        BMNEW = 2,
        BMDEL = 4,
        BMCHG = 8
    };
    enum
    {
        SPREVCH = 0,
        SNEXTCH = 1,
        SPREVANY = 2,
        SNEXTANY = 3,
        SNEXTICH = 4,
        SPREVBMK = 5,
    };
private:
    struct BE
    {
        CString text;
        FilePos ref;
        FilePos text_ref;
        DWORD flags;
        int level;
        CString *tmp;
    };
public:
    Bookmarks(const CString& filename);
    ~Bookmarks();

    // access
    int GetSize()
    {
        return m_bmk.GetSize();
    }
    int Level(int idx)
    {
        return m_bmk[idx].level;
    }
    int Flags(int idx)
    {
        return m_bmk[idx].flags;
    }
    FilePos Ref(int idx)
    {
        return m_bmk[idx].ref;
    }
    CString Text(int idx, TextFile *parser);

    // modification
    int Add(const CString& text, FilePos pos)
    {
        return AddImp(0, 0, 0, &text, pos, BMK, -1);
    }
    int AddTocEnt(int para_start, int para_count, int docid, FilePos pos, int level)
    {
        return AddImp(para_start, para_count, docid, NULL, pos, 0, level);
    }
    void Remove(int index);
    void Change(int index, const CString& text);
    int Commit(int cidx);
    void Rollback();

    int UserBookmarks();

    int Encoding()
    {
        return m_encoding;
    }
    int Format()
    {
        return m_format;
    }
    FilePos StartPos()
    {
        return m_startpos;
    }
    void SetFormat(int fmt)
    {
        if (m_format != fmt) m_changed = true; m_format = fmt;
    }
    void SetEncoding(int enc)
    {
        if (m_encoding != enc) m_changed = true; m_encoding = enc;
    }
    void SetStartPos(FilePos p)
    {
        if (p != m_startpos) m_changed = true; m_startpos = p;
    }
    int BFind(FilePos p, int type);
    bool BookmarksInRange(FilePos start, FilePos end);
    bool BookmarkFind(FilePos& start, FilePos end);

    void SaveToRegistry();
    void LoadFromRegistry();
    static void CleanupRegistry(int max_count);
    bool SaveInfo();

    static CString find_last_file();
    static void get_recent_files(CStringArray& fl, int num, FILETIME& toptime);

    void NormalizeLevels();
    int NumTopMarks()
    {
        return m_topbmk;
    }
    int NumBookmarks()
    {
        return m_ubmk;
    }

    // export/import
    static bool ExportAllBookmarks(const CString& destfile);

private:
    CArray<BE, BE&> m_bmk;
    int m_format;
    int m_encoding;
    FilePos m_startpos;
    CString m_filename;
    CString m_shortname;
    bool m_changed;
    int m_topbmk;
    int m_ubmk;

    void Check(int index)
    {
        ASSERT(index >= 0 && index < m_bmk.GetSize());
    }
    int AddImp(int para_start, int para_count, int docid,
        const CString *text,
        FilePos pos, DWORD flags, int level);
};

#endif // !defined(AFX_BOOKMARKS_H__BEA3E1C9_F320_44CA_A735_A38A811FD7CB__INCLUDED_)
