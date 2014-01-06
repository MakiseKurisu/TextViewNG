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
* $Id: BufFile.cpp,v 1.24.2.1 2003/04/12 22:52:32 mike Exp $
*
*/

#define _WIN32_WINNT _WIN32_WINNT_MAXVER

#include <afx.h>

#include "ptr.h"
#include "BufFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

CBufFile::CBufFile(auto_ptr<RFile> file) :
m_fp(file),
m_ptr(0)
{
    m_cur = &m_b1;
}

void CBufFile::swapbuf()
{
    if (m_cur->len)
        m_cur = m_cur == &m_b1 ? &m_b2 : &m_b1;
}

int CBufFile::nextbuf_ch()
{
    seek(m_cur->off + m_ptr);
    return m_ptr < m_cur->len ? m_cur->buf[m_ptr++] : BEOF;
}

int CBufFile::read(void *buf, int count)
{
    BYTE *bp = (BYTE*) buf;
    while (count > 0)
    {
        // fill in our buffer
        if (m_ptr >= m_cur->len)
            seek(m_cur->off + m_ptr);
        // if there are no bytes still, then exit
        if (m_ptr >= m_cur->len)
            break;
        // copy whatever is left in our buffer
        int nb = m_cur->len - m_ptr;
        if (nb > count)
            nb = count;
        memcpy(bp, m_cur->buf + m_ptr, nb);
        m_ptr += nb;
        bp += nb;
        count -= nb;
    }
    return bp - (BYTE*) buf;
}

void CBufFile::seek(DWORD pos)
{
    if (pos >= m_cur->off && pos < m_cur->off + m_cur->len) // inside current buffer
        m_ptr = pos - m_cur->off;
    else
    {
        swapbuf();
        if (pos >= m_cur->off && pos < m_cur->off + m_cur->len) // inside other buffer
            m_ptr = pos - m_cur->off;
        else if (pos >= m_fp->size())
        {
            // don't seek past eof
            m_cur->off = pos;
            m_cur->len = m_ptr = 0;
        }
        else
        {
            // do seek on underlying file and fill next buffer
            m_cur->off = pos&RFile::BMASK;
            m_fp->seek(m_cur->off);
            m_cur->len = m_fp->read(m_cur->buf);
            m_ptr = pos - m_cur->off;
        }
    }
}

