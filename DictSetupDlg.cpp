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
 * $Id: DictSetupDlg.cpp,v 1.6.2.4 2004/10/21 15:35:24 mike Exp $
 * 
 */

#include <afxcmn.h>
#include <afxtempl.h>
#include <afxext.h>

#include "resource.h"

#include "ptr.h"
#include "DictSetupDlg.h"
#include "TextViewNG.h"
#include "ZipFile.h"
#include "FileOpenDialog.h"
#include "Unicode.h"
#include "TextParser.h"
#include "Dictionary.h"
#include "config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const TCHAR  *g_res_dicts[]={
  _T(":Disable"),
#ifdef _WIN32_WCE
  _T(":SlovoEd"),
  _T(":Lingvo"),
#endif
};
#define	RESERVED_DICTS	((int)(sizeof(g_res_dicts)/sizeof(g_res_dicts[0])))

/////////////////////////////////////////////////////////////////////////////
// CDictSetupDlg dialog


CDictSetupDlg::CDictSetupDlg(CWnd* pParent /*=NULL*/)
: CDialog(CDictSetupDlg::IDD, pParent), m_cur(0)
{
  //{{AFX_DATA_INIT(CDictSetupDlg)
		// NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT
}


void CDictSetupDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CDictSetupDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDictSetupDlg, CDialog)
//{{AFX_MSG_MAP(CDictSetupDlg)
ON_COMMAND(IDC_ADD_DICT,OnAddDict)
ON_COMMAND(IDC_REMOVE_DICT,OnRemoveDict)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDictSetupDlg message handlers

BOOL CDictSetupDlg::OnInitDialog() 
{
  CDialog::OnInitDialog();

#if POCKETPC
  ((CCeCommandBar *)m_pWndEmptyCB)->LoadToolBar(cIDR_DIALOG);
#endif

  // fetch dictionaries list
  CStringArray list;
  GetDictList(list,m_cur);
  for (int d=RESERVED_DICTS;d<list.GetSize();d++) {
    int	  index=SendDlgItemMessage(IDC_DICTLIST,LB_ADDSTRING,0,
		  (LPARAM)(LPCTSTR)list[d]);
    if (index==LB_ERR)
      break;
    SendDlgItemMessage(IDC_DICTLIST,LB_SETITEMDATA,index,m_cur-RESERVED_DICTS==d);
  }

  return TRUE;
}

void  CDictSetupDlg::OnAddDict() {
  if (SendDlgItemMessage(IDC_DICTLIST,LB_GETCOUNT)>=MAXDICT)
    return;
  CString   path=GetFileName(NULL,this);
  if (path.GetLength()>0) {
    CString msg;
    IDict *d=IDict::Create(path,&msg);
    if (!d)
      MessageBox(_T("Can't open dictionary: ")+msg,_T("Error"),MB_ICONERROR|MB_OK);
    else {
      SendDlgItemMessage(IDC_DICTLIST,LB_ADDSTRING,0,(LPARAM)(LPCTSTR)path);
      delete d;
    }
  }
}

void  CDictSetupDlg::OnRemoveDict() {
  int	index=SendDlgItemMessage(IDC_DICTLIST,LB_GETCURSEL);
  if (index!=LB_ERR)
    SendDlgItemMessage(IDC_DICTLIST,LB_DELETESTRING,index);
}

void CDictSetupDlg::OnOK() {
  int	    sel=SendDlgItemMessage(IDC_DICTLIST,LB_GETCURSEL);
  int	    count=SendDlgItemMessage(IDC_DICTLIST,LB_GETCOUNT);
  CStringArray list;
  int	      cur=m_cur >= RESERVED_DICTS ? 0 : m_cur;
  for (int i=0;i<count;++i) {
    if (i==sel || (sel==LB_ERR && SendDlgItemMessage(IDC_DICTLIST,LB_GETITEMDATA,i)))
      cur=i+RESERVED_DICTS;
    int	  len=SendDlgItemMessage(IDC_DICTLIST,LB_GETTEXTLEN,i);
    CString tmp;
    TCHAR *cp=tmp.GetBuffer(len);
    SendDlgItemMessage(IDC_DICTLIST,LB_GETTEXT,i,(LPARAM)cp);
    tmp.ReleaseBuffer(len);
    list.Add(tmp);
  }
  PutDictList(list,cur);
  CDialog::OnOK();
}

void  CDictSetupDlg::GetDictList(CStringArray& list,int& cur) {
  list.RemoveAll();
  cur=0;
  for (int k=0;k<RESERVED_DICTS;++k)
    list.Add(g_res_dicts[k]);
  // fetch dictionaries list
  CString   dlist=CTVApp::GetStr(_T("Dictionary"));
  int	i=0;
  int	cc=0;
  while (i<dlist.GetLength() && dlist[i]>='0' && dlist[i]<='9') {
    cc=cc*10+dlist[i]-'0';
    ++i;
  }
  if (i<dlist.GetLength() && dlist[i]==_T('?'))
    ++i;
  for (;i<dlist.GetLength();) {
    int	  end=dlist.Find(_T('?'),i);
    if (end<0)
      end=dlist.GetLength();
    if (dlist[i]==_T('*')) {
      ++i;
      if (i>=end)
	continue;
    }
    list.Add(dlist.Mid(i,end-i));
    i=end+1;
  }
  if (cc>=0 && cc<list.GetSize())
    cur=cc;
}

void  CDictSetupDlg::PutDictList(const CStringArray& list,int cur) {
  CString   dlist;
  dlist.Format(_T("%d"),cur);

  for (int i=0;i<list.GetSize();++i) {
    dlist+=_T("?");
    dlist+=list[i];
  }
  CTVApp::SetStr(_T("Dictionary"),dlist);
}

void CDictSetupDlg::SetActiveDict(int cur) {
  int	cd;
  CStringArray	list;
  GetDictList(list,cd);
  list.RemoveAt(0,RESERVED_DICTS);
  PutDictList(list,cur);
}
