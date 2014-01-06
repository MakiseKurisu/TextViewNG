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
* $Id: FastArray.cpp,v 1.4.2.1 2003/04/12 22:52:32 mike Exp $
*
*/

#define _WIN32_WINNT _WIN32_WINNT_MAXVER

#include <afx.h>

#include "FastArray.h"

#define START_CHUNKS 16
#define CHUNKS_INC 16

FastArrayImp::FastArrayImp(unsigned item_size, HANDLE heap, bool dofree) :
m_item_size(item_size), m_heap(heap), m_dofree(dofree)
{
    m_chunks = (char**)HeapAlloc(m_heap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, sizeof(char*)*START_CHUNKS);
    if (!m_chunks)
        AfxThrowMemoryException();
    m_chunks[0] = (char*)HeapAlloc(m_heap, HEAP_NO_SERIALIZE, m_item_size*CHUNK_ITEMS);
    if (!m_chunks[0]) {
        if (m_dofree)
            HeapFree(m_heap, HEAP_NO_SERIALIZE, (void*)m_chunks);
        AfxThrowMemoryException();
    }
    m_nchunks = START_CHUNKS;
    m_curchunk = m_chunkptr = 0;
}

FastArrayImp::~FastArrayImp() {
    if (m_dofree) {
        for (unsigned i = 0; i < m_nchunks; ++i)
            if (m_chunks[i])
                HeapFree(m_heap, HEAP_NO_SERIALIZE, (void*)m_chunks[i]);
        HeapFree(m_heap, HEAP_NO_SERIALIZE, (void*)m_chunks);
    }
}

void *FastArrayImp::SlowGet() {
    if (++m_curchunk >= m_nchunks) {
        m_nchunks += CHUNKS_INC;
        m_chunks = (char**)HeapReAlloc(m_heap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, (void*)m_chunks, sizeof(char*)*m_nchunks);
        if (!m_chunks)
            AfxThrowMemoryException();
    }
    if (!m_chunks[m_curchunk]) {
        m_chunks[m_curchunk] = (char*)HeapAlloc(m_heap, HEAP_NO_SERIALIZE, m_item_size*CHUNK_ITEMS);
        if (!m_chunks[m_curchunk])
            AfxThrowMemoryException();
    }
    m_chunkptr = 1;
    return (void*)m_chunks[m_curchunk];
}
