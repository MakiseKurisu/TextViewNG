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
 * $Id: FDC.cpp,v 1.18.2.5 2003/09/27 11:30:55 mike Exp $
 * 
 */

#define _WIN32_WINNT	_WIN32_WINNT_MAXVER

#include <afx.h>

#include "config.h"
#include "FDC.h"
#include "ptr.h"
#include "Attr.h"
#include "Unicode.h"
#include "Colors.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////
// Font cache
struct Font {
  HFONT	  m_font;
  int	  m_size;
  int	  m_refcount;
  int	  m_height;
  int	  m_ascent;
  int	  m_angle;
  int	  m_hwidth;
  unsigned m_flags;
  bool	  m_ok;

  Font() : m_font(NULL), m_height(-1), m_ascent(-1), m_ok(false),
    m_angle(0), m_hwidth(-1) { }
  ~Font() { DeleteObject(m_font); }
  void	  Grab() { ++m_refcount; }
  void	  Release() { if (--m_refcount==0) delete this; }
};

struct FontCache {
  CPtrArray m_lru;
  int	    m_cachemax; // max number of entries in cache
  CString   m_face;
  int	    m_basesize;
  bool	    m_defbold;
  int	    m_cleartype;
  int	    m_angle;

  FontCache();
  ~FontCache() { RemoveAll(); }

  Font	    *AllocFont();
  void	    RemoveAll();
  void	    Release(Font *f);
  void	    SetDefault(const TCHAR *f,int bs,bool b,int ct,int an);
  Font	    *FindOrAlloc(int size,unsigned flags,int an);
};

#define	FNT(i)	  ((Font*)(m_lru[(i)]))

static FontCache g_fcache;

FontCache::FontCache() :
  m_cachemax(6), m_face(_T("Tahoma")),
  m_basesize(12), m_defbold(false), m_cleartype(0), m_angle(0)
{
}

void	FontCache::RemoveAll() {
  for (int i=0;i<m_lru.GetSize();++i) {
    FNT(i)->m_ok=false;
    FNT(i)->Release();
  }
  m_lru.RemoveAll();
}

Font	*FontCache::AllocFont() {
  if (m_lru.GetSize()<m_cachemax || FNT(0)->m_refcount>1) { // allocate new
    Font    *f=new Font;
    f->m_refcount=1;
    m_lru.Add(f);
    return f;
  }
  // reuse
  Font	  *f=FNT(0);
  m_lru.RemoveAt(0);
  m_lru.Add(f);
  DeleteObject(f->m_font);
  f->m_ok=false;
  f->m_ascent=f->m_height=-1;
  return f;
}

void  FontCache::SetDefault(const TCHAR *f,int bs,bool b,int ct,int an) {
  if (m_face!=f || ct!=m_cleartype)
    RemoveAll();
  m_face=f;
  m_basesize=bs;
  m_defbold=b;
  m_cleartype=ct;
  m_angle=an;
}

Font  *FontCache::FindOrAlloc(int size,unsigned flags,int an) {
  int	i;
  Font	*f;
  for (i=0;i<m_lru.GetSize();++i) {
    f=FNT(i);
    if (f->m_size==size && f->m_flags==flags && f->m_angle==an) {
      if (i!=m_lru.GetUpperBound()) {
	m_lru.RemoveAt(i);
	m_lru.Add(f);
      }
      return f;
    }
  }
  f=AllocFont();
  LOGFONT   lf;
  memset(&lf,0,sizeof(lf));
  lf.lfHeight=-size;
  lf.lfEscapement=lf.lfOrientation=an;
  lf.lfWeight=flags&Attr::BOLD ? FW_BOLD : FW_NORMAL;
  lf.lfItalic=flags&Attr::ITALIC ? 1 : 0;
  lf.lfUnderline=flags&Attr::UNDERLINE ? 1 : 0;
  lf.lfCharSet=DEFAULT_CHARSET;
  lf.lfOutPrecision=OUT_DEFAULT_PRECIS;
  lf.lfClipPrecision=CLIP_DEFAULT_PRECIS;
  if (m_cleartype==1)
    lf.lfQuality=CLEARTYPE_QUALITY;
  else if (m_cleartype==2)
    lf.lfQuality=ANTIALIASED_QUALITY;
  else
    lf.lfQuality=NONANTIALIASED_QUALITY;
  lf.lfPitchAndFamily=DEFAULT_PITCH|FF_DONTCARE;
  if (flags&CFDC::FORCETAHOMA)
    _tcsncpy_s(lf.lfFaceName,LF_FACESIZE,_T("Tahoma"),LF_FACESIZE-1);
  else if (flags&Attr::XFONT) {
    _tcsncpy_s(lf.lfFaceName,LF_FACESIZE,_T("NewtonUni"),LF_FACESIZE-1);
    lf.lfHeight-=2; // adjust relative size to tahoma
  } else
    _tcsncpy_s(lf.lfFaceName,LF_FACESIZE,m_face,LF_FACESIZE-1);
  f->m_font=CreateFontIndirect(&lf);
  f->m_size=size;
  f->m_flags=flags;
  f->m_angle=an;
  f->m_ok=true;
  return f;
}

void  FontCache::Release(Font *f) {
  int	i;
  for (i=0;i<m_lru.GetSize();++i)
    if (FNT(i)==f) {
      if (f->m_refcount>1) {
	f->Release();
	if (i>0 && FNT(i)->m_refcount>1) {
	  m_lru.RemoveAt(i);
	  m_lru.InsertAt(0,f);
	}
	while (m_lru.GetSize()>m_cachemax && FNT(0)->m_refcount==1) {
	  f=FNT(0);
	  m_lru.RemoveAt(0);
	  f->Release();
	}
      }
      return;
    }
  f->Release();
}

void  CFDC::SetDefaultFont(const TCHAR *face,int basesize,bool bold,
			   int cleartype,int angle) {
  g_fcache.SetDefault(face,basesize,bold,cleartype,angle);
}

void  CFDC::SelectFontAbs(int size,unsigned flags,bool zesc) {
  Font	*f=(Font*)m_hFont;
  if (g_fcache.m_defbold && !(flags&FORCENORMALWEIGHT))
    flags|=Attr::BOLD;
  int ang=zesc ? 0 : g_fcache.m_angle;
  if (f && f->m_ok && f->m_size==size && f->m_flags==flags && f->m_angle==ang)
    return;
  if (f)
    g_fcache.Release(f);
  f=g_fcache.FindOrAlloc(size,flags,ang);
  f->Grab();
  m_hFont=(void*)f;
  HGDIOBJ obj=SelectObject(m_hDC,f->m_font);
  if (!m_hOrigFont)
    m_hOrigFont=obj;
}

void  CFDC::SelectFont(int size,unsigned flags,bool zesc) {
  SelectFontAbs(size+g_fcache.m_basesize,flags,zesc);
}

CFDC::CFDC(HWND hWnd,PAINTSTRUCT *ps) :
  m_hFont(NULL), m_hOrigFont(NULL), m_hOrigPen(NULL), m_hWnd(hWnd), m_ps(ps),
  m_lpx(0)
{
  if (m_ps)
    m_hDC=BeginPaint(m_hWnd,m_ps);
  else
    m_hDC=GetDC(m_hWnd);
  SetBkMode(m_hDC,TRANSPARENT);
  ::SetTextColor(m_hDC,m_fg=C_NORM);
  ::SetBkColor(m_hDC,m_bg=C_BG);
  m_pfg=RGB(0,0,0); // default pen is black
}

CFDC::~CFDC() {
  if (m_hOrigFont)
    SelectObject(m_hDC,m_hOrigFont);
  if (m_hOrigPen)
    DeleteObject(SelectObject(m_hDC,m_hOrigPen));
  if (m_hFont)
    g_fcache.Release((Font*)m_hFont);
  if (m_ps)
    EndPaint(m_hWnd,m_ps);
  else
    ReleaseDC(m_hWnd,m_hDC);
}

void  CFDC::GetFontSize(int& height,int& ascent) {
  Font	*f=(Font*)m_hFont;
  if (!f || f->m_height<0) {
    TEXTMETRIC	tm;
    GetTextMetrics(m_hDC,&tm);
    height=tm.tmAscent+tm.tmDescent+tm.tmExternalLeading;
    ascent=tm.tmAscent;
    if (f) {
      f->m_height=height;
      f->m_ascent=ascent;
    }
  } else {
    height=f->m_height;
    ascent=f->m_ascent;
  }
}

int  CFDC::GetHypWidth() {
  Font	*f=(Font*)m_hFont;
  if (!f || f->m_hwidth<0) {
    SIZE    sz;
    ::GetTextExtentPoint32(m_hDC,_T("-"),1,&sz);
    if (f)
      f->m_hwidth=sz.cx;
    return sz.cx;
  }
  return f->m_hwidth;
}

void  CFDC::SetCacheSize(int cs) {
  if (cs>0 && cs<100)
    g_fcache.m_cachemax=cs;
}

int  CFDC::GetLPX() {
  if (!m_lpx)
    m_lpx=GetDeviceCaps(m_hDC,LOGPIXELSX);
  return m_lpx;
}

void  CFDC::GetTextExtent(const wchar_t *str,int len,
			  int width,int& nch,int *dx,SIZE& sz)
{
  GetTextExtentExPoint(m_hDC,str,len,width,&nch,dx,&sz);
}

void  CFDC::SetColor(COLORREF c) {
  if (c!=m_pfg) {
    HGDIOBJ obj=::SelectObject(m_hDC,::CreatePen(PS_SOLID,0,c));
    if (m_hOrigPen)
      DeleteObject(obj);
    else
      m_hOrigPen=obj;
    m_pfg=c;
  }
}
