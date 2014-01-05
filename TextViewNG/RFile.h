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
* $Id: RFile.h,v 1.11.2.4 2005/06/18 16:29:27 mike Exp $
*
*/

#if !defined(AFX_RFILE_H__3965961A_8172_4D80_990B_CC46A6EE9D62__INCLUDED_)
#define AFX_RFILE_H__3965961A_8172_4D80_990B_CC46A6EE9D62__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// block oriented file i/o
class RFile
{
public:
    static int  BSZ, BMASK; // block size, must be a power of two

    RFile(const CString& filename) : m_fh(INVALID_HANDLE_VALUE),
        m_fn(filename), m_didreopen(false), m_diderror(false),
        m_ptr(0) { }
    virtual ~RFile() {
        if (m_fh != INVALID_HANDLE_VALUE)
            CloseHandle(m_fh);
    }

    // generic file operations
    virtual DWORD	  size();
    virtual DWORD	  read(void *buf);
    virtual void	  seek(DWORD pos);

    // compression
    virtual CString CompressionInfo() { return _T("None"); }

    // buffer size setting
    static void	  InitBufSize();

    // RFile helpers
    bool		  Reopen();
    void		  ShowError();
    DWORD		  read2(void *buf, DWORD size);

protected:
    HANDLE	  m_fh;
    CString	  m_fn;
    DWORD		  m_ptr;

    bool		  m_didreopen;
    bool		  m_diderror;

    void		  seek2(DWORD where, DWORD how);
    DWORD		  pos();
};

// I/O error messages
CString	FileExceptionInfo(const CString& filename, DWORD dwError);
CString FileName(const CString& file);

#endif // !defined(AFX_RFILE_H__3965961A_8172_4D80_990B_CC46A6EE9D62__INCLUDED_)
