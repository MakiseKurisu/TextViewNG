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
* $Id: TVFrame.cpp,v 1.75.2.20 2004/11/01 11:15:10 mike Exp $
*
*/

#define _WIN32_WINNT _WIN32_WINNT_MAXVER

#include <afxcmn.h>
#include <afxtempl.h>
#include <afxext.h>

#include "resource.h"

#include "ptr.h"
#include "Unicode.h"
#include "TextViewNG.h"
#include "TVFrame.h"
#include "FileOpenDialog.h"
#include "DummyView.h"
#include "Keys.h"
#include "TextFile.h"
#include "TextFormatter.h"
#include "Rotate.h"
#include "TView.h"
#include "config.h"
#include "Colors.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MSG_OPEN_FILE (WM_APP+1)

#define MAXTTSIZE 128

/////////////////////////////////////////////////////////////////////////////
// CTVFrame

IMPLEMENT_DYNAMIC(CTVFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CTVFrame, CFrameWnd)
    //{{AFX_MSG_MAP(CTVFrame)
    ON_WM_CREATE()
    ON_WM_SETFOCUS()
    ON_COMMAND(ID_FULLSCREEN, OnFullscreen)
    ON_UPDATE_COMMAND_UI(ID_FULLSCREEN, OnUpdateFullscreen)
    ON_WM_ACTIVATE()
    ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, OnUpdateFileOpen)
    ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
    ON_COMMAND(ID_INIT, OnInitView)
    ON_WM_COPYDATA()
    ON_UPDATE_COMMAND_UI(ID_MAIN_TOOLS, OnUpdateMainTools)
    ON_UPDATE_COMMAND_UI(ID_MAIN_OPTIONS, OnUpdateMainOptions)
    ON_WM_INITMENUPOPUP()
    ON_WM_SIZE()
    ON_WM_MOVE()
    ON_WM_NCMOUSEMOVE()
    ON_WM_MOVING()
    ON_WM_ERASEBKGND()
    //}}AFX_MSG_MAP
    ON_COMMAND_RANGE(RECENT_BASE, RECENT_BASE + RECENT_FILES, OnRecentFile)
    ON_UPDATE_COMMAND_UI_RANGE(RECENT_BASE, RECENT_BASE + RECENT_FILES, OnUpdateRecentFile)
    ON_MESSAGE(MSG_OPEN_FILE, OnOpenFile)
    ON_MESSAGE(WM_POWERBROADCAST, OnPower)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTVFrame construction/destruction

CTVFrame::CTVFrame() : m_fullscreen(false), m_in_fullscreen(0),
m_realview(false), m_tooltips(NULL), m_buttoncount(0),
m_okstate(false)
{
    m_wndView = new DummyView;
    m_toptime.dwLowDateTime = m_toptime.dwHighDateTime = 0;
    m_tooltips = NULL;
    m_buttoncount = 0;
    m_mainmenu = NULL;
    memset(&m_wndpos, 0, sizeof(m_wndpos));

    m_Docking.bLeft = false;
    m_Docking.bRight = false;
    m_Docking.bTop = false;
    m_Docking.bBottom = false;
    m_Docking.pPoint.x = 0;
    m_Docking.pPoint.y = 0;
}

CTVFrame::~CTVFrame()
{
    for (int i = 0; i < m_buttoncount; ++i)
        delete[] m_tooltips[i];
    delete[] m_tooltips;
    delete m_mainmenu;
    if (m_wndpos.length)
        AfxGetApp()->WriteProfileBinary(_T("Parameters"), _T("WindowPos"), (LPBYTE) &m_wndpos, sizeof(m_wndpos));
}

int CTVFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;
    if (!m_wndView->Create(NULL, NULL, WS_VISIBLE, // XXX
        CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
    {
        TRACE0("Failed to create view window\n");
        return -1;
    }

    if (!m_wndCommandBar.CreateEx(this, TBSTYLE_FLAT) ||
        !m_wndCommandBar.LoadToolBar(IDR_MAINFRAME))
    {
        TRACE0("Failed to create CommandBar\n");
        return -1; // fail to create
    }

    GetWindowRect(&m_normsize);

    m_recent = CreatePopupMenu();
    CMenu *main = new CMenu;
    if (main)
    {
        main->LoadMenu(IDR_MAINFRAME);
        CMenu *sub = main->GetSubMenu(0);
        if (sub)
            sub->InsertMenu(ID_FILE_OPEN, MF_BYCOMMAND | MF_POPUP, (UINT) m_recent, _T("Recent Files"));
        m_mainmenu = main;
        SetMenu(NULL);
    }

    return 0;
}

void CTVFrame::InitWindowPos(int nCmdShow)
{
    UINT sz;
    LPBYTE buffer = NULL;
    if (AfxGetApp()->GetProfileBinary(_T("Parameters"), _T("WindowPos"), &buffer, &sz) &&
        sz == sizeof(m_wndpos) && buffer)
    {
        memcpy(&m_wndpos, buffer, sizeof(m_wndpos));
        ::SetWindowPlacement(m_hWnd, &m_wndpos);
    }
    else
        ShowWindow(nCmdShow);
    delete[] buffer;
}

/////////////////////////////////////////////////////////////////////////////
// CTVFrame diagnostics

#ifdef _DEBUG
void CTVFrame::AssertValid() const
{
    CFrameWnd::AssertValid();
}

void CTVFrame::Dump(CDumpContext& dc) const
{
    CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTVFrame message handlers
void CTVFrame::OnSetFocus(CWnd* pOldWnd)
{
    // forward focus to the view window
    if (m_wndView.get() && m_wndView->m_hWnd)
        m_wndView->SetFocus();
}

BOOL CTVFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
    // let the view have first crack at the command
    if (m_wndView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
        return TRUE;

    // otherwise, do default handling
    return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CTVFrame::DoFullScreen(bool fs)
{
    if (m_in_fullscreen)
        return;
    m_in_fullscreen++;
    if (fs)
    {
        ::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
        ModifyStyle(WS_CAPTION | WS_THICKFRAME, 0, SWP_FRAMECHANGED);
        m_wndCommandBar.ShowWindow(SW_HIDE);
        ::SetWindowPos(m_hWnd, NULL, 0, 0,
            GetSystemMetrics(SM_CXSCREEN),
            GetSystemMetrics(SM_CYSCREEN),
            SWP_NOZORDER | SWP_NOACTIVATE);
    }
    else
    {
        ::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
        ModifyStyle(0, WS_CAPTION | WS_THICKFRAME, SWP_FRAMECHANGED);
        m_wndCommandBar.ShowWindow(SW_SHOW);
        ::SetWindowPos(m_hWnd, NULL,
            m_normsize.left, m_normsize.top,
            m_normsize.right - m_normsize.left, m_normsize.bottom - m_normsize.top,
            SWP_NOZORDER | SWP_NOACTIVATE);
    }
    m_in_fullscreen--;
}

void CTVFrame::OnFullscreen()
{
    m_fullscreen = !m_fullscreen;
    DoFullScreen(m_fullscreen);
}

void CTVFrame::OnUpdateFullscreen(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_fullscreen);
    pCmdUI->Enable();
}

void CTVFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
    CFrameWnd::OnActivate(nState, pWndOther, bMinimized);
    if (nState == WA_ACTIVE || nState == WA_CLICKACTIVE)
    {
        DoFullScreen(m_fullscreen);
        // this hack is needed for CE 2.x and HPC 2k because I am too lazy to find out
        // what's wrong with focus setting
        if (m_wndView.get() && m_wndView->m_hWnd)
        {
            // always focus view when we are activated
            m_wndView->SetFocus();
            Keys::SetWindow(m_wndView->m_hWnd);
        }
    }
    else
    {
        // we should really remove fullscreen when the frame is deactivated
        // haha fun, this isnt called under emulation, so take care
        DoFullScreen(false);
        Keys::SetWindow(0);
    }
}

bool CTVFrame::TryOpenFile(CString& filename)
{
    if (filename.GetLength() > 0)
    {
        // selected some file
        TextFile *tf = TextFile::Open(filename);
        if (tf == NULL && filename.GetLength() > 2 && filename[0] == _T('"') &&
            filename[filename.GetLength() - 1] == _T('"'))
        {
            filename = filename.Mid(1, filename.GetLength() - 2);
            tf = TextFile::Open(filename);
        }
        if (tf)
        {
            if (m_realview)
            {
                auto_ptr<TextFile> text(tf);
                ((CTView *) m_wndView.get())->SetFile(text);
                SetWindowText(_T("TextViewNG: ") + FileName(filename));
            }
            return true;
        }
        else
        {
            CString msg;
            msg.Format(_T("Can't open file '%s'"), (LPCTSTR) filename);
            MessageBox(msg, _T("Error"), MB_ICONERROR | MB_OK);
        }
    }
    return false;
}

void CTVFrame::OnUpdateFileOpen(CCmdUI* pCmdUI)
{
    pCmdUI->Enable();
}

void CTVFrame::OnFileOpen()
{
    CString ipath = CTVApp::GetStr(_T("OpenPath"));
    CString sv = ipath;
    CString str = GetFileName(&ipath, this);
    if (sv != ipath)
    {
        CTVApp::SetStr(_T("OpenPath"), ipath);
    }
    TryOpenFile(str);
}

BOOL CTVFrame::OnCopyData(CWnd *pWnd, COPYDATASTRUCT *pcd)
{
    // filename is always unicode here
    CString filename(Unicode::ToCS((const wchar_t *) pcd->lpData, pcd->cbData / sizeof(wchar_t)));
    if (filename.GetLength() == 0)
        PostMessage(MSG_OPEN_FILE, 0, 0);
    else
        PostMessage(MSG_OPEN_FILE, 0, (LPARAM)new CString(filename));
    return TRUE;
}

void CTVFrame::SaveWndPos()
{
    WINDOWPLACEMENT pl;
    memset(&pl, 0, sizeof(pl));
    pl.length = sizeof(pl);
    ::GetWindowPlacement(m_hWnd, &pl);
    if (pl.showCmd != SW_MAXIMIZE && pl.showCmd != SW_SHOWMAXIMIZED)
        pl.showCmd = SW_SHOWNORMAL;
    memcpy(&m_wndpos, &pl, sizeof(m_wndpos));
}

bool CTVFrame::InitView()
{
    bool dictmode = false;
    // initialize xml parser
    //XMLParser::LoadStyles();
    bool triednoquote = false;
    CString filename(AfxGetApp()->m_lpCmdLine);
    if (filename == _T("-d")) // show dictionary
    {
        filename = _T("NUL");
        dictmode = true;
    }
    if (filename.GetLength() == 0) // no filename provided, try to fetch most recent
    {
        filename = Bookmarks::find_last_file();
    }
    TextFile *tf;
    for (;;) // repeat this until it works, or until the user chooses to exit
    {
        if (filename.GetLength() == 0) // still no luck, open fs browser
        {
            CString ipath = CTVApp::GetStr(_T("OpenPath"));
            CString sv(ipath);
            filename = GetFileName(&ipath, this);
            if (sv != ipath)
            {
                CTVApp::SetStr(_T("OpenPath"), ipath);
            }
        }
        if (filename.GetLength() == 0) // everything failed, barf at the user
        {
            MessageBox(_T("No file name provided on the command line, and no recent files found."), _T("Error"), MB_ICONERROR | MB_OK);
            return false;
        }
        tf = TextFile::Open(filename);
        if (tf == NULL)
        {
            if (!triednoquote && filename.GetLength() > 2 && filename[0] == _T('"') && filename[filename.GetLength() - 1] == _T('"'))
            {
                triednoquote = true;
                filename = filename.Mid(1, filename.GetLength() - 2);
                continue;
            }
            CString msg;
            msg.Format(_T("Can't open file '%s'"), (LPCTSTR) filename);
            MessageBox(msg, _T("Error"), MB_ICONERROR | MB_OK);
            filename.Empty();
        }
        else
        {
            break;
        }
    }
    // replace a dummy view with a real one
    HWND wnd = m_wndView->Detach();
    CTView *tv = new CTView;
    tv->Attach(wnd);
    tv->Init();
    auto_ptr<TextFile> text(tf);
    tv->SetFile(text);
    SetWindowText(_T("TextViewNG: ") + FileName(filename));
    m_wndView.reset(tv);
    m_realview = true;
    return true;
}

void CTVFrame::OnInitView()
{
    if (!InitView())
        PostMessage(WM_COMMAND, ID_APP_EXIT);
}

void CTVFrame::UpdateRecentFiles(CMenu *menu)
{
    FILETIME tm;
    m_recentlist.RemoveAll();
    Bookmarks::get_recent_files(m_recentlist, RECENT_FILES, tm);
    if (tm.dwLowDateTime != m_toptime.dwLowDateTime || tm.dwHighDateTime != m_toptime.dwHighDateTime)
    {
        m_toptime = tm;
        while (menu->RemoveMenu(0, MF_BYPOSITION));
        for (int ii = 0; ii < m_recentlist.GetSize(); ++ii)
            menu->AppendMenu(MF_STRING, RECENT_BASE + ii, FileName(m_recentlist[ii]));
        if (m_recentlist.GetSize() == 0)
            menu->AppendMenu(MF_STRING | MF_GRAYED, 1, _T("No Files"));
    }
}

void CTVFrame::UpdateColors(CMenu *menu)
{
    for (;;)
    {
        MENUITEMINFO ii;
        memset(&ii, 0, sizeof(ii));
        ii.cbSize = sizeof(ii);
        ii.fMask = MIIM_TYPE;
        if (!menu->GetMenuItemInfo(2, &ii, TRUE))
            break;
        if (ii.fType & MFT_SEPARATOR)
            break;
        menu->DeleteMenu(2, MF_BYPOSITION);
    }

    AddColorProfileNames(menu, 2);
}

void CTVFrame::OnUpdateRecentFile(CCmdUI *pCmdUI)
{
    pCmdUI->Enable();
}

void CTVFrame::OnRecentFile(UINT cmd)
{
    if (cmd - RECENT_BASE < (UINT) m_recentlist.GetSize())
    {
        TextFile *tf = TextFile::Open(m_recentlist[cmd - RECENT_BASE]);
        if (tf)
        {
            if (m_realview)
            {
                auto_ptr<TextFile> text(tf);
                ((CTView *) m_wndView.get())->SetFile(text);
                SetWindowText(_T("TextViewNG: ") + FileName(m_recentlist[cmd - RECENT_BASE]));
            }
        }
        else
        {
            CString msg;
            msg.Format(_T("Can't open file '%s'"), (LPCTSTR) m_recentlist[cmd - RECENT_BASE]);
            MessageBox(msg, _T("Error"), MB_ICONERROR | MB_OK);
        }
    }
}

static void GetTbPopupPoint(CToolBarCtrl& tb, UINT cmd, int& x, int& y, int& align)
{
    RECT rc;
    tb.GetItemRect(tb.CommandToIndex(cmd), &rc);
    tb.ClientToScreen(&rc);
    x = rc.left;
    y = rc.bottom;
    align = TPM_TOPALIGN;
}

void CTVFrame::OnUpdateMainTools(CCmdUI* pCmdUI)
{
    pCmdUI->Enable();
}

void CTVFrame::OnUpdateMainOptions(CCmdUI* pCmdUI)
{
    pCmdUI->Enable();
}

BOOL CTVFrame::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
    TBNOTIFY *tn = (TBNOTIFY*) lParam;

    if (tn->hdr.code == TBN_BEGINDRAG)
    {
        // abuse the notification to show a popup menu
        int sm = -1;
        switch (tn->iItem)
        {
        case ID_MAIN_TOOLS:
            sm = 0;
            break;
        case ID_MAIN_OPTIONS:
            sm = 1;
            break;
        }
        if (sm >= 0)
        {
            CMenu *menu = m_mainmenu;
            if (menu)
            {
                CMenu *sub = menu->GetSubMenu(sm);
                if (sub)
                {
                    int x, y, align;
                    GetTbPopupPoint(m_wndCommandBar.GetToolBarCtrl(), tn->iItem, x, y, align);
                    sub->TrackPopupMenu(TPM_LEFTALIGN | align, x, y, this, NULL);
                    // unwedge the toolbar :)
                    m_wndCommandBar.GetToolBarCtrl().PressButton(tn->iItem, FALSE);
                }
            }
        }
    }
    return CFrameWnd::OnNotify(wParam, lParam, pResult);
}

void CTVFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
    CFrameWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
    if (pPopupMenu->m_hMenu == m_recent)
        UpdateRecentFiles(pPopupMenu);
    else if (pPopupMenu->m_hMenu == m_colors)
        UpdateColors(pPopupMenu);
}

void CTVFrame::OnSize(UINT nType, int cx, int cy)
{
    CFrameWnd::OnSize(nType, cx, cy);

    if (cx > 0 && cy > 0 && !m_in_fullscreen)
        GetWindowRect(&m_normsize);

    SaveWndPos();
}

void CTVFrame::OnMove(int x, int y)
{
    CFrameWnd::OnMove(x, y);

    if (!m_in_fullscreen)
        GetWindowRect(&m_normsize);
    SaveWndPos();
}

void CTVFrame::OnNcMouseMove(UINT nHitTest, CPoint point)
{
    if (m_Docking.bLeft || m_Docking.bRight)
    {
        m_Docking.pPoint.x = point.x;
    }
    if (m_Docking.bTop || m_Docking.bBottom)
    {
        m_Docking.pPoint.y = point.y;
    }

    CFrameWnd::OnNcMouseMove(nHitTest, point);
}

void CTVFrame::OnMoving(UINT fwSide, LPRECT pRect)
{
    const int DOCK_WIDTH = 30;
    CRect rectScreen;
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rectScreen, 0);
    POINT curPnt;
    ::GetCursorPos(&curPnt);

    if (m_Docking.bRight)
    {
        if (curPnt.x < m_Docking.pPoint.x - DOCK_WIDTH / 2)
        {
            pRect->left = rectScreen.right - (pRect->right - pRect->left) - DOCK_WIDTH;
            pRect->right = rectScreen.right - DOCK_WIDTH;
            if (pRect->left < 0 && m_Docking.bLeft)
            {
                pRect->right = pRect->right - pRect->left;
                pRect->left = 0;
            }
            m_Docking.bRight = false;
        }
        else
        {
            pRect->left = rectScreen.right - (pRect->right - pRect->left);
            pRect->right = rectScreen.right;
        }
    }
    else if (m_Docking.bLeft)
    {
        if (curPnt.x > m_Docking.pPoint.x + DOCK_WIDTH / 2)
        {
            pRect->right = rectScreen.left + (pRect->right - pRect->left) + DOCK_WIDTH;
            pRect->left = rectScreen.left + DOCK_WIDTH;
            if (pRect->right < 0 && m_Docking.bRight)
            {
                pRect->left = pRect->left - pRect->right;
                pRect->right = 0;
            }
            m_Docking.bLeft = false;
        }
        else
        {
            pRect->right = rectScreen.left + (pRect->right - pRect->left);
            pRect->left = rectScreen.left;
        }
    }
    else if (!m_Docking.bRight && pRect->right > rectScreen.right - DOCK_WIDTH)
    {
        pRect->left = rectScreen.right - (pRect->right - pRect->left);
        pRect->right = rectScreen.right;
        m_Docking.bRight = true;
        m_Docking.pPoint = curPnt;
    }
    else if (!m_Docking.bLeft && pRect->left < rectScreen.left + DOCK_WIDTH)
    {
        pRect->right = rectScreen.left + (pRect->right - pRect->left);
        pRect->left = rectScreen.left;
        m_Docking.bLeft = true;
        m_Docking.pPoint = curPnt;
    }

    if (m_Docking.bBottom)
    {
        if (curPnt.y < m_Docking.pPoint.y - DOCK_WIDTH / 2)
        {
            pRect->top = rectScreen.bottom - (pRect->bottom - pRect->top) - DOCK_WIDTH;
            pRect->bottom = rectScreen.bottom - DOCK_WIDTH;
            if (pRect->top < 0 && m_Docking.bTop)
            {
                pRect->bottom = pRect->bottom - pRect->top;
                pRect->top = 0;
            }
            m_Docking.bBottom = false;
        }
        else
        {
            pRect->top = rectScreen.bottom - (pRect->bottom - pRect->top);
            pRect->bottom = rectScreen.bottom;
        }
    }
    else if (m_Docking.bTop)
    {
        if (curPnt.y > m_Docking.pPoint.y + DOCK_WIDTH / 2)
        {
            pRect->bottom = rectScreen.top + (pRect->bottom - pRect->top) + DOCK_WIDTH;
            pRect->top = rectScreen.top + DOCK_WIDTH;
            if (pRect->bottom < 0 && m_Docking.bBottom)
            {
                pRect->top = pRect->top - pRect->bottom;
                pRect->bottom = 0;
            }
            m_Docking.bTop = false;
        }
        else
        {
            pRect->bottom = rectScreen.top + (pRect->bottom - pRect->top);
            pRect->top = rectScreen.top;
        }
    }
    else if (!m_Docking.bBottom && pRect->bottom > rectScreen.bottom - DOCK_WIDTH)
    {
        pRect->top = rectScreen.bottom - (pRect->bottom - pRect->top);
        pRect->bottom = rectScreen.bottom;
        m_Docking.bBottom = true;
        m_Docking.pPoint = curPnt;
    }
    else if (!m_Docking.bTop && pRect->top < rectScreen.top + DOCK_WIDTH)
    {
        pRect->bottom = rectScreen.top + (pRect->bottom - pRect->top);
        pRect->top = rectScreen.top;
        m_Docking.bTop = true;
        m_Docking.pPoint = curPnt;
    }

    CFrameWnd::OnMoving(fwSide, pRect);
}

LRESULT CTVFrame::OnOpenFile(WPARAM wParam, LPARAM lParam)
{
    CString *str = (CString*) lParam;
    if (str)
    {
        TryOpenFile(*str);
        delete str;
    }
    return 0;
}

BOOL CTVFrame::OnEraseBkgnd(CDC* pDC)
{
    return FALSE; // frame has no background
}

/////////////////////////////////////////////////////
// some hires support
static int getDPI()
{
    HDC hDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
    int dpi = GetDeviceCaps(hDC, LOGPIXELSY);
    DeleteDC(hDC);
    return dpi;
}

UINT getIDR_DIALOG()
{
    return getDPI() >= 120 ? IDR_DIALOG_HR : IDR_DIALOG;
}

UINT getIDR_CONTENTS()
{
    return getDPI() >= 120 ? IDR_CONTENTS_HR : IDR_CONTENTS;
}

UINT getIDR_MAINFRAME()
{
    return getDPI() >= 120 ? IDR_MAINFRAME_HR : IDR_MAINFRAME;
}
