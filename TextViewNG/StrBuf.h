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
 * $Id: StrBuf.h,v 1.2.2.1 2003/04/12 22:52:33 mike Exp $
 * 
 */

#ifndef STRBUF_H
#define STRBUF_H

class StrBuf {
  enum {
    MAX_WASTE=256,
    BLOCKSADD=64
  };
  struct Block {
    wchar_t   *data;
    int	      cur;
    int	      max;
  };
  Block	      *m_blocks;
  int	      m_cblk;
  int	      m_numblk;
  HANDLE      m_heap;
  int	      m_blocksize;
  bool	      m_freemem;
public:
  StrBuf(HANDLE heap,bool release_mem=false,int bsz=4096) : m_blocks(0),
    m_cblk(-1), m_numblk(0), m_heap(heap), m_blocksize(bsz),
    m_freemem(release_mem) { }
  ~StrBuf();
  wchar_t   *Get(int char_length);
  wchar_t   *Append(const wchar_t *str,int char_length) {
    wchar_t   *space=Get(char_length);
    memcpy(space,str,char_length*sizeof(wchar_t));
    return space;
  }
  void	    RemoveAll();
};

#endif
