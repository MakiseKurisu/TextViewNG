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
 * $Id: TVFrame.h,v 1.36.2.8 2004/10/29 15:18:46 mike Exp $
 * 
 */

#if !defined(AFX_TVFRAME_H__4A0EA62A_A9B7_4B85_B071_810F0B329F71__INCLUDED_)
#define AFX_TVFRAME_H__4A0EA62A_A9B7_4B85_B071_810F0B329F71__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CTVFrame : public CFrameWnd
{
public:
  CTVFrame();
protected:
  DECLARE_DYNAMIC(CTVFrame)

    // Attributes
public:

  // Operations
public:

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CTVFrame)
public:
  virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
protected:
  virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
  //}}AFX_VIRTUAL

  // Implementation
public:
  virtual ~CTVFrame();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif

  bool		  InitView();
  void		  InitWindowPos(int nCmdShow);


protected:  // control bar embedded members

#ifdef _WIN32_WCE
  CCeCommandBar		m_wndCommandBar;
#else
  CToolBar		m_wndCommandBar;
  CMenu			*m_mainmenu;
#endif

  auto_ptr<CWnd>	m_wndView;
  bool			m_realview;
  FILETIME		m_toptime;
  CStringArray		m_recentlist;

  TCHAR			**m_tooltips;
  int			m_buttoncount;
  RECT			m_normsize;
  bool			m_okstate;

#if BE300
  bool			m_fSQTray;
#endif

  HMENU			m_recent;
  HMENU			m_dicts;
  HMENU			m_colors;
  int			m_in_fullscreen;
  bool			m_fullscreen;

#ifndef _WIN32_WCE
  WINDOWPLACEMENT	m_wndpos;
#endif
  void			SaveWndPos();

  void			DoFullScreen(bool fs);
  void			UpdateRecentFiles(CMenu *menu);
  void			UpdateDictionaries(CMenu *menu);
  void			UpdateColors(CMenu *menu);
  bool			TryOpenFile(CString& filename);
  //{{AFX_MSG(CTVFrame)
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnSetFocus(CWnd *pOldWnd);
  afx_msg void OnFullscreen();
  afx_msg void OnUpdateFullscreen(CCmdUI* pCmdUI);
  afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
  afx_msg void OnUpdateFileOpen(CCmdUI* pCmdUI);
  afx_msg void OnFileOpen();
  afx_msg void OnInitView();
  afx_msg BOOL OnCopyData(CWnd *pWnd,COPYDATASTRUCT *pcd);
  afx_msg void OnUpdateMainTools(CCmdUI* pCmdUI);
  afx_msg void OnUpdateMainOptions(CCmdUI* pCmdUI);
  afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnMove(int x, int y);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  //}}AFX_MSG
  afx_msg void OnRecentFile(UINT cmd);
  afx_msg void OnUpdateRecentFile(CCmdUI *pCmdUI);
  afx_msg LRESULT OnOpenFile(WPARAM wParam,LPARAM lParam);
#ifdef WM_POWERBROADCAST
  afx_msg LRESULT OnPower(WPARAM wParam,LPARAM lParam) {
    if (m_wndView.get()) return m_wndView->SendMessage(WM_POWERBROADCAST,wParam,lParam);
    return TRUE;
  }
#endif
  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TVFRAME_H__4A0EA62A_A9B7_4B85_B071_810F0B329F71__INCLUDED_)
