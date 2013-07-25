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
* $Id: ColorSelector.cpp,v 1.10.2.3 2004/10/29 15:18:46 mike Exp $
*
*/

#define _WIN32_WINNT	_WIN32_WINNT_MAXVER

#include <afxext.h>

#include "resource.h"
#include "Colors.h"
#include "ColorSelector.h"
#include "config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE [] = __FILE__;
#endif

#define	GAMMA_BASE  10000

/////////////////////////////////////////////////////////////////////////////
// CColorSelector dialog


CColorSelector::CColorSelector(ColorItem *colors, CWnd* pParent /*=NULL*/)
	: CDialog(CColorSelector::IDD, pParent), m_colors(colors), m_index(0)
{
	//{{AFX_DATA_INIT(CColorSelector)
	// NOTE: the ClassWizard will add member initialization here
	m_gamma = 0;
	//}}AFX_DATA_INIT
}


void CColorSelector::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CColorSelector)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Scroll(pDX, IDC_GAMMA, m_gamma);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CColorSelector, CDialog)
	//{{AFX_MSG_MAP(CColorSelector)
	ON_WM_DRAWITEM()
	ON_WM_HSCROLL()
	ON_CBN_SELENDOK(IDC_COLORS, OnSelendokColors)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorSelector message handlers

void CColorSelector::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT ds)
{
	if (nIDCtl == IDC_SAMPLE) {
		SetBkColor(ds->hDC, m_colors[m_index].tempval);
		ExtTextOut(ds->hDC, ds->rcItem.left, ds->rcItem.top, ETO_OPAQUE | ETO_CLIPPED,
			&ds->rcItem, _T(""), 0, NULL);
	}
	else
		CDialog::OnDrawItem(nIDCtl, ds);
}

void CColorSelector::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO  info;
	pScrollBar->GetScrollInfo(&info, SIF_ALL);

	nPos &= 0xffff;

	int	page = (info.nMax - info.nMin + 1) / 15;

	// Determine the new position of scroll box.
	switch (nSBCode)
	{
	case SB_LEFT:    // Scroll to far left.
		info.nPos = info.nMin;
		break;

	case SB_RIGHT:    // Scroll to far right.
		info.nPos = info.nMax;
		break;

	case SB_ENDSCROLL:  // End scroll.
		break;

	case SB_LINELEFT:    // Scroll left.
		info.nPos--;
		break;

	case SB_LINERIGHT:  // Scroll right.
		info.nPos++;
		break;

	case SB_PAGELEFT:   // Scroll one page left.
		info.nPos -= page;
		break;

	case SB_PAGERIGHT:    // Scroll one page right.
		info.nPos += page;
		break;

	case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
		info.nPos = nPos;      // of the scroll box at the end of the drag operation.
		break;

	case SB_THUMBTRACK:   // Drag scroll box to specified position. nPos is the
		info.nPos = nPos;     // position that the scroll box has been dragged to.
		break;
	}

	if (info.nPos < info.nMin)
		info.nPos = info.nMin;
	if (info.nPos > (int) info.nMax)
		info.nPos = info.nMax;

	// Set the new position of the thumb (scroll box).
	if (nSBCode != SB_THUMBTRACK)
		pScrollBar->SetScrollPos(info.nPos);

	int	id = pScrollBar->GetDlgCtrlID();
	if (id == IDC_GAMMA) {
		CString   tmp;
		tmp.Format(_T("%1.2f"), (info.nPos + GAMMA_BASE) / 10000.0);
		SetDlgItemText(IDC_TXTGAMMA, tmp);
	}
	else {
		int	r = ((CScrollBar*) GetDlgItem(IDC_RED))->GetScrollPos();
		int	g = ((CScrollBar*) GetDlgItem(IDC_GREEN))->GetScrollPos();
		int	b = ((CScrollBar*) GetDlgItem(IDC_BLUE))->GetScrollPos();
		switch (id) {
		case IDC_RED:
			r = info.nPos;
			break;
		case IDC_GREEN:
			g = info.nPos;
			break;
		case IDC_BLUE:
			b = info.nPos;
			break;
		}
		if (m_colors[m_index].tempval != RGB(r, g, b)) {
			SetDlgItemInt(IDC_SRED, r);
			SetDlgItemInt(IDC_SGREEN, g);
			SetDlgItemInt(IDC_SBLUE, b);
			m_colors[m_index].tempval = RGB(r, g, b);
			GetDlgItem(IDC_SAMPLE)->Invalidate(FALSE);
		}
	}
}

BOOL CColorSelector::OnInitDialog()
{
	CDialog::OnInitDialog();

#if POCKETPC
	((CCeCommandBar *) m_pWndEmptyCB)->LoadToolBar(cIDR_DIALOG);
#endif

	// init combo box
	CComboBox   *box = (CComboBox*) GetDlgItem(IDC_COLORS);
	if (box) {
		for (int i = 0; m_colors[i].name; ++i)
			box->AddString(m_colors[i].name);
		box->SetCurSel(0);
	}
	// init scrollbar ranges
	SCROLLINFO  si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = 255;
	si.nPage = 0;
	//si.nMax+=si.nPage-1;
	// red
	si.nPos = si.nTrackPos = GetRValue(m_colors[m_index].tempval);
	((CScrollBar*) GetDlgItem(IDC_RED))->SetScrollInfo(&si);
	// green
	si.nPos = si.nTrackPos = GetGValue(m_colors[m_index].tempval);
	((CScrollBar*) GetDlgItem(IDC_GREEN))->SetScrollInfo(&si);
	// blue
	si.nPos = si.nTrackPos = GetBValue(m_colors[m_index].tempval);
	((CScrollBar*) GetDlgItem(IDC_BLUE))->SetScrollInfo(&si);
	si.nMin = 0;
	si.nMax = 40000;
	si.nPage = 0;
	si.nPos = si.nTrackPos = m_gamma;
	((CScrollBar*) GetDlgItem(IDC_GAMMA))->SetScrollInfo(&si);
	// set text
	SetDlgItemInt(IDC_SRED, ((CScrollBar*) GetDlgItem(IDC_RED))->GetScrollPos());
	SetDlgItemInt(IDC_SGREEN, ((CScrollBar*) GetDlgItem(IDC_GREEN))->GetScrollPos());
	SetDlgItemInt(IDC_SBLUE, ((CScrollBar*) GetDlgItem(IDC_BLUE))->GetScrollPos());
	CString   tmp;
	tmp.Format(_T("%1.2f"), (m_gamma + GAMMA_BASE) / 10000.0);
	SetDlgItemText(IDC_TXTGAMMA, tmp);
	return TRUE;
}

bool	myChooseColors(ColorItem *colors, int *gamma, CWnd *parent) {
	CColorSelector    dlg(colors, parent);
	// fill in temporary values
	for (int i = 0; colors[i].name; ++i)
		colors[i].tempval = colors[i].value;
	dlg.m_gamma = *gamma / 100 - GAMMA_BASE;
	if (dlg.DoModal() == IDOK) {
		bool changed = false;
		for (int j = 0; colors[j].name; ++j)
			if (colors[j].value != colors[j].tempval) {
				colors[j].value = colors[j].tempval;
				colors[j].tempval = 1;
				changed = true;
			}
			dlg.m_gamma += GAMMA_BASE;
			if (dlg.m_gamma * 100 != *gamma) {
				*gamma = dlg.m_gamma * 100;
				changed = true;
			}
			return changed;
	}
	return false;
}

void CColorSelector::OnSelendokColors() {
	int newidx = ((CComboBox*) GetDlgItem(IDC_COLORS))->GetCurSel();
	if (newidx == LB_ERR || newidx == m_index)
		return;
	m_index = newidx;
	((CScrollBar*) GetDlgItem(IDC_RED))->SetScrollPos(GetRValue(m_colors[m_index].tempval));
	((CScrollBar*) GetDlgItem(IDC_GREEN))->SetScrollPos(GetGValue(m_colors[m_index].tempval));
	((CScrollBar*) GetDlgItem(IDC_BLUE))->SetScrollPos(GetBValue(m_colors[m_index].tempval));
	// set text
	SetDlgItemInt(IDC_SRED, ((CScrollBar*) GetDlgItem(IDC_RED))->GetScrollPos());
	SetDlgItemInt(IDC_SGREEN, ((CScrollBar*) GetDlgItem(IDC_GREEN))->GetScrollPos());
	SetDlgItemInt(IDC_SBLUE, ((CScrollBar*) GetDlgItem(IDC_BLUE))->GetScrollPos());
	GetDlgItem(IDC_SAMPLE)->Invalidate(FALSE);
}
