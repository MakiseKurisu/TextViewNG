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
 * $Id: Dictionary.cpp,v 1.23.2.4 2007/03/24 23:03:39 mike Exp $
 * 
 */

#include <afx.h>
#include <afxtempl.h>

#include "ptr.h"
#include "zlib.h"
#include "Unicode.h"
#include "RFile.h"
#include "TextParser.h"
#include "Dictionary.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

class Dict : public IDict
{
public:
  Dict(RFile *fp);
  ~Dict() { }

  int	  NumWords() { return m_numwords; }
  Buffer<wchar_t> GetWordW(int index);
  bool	  Find(const wchar_t *word,int& index,int& found);
  bool	  FindImp(const wchar_t *word,int& index,int *found);
  bool	  Valid() { return m_ok; }
  int	  GetStartPofWord(int index);
  int	  GetWordFromP(int para);
  int	  GetNumP() { return m_numpara; }
protected:
  struct Block {
    Buffer<char>  key;	    /* key */
    int		wordidx;    /* first word index */
    int		numwords;   /* number of words in this block */
    int		size;	    /* uncompressed block size */
    int		csize;	    /* compressed size */
    int		off;	    /* offset in the file */
    int		numpara;    /* number of paragraphs in this block */
    int		paraidx;    /* starting paragraph */
  };

  auto_ptr<RFile>	m_rf;
  DWORD			m_numblk;
  DWORD			m_numwords;
  int			m_curblk;
  CPtrArray		m_windex; // words in current block
  CPtrArray		m_kindex; // keys
  CUIntArray		m_pindex; // paragraphs
  Buffer<char>		m_buffer;
  CArray<Block,Block&>	m_blocks;
  bool			m_ok;
  LCID			m_lcid;
  UINT			m_ms_codepage;
  int			m_codepage;
  int			m_numpara;

  bool	      GetBlk(int num);
  const char  *GetWordImp(int index);
  bool	      OpenOld();
  bool	      OpenNew();
};

static DWORD    getdword(RFile *fp) {
  BYTE	  b[4];
  int	  rd=fp->read2(b,4);
  if (rd!=4)
    return 0;
  return ((DWORD)b[3]<<24)|((DWORD)b[2]<<16)|((DWORD)b[1])<<8|b[0];
}

bool Dict::OpenNew() {
  if ((m_lcid=getdword(m_rf.get()))==0)
    return false;
  if (!IsValidLocale(m_lcid,LCID_INSTALLED))
    m_lcid=GetUserDefaultLCID();
  if ((m_numwords=getdword(m_rf.get()))==0)
    return false;
  if ((m_ms_codepage=getdword(m_rf.get()))==0)
    return false;
  if ((m_codepage=Unicode::GetIntCodePage(m_ms_codepage))<0)
    m_codepage=Unicode::DefaultCodePage();
  DWORD	btab=getdword(m_rf.get());
  if (btab==0)
    return false;
  m_rf->seek(btab);
  if ((m_numblk=getdword(m_rf.get()))==0 || m_numblk>m_numwords)
    return false;
  DWORD idx,i,off,maxblock,paraidx;
  for (i=idx=paraidx=maxblock=0,off=20;i<(int)m_numblk;++i) {
    Block   blk;
    int	    keylen;
    if ((blk.size=getdword(m_rf.get()))==0)
      return false;
    if ((blk.csize=getdword(m_rf.get()))==0)
      return false;
    if ((keylen=getdword(m_rf.get()))==0)
      return false;
    if ((blk.numwords=getdword(m_rf.get()))==0)
      return false;
    if ((blk.numpara=getdword(m_rf.get()))==0)
      return false;
    blk.numpara+=blk.numwords; // append and empty line after each word
    blk.wordidx=idx;
    blk.paraidx=paraidx;
    blk.off=off;
    idx+=blk.numwords;
    paraidx+=blk.numpara;
    off+=blk.csize;
    blk.key=Buffer<char>(keylen);
    m_blocks.Add(blk);
    if (blk.size>(int)maxblock)
      maxblock=blk.size;
  }
  if (idx!=m_numwords)
    return false;
  m_numpara=paraidx;
  for (i=0;i<(int)m_numblk;++i)
    if ((m_rf->read2(m_blocks[i].key,m_blocks[i].key.size()))!=(DWORD)m_blocks[i].key.size())
      return false;
  m_buffer=Buffer<char>(maxblock);
  return true;
}

Dict::Dict(RFile *fp) :
  m_ok(false), m_numwords(0), m_numpara(0), m_numblk(0), m_curblk(-1), m_rf(fp)
{
  if (!OpenNew())
      goto fail;
  m_ok=true;
  return;
fail:
  m_buffer=Buffer<char>();
  m_blocks.RemoveAll();
}

bool  Dict::GetBlk(int num) {
  if (num<0 || num>=(int)m_numblk)
    return false;
  if (m_curblk==num)
    return true;
  m_curblk=-1;
  m_rf->seek(m_blocks[num].off);
  if (m_blocks[num].size==m_blocks[num].csize) { // uncompressed
    if (m_blocks[num].size!=(int)m_rf->read2(m_buffer,m_blocks[num].size))
      return false;
  } else {
    Buffer<unsigned char>    in(m_blocks[num].csize);
    if (m_blocks[num].csize!=(int)m_rf->read2(in,m_blocks[num].csize))
      return false;
    uLongf  len=m_blocks[num].size;
    int ret=uncompress((unsigned char *)(char *)m_buffer,&len,
      in,m_blocks[num].csize);
    if (ret!=Z_OK || (int)len!=m_blocks[num].size)
      return false;
  }
  char	  *p=m_buffer;
  char	  *e=p+m_blocks[num].size;
  int	  i,pnum;
  m_windex.SetSize(m_blocks[num].numwords);
  m_kindex.SetSize(m_blocks[num].numwords);
  m_pindex.SetSize(m_blocks[num].numwords+1);
  for (i=pnum=0;i<m_blocks[num].numwords && p<e;++i) {
    m_kindex[i]=p;
    m_pindex[i]=pnum;
    while (p<e && *p)
      ++p;
    if (p<e)
      ++p;
    m_windex[i]=p;
    while (p<e && *p) {
      if (*p=='\n')
	++pnum;
      ++p;
    }
    if (p<e)
      ++p;
    pnum+=2; // implicit empty line after each word
  }
  if (i!=m_blocks[num].numwords || pnum!=m_blocks[num].numpara)
    return false;
  m_curblk=num;
  m_pindex[m_blocks[num].numwords]=m_blocks[num].numpara;
  return true;
}

Buffer<wchar_t> Dict::GetWordW(int index) {
  const char  *word=GetWordImp(index);
  if (!word)
    return Buffer<wchar_t>();
  return Unicode::ToWCbuf(m_codepage,word,strlen(word));
}

CString	  IDict::GetWord(int index) {
  CString	    ret(Unicode::ToCS(GetWordW(index)));
  ret.Replace(_T("\n"),_T("\r\n"));
  return ret;
}

const char *Dict::GetWordImp(int index) {
  if (index<0 || index>=(int)m_numwords)
    return NULL;
  if (m_curblk<0 || index<m_blocks[m_curblk].wordidx ||
      index>=m_blocks[m_curblk].wordidx+m_blocks[m_curblk].numwords)
  {
    int	low=0;
    int	high=m_numblk-1;
    int	mid;
    for (int ni=0;;++ni) {
      if (ni>(int)m_numblk) // prevent loops on unsorted invalid data
	return NULL;
      if (low>high)
	return NULL;
      mid=(low+high)>>1;
      if (index<m_blocks[mid].wordidx)
	high=mid-1;
      else if (index>=m_blocks[mid].wordidx+m_blocks[mid].numwords)
	low=mid+1;
      else
	break;
    }
    if (!GetBlk(mid))
      return NULL;
  }
  return (const char *)m_windex[index-m_blocks[m_curblk].wordidx];
}

static int    compare_buf_str(Buffer<char>& b1,const char *b2,int l2=-1) {
  if (l2<0)
    l2=strlen(b2);
  int	res=memcmp(b1,b2,min(b1.size(),l2));
  if (res==0)
    res=b1.size()<l2 ? -1 : b1.size()>l2 ? 1 : 0;
  return res;
}

static int    compare_buf_str_len(Buffer<char>& b1,const char *b2,int l2=-1) {
  if (l2<0)
    l2=strlen(b2);
  if (l2>b1.size())
    l2=b1.size();
  const char *p=b1;
  while (l2-->0 && *p++==*b2++) ;
  return p-b1;
}

static inline int    compare_bufs(Buffer<char>& b1,Buffer<char>& b2) {
  return compare_buf_str(b1,b2,b2.size());
}

bool	Dict::FindImp(const wchar_t *word,int& index,int *found) {
  Buffer<char>	  sortkey(Unicode::SortKey(m_lcid,word));
  int	  low=0;
  int	  high=m_numblk-1;
  int	  mid;
  for (int ni=0;;++ni) {
    if (ni>(int)m_numblk) // prevent loops on unsorted data
      return false;
    if (low>high) {
      if (low==0) {
	index=0;
	if (found) {
	  *found=0;
	  return true;
	}
      }
      return false;
    }
    mid=(low+high)>>1;
    int cmp=compare_bufs(sortkey,m_blocks[mid].key);
    if (cmp<0)
      high=mid-1;
    else {
      if (mid==(int)m_numblk-1) // last block, stop search
	break;
      cmp=compare_bufs(sortkey,m_blocks[mid+1].key);
      if (cmp<0) // found it
	break;
      low=mid+1;
    }
  }
  int	blk=mid;
  if (!GetBlk(blk))
    return false;
  low=0;
  high=m_blocks[blk].numwords-1;
  for (int nj=0;;++nj) {
    if (nj>m_blocks[blk].numwords) // prevent loops on invalid data
      return false;
    if (low>high) { // no such word in this block
      if (!found) // shortcut
	return false;
      int   idx=m_blocks[blk].wordidx+high;
      if (idx<0)
	idx=0;
      if (idx<(int)m_numwords-1) {
	if (!GetWordImp(idx))
	  return false;
	int cur=idx-m_blocks[m_curblk].wordidx;
	int l1=compare_buf_str_len(sortkey,(const char *)m_kindex[cur]);
	if (++cur>=m_blocks[m_curblk].numwords) {
	  if (!GetWordImp(idx+1))
	    return false;
	  cur=0;
	}
	if (compare_buf_str_len(sortkey,(const char *)m_kindex[cur])>l1)
	  ++idx;
      }
      index=idx;
      *found=2;
      return true;
    }
    mid=(low+high)>>1;
    int cmp=compare_buf_str(sortkey,(const char *)m_kindex[mid]);
    if (cmp<0)
      high=mid-1;
    else if (cmp>0)
      low=mid+1;
    else { // found
      index=m_blocks[blk].wordidx+mid;
      if (found)
	*found=1;
      return true;
    }
  }
}

static struct {
  const wchar_t	  *pattern;
  const wchar_t	  *replacement;
} endings[]={
  { L"s",	NULL  },
  { L"se",	NULL  },
  { L"sei",	L"y"  },
  { L"de",	NULL  },
  { L"de",	L"e"  },
  { L"dei",	L"y"  },
  { L"de.",	L"1"  },
  { L"gni",	NULL  },
  { L"gni",	L"e"  },
  { L"gniy",	L"ie" },
  { L"gni.",	L"1"  },
  { L"re",	NULL  },
  { L"re",	L"e"  },
  { L"rei",	L"y"  },
  { L"re."	L"1"  },
  { L"tse",	NULL  },
  { L"tse",	L"e"  },
  { L"tsei",	L"y"  },
  { L"tse.",	L"1"  },
  { NULL,	NULL  }
};

bool  Dict::Find(const wchar_t *word,int& index,int& found) {
  if (PRIMARYLANGID(m_lcid)!=LANG_ENGLISH)
    return FindImp(word,index,&found);
  // handle english word endings
  wchar_t	tmpbuf[64];
  int		wordlen=wcslen(word);
  if (wordlen>=sizeof(tmpbuf)/sizeof(tmpbuf[0]))
    return FindImp(word,index,&found);
  // check exact match
  if (FindImp(word,index,NULL)) {
    found=1;
    return true;
  }
  for (int ending=0;endings[ending].pattern;++ending) {
    const wchar_t   *pattern=endings[ending].pattern;
    for (int i=0;i<wordlen;++i) {
      if (pattern[i]==L'.') { // doubled char here
	if (i+1<wordlen && word[wordlen-i-1]==word[wordlen-i-2]) {
	  i+=2;
	  goto match;
	}
	break;
      }
      if (!pattern[i]) { // end of pattern, matched
match:
	int   k;
	for (k=0;k<wordlen-i;++k)
	  tmpbuf[k]=word[k];
	const wchar_t	*replacement=endings[ending].replacement;
	if (replacement) {
	  if (replacement[0]==L'1')
	    tmpbuf[k++]=word[wordlen-i];
	  else
	    while (*replacement)
	      tmpbuf[k++]=*replacement++;
	}
	tmpbuf[k++]=L'\0';
	// relookup
	if (FindImp(tmpbuf,index,NULL)) {
	  found=1;
	  return true;
	}
	break;
      }
      if (pattern[i]!=word[wordlen-i-1])
	break;
    }
  }
  // still no match, proceed as usual
  return FindImp(word,index,&found);
}

int   Dict::GetWordFromP(int para) {
  if (para<0 || para>=m_numpara)
    return 0;
  int	low=0;
  int	high=m_numblk-1;
  int	mid;
  int   ni;
  for (ni=0;;++ni) {
    if (ni>(int)m_numblk) // prevent loops on unsorted invalid data
      return 0;
    if (low>high)
      return 0;
    mid=(low+high)>>1;
    if (para<m_blocks[mid].paraidx)
      high=mid-1;
    else if (para>=m_blocks[mid].paraidx+m_blocks[mid].numpara)
      low=mid+1;
    else
      break;
  }
  if (!GetBlk(mid))
    return 0;
  low=0;
  high=m_blocks[m_curblk].numwords;
  para-=m_blocks[m_curblk].paraidx;
  for (ni=0;;++ni) {
    if (ni>m_blocks[m_curblk].numwords)
      return 0;
    if (low>high)
      return 0;
    mid=(low+high)>>1;
    if (para<(int)m_pindex[mid])
      high=mid-1;
    else if (para>=(int)m_pindex[mid+1])
      low=mid+1;
    else
      break;
  }
  return m_blocks[m_curblk].wordidx+mid;
}

int   Dict::GetStartPofWord(int index) {
  if (GetWordImp(index))
    return m_blocks[m_curblk].paraidx+m_pindex[index-m_blocks[m_curblk].wordidx];
  return 0;
}

static class SimpleDictInit: public IDict::DictInit {
  static IDict *create_simple_dict(RFile *fp) { return new Dict(fp); }
public:
  SimpleDictInit() : DictInit("DICq",create_simple_dict) { }
} g_dict_init;

// create a proper dictionary
IDict *IDict::Create(const CString& filename,CString *errmsg) {
  auto_ptr<RFile>	  fp(new RFile(filename));

  if (!fp->Reopen()) {
    if (errmsg)
      *errmsg=FileExceptionInfo(filename,GetLastError());
    return NULL;
  }

  BYTE	  sig[SIGSIZE];
  if (fp->read2(sig,SIGSIZE)!=SIGSIZE) {
    if (errmsg)
      errmsg->Format(_T("%s: Ivalid dictionary file"),(const TCHAR*)filename);
    return NULL;
  }
  IDict *d=NULL;
  for (DictInit	*di=DictInit::m_head;di;di=di->m_next)
    if (memcmp(sig,di->m_sig,SIGSIZE)==0) {
      d=di->m_create(fp.release());
      break;
    }
  if (d && d->Valid())
    return d;
  delete d;
  if (errmsg)
    errmsg->Format(_T("%s: Ivalid dictionary file"),(const TCHAR*)filename);
  return NULL;
}

IDict::DictInit	  *IDict::DictInit::m_head;

IDict::DictInit::DictInit(const char *sig,IDict *(*create)(RFile *)) {
  m_next=m_head;
  m_create=create;
  strncpy(m_sig,sig,sizeof(m_sig));
  m_head=this;
}

Buffer<wchar_t>	DictParser::GetParagraphImp(int para,bool& isfirst) {
  int		    ws=m_dict->GetWordFromP(para);
  Buffer<wchar_t>   word(m_dict->GetWordW(ws));
  int		    ps=m_dict->GetStartPofWord(ws);

  isfirst=ps==para;
  const wchar_t	    *wp=word,*we=wp+word.size();
  while (ps!=para && wp<we) {
    while (wp<we && *wp!=L'\n')
      ++wp;
    if (wp<we)
      ++wp;
    ++ps;
  }
  const wchar_t	    *wq=wp;
  while (wq<we && *wq!=L'\n')
    ++wq;
  return Buffer<wchar_t>(wp,wq-wp);
}

Paragraph DictParser::GetParagraph(int docid,int para) {
  bool isfirst;
  Buffer<wchar_t>   txt(GetParagraphImp(para,isfirst));
  Paragraph	p(txt.size());
  p.str=txt;
  for (int j=0;j<p.str.size() && p.str[j]==L' ';++j)
    p.str[j]=0xA0; // replace leading spaces with nbsp
  p.cflags.Zero();
  if (isfirst)
    for (int i=0;i<p.str.size()-1;++i) {
      if (p.str[i]==L' ' && p.str[i+1]==L' ')
	break;
      p.cflags[i].bold=1;
    }

  return p;
}

int   DictParser::GetPLength(int docid,int para) {
  bool isfirst;
  return GetParagraphImp(para,isfirst).size();
}

DictParser  *DictParser::OpenDict(const CString& path,CString *errmsg) {
  IDict	  *d=IDict::Create(path,errmsg);
  if (!d)
    return NULL;
  return new DictParser(auto_ptr<IDict>(d));
}

bool  DictParser::LookupReference(const wchar_t *rname,FilePos& dest) {
  int	  index,found;
  if (m_dict->Find(rname,index,found)) {
    dest.docid=0;
    dest.para=m_dict->GetStartPofWord(index);
    dest.off=0;
    return true;
  }
  return false;
}
