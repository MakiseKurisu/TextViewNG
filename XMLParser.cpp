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
 * $Id: XMLParser.cpp,v 1.106.2.12 2004/10/13 13:50:55 mike Exp $
 * 
 */

#include <afxwin.h>
#include <afxtempl.h>
#include <setjmp.h>

#include <initguid.h>
DEFINE_GUID(CLSID_DOMDocument30,0xf5078f32,0xc551,0x11d3,0x89,0xb9,0x00,0x00,0xf8,0x1f,0xe2,0x21);

#include "FastArray.h"
#include "TextParser.h"
#include "XMLParser.h"
#include "TextViewNG.h"
#include "Unicode.h"
#include "StrBuf.h"
#include "WMap.h"
#include "Image.h"

#include "xscanf.h"

#include "expat.h"

#define	FB_NS	  L"http://www.gribuser.ru/xml/fictionbook/2.0"
#define	FB_NS_LEN (sizeof(FB_NS)/sizeof(wchar_t)-1)
#define	XLINK_NS  L"http://www.w3.org/1999/xlink"
#define	HR_STYLE  L"text/css"

enum {
  CDATA=0x1000000,
  LEADSP=0x2000000,
  LOCAL=0x4000000,
  REALLYLOCAL = 0x8000000,
  TRAILSP = 0x10000000,

  // PE flags
  PE_IMAGE=0x80000000,
};
enum { MAX_CONTENTS_LEN=80 };

enum {
  ERR_NOTFB2=1
};

struct CFMT {
  Attr	  attr;
  int	  lindent;
  int	  rindent;
  int	  findent;
  BYTE	  flags;
};

struct CachedImage {
  CachedImage	      *next;
  HBITMAP	      hBmp;
  int		      width;
  int		      height;
  int		      maxwidth;
  int		      maxheight;
  int		      rotation;
  const wchar_t     *name;
  CachedImage() : hBmp(NULL), name(NULL), next(NULL) { }
  void	      Release() { if (hBmp) DeleteObject(hBmp); name=NULL; hBmp=NULL; }
};

class ImageCache {
  CachedImage	      *m_head,*m_tail;
  int		      m_count;
  int		      m_max;
public:
  ImageCache(int max=4) : m_head(NULL), m_tail(NULL), m_count(0), m_max(max) { }
  ~ImageCache() { RemoveAll(); }

  CachedImage	      *Lookup(const wchar_t *name,bool& alloc);
  void		      Remove(CachedImage *img);
  void		      RemoveAll() {
    while (m_head) {
      CachedImage *next=m_head->next;
      m_head->Release();
      delete m_head;
      m_head=next;
    }
    m_count=0;
    m_head=m_tail=NULL;
  }
};

CachedImage   *ImageCache::Lookup(const wchar_t *name,bool& alloc) {
  CachedImage	*img;

  alloc=false;
  for (img=m_head;img;img=img->next)
    if (!wcscmp(name,img->name))
      return img;

  alloc=true;
  // not found, try to allocate new
  if (m_count<m_max) {
    img=new CachedImage;
    ++m_count;
    img->name=name;
    if (m_tail) {
      m_tail->next=img;
      m_tail=img;
    } else
      m_head=m_tail=img;
    return img;
  }
  // too many entries, reuse head
  ASSERT(m_head!=NULL);
  img=m_head;
  if (m_head!=m_tail)
    m_head=m_head->next;
  img->Release();
  img->name=name;
  img->next=NULL;
  if (img!=m_tail)
    m_tail->next=img;
  m_tail=img;
  return img;
}

void  ImageCache::Remove(CachedImage *img) {
  img->Release();
  if (img==m_head) {
    m_head=m_head->next;
    if (img==m_tail)
      m_tail=NULL;
  } else { // tough, will have to traverse the list
    CachedImage	*tmp=NULL;
    for (tmp=m_head;tmp;tmp=tmp->next)
      if (tmp->next==img)
	break;
    tmp->next=img->next;
    if (img==m_tail)
      m_tail=tmp;
  }
  delete img;
  --m_count;
}

class XMLParserImp: public XMLParser {
public:

  struct SP_State {
    enum {
      START,
      NAME,
      FLAGS,
      FM,LM,RM,SIZE,COLOR
    };
    wchar_t				stylename[128];
    int					stylenameptr;
    ElemFmt				format;
    int					state;
    int					num;
    bool				sign;
    void				Init() { stylenameptr=0; format.Clear(); state=START; }
    void				NAdd(wchar_t ch) {
      if (stylenameptr<sizeof(stylename)/sizeof(wchar_t)-1)
	stylename[stylenameptr++]=ch;
    }
  };

  struct ParseState {
    enum { MAX_NEST=64 };
    int					len; // current len
    int					start;
    DWORD				attr;
    Attr				last_frag_fmt;
    bool				last_frag_trailsp;
    bool				root_element;
    CFMT				cfmt;
    CFMT				attr_stack[MAX_NEST];
    int					attr_stack_ptr;
    int					acch_lev,in_stylesheet;
    int					enable;
    int					section_nest;
    int					title_start;
    int					link_start;
    const wchar_t			*link_name;
    int					pf_start,pl_start,numfrags;
    int					binary;
    FmtArray				*styles;
    WMap				*stylemap;
    jmp_buf				jout;

    void PushA() {
      if (attr_stack_ptr<MAX_NEST)
	attr_stack[attr_stack_ptr++]=cfmt;
    }
    void PopA() {
      if (attr_stack_ptr>0)
	cfmt=attr_stack[--attr_stack_ptr];
    }
    DWORD Att() { return attr|cfmt.attr.wa; }
    void  ApplyFmt(ElemFmt *e,int nest=0);
  };

  struct Frag {    // smallest element - character data
    union {
      DWORD	    fpos;    // offset into the file
      const wchar_t *str;    // pointer to a cached value
      wchar_t	    local[2]; // cached right here
    };
    DWORD   len;    // raw char count
    DWORD   attr;   // attributes of this run
  };
  struct PE { // paragraph
    enum {
      FRAGBITS=10,
      MAXFRAGS=1<<FRAGBITS,
      FRAGSHIFT=32-FRAGBITS,
      IDXMASK=(1<<FRAGSHIFT)-1
    };
    union {
      DWORD		  idx_nf; // offset into m_frags
      const wchar_t	  *name; // on an image name
    };
    int		  start; // start of parsed paragraph
    DWORD	  linkidx_nl; // offset into m_links
    DWORD	  indent; // left, right and first line indentation
    DWORD	  flags;

    DWORD	  nfrags() { return idx_nf>>FRAGSHIFT; }
    DWORD	  idx() { return idx_nf&IDXMASK; }

    DWORD	  nlinks() { return linkidx_nl>>FRAGSHIFT; }
    DWORD	  lidx() { return linkidx_nl&IDXMASK; }

    DWORD	  li() { return (indent>>10)&0x2ff; }
    DWORD	  ri() { return indent&0x2ff; }
    DWORD	  fi() { return (indent>>20)&0x2ff; }

    void	  setidx_nf(DWORD idx,DWORD nf) { idx_nf=(idx&IDXMASK)|(nf<<FRAGSHIFT); }
    void	  setidx_nl(DWORD idx,DWORD nf) { linkidx_nl=(idx&IDXMASK)|(nf<<FRAGSHIFT); }
    void	  setindent(DWORD l,DWORD r,DWORD f) { indent=((f&0x2ff)<<20)|((l&0x2ff)<<10)|(r&0x2ff); }
    void	  Zero() { memset(this,0,sizeof(*this)); }
  };

  struct Document { // subdocument
    int	    start; // start paragraph
    int	    length; // length in paragraphs
    CString name;
  };

  struct Link {
    int		  start;
    int		  length;
    const wchar_t *target;
  };

  struct Binary {
    wchar_t	      *id;
    wchar_t	      *type;
    int		      numfrags;
    int		      startfrag;
  };

  friend class Base64BinReader;

  FastArray<Frag>	m_frags;
  FastArray<PE>		m_pp;
  FastArray<Link>	m_links;
  FastArray<Binary>	m_binarystorage;
  FastArray<const wchar_t *> m_inline_images;
  StrBuf		m_buffer;
  XML_Parser		m_parser;
  CArray<Document,Document&>	m_docs;
  WMap			m_references;
  WMap			m_binaries;
  ParseState		*m_ps;
  SP_State		*m_sps;
  CString		m_cover;
  ImageCache		m_imcache;

  Paragraph		GetParagraphImp(int idx);

  // paragraphs
  virtual Paragraph	GetParagraph(int docid,int para);
  virtual int		Length(int docid); // in paragraphs
  virtual int		GetPLength(int docid,int para);
  virtual int		GetPStart(int docid,int para);
  virtual int		GetTotalLength(int docid);
  virtual int		LookupParagraph(int docid,int charpos);

  // documents
  virtual int		GetSubDocCount() { return m_docs.GetSize(); }
  virtual CString	GetSubDocName(int docid);

  // links
  virtual bool		LookupReference(const wchar_t *name,FilePos& dest);

  // images
  virtual bool		GetImage(const wchar_t *name,HDC hDC,int maxwidth,
    int maxheight,int rotation,Image& img);
  virtual void		InvalidateImageCache() { m_imcache.RemoveAll(); }

  // construction and destruction
			XMLParserImp(Meter *m,CBufFile *fp,Bookmarks *bmk,
				     HANDLE heap);
  virtual		~XMLParserImp();

  virtual bool		ParseFile(int encoding);

  // paragraphs
  void			AddP(int pstart,int lstart,int start,int len,CFMT& fmt);
  void			AddImage(const wchar_t *href,int start,CFMT& fmt);
  void			AddQ(int start);
  void			AddToc(FilePos pos,int level);
  void			PushWS(); // check for leading spaces/format flags

  // stylesheet
  void			ParseStylesheet(const wchar_t *text,int len);

  // callbacks
  void			StartElement(const wchar_t *name,const wchar_t **attr);
  void			EndElement(const wchar_t *name);
  void			CharData(const wchar_t *text,int len);

  // expat callacks
  static void		StartElementCB(void *udata,const wchar_t *name,
				       const wchar_t **attr);
  static void		EndElementCB(void *udata,const wchar_t *name);
  static void		CharDataCB(void *udata,const wchar_t *text,int len);
  static int		UnknownEncodingCB(void *data,const wchar_t *name,
					  XML_Encoding *info);
  static void		StartCDataCB(void *udata);
  static void		EndCDataCB(void *udata);

  // binary access
  ImageLoader::BinReader *OpenBinary(const wchar_t *name,const wchar_t **type,
    const wchar_t **vname);
};

const TCHAR   *XMLParser::ElemFmt::flag_names=_T("apofestcdlqxrivbgh");


XMLParserImp::XMLParserImp(Meter *m,CBufFile *fp,Bookmarks *bmk,HANDLE heap) :
  XMLParser(m,fp,heap,bmk), m_parser(NULL), m_pp(heap), m_frags(heap),
  m_links(heap), m_buffer(heap), m_references(heap), m_binaries(heap),
  m_binarystorage(heap), m_inline_images(heap)
{
}

XMLParserImp::~XMLParserImp() {
  // destroy parser, if any
  if (m_parser)
    XML_ParserFree((XML_Parser)m_parser);
  // destroy image cache if any
}

int	  XMLParserImp::Length(int docid) {
  return docid<0 || docid>=m_docs.GetSize() ? 0 : m_docs[docid].length;
}

#define	  SHY 0xAD

Paragraph XMLParserImp::GetParagraph(int docid,int para) {
  if (docid<0 || docid>=m_docs.GetSize() ||
      para<0 || para>=m_docs[docid].length)
    return Paragraph();
  return GetParagraphImp(m_docs[docid].start+para);
}

CString	XMLParserImp::GetSubDocName(int docid) {
  if (docid<0 || docid>=m_docs.GetSize())
    return CString();
  if (docid==0 && m_docs[docid].name.GetLength()==0)
    return _T("Main");
  return m_docs[docid].name;
}

static int  RClamp(int v,int min,int max) {
  if (v<min)
    return min;
  if (v>max)
    return max;
  return v;
}

Paragraph XMLParserImp::GetParagraphImp(int idx) {
  if (m_pp[idx].flags&PE_IMAGE) { // a very special case
    Paragraph	p(ImageLoader::IMAGE_VSIZE);
    p.flags=(BYTE)m_pp[idx].flags;
    p.lindent=m_pp[idx].li();
    p.rindent=m_pp[idx].ri();
    p.findent=m_pp[idx].fi();
    for (int i=0;i<ImageLoader::IMAGE_VSIZE;++i) {
      p.str[i]=L' ';
      p.cflags[i].wa=0;
    }
    // abuse links for image href
    p.links=Buffer<Paragraph::Link>(1);
    p.links[0].off=0;
    p.links[0].len=ImageLoader::IMAGE_VSIZE;
    p.links[0].target=m_pp[idx].name;
    p.flags|=Paragraph::image;
    return p;
  }
  // here we have to read the paragraphs from file
  int	      len=m_pp[idx+1].start-m_pp[idx].start,np=m_pp[idx].nfrags();
  int	      fragbase=m_pp[idx].idx();
  Paragraph   p(len);
  p.flags=(BYTE)m_pp[idx].flags;
  p.lindent=m_pp[idx].li();
  p.rindent=m_pp[idx].ri();
  p.findent=m_pp[idx].fi();

  // check for inline images
  int	ilinks=0;
  for (int ii=0;ii<np;++ii)
    if (m_frags[ii+fragbase].attr & PE_IMAGE)
      ++ilinks;

  // add links if any
  int	nlinks=m_pp[idx].nlinks();
  if (nlinks+ilinks) {
    int			      off=m_pp[idx].lidx();
    Buffer<Paragraph::Link>   links(nlinks+ilinks);
    for (int link=0;link<nlinks;++link) {
      links[link].off=m_links[off+link].start;
      links[link].len=m_links[off+link].length;
      links[link].target=m_links[off+link].target;
    }
    p.links=links;
  }
  ilinks=0;

  // add character data
  wchar_t     *bp=p.str;
  wchar_t     *be=bp+len;
  wchar_t     *bs=bp;
  Attr	      *ap=p.cflags;
  for (int f=0;f<np && bp<be;++f) {
    Frag    *fp=&m_frags[f+fragbase];
    // add a space if needed
    if (fp->attr&LEADSP) {
      *bp++=' ';
      (*ap++).wa=(WORD)fp->attr;
    }
    if (fp->attr&PE_IMAGE) { // inline image
      int	    globidx=fp->local[0] | ((unsigned int)fp->local[1]<<16);
      p.links[nlinks+ilinks].off=p.links[nlinks+ilinks].len=0;
      p.links[nlinks+ilinks].target=m_inline_images[globidx];
      if (bp<be) {
	*bp++=nlinks+ilinks;
	(*ap++).wa=(WORD)fp->attr;
      }
      ++ilinks;
    } else if (fp->attr&(LOCAL|REALLYLOCAL)) { // cached value
      const wchar_t   *wp=fp->attr & REALLYLOCAL ? fp->local : fp->str;
      const wchar_t   *we=wp+fp->len;
      while (bp<be && wp<we) {
	*bp++=*wp++;
	(*ap++).wa=(WORD)fp->attr;
      }
    } else {
      Buffer<char>    buf(fp->len);
      m_fp->seek(fp->fpos);
      if (m_fp->read(buf,fp->len)!=(int)fp->len) {
	ASSERT(0);
	break;
      }
      Buffer<wchar_t> wbuf(fp->len);
      char	    *cp=buf;
      int ul;
      if (fp->attr&CDATA)
	ul=XML_ConvertCharacterData((XML_Parser)m_parser,&cp,fp->len,wbuf,fp->len);
      else
	ul=XML_ParseCharacterData((XML_Parser)m_parser,&cp,fp->len,wbuf,fp->len);
      if (ul<0) // xml parser returned an error
	break;
      wchar_t	    *wp=wbuf,*we=wbuf+ul;
      // skip leading ws
      while (wp<we && (*wp<=32 || *wp==SHY))
	++wp;
      goto in;
      while (bp<be && wp<we) { // copy, compacting whitespace
	// add a space
	*bp++=L' ';
	(*ap++).wa=(WORD)fp->attr;
  in:
	// copy non-ws
	while (bp<be && wp<we && *wp>32)
	  if (*wp==SHY) {
	    if (bp>bs)
	      ap[-1].hyphen=true;
	    ++wp;
	  } else {
	    *bp++=*wp++;
	    (*ap++).wa=(WORD)fp->attr;
	  }
	// skip over spaces
	while (wp<we && (*wp<=32 || *wp==SHY))
	  ++wp;
      }
    }
    // add a trailing space if required
    if (fp->attr&TRAILSP && bp<be) {
      *bp++=L' ';
      (*ap++).wa=(WORD)fp->attr;
    }
  }
  // to avoid endless loops later, we have to pad the paragraph up to the
  // advertised length
  while (bp-bs<len) {
    *bp++=L' ';
    (*ap++).wa=0;
  }
  p.str.setsize(len);
  p.cflags.setsize(len);
  p.len=len;

  p.links.setsize(nlinks+ilinks);

  return p;
}

int	  XMLParserImp::GetPStart(int docid,int para) {
  if (docid<0 || docid>=m_docs.GetSize() || para<0)
    return 0;
  if (para>=m_docs[docid].length)
    return m_pp[m_docs[docid].start+m_docs[docid].length].start;
  return m_pp[m_docs[docid].start+para].start;
}

int	  XMLParserImp::GetPLength(int docid,int para) {
  if (docid<0 || docid>=m_docs.GetSize() || para<0 || para>=m_docs[docid].length)
    return 0;
  int idx=m_docs[docid].start+para;
  return m_pp[idx+1].start-m_pp[idx].start;
}

int	  XMLParserImp::GetTotalLength(int docid) {
  if (docid<0 || docid>=m_docs.GetSize())
    return 0;
  return m_pp[m_docs[docid].start+m_docs[docid].length].start;
}

int   XMLParserImp::LookupParagraph(int docid,int charpos) {
  if (docid<0 || docid>=m_docs.GetSize())
    return 0;
  int	i=m_docs[docid].start,j=i+m_docs[docid].length;
  if (charpos<0 || charpos>m_pp[j].start)
    return 0;
  if (charpos==m_pp[j].start)
    return j-i;
  --j;
  while (i<=j) {
    int	  m=(i+j)>>1;
    if (charpos<m_pp[m].start)
      j=m-1;
    else if (charpos>=m_pp[m+1].start)
      i=m+1;
    else
      return m-m_docs[docid].start;
  }
  return 0;
}

bool	  XMLParserImp::LookupReference(const wchar_t *name,FilePos& dest) {
  void	  *data;

  if (!m_references.Lookup(name,data))
    return false;
  int	  para=(int)data;
  // find the docid
  for (int docid=0;docid<m_docs.GetSize();++docid)
    if (para>=m_docs[docid].start && para<m_docs[docid].start+m_docs[docid].length) {
      dest.para=para-m_docs[docid].start;
      dest.off=0;
      dest.docid=docid;
      return true;
    }
  // should not happen
  return false;
}

bool	  XMLParserImp::GetImage(const wchar_t *name,HDC hDC,int maxwidth,
				 int maxheight,int rotation,Image& img)
{
  bool		alloc;
  CachedImage	*cim=m_imcache.Lookup(name,alloc);
  if (!alloc && cim->hBmp && (cim->maxwidth==maxwidth ||
      (cim->width<cim->maxwidth && cim->width<maxwidth)) &&
      (cim->maxheight==maxheight || (cim->height<cim->maxheight &&
      cim->height<maxheight)) &&
    cim->rotation==rotation)
  {
    img.hBmp=cim->hBmp;
    img.height=cim->height;
    img.width=cim->width;
    return true;
  }
  const wchar_t		  *type,*vname;
  ImageLoader::BinReader  *rdr;
  if (rdr=OpenBinary(name,&type,&vname)) {
    int	  width,height;
    HBITMAP  hBmp;
    bool ret=ImageLoader::Load(hDC,type,rdr,maxwidth,maxheight,rotation,hBmp,width,height);
    delete rdr;
    if (ret) {
      cim->maxwidth=maxwidth;
      cim->maxheight=maxheight;
      cim->rotation=rotation;
      cim->width=width;
      cim->height=height;
      cim->hBmp=hBmp;
      cim->name=vname;
      img.hBmp=hBmp;
      img.width=width;
      img.height=height;
      return true;
    }
  }
  m_imcache.Remove(cim);
  return false;
}

void	  XMLParserImp::AddQ(int start) {
  PE	pe;
  pe.Zero();
  pe.start=start;
  m_pp.Add(pe);
}

void	  XMLParserImp::AddToc(FilePos pos,int level) {
  if (m_bmk) {
    FilePos ref(pos);
    ref.off=0;
    m_bmk->AddTocEnt(pos.para,pos.off,pos.docid,ref,level);
  }
}

void	  XMLParserImp::AddP(int pstart,int lstart,int start,int len,CFMT& fmt)
{
  if (pstart==m_frags.GetSize() || len==0) {
    AddQ(start);
    return;
  }
  PE	pe;
  pe.setidx_nf(pstart,m_frags.GetSize()-pstart);
  pe.setidx_nl(lstart,m_links.GetSize()-lstart);
  pe.start=start;
  pe.flags=fmt.flags;
  pe.setindent(RClamp(fmt.lindent,0,500),RClamp(fmt.rindent,0,500),
    RClamp(fmt.findent,0,500));
  m_pp.Add(pe);
}

void	  XMLParserImp::AddImage(const wchar_t *href,int start,CFMT& fmt)
{
  PE	pe;
  pe.name=m_buffer.Append(href,wcslen(href)+1);
  pe.flags=fmt.flags|PE_IMAGE;
  pe.setidx_nl(0,0);
  pe.start=start;
  pe.setindent(RClamp(fmt.lindent,0,500),RClamp(fmt.rindent,0,500),
    RClamp(fmt.findent,0,500));
  m_pp.Add(pe);
}

void	  XMLParserImp::PushWS() {
  Frag    f;
  // if we already have a leading space, but its charformat differs from current
  // we have to create a fully whitespace frag
  if (m_ps->attr&LEADSP && m_ps->last_frag_fmt!=m_ps->cfmt.attr) {
    f.attr=m_ps->last_frag_fmt.wa|REALLYLOCAL;
    f.len=1;
    f.local[0]=L' ';
    m_frags.Add(f);
    m_ps->numfrags++;
    m_ps->len++;
    m_ps->attr&=~LEADSP;
  }
}

class Base64BinReader: public ImageLoader::BinReader {
  enum { FEOF=65535 };

  XMLParserImp	  *m_parser;
  Buffer<wchar_t> m_fragbuf;
  int		  m_maxfragsize;
  int		  m_ptr;
  int		  m_fragptr;
  int		  m_fragtop;

  unsigned int	  m_chunk;
  int		  m_chunkbytes;

  bool		  NextFrag();
  wchar_t	  NextFragChar();
  wchar_t	  Char() { return m_ptr<m_fragbuf.size() ?
    m_fragbuf[m_ptr++] : NextFragChar(); }
public:
  Base64BinReader(XMLParserImp *xp,XMLParserImp::Binary *b) :
      m_parser(xp), m_maxfragsize(0), m_ptr(0),
      m_fragptr(b->startfrag), m_fragtop(b->startfrag+b->numfrags),
      m_chunk(0), m_chunkbytes(0) { }
  virtual int	  Read(void *buffer,int count);
};

ImageLoader::BinReader	*XMLParserImp::OpenBinary(const wchar_t *name,
						  const wchar_t **type,
						  const wchar_t **vname) {
  void	      *ptr;
  if (!m_binaries.Lookup(name,ptr))
    return NULL;
  Binary      *b=(Binary*)ptr;
  if (type)
    *type=b->type;
  if (vname)
    *vname=b->id;
  // we only support base64
  return new Base64BinReader(this,b);
}

bool  Base64BinReader::NextFrag() {
  if (m_fragptr>=m_fragtop)
    return false;
  XMLParserImp::Frag    *fp=&m_parser->m_frags[m_fragptr];
  if ((int)fp->len>m_maxfragsize) {
    m_maxfragsize=fp->len;
    m_fragbuf=Buffer<wchar_t>(m_maxfragsize);
  }
  if (fp->attr&REALLYLOCAL) {
    m_fragbuf.setsize(fp->len);
    memcpy(m_fragbuf,fp->local,fp->len*sizeof(wchar_t));
  } else if (fp->attr&LOCAL) {
    m_fragbuf.setsize(fp->len);
    memcpy(m_fragbuf,fp->str,fp->len*sizeof(wchar_t));
  } else {
    Buffer<char>    buf(fp->len);
    m_parser->m_fp->seek(fp->fpos);
    if (m_parser->m_fp->read(buf,fp->len)!=(int)fp->len) {
      ASSERT(0);
      return false;
    }
    char	    *cp=buf;
    int ul;
    if (fp->attr&CDATA)
      ul=XML_ConvertCharacterData((XML_Parser)m_parser->m_parser,&cp,fp->len,m_fragbuf,fp->len);
    else
      ul=XML_ParseCharacterData((XML_Parser)m_parser->m_parser,&cp,fp->len,m_fragbuf,fp->len);
    if (ul<0) // xml parser returned an error
      return false;
    m_fragbuf.setsize(ul);
  }
  ++m_fragptr;
  m_ptr=0;
  return true;
}

wchar_t	Base64BinReader::NextFragChar() {
  if (!NextFrag())
    return FEOF;
  return m_ptr<m_fragbuf.size() ? m_fragbuf[m_ptr++] : FEOF;
}

int   Base64BinReader::Read(void *buffer,int count) {
  char	*dest=(char*)buffer;
  char	*de=dest+count,*ds=dest;

  do {
    // store accumulated bytes
    while (m_chunkbytes>0 && ds<de) {
      *ds++=((char*)&m_chunk)[2];
      m_chunkbytes--;
      m_chunk<<=8;
    }
    if (ds>=de)
      break;
    // fill in three more octets
    m_chunk=0; m_chunkbytes=3;
    int i=18;
    do {
      wchar_t	c=Char();
      if (c==FEOF) {
	m_chunkbytes=0;
	break;
      }
      if (c>='A' && c<='Z')
	m_chunk|=(unsigned int)(c-'A')<<i;
      else if (c>='a' && c<='z')
	m_chunk|=(unsigned int)(c-'a'+26)<<i;
      else if (c>='0' && c<='9')
	m_chunk|=(unsigned int)(c-'0'+52)<<i;
      else if (c=='+')
	m_chunk|=62<<i;
      else if (c=='/')
	m_chunk|=63<<i;
      else if (c=='=') {
	if (i==6)
	  m_chunkbytes=1;
	else if (i==0)
	  m_chunkbytes=2;
	// force eof
	m_fragtop=m_fragptr;
	m_fragbuf.setsize(0);
	break;
      } else
	continue; // silently ignore all other chars
      i-=6;
    } while (i>=0);
    if (!m_chunkbytes) // EOF
      break;
  } while (ds<de);
  return ds-dest;
}

static void   *my_malloc(void *priv,size_t size) {
  return HeapAlloc((HANDLE)priv,HEAP_NO_SERIALIZE,size);
}

static void   *my_realloc(void *priv,void *ptr,size_t size) {
  return HeapReAlloc((HANDLE)priv,HEAP_NO_SERIALIZE,ptr,size);
}

static void   my_free(void *priv,void *ptr) {
  HeapFree((HANDLE)priv,HEAP_NO_SERIALIZE,ptr);
}

static void normalize_space(wchar_t *dest,int dlen,const wchar_t *s,int len)
{
  wchar_t	*q=dest+dlen;
  const wchar_t	*e=s+len;
  while (s<e && *s<=32)
      ++s;
  goto in;
  while (s<e && dest<q) {
    while (s<e && *s<=32)
      ++s;
    if (s<e)
      *dest++=L' ';
in:
    while (s<e && dest<q && *s>32)
      *dest++=*s++;
  }
}

static int    normalized_length(const wchar_t *s,int len) {
  const wchar_t *e=s+len;
  int	      nl=0;
  while (s<e && *s<=32)
    ++s;
  goto in;
  while (s<e) {
    while (s<e && *s<=32)
      ++s;
    if (s<e)
      ++nl;
in:
    while (s<e && *s>32)
      ++nl,++s;
  }
  return nl;
}

static bool iswhitespace(const wchar_t *text,int len) {
  const wchar_t *end=text+len;
  while (text<end) {
    if (*text!=0x0d && *text!=0x0a && *text!=0x20 && *text!=0x09)
      return false;
    ++text;
  }
  return true;
}

static XMLParser::FmtArray	g_eformat;
static WMap			*g_elements;

static struct Cleaner {
  ~Cleaner() { delete g_elements; }
} g_cleaner;

static XMLParser::ElemFmt    *LookupElem(const wchar_t *name) {
  void *data;
  if (name && g_elements->Lookup(name,data))
    return &g_eformat[(int)data];
  return &g_eformat[0]; // default
}

static const wchar_t	      *LocalName(const wchar_t *name) {
  const wchar_t	  *end=wcschr(name,L'|');
  return end ? end+1 : name;
}

void XMLParserImp::ParseState::ApplyFmt(ElemFmt *e,int nest) {
  PushA();
  if (e->fsz!=XMLParser::ElemFmt::NOCHG)
    cfmt.attr.fsize=nest ? max(e->fsz-nest,1) : e->fsz;
  if (e->bold!=XMLParser::ElemFmt::NOCHG)
    cfmt.attr.bold=e->bold;
  if (e->italic!=XMLParser::ElemFmt::NOCHG)
    cfmt.attr.italic=e->italic;
  if (e->underline!=XMLParser::ElemFmt::NOCHG)
    cfmt.attr.underline=e->underline;
  if (e->color!=XMLParser::ElemFmt::NOCHG)
    cfmt.attr.color=e->color;
  if (e->align!=XMLParser::ElemFmt::NOCHG)
    cfmt.flags=(cfmt.flags & ~Paragraph::align_mask) | (e->align & Paragraph::align_mask);
  if (e->lindent!=XMLParser::ElemFmt::NOCHG) {
    if (e->lindent<0)
      cfmt.lindent-=e->lindent;
    else
      cfmt.lindent=e->lindent;
  }
  if (e->rindent!=XMLParser::ElemFmt::NOCHG) {
    if (e->rindent<0)
      cfmt.rindent-=e->rindent;
    else
      cfmt.rindent=e->rindent;
  }
  if (e->findent!=XMLParser::ElemFmt::NOCHG) {
    if (e->findent<0)
      cfmt.findent-=e->findent;
    else
      cfmt.findent=e->findent;
  }
}

const wchar_t *GetAttr(const wchar_t **attr,const wchar_t *name,const wchar_t *def=NULL)
{
  while (*attr) {
    if (!wcscmp(name,*attr))
      return attr[1];
    attr+=2;
  }
  return def;
}

void XMLParserImp::StartElement(const wchar_t *ns_name,const wchar_t **attr) {
  if (!m_ps->root_element) { // special case for root
    const wchar_t *p = ns_name;
    const wchar_t *q = FB_NS;
    const wchar_t *e = q + FB_NS_LEN - 1;
    while (*p && q < e)
      if (*p++ != *q++)
	longjmp(m_ps->jout,ERR_NOTFB2);
    if (!p[0] || wcscmp(p+1,L"|FictionBook")!=0)
      longjmp(m_ps->jout,ERR_NOTFB2);
    m_ps->root_element=true;
  }
  ElemFmt	*elem=LookupElem(ns_name);
  if (elem->flags&ElemFmt::DOCUMENT) {
    if (m_docs.GetSize()==0)
      AddQ(m_ps->start);
    Document  d;
    d.start=m_pp.GetSize();
    d.length=0;
    d.name=GetAttr(attr,L"name",L"");
    m_docs.Add(d);
    m_ps->start=0;
    // add a cover page if it was present
    if (m_docs.GetSize()==1 && m_cover.GetLength()>0) {
      AddImage(m_cover,m_ps->start,m_ps->cfmt);
      m_ps->start+=ImageLoader::IMAGE_VSIZE;
    }
  }
  if (elem->flags&ElemFmt::ENABLE)
    ++m_ps->enable;
  if (elem->flags&ElemFmt::SECTION)
    m_ps->section_nest++;
  if (elem->flags&ElemFmt::ELINE)
    AddQ(m_ps->start);
  if (elem->flags&ElemFmt::FMT) // apply formatting
    m_ps->ApplyFmt(elem);
  if (elem->flags&ElemFmt::STYLE) {
    const wchar_t   *style=GetAttr(attr,L"name");
    void	    *val;
    if (style && m_ps->stylemap->Lookup(style,val))
      m_ps->ApplyFmt(&m_ps->styles->operator[]((int)val));
    else
      m_ps->PushA();
  }
  if (elem->flags&ElemFmt::HEADER) {
    m_ps->PushA();
    m_ps->cfmt.flags |= Paragraph::header;
  }
  if (elem->flags&ElemFmt::STYLESHEET) {
    const wchar_t *type=GetAttr(attr,L"type");
    if (m_ps->in_stylesheet || (type && !wcscmp(type,HR_STYLE)))
      if (!m_ps->in_stylesheet++)
	m_sps->Init();
  }
  if (elem->flags&ElemFmt::TITLE) // start a toc entry
    m_ps->title_start=m_pp.GetSize();
  if (elem->flags&ElemFmt::LINKDEST) { // link destination
    const wchar_t *id=GetAttr(attr,L"id");
    if (id) {
      wchar_t *copy=m_buffer.Append(id,wcslen(id)+1);
      m_references.Add(copy,(void*)m_pp.GetSize());
    }
  }
  if (elem->flags&ElemFmt::LINK) { // link
    const wchar_t   *dest=GetAttr(attr,XLINK_NS L"|href");
    const wchar_t   *type=GetAttr(attr,L"type");
    ElemFmt	    *linkformat;
    if (type && !wcscmp(type,L"note"))
      linkformat=LookupElem(L"|>footnote");
    else
      linkformat=LookupElem(L"|>link");
    m_ps->ApplyFmt(linkformat);
    if (dest) {
      m_ps->link_start=m_ps->len;
      m_ps->link_name=m_buffer.Append(dest,wcslen(dest)+1);
    } else
      m_ps->link_name=NULL;
  }
  if ((m_ps->enable && elem->flags&ElemFmt::PARA) ||
      elem->flags&ElemFmt::DESCCAT)
  { // start a new paragraph
    m_ps->attr&=~(LEADSP|TRAILSP);
    m_ps->acch_lev++;
    m_ps->pf_start=m_frags.GetSize();
    m_ps->pl_start=m_links.GetSize();
    m_ps->numfrags=0;
    m_ps->len=0;
    m_ps->last_frag_trailsp=false;
    m_ps->last_frag_fmt=m_ps->cfmt.attr;
  }
  if (elem->flags&(ElemFmt::DESCCAT|ElemFmt::DESCITEM)) {
    Frag	  f;
    if (elem->flags&ElemFmt::DESCITEM) { // add a few nbsps
      f.attr=m_ps->Att()|REALLYLOCAL;
      f.len=2;
      f.local[0]=f.local[1]=0xa0; // nbsp
      m_frags.Add(f);
      ++m_ps->numfrags;
      m_ps->len+=f.len;
    }
    // add an element name
    m_ps->ApplyFmt(LookupElem(L">keyword"));
    const wchar_t   *ln=LocalName(ns_name);
    f.len=wcslen(ln);
    f.attr=m_ps->Att()|LOCAL;
    f.str=m_buffer.Append(ln,f.len);
    m_frags.Add(f);
    ++m_ps->numfrags;
    m_ps->len+=f.len;
    m_ps->PopA();
    if (elem->flags&ElemFmt::DESCITEM && attr && *attr) { // print attributes as well
      // add " ("
      f.attr=m_ps->Att()|REALLYLOCAL;
      f.len=2;
      f.local[0]=L' '; f.local[1]=L'(';
      m_frags.Add(f);
      ++m_ps->numfrags;
      m_ps->len+=f.len;
      // iterate over attrbutes
      while (*attr) {
	//attr name
	f.attr=m_ps->Att()|LOCAL;
	ln=LocalName(*attr);
	f.len=wcslen(ln);
	f.str=m_buffer.Append(ln,f.len);
	m_frags.Add(f);
	++m_ps->numfrags;
	m_ps->len+=f.len;
	// "="
	f.attr=m_ps->Att()|REALLYLOCAL;
	f.len=1;
	f.local[0]=L'=';
	m_frags.Add(f);
	++m_ps->numfrags;
	++m_ps->len;
	// value
	f.attr=m_ps->Att()|LOCAL;
	f.len=wcslen(attr[1]);
	f.str=m_buffer.Append(attr[1],f.len);
	m_frags.Add(f);
	++m_ps->numfrags;
	m_ps->len+=f.len;
	attr+=2;
      }
      // add ")"
      f.attr=m_ps->Att()|REALLYLOCAL;
      f.len=1;
      f.local[0]=L')';
      m_frags.Add(f);
      ++m_ps->numfrags;
      m_ps->len+=f.len;
    }
    if (elem->flags&ElemFmt::DESCCAT) {
      // if last frag is a trailsp, then discard the last space
      if (m_ps->last_frag_trailsp) {
	m_ps->len--;
	m_frags[m_frags.GetSize()-1].attr&=~TRAILSP;
      }
      AddP(m_ps->pf_start,m_ps->pl_start,m_ps->start,m_ps->len,m_ps->cfmt);
      m_ps->start+=m_ps->len;
      m_ps->acch_lev--;
    } else { // add a ": "
      f.attr=m_ps->Att()|REALLYLOCAL;
      f.len=2;
      f.local[0]=L':'; f.local[1]=L' ';
      m_frags.Add(f);
      ++m_ps->numfrags;
      m_ps->len+=f.len;
    }
  }
  if (elem->flags&ElemFmt::BINARY) {
    const wchar_t *id=GetAttr(attr,L"id");
    const wchar_t *type=GetAttr(attr,L"content-type");
    if (id && type) {
      m_ps->binary=1;
      Binary  *b=m_binarystorage.Get();
      b->id=m_buffer.Append(id,wcslen(id)+1);
      b->type=m_buffer.Append(type,wcslen(type)+1);
      b->startfrag=m_frags.GetSize();
      b->numfrags=0;
      m_binaries.Add(b->id,(void*)b);
    }
  }
  if (elem->flags&ElemFmt::IMAGE) {
    const wchar_t *href=GetAttr(attr,XLINK_NS L"|href");
    if (href && href[0]==L'#') {
      if (m_ps->acch_lev) { // inline image
	const wchar_t *hcopy=m_buffer.Append(href+1,wcslen(href));
	int	      index=m_inline_images.GetSize();
	m_inline_images.Add(hcopy);
	PushWS();
	if (m_ps->attr&LEADSP)
	  ++m_ps->len;
	Frag    f;
	m_ps->cfmt.attr.img=1;
	f.attr=m_ps->Att()|PE_IMAGE;
	m_ps->cfmt.attr.img=0;
	f.len=1;
	f.local[0]=index;
	f.local[1]=index>>16;
	m_frags.Add(f);
	++m_ps->numfrags;
	++m_ps->len;
	m_ps->last_frag_trailsp=0;
	m_ps->attr&=~(LEADSP|TRAILSP);
	m_ps->last_frag_fmt=m_ps->cfmt.attr;
      } else {
	AddImage(href+1,m_ps->start,m_ps->cfmt);
	// image virtual size is always the same
	m_ps->start+=ImageLoader::IMAGE_VSIZE;

	if (m_docs.GetSize() == 0)
	  m_cover = href+1;
      }
    }
  }
}

void XMLParserImp::EndElement(const wchar_t *ns_name) {
  ProgSetCur(XML_GetCurrentByteIndex(m_parser));
  ElemFmt   *elem=LookupElem(ns_name);

  if (m_ps->enable && elem->flags&ElemFmt::PARA) { // end a paragraph
    // if last frag is a trailsp, then discard the last space
    if (m_ps->last_frag_trailsp) {
      m_ps->len--;
      m_frags[m_frags.GetSize()-1].attr&=~TRAILSP;
    }
    AddP(m_ps->pf_start,m_ps->pl_start,m_ps->start,m_ps->len,m_ps->cfmt);
    m_ps->start+=m_ps->len;
    m_ps->acch_lev--;
  }
  if (elem->flags&ElemFmt::HEADER)
    m_ps->PopA();
  if (elem->flags&ElemFmt::STYLE)
    m_ps->PopA();
  if (elem->flags&ElemFmt::STYLESHEET && m_ps->in_stylesheet)
      --m_ps->in_stylesheet;
  if (elem->flags&ElemFmt::FMT) // apply formatting
    m_ps->PopA();
  if (elem->flags&ElemFmt::LINK) { // link
    if (m_ps->link_name) {
      XMLParserImp::Link	link;
      link.start=m_ps->link_start;
      link.length=m_ps->len-m_ps->link_start;
      link.target=m_ps->link_name;
      m_links.Add(link);
    }
    m_ps->PopA();
  }
  if (elem->flags&ElemFmt::SECTION) // snag title attribute
    m_ps->section_nest--;
  if (elem->flags&ElemFmt::SPACE) // add spaces after this element
    m_ps->attr|=LEADSP;
  if (elem->flags&ElemFmt::TITLE) {
    if (m_docs.GetSize()>0 && m_ps->section_nest)
      AddToc(
	FilePos(
	  m_ps->title_start-m_docs[m_docs.GetSize()-1].start,
	  m_pp.GetSize()-m_ps->title_start,
	  m_docs.GetSize()-1
	),
	m_ps->section_nest
      );
  }
  if (elem->flags&ElemFmt::AELINE) // add an empty line after the element
    AddQ(m_ps->start);
  if (elem->flags&ElemFmt::ENABLE)
    --m_ps->enable;
  if (elem->flags&ElemFmt::DOCUMENT) {
    if (m_docs.GetSize()>0)
      m_docs[m_docs.GetSize()-1].length=
	m_pp.GetSize()-m_docs[m_docs.GetSize()-1].start;
    AddQ(m_ps->start);
  }
  if (elem->flags&ElemFmt::BINARY && m_ps->binary) {
    m_ps->binary=0;
    int	  idx=m_binarystorage.GetSize()-1;
    m_binarystorage[idx].numfrags=m_frags.GetSize()-m_binarystorage[idx].startfrag;
  }
}

void XMLParserImp::CharData(const wchar_t *text,int len) {
  if (m_ps->acch_lev && len) {
    int l=normalized_length(text,len);
    if (!l) { // whitespace frag, try to add a leading space to the next frag
      if (m_ps->numfrags) {
	m_ps->attr|=LEADSP;
	m_ps->last_frag_fmt=m_ps->cfmt.attr;
      }
      return;
    }

    PushWS();

    if (*text<=32 && m_ps->numfrags)
      m_ps->attr|=LEADSP;
    if (text[len-1]<=32) {
      m_ps->attr|=TRAILSP;
      m_ps->len++;
    }
    m_ps->len+=l;
    if (m_ps->attr&LEADSP)
      m_ps->len++;
    // here we check the previous frag and if it has TRAILSP and this has a LEADSP,
    // and their charformats are the same, then we can discard current LEADSP
    if (m_ps->numfrags && m_ps->last_frag_trailsp && m_ps->attr&LEADSP &&
	m_ps->last_frag_fmt==m_ps->cfmt.attr)
    {
      m_ps->attr&=~LEADSP;
      m_ps->len--;
    }
    Frag    f;
    f.attr=m_ps->Att();
    if (l<5 || XML_IsExpanding(m_parser)) { // cache short fragments
      wchar_t	*buf;
      if (l <= sizeof(f.local)/sizeof(wchar_t)) {
	f.attr|=REALLYLOCAL;
	buf=f.local;
      } else {
	f.attr|=LOCAL;
	buf=m_buffer.Get(l);
      }
      f.str=buf;
      f.len=l;
      normalize_space(buf,l,text,len);
    } else {
      f.len=XML_GetCurrentByteCount(m_parser);
      f.fpos=XML_GetCurrentByteIndex(m_parser);
    }
    m_frags.Add(f);
    ++m_ps->numfrags;
    m_ps->last_frag_trailsp=(m_ps->attr&TRAILSP)!=0;
    m_ps->attr&=~(LEADSP|TRAILSP);
    m_ps->last_frag_fmt=m_ps->cfmt.attr;
  }
  if (m_ps->in_stylesheet && len)
    ParseStylesheet(text,len);
  if (m_ps->binary && len && !iswhitespace(text,len)) {
    Frag  f;
    f.attr=m_ps->attr&CDATA; // text attrs are not needed in this context
    if (XML_IsExpanding(m_parser)) {
      f.str=m_buffer.Get(len);
      memcpy((wchar_t*)f.str,text,len*sizeof(wchar_t));
      f.attr|=LOCAL;
    } else {
      f.len=XML_GetCurrentByteCount(m_parser);
      f.fpos=XML_GetCurrentByteIndex(m_parser);
    }
    m_frags.Add(f);
  }
}

// expat callbacks
void	  XMLParserImp::StartElementCB(void *udata,const wchar_t *name,
				       const wchar_t **attr)
{
  ((XMLParserImp*)udata)->StartElement(name,attr);
}

void	  XMLParserImp::EndElementCB(void *udata,const wchar_t *name) {
  ((XMLParserImp*)udata)->EndElement(name);
}

void	  XMLParserImp::CharDataCB(void *udata,const wchar_t *text,int len) {
  ((XMLParserImp*)udata)->CharData(text,len);
}

void	  XMLParserImp::StartCDataCB(void *udata) {
  ((XMLParserImp*)udata)->m_ps->attr|=CDATA;
}

void	  XMLParserImp::EndCDataCB(void *udata) {
  ((XMLParserImp*)udata)->m_ps->attr&=~CDATA;
}

int XMLParserImp::UnknownEncodingCB(void *data,const wchar_t *name,
				    XML_Encoding *info)
{
  int	cp=Unicode::FindCodePage(Unicode::ToCS(name,wcslen(name)));
  const wchar_t	*tab=NULL;
  if (cp>=0)
    tab=Unicode::GetTable(cp);
  if (!tab)
    return 0;
  info->data=NULL;
  info->convert=NULL;
  info->release=NULL;
  for (int i=0;i<256;++i)
    info->map[i]=tab[i];
  ((XMLParserImp*)data)->m_encoding=cp;
  return 1;
}

bool	  XMLParserImp::ParseFile(int encoding) {
  XML_Memory_Handling_Suite  xmm={ my_malloc, my_realloc, my_free };
  xmm.priv=(void*)m_heap;

  ParseState	    pstate;
  memset(&pstate,0,sizeof(pstate));
  m_ps=&pstate;
  FmtArray	    docstyles;
  docstyles.SetSize(0,64);
  WMap		    docstylemap(m_heap,true);
  pstate.styles=&docstyles;
  pstate.stylemap=&docstylemap;

  SP_State	    spstate;
  m_sps=&spstate;

  const wchar_t	*enc=NULL;
  if (encoding>=0) {
    enc=Unicode::GetCodePageNameW(encoding);
    if (!enc)
      encoding=-1;
  }
  if (!(m_parser=XML_ParserCreate_MM(enc,&xmm,L"|")))
    return false;
  XML_SetElementHandler(m_parser,XMLParserImp::StartElementCB,
    XMLParserImp::EndElementCB);
  XML_SetCharacterDataHandler(m_parser,XMLParserImp::CharDataCB);
  XML_SetCdataSectionHandler(m_parser,StartCDataCB,EndCDataCB);
  XML_SetUnknownEncodingHandler(m_parser,XMLParserImp::UnknownEncodingCB,this);
  XML_SetUserData(m_parser,this);
  m_encoding=-1;
  // now suck in the file
  m_fp->seek(0);
  switch (setjmp(pstate.jout)) {
  case 0:
    for (;;) {
      void    *buf=(char*)XML_GetBuffer(m_parser,RFile::BSZ);
      if (!buf) {
	CTVApp::Barf(_T("XML parser: Out of memory"));
	goto fail;
      }
      int	    nr=m_fp->read(buf,RFile::BSZ);
      if (!XML_ParseBuffer(m_parser,nr,nr<RFile::BSZ)) {
	CTVApp::Barf(
  #ifdef UNICODE
	  _T("XML parse error: %s at line %d, column %d"),
  #else
	  _T("XML parse error: %S at line %d, column %d"),
  #endif
	  XML_ErrorString(XML_GetErrorCode(m_parser)),
	  XML_GetCurrentLineNumber(m_parser),XML_GetCurrentColumnNumber(m_parser)+1);
	goto fail;
      }
      if (nr<RFile::BSZ)
	break;
    }
    break;
  case ERR_NOTFB2:
    CTVApp::Barf(_T("Not a FictionBook2 document"));
    goto fail;
  }
  if (m_docs.GetSize()>0) {
    Document    d;
    d.start=0;
    d.length=m_docs[0].start-1;
    d.name=_T("#Description");
    m_docs.Add(d);
  }
  return true;
fail:
  //XML_ParserFree(m_parser);
  // parser is now destroyed in the XMLParserImp destructor
  return false;
}

XMLParser *XMLParser::MakeParser(Meter *m,CBufFile *fp,Bookmarks *bmk,HANDLE heap) {
  return new XMLParserImp(m,fp,bmk,heap);
}

void	  XMLParserImp::ParseStylesheet(const wchar_t *text,int len) {
  wchar_t   ch;
  while (len--) {
    ch=*text++;
    switch (m_sps->state) {
    case SP_State::START:
      if (!iswspace(ch)) {
	m_sps->NAdd(ch);
	m_sps->state=SP_State::NAME;
      }
      break;
    case SP_State::NAME:
      if (!iswspace(ch))
	m_sps->NAdd(ch);
      else
	m_sps->state=SP_State::FLAGS;
      break;
    case SP_State::FLAGS:
flags:
      switch (ch) {
      case L'b':
	m_sps->format.bold=1;
	break;
      case L'B':
	m_sps->format.bold=0;
	break;
      case L'i':
	m_sps->format.italic=1;
	break;
      case L'I':
	m_sps->format.italic=0;
	break;
      case L'u':
	m_sps->format.underline=1;
	break;
      case L'U':
	m_sps->format.underline=0;
	break;
      case L'r':
	m_sps->format.align=Paragraph::right;
	break;
      case L'c':
	m_sps->format.align=Paragraph::center;
	break;
      case L'j':
	m_sps->format.align=Paragraph::justify;
	break;
      case L'l':
	m_sps->format.align=0;
	break;
      case L'h':
	m_sps->state=SP_State::COLOR;
	m_sps->num=0; m_sps->sign=false;
	break;
      case L'L':
	m_sps->state=SP_State::LM;
	m_sps->num=0; m_sps->sign=false;
	break;
      case L'R':
	m_sps->state=SP_State::RM;
	m_sps->num=0; m_sps->sign=false;
	break;
      case L'F':
	m_sps->state=SP_State::FM;
	m_sps->num=0; m_sps->sign=false;
	break;
      case L'S':
	m_sps->state=SP_State::SIZE;
	m_sps->num=0; m_sps->sign=false;
	break;
      case L';':
	m_sps->stylename[m_sps->stylenameptr]=L'\0';
	m_ps->stylemap->AddCopy(m_sps->stylename,(void*)m_ps->styles->GetSize());
	m_ps->styles->Add(m_sps->format);
	m_sps->Init();
	break;
      }
      break;
    case SP_State::FM:
    case SP_State::LM:
    case SP_State::RM:
    case SP_State::SIZE:
    case SP_State::COLOR:
      if (iswdigit(ch)) {
	m_sps->num=m_sps->num*10+ch-L'0';
      } else if (ch=='-') 
	m_sps->sign=1;
      else { // end of spec
	int   val=m_sps->num;
	if (m_sps->sign)
	  val=-val;
	switch (m_sps->state) {
	case SP_State::FM:
	  val=RClamp(val,-100,100);
	  m_sps->format.findent=val;
	  break;
	case SP_State::LM:
	  val=RClamp(val,-100,100);
	  m_sps->format.lindent=val;
	  break;
	case SP_State::RM:
	  val=RClamp(val,-100,100);
	  m_sps->format.rindent=val;
	  break;
	case SP_State::COLOR:
	  val=RClamp(val,0,7);
	  m_sps->format.color=val;
	  break;
	case SP_State::SIZE:
	  val=RClamp(val,-100,100);
	  m_sps->format.fsz=val;
	  break;
	}
	m_sps->state=SP_State::FLAGS;
	goto flags;
      }
      break;
    }
  }
}

void XMLParser::SaveStyles() {
  for (int i=0;i<g_eformat.GetSize();++i)
    if (g_eformat[i].name.GetLength() && g_eformat[i].name[0]!=_T('*')) {
      CString	str;
      str.Format(_T("%d,%d,%d,%d,%d,%d,%d,%d,%d"),
	g_eformat[i].fsz,
	g_eformat[i].bold,
	g_eformat[i].italic,
	g_eformat[i].underline,
	g_eformat[i].color,
	g_eformat[i].align,
	g_eformat[i].lindent,
	g_eformat[i].rindent,
	g_eformat[i].findent);
      AfxGetApp()->WriteProfileString(_T("Styles"),g_eformat[i].name,str);
    }
}


static void AddElement(const wchar_t *name,int namelen,int id) {
  wchar_t   buf[128];
  wcscpy(buf,FB_NS);
  buf[FB_NS_LEN]=L'|';
  if (namelen>sizeof(buf)/sizeof(wchar_t)-2-FB_NS_LEN)
    namelen=sizeof(buf)/sizeof(wchar_t)-2-FB_NS_LEN;
  wcsncpy(buf+FB_NS_LEN+1,name,namelen);
  buf[FB_NS_LEN+1+namelen]=L'\0';
  g_elements->AddCopy(buf,(void*)id);
  g_elements->AddCopy(buf+FB_NS_LEN,(void*)id);
  g_elements->AddCopy(buf+FB_NS_LEN+1,(void*)id);
}

static inline int FLAG(wchar_t x) {
  return x==L'+' ? 1 : x==L'-' ? 0 : XMLParser::ElemFmt::NOCHG;
}

static void ParseXMLSettings(const wchar_t *fmt,DWORD sz) {
  const wchar_t  *p=fmt;
  const wchar_t  *e=fmt+sz;
  const wchar_t  *le;
  wchar_t	 *q;

  if (p<e && *p==0xFEFF)
    ++p;
  while (p<e) {
    for (le=p;le<e && *le!=L'\r' && *le!=L'\n';++le) ;
    // process line here
    if (p!=le && *p!=L'#' && *p!=L' ' && *p!=L'\t') {
      XMLParser::ElemFmt	fe;
      wchar_t			*name=NULL;
      int			size;
      wchar_t			bold,italic,underline;
      int			color;
      wchar_t			align;
      int			leftm,rightm,firstm;
      wchar_t			*action=NULL,*elements=NULL;
      int			fields;
      if (Scan::xscanf(p,le-p,L"%S %-100,100,127d "
			L"%c %c %c %0,8,127d %c %-100,100,127d "
			L"%-100,100,127d %-100,100,127d %S %S",NULL,&fields,
			&name,&size,&bold,&italic,&underline,
			&color,&align,&leftm,&rightm,&firstm,
			&action,&elements)==0 && fields==12)
      {
	for (q=name;*q;++q)
	  if (*q==L'_')
	    *q=L' ';
	fe.name=name;
	fe.fsz=size;
	fe.bold=FLAG(bold);
	fe.italic=FLAG(italic);
	fe.underline=FLAG(underline);
	fe.color=color;
	switch (align) {
	case 'R':
	  fe.align=Paragraph::right;
	  break;
	case 'C':
	  fe.align=Paragraph::center;
	  break;
	case 'J':
	  fe.align=Paragraph::justify;
	  break;
	default:
	  fe.align=XMLParser::ElemFmt::NOCHG;
	  break;
	}
	fe.lindent=leftm;
	fe.rindent=rightm;
	fe.findent=firstm;
	fe.flags=0;
	for (q=action;*q;++q) {
	  const TCHAR *fp=XMLParser::ElemFmt::flag_names;
	  for (int flags=1;*fp;flags<<=1,++fp)
	    if (*fp==*q) {
	      fe.flags|=flags;
	      break;
	    }
	}
	for (p=elements;*p;) {
	  const wchar_t *q; // XXX
	  for (q=p;*q && *q!=L',';++q) ;
	  if (q!=p)
	    AddElement(p,q-p,g_eformat.GetSize());
	  p=q;
	  if (*p==L',')
	    ++p;
	}
	// ok, insert it now
	g_eformat.Add(fe);
      }
      delete[] name;
      delete[] action;
      delete[] elements;
    }
    // skip end of line
    for (p=le;p<e && (*p=='\r' || *p=='\n');++p) ;
  }
}

void XMLParser::LoadStyles() {
  ElemFmt ef;
  // delete all elements
  g_eformat.RemoveAll();
  delete g_elements;
  g_elements=new WMap(GetProcessHeap(),true);
  // always initialize a default element
  ef.Clear();
  g_eformat.Add(ef);
  // read parser settings from a resource
  HMODULE hMod=AfxGetResourceHandle();
  HRSRC	  hRes=FindResource(hMod,_T("xml_elements"),RT_RCDATA);
  if (hRes) {
    DWORD   rsize=SizeofResource(hMod,hRes);
    HGLOBAL hGlob=LoadResource(hMod,hRes);
    if (hGlob) {
      void  *res=LockResource(hGlob);
      if (res) {
	ParseXMLSettings((const wchar_t *)res,rsize/sizeof(wchar_t));
	UnlockResource(hGlob);
      }
      FreeResource(hGlob);
    }
  }
  // adjust formatting from registry
  for (int fnum=0;fnum<g_eformat.GetSize();++fnum)
    if (g_eformat[fnum].name.GetLength()>0 && g_eformat[fnum].name[0]!=_T('*')) {
      CString	str=AfxGetApp()->GetProfileString(_T("Styles"),g_eformat[fnum].name);
      int	f,b,i,c,a,li,ri,fi,u;
      if (_stscanf(str,_T("%d,%d,%d,%d,%d,%d,%d,%d,%d"),&f,&b,&i,&u,&c,&a,&li,&ri,&fi)==9) {
	g_eformat[fnum].fsz=f;
	g_eformat[fnum].bold=b;
	g_eformat[fnum].italic=i;
	g_eformat[fnum].underline=u;
	g_eformat[fnum].color=c;
	g_eformat[fnum].align=a;
	g_eformat[fnum].lindent=li;
	g_eformat[fnum].rindent=ri;
	g_eformat[fnum].findent=fi;
      }
    }
}

XMLParser::FmtArray&  XMLParser::GetXMLStyles() {
  return g_eformat;
}

void	XMLParser::ElemFmt::Clear() {
    fsz=NOCHG;
    bold=NOCHG;
    italic=NOCHG;
    color=NOCHG;
    align=NOCHG;
    underline=NOCHG;
    flags=0;
    lindent=NOCHG;
    rindent=NOCHG;
    findent=NOCHG;
    name.Empty();
}
