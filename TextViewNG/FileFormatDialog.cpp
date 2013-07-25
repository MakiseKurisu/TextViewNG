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
* $Id: FileFormatDialog.cpp,v 1.9.2.3 2004/10/21 15:35:24 mike Exp $
*
*/

#define _WIN32_WINNT	_WIN32_WINNT_MAXVER

#include <afxext.h>
#include <afxtempl.h>

#include "resource.h"
#include "config.h"

#include "ptr.h"
#include "FileFormatDialog.h"
#include "TextFile.h"
#include "Unicode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE [] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileFormatDialog dialog


CFileFormatDialog::CFileFormatDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CFileFormatDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFileFormatDialog)
	m_encoding = 0;
	m_format = 0;
	m_defencoding = 0;
	//}}AFX_DATA_INIT
}

void CFileFormatDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileFormatDialog)
	DDX_CBIndex(pDX, IDC_ENCODING, m_encoding);
	DDX_CBIndex(pDX, IDC_FILEFORMAT, m_format);
	DDX_CBIndex(pDX, IDC_DEF_ENCODING, m_defencoding);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFileFormatDialog, CDialog)
	//{{AFX_MSG_MAP(CFileFormatDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileFormatDialog message handlers

BOOL CFileFormatDialog::OnInitDialog()
{
#if POCKETPC
	((CCeCommandBar *) m_pWndEmptyCB)->LoadToolBar(cIDR_DIALOG);
#endif

	// initialize combo boxes
	CComboBox   *cb = (CComboBox*) GetDlgItem(IDC_FILEFORMAT);
	if (cb) {
		for (int i = -1; i < TextParser::GetNumFormats(); ++i)
			cb->AddString(TextFile::GetFormatName(i));
	}
	cb = (CComboBox*) GetDlgItem(IDC_ENCODING);
	CComboBox   *cb2 = (CComboBox*) GetDlgItem(IDC_DEF_ENCODING);
	if (cb) {
		cb->AddString(_T("Auto"));
		cb2->AddString(_T("Auto"));
		for (int i = 0; i < Unicode::GetNumCodePages(); ++i) {
			cb->AddString(Unicode::GetCodePageName(i));
			cb2->AddString(Unicode::GetCodePageName(i));
		}
	}
	CDialog::OnInitDialog();
	return TRUE;
}
