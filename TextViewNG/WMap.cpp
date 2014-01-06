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
* $Id: WMap.cpp,v 1.2.2.1 2003/04/12 22:52:34 mike Exp $
*
*/

#define _WIN32_WINNT _WIN32_WINNT_MAXVER

#include <afx.h>

#include "FastArray.h"
#include "StrBuf.h"
#include "WMap.h"

WMap::WMap(HANDLE heap, bool freemem) : m_hepool(heap, freemem),
m_strbuf(heap, freemem), m_heap(heap), m_freemem(freemem),
m_cursize(STARTSIZE), m_curmask(STARTSIZE - 1), m_numkeys(0)
{
    m_array = (HE**)HeapAlloc(m_heap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY,
        m_cursize*sizeof(HE*));
    if (!m_array)
        AfxThrowMemoryException();
}

WMap::~WMap() {
    if (m_freemem)
        HeapFree(m_heap, HEAP_NO_SERIALIZE, m_array);
}

// Hash algorithm is One-at-a-Time by Bob Jenkins,
// http://burtleburtle.net/bob/hash/evahash.html

// microsoft's arm compiler from evt3 really sucks here, because it does
// not realize that x+x*2^n can be very efficientlly computed
// in one instruction using builtin shifter on ARM cpus, it
// generates three instructions instead, like this pseudocode:
//   tmp=2^n
//   tmp|=1
//   x*tmp
unsigned int WMap::Hash(const wchar_t *data) {
    unsigned int hash;
    if (!*data) // shortcut
        return 0;
    hash = 0;
    do {
        hash += *data++;
        hash += hash << 10;
        hash ^= hash >> 6;
    } while (*data);
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

WMap::HE  *WMap::RealLookup(const wchar_t *key, bool add, bool copykey) {
    UINT   hash = Hash(key);
    UINT   bucket = hash&m_curmask;
    HE   *he;

    for (he = m_array[bucket]; he; he = he->next)
        if (!wcscmp(key, he->key))
            return he;
    if (!add)
        return NULL;
    he = m_hepool.Get();
    he->key = copykey ? m_strbuf.Append(key, wcslen(key) + 1) : key;
    he->hash = hash;
    he->next = m_array[bucket];
    m_array[bucket] = he;
    if (++m_numkeys*FILLFACTOR > m_cursize)
        Extend();
    return he;
}

bool WMap::Lookup(const wchar_t *key, void*& value) {
    HE *he = RealLookup(key, false, false);
    if (he) {
        value = he->value;
        return true;
    }
    return false;
}

void WMap::RemoveAll() {
    m_hepool.RemoveAll();
    m_strbuf.RemoveAll();
    m_numkeys = 0;
}

void WMap::Extend() {
    int newsize = m_cursize << 1;
    HE **na = (HE**)HeapReAlloc(m_heap, HEAP_NO_SERIALIZE, m_array, newsize*sizeof(HE*));
    if (!na)
        AfxThrowMemoryException();
    memset(na + m_cursize, 0, m_cursize*sizeof(HE**));
    for (int i = 0; i < m_cursize; ++i) {
        HE   **prev = &na[i], *cur = na[i];
        while (cur) {
            if (cur->hash & m_cursize) {
                *prev = cur->next;
                cur->next = na[i + m_cursize];
                na[i + m_cursize] = cur;
                cur = *prev;
            }
            else {
                prev = &cur->next;
                cur = cur->next;
            }
        }
    }
    m_cursize = newsize;
    m_curmask = m_cursize - 1;
    m_array = na;
}
