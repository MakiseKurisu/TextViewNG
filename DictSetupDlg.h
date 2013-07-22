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
 * $Id: DictSetupDlg.h,v 1.3.2.2 2003/08/23 01:15:02 mike Exp $
 * 
 */

#if !defined(AFX_DICTSETUPDLG_H__0FD6C32E_A997_4C36_8D0C_11C693F9D95C__INCLUDED_)
#define AFX_DICTSETUPDLG_H__0FD6C32E_A997_4C36_8D0C_11C693F9D95C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CDictSetupDlg dialog

class CDictSetupDlg : public CDialog
{
  // Construction
public:
  CDictSetupDlg(CWnd* pParent = NULL);   // standard constructor
  
  enum { MAXDICT = 4096 }; // more than enough

  // Dialog Data
  //{{AFX_DATA(CDictSetupDlg)
  enum { IDD = IDD_DICTSETUP };
		// NOTE: the ClassWizard will add data members here
  //}}AFX_DATA
  
  
  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CDictSetupDlg)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL
 
  int	      m_cur;
  // Implementation
protected:
  
  virtual void OnOK();

  // Generated message map functions
  //{{AFX_MSG(CDictSetupDlg)
  virtual BOOL OnInitDialog();
  afx_msg void OnAddDict();
  afx_msg void OnRemoveDict();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

  static void	PutDictList(const CStringArray& list,int cur);

public:
  static void	GetDictList(CStringArray& list,int& cur);
  static void	SetActiveDict(int cur);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DICTSETUPDLG_H__0FD6C32E_A997_4C36_8D0C_11C693F9D95C__INCLUDED_)
