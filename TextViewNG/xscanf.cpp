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
 * $Id: xscanf.cpp,v 1.4.2.1 2003/04/12 22:52:34 mike Exp $
 * 
 */

#pragma warning(disable:4100)

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "xscanf.h"

#define	MAXARGS	  8

static int    Integer(const wchar_t **str,const wchar_t *end,int *val,
		      int *argv,int argc)
{
  const wchar_t	  *p=*str;
  int		  iv=0;
  int		  sign=0;
  bool		  range=true;

  if (argc>1 && argv[0]>=argv[1])
    range=false;
  if (*p==L'-') {
    ++p;
    sign=1;
  } else if (*p==L'+')
    ++p;
  *str=p;
  while (p<end && iswdigit(*p))
    iv=iv*10+*p++-L'0';
  if (p==*str) {
    if (argc>2 && p<end && *p==L'*') {
      *val=argv[2];
      *str=p+1;
      return 0;
    }
    return -1;
  }
  if (sign)
    iv=-iv;
  if (range && argc>0 && iv<argv[0])
    iv=argv[0];
  if (range && argc>1 && iv>=argv[1])
    iv=argv[1];
  *val=iv;
  *str=p;
  return 0;
}

static int    String(const wchar_t **str,const wchar_t *end,void *dstr,
		     bool alloc,int delim,int *argv,int argc)
{
  const wchar_t	  *p=*str;

  // find end of string
  if (delim==L' ') {
    while (p<end && !iswspace(*p))
      ++p;
  } else {
    while (p<end && *p!=delim)
      ++p;
  }
  int	    length=p-*str;
  wchar_t   *dest=(wchar_t*)dstr;
  // allocate storage if needed
  if (alloc) {
    dest=new wchar_t[length+1];
    *(wchar_t**)dstr=dest;
  }
  // copy chars
  memcpy(dest,*str,length*sizeof(wchar_t));
  dest[length]=L'\0';
  *str+=length;
  return 0;
}

int	Scan::vxscanf(const wchar_t *str,int len,const wchar_t *format,
		int *chars,int *fields,va_list args)
{
  const wchar_t	  *startstr=str;
  const wchar_t	  *endstr=str+len;

  int		  fieldnum=0;
  int		  error=0;

  int		  fmt_args[MAXARGS];
  int		  num_fmt_args,cur_fmt_arg,seen_arg_char,arg_sign;
  wchar_t	  fmt_char;

  while (*format && str<endstr)
    switch (*format) {
    case L' ': // skip 0..inf whitespace chars
      while (str < endstr && iswspace(*str))
	++str;
      ++format;
      break;
    case L'%': // format spec
      // extract args from format(!), can be negative
      ++format;
      num_fmt_args=seen_arg_char=arg_sign=cur_fmt_arg=0;
      while (*format)
	switch (*format) {
	case L'+':
	  ++format;
	  break;
	case L'-':
	  ++format;
	  arg_sign=1;
	  break;
	case L',': // end of arg
	  if (seen_arg_char && num_fmt_args<MAXARGS)
	      fmt_args[num_fmt_args++]=arg_sign ? -cur_fmt_arg : cur_fmt_arg;
	  seen_arg_char=arg_sign=cur_fmt_arg=0;
	  ++format;
	  break;
	case L'0': case L'1': case L'2': case L'3': case L'4':
	case L'5': case L'6': case L'7': case L'8': case L'9':
	  cur_fmt_arg=cur_fmt_arg*10+*format++-L'0';
	  seen_arg_char=1;
	  break;
	default: // end of format spec
	  if (seen_arg_char && num_fmt_args<MAXARGS)
	      fmt_args[num_fmt_args++]=arg_sign ? -cur_fmt_arg : cur_fmt_arg;
	  fmt_char=*format++;
	  goto done_fmt;
	}
      return -1; // invalid format string
done_fmt:
      // handle format specs now
      switch (fmt_char) {
      case L's': // whitespace delimited string
	if (String(&str,endstr,va_arg(args,wchar_t*),false,L' ',fmt_args,num_fmt_args)<0)
	  goto format_error;
	++fieldnum;
	break;
      case L'S': // whitespace delimited string, allocate storage
	if (String(&str,endstr,va_arg(args,wchar_t**),true,L' ',fmt_args,num_fmt_args)<0)
	  goto format_error;
	++fieldnum;
	break;
      case L'd': // (signed) integer
	if (Integer(&str,endstr,va_arg(args,int*),fmt_args,num_fmt_args)<0)
	  goto format_error;
	++fieldnum;
	break;
      case L'c': // a single character
	*va_arg(args,wchar_t*)=*str++;
	++fieldnum;
	break;
      case L't': // a delimited string
	if (String(&str,endstr,va_arg(args,wchar_t*),false,*format,fmt_args,num_fmt_args)<0)
	  goto format_error;
	++fieldnum;
	break;
      case L'T': // the same, but allocate storage
	if (String(&str,endstr,va_arg(args,wchar_t**),true,*format,fmt_args,num_fmt_args)<0)
	  goto format_error;
	++fieldnum;
	break;
      default: // invalid format char
	return -1;
      }
      break;
    default: // match the character as is
      if (*format!=*str) {
	error=NOMATCH;
	goto out;
      }
      ++str;
      ++format;
      break;
    }
out:
  if (chars)
    *chars=str-startstr;
  if (fields)
    *fields=fieldnum;
  return error;
format_error:
  error=FORMAT;
  goto out;
}

int	Scan::xscanf(const wchar_t *str,int len,const wchar_t *format,
	       int *chars,int *fields,...)
{
  va_list   ap;
  va_start(ap,fields);
  int	ret=vxscanf(str,len,format,chars,fields,ap);
  va_end(ap);
  return ret;
}

