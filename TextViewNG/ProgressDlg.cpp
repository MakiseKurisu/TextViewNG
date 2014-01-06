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
* $Id: ProgressDlg.cpp,v 1.16.2.1 2003/04/12 22:52:33 mike Exp $
*
*/

#define _WIN32_WINNT _WIN32_WINNT_MAXVER

#include <afxwin.h>
#include <afxtempl.h>

#include "resource.h"

#include "ptr.h"
#include "ProgressDlg.h"
#include "config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SHOWDELAY 700
#define MINUPDATE 500

/////////////////////////////////////////////////////////////////////////////
// CProgressDlg dialog


CProgressDlg::CProgressDlg(const CString& msg, CWnd* pParent /*=NULL*/)
: CDialog(CProgressDlg::IDD, pParent), m_msg(msg), m_last(0), m_tail(0), m_nsamp(0),
m_curbytes(0), m_lastbytes(0), m_lastupdate(0)
{
    Create(CProgressDlg::IDD, pParent);
    CenterWindow();
    m_starttime = GetTickCount();
    m_visible = false;
    //{{AFX_DATA_INIT(CProgressDlg)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}


void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CProgressDlg)
    // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}

void CProgressDlg::SetMax(DWORD max) {
    SendDlgItemMessage(IDC_PROGRESS, PBM_SETRANGE32, 0, max);
    m_starttime = GetTickCount();
}

void CProgressDlg::SetCur(DWORD cur) {
    if (cur > m_last) {
        m_last = (cur + 65535)&~65535;
        SendDlgItemMessage(IDC_PROGRESS, PBM_SETPOS, cur);
        DWORD   tm = GetTickCount();
        DWORD   delta = cur - m_lastbytes;
        m_lastbytes = cur;
        // delete older samples
        while (m_nsamp > 0 && (tm - m_samples[m_tail << 1] > 1000 || m_nsamp == MAXSAMP)) {
            m_curbytes -= m_samples[(m_tail << 1) + 1];
            if (++m_tail >= MAXSAMP)
                m_tail = 0;
            --m_nsamp;
        }
        // append last sample
        DWORD   pos = m_tail + m_nsamp;
        if (pos >= MAXSAMP)
            pos -= MAXSAMP;
        m_samples[pos << 1] = tm;
        m_samples[(pos << 1) + 1] = delta;
        m_curbytes += delta;
        ++m_nsamp;
        if (!m_visible && tm - m_starttime > SHOWDELAY) {
            m_visible = true;
            ShowWindow(SW_SHOW);
            UpdateWindow();
        }
        if (m_visible && tm - m_lastupdate > MINUPDATE) {
            // calculate speed
            DWORD   timedelta = tm - m_samples[m_tail << 1];
            DWORD   speed = 0;
            if (timedelta > 0)
                speed = (DWORD)(((__int64)m_curbytes * 1000 / timedelta) >> 10);
            TCHAR   buf[128];
            _sntprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%u KB\t%u KB/s"), cur >> 10, speed);
            SetDlgItemText(IDC_PROGRESSTEXT, buf);
            GetDlgItem(IDC_PROGRESSTEXT)->UpdateWindow();
            m_lastupdate = tm;
        }
    }
}

BEGIN_MESSAGE_MAP(CProgressDlg, CDialog)
    //{{AFX_MSG_MAP(CProgressDlg)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProgressDlg message handlers

BOOL CProgressDlg::OnInitDialog() {
    CDialog::OnInitDialog();

    CString   msg;
    msg.Format(_T("Loading %s..."), (LPCTSTR)m_msg);
    SetWindowText(msg);

    return TRUE;
}