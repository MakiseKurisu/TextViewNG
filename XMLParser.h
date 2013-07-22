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
 * $Id: XMLParser.h,v 1.39.2.2 2004/01/08 11:06:06 mike Exp $
 * 
 */

#if !defined(AFX_XMLPARSER_H__582B90AF_6795_4F8F_849D_100EFF8AC338__INCLUDED_)
#define AFX_XMLPARSER_H__582B90AF_6795_4F8F_849D_100EFF8AC338__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TextParser.h"

class XMLParser : public TextParser  
{
protected:
  friend class TextParser;

  XMLParser(Meter *m,CBufFile *fp,HANDLE heap,Bookmarks *bmk) : TextParser(m,fp,heap,bmk) { }

  static XMLParser	*MakeParser(Meter *m,CBufFile *fp,Bookmarks *bmk,HANDLE heap);
  virtual bool		ParseFile(int encoding) = 0;
public:

  // formatting details
  struct ElemFmt {
    char	fsz; //signed
    BYTE	bold;
    BYTE	italic;
    BYTE	color;
    BYTE	align;
    BYTE	underline;
    DWORD	flags;
    int		lindent;
    int		rindent;
    int		findent;
    CString	name;
    enum { NOCHG=127,
      SECTION=0x01,  /* a new section */
      PARA=0x02,    /* text container */
      DOCUMENT=0x04,    /* subdocument */
      FMT=0x08,	    /* apply this style */
      ELINE=0x10,   /* add an empty line before this element */
      SPACE=0x20,   /* insert spaces between such elements */
      TITLE=0x40,   /* collect text contents and add a chapter */
      ENABLE=0x80,  /* enable P-type tags */
      LINKDEST=0x100,/* can have an id attribute */
      LINK=0x200,   /* link */
      AELINE=0x400, /* an empty line *after* the element */
      STYLE=0x800,  /* apply style from @name */
      DESCCAT=0x1000, /* description category */
      DESCITEM=0x2000, /* description item */
      STYLESHEET=0x4000, /* this is a stylesheet */
      BINARY=0x8000,  /* binary */
      IMAGE=0x10000, /* image element */
      HEADER=0x20000, /* header */
    };
    static const TCHAR	*flag_names;
    void  Clear();
  };

  typedef CArray<ElemFmt,ElemFmt&>  FmtArray; 
  static FmtArray&  GetXMLStyles();
  static void	    SaveStyles();
  static void	    LoadStyles();
};

#endif // !defined(AFX_XMLPARSER_H__582B90AF_6795_4F8F_849D_100EFF8AC338__INCLUDED_)
