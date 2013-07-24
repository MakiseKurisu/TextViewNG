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
 * $Id: Attr.h,v 1.2.2.3 2003/09/27 11:30:55 mike Exp $
 * 
 */

#ifndef ATTR_H
#define ATTR_H

// unified attributes
struct Attr { // XXX assumes little endian byteorder and M$ extensions
  // bit fields
  union {
    struct {
      signed char   fsize:6;  // font size
      unsigned char bold:1;   // bold flag
      unsigned char italic:1; // italic flag
      unsigned char xfont:1;  // alternate dictionary font
      unsigned char img:1;    // this is an inline image

      unsigned char underline:1; // underline flag
      unsigned char color:3;  // color index
      unsigned char hibg:1;   // highlight background

      unsigned char hyphen:1; // can break word here
      
    };

    WORD	  wa;
  };
  WORD	fontattr() { return wa & 0x5c0; }
  WORD	fontflags() { return wa & 0x3ff; }
  enum {
    BOLD = 0x40,
    ITALIC = 0x80,
    UNDERLINE = 0x400,
    XFONT = 0x100
  };
  bool	operator==(Attr other) { return wa==other.wa; }
  bool	operator!=(Attr other) { return wa!=other.wa; }
};

#endif
