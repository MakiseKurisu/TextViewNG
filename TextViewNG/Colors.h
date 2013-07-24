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
 * $Id: Colors.h,v 1.9.2.3 2004/10/29 15:18:46 mike Exp $
 * 
 */

#ifndef COLORS_H
#define COLORS_H

struct ColorItem {
  const TCHAR	*name;
  COLORREF	value;
  COLORREF	tempval;
};

extern ColorItem	g_colors[];
extern int		g_color_profile;

// first four are A_ attributes
#define	CI_NORM		0

#define	CI_BG		8
#define	CI_HBG		9
#define	CI_GAUGE	10
#define	CI_TOCL0	11
#define	CI_TOCBM	12
#define	CI_UINP		13
#define	CI_UINPBG	14
#define	CI_AS		15

#define	C_NORM		(g_colors[CI_NORM].value)
#define	C_TCOLOR(c)	(g_colors[(c)].value)
#define	C_BG		(g_colors[CI_BG].value)
#define	C_HBG		(g_colors[CI_HBG].value)
#define	C_GAUGE		(g_colors[CI_GAUGE].value)
#define	C_TOCL0		(g_colors[CI_TOCL0].value)
#define	C_TOCBM		(g_colors[CI_TOCBM].value)
#define	C_UINP		(g_colors[CI_UINP].value)
#define	C_UINPBG	(g_colors[CI_UINPBG].value)
#define	C_AS		(g_colors[CI_AS].value)

void	SaveColors();
void	LoadColors();
bool	NextColorProfile();
void	AddColorProfileNames(void *menu,int startpos);

#endif
