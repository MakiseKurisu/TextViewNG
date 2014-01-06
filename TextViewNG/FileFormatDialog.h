/*
* Copyright (c) 2001,2002,2003 Mike Matsnev. All Rights Reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
* notice immediately at the beginning of the file, without modification,
* this list of conditions, and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* 3. Absolutely no warranty of function or purpose is made by the author
* Mike Matsnev.
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
* $Id: FileFormatDialog.h,v 1.6.2.2 2003/05/28 17:11:53 mike Exp $
*
*/

#if !defined(AFX_FILEFORMATDIALOG_H__8BB8FB0D_893A_43ED_ADFB_5A6BBD790AFC__INCLUDED_)
#define AFX_FILEFORMATDIALOG_H__8BB8FB0D_893A_43ED_ADFB_5A6BBD790AFC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// FileFormatDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFileFormatDialog dialog

class CFileFormatDialog : public CDialog
{
    // Construction
public:
    CFileFormatDialog(CWnd* pParent = NULL); // standard constructor

    // Dialog Data
    //{{AFX_DATA(CFileFormatDialog)
    enum
    {
        IDD = IDD_FILEFORMAT
    };
    int m_encoding;
    int m_format;
    int m_defencoding;
    //}}AFX_DATA


    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CFileFormatDialog)
protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
    //}}AFX_VIRTUAL

    // Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CFileFormatDialog)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILEFORMATDIALOG_H__8BB8FB0D_893A_43ED_ADFB_5A6BBD790AFC__INCLUDED_)
