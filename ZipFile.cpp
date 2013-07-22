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
 * $Id: ZipFile.cpp,v 1.17.2.8 2004/07/07 12:04:47 mike Exp $
 * 
 */
#include <afx.h>
#include <afxwin.h>

#include "ptr.h"
#include "ZipFile.h"
#include "BufFile.h"
#include "TextViewNG.h"
#include "Unicode.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ZipFile::ZipFile(const CString& fn) :
  RFile(fn), m_zs(NULL), m_curfile(NULL), m_curpos(NULL)
{
  m_rootdir.off=-1;
  m_rootdir.children=new CMapStringToPtr;
  m_rootdir.parent=NULL;
  m_curdir=&m_rootdir;
  Reopen();
}

ZipFile::~ZipFile() {
  if (m_zs) {
    inflateEnd(m_zs);
    delete m_zs;
  }
}

ZipFile::ZFile::~ZFile() {
  if (isdir()) { // directory
    CString   key;
    for (POSITION p=children->GetStartPosition();p!=NULL;) {
      void *ptr;
      children->GetNextAssoc(p,key,ptr);
      delete (ZFile*)ptr;
    }
    delete children;
  }
}

void	ZipFile::rewind() {
  if (!m_curfile)
    return;
  if (m_ptr==0)
    return;
  RFile::seek(m_curfile->off);
  if (m_curfile->iscmp()) {
    inflateReset(m_zs);
    m_zs->avail_in=0;
  }
  m_ptr=0;
}

void   ZipFile::seek(DWORD pos) {
  if (!m_curfile)
    return;
  pos&=BMASK;
  if (pos>=(DWORD)m_curfile->size) // special case for eof
    m_ptr=m_curfile->size;
  else if (pos==0) // start of file
    rewind();
  else if (m_curfile->iscmp()) { // compressed!
    if (pos!=m_ptr) {// do something
      if (pos<m_ptr) // before current position
	rewind();
      Buffer<char>  tmp(BSZ);
      while (pos>m_ptr && read(tmp)==(unsigned)BSZ) ;
    }
  } else {
    m_ptr=pos;
    RFile::seek(m_curfile->off+m_ptr);
  }
}

DWORD   ZipFile::read(void *buf) {
  if (!m_curfile || m_ptr>=(DWORD)m_curfile->size) // invalid file or at eof
    return 0;
  if (!m_curfile->iscmp()) {// uncompressed
    DWORD nr=RFile::read2(buf,min(BSZ,m_curfile->size));
    m_ptr+=nr;
    return nr;
  }
  m_zs->avail_out=BSZ;
  m_zs->next_out=(Bytef*)buf;
  int	r;
  while (m_zs->avail_out>0) {
    if (m_zs->avail_in==0) { // fill in next buffer
      m_zs->avail_in=RFile::read2(m_in,BSZ);
      m_zs->next_in=m_in;
    }
    r=inflate(m_zs,Z_SYNC_FLUSH);
    if (r!=Z_OK)
      break;
  }
  m_ptr+=BSZ-m_zs->avail_out;
  return BSZ-m_zs->avail_out;
}

DWORD   ZipFile::size() {
  if (!m_curfile)
    return 0;
  return m_curfile->size;
}

// compression
CString ZipFile::CompressionInfo() {
  if (!m_curfile)
    return _T("No open file");
  if (m_curfile->iscmp() && m_curfile->size>0) {
    CString ret;
    ret.Format(_T("ZIP, deflated (%.2f)"),
      (double)m_curfile->csize/(double)m_curfile->size);
    return ret;
  }
  return _T("ZIP, stored");
}

// zip file specific methods
bool  ZipFile::SetDir(const CString& dir) {
  if (!dir.GetLength())
    return false;
  if (dir==_T("..")) { // up one level
    if (m_curdir->parent) // good
      m_curdir=m_curdir->parent;
    else
      return false;
    return true;
  }
  int	    idx=dir[0]==_T('\\');
  ZFile	    *cur=idx?&m_rootdir:m_curdir;
  while (idx<dir.GetLength()) {
    // find next element
    int	    spos=dir.Find(_T('\\'),idx);
    CString elem;
    if (spos<0) { // last element
      elem=dir.Mid(idx);
      idx=dir.GetLength();
    } else {
      elem=dir.Mid(idx,spos-idx);
      idx=spos+1;
    }
    void    *ptr;
    if (cur->children->Lookup(elem,ptr)) { // ok, found it
      cur=(ZFile*)ptr;
      if (cur->off>=0) // not a directory
	return false;
    } else // dir not found
      return false;;
  }
  m_curdir=cur;
  m_curpos=NULL;
  return true;
}

bool  ZipFile::GetNextFileInfo(CString& fname,bool& isdir,__int64& size) {
  if (m_curpos==NULL)
    return false;
  void	*ptr;
  m_curdir->children->GetNextAssoc(m_curpos,fname,ptr);
  isdir=((ZFile*)ptr)->isdir();
  size=((ZFile*)ptr)->size;
  return true;
}

void  ZipFile::Reset() {
  m_curpos=m_curdir->children->GetStartPosition();
}

#define	LOCALSIG    0x04034b50
#define	DIRSIG	    0x02014b50

#pragma pack(push)
#pragma pack(1)
// zip file local header
struct LocalZipHdr {
  DWORD	    sig;	      // signature
  WORD	    extver;	      // version needed to extract
  WORD	    flags;	      // general purpose flag
  WORD	    compmeth;	      // compression method
  WORD	    modtime;	      // last modification time
  WORD	    moddate;	      // last modification date
  DWORD	    crc32;
  DWORD	    compsize;	      // compressed size
  DWORD	    usize;	      // uncompressed size
  WORD	    namelength;	      // filename length
  WORD	    extralength;      // extra field length
  // filename (variable size)
  // extra field (variable size)
};
#pragma pack(pop)

static DWORD	  dword(const DWORD& dw) { // convert from unaligned le-dword
  const	BYTE  *b=(const BYTE *)&dw;
  return (DWORD)b[0]|((DWORD)b[1]<<8)|((DWORD)b[2]<<16)|((DWORD)b[3]<<24);
}

static WORD	  word(const WORD& w) { // convert from unaligned word
  const BYTE  *b=(const BYTE *)&w;
  return (WORD)b[0]|((WORD)b[1]<<8);
}

// copy the string without _any_ conversion
static CString	  rawconv(const Buffer<char>& buf) {
  CString   ret;
  TCHAR	    *ptr=ret.GetBuffer(buf.size());
  for (int i=0;i<buf.size();++i)
    ptr[i]=(unsigned char)buf[i];
  ret.ReleaseBuffer(buf.size());
  return ret;
}

bool  ZipFile::ReadZip() {
  int	cp=-1;
  int	acp=Unicode::GetIntCodePage(::GetACP());
  int	oemcp=Unicode::GetIntCodePage(::GetOEMCP());

  // iterate over zip file until we find a central dir
  for (;;) {
    LocalZipHdr	  hdr;
    if (RFile::read2(&hdr,sizeof(hdr))!=sizeof(hdr)) // invalid file
      return false;
    if (dword(hdr.sig)==DIRSIG) { // found central directory
      // version here will be used to select codepage
      CString	rcp(CTVApp::GetStr(_T("ZipEncoding")));
      if (rcp.GetLength()>0)
	cp=Unicode::FindCodePage(rcp);
      if (cp<0) {
	BYTE  os=word(hdr.extver)>>8;
	if (os==11) // ntfs
	  cp=acp;
	else if (os==5 && acp==1251) // unix
	  cp=Unicode::FindCodePage(_T("koi8-r"));
	else
	  cp=oemcp;
      }
      if (cp<0)
	cp=oemcp;
      break;
    }
    if (dword(hdr.sig)!=LOCALSIG) // invalid zip
      return false;
    // check validity
    if (word(hdr.flags)&0x0004) // unsupported format
      return false;
    if (word(hdr.namelength)==0) // invalid zip
      return false;
    if (word(hdr.compmeth)==0 || word(hdr.compmeth)==8) { // storing and deflating only supported
      Buffer<char>    name(word(hdr.namelength));
      if (RFile::read2(name,word(hdr.namelength))!=word(hdr.namelength)) // invalid zip
	return false;
      if (name[name.size()-1]=='/') { // this is a directory, skip it
	RFile::seek2(word(hdr.extralength),FILE_CURRENT);
	continue;
      }
      // skip over extra field
      RFile::seek2(word(hdr.extralength),FILE_CURRENT);
      // add file to in-memory directory
      // what a PITA. we don't know the codepage yet
      CString	      uname(rawconv(name));
      int	      idx=0;
      int	      end=uname.ReverseFind(_T('/')); // see if we have a path
      ZFile	      *cur=&m_rootdir;
      while (idx<end) { // locate directories, creating if needed
	int	spos=uname.Find(_T('/'),idx);
	CString	elem;
	if (spos<0 || spos>=end) { // this is the last component
	  elem=uname.Mid(idx,end-idx);
	  idx=end;
	} else {
	  elem=uname.Mid(idx,spos-idx);
	  idx=spos+1;
	}
	void	*ptr;
	if (!cur->children->Lookup(elem,ptr)) { // no such dir, create it
	  ZFile	  *dir=new ZFile;
	  dir->parent=cur;
	  dir->off=-1;
	  dir->children=new CMapStringToPtr;
	  cur->children->SetAt(elem,dir);
	  ptr=dir;
	}
	cur=(ZFile*)ptr;
	if (cur->off>=0) // invalid zip, we already have a file with this name
	  return false;
      }
      void    *ptr;
      uname.Delete(0,end+1);
      if (cur->children->Lookup(uname,ptr)) // invalid zip, we already have this file
	return false;
      ZFile   *file=new ZFile;
      file->off=(int)RFile::pos();
      file->size=dword(hdr.usize);
      if (word(hdr.compmeth)==0)
	file->csize=-1;
      else
	file->csize=dword(hdr.compsize);
      file->parent=cur;
      cur->children->SetAt(uname,file);
      // skip over compressed data
      RFile::seek2(dword(hdr.compsize),FILE_CURRENT);
    } else // skip this file
      RFile::seek2(dword(hdr.compsize)+word(hdr.namelength)+word(hdr.extralength),FILE_CURRENT);
  }
  if (cp<0)
    cp=Unicode::DefaultCodePage();
  m_rootdir.fixup_encoding(cp);
  return true;
}

bool  ZipFile::Open(const CString& filename) {
  void	  *ptr;
  if (!m_curdir->children->Lookup(filename,ptr))
    return false;
  m_curfile=(ZFile*)ptr;
  if (m_curfile->iscmp()) {
    if (m_zs)
      inflateReset(m_zs);
    else {
      m_in=Buffer<Bytef>(BSZ);
      m_zs=new z_stream;
      memset(m_zs,0,sizeof(*m_zs));
      inflateInit2(m_zs,-MAX_WBITS);
    }
  }
  m_ptr=0;
  RFile::seek(m_curfile->off);
  return true;
}

bool  ZipFile::IsSingleFile(CString *s) {
  if (m_rootdir.off<0 && m_rootdir.children &&
      m_rootdir.children->GetCount()==1)
  {
    POSITION	p=m_rootdir.children->GetStartPosition();
    if (p!=NULL) {
      CString tmp;
      void    *q;
      m_rootdir.children->GetNextAssoc(p,tmp,q);
      ZFile   *f=(ZFile*)q;
      if (f->off>=0) {
	if (s)
	  *s=tmp;
	return true;
      }
    }
  }
  return false;
}

void  ZipFile::ZFile::fixup_encoding(int enc) {
  if (!isdir())
    return;
  CString	  fn,rc;
  void		  *ptr;
  CMapStringToPtr *map=new CMapStringToPtr;
  for (POSITION pos=children->GetStartPosition();pos;) {
    children->GetNextAssoc(pos,fn,ptr);
    // contrive a char buffer, copy data and convert
    Buffer<char>    tmp(fn.GetLength());
    for (int i=0;i<fn.GetLength();++i)
      tmp[i]=(char)fn[i];
    rc=Unicode::ToCS(enc,tmp,tmp.size());
    map->SetAt(rc,ptr);
    if (((ZFile*)ptr)->isdir())
      ((ZFile*)ptr)->fixup_encoding(enc);
  }
  delete children;
  children=map;
}
