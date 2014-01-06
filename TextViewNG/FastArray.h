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
* $Id: FastArray.h,v 1.4.2.1 2003/04/12 22:52:32 mike Exp $
*
*/

#ifndef FASTARRAY_H
#define FASTARRAY_H

class FastArrayImp {
protected:
 HANDLE m_heap;
 unsigned m_item_size;
 char **m_chunks;
 unsigned m_nchunks;
 enum {
 CHUNK_BITS = 10, // 1<<10 items per chunk
 CHUNK_ITEMS = 1 << CHUNK_BITS,
 CHUNK_MASK = CHUNK_ITEMS - 1
 };
 unsigned m_curchunk;
 unsigned m_chunkptr;
 bool m_dofree;

 FastArrayImp(unsigned item_size, HANDLE heap, bool dofree);
 ~FastArrayImp();
 void *Get() {
 if (m_chunkptr < CHUNK_ITEMS)
 return m_chunks[m_curchunk] + m_item_size*m_chunkptr++;
 return SlowGet();
 }
 int Size() { return (m_curchunk << CHUNK_BITS) + m_chunkptr; }
 void *Item(unsigned ii) {
 return m_chunks[ii >> CHUNK_BITS] + m_item_size*(ii&CHUNK_MASK);
 }
 void *SlowGet();
 void RemoveAll() { m_chunkptr = m_curchunk = 0; }
};

// WARNING: this is intended for use with structures _without_ constructors and
// destructors
template<class T>
class FastArray : public FastArrayImp {
public:
 FastArray(HANDLE heap, bool dofree = false) : FastArrayImp(sizeof(T), heap, dofree) { }

 T *Get() { return (T*)FastArrayImp::Get(); }
 T& operator [](int ii) { return *(T*)FastArrayImp::Item(ii); }
 int GetSize() { return Size(); }
 void Add(T& tt) { memcpy(Get(), &tt, sizeof(T)); }
 // does _not_ release memory
 void RemoveAll() { FastArrayImp::RemoveAll(); }
};

#endif
