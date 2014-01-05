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
* $Id: MiscOptDlg.cpp,v 1.10.2.5 2004/10/21 15:35:24 mike Exp $
*
*/

#define _WIN32_WINNT	_WIN32_WINNT_MAXVER

#include <afxext.h>

#include "resource.h"
#include "MiscOptDlg.h"
#include "config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMiscOptDlg dialog


CMiscOptDlg::CMiscOptDlg(CWnd* pParent /*=NULL*/)
: CDialog(CMiscOptDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CMiscOptDlg)
    m_fcsize = 0;
    m_fbsize = 0;
    m_allowmulti = 0;
    m_rotb = 0;
    m_lastfiles = 0;
    //}}AFX_DATA_INIT
}


void CMiscOptDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CMiscOptDlg)
    DDX_Text(pDX, IDC_FCSIZE, m_fcsize);
    DDX_Text(pDX, IDC_FBSIZE, m_fbsize);
    DDX_Check(pDX, IDC_ALLOWMULTI, m_allowmulti);
    DDX_Check(pDX, IDC_ROTB, m_rotb);
    DDX_Text(pDX, IDC_LASTFILES, m_lastfiles);
    DDX_Check(pDX, IDC_REPLIMIT, m_autorepeatlimit);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMiscOptDlg, CDialog)
    //{{AFX_MSG_MAP(CMiscOptDlg)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMiscOptDlg message handlers

BOOL CMiscOptDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    SendDlgItemMessage(IDC_FCSIZESPIN, UDM_SETRANGE32, 2, 32);
    SendDlgItemMessage(IDC_FBSIZESPIN, UDM_SETRANGE32, 8, 512);
    SendDlgItemMessage(IDC_LASTFSPIN, UDM_SETRANGE32, 16, 1024);
    return TRUE;
}
