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
* $Id: ContentsDlg.cpp,v 1.31.2.11 2004/10/21 15:35:24 mike Exp $
*
*/

#pragma warning(disable:4100)
#define _WIN32_WINNT _WIN32_WINNT_MAXVER

#include <afxcmn.h>
#include <afxtempl.h>

#include "resource.h"
#include "ptr.h"
#include "FilePos.h"
#include "Bookmarks.h"
#include "ContentsDlg.h"
#include "TextViewNG.h"
#include "TextFile.h"
#include "config.h"
#include "XListBox.h"
#include "InputBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CContentsDlg dialog


CContentsDlg::CContentsDlg(Bookmarks& toc, TextFile *tp, FilePos cur,
    CWnd* pParent /*=NULL*/) :
    CDialog(CContentsDlg::IDD, pParent), m_index(NO_ITEM), m_toc(toc), m_cur(cur),
    m_tp(tp)
{
    //{{AFX_DATA_INIT(CContentsDlg)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

void CContentsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CContentsDlg)
    // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CContentsDlg, CDialog)
    //{{AFX_MSG_MAP(CContentsDlg)
    ON_MESSAGE(XLM_CONTEXTMENU, OnXContextMenu)
    ON_MESSAGE(XLM_DBLCLK, OnXDblClick)
    ON_COMMAND(ID_PP_EDIT, OnEdit)
    ON_COMMAND(ID_PP_DELETE, OnDelete)
    ON_COMMAND(ID_L0, OnL0)
    ON_COMMAND(ID_L1, OnL1)
    ON_COMMAND(ID_L2, OnL2)
    ON_COMMAND(ID_L3, OnL3)
    ON_COMMAND(ID_L4, OnL4)
    ON_COMMAND(ID_L5, OnL5)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CContentsDlg message handlers

BOOL CContentsDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    HWND      hXLB = ::GetDlgItem(m_hWnd, IDC_CONTENTS);
    RECT   r;
    GetClientRect(&r);

    RECT   rwin;
    ::GetWindowRect(hXLB, &rwin);
    r.bottom = r.top + (rwin.bottom - rwin.top);

    ::MoveWindow(hXLB, r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE);

    XLB_SetImageList(hXLB, CTVApp::ImageList()->GetSafeHandle(), true);
    XLB_SetGTFunc(hXLB, GetText, this);

    // insert items
    bool     havesubdocs = m_tp->GetSubDocCount() > 1;
    int     curlevel = havesubdocs ? -1 : 0; // -1 means Document-level
    int     curpos = m_toc.BFind(m_cur, Bookmarks::SPREVCH);
    bool     docmatch = curpos < m_toc.GetSize() && m_toc.Ref(curpos).docid == m_cur.docid;
    int     docid = -1;
    if (!docmatch)
        curpos = -1;

    XLB_Handle  *handle = XLB_GetHandle(hXLB);

    int   prev = 0;
    for (int i = 0; i < m_toc.GetSize(); ++i) {
        if (havesubdocs && docid < m_toc.Ref(i).docid) // create a new entry under root!
            do {
                ++docid;
                curlevel = -1;
                XLB_AppendItem(handle, NULL, NULL, IM_CNODE, 0, -docid - 1);
            } while (docid < m_toc.Ref(i).docid);
        int       lev = m_toc.Level(i);
        if (lev < 0) {
            if (prev >= 0)
                ++curlevel;
        }
        else
            curlevel = lev;
        prev = lev;
        XLB_AppendItem(handle, NULL, NULL,
            (m_toc.Flags(i)&Bookmarks::BMK) ? IM_BMK : IM_CLEAF,
            curlevel + 1, i);
        if (curpos == i)
            XLB_SetSelection(hXLB, XLB_GetItemCount(hXLB) - 1);
    }
    if (havesubdocs)
    while (docid < m_tp->GetSubDocCount() - 1) {
        ++docid;
        XLB_AppendItem(handle, NULL, NULL, IM_CNODE, 0, -docid - 1);
    }
    XLB_CollapseLevel(hXLB, 0); // does an implicit UpdateState
    XLB_EnsureVisible(hXLB, XLB_GetSelection(hXLB), true);
    return TRUE;
}

CString  CContentsDlg::GetText(void *ugtdata, int num, int item, LONG data)
{
    CContentsDlg *dlg = (CContentsDlg*)ugtdata;

    if (num == 0)
        return data >= 0 ?
        dlg->m_toc.Text(data, dlg->m_tp) :
        dlg->m_tp->GetSubDocName(-data - 1);
    else if (num == 1 && data >= 0) {
        CString   ret;
        ret.Format(_T("%d%%"),
            dlg->m_toc.Ref(data).para * 100 /
            (dlg->m_tp->Length(dlg->m_toc.Ref(data).docid) + 1)
            );
        return ret;
    }

    return CString();
}

void CContentsDlg::OnOK() {
    HWND     hXLB = ::GetDlgItem(m_hWnd, IDC_CONTENTS);

    m_index = NO_ITEM;

    int     sel = XLB_GetSelection(hXLB);
    if (sel >= 0)
        m_index = XLB_GetData(hXLB, sel);

    EndDialog(IDOK);
}

void CContentsDlg::OnCancel() {
    m_toc.Rollback();
    EndDialog(IDCANCEL);
}

LRESULT CContentsDlg::OnXContextMenu(WPARAM wParam, LPARAM lParam) {
    HWND     hXLB = ::GetDlgItem(m_hWnd, IDC_CONTENTS);

    int     sel = XLB_GetSelection(hXLB);
    if (sel < 0)
        return 0;

    int     idx = XLB_GetData(hXLB, sel);
    if (idx < 0 || !(m_toc.Flags(idx)&Bookmarks::BMK))
        return 0;

    CPoint    pt;
    pt.x = LOWORD(wParam);
    pt.y = HIWORD(wParam);
    ::ClientToScreen(hXLB, &pt);

    CMenu menu;
    menu.CreatePopupMenu();
    menu.AppendMenu(MF_STRING, ID_PP_EDIT, _T("Edit"));
    menu.AppendMenu(MF_STRING, ID_PP_DELETE, _T("Delete"));
    menu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this);

    return 0;
}

LRESULT CContentsDlg::OnXDblClick(WPARAM wParam, LPARAM lParam) {
    OnOK();
    return 0;
}

void  CContentsDlg::OnEdit() {
    HWND     hXLB = ::GetDlgItem(m_hWnd, IDC_CONTENTS);

    int     sel = XLB_GetSelection(hXLB);
    if (sel < 0)
        return;

    int     idx = XLB_GetData(hXLB, sel);
    if (idx < 0 || idx >= m_toc.GetSize() || !(m_toc.Flags(idx)&Bookmarks::BMK))
        return;

    CAddBmDialog   dlg(this);
    dlg.m_text = m_toc.Text(idx, m_tp);
    if (dlg.DoModal() == IDOK && dlg.m_text != m_toc.Text(idx, m_tp)) {
        m_toc.Change(idx, dlg.m_text);
        XLB_SetItemText1(hXLB, sel, dlg.m_text);
    }
}

void  CContentsDlg::OnDelete() {
    HWND     hXLB = ::GetDlgItem(m_hWnd, IDC_CONTENTS);

    int     sel = XLB_GetSelection(hXLB);
    if (sel < 0)
        return;

    int     idx = XLB_GetData(hXLB, sel);
    if (idx >= 0 && idx < m_toc.GetSize() && m_toc.Flags(idx)&Bookmarks::BMK &&
        MessageBox(_T("Delete this bookmark?"), _T("Confirm"), MB_YESNO) == IDYES)
    {
        m_toc.Remove(idx);
        XLB_DeleteItem(hXLB, sel);
    }
}

void  CContentsDlg::OnL1() {
    XLB_CollapseLevel(::GetDlgItem(m_hWnd, IDC_CONTENTS), 0);
}

void  CContentsDlg::OnL2() {
    XLB_CollapseLevel(::GetDlgItem(m_hWnd, IDC_CONTENTS), 1);
}

void  CContentsDlg::OnL3() {
    XLB_CollapseLevel(::GetDlgItem(m_hWnd, IDC_CONTENTS), 2);
}

void  CContentsDlg::OnL4() {
    XLB_CollapseLevel(::GetDlgItem(m_hWnd, IDC_CONTENTS), 3);
}

void  CContentsDlg::OnL5() {
    XLB_CollapseLevel(::GetDlgItem(m_hWnd, IDC_CONTENTS), 4);
}

void  CContentsDlg::OnL0() {
    XLB_CollapseLevel(::GetDlgItem(m_hWnd, IDC_CONTENTS), -1);
}
