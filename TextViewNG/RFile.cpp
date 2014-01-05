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
* $Id: RFile.cpp,v 1.6.2.2 2004/07/07 12:04:47 mike Exp $
*
*/

#define _WIN32_WINNT	_WIN32_WINNT_MAXVER

#include <afxwin.h>

#include "ptr.h"
#include "RFile.h"
#include "TextViewNG.h"

int RFile::BSZ = 16384;
int RFile::BMASK = ~16383;

void  RFile::InitBufSize() {
    int	rfbs = CTVApp::GetInt(_T("FileBufSize"), 16384);
    if (rfbs < 8192)
        rfbs = 8192;
    if (rfbs > 1048576)
        rfbs = 1048576;
    int fbs = 8192;
    while ((fbs << 1) <= rfbs)
        fbs <<= 1;
    BSZ = fbs;
    BMASK = ~(fbs - 1);
}

CString	FileExceptionInfo(const CString& filename, DWORD dwErr) {
    CString ret;
    TCHAR	  *buf = ret.GetBuffer(1024);
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErr, LANG_USER_DEFAULT, buf, 1024, NULL);
    ret.ReleaseBuffer();
    return filename + _T(": ") + ret;
}

CString	FileName(const CString& file) {
    return file.Right(file.GetLength() - max(file.ReverseFind(_T('/')),
        file.ReverseFind(_T('\\'))) - 1);
}

void RFile::ShowError() {
    if (m_diderror)
        return;

    m_diderror = true;

    DWORD	dwErr = GetLastError();

    AfxMessageBox(FileExceptionInfo(m_fn, dwErr), MB_ICONERROR | MB_OK, 0);
}

bool  RFile::Reopen() {
    if (m_didreopen)
        return false;
    if (m_fh != INVALID_HANDLE_VALUE) {
        CloseHandle(m_fh);
        m_fh = INVALID_HANDLE_VALUE;
    }

    HANDLE fh = CreateFile(m_fn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (fh == INVALID_HANDLE_VALUE) {
        m_didreopen = true;
        return false;
    }
    m_fh = fh;
    SetFilePointer(fh, m_ptr, NULL, FILE_BEGIN);
    return true;
}

// to avoid dealing with exceptions we access the file handle directly
DWORD RFile::size() {
    DWORD ret = GetFileSize(m_fh, NULL);
    if (ret == 0xffffffff) {
        if (GetLastError() == 1617) {
            if (!Reopen()) {
                AfxMessageBox(_T("Can't reopen file."), MB_ICONERROR | MB_OK, 0);
                return 0;
            }
            ret = GetFileSize(m_fh, NULL);
            if (ret == 0xffffffff)
                goto err;
        }
        else {
        err:
            ShowError();
            ret = 0;
        }
    }
    return ret;
}
DWORD RFile::read(void *buf) { return read2(buf, BSZ); }
DWORD RFile::read2(void *buf, DWORD size) {
    DWORD	rd = 0;

    if (!ReadFile(m_fh, buf, size, &rd, NULL)) {
        if (GetLastError() == 1617) {
            if (!Reopen()) {
                AfxMessageBox(_T("Can't reopen file."), MB_ICONERROR | MB_OK, 0);
                return 0;
            }
            if (!ReadFile(m_fh, buf, size, &rd, NULL))
                ShowError();
        }
        else
            ShowError();
    }
    m_ptr += rd;
    return rd;
}

void  RFile::seek2(DWORD pos, DWORD how) {
    DWORD np;
    if ((np = SetFilePointer(m_fh, pos, NULL, how)) == 0xffffffff) {
        if (GetLastError() == 1617) {
            if (!Reopen()) {
                AfxMessageBox(_T("Can't reopen file."), MB_ICONERROR | MB_OK, 0);
                return;
            }
            if ((np = SetFilePointer(m_fh, pos, NULL, how)) == 0xffffffff)
                ShowError();
        }
        else
            ShowError();
    }
    if (np != 0xffffffff)
        m_ptr = np;
}
void  RFile::seek(DWORD pos) { seek2(pos, FILE_BEGIN); }

DWORD RFile::pos() {
    return m_ptr;
}