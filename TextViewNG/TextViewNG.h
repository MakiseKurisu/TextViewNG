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
 * $Id: TextViewNG.h,v 1.16.2.2 2003/06/30 00:24:09 mike Exp $
 * 
 */

#if !defined(AFX_TEXTVIEWNG_H__98C46B55_4333_4A32_A3D4_2CA826387359__INCLUDED_)
#define AFX_TEXTVIEWNG_H__98C46B55_4333_4A32_A3D4_2CA826387359__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// common image list
#define	IM_BLANK  0
#define	IM_BMK	  1
#define	IM_TEXT	  2
#define	IM_DIR	  3
#define	IM_ZIP	  4
#define	IM_FILE	  5
#define IM_CNODE  6
#define	IM_CLEAF  7
#define IM_IMAGE  8

/////////////////////////////////////////////////////////////////////////////
// CTVApp:
// See TextViewNG.cpp for the implementation of this class
//

class CImageList;

class CTVApp : public CWinApp
{
public:
  static CImageList *ImageList();
  static void QueueCmd(UINT cmd);
  static UINT TopQueuedCmd();
  static CString  GetStr(const TCHAR *name,const TCHAR *def=NULL) {
    return AfxGetApp()->GetProfileString(_T("Parameters"),name,def);
  }
  static int	  GetInt(const TCHAR *name,int def=0) {
    return AfxGetApp()->GetProfileInt(_T("Parameters"),name,def);
  }
  static void	  SetStr(const TCHAR *name,const TCHAR *val) {
    AfxGetApp()->WriteProfileString(_T("Parameters"),name,val);
  }
  static void	  SetInt(const TCHAR *name,int val) {
    AfxGetApp()->WriteProfileInt(_T("Parameters"),name,val);
  }
  static void	  Barf(const TCHAR *format,...);
  CTVApp();

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CTVApp)
public:
  virtual BOOL InitInstance();
  virtual BOOL OnIdle(LONG lCount);
  //}}AFX_VIRTUAL

  // Implementation

  //{{AFX_MSG(CTVApp)
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
protected:
  CUIntArray	      m_cmdqueue;
  auto_ptr<CImageList>	m_imagelist;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTVIEWNG_H__98C46B55_4333_4A32_A3D4_2CA826387359__INCLUDED_)
