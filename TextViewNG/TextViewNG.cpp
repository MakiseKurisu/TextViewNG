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
* $Id: TextViewNG.cpp,v 1.40.2.6 2003/10/12 15:10:33 mike Exp $
*
*/

#pragma warning(disable:4100)
#define _WIN32_WINNT	_WIN32_WINNT_MAXVER

#include <afxcmn.h>
#include <stdarg.h>
#include <afxext.h>

#include "resource.h"

#include "config.h"
#include "ptr.h"
#include "RFile.h"
#include "TextViewNG.h"
#include "TVFrame.h"
#include "Keys.h"
#include "Unicode.h"
#include "XListBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE [] = __FILE__;
#endif

// ugly hacks
#ifndef WS_OVERLAPPEDWINDOW
#define WS_OVERLAPPEDWINDOW 0
#endif

#define	IPCCLASS  _T("TextViewNGIPC")
#define	IPCLEN	  (sizeof(IPCCLASS)/sizeof(TCHAR))

#define	REGNAME	_T("TextViewNG")

/////////////////////////////////////////////////////////////////////////////
// CTVApp

BEGIN_MESSAGE_MAP(CTVApp, CWinApp)
	//{{AFX_MSG_MAP(CTVApp)
	//}}AFX_MSG_MAP
	// Standard file based document commands
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTVApp construction

CTVApp::CTVApp()
	: CWinApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CTVApp object

static CTVApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CTVApp initialization
BOOL CTVApp::InitInstance()
{
	// init XListBox control
	XLB_Init();

	SetRegistryKey(REGNAME);

	RFile::InitBufSize();
	Keys::InitKeys();

	CTVFrame* pFrame = new CTVFrame();
	m_pMainWnd = pFrame;

	pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, NULL);
#ifndef _WIN32_WCE
	pFrame->ModifyStyleEx(WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE, 0);
	pFrame->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
#endif

	pFrame->SetWindowText(_T("TextViewNG"));
	pFrame->InitWindowPos(m_nCmdShow);
	pFrame->UpdateWindow();

	// create a dummy window
	WNDCLASS    wc;
	memset(&wc, 0, sizeof(wc));
	wc.hInstance = AfxGetInstanceHandle();
	wc.lpszClassName = IPCCLASS;
	wc.lpfnWndProc = DefWindowProc;
	RegisterClass(&wc);
	TCHAR	szExeName[MAX_PATH] = { '\0' };
	::GetModuleFileName(NULL, szExeName, MAX_PATH);
	CreateWindow(IPCCLASS, szExeName, WS_POPUP, 0, 0, 1, 1, pFrame->m_hWnd, NULL, wc.hInstance, NULL);

	pFrame->PostMessage(WM_COMMAND, ID_INIT);

	return TRUE;
}

void CTVApp::Barf(const TCHAR *format, ...) {
	TCHAR	    buffer[2048];
	va_list   ap;

	va_start(ap, format);
	_vsntprintf_s(buffer, sizeof(buffer) / sizeof(buffer[0]), _TRUNCATE, format, ap);
	va_end(ap);
	CWnd	  *wnd = AfxGetMainWnd();
	if (wnd)
		wnd->MessageBox(buffer, _T("Error"), MB_OK | MB_ICONERROR);
	else
		::MessageBox(NULL, buffer, _T("Error"), MB_OK | MB_ICONERROR);
}

void CTVApp::QueueCmd(UINT cmd) {
	theApp.m_cmdqueue.Add(cmd);
}

UINT CTVApp::TopQueuedCmd() {
	if (theApp.m_cmdqueue.GetSize() > 0)
		return theApp.m_cmdqueue[theApp.m_cmdqueue.GetSize() - 1];
	return 0;
}


BOOL CTVApp::OnIdle(LONG lCount)
{
	// force processing queued messages even before the framework
	if (lCount == 0 && m_cmdqueue.GetSize() > 0) {
		m_pMainWnd->PostMessage(WM_COMMAND, m_cmdqueue[0], 0);
		m_cmdqueue.RemoveAt(0);
	}
	return CWinApp::OnIdle(lCount);
}

CImageList* CTVApp::ImageList() {
	if (!theApp.m_imagelist.get()) {
		theApp.m_imagelist.reset(new CImageList());
		theApp.m_imagelist->Create(IDB_SMALLICONS, 16, 0, RGB(255, 0, 255));
	}
	return theApp.m_imagelist.get();
}

static bool  FindOtherWindow(CWinThread* pThread, LPCTSTR cmdline) {
	// WinCE: Only one application instance can be run
	HANDLE  hMuTex;
	TCHAR	szTempName[MAX_PATH] = { '\0' }, szExeName[MAX_PATH] = { '\0' };
	int	  i, iValue;

	iValue = ::GetModuleFileName(NULL, szExeName, MAX_PATH);
	HKEY	hKey;
	CString kname(_T("Software\\") REGNAME _T("\\"));
	TCHAR	  *exp = szExeName + iValue;
	while (exp > szExeName && exp[-1] != _T('/') && exp[-1] != _T('\\'))
		--exp;
	kname += exp;
	if (kname.Right(4).CompareNoCase(_T(".exe")) == 0)
		kname = kname.Left(kname.GetLength() - 4);
	kname += _T("\\Parameters");
	bool	allow = false;
	if (::RegOpenKeyEx(HKEY_CURRENT_USER, kname, 0, HR_REG_PERM, &hKey) == ERROR_SUCCESS) {
		DWORD   type;
		DWORD   val = 0;
		DWORD   sizeval = sizeof(val);
		if (::RegQueryValueEx(hKey, _T("AllowOnlyOneInstance"), 0, &type, (BYTE*) &val, &sizeval) == ERROR_SUCCESS &&
			type == REG_DWORD && sizeval == sizeof(val))
			allow = val == 0;
		RegCloseKey(hKey);
	}
	if (allow)
		return false;
	_tcscpy_s(szTempName, MAX_PATH, szExeName);
	for (i = 0; i < iValue; i++)
	{
		if (szTempName[i] == '\\')
			szTempName[i] = '/';
	}

	hMuTex = ::CreateMutex(NULL, FALSE, szTempName);
	if (hMuTex != NULL)
	{
		if (::GetLastError() == ERROR_ALREADY_EXISTS)
		{
			HWND  hWnd = ::FindWindow(IPCCLASS, szExeName);
			HWND  hFrame = hWnd ? ::GetParent(hWnd) : NULL;

			if (hFrame)
			{
				if (cmdline && cmdline[0]) {
					COPYDATASTRUCT	  cd;
					cd.dwData = 0;
					// force the filename to unicode
					cd.cbData = wcslen(cmdline)*sizeof(wchar_t) ;
					cd.lpData = (void *) cmdline;
					::SendMessage(hFrame, WM_COPYDATA, 0, (LPARAM)&cd);
				}
				::SetForegroundWindow(hFrame);
				return true;
			}
		}
	}
	return false;
}

int AFXAPI AfxWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
#if defined(_DEBUG) && defined(_CRT_WARN)
	int	dbgflag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	dbgflag |= _CRTDBG_ALLOC_MEM_DF | _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(dbgflag);
	HANDLE	  hReport = CreateFile(_T("crtreport.txt"), GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, hReport);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, hReport);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, hReport);
#endif
	ASSERT(hPrevInstance == NULL);

	AfxGetModuleState()->m_bDLL = FALSE; // XXX

	int nReturnCode = -1;
	CWinThread* pThread = AfxGetThread();
	CWinApp* pApp = AfxGetApp();

	// AFX internal initialization
	if (!AfxWinInit(hInstance, hPrevInstance, lpCmdLine, nCmdShow))
		goto InitFailure;

	// App global initializations (rare)
	if (pApp != NULL && !pApp->InitApplication())
		goto InitFailure;

	if (FindOtherWindow(pThread, lpCmdLine))
		goto InitFailure;

	// Perform specific initializations
	if (!pThread->InitInstance())
	{
		if (pThread->m_pMainWnd != NULL)
		{
			TRACE0("Warning: Destroying non-NULL m_pMainWnd\n");
			pThread->m_pMainWnd->DestroyWindow();
		}
		nReturnCode = pThread->ExitInstance();
		goto InitFailure;
	}
	nReturnCode = pThread->Run();

InitFailure:
#ifdef _DEBUG
	// Check for missing AfxLockTempMap calls
	if (AfxGetModuleThreadState()->m_nTempMapLock != 0)
	{
		TRACE1("Warning: Temp map lock count non-zero (%ld).\n",
			AfxGetModuleThreadState()->m_nTempMapLock);
	}
	AfxLockTempMaps();
	AfxUnlockTempMaps(-1);
#endif

	AfxWinTerm();
	return nReturnCode;
}

extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	return AfxWinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}