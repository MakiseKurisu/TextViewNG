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
* $Id: BufFile.h,v 1.19.2.1 2003/04/12 22:52:32 mike Exp $
*
*/

#if !defined(AFX_BUFFILE_H__867464A4_888C_4590_A2F7_7126AC072CB6__INCLUDED_)
#define AFX_BUFFILE_H__867464A4_888C_4590_A2F7_7126AC072CB6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "RFile.h"

#define	BEOF	(-1)

class CBufFile
{
public:
    CBufFile(auto_ptr<RFile> file);
    ~CBufFile() { }

    int	    ch() { return m_ptr < m_cur->len ? m_cur->buf[m_ptr++] : nextbuf_ch(); }
    int	    read(void *buf, int count); // read plain chars

    DWORD	    pos() { return m_cur->off + m_ptr; }
    void	    seek(DWORD pos);
    DWORD	    size() { return m_fp->size(); }
    DWORD	    prevpos() { return m_cur->off + m_ptr - 1; }

    CString   CompressionInfo() { return m_fp->CompressionInfo(); }

protected:
    int		  nextbuf_ch();
    void		  swapbuf();

    struct Buf {
        Buffer<BYTE>  buf;
        DWORD	  off;
        DWORD	  len;
        Buf() : buf(RFile::BSZ), off(0), len(0) { }
    };

    auto_ptr<RFile> m_fp;

    DWORD		  m_ptr;
    Buf		  m_b1, m_b2;
    Buf		  *m_cur;
};

#endif // !defined(AFX_BUFFILE_H__867464A4_888C_4590_A2F7_7126AC072CB6__INCLUDED_)
