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
* $Id: InputBox.cpp,v 1.15.2.5 2007/03/26 19:26:52 mike Exp $
*
*/

#define _WIN32_WINNT _WIN32_WINNT_MAXVER

#include <afxwin.h>
#include <afxext.h>

#include "resource.h"
#include "InputBox.h"
#include "config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// InputBox dialog


InputBox::InputBox(const CString& prompt, const CString& title,
    CWnd* pParent /*=NULL*/)
    : CDialog(InputBox::IDD, pParent),
    m_prompt(prompt), m_title(title)
{
    //{{AFX_DATA_INIT(InputBox)
    m_str = _T("");
    //}}AFX_DATA_INIT
}


void InputBox::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(InputBox)
    DDX_Text(pDX, IDC_WORD, m_str);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(InputBox, CDialog)
    //{{AFX_MSG_MAP(InputBox)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// InputBox message handlers

bool GetUserInput(const CString& prompt, const CString& title,
    CString& str, CWnd *parent)
{
    InputBox box(prompt, title, parent);
    box.m_str = str;;
    if (box.DoModal() == IDOK)
    {
        str = box.m_str;
        return true;
    }
    return false;
}

BOOL InputBox::OnInitDialog()
{
    CDialog::OnInitDialog();
    SetDlgItemText(IDC_LABEL, m_prompt);
    SetWindowText(m_title);
    return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
// CAddBmDialog dialog


CAddBmDialog::CAddBmDialog(CWnd* pParent /*=NULL*/)
: CDialog(CAddBmDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CAddBmDialog)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}


void CAddBmDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAddBmDialog)
    DDX_Text(pDX, IDC_BMKEDIT, m_text);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddBmDialog, CDialog)
    //{{AFX_MSG_MAP(CAddBmDialog)
    ON_WM_SIZE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddBmDialog message handlers

void CAddBmDialog::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);
    RECT rc;
    GetClientRect(&rc);
    HWND hWnd;
    GetDlgItem(IDC_BMKEDIT, &hWnd);
    if (hWnd)
        ::SetWindowPos(hWnd, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE);
}


/////////////////////////////////////////////////////////////////////////////
// CFindDlg dialog


CFindDlg::CFindDlg(CWnd* pParent /*=NULL*/)
: CDialog(CFindDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CFindDlg)
    // NOTE: the ClassWizard will add member initialization here
    m_matchcase = 0;
    //}}AFX_DATA_INIT
}


void CFindDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CFindDlg)
    DDX_Text(pDX, IDC_FINDTEXT, m_text);
    DDX_Check(pDX, IDC_MATCHCASE, m_matchcase);
    DDX_Check(pDX, IDC_FROMCURPOS, m_fromcurpos);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFindDlg, CDialog)
    //{{AFX_MSG_MAP(CFindDlg)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFindDlg message handlers

BOOL CAddBmDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    return TRUE;
}

BOOL CFindDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    return TRUE;
}
