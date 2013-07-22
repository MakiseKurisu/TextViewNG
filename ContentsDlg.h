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
 * $Id: ContentsDlg.h,v 1.19.2.7 2003/07/17 19:58:53 mike Exp $
 * 
 */

#if !defined(AFX_CONTENTSDLG_H__9A30007D_827B_4734_B3EB_1A073E9DE84D__INCLUDED_)
#define AFX_CONTENTSDLG_H__9A30007D_827B_4734_B3EB_1A073E9DE84D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CContentsDlg dialog

class CContentsDlg : public CDialog
{
  // Construction
public:
  CContentsDlg(Bookmarks& toc,TextFile *tp,FilePos cur,CWnd* pParent = NULL);

  enum {
    NO_ITEM=-1000000, // less than that number of subdocuments
  };
  int	m_index;

  // Dialog Data
  //{{AFX_DATA(CContentsDlg)
  enum { IDD = IDD_CONTENTS };
  // NOTE: the ClassWizard will add data members here
  //}}AFX_DATA
  
  
  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CContentsDlg)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL
  
  // Implementation
protected:
  void OnOK();
  void OnCancel();

  // Generated message map functions
  //{{AFX_MSG(CContentsDlg)
  virtual BOOL OnInitDialog();
  afx_msg LRESULT OnXContextMenu(WPARAM wParam,LPARAM lParam);
  afx_msg LRESULT OnXDblClick(WPARAM wParam,LPARAM lParam);
  afx_msg void OnEdit();
  afx_msg void OnDelete();
  afx_msg void OnL0();
  afx_msg void OnL1();
  afx_msg void OnL2();
  afx_msg void OnL3();
  afx_msg void OnL4();
  afx_msg void OnL5();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

  FilePos     m_cur;
  Bookmarks&  m_toc;
  TextFile    *m_tp;

  static CString  GetText(void *ugtdata,int num,int item,LONG data);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONTENTSDLG_H__9A30007D_827B_4734_B3EB_1A073E9DE84D__INCLUDED_)
