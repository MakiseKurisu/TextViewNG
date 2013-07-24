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
 * $Id: TextParser.cpp,v 1.53.2.4 2003/10/21 23:26:24 mike Exp $
 * 
 */

#define _WIN32_WINNT	_WIN32_WINNT_MAXVER

#include <afxtempl.h>
#include <afxcmn.h>

#include "TextParser.h"
#include "Unicode.h"
#include "FastArray.h"
#include "Image.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

struct Para { // paragraph reference
  DWORD     off; // offset into file
  DWORD     start; // start of parsed para
  DWORD	    rlen; // raw length
};

typedef FastArray<Para>	PArray;

struct SimpleFormat {
  const TCHAR		*name;
  DWORD			mask,cmp;
};

#define	MAXPLEN	4096

// soft hyphen
#define	SHY		0x00AD

// Moshkov's formatting
#define	STARTBOLD	20
#define	ENDBOLD		21

// formats list
static SimpleFormat	g_simple_formats[]={
  { _T("Line per paragraph"), 0xff, 0x0a },
  { _T("Indented first line"), 0xffff, 0x0a20 },
  { _T("MAC Line per paragraph"), 0xff, 0x0d },
  { _T("MAC Indented first line"), 0xffff, 0x0d20 }
};
#define	NUM_SIMPLE_FORMATS  (sizeof(g_simple_formats)/sizeof(g_simple_formats[0]))

static const TCHAR	*g_ext_formats[]={
//  _T("XML"),
  _T("PNG"),
  _T("JPEG")
};
#define	NUM_EXT_FORMATS  (sizeof(g_ext_formats)/sizeof(g_ext_formats[0]))

//#define	XML_FORMAT  (NUM_SIMPLE_FORMATS)
#define	PNG_FORMAT  (NUM_SIMPLE_FORMATS)
#define	JPEG_FORMAT  (NUM_SIMPLE_FORMATS+1)

TextParser::~TextParser() {
  if (m_heap)
    HeapDestroy(m_heap);
}

class SimpleTextParser : public TextParser {
protected:
  PArray		m_pp; // paragraph list
  SimpleFormat		*m_sf;

  void	    GenericFileParse(CBufFile *fp,PArray& pp,DWORD mask,DWORD cmp);
  void	    GenericFileParseA(CBufFile *fp,PArray& pp,DWORD mask,DWORD cmp);
  void	    GenericFileParseW(CBufFile *fp,PArray& pp,DWORD mask,DWORD cmp);
  Paragraph GenericBufParse(CBufFile *fp,const Para& p,int len);
public:
  SimpleTextParser(Meter *m,CBufFile *fp,HANDLE heap,int format,int encoding,Bookmarks *bmk) :
      TextParser(m,fp,heap,bmk), m_sf(&g_simple_formats[format]), m_pp(heap)
  {
    m_format=format;
    m_encoding=encoding;
    if (m_encoding<0) {
      char    tmp[1024];
      DWORD   sp=m_fp->pos();
      int nb=m_fp->read(tmp,sizeof(tmp));
      m_encoding=Unicode::DetectCodePage(tmp,nb);
	  m_fp->seek(sp);
    }
    GenericFileParse(m_fp,m_pp,m_sf->mask,m_sf->cmp);
    if (bmk) {
      for (int i=0;i<Length(0)-1;++i)
	if ((i==0 || GetPLength(0,i-1)==0) && GetPLength(0,i+1)==0 &&
	    GetPLength(0,i)>0 && GetPLength(0,i)<90)
	  bmk->AddTocEnt(i,1,0,FilePos(i,0,0),0);
    }
  }

  // paragraphs
  virtual int		Length(int docid) { return m_pp.GetSize()-1; } // in paragraphs
  virtual Paragraph	GetParagraph(int docid,int para) {
    if (para>=0 && para<m_pp.GetSize()-1)
      return GenericBufParse(m_fp,m_pp[para],m_pp[para+1].start-m_pp[para].start);
    return Paragraph();
  }
  virtual int		GetPLength(int docid,int para) {
    if (para<0 || para>=m_pp.GetSize()-1)
      return 0;
    return m_pp[para+1].start-m_pp[para].start;
  }
  virtual int		GetPStart(int docid,int para) {
    if (para<0)
      return 0;
    if (para>=m_pp.GetSize()-1)
      return m_pp[m_pp.GetSize()-1].start;
    return m_pp[para].start;
  }
  virtual int		GetTotalLength(int docid) {
    return m_pp[m_pp.GetSize()-1].start;
  }
  virtual int		LookupParagraph(int docid,int charpos);
};

int   SimpleTextParser::LookupParagraph(int docid,int charpos) {
  if (charpos<0 || charpos>(int)m_pp[m_pp.GetSize()-1].start)
    return 0;
  if (charpos==(int)m_pp[m_pp.GetSize()-1].start)
    return m_pp.GetSize()-1;
  int	i=0,j=m_pp.GetSize()-1;
  while (i<=j) {
    int	  m=(i+j)>>1;
    if (charpos<(int)m_pp[m].start)
      j=m-1;
    else if (charpos>=(int)m_pp[m+1].start)
      i=m+1;
    else
      return m;
  }
  return 0;
}

#define	RSPACE(x)   ((x)<=32)
#define	SPACE(x)    (RSPACE(x) || (x)==SHY || (x)==0xFEFF)

static void Addpara(int enc,PArray& pp,Buffer<char>& b,int& parsed_start,int len,DWORD start)
{
	// convert to unicode
	int wclen=Unicode::WCLength(enc,b,len);
	Buffer<wchar_t> wb(wclen);
	Unicode::ToWC(enc,b,len,wb,wclen);
	// now count length
	int i,plen=0;
	// skip leading spaces
	for (i=0;i<wclen && SPACE(wb[i]);++i);
	// count length
	while (i<wclen)
	{
		// word
		while (i<wclen && !RSPACE(wb[i]))
		{
			if (wb[i]!=SHY && wb[i]!=0xFEFF)
			{
				++plen;
			}
			++i;
		}
		// spaces
		while (i<wclen && SPACE(wb[i]))
		{
			++i;
		}
		if (i<wclen) // this was not trailing space
		{
			++plen;
		}
	}

	Para	p;
	p.start=parsed_start;
	p.rlen=len;
	p.off=start;
	pp.Add(p);
	parsed_start+=plen;
}

void   SimpleTextParser::GenericFileParseW(CBufFile *fp,PArray& pp,DWORD mask,DWORD cmp)
{
	int				ch=0;
	int				ch2=0;
	DWORD			hist=0;
	Buffer<char>	b(MAXPLEN);
	int				rlen=0;
	DWORD			start=fp->pos();
	int				parsed_start=0;
	for (;;) 
	{
		ch=fp->ch();
		ch2=fp->ch();
		if (ch==BEOF && ch2==BEOF)
		{
			Addpara(m_encoding,pp,b,parsed_start,rlen,start);
			break;
		}

		hist=hist<<8|ch;
		hist=hist<<8|ch2;
		if (hist==0x0d000a00 || hist==0x0d000d00|| hist==0x0a000a00 || (hist&mask)==cmp || rlen>=MAXPLEN)
		{
			bool f=rlen>=MAXPLEN;
			ProgSetCur(fp->pos());
			Addpara(m_encoding,pp,b,parsed_start,rlen,start);
			rlen=0;
			start=m_fp->pos();
			if (f)
			{
				b[rlen++]=ch;
				b[rlen++]=ch2;
			}
		}
		else
		{
			b[rlen++]=ch;
			b[rlen++]=ch2;
		}
	}

	Para	p;
	p.off=0;
	p.rlen=0;
	p.start=parsed_start;
	pp.Add(p);
}

void   SimpleTextParser::GenericFileParseA(CBufFile *fp,PArray& pp,DWORD mask,DWORD cmp)
{
	int				ch=0;
	DWORD			hist=0;
	Buffer<char>	b(MAXPLEN);
	int				rlen=0;
	DWORD			start=fp->pos();
	int				parsed_start=0;
	for (;;) 
	{
		ch=fp->ch();
		if (ch==BEOF)
		{
			Addpara(m_encoding,pp,b,parsed_start,rlen,start);
			break;
		}

		hist=hist<<8|ch;
		if ((hist&0xffff)==0x0d0a || (hist&0xffff)==0x0d0d || (hist&0xffff)==0x0a0a || (hist&mask)==cmp || rlen>=MAXPLEN)
		{
			bool f=rlen>=MAXPLEN;
			ProgSetCur(fp->pos());
			Addpara(m_encoding,pp,b,parsed_start,rlen,start);
			rlen=0;
			start=m_fp->pos();
			if (f)
			{
				b[rlen++]=ch;
			}
		}
		else
		{
			b[rlen++]=ch;
		}
	}

	Para	p;
	p.off=0;
	p.rlen=0;
	p.start=parsed_start;
	pp.Add(p);
}

void   SimpleTextParser::GenericFileParse(CBufFile *fp,PArray& pp,DWORD mask,DWORD cmp)
{
	if (Unicode::GetCodePage(m_encoding)==CP_UTF16)
	{
		GenericFileParseW(fp,pp,mask,cmp);
	}
	else
	{
		GenericFileParseA(fp,pp,mask,cmp);
	}
}

// generic buffer parser for all simple formats
Paragraph  SimpleTextParser::GenericBufParse(CBufFile *fp,const Para& p,int len) {

  if (!len)
    return Paragraph();

  // read entire buffer
  Buffer<char>	  mbbuf(p.rlen);
  fp->seek(p.off);
  int		  nread=fp->read(mbbuf,p.rlen);
  ASSERT(nread==(int)p.rlen);
  // convert to unicode here
  int		  wclen=Unicode::WCLength(m_encoding,mbbuf,nread);
  Buffer<wchar_t> wcbuf(wclen);
  Unicode::ToWC(m_encoding,mbbuf,nread,wcbuf,wclen);
  // strip whitespace and soft hyphens
  Paragraph	  ret(len);
  wchar_t	  *bp=ret.str;
  Attr		  *cfp=ret.cflags;
  Attr		  fmt;
  int		  count=0;
  int		  i;

  ret.cflags.Zero();
  fmt.wa=0;
  // skip leading spaces
  for (i=0;i<wclen && SPACE(wcbuf[i]);++i);
  // copy text
  while (i<wclen && count<len) {
    // copy word
    while (i<wclen && count<len && !RSPACE(wcbuf[i])) {
      if (wcbuf[i]!=SHY)
	bp[count++]=wcbuf[i];
      else { // handle hyphenation
	// XXX
      }
      ++i;
    }
    // skip spaces
    while (i<wclen && SPACE(wcbuf[i]))
      ++i;
    if (i<wclen && count<len) // not a trailing space
      bp[count++]=' ';
  }
  ASSERT(len==count);
  //p.len=count; // update paragraph length
  ret.str.setsize(len);
  ret.len=len;
  ret.findent=3; // XXX
  return ret;
}

// XXX depends on the order of records in g_simple_formats
int		TextParser::DetectFormat(CBufFile *fp) {
  int		lines,ws,chars,check=_T('\n'),base=0;
  Buffer<BYTE>	buf(2048);
  int		nb;

  fp->seek(0);
  nb=fp->read(buf,2048);
  /* check if this is some sort of xml
  if (nb>8 && (memcmp("<?xml",buf,5)==0 || memcmp("\xef\xbb\xbf<?xml",buf,8)==0))
    return XML_FORMAT; */
  /* check for png */
  if (nb>=4 && memcmp("\x89PNG",buf,4)==0)
    return PNG_FORMAT;
  /* check for jpeg */
  if (nb>=10 && buf[0]==0xff && buf[1]==0xd8 &&
      (memcmp("JFIF",buf+6,4)==0 || memcmp("Exif",buf+6,4)==0))
    return JPEG_FORMAT;
  /* check if this is macintosh crap with their CR madness */
  for (chars=0;chars<nb;++chars) {
    if (buf[chars]=='\n')
      goto ok;
  }
  check='\r';
  base=2;
ok:
  /* we read first 50 lines and if more than 3 of them start with spaces,
  and there are no lines longer than 80 chars, then this a spaced format */
  int n=0;
  for (lines=ws=0;lines<50;++lines) {
    for (chars=0;;++chars) {
      if (n>=nb) {
	++lines;
	goto done;
      }
      int     ch=buf[n++];
      if (chars==0 && ch==_T(' '))
	++ws;
      if (ch==check)
	break;
    }
    if (chars>80 && lines>5) /* got a long line, but after the first five */
      return base;
  }
done:
  if (lines>10 && ws>3)
    return base+1;
  return base;
}

int		TextParser::GetNumFormats() {
  return NUM_SIMPLE_FORMATS+1;
}

const TCHAR	*TextParser::GetFormatName(int format) {
  if (format<0 || format>=NUM_SIMPLE_FORMATS+NUM_EXT_FORMATS)
    return _T("Invalid format ID");
  if (format>=NUM_SIMPLE_FORMATS)
    return g_ext_formats[format-NUM_SIMPLE_FORMATS];
  return g_simple_formats[format].name;
}

/* Image parser */
class ImageParser : public TextParser, public ImageLoader::BinReader {
protected:
  Image	  m_cache;
  int	  m_crot,m_cmaxw,m_cmaxh;
public:
  ImageParser(Meter *m,CBufFile *fp,HANDLE heap,int format,int encoding,Bookmarks *bmk) :
      TextParser(m,fp,heap,bmk)
  {
    m_format=format;
    m_encoding=encoding;
    m_cache.hBmp=0;
  }
  ~ImageParser() {
    if (m_cache.hBmp)
      DeleteObject(m_cache.hBmp);
  }

  // paragraphs
  virtual int		Length(int docid) { return 1; } // in paragraphs
  virtual Paragraph	GetParagraph(int docid,int para);
  virtual int		GetPLength(int docid,int para) { return para==0 ? 32 : 0; }
  virtual int		GetPStart(int docid,int para) { return 0; }
  virtual int		GetTotalLength(int docid) { return 32; }
  virtual int		LookupParagraph(int docid,int charpos) { return 0; }

  // images
  virtual bool		GetImage(const wchar_t *name,HDC hDC,
  int maxwidth,int maxheight,int rotation,Image& img);
  virtual void		InvalidateImageCache() {
    if (m_cache.hBmp) {
      DeleteObject(m_cache.hBmp);
      m_cache.hBmp=0;
    }
  }
  virtual bool		IsImage() { return true; }

  // BinReader interface
  virtual int		Read(void *buffer,int count) {
    return m_fp->read(buffer,count);
  }
};

Paragraph ImageParser::GetParagraph(int docid,int para) {
  if (para!=0)
    return Paragraph();
  Paragraph	p(ImageLoader::IMAGE_VSIZE);
  p.lindent=p.rindent=p.findent=0;
  for (int i=0;i<ImageLoader::IMAGE_VSIZE;++i) {
    p.str[i]=L' ';
    p.cflags[i].wa=0;
  }
  // abuse links for image href
  p.links=Buffer<Paragraph::Link>(1);
  p.links[0].off=0;
  p.links[0].len=ImageLoader::IMAGE_VSIZE;
  p.links[0].target=L"1";
  p.flags=Paragraph::image;
  return p;
}

bool	ImageParser::GetImage(const wchar_t *name,HDC hDC,
  int maxwidth,int maxheight,int rotation,Image& img)
{
  if (!name || wcscmp(name,L"1"))
    return false;
  if (m_cache.hBmp &&
      (m_cmaxw==maxwidth || (m_cache.width<m_cmaxw && m_cache.width<maxwidth)) &&
      (m_cmaxh==maxheight || (m_cache.height<m_cmaxh && m_cache.height<maxheight)) &&
      m_crot==rotation)
  {
    img=m_cache;
    return true;
  }
  if (m_cache.hBmp) {
    DeleteObject(m_cache.hBmp);
    m_cache.hBmp=0;
  }
  m_fp->seek(0);
  bool ret=ImageLoader::Load(hDC,
    m_format==PNG_FORMAT ? L"image/png" : L"image/jpeg",
    this,maxwidth,maxheight,rotation,
    m_cache.hBmp,m_cache.width,m_cache.height);
  if (ret) {
    img=m_cache;
    m_crot=rotation;
    m_cmaxh=maxheight;
    m_cmaxw=maxwidth;
  } else
    m_cache.hBmp=0;
  return ret;
}


TextParser	*TextParser::Create(Meter *m,CBufFile *fp,int format,int encoding,Bookmarks *bmk) {
	if (format<0)
		return NULL;
	HANDLE    heap;

	heap=HeapCreate(HEAP_NO_SERIALIZE,1048576*4,0); // reserve up to 4 megs of ram
	if (!heap)
		return NULL;
	TRY {
		switch (format)
		{
		case PNG_FORMAT:
		case JPEG_FORMAT: /* Images */
			return new ImageParser(m,fp,heap,format,encoding,bmk);
			break;
		/*
		case XML_FORMAT:
			XMLParser *p=XMLParser::MakeParser(m,fp,bmk,heap);
			p->m_format=NUM_SIMPLE_FORMATS;
			if (p->ParseFile(encoding))
				return p;
			delete p;
			break;
		*/
		default:	//format<NUM_SIMPLE_FORMATS
			fp->seek(0);
			return new SimpleTextParser(m,fp,heap,format,encoding,bmk);
		}
	} CATCH_ALL(e) {
		HeapDestroy(heap);
		THROW_LAST();
	}
	END_CATCH_ALL
	return NULL;
}

// hyphenation code by Mark Lipsman, modified my Mike

static BYTE   vlist[0x92]={
//0 1 2 3 4 5 6 7 8 9 a b c d e f
  0,1,0,0,1,0,1,1,0,0,0,0,0,0,1,0, // 0x400-0x40f
  1,0,0,0,0,1,0,0,1,0,0,0,0,0,1,0, // 0x410-0x41f
  0,0,0,1,0,0,0,0,0,0,2,1,3,1,1,1, // 0x420-0x42f
  1,0,0,0,0,1,0,0,1,0,0,0,0,0,1,0, // 0x430-0x43f
  0,0,0,1,0,0,0,0,0,0,2,1,3,1,1,1, // 0x440-0x44f
  0,1,0,0,1,0,1,1,0,0,0,0,0,0,1,0, // 0x450-0x45f
};

#define isLetter(ch)	((ch)>=0x0401 && (ch)<=0x0491)
#define isVowel(ch)	(vlist[(ch)-0x400]==1)
#define isHardSign(ch)	(vlist[(ch)-0x400]==2) 
#define isSoftSign(ch)	(vlist[(ch)-0x400]==3)
#define isConsonant(ch)	(vlist[(ch)-0x400]==0)

void  Paragraph::Hyphenate() {
  if (flags&hypdone)
    return;
  flags|=hypdone;

  DWORD		len=str.size();
  if (!len)
    return;
  const wchar_t	*s=str;
  Attr		*a=cflags;
  DWORD		start,end,i,j;

  for (start=0;start<len;) {
    // find start of word
    while (start<len && !isLetter(s[start]))
      ++start;
    // find end of word
    for (end=start+1;end<len && isLetter(s[end]);++end) ;
    // now look over word, placing hyphens
    if (end-start>3) // word must be long enough
      for (i=start;i<end-3;++i)
	if (isVowel(s[i]))
	  for (j=i+1;j<end;++j)
	    if (isVowel(s[j])) {
	      if (isConsonant(s[i+1]) && isConsonant(s[i+2]))
		++i;
	      else if (isConsonant(s[i+1]) && 
			(isHardSign(s[i+2]) || isSoftSign(s[i+2])))
		i+=2;
	      if (i-start>1 && end-i>2)
		a[i+1].hyphen=true;
	      break;
	    }
    start=end;
  }
}
