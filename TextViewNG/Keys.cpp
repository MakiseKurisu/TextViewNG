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
 * $Id: Keys.cpp,v 1.8.2.13 2006/05/07 18:08:15 mike Exp $
 * 
 */

#pragma warning(disable:4100)
#define _WIN32_WINNT	_WIN32_WINNT_MAXVER

#include <afxext.h>

#include "Keys.h"
#include "resource.h"
#include "config.h"

#define	BUTTON_W_CH   7
#define	BUTTON_SPACE  5
#define	MARGIN	      3

#define	LABEL_BASE    1000
#define	LABEL_MAX     1999
#define	BUTTON_BASE   2000
#define	BUTTON_MAX    2999
#define	ACTION_BASE   3000
#define	ACTION_MAX    3999
#define	KEY_BASE      4000
#define	KEY_MAX	      4999

#define	MINHK	      0xc0
#define	MAXHK	      0xcf

// keys
static struct {
  const TCHAR   *name;
  UINT		cmd;
  UINT		vk1,vk2,vk3;
} g_actions[]={
  { _T("Line forward"),	    ID_LINE_DOWN				    },
  { _T("Line backward"),    ID_LINE_UP					    },
  { _T("Page forward"),	    ID_PAGE_DOWN,   VK_DOWN,  VK_NEXT,	  VK_SPACE  },
  { _T("Page backward"),    ID_PAGE_UP,	    VK_UP,    VK_PRIOR		    },
  { _T("Start of file"),    ID_START_OF_FILE, VK_HOME			    },
  { _T("End of file"),	    ID_END_OF_FILE, VK_END			    },
  { _T("Toggle fullscreen"),ID_FULLSCREEN,  VK_RETURN			    },
  { _T("Find"),		    ID_FIND					    },
  { _T("Find next"),	    ID_FINDNEXT,    VK_F3			    },
  { _T("Contents"),	    ID_BOOKMARKS				    },
  { _T("Back"),		    ID_BACK,	    VK_LEFT			    },
  { _T("Forward"),	    ID_FORWARD,	    VK_RIGHT			    },
  { _T("Exit"),		    ID_APP_EXIT					    },
  { _T("Rotate"),	    ID_ROTATE					    },
  { _T("Next bookmark/section"), ID_NEXTBM				    },
  { _T("Previous bookmark/section"), ID_PREVBM				    },
  { _T("Next section"),	    ID_NEXTCH					    },
  { _T("Previous section"), ID_PREVCH					    },
  { _T("Start autoscroll"), ID_AS_START					    },
  { _T("Stop autoscroll"),  ID_AS_STOP					    },
  { _T("Toggle autoscroll"),ID_AS_TOGGLE				    },
  { _T("Faster autoscroll"),ID_AS_FASTER				    },
  { _T("Slower autoscroll"),ID_AS_SLOWER				    },
  { _T("Faster AS (fine)"), ID_AS_FASTER_FINE				    },
  { _T("Slower AS (fine)"), ID_AS_SLOWER_FINE				    },
  { _T("Open file"),	    ID_FILE_OPEN				    },
  { _T("Next color profile"), ID_NEXT_PROFILE				    },
};
#define	NUMACTIONS    (sizeof(g_actions)/sizeof(g_actions[0]))

static struct {
  UINT	      vk;
  const TCHAR *name;
} g_keys[]={
  { VK_LEFT,	_T("Left")  },
  { VK_UP,	_T("Up")    },
  { VK_RIGHT,	_T("Right") },
  { VK_DOWN,	_T("Down")  },
  { VK_RETURN,	_T("Enter") },
  { VK_SPACE,	_T("Space") },
  { VK_PRIOR,	_T("PgUp")  },
  { VK_NEXT,	_T("PgDown")},
  { VK_HOME,	_T("Home")  },
  { VK_END,	_T("End")   },
  { VK_INSERT,	_T("Insert")  },
  { VK_DELETE,	_T("Delete")  },
  { VK_F1,	_T("F1")      },
  { VK_F2,	_T("F2")      },
  { VK_F3,	_T("F3")      },
  { VK_F4,	_T("F4")      },
  { VK_F5,	_T("F5")      },
  { VK_F6,	_T("F6")      },
  { VK_F7,	_T("F7")      },
  { VK_F8,	_T("F8")      },
  { VK_F9,	_T("F9")      },
  { VK_F10,	_T("F10")     },
  { VK_F11,	_T("F11")     },
  { VK_F12,	_T("F12")     },
  { 0x86,	_T("Action")  },
};
#define	NUMKEYS	(sizeof(g_keys)/sizeof(g_keys[0]))

#define	MAXVK	256

BOOL	    (*g_UnregisterFunc1)(UINT one,UINT two);

static bool IsMod(UINT vk) {
  return  vk == VK_LWIN || vk == VK_RWIN ||
	  vk == VK_SHIFT || vk == VK_CONTROL ||
	  vk == VK_MENU || vk == VK_LSHIFT ||
	  vk == VK_RSHIFT || vk == VK_LCONTROL ||
	  vk == VK_RCONTROL || vk == VK_LMENU ||
	  vk == VK_RMENU;
}

static void GrabKey(HWND hWnd,UINT vk) {
  if (g_UnregisterFunc1)
    g_UnregisterFunc1(MOD_WIN,vk);
  RegisterHotKey(hWnd,vk,MOD_WIN,vk);
}

static void ReleaseKey(HWND hWnd,UINT vk) {
  UnregisterHotKey(hWnd,vk);
}

static const TCHAR  *GetKeyName(UINT vk) {
  if (vk == 0)
    return _T("");
  for (int i=0;i<NUMKEYS;++i)
    if (g_keys[i].vk==vk)
      return g_keys[i].name;
  static TCHAR	  buf[32];
  if (vk>=MINHK && vk<MAXHK)
    _stprintf_s(buf,32,_T("App %d"),vk-MINHK);
  else
    _stprintf_s(buf,32,_T("Key %02x"),vk);
  return buf;
}

static CString	GetKeyNames(int i) {
  CString rv;

  if (g_actions[i].vk1 != 0)
    rv = GetKeyName(g_actions[i].vk1);
  if (g_actions[i].vk2 != 0) {
    if (!rv.IsEmpty()) rv += ',';
    rv += GetKeyName(g_actions[i].vk2);
  }
  if (g_actions[i].vk3 != 0) {
    if (!rv.IsEmpty()) rv += ',';
    rv += GetKeyName(g_actions[i].vk3);
  }

  return rv;
}

static int  GetActionId(UINT cmd) {
  for (int i=0;i<NUMACTIONS;++i)
    if (g_actions[i].cmd==cmd)
      return i;
  return NUMACTIONS-1;
}

static int  LookupKey(UINT vk) {
  for (int i = 0; i < NUMACTIONS; ++i) {
    if (g_actions[i].vk1 == vk)
      return g_actions[i].cmd;
    if (g_actions[i].vk2 == vk)
      return g_actions[i].cmd;
    if (g_actions[i].vk3 == vk)
      return g_actions[i].cmd;
  }
  return 0;
}

static bool HaveKeys(int i) {
  return g_actions[i].vk1 || g_actions[i].vk2 || g_actions[i].vk3;
}

static bool HaveSlots(int i) {
  return !g_actions[i].vk1 || !g_actions[i].vk2 || !g_actions[i].vk3;
}

static void AddKey(int i,UINT vk) {
  if (!g_actions[i].vk1) {
    g_actions[i].vk1 = vk;
    return;
  }
  if (!g_actions[i].vk2) {
    g_actions[i].vk2 = vk;
    return;
  }
  if (!g_actions[i].vk3) {
    g_actions[i].vk3 = vk;
    return;
  }
}

static HWND	g_keyowner;

void Keys::InitKeys() {
  for (int i = 0; i < NUMACTIONS; ++i) {
    CString   kn;
    kn.Format(_T("%u"),g_actions[i].cmd);
    CString   val(AfxGetApp()->GetProfileString(_T("Keys2"),kn));
    if (!val.IsEmpty())
      if (_stscanf_s(val,_T("%u,%u,%u"),&g_actions[i].vk1,&g_actions[i].vk2,&g_actions[i].vk3) != 3)
	g_actions[i].vk1 = g_actions[i].vk2 = g_actions[i].vk3 = 0;
  }

#ifdef _WIN32_WCE
  if (!g_UnregisterFunc1) {
    HMODULE hLib=GetModuleHandle(_T("coredll.dll"));
    if (hLib)
      g_UnregisterFunc1=(BOOL (*)(UINT,UINT))GetProcAddress(hLib,_T("UnregisterFunc1"));
  }
#endif
}

static void SaveKeys() {
  for (int i = 0; i < NUMACTIONS; ++i) {
    CString   kn;
    kn.Format(_T("%u"),g_actions[i].cmd);
    CString   val;
    val.Format(_T("%u,%u,%u"),g_actions[i].vk1,g_actions[i].vk2,g_actions[i].vk3);
    AfxGetApp()->WriteProfileString(_T("Keys2"),kn,val);
  }
}

static void GrabAct(HWND hWnd,int i) {
  if (g_actions[i].vk1)
    GrabKey(hWnd,g_actions[i].vk1);
  if (g_actions[i].vk2)
    GrabKey(hWnd,g_actions[i].vk2);
  if (g_actions[i].vk3)
    GrabKey(hWnd,g_actions[i].vk3);
}

static void ReleaseAct(HWND hWnd,int i) {
  if (g_actions[i].vk1)
    ReleaseKey(hWnd,g_actions[i].vk1);
  if (g_actions[i].vk2)
    ReleaseKey(hWnd,g_actions[i].vk2);
  if (g_actions[i].vk3)
    ReleaseKey(hWnd,g_actions[i].vk3);
}

static void GrabAllKeys(HWND hWnd) {
  for (int i = 0; i < NUMACTIONS; ++i)
    GrabAct(hWnd,i);
}

static void ReleaseAllKeys(HWND hWnd) {
  for (int i = 0; i < NUMACTIONS; ++i)
    ReleaseAct(hWnd,i);
}

void Keys::SetWindow(HWND hWnd) {
  if (g_keyowner)
    ReleaseAllKeys(g_keyowner);
  g_keyowner=hWnd;
  if (g_keyowner)
    GrabAllKeys(g_keyowner);
  else {
#ifdef SPI_APPBUTTONCHANGE
    // WinCE doesn't have SendMessageTimeout, so we have to post it
    ::PostMessage(HWND_BROADCAST,WM_WININICHANGE,SPI_APPBUTTONCHANGE,0);
#endif
  }
}

bool  Keys::TranslateKey(UINT vk,UINT& cmd,int angle) {
  UINT	cc = LookupKey(vk);
  if (!cc)
    return false;
  cmd = cc;
  if (angle) {
    int	  idx;
    switch (vk) {
    case VK_LEFT:
      idx=0;
      break;
    case VK_UP:
      idx=1;
      break;
    case VK_RIGHT:
      idx=2;
      break;
    case VK_DOWN:
      idx=3;
      break;
    default:
      return true;
    }
    int delta;
    switch (angle) {
    case 900:
      delta=1;
      break;
    case 1800:
      delta=2;
      break;
    case 2700:
      delta=3;
      break;
    default:
      delta=0;
    }
    idx=(idx+delta)%4;
    switch (idx) {
    case 0:
      vk=VK_LEFT;
      break;
    case 1:
      vk=VK_UP;
      break;
    case 2:
      vk=VK_RIGHT;
      break;
    case 3:
      vk=VK_DOWN;
      break;
    }
    cc = LookupKey(vk);
    if (!cc)
      return false;
    cmd=cc;
    return true;
  }
  return true;
}

class CKeysDlg : public CDialog
{
  // Construction
public:
  CKeysDlg(CWnd* pParent = NULL);   // standard constructor
  
  // Dialog Data
  //{{AFX_DATA(CKeysDlg)
  enum { IDD = IDD_KEYS };
  //}}AFX_DATA
  
  
  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CKeysDlg)
protected:
  virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
  //}}AFX_VIRTUAL
  
  // Implementation
protected:
  void	  SetButtons(int i) {
    ::EnableWindow(::GetDlgItem(m_hWnd,IDCLEAR),HaveKeys(i));
    ::EnableWindow(::GetDlgItem(m_hWnd,IDASSIGN),HaveSlots(i));
  }

  void	  SetText(int i) {
    LVITEM    ii;

    ii.mask = LVIF_TEXT;
    ii.stateMask = 0;
    ii.iItem = i;
    ii.iSubItem = 1;

    CString ks = GetKeyNames(i);

    ii.pszText = (TCHAR *)(const TCHAR *)ks;

    ::SendMessage(::GetDlgItem(m_hWnd,IDC_COMMANDS),LVM_SETITEMTEXT,i,(LPARAM)&ii);
  }

  // Generated message map functions
  //{{AFX_MSG(CKeysDlg)
  virtual BOOL OnInitDialog();
  afx_msg void OnActivate(NMHDR* pNMHDR, LRESULT* pResult);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

CKeysDlg::CKeysDlg(CWnd* pParent /*=NULL*/)
: CDialog(CKeysDlg::IDD, pParent)
{
  //{{AFX_DATA_INIT(CKeysDlg)
  //}}AFX_DATA_INIT
}

BEGIN_MESSAGE_MAP(CKeysDlg, CDialog)
//{{AFX_MSG_MAP(CKeysDlg)
ON_NOTIFY(LVN_ITEMCHANGED, IDC_COMMANDS, OnActivate)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CKeysDlg::OnInitDialog() 
{
  CDialog::OnInitDialog();

#if POCKETPC
  ((CCeCommandBar *)m_pWndEmptyCB)->LoadToolBar(cIDR_DIALOG);
#endif

#if POCKETPC
  // resize the list box
  RECT	    rcC,rcB1,rcB2,rcL;
  GetClientRect(&rcC);

  ::GetWindowRect(::GetDlgItem(m_hWnd,IDC_LABEL),&rcL); ScreenToClient(&rcL);
  ::GetWindowRect(::GetDlgItem(m_hWnd,IDASSIGN),&rcB1); ScreenToClient(&rcB1);
  ::GetWindowRect(::GetDlgItem(m_hWnd,IDCLEAR),&rcB2); ScreenToClient(&rcB2);

  int	    delta = rcC.bottom - rcB1.bottom - 4;

  if (delta > 0) {
    rcL.top += delta; rcL.bottom += delta;
    rcB1.top += delta; rcB1.bottom += delta;
    rcB2.top += delta; rcB2.bottom += delta;
    GetDlgItem(IDC_LABEL)->MoveWindow(&rcL);
    GetDlgItem(IDASSIGN)->MoveWindow(&rcB1);
    GetDlgItem(IDCLEAR)->MoveWindow(&rcB2);

    GetDlgItem(IDC_COMMANDS)->GetWindowRect(&rcL);
    ScreenToClient(&rcL);
    rcL.bottom += delta;
    rcL.left = rcC.left - 1;
    rcL.right = rcC.right + 1;
    GetDlgItem(IDC_COMMANDS)->MoveWindow(&rcL);
  }
#endif

  ::EnableWindow(::GetDlgItem(m_hWnd,IDASSIGN),FALSE);
  ::EnableWindow(::GetDlgItem(m_hWnd,IDCLEAR),FALSE);

  CWnd	  *lv = GetDlgItem(IDC_COMMANDS);

#ifdef LVS_EX_FULLROWSELECT
  lv->SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_FULLROWSELECT,LVS_EX_FULLROWSELECT);
#endif

  RECT	  rcLV;
  lv->GetClientRect(&rcLV);

  int	  vw = rcLV.right - rcLV.left - GetSystemMetrics(SM_CXVSCROLL);

  LVCOLUMN   col;
  memset(&col, 0, sizeof(col));

  col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
  col.fmt = LVCFMT_LEFT;
  col.cx = vw / 2;

  GetDlgItem(IDC_COMMANDS)->SendMessage(LVM_INSERTCOLUMN,0,(LPARAM)&col);

  col.cx = vw - col.cx;
  GetDlgItem(IDC_COMMANDS)->SendMessage(LVM_INSERTCOLUMN,1,(LPARAM)&col);

  CString   ks;

  for (int i = 0; i < NUMACTIONS; ++i) {
    LVITEM    ii;
    memset(&ii, 0, sizeof(ii));

    ii.mask = LVIF_TEXT;
    ii.stateMask = 0;
    ii.pszText = (TCHAR *)g_actions[i].name;
    ii.iItem = i;
    ii.iSubItem = 0;

    lv->SendMessage(LVM_INSERTITEM,0,(LPARAM)&ii);

    ks = GetKeyNames(i);

    ii.iSubItem = 1;
    ii.pszText = (TCHAR *)(const TCHAR *)ks;

    lv->SendMessage(LVM_SETITEMTEXT,i,(LPARAM)&ii);
  }

  return TRUE;
}

class CKeyGrabDlg : public CDialog
{
  // Construction
public:
  CKeyGrabDlg(CWnd* pParent = NULL);   // standard constructor

  UINT	      m_vk[16];

  void	Record(UINT vk) {
    SetTimer(1,50,NULL);
    for (int i = 0; i < sizeof(m_vk)/sizeof(m_vk[0]); ++i)
      if (m_vk[i] == 0) {
	m_vk[i] = vk;
	return;
      }
  }

  UINT	Get() {
    UINT    vk = 0;
    for (int i = 0; i < sizeof(m_vk)/sizeof(m_vk[0]); ++i)
      if (LOWORD(m_vk[i]) && (vk == 0 || LOWORD(m_vk[i]) < vk))
	vk = LOWORD(m_vk[i]);
    return vk;
  }

#if 0
  CString List() {
    CString s;
    for (int i = 0; i < sizeof(m_vk)/sizeof(m_vk[0]); ++i) {
      if (m_vk[i] == 0)
	break;
      CString t; t.Format(_T("0x%x"),m_vk[i]);
      if (!s.IsEmpty())
	s+=' ';
      s+=t;
    }
    return s;
  }
#endif

  enum { IDD = IDD_KEYGRAB };
  
protected:
  static LRESULT CALLBACK GrabKeyProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
  
  // Generated message map functions
  //{{AFX_MSG(CKeyGrabDlg)
  virtual BOOL OnInitDialog();
  afx_msg void OnDestroy();
  afx_msg void OnTimer(UINT nIDEvent);
  //}}AFX_MSG
  afx_msg LRESULT OnHotkey(WPARAM,LPARAM);
  afx_msg LRESULT OnApp(WPARAM,LPARAM);
  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CKeyGrabDlg dialog

CKeyGrabDlg::CKeyGrabDlg(CWnd* pParent /*=NULL*/)
: CDialog(CKeyGrabDlg::IDD, pParent)
{
  memset(m_vk,0,sizeof(m_vk));
}

BEGIN_MESSAGE_MAP(CKeyGrabDlg, CDialog)
//{{AFX_MSG_MAP(CKeyGrabDlg)
ON_WM_DESTROY()
ON_WM_TIMER()
//}}AFX_MSG_MAP
ON_MESSAGE(WM_HOTKEY, OnHotkey)
ON_MESSAGE(WM_APP, OnApp)
#if POCKETPC
ON_WM_SETTINGCHANGE()
#endif
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKeyGrabDlg message handlers

void CKeyGrabDlg::OnTimer(UINT nIDEvent) 
{
  for (int i = 0; i < sizeof(m_vk)/sizeof(m_vk[0]); ++i)
    if (LOWORD(m_vk[i]) && (GetAsyncKeyState(LOWORD(m_vk[i])) & 0x8000)) {
      SetTimer(1,50,NULL);
      return;
    }
  EndDialog(IDOK);
}

BOOL CKeyGrabDlg::OnInitDialog() {
  CDialog::OnInitDialog();

#if POCKETPC
  ((CCeCommandBar *)m_pWndEmptyCB)->LoadToolBar(cIDR_DIALOG);
#endif

  RECT	rcC;
  GetClientRect(&rcC);

  HWND hWnd = ::CreateWindow(_T("Static"),_T("Press any key..."),WS_VISIBLE|WS_CHILD|SS_CENTER,
    rcC.left,rcC.top + (rcC.bottom-rcC.top)/2 - HIWORD(GetDialogBaseUnits()),
    rcC.right-rcC.left,HIWORD(GetDialogBaseUnits()*2),m_hWnd,
    (HMENU)47793,AfxGetInstanceHandle(),NULL);

  // release all keys
  if (g_keyowner)
    ReleaseAllKeys(g_keyowner);

  // and grab all
  for (int i=MINHK;i<=MAXHK;++i)
    GrabKey(m_hWnd,i);

  SetWindowLong(m_hWnd,GWL_USERDATA,GetWindowLong(m_hWnd,GWL_WNDPROC));
  SetWindowLong(m_hWnd,GWL_WNDPROC,(LONG)GrabKeyProc);

  SetWindowLong(hWnd,GWL_USERDATA,GetWindowLong(hWnd,GWL_WNDPROC));
  SetWindowLong(hWnd,GWL_WNDPROC,(LONG)GrabKeyProc);

  SetFocus();

  return TRUE;
}

void CKeyGrabDlg::OnDestroy() {
  // release keys
  for (int i=MINHK;i<=MAXHK;++i)
    ReleaseKey(m_hWnd,i);
  // and regrab
  if (g_keyowner)
    GrabAllKeys(g_keyowner);
  CDialog::OnDestroy();
}

LRESULT CKeyGrabDlg::OnHotkey(WPARAM wp,LPARAM lp) {
  Record(MAKELONG(HIWORD(lp),LOWORD(lp)));
  return 0;
}

LRESULT CKeyGrabDlg::OnApp(WPARAM wp,LPARAM lp) {
  if (wp == 0x86)
    Record(13);
  return 0;
}

LRESULT CALLBACK CKeyGrabDlg::GrabKeyProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam) {
  WNDPROC wp = (WNDPROC)GetWindowLong(hWnd,GWL_USERDATA);

  HWND	  mWnd = hWnd;

  if (::GetDlgCtrlID(hWnd) == 47793)
    mWnd = ::GetParent(hWnd);

  if (mWnd == NULL)
    return CallWindowProc(wp,hWnd,uMsg,wParam,lParam);

  if (uMsg == WM_KEYDOWN) {
    if (!IsMod(wParam) && !(lParam & 0x40000000))
      ::PostMessage(mWnd,WM_APP,wParam,0);
  }

  if (uMsg == WM_KEYUP) {
    if (!IsMod(wParam))
      ::PostMessage(mWnd,WM_HOTKEY,wParam,MAKELONG(0,wParam));
    return 0;
  }

  if (uMsg == WM_DESTROY)
    SetWindowLong(hWnd,GWL_WNDPROC,GetWindowLong(hWnd,GWL_USERDATA));

  return CallWindowProc(wp,hWnd,uMsg,wParam,lParam);
}

void Keys::SetupKeys(CWnd *parent) {
  CKeysDlg    dlg;
  if (dlg.DoModal()==IDOK)
    SaveKeys();
  else {
    HWND  owner=g_keyowner;
    Keys::SetWindow(0);
    Keys::InitKeys();
    Keys::SetWindow(owner);
  }
}

BOOL CKeysDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
  int sel = ::SendMessage(::GetDlgItem(m_hWnd,IDC_COMMANDS),LVM_GETSELECTIONMARK,0,0);

  switch (LOWORD(wParam)) {
    case IDASSIGN:
      if (sel >= 0 && sel < NUMACTIONS && HaveSlots(sel)) {
	CKeyGrabDlg dlg(this);
	dlg.DoModal();
	if (dlg.Get()) {
	  UINT  vk = dlg.Get();
	  for (int j = 0; j < NUMACTIONS; ++j) {
	    if (g_actions[j].vk1 == vk) {
	      g_actions[j].vk1 = 0;
	      SetText(j);
	    }
	    if (g_actions[j].vk2 == vk) {
	      g_actions[j].vk2 = 0;
	      SetText(j);
	    }
	    if (g_actions[j].vk3 == vk) {
	      g_actions[j].vk3 = 0;
	      SetText(j);
	    }
	  }

	  if (g_keyowner)
	    ReleaseAct(g_keyowner,sel);

	  AddKey(sel,vk);

	  if (g_keyowner)
	    GrabAct(g_keyowner,sel);

	  SetButtons(sel);
	  SetText(sel);
	}
      }
      return TRUE;
    case IDCLEAR:
      if (sel >= 0 && sel < NUMACTIONS) {
	if (g_keyowner)
	  ReleaseAct(g_keyowner,sel);
	g_actions[sel].vk1 = g_actions[sel].vk2 = g_actions[sel].vk3 = 0;
	SetButtons(sel);
	SetText(sel);
      }
      return TRUE;
  }

  return CDialog::OnCommand(wParam, lParam);
}

void CKeysDlg::OnActivate(NMHDR* pNMHDR, LRESULT* pResult) 
{
  NMLISTVIEW  *nv = (NMLISTVIEW *)pNMHDR;

  if (nv->iItem >= 0 && (nv->uChanged & LVIF_STATE) && 
      (nv->uNewState & LVIS_SELECTED))
  {
    SetButtons(nv->iItem);
  }
#if 0
  int sel = ::SendMessage(::GetDlgItem(m_hWnd,IDC_COMMANDS),LVM_GETSELECTIONMARK,0,0);

  if (sel >= 0 && sel < NUMACTIONS)
    SetButtons(sel);
#endif

  *pResult = 0;
}
