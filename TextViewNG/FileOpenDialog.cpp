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
* $Id: FileOpenDialog.cpp,v 1.24.2.15 2006/07/01 20:28:47 mike Exp $
*
*/

#pragma warning(disable:4100)
#define _WIN32_WINNT _WIN32_WINNT_MAXVER

#include <afxcmn.h>

#include "resource.h"

#include "ptr.h"
#include "FileOpenDialog.h"
#include "TextViewNG.h"
#include "config.h"
#include "XListBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define T_UP 0
#define T_DIR 1
#define T_FILE 2

// string compare
#define CmpI(s1,s2) (::CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, (s1), -1, (s2), -1) - 2)

CString GetFileName(CString *filepath, CWnd *parent)
{
 CFileOpenDialog dlg(parent);

 if (filepath && filepath->GetLength() > 0)
 {
 DWORD attr = GetFileAttributes(*filepath);
 if (attr != 0xffffffff && ((attr&FILE_ATTRIBUTE_DIRECTORY)))
 {
 dlg.m_path = *filepath;
 }
 }
 int ret = dlg.DoModal();
 if (filepath)
 {
 *filepath = dlg.m_path;
 }
 if (ret == IDOK)
 {
 return dlg.m_filename;
 }
 else
 {
 return CString();
 }
}

/////////////////////////////////////////////////////////////////////////////
// CFileOpenDialog dialog


CFileOpenDialog::CFileOpenDialog(CWnd* pParent /*=NULL*/)
: CDialog(CFileOpenDialog::IDD, pParent), m_path(_T(""))
{
 //{{AFX_DATA_INIT(CFileOpenDialog)
 // NOTE: the ClassWizard will add member initialization here
 //}}AFX_DATA_INIT
}


void CFileOpenDialog::DoDataExchange(CDataExchange* pDX)
{
 CDialog::DoDataExchange(pDX);
 //{{AFX_DATA_MAP(CFileOpenDialog)
 // NOTE: the ClassWizard will add DDX and DDV calls here
 //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFileOpenDialog, CDialog)
 //{{AFX_MSG_MAP(CFileOpenDialog)
 ON_MESSAGE(XLM_CLICK, OnXClick)
 ON_WM_SIZE()
 //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileOpenDialog message handlers

BOOL CFileOpenDialog::OnInitDialog()
{
 CDialog::OnInitDialog();

 HWND hList = ::GetDlgItem(m_hWnd, IDC_FILELIST);
 XLB_SetImageList(hList, CTVApp::ImageList()->GetSafeHandle(), true);

 // fill in the dialog
 DWORD attr = GetFileAttributes(m_path);
 if (attr&FILE_ATTRIBUTE_DIRECTORY)
 FindFiles();
 else
 OpenItem(m_path);
 return TRUE;
}

static int get_file_icon(const TCHAR *name, bool inzip = false) {
 int l = _tcslen(name);
 if (l <= 4)
 return IM_FILE;
 if (!CmpI(name + l - 4, _T(".txt")) || !CmpI(name + l - 4, _T(".xml")) || !CmpI(name + l - 4, _T(".dic")))
 return IM_TEXT;
 return IM_FILE;
}

static void FormatSize(__int64 size, CString& str) {
 if (size < 1000)
 str.Format(_T("%d"), (int)size);
 else if (size < 1024 * 10) // < 10k
 str.Format(_T("%.2fk"), size / 1024.0);
 else if (size < 1024 * 100) // < 100k
 str.Format(_T("%.1fk"), size / 1024.0);
 else if (size < 1024 * 1000) // < 1000k
 str.Format(_T("%dk"), (int)(size >> 10));
 else if (size < 1024 * 1024 * 10) // < 10m
 str.Format(_T("%.2fm"), size / 1048576.0);
 else if (size < 1024 * 1024 * 100) // < 100m
 str.Format(_T("%.1fm"), size / 1048576.0);
 else if (size < 1024 * 1024 * 1000) // < 1000m
 str.Format(_T("%dm"), (int)(size >> 20));
 else if (size < 1024 * 1024 * 1024 * 10i64) // < 10g
 str.Format(_T("%.2fg"), size / 1073741824.0);
 else if (size < 1024 * 1024 * 1024 * 100i64) // < 100g
 str.Format(_T("%.1fg"), size / 1073741824.0);
 else // >= 100g
 str.Format(_T("%d"), (int)(size >> 30));
}

void CFileOpenDialog::FindFiles(bool showall)
{
 // show current path
 CString tmppath(m_path);
 if (tmppath.GetLength() > 0 && tmppath[0] == _T('\\'))
 tmppath.Delete(0);
 SetDlgItemText(IDC_FILEPATH, tmppath);

 // fetch our control
 HWND hList = ::GetDlgItem(m_hWnd, IDC_FILELIST);
 // delete all files
 XLB_DeleteAllItems(hList);

 XLB_Handle *handle = XLB_GetHandle(hList);

 LARGE_INTEGER ui;
 CString tmp;

 // have to handle fucking disk drives
 if (m_path.GetLength() == 0 || (m_path.GetLength() == 1 && m_path[0] == _T('\\')))
 {
 TCHAR buf[4];
 buf[1] = _T(':'); buf[3] = _T('\0');
 int i = 0;
 for (TCHAR drive = 'A'; drive <= 'Z'; ++drive) {
 buf[0] = drive;
 buf[2] = _T('\\');
 UINT dt = GetDriveType(buf);
 if (dt != DRIVE_UNKNOWN && dt != DRIVE_NO_ROOT_DIR) {
 buf[2] = _T('\0');
 XLB_AppendItem(handle, buf, NULL, IM_DIR, 0, T_DIR);
 ++i;
 }
 }
 }
 else
 {
 // build a search pattern
 CString pat = m_path;
 if (pat.GetLength() == 0 || pat[pat.GetLength() - 1] != _T('\\'))
 pat += _T('\\');
 pat += _T('*');

 // remove leading \ 
 if (pat.GetLength() > 0 && pat[0] == _T('\\'))
 pat.Delete(0);

 // now try to find files
 WIN32_FIND_DATA fd;
 HANDLE fh;
 bool run;

 fh = FindFirstFile(pat, &fd);
 run = fh != INVALID_HANDLE_VALUE;
 for (; run;) {
 if (!(fd.cFileName[0] == _T('.') && (!fd.cFileName[1] || (fd.cFileName[1] == _T('.') && !fd.cFileName[2]))))
 {
 bool dir = (fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) != 0;
 int icon = IM_DIR;
 if (!dir) {
 icon = get_file_icon(fd.cFileName);
 ui.LowPart = fd.nFileSizeLow;
 ui.HighPart = fd.nFileSizeHigh;
 FormatSize(ui.QuadPart, tmp);
 }
 if (dir || showall || icon != IM_FILE)
 XLB_AppendItem(handle, fd.cFileName, dir ? NULL : (const TCHAR *)tmp,
 dir ? IM_DIR : icon, 0, dir ? T_DIR : T_FILE);
 }
 run = FindNextFile(fh, &fd) != 0;
 }
 if (fh != INVALID_HANDLE_VALUE)
 FindClose(fh);

 if (m_path.GetLength() > 1)
 XLB_AppendItem(handle, _T(".."), NULL, IM_DIR, 0, T_UP);
 }
 XLB_UpdateState(hList);
 XLB_SortItems(hList);
 ::InvalidateRect(hList, NULL, FALSE);
}

LRESULT CFileOpenDialog::OnXClick(WPARAM wParam, LPARAM lParam)
{
 HWND list = ::GetDlgItem(m_hWnd, IDC_FILELIST);
 ActivateItem(list, XLB_GetSelection(list));
 return 0;
}

void CFileOpenDialog::ActivateItem(HWND list, int item) {
 int type = XLB_GetData(list, item);

 if (type == T_DIR || type == T_UP) {
 // change dir
 CString path = XLB_GetItemText1(list, item);
 CString last;
 if (path == _T("..")) {
 // one level up
 int bsp = m_path.ReverseFind(_T('\\'));
 if (bsp > 0) {
 last = m_path.Right(m_path.GetLength() - bsp - 1);
 m_path = m_path.Left(bsp);
 }
 else {
 last = m_path;
 if (!last.IsEmpty() && last[0] == _T('\\'))
 last.Delete(0);
 m_path = _T("\\");
 }
 }
 else {
 if (m_path.GetLength() > 0 && m_path[m_path.GetLength() - 1] != _T('\\'))
 m_path += _T('\\');
 m_path += path;
 }
 FindFiles();
 int itemCount = XLB_GetItemCount(list);
 for (int i = 0; i < itemCount; ++i)
 if (last == XLB_GetItemText1(list, i)) {
 XLB_SetSelection(list, i);
 XLB_EnsureVisible(list, i, false);
 break;
 }
 }
 else {
 // selected item
 m_filename = m_path;
 if (m_filename.GetLength() == 0 || m_filename[m_filename.GetLength() - 1] != _T('\\'))
 m_filename += _T('\\');
 m_filename += XLB_GetItemText1(list, item);
 // strip leading \ 
 if (m_filename.GetLength() > 0 && m_filename[0] == _T('\\'))
 m_filename.Delete(0);
 EndDialog(IDOK);
 }
}

void CFileOpenDialog::OnSize(UINT nType, int cx, int cy)
{
 CDialog::OnSize(nType, cx, cy);
 RECT r, rc;
 GetClientRect(&r);
 rc = r;
 rc.bottom = rc.top + HIWORD(GetDialogBaseUnits()) + 5;
 --rc.left; --rc.top; ++rc.right;
 CWnd *ctl = GetDlgItem(IDC_FILEPATH);
 if (ctl)
 ctl->MoveWindow(&rc);
 CWnd *list = GetDlgItem(IDC_FILELIST);
 if (list) {
 rc = r;
 rc.top += HIWORD(GetDialogBaseUnits()) + 5;
 list->MoveWindow(&rc);
 XLB_EnsureVisible(list->m_hWnd, XLB_GetSelection(list->m_hWnd));
 }
}

void CFileOpenDialog::OnOK() {
 HWND list = ::GetDlgItem(m_hWnd, IDC_FILELIST);
 int item = XLB_GetSelection(list);
 if (item >= 0)
 ActivateItem(list, item);
}

void CFileOpenDialog::OpenItem(const CString &path) {
 // looks like a zip file
 CFile *fp = new CFile;
 CFileException ex;
 if (!fp->Open(path, CFile::modeRead | CFile::shareDenyWrite, &ex)) {
 MessageBox(_T("Can't open zip file"), _T("Error"), MB_OK | MB_ICONERROR);
 delete fp;
 }
}
