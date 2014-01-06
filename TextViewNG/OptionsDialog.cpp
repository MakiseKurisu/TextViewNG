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
* $Id: OptionsDialog.cpp,v 1.16.2.4 2004/10/21 15:35:24 mike Exp $
*
*/

#pragma warning(disable:4100)
#define _WIN32_WINNT _WIN32_WINNT_MAXVER

#include <afxext.h>

#include "config.h"
#include "resource.h"
#include "OptionsDialog.h"
#include "Colors.h"
#include "ColorSelector.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsDialog dialog


COptionsDialog::COptionsDialog(CWnd* pParent /*=NULL*/)
: CDialog(COptionsDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(COptionsDialog)
    m_bold = FALSE;
    m_cleartype = 0;
    m_face = _T("");
    m_size = 0;
    m_justify = FALSE;
    m_margins = 0;
    m_hyphenate = FALSE;
    m_angle = 0;
    m_columns = 0;
    //}}AFX_DATA_INIT
}


void COptionsDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(COptionsDialog)
    DDX_Check(pDX, IDC_BOLD, m_bold);
    DDX_CBString(pDX, IDC_FACE, m_face);
    DDX_Text(pDX, IDC_FONTSIZE, m_size);
    DDX_Check(pDX, IDC_JUSTIFY, m_justify);
    DDX_Text(pDX, IDC_MARGINS, m_margins);
    DDX_Check(pDX, IDC_SHYP, m_hyphenate);
    DDX_CBIndex(pDX, IDC_ROTATE, m_angle);
    DDX_CBIndex(pDX, IDC_COLUMNS, m_columns);
    DDX_CBIndex(pDX, IDC_CLEARTYPE, m_cleartype);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsDialog, CDialog)
    //{{AFX_MSG_MAP(COptionsDialog)
    ON_WM_DRAWITEM()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsDialog message handlers

void COptionsDialog::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT ds)
{
    if (nIDCtl == IDC_TEXTSAMPLE)
    {
        LOGFONT   lf;

        UpdateData();
        memset(&lf, 0, sizeof(lf));
        lf.lfHeight = -m_size;
        lf.lfWeight = m_bold ? FW_BOLD : FW_NORMAL;
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
        lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        switch (m_cleartype)
        {
        case 1:
            lf.lfQuality = CLEARTYPE_QUALITY;
            break;
        case 2:
            lf.lfQuality = CLEARTYPE_NATURAL_QUALITY;
            break;
        case 3:
            lf.lfQuality = ANTIALIASED_QUALITY;
            break;
        default:
            lf.lfQuality = NONANTIALIASED_QUALITY;
        }
        lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
        _tcsncpy_s(lf.lfFaceName, LF_FACESIZE, m_face, LF_FACESIZE - 1);
        CFont   font;
        font.CreateFontIndirect(&lf);
        HGDIOBJ prev = SelectObject(ds->hDC, font);
        SetBkColor(ds->hDC, C_BG);
        SetTextColor(ds->hDC, C_NORM);
        SIZE sz1, sz2;
        TCHAR *s1 = _T("Sample");
        TCHAR *s2 = _T("Text");
        int len1 = _tcslen(s1);
        int len2 = _tcslen(s2);
        GetTextExtentPoint(ds->hDC, s1, len1, &sz1);
        GetTextExtentPoint(ds->hDC, s2, len2, &sz2);
        int ox = (ds->rcItem.right - ds->rcItem.left - sz1.cx - sz2.cx - 5) / 2;
        int oy = (ds->rcItem.bottom - ds->rcItem.top - max(sz1.cy, sz2.cy)) / 2;
        ExtTextOut(ds->hDC, ds->rcItem.left + ox, ds->rcItem.top + oy, ETO_CLIPPED | ETO_OPAQUE, &ds->rcItem, s1, len1, NULL);
        SetBkColor(ds->hDC, C_HBG);
        RECT r;
        r.top = ds->rcItem.top + oy;
        r.left = ds->rcItem.left + ox + sz1.cx + 5;
        r.bottom = r.top + sz2.cy;
        r.right = r.left + sz2.cx;
        ExtTextOut(ds->hDC, r.left, r.top, ETO_CLIPPED | ETO_OPAQUE, &r, s2, len2, NULL);
        r.bottom = ds->rcItem.bottom;
        r.left = ds->rcItem.left;
        r.top = r.bottom - 2;
        r.right = ds->rcItem.right;
        SetBkColor(ds->hDC, C_GAUGE);
        ExtTextOut(ds->hDC, r.left, r.top, ETO_CLIPPED | ETO_OPAQUE, &r, _T(""), 0, NULL);
        SelectObject(ds->hDC, prev);
    }
    else
        CDialog::OnDrawItem(nIDCtl, ds);
}

BOOL COptionsDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    bool invalidate = false;
    switch (LOWORD(wParam))
    {
    case IDC_FACE:
    case IDC_CLEARTYPE:
        if (HIWORD(wParam) == CBN_SELENDOK)
            invalidate = true;
        break;
    case IDC_FONTSIZE:
        if (HIWORD(wParam) == EN_CHANGE)
            invalidate = true;
        break;
    case IDC_BOLD:
        if (HIWORD(wParam) == BN_CLICKED)
            invalidate = true;
        break;
    }
    if (invalidate)
    {
        CWnd *tf = GetDlgItem(IDC_TEXTSAMPLE);
        if (tf)
        {
            tf->Invalidate(FALSE);
        }
    }
    return CDialog::OnCommand(wParam, lParam);
}

static int CALLBACK EnumFontFamProc(
    _In_    ENUMLOGFONT *lpelf,
    _In_    NEWTEXTMETRIC *lpntm,
    _In_    DWORD FontType,
    _In_    LPARAM lParam
    )
{
    LRESULT ret = ::SendMessage((HWND)lParam, CB_ADDSTRING, 0, (LPARAM)lpelf->elfLogFont.lfFaceName);
    ASSERT(ret != CB_ERR && ret != CB_ERRSPACE);
    return TRUE;
}

BOOL COptionsDialog::OnInitDialog()
{
    // populate font families
    CClientDC dc(this);
    HWND hCB;
    GetDlgItem(IDC_FACE, &hCB);
    EnumFontFamilies(dc.m_hAttribDC, NULL, (FONTENUMPROC)EnumFontFamProc, (LPARAM)hCB);

    // add angles and columns
    for (int angle = 0; angle < 360; angle += 90)
    {
        CString tmp;
        tmp.Format(_T("%d\xb0"), angle);
        SendDlgItemMessage(IDC_ROTATE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)tmp);
    }
    for (int column = 1; column < 5; ++column)
    {
        CString tmp;
        tmp.Format(_T("%d"), column);
        SendDlgItemMessage(IDC_COLUMNS, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)tmp);
    }

    SendDlgItemMessage(IDC_CLEARTYPE, CB_ADDSTRING, 0, (LPARAM)_T("None"));
    SendDlgItemMessage(IDC_CLEARTYPE, CB_ADDSTRING, 0, (LPARAM)_T("ClearType"));
    SendDlgItemMessage(IDC_CLEARTYPE, CB_ADDSTRING, 0, (LPARAM)_T("Natural"));
    SendDlgItemMessage(IDC_CLEARTYPE, CB_ADDSTRING, 0, (LPARAM)_T("Standard"));

    // resize the input window
    RECT   rface, rsize, rspin;
    GetDlgItem(IDC_FACE)->GetWindowRect(&rface);
    GetDlgItem(IDC_FONTSIZE)->GetWindowRect(&rsize);
    GetDlgItem(IDC_SIZESPIN)->GetWindowRect(&rspin);
    ScreenToClient(&rface);
    ScreenToClient(&rsize);
    ScreenToClient(&rspin);
    rsize.bottom = rspin.bottom = rface.bottom;
    GetDlgItem(IDC_FONTSIZE)->MoveWindow(&rsize);
    GetDlgItem(IDC_SIZESPIN)->MoveWindow(&rspin);
    CDialog::OnInitDialog();
    SendDlgItemMessage(IDC_SIZESPIN, UDM_SETRANGE, 0, MAKELONG(50, 1));
    SendDlgItemMessage(IDC_MARGINSPIN, UDM_SETRANGE, 0, MAKELONG(50, 0));
    return TRUE;
}
