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
 * $Id: StrBuf.cpp,v 1.2.2.1 2003/04/12 22:52:33 mike Exp $
 * 
 */

#include <afx.h>
#include "StrBuf.h"

StrBuf::~StrBuf() {
  if (m_freemem)
    RemoveAll();
}

void  StrBuf::RemoveAll() {
  for (int i=0;i<=m_cblk;++i)
    HeapFree(m_heap,HEAP_NO_SERIALIZE,m_blocks[i].data);
  if (m_numblk>0)
    HeapFree(m_heap,HEAP_NO_SERIALIZE,m_blocks);
  m_numblk=0;
  m_cblk=-1;
}

wchar_t	    *StrBuf::Get(int char_length) {
  if (m_numblk==0 || m_blocks[m_cblk].cur+char_length>m_blocks[m_cblk].max) {
    // shrink the block if too much is wasted
    if (m_numblk!=0 && m_blocks[m_cblk].cur+MAX_WASTE<m_blocks[m_cblk].max &&
	HeapReAlloc(m_heap,HEAP_NO_SERIALIZE|HEAP_REALLOC_IN_PLACE_ONLY,
		  m_blocks[m_cblk].data,m_blocks[m_cblk].cur))
      m_blocks[m_cblk].max=m_blocks[m_cblk].cur;
    // allocate a new blocks list if needed
    if (m_cblk+1>=m_numblk) {
      int newblk=m_numblk+BLOCKSADD;
      void  *mem=m_blocks ? HeapReAlloc(m_heap,HEAP_NO_SERIALIZE,m_blocks,newblk*sizeof(Block)) :
			    HeapAlloc(m_heap,HEAP_NO_SERIALIZE,newblk*sizeof(Block));
      if (mem==NULL)
	AfxThrowMemoryException();
      m_blocks=(Block*)mem;
      m_numblk=newblk;
    }
    // allocate a new block
    m_blocks[m_cblk+1].max=char_length>m_blocksize ? char_length : m_blocksize ;
    m_blocks[m_cblk+1].data=(wchar_t*)HeapAlloc(m_heap,HEAP_NO_SERIALIZE,
						m_blocks[m_cblk+1].max*sizeof(wchar_t));
    if (m_blocks[m_cblk+1].data==NULL)
      AfxThrowMemoryException();
    ++m_cblk;
    m_blocks[m_cblk].cur=0;
  }
  wchar_t   *ret=m_blocks[m_cblk].data+m_blocks[m_cblk].cur;
  m_blocks[m_cblk].cur+=char_length;
  return ret;
}
