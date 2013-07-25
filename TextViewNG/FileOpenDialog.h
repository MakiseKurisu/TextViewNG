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
* $Id: FileOpenDialog.h,v 1.11.2.3 2003/09/10 21:44:30 mike Exp $
*
*/

#if !defined(AFX_FILEOPENDIALOG_H__D39079BD_1C01_4E78_BEE5_5D89AB83F97C__INCLUDED_)
#define AFX_FILEOPENDIALOG_H__D39079BD_1C01_4E78_BEE5_5D89AB83F97C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "ZipFile.h"

// FileOpenDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFileOpenDialog dialog

class CFileOpenDialog : public CDialog
{
	// Construction
public:
	CFileOpenDialog(CWnd* pParent = NULL);   // standard constructor

	CString m_filename;
	CString m_path;
	CString m_vpath;
	// Dialog Data
	//{{AFX_DATA(CFileOpenDialog)
	enum { IDD = IDD_FILEOPEN };
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileOpenDialog)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:
	void OpenItem(const CString& path);
	void ActivateItem(HWND hWnd, int item);
	void FindFiles(bool showall = true);

	auto_ptr<ZipFile> m_zip; // zip file if any

	// Generated message map functions
	//{{AFX_MSG(CFileOpenDialog)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnXClick(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CString	  GetFileName(CString *startpath, CWnd *parent = NULL);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILEOPENDIALOG_H__D39079BD_1C01_4E78_BEE5_5D89AB83F97C__INCLUDED_)
