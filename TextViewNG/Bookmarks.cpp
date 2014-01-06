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
* $Id: Bookmarks.cpp,v 1.42.2.6 2005/06/18 19:44:12 mike Exp $
*
*/

#define _WIN32_WINNT _WIN32_WINNT_MAXVER

#include <afxwin.h>
#include <afxtempl.h>

#include "config.h"
#include "ptr.h"
#include "FilePos.h"
#include "Bookmarks.h"
#include "TextViewNG.h"
#include "Unicode.h"
#include "TextFile.h"

struct Goodch {
    static bool isgoodch(TCHAR ch) { return (ch >= _T('0') && ch <= _T('9')) || (ch >= _T('a') && ch <= _T('z')) || (ch >= _T('A') && ch <= _T('Z')) || ch == _T('.') || ch == _T('-') || ch == _T('_') || ch == _T(' '); }
};
struct Goodch2 {
    static bool isgoodch(TCHAR ch) { return ch != _T('%') && ch != _T('/') && ch != _T('\\'); }
};
struct Goodch3 {
    static bool isgoodch(TCHAR ch) { return ch != _T('%') && ch != _T('\r') && ch != _T('\n'); }
};

template<class T>
static CString param_escape(const CString& str) {
    int   rlen;
    int   i;
    CString ret;
    TCHAR   *cp;

    for (i = rlen = 0; i < str.GetLength(); ++i)
        if (T::isgoodch(str[i]))
            ++rlen;
        else
            rlen += 5;
    cp = ret.GetBuffer(rlen);
    for (i = 0; i < str.GetLength(); ++i)
        if (T::isgoodch(str[i]))
            *cp++ = str[i];
        else {
            _stprintf_s(cp, rlen, _T("%%%04X"), ((unsigned)str[i]) & 0xffff);
            cp += 5;
        }
        ret.ReleaseBuffer(rlen);
        return ret;
}

#define escape(x)   param_escape<Goodch>(x)
#define escape2(x)  param_escape<Goodch2>(x)
#define escape3(x)  param_escape<Goodch3>(x)

static CString unescape(const CString& str) {
    int   rlen;
    int   i, j;
    TCHAR   n;
    CString ret;
    TCHAR   *cp;

    for (i = rlen = 0; i < str.GetLength(); ++i) {
        if (str[i] == _T('%')) {
            if (i + 4 < str.GetLength())
                i += 4;
            else
                break;
        }
        ++rlen;
    }
    cp = ret.GetBuffer(rlen);
    for (i = 0; i < str.GetLength(); ++i) {
        if (str[i] == _T('%')) {
            if (i + 4 >= str.GetLength())
                break;
            for (j = 1 + i, n = 0; j < 5 + i; ++j) {
                n <<= 4;
                if (str[j] >= _T('0') && str[j] <= _T('9'))
                    n += str[j] - _T('0');
                else if (str[j] >= _T('A') && str[j] <= _T('F'))
                    n += str[j] - _T('A') + 10;
                else if (str[j] >= _T('a') && str[j] <= _T('f'))
                    n += str[j] - _T('a') + 10;
            }
            *cp++ = n == 0 ? _T(' ') : n;
            i += 4;
        }
        else
            *cp++ = str[i];
    }
    ret.ReleaseBuffer(rlen);
    return ret;
}

Bookmarks::Bookmarks(const CString& filename) : m_filename(filename),
m_shortname(filename), m_changed(true), m_topbmk(0), m_ubmk(0)
{
    // extract filename
    int last = max(m_shortname.ReverseFind(_T('\\')), m_shortname.ReverseFind(_T('/')));
    m_shortname.Delete(0, last + 1);
    // find info in registry
    CString info = AfxGetApp()->GetProfileString(_T("Bookmarks"), escape2(m_shortname));
    DWORD dummy;
    if (!info.GetLength() || _stscanf_s(info, _T("%d,%d,%u,%u"), &m_format, &m_encoding, &dummy, &dummy) != 4)
    {
        m_format = -1;
        m_encoding = CTVApp::GetInt(_T("DefEncoding"), -1);
        m_startpos.off = m_startpos.para = 0;
    }
}

Bookmarks::~Bookmarks() {
    SaveInfo();
}

CString Bookmarks::Text(int idx, TextFile *parser) {
    if (m_bmk[idx].flags & BMK) {
        // return changed text if possible
        if (m_bmk[idx].flags & BMCHG && m_bmk[idx].tmp)
            return *m_bmk[idx].tmp;
        return m_bmk[idx].text;
    }
    if (m_bmk[idx].text.GetLength() > 0)
        return m_bmk[idx].text;
    FilePos p(m_bmk[idx].text_ref);
    if (p.off == 0 || !parser)
        return CString();
    // ok, here we extract the bookmark from the file
    CString ret;
    int   ps = p.para;
    int   pe = p.para + p.off;
    while (ps < pe) {
        if (p.para != ps)
            ret += _T(" ");
        ret += Unicode::ToCS(parser->GetParagraph(p.docid, ps).str);
        ++ps;
    }
    return ret;
}

int  Bookmarks::AddImp(int para_start, int para_count, int docid, const CString *text,
    FilePos pos, DWORD flags, int level)
{
    m_changed = true;
    BE be;
    if (text)
        be.text = *text;
    be.ref = pos;
    be.text_ref.para = para_start;
    be.text_ref.off = para_count;
    be.text_ref.docid = docid;
    ASSERT(docid >= 0);
    be.flags = flags;
    be.level = level;
    int index = BFind(pos, SNEXTICH);
    if (index >= m_bmk.GetSize())
        m_bmk.Add(be);
    else
        m_bmk.InsertAt(index, be);
    if (flags&BMK)
        ++m_topbmk, ++m_ubmk;
    else if (level == 0)
        ++m_topbmk;
    return index;
}

void  Bookmarks::Remove(int index) {
    Check(index);
    if (m_bmk[index].flags&BMK)
        m_bmk[index].flags |= BMDEL;
}

void  Bookmarks::Change(int index, const CString& text) {
    Check(index);
    if (m_bmk[index].flags&BMK) {
        if (m_bmk[index].flags&BMCHG)
            *m_bmk[index].tmp = text;
        else
            m_bmk[index].tmp = new CString(text);
        m_bmk[index].flags |= BMCHG;
    }
}

int  Bookmarks::Commit(int cidx) {
    for (int i = 0; i < m_bmk.GetSize();) {
        if (m_bmk[i].flags&BMCHG) {
            m_changed = true;
            m_bmk[i].text = *m_bmk[i].tmp;
            delete m_bmk[i].tmp;
            m_bmk[i].flags &= ~BMCHG;
            m_bmk[i].flags |= BMNEW;
        }
        if (m_bmk[i].flags&BMDEL) {
            m_changed = true;
            m_bmk.RemoveAt(i);
            if (cidx > i)
                --cidx;
            --m_topbmk;
            --m_ubmk;
        }
        else
            ++i;
    }
    return cidx;
}

void  Bookmarks::Rollback() {
    for (int i = 0; i < m_bmk.GetSize(); ++i) {
        if (m_bmk[i].flags&BMCHG)
            delete m_bmk[i].tmp;
        m_bmk[i].flags &= ~(BMCHG | BMDEL);
    }
}

void  Bookmarks::SaveToRegistry() {
    if (m_shortname == _T("NUL"))
        return;
    HKEY   hBmk = AfxGetApp()->GetSectionKey(_T("Bookmarks"));
    if (!hBmk)
        return;
    CString sect(escape2(m_shortname));
    RegDeleteKey(hBmk, sect);
    HKEY res;
    DWORD disp;
    CString name, value;
    if (RegCreateKeyEx(hBmk, sect, 0, REG_NONE, 0, HR_REG_PERM, NULL, &res, &disp) == ERROR_SUCCESS) {
        value.Format(_T("%d,%d,%d"), m_startpos.para, m_startpos.off, m_startpos.docid);
        RegSetValueEx(res, NULL, 0, REG_SZ, (LPBYTE)(const TCHAR *)value,
            (value.GetLength() + 1)*sizeof(TCHAR));
        int n = 0;
        for (int i = 0; i < m_bmk.GetSize(); ++i) {
            if (!(m_bmk[i].flags&BMK))
                continue;
            name.Format(_T("%d"), n);
            value.Format(_T("%d,%d,%d,%s"), m_bmk[i].ref.para, m_bmk[i].ref.off,
                m_bmk[i].ref.docid, (const TCHAR *)m_bmk[i].text);
            RegSetValueEx(res, name, 0, REG_SZ, (LPBYTE)(const TCHAR *)value,
                (value.GetLength() + 1)*sizeof(TCHAR));
            ++n;
        }
        RegCloseKey(res);
    }
    RegCloseKey(hBmk);
}

bool  Bookmarks::SaveInfo() {
    if (m_changed) {
        if (m_shortname == _T("NUL"))
            return false;
        SYSTEMTIME tm;
        FILETIME ftm = { 0, 0 };
        GetLocalTime(&tm);
        SystemTimeToFileTime(&tm, &ftm);
        CString info;
        info.Format(_T("%d,%d,%u,%u,%s"), m_format, m_encoding, ftm.dwLowDateTime, ftm.dwHighDateTime, (LPCTSTR)m_filename);
        AfxGetApp()->WriteProfileString(_T("Bookmarks"), escape2(m_shortname), info);
        m_changed = false;
        return true;
    }
    return false;
}

void  Bookmarks::LoadFromRegistry() {
    HKEY   hBmk = AfxGetApp()->GetSectionKey(_T("Bookmarks"));
    if (!hBmk)
        return;
    CString sect(escape2(m_shortname));
    HKEY res;
    CString name, value;
    if (RegOpenKeyEx(hBmk, sect, 0, HR_REG_PERM, &res) == ERROR_SUCCESS) {
        TCHAR     buf[4096];
        DWORD     type, len = sizeof(buf);
        if (RegQueryValueEx(res, NULL, 0, &type, (LPBYTE)buf, &len) == ERROR_SUCCESS && type == REG_SZ) {
            FilePos pp;
            if (_stscanf_s(buf, _T("%d,%d,%d"), &pp.para, &pp.off, &pp.docid) >= 2)
                SetStartPos(pp);
        }
        for (int i = 0;; ++i) {
            name.Format(_T("%d"), i);
            len = sizeof(buf);
            if (RegQueryValueEx(res, name, 0, &type, (LPBYTE)buf, &len) == ERROR_SUCCESS && type == REG_SZ) {
                FilePos pp;
                int  size_cp = sizeof(buf) / sizeof(TCHAR);
                TCHAR *cp = value.GetBuffer(size_cp);
                if (_stscanf_s(buf, _T("%d,%d,%d,%[^\001]"), &pp.para, &pp.off, &pp.docid, cp, size_cp == 4 || _stscanf_s(buf, _T("%d,%d,%[^\001]"), &pp.para, &pp.off, cp, size_cp) == 3))
                {
                    value.ReleaseBuffer();
                    Add(value, pp);
                }
                else
                    value.ReleaseBuffer(0);
            }
            else
                break;
        }
        RegCloseKey(res);
    }
    RegCloseKey(hBmk);
}

CString Bookmarks::find_last_file() {
    HKEY     hKey = AfxGetApp()->GetSectionKey(_T("Bookmarks"));
    if (!hKey)
        return CString();
    CString   filename, tmp;
    FILETIME  tt;
    tt.dwLowDateTime = tt.dwHighDateTime = 0;
    for (DWORD index = 0;; ++index) {
        TCHAR   name[1024];
        DWORD   namelen = sizeof(name) / sizeof(TCHAR);
        TCHAR   value[4096];
        DWORD   valuelen = sizeof(value);
        DWORD   type;
        if (RegEnumValue(hKey, index, name, &namelen, NULL, &type, (LPBYTE)value, &valuelen) != ERROR_SUCCESS)
            break;
        if (type != REG_SZ)
            continue;
        int     dummy;
        TCHAR   *cp = tmp.GetBuffer(valuelen);
        FILETIME  tm;
        if (_stscanf_s(value, _T("%d,%d,%u,%u,%[^\001]"), &dummy, &dummy, &tm.dwLowDateTime, &tm.dwHighDateTime, cp, valuelen) != 5) {
            tmp.ReleaseBuffer(0);
            continue;
        }
        tmp.ReleaseBuffer();
        if (tm.dwHighDateTime > tt.dwHighDateTime ||
            (tm.dwHighDateTime == tt.dwHighDateTime && tm.dwLowDateTime > tt.dwLowDateTime))
        {
            tt = tm;
            filename = tmp;
        }
    }
    RegCloseKey(hKey);
    return filename;
}

struct Item {
    TCHAR       *name;
    FILETIME    time;
};

static int __cdecl itemcmp(const void *v1, const void *v2) {
    const Item *i1 = (const Item *)v1;
    const Item *i2 = (const Item *)v2;

    if (i1->time.dwHighDateTime < i2->time.dwHighDateTime)
        return 1;
    if (i1->time.dwHighDateTime > i2->time.dwHighDateTime)
        return -1;
    if (i1->time.dwLowDateTime < i2->time.dwLowDateTime)
        return 1;
    if (i1->time.dwLowDateTime > i2->time.dwLowDateTime)
        return -1;
    return 0;
}

void Bookmarks::CleanupRegistry(int max_count) {
    HKEY     hKey = AfxGetApp()->GetSectionKey(_T("Bookmarks"));
    if (!hKey)
        return;
    CArray<Item, Item&>  ilist;
    Item        ii;
    for (DWORD index = 0;; ++index) {
        TCHAR   name[1024];
        DWORD   namelen = sizeof(name) / sizeof(TCHAR);
        TCHAR   value[4096];
        DWORD   valuelen = sizeof(value);
        DWORD   type;
        if (RegEnumValue(hKey, index, name, &namelen, NULL, &type, (LPBYTE)value, &valuelen) != ERROR_SUCCESS)
            break;
        if (type != REG_SZ)
            continue;
        int     dummy;
        if (_stscanf_s(value, _T("%d,%d,%u,%u,"), &dummy, &dummy, &ii.time.dwLowDateTime, &ii.time.dwHighDateTime) != 4)
            continue;
        ii.name = _tcsdup(name);
        if (ii.name)
            ilist.Add(ii);
    }
    if (ilist.GetSize() > max_count) {
        qsort(ilist.GetData(), ilist.GetSize(), sizeof(Item), itemcmp);
        for (int kk = max_count; kk < ilist.GetSize(); ++kk) {
            RegDeleteKey(hKey, ilist[kk].name);
            RegDeleteValue(hKey, ilist[kk].name);
        }
    }
    RegCloseKey(hKey);
    for (int jj = 0; jj < ilist.GetSize(); ++jj)
        free(ilist[jj].name);
}

void Bookmarks::get_recent_files(CStringArray& fl, int num, FILETIME& toptime) {
    HKEY     hKey = AfxGetApp()->GetSectionKey(_T("Bookmarks"));
    if (!hKey)
        return;
    CArray<Item, Item&>  ilist;
    Item        ii;
    CString       tmp;
    for (DWORD index = 0;; ++index) {
        TCHAR   name[1024];
        DWORD   namelen = sizeof(name) / sizeof(TCHAR);
        TCHAR   value[4096];
        DWORD   valuelen = sizeof(value);
        DWORD   type;
        if (RegEnumValue(hKey, index, name, &namelen, NULL, &type, (LPBYTE)value, &valuelen) != ERROR_SUCCESS)
            break;
        if (type != REG_SZ)
            continue;
        int     dummy;
        TCHAR   *cp = tmp.GetBuffer(valuelen);
        if (_stscanf_s(value, _T("%d,%d,%u,%u,%[^\001]"), &dummy, &dummy, &ii.time.dwLowDateTime, &ii.time.dwHighDateTime, cp, valuelen) != 5)
        {
            tmp.ReleaseBuffer(0);
            continue;
        }
        tmp.ReleaseBuffer();
        ii.name = _tcsdup(tmp);
        if (ii.name)
            ilist.Add(ii);
    }
    qsort(ilist.GetData(), ilist.GetSize(), sizeof(Item), itemcmp);
    for (int kk = 0; kk < ilist.GetSize() && kk < num; ++kk)
        fl.Add(ilist[kk].name);
    if (ilist.GetSize() > 0)
        toptime = ilist[0].time;
    RegCloseKey(hKey);
    for (int jj = 0; jj < ilist.GetSize(); ++jj)
        free(ilist[jj].name);
}

int   Bookmarks::BFind(FilePos p, int type) {
    int low = 0, high = m_bmk.GetSize() - 1;
    while (low <= high) {
        int mid = (low + high) >> 1;
        if (p < m_bmk[mid].ref)
            high = mid - 1;
        else if (m_bmk[mid].ref < p)
            low = mid + 1;
        else {
            switch (type) {
            case SPREVCH:
                while (mid > 0 && m_bmk[mid].flags&BMK)
                    --mid;
                break;
            case SNEXTICH:
                while (mid < m_bmk.GetSize() && m_bmk[mid].ref == p && !(m_bmk[mid].flags&BMK))
                    ++mid;
                break;
            case SNEXTCH:
                while (mid < m_bmk.GetSize() && m_bmk[mid].flags&BMK)
                    ++mid;
                break;
            case SPREVANY: case SNEXTANY:
                break;
            case SPREVBMK:
                while (mid > 0 && !(m_bmk[mid].flags&BMK))
                    --mid;
                if (!(m_bmk[mid].flags&BMK))
                    return -1;
                break;
            }
            return mid;
        }
    }
    // no exact match, this is expected
    switch (type) {
    case SPREVBMK:
        while (high > 0 && !(m_bmk[high].flags&BMK))
            --high;
        if (high < 0 || !(m_bmk[high].flags&BMK))
            return -1;
        break;
    case SPREVCH:
        while (high > 0 && m_bmk[high].flags&BMK)
            --high;
        break;
    case SNEXTICH:
        return low;
    case SNEXTCH:
        while (low < m_bmk.GetSize() && m_bmk[low].flags&BMK)
            ++low;
    case SNEXTANY:
        return low;
    case SPREVANY:
        break;
    }
    return high < 0 ? 0 : high;
}

int   Bookmarks::UserBookmarks() {
    int   n = 0;
    for (int i = 0; i < m_bmk.GetSize(); ++i)
        if (m_bmk[i].flags&BMK)
            ++n;
    return n;
}

void  Bookmarks::NormalizeLevels() {
    int minlevel = -1;
    for (int ii = 0; ii < m_bmk.GetSize(); ++ii) {
        if (m_bmk[ii].flags&BMK)
            continue;
        if (minlevel<0 || minlevel>m_bmk[ii].level)
            minlevel = m_bmk[ii].level;
    }
    m_topbmk = m_ubmk = 0;
    for (int jj = 0; jj < m_bmk.GetSize(); ++jj) {
        if (m_bmk[jj].flags&BMK) {
            ++m_topbmk;
            ++m_ubmk;
            continue;
        }
        if ((m_bmk[jj].level -= minlevel) == 0)
            ++m_topbmk;
    }
}

bool  Bookmarks::BookmarksInRange(FilePos start, FilePos end) {
    if (!m_ubmk) // shortcut when there are no user bookmarks
        return false;
    // ok, have to do some searching
    int ptr = BFind(start, SNEXTANY);
    if (ptr >= m_bmk.GetSize())
        return false;
    while (m_bmk[ptr].ref < end) {
        if (m_bmk[ptr].flags&BMK)
            return true;
        ++ptr;
    }
    return false;
}

bool  Bookmarks::BookmarkFind(FilePos& start, FilePos end) {
    if (!m_ubmk) // shortcut when there are no user bookmarks
        return false;
    // ok, have to do some searching
    int ptr = BFind(start, SNEXTANY);
    while (ptr < m_bmk.GetSize() && m_bmk[ptr].ref < end) {
        if (m_bmk[ptr].flags&BMK) {
            start = m_bmk[ptr].ref;
            return true;
        }
        ++ptr;
    }
    return false;
}

static void  utf8write(HANDLE hFile, const CString& str) {
    Buffer<char> utf8(Unicode::ToUtf8(str));
    DWORD  nw;

    // XXX no error checking
    WriteFile(hFile, utf8, utf8.size(), &nw, NULL);
}

bool  Bookmarks::ExportAllBookmarks(const CString& destfile) {
    // open dest file
    HANDLE  hFile = ::CreateFile(destfile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (hFile == NULL)
        return false;

    // open registry entry
    HKEY   hBmk = AfxGetApp()->GetSectionKey(_T("Bookmarks"));
    if (hBmk == NULL) {
        ::CloseHandle(hFile);
        return false;
    }

    // write utf-8 BOM
    DWORD nw;
    ::WriteFile(hFile, "\xef\xbb\xbf", 3, &nw, NULL);

    // enumerate files
    for (int i = 0;; ++i) {
        CString   name;
        DWORD     namelen = 1024;
        TCHAR     *namep = name.GetBuffer(namelen);
        CString   cls;
        DWORD     clslen = 1024;
        TCHAR     *clsp = cls.GetBuffer(clslen);
        FILETIME  wtime;
        DWORD     type;

        if (::RegEnumKeyEx(hBmk, i, namep, &namelen, NULL, clsp, &clslen, &wtime) != ERROR_SUCCESS)
            break;
        name.ReleaseBuffer(namelen);
        cls.ReleaseBuffer(clslen);

        // we have a file name at this point
        name = unescape(name);

        // enumerate bookmarks
        HKEY  hKey;
        if (::RegOpenKeyEx(hBmk, name, 0, HR_REG_PERM, &hKey) == ERROR_SUCCESS) {
            // get default settings
            clslen = 4096;
            clsp = cls.GetBuffer(clslen);
            clslen <<= 1;
            if (::RegQueryValueEx(hKey, NULL, NULL, &type, (LPBYTE)clsp, &clslen) == ERROR_SUCCESS && type == REG_SZ)
            {
                cls.ReleaseBuffer();

                // write file header
                CString tmp;
                tmp.Format(_T("-%s,%s\r\n"), (const TCHAR *)cls, (const TCHAR *)escape3(name));
                utf8write(hFile, tmp);

                for (int j = 0;; ++j) {
                    name.Format(_T("%d"), j);
                    clslen = 4096;
                    clsp = cls.GetBuffer(clslen);
                    clslen <<= 1;
                    if (::RegQueryValueEx(hKey, name, NULL, &type, (LPBYTE)clsp, &clslen) != ERROR_SUCCESS || type != REG_SZ)
                        break;
                    cls.ReleaseBuffer();
                    tmp.Format(_T(" %s\r\n"), (const TCHAR *)escape3(cls));
                    utf8write(hFile, tmp);
                }
            }

            ::RegCloseKey(hKey);
        }
    }

    // close file
    ::CloseHandle(hFile);
    ::RegCloseKey(hBmk);

    return true;
}