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
 * $Id: StylesDlg.cpp,v 1.15.2.2 2004/10/21 15:35:24 mike Exp $
 * 
 */

#include <afxext.h>
#include <afxtempl.h>

#include "config.h"
#include "resource.h"

#include "XMLParser.h"
#include "StylesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	MINFSIZE    -5
#define	MAXFSIZE    +10

/////////////////////////////////////////////////////////////////////////////
// CStylesDlg dialog


CStylesDlg::CStylesDlg(CWnd* pParent /*=NULL*/)
: CDialog(CStylesDlg::IDD, pParent), m_cur(-1)
{
  m_fmt.Append(XMLParser::GetXMLStyles());
  //{{AFX_DATA_INIT(CStylesDlg)
		// NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT
}


void CStylesDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  if (pDX->m_bSaveAndValidate)
    GetValues();
  //{{AFX_DATA_MAP(CStylesDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStylesDlg, CDialog)
//{{AFX_MSG_MAP(CStylesDlg)
ON_CBN_SELCHANGE(IDC_ELEMENTS, OnSelchangeElements)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStylesDlg message handlers

static const TCHAR  *g_onoff[]={
  _T("Off"),
  _T("On"),
  _T("Inherit"),
  0
};

static const TCHAR  *g_colors[]={
  _T("Normal"),
  _T("HL 1"),
  _T("HL 2"),
  _T("HL 3"),
  _T("HL 4"),
  _T("HL 5"),
  _T("HL 6"),
  _T("HL 7"),
  _T("Inherit")
};

static struct {
  BYTE	  fmt;
  const TCHAR *name;
}		    g_align[]={
  { 0,	_T("Default") },
  { Paragraph::right, _T("Right") },
  { Paragraph::center, _T("Center") },
  { Paragraph::justify, _T("Justified") },
  { 0,  _T("Inherit") },
  { 0, 0 }
};

BOOL CStylesDlg::OnInitDialog() 
{
  CDialog::OnInitDialog();

#if POCKETPC
  ((CCeCommandBar *)m_pWndEmptyCB)->LoadToolBar(cIDR_DIALOG);
#endif

  // fill in comboboxes
  int	i;
  for (i=MINFSIZE;i<=MAXFSIZE;++i) {
    CString tmp;
    tmp.Format(_T("%+d"),i);
    SendDlgItemMessage(IDC_FONT,CB_ADDSTRING,0,(LPARAM)(const TCHAR *)tmp);
  }
  SendDlgItemMessage(IDC_FONT,CB_ADDSTRING,0,(LPARAM)_T("Inherit"));
  for (i=0;g_onoff[i];++i) {
    SendDlgItemMessage(IDC_FBOLD,CB_ADDSTRING,0,(LPARAM)g_onoff[i]);
    SendDlgItemMessage(IDC_FITALIC,CB_ADDSTRING,0,(LPARAM)g_onoff[i]);
    SendDlgItemMessage(IDC_FUNDERLINE,CB_ADDSTRING,0,(LPARAM)g_onoff[i]);
  }
  for (i=0;g_colors[i];++i)
    SendDlgItemMessage(IDC_COLOR,CB_ADDSTRING,0,(LPARAM)g_colors[i]);
  for (i=0;g_align[i].name;++i)
    SendDlgItemMessage(IDC_ALIGN,CB_ADDSTRING,0,(LPARAM)g_align[i].name);
  for (i=0;i<m_fmt.GetSize();++i)
    if (m_fmt[i].name.GetLength()>0 && m_fmt[i].flags&XMLParser::ElemFmt::FMT) {
      LRESULT idx=SendDlgItemMessage(IDC_ELEMENTS,CB_ADDSTRING,0,(LPARAM)(const TCHAR *)m_fmt[i].name);
      if (idx!=CB_ERR)
	SendDlgItemMessage(IDC_ELEMENTS,CB_SETITEMDATA,idx,i);
    }
  SendDlgItemMessage(IDC_ELEMENTS,CB_SETCURSEL);
  SendDlgItemMessage(IDC_LISPIN,UDM_SETRANGE32,-100,100);
  SendDlgItemMessage(IDC_RISPIN,UDM_SETRANGE32,-100,100);
  SendDlgItemMessage(IDC_FIRSTSPIN,UDM_SETRANGE32,-100,100);
  SetValues(0);
  return TRUE;
}

void CStylesDlg::OnSelchangeElements() 
{
  GetValues();
  SetValues(SendDlgItemMessage(IDC_ELEMENTS,CB_GETCURSEL));
}

void CStylesDlg::GetValues() {
  if (m_cur>=0) {
    int	  n;
    n=SendDlgItemMessage(IDC_FONT,CB_GETCURSEL);
    if (n==SendDlgItemMessage(IDC_FONT,CB_GETCOUNT)-1)
      n=XMLParser::ElemFmt::NOCHG;
    else
      n+=MINFSIZE;
    m_fmt[m_cur].fsz=n;
    n=SendDlgItemMessage(IDC_FBOLD,CB_GETCURSEL);
    if (n==SendDlgItemMessage(IDC_FBOLD,CB_GETCOUNT)-1)
      n=XMLParser::ElemFmt::NOCHG;
    m_fmt[m_cur].bold=n;
    n=SendDlgItemMessage(IDC_FITALIC,CB_GETCURSEL);
    if (n==SendDlgItemMessage(IDC_FITALIC,CB_GETCOUNT)-1)
      n=XMLParser::ElemFmt::NOCHG;
    m_fmt[m_cur].italic=n;
    n=SendDlgItemMessage(IDC_FUNDERLINE,CB_GETCURSEL);
    if (n==SendDlgItemMessage(IDC_FUNDERLINE,CB_GETCOUNT)-1)
      n=XMLParser::ElemFmt::NOCHG;
    m_fmt[m_cur].underline=n;
    n=SendDlgItemMessage(IDC_COLOR,CB_GETCURSEL);
    if (n==SendDlgItemMessage(IDC_COLOR,CB_GETCOUNT)-1)
      n=XMLParser::ElemFmt::NOCHG;
    m_fmt[m_cur].color=n;
    n=SendDlgItemMessage(IDC_ALIGN,CB_GETCURSEL);
    if (n==SendDlgItemMessage(IDC_ALIGN,CB_GETCOUNT)-1)
      n=XMLParser::ElemFmt::NOCHG;
    else
      n=g_align[n].fmt;
    m_fmt[m_cur].align=n;
    m_fmt[m_cur].lindent=GetDlgItemInt(IDC_LI);
    if (m_fmt[m_cur].lindent==0)
      m_fmt[m_cur].lindent=XMLParser::ElemFmt::NOCHG;
    m_fmt[m_cur].rindent=GetDlgItemInt(IDC_RI);
    if (m_fmt[m_cur].rindent==0)
      m_fmt[m_cur].rindent=XMLParser::ElemFmt::NOCHG;
    m_fmt[m_cur].findent=GetDlgItemInt(IDC_FIRST);
    if (m_fmt[m_cur].findent==0)
      m_fmt[m_cur].findent=XMLParser::ElemFmt::NOCHG;
  }
}

void CStylesDlg::SetValues(int idx) {
  LRESULT   ri=SendDlgItemMessage(IDC_ELEMENTS,CB_GETITEMDATA,idx);
  if (ri==CB_ERR)
    return;
  int	n;
  n=m_fmt[ri].fsz;
  if (n==XMLParser::ElemFmt::NOCHG)
    n=SendDlgItemMessage(IDC_FONT,CB_GETCOUNT)-1;
  else {
    if (n<MINFSIZE)
      n=MINFSIZE;
    if (n>MAXFSIZE)
      n=MAXFSIZE;
    n-=MINFSIZE;
  }
  SendDlgItemMessage(IDC_FONT,CB_SETCURSEL,n);
  n=m_fmt[ri].bold;
  if (n==XMLParser::ElemFmt::NOCHG)
    n=SendDlgItemMessage(IDC_FBOLD,CB_GETCOUNT)-1;
  SendDlgItemMessage(IDC_FBOLD,CB_SETCURSEL,n);
  n=m_fmt[ri].italic;
  if (n==XMLParser::ElemFmt::NOCHG)
    n=SendDlgItemMessage(IDC_FITALIC,CB_GETCOUNT)-1;
  SendDlgItemMessage(IDC_FITALIC,CB_SETCURSEL,n);
  n=m_fmt[ri].underline;
  if (n==XMLParser::ElemFmt::NOCHG)
    n=SendDlgItemMessage(IDC_FUNDERLINE,CB_GETCOUNT)-1;
  SendDlgItemMessage(IDC_FUNDERLINE,CB_SETCURSEL,n);
  n=m_fmt[ri].color;
  if (n==XMLParser::ElemFmt::NOCHG)
    n=SendDlgItemMessage(IDC_COLOR,CB_GETCOUNT)-1;
  SendDlgItemMessage(IDC_COLOR,CB_SETCURSEL,n);
  n=m_fmt[ri].align;
  if (n==XMLParser::ElemFmt::NOCHG)
    n=SendDlgItemMessage(IDC_ALIGN,CB_GETCOUNT)-1;
  else {
    for (int i=0;g_align[i].name;++i)
      if (n==g_align[i].fmt) {
	n=i;
	goto ok;
      }
    n=SendDlgItemMessage(IDC_ALIGN,CB_GETCOUNT)-1;
ok:;
  }
  SendDlgItemMessage(IDC_ALIGN,CB_SETCURSEL,n);
  SetDlgItemInt(IDC_LI,m_fmt[ri].lindent==XMLParser::ElemFmt::NOCHG ? 0 :
    m_fmt[ri].lindent);
  SetDlgItemInt(IDC_RI,m_fmt[ri].rindent==XMLParser::ElemFmt::NOCHG ? 0 :
    m_fmt[ri].rindent);
  SetDlgItemInt(IDC_FIRST,m_fmt[ri].findent==XMLParser::ElemFmt::NOCHG ? 0 :
    m_fmt[ri].findent);
  m_cur=ri;
}

bool CStylesDlg::SaveChanges() {
  XMLParser::FmtArray&	fa=XMLParser::GetXMLStyles();
  bool flag=false;
  if (fa.GetSize()==m_fmt.GetSize()) {
    for (int i=0;i<fa.GetSize();++i)
      if (fa[i].align!=m_fmt[i].align ||
	  fa[i].bold!=m_fmt[i].bold ||
	  fa[i].color!=m_fmt[i].color ||
	  fa[i].flags!=m_fmt[i].flags ||
	  fa[i].fsz!=m_fmt[i].fsz ||
	  fa[i].italic!=m_fmt[i].italic ||
	  fa[i].underline!=m_fmt[i].underline ||
	  fa[i].lindent!=m_fmt[i].lindent ||
	  fa[i].rindent!=m_fmt[i].rindent ||
	  fa[i].findent!=m_fmt[i].findent) {
	flag=true;
	fa[i].align=m_fmt[i].align;
	fa[i].bold=m_fmt[i].bold;
	fa[i].color=m_fmt[i].color;
	fa[i].flags=m_fmt[i].flags;
	fa[i].fsz=m_fmt[i].fsz;
	fa[i].italic=m_fmt[i].italic;
	fa[i].underline=m_fmt[i].underline;
	fa[i].lindent=m_fmt[i].lindent;
	fa[i].rindent=m_fmt[i].rindent;
	fa[i].findent=m_fmt[i].findent;
      }
  }
  return flag;
}
