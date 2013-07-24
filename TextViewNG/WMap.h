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
 * $Id: WMap.h,v 1.2.2.1 2003/04/12 22:52:34 mike Exp $
 * 
 */

#ifndef WMAP_H
#define	WMAP_H

class WMap {
private:
  enum {
    STARTSIZE=128,
    FILLFACTOR=3
  };
  struct HE {
    HE		    *next;
    const wchar_t   *key;
    UINT	    hash;
    void	    *value;
  };

  int		m_cursize;
  int		m_curmask;
  int		m_numkeys;
  HE		**m_array;
  FastArray<HE>	m_hepool;
  StrBuf	m_strbuf;
  HANDLE	m_heap;
  bool		m_freemem;

  void		Extend(); // grow the array
  HE		*RealLookup(const wchar_t *key,bool add,bool copykey);
public:
  WMap(HANDLE heap,bool freemem=false);
  ~WMap();

  bool		Lookup(const wchar_t *key,void*& value);
  void		Add(const wchar_t *key,void *value) { RealLookup(key,true,false)->value=value; }
  void		AddCopy(const wchar_t *key,void *value) { RealLookup(key,true,true)->value=value; }

  void		RemoveAll();

  static unsigned int Hash(const wchar_t *data);
};

#endif
