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
* $Id: ColorSelector.h,v 1.9.2.1 2003/04/12 22:52:32 mike Exp $
*
*/

#if !defined(AFX_COLORSELECTOR_H__751AE4B8_187D_431C_9899_56D3CCF1590C__INCLUDED_)
#define AFX_COLORSELECTOR_H__751AE4B8_187D_431C_9899_56D3CCF1590C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// ColorSelector.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CColorSelector dialog

class CColorSelector : public CDialog
{
	// Construction
public:
	CColorSelector(ColorItem *colors, CWnd* pParent = NULL);   // standard constructor

	// Dialog Data
	//{{AFX_DATA(CColorSelector)
	enum { IDD = IDD_COLORSEL };
	// NOTE: the ClassWizard will add data members here
	int	m_gamma;
	//}}AFX_DATA


	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorSelector)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:
	int	    m_index;
	ColorItem *m_colors;

	// Generated message map functions
	//{{AFX_MSG(CColorSelector)
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnInitDialog();
	afx_msg void OnSelendokColors();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

bool	myChooseColors(ColorItem *colors, int *gamma, CWnd *parent = NULL);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORSELECTOR_H__751AE4B8_187D_431C_9899_56D3CCF1590C__INCLUDED_)
