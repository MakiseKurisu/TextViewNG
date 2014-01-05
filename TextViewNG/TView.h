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
* $Id: TView.h,v 1.67.2.28 2007/03/26 19:26:52 mike Exp $
*
*/

#if !defined(AFX_TVIEW_H__BF90ABF1_7A00_4E3D_8E68_A4B0C27E2A62__INCLUDED_)
#define AFX_TVIEW_H__BF90ABF1_7A00_4E3D_8E68_A4B0C27E2A62__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CTView window

class CTView : public CWnd, public CRotate
{
    // Construction
public:
    CTView();
    void SetFile(auto_ptr<TextFile> tfile);

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CTView)
public:
    virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    //}}AFX_VIRTUAL

    // Implementation
public:
    virtual ~CTView();
    void	  Init();

    // painting aid
    struct IGetLine {
        virtual int		  Length() = 0;
        virtual const Line&	  At(int pos) = 0;
    };

protected:
    enum {
        mLine = 1,
        mPage = 2,
        mFile = 3,

        mFwd = 0,
        mBack = 1
    };

    // text appearence (font, margins)
    struct xx_TextDisp {
        int		    fontsize;
        bool	    bold;
        int		    cleartype;
        CString	    fontface;
        int		    margin_width;
        bool	    justify;
        bool	    hyphenate;
        int		    angle;

        xx_TextDisp() { LoadSettings(); }
        void	    SetFont(const CString& face, bool pbold, int size, int pct);
        void	    SaveSettings();
        void	    LoadSettings();
        void	    SetDefFont();
    } m_TextDisp;

    // window size, transforms
    struct xx_Window {
        int		    width;
        int		    height;
        int		    rwidth;
        int		    rheight;
        int		    columns;
        RECT	    cli;
        int		    progress_height;
        bool	    showprog;
        bool	    progoverride;
        bool	    rotbuttons;
        bool	    autorepeatlimit;
        DWORD	    lastkeypress;

        union {
            struct {
                unsigned int  position : 1;
                unsigned int  time : 1;
                unsigned int  as_delay : 1;
                unsigned int  battery : 1;
                unsigned int  top : 1;
                unsigned int  chapter : 1;
            };
            unsigned int  flags;
        } pb;
        struct xx_PD {
            int  pos;
            int  total;
            int  tm;
            int  as;
            int  bat;
            int  top, cc;
            CString title;
        } pd;
        int		    pb_width;
        UINT	    pd_timer;

        xx_Window() : width(1), progress_height(0), pb_width(0), pd_timer(0), rwidth(1), progoverride(true) { LoadSettings(); }
        void	    LoadSettings();
        void	    SaveSettings();
        bool	    showprogress() { return showprog && progoverride; }
    } m_Window;

    bool		UpdateWindowPD();
    void		UpdateProgressBar();
    void		StartWindowPDTimer();

    // history list
    struct xx_History {
        CList<FilePos, FilePos&> pstack;
        POSITION		    stacktop;

        xx_History() : stacktop(NULL) { }

        void Clear() { pstack.RemoveAll(); stacktop = NULL; }
    } m_History;

    struct xx_Selection {
        FilePos	    start;
        int		    len;

        xx_Selection() : len(0) { }
    } m_Sel;

    // text searching
    struct xx_Search {
        FilePos	    matchpos;
        Buffer<wchar_t> searchstr;
        bool	    matchcase;
        bool            fromcurpos;

        xx_Search() : matchcase(false), fromcurpos(true) { }
    } m_Search;

    // mouse handler info
    struct xx_Mouse {
        CPoint	    start, last, end;
        bool	    trackmouse;
        bool	    dmove;

        xx_Mouse() {
            start.x = start.y = end.x = end.y = 0;
            trackmouse = false;
        }
    } m_Mouse;

    // dictionary state
    struct xx_Dict {
        Buffer<wchar_t> lastdictlookup;
        bool	    okstate;
        CString	    curdict;
        auto_ptr<TextParser>	curdictparser;

        xx_Dict() : okstate(false) { }
    } m_Dict;

    // text input popup window state
    struct xx_UI {
        CString	    inp;
        bool	    visible;
        UINT	    timer;
        RECT	    rc;

        xx_UI() : visible(false), timer(0) { }
    } m_UI;

    // global update timer
    UINT		  m_timer;

    // current bookmarks popup window
    struct xx_BP : public IGetLine {
        LineArray	    lines;
        RECT	    rc;
        Buffer<wchar_t> text;
        bool	    visible;
        int		    bmkidx;

        xx_BP() : visible(false), bmkidx(-1) { }

        int		    Length() { return lines.GetSize(); }
        const Line&	    At(int i) { return lines[i]; }
    } m_BP;

    // autoscroll
    struct xx_AS {
        // currently shown and underlined line
        int		    column;
        int		    line;
        int		    delay; // per-line delay in microseconds
        UINT	    timer;
        FilePos	    top_pos;

        xx_AS() : column(-1), line(-1), timer(0) { LoadSettings(); }
        void  LoadSettings();
        void  SaveSettings();
    } m_AS;

    // main formatter and text file
    auto_ptr<TextFormatter> m_formatter;
    auto_ptr<TextFile>	  m_textfile;

    void		  PaintColumn(CFDC& dc, const RECT& update,
        const RECT& rc, const RECT& cli,
        IGetLine *lines, int margin, bool chkbmk);
    void		  PaintUserInput(CFDC& dc, const RECT& rc, const RECT& cli,
        const Buffer<wchar_t>& text);
    void		  PaintProgressBar(CFDC& dc, const RECT& rc, const RECT& cli);
    void		  PaintBookmarkPopup(CFDC& dc, const RECT& rc, const RECT& cli);
    void		  PaintLine(CFDC& dc, const RECT& cli, RECT& line, int margin, const Line& l);
    void		  PaintSingleLine(int column, int line, COLORREF underline);
    void		  PaintSbItem(CFDC& dc, const wchar_t *text, int len,
        const RECT& rc, const RECT& cli, int& pb_width);
    void		  RedrawProgressBar();

    void		  Move(int dir, int amount);
    void		  PushPos();
    void		  MoveAbs(FilePos pos);
    void		  EnsureVisible(FilePos pos);
    void		  MovePercent(int p);
    void		  DoFind();
    void		  CalcSizes();
    void		  HandleMouseDown(CPoint point);
    void		  TrackMouse();
    void		  DisplaySelectionMenu(CPoint point);
    FilePos	  CurFilePos();


    void		  ComplexLine(CFDC& dc, const RECT& cli, RECT& line,
        int x, const Line& l);
    void		  HighlightBookmarks(CFDC& dc, const RECT& cli, int left,
        int y, int margin, const Line& l,
        FilePos bmkstart, FilePos bmkend);

    void		  InvalidateRect(const RECT& rc);

    bool		  LookupAddr(const POINT& pt, FilePos& p);
    bool		  LookupPoint(FilePos p, POINT& pt);
    void		  CalcSelection(FilePos& p, int& len);
    bool		  GetSelText(Buffer<wchar_t>& str);
    void		  SaveInfo();

    void		  ShowText();
    void		  HideText();

    void		  DisplayBookmarkPopup(const POINT& spot, const Buffer<wchar_t>& text);
    void		  DisplayBookmarkPopup(int index);
    void		  FormatBookmarkPopup(CFDC& dc, const POINT& spot);
    void		  HideBookmarkPopup();

    void		  QueueRepaint() { m_BP.visible = false; Invalidate(FALSE); }
    void		  QueueRepaint(const RECT& rc);
    void		  SetSelection(const FilePos& p, int len);
    int		  GetSelParagraphCount();

    void		  Advance(FilePos& p, int len);
    void		  InvalidateRange(const FilePos& p1, const FilePos& p2);

    static void	  CopyToClipboard(const wchar_t *text, int length, HWND hWnd);

    bool		  CanAddUIChar(TCHAR ch);

    COLORREF	  v_C_BG();

    // autoscroll support
    void		  StepAS();
    void		  RestartASTimer();

    //{{AFX_MSG(CTView)
    afx_msg void OnPaint();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnAppAbout();
    afx_msg void OnUpdateOptions(CCmdUI* pCmdUI);
    afx_msg void OnOptions();
    afx_msg void OnUpdateFileformat(CCmdUI* pCmdUI);
    afx_msg void OnFileformat();
    afx_msg void OnUpdateBack(CCmdUI* pCmdUI);
    afx_msg void OnBack();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnUpdateFind(CCmdUI* pCmdUI);
    afx_msg void OnFind();
    afx_msg void OnFindnext();
    afx_msg void OnUpdateFindnext(CCmdUI* pCmdUI);
    afx_msg void OnUpdateColors(CCmdUI* pCmdUI);
    afx_msg void OnColors();
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnAddBmk();
    afx_msg void OnUpdateAddBmk(CCmdUI* pCmdUI);
    afx_msg void OnBookmarks();
    afx_msg void OnUpdateBookmarks(CCmdUI* pCmdUI);
    afx_msg void OnLineUp();
    afx_msg void OnLineDown();
    afx_msg void OnPageUp();
    afx_msg void OnPageDown();
    afx_msg void OnStartFile();
    afx_msg void OnEndFile();
    afx_msg void OnKeys();
    afx_msg void OnUpdateKeys(CCmdUI* pCmdUI);
    afx_msg void OnDestroy();
    afx_msg void OnStyles();
    afx_msg void OnUpdateStyles(CCmdUI* pCmdUI);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnMiscopt();
    afx_msg void OnUpdateMiscopt(CCmdUI* pCmdUI);
    afx_msg void OnEditCopy();
    afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
    afx_msg void OnRotate();
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnNextSection();
    afx_msg void OnPrevSection();
    afx_msg void OnNextBm();
    afx_msg void OnPrevBm();
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnForward();
    afx_msg void OnUpdateForward(CCmdUI* pCmdUI);
    afx_msg void OnOK();
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void StopAS();
    afx_msg void StartAS();
    afx_msg void FasterAS();
    afx_msg void SlowerAS();
    afx_msg void FasterASFine();
    afx_msg void SlowerASFine();
    afx_msg void ToggleAS();
    afx_msg void OnTogglePB();
    afx_msg void OnTogglePBChapter();
    afx_msg void OnTogglePBPos();
    afx_msg void OnTogglePBTop();
    afx_msg void OnTogglePBAS();
    afx_msg void OnTogglePBTime();
    afx_msg void OnTogglePBBat();
    afx_msg void OnUpdateTogglePB(CCmdUI* pCmdUI);
    afx_msg void OnUpdateTogglePBChapter(CCmdUI *pCmdUI);
    afx_msg void OnUpdateTogglePBPos(CCmdUI* pCmdUI);
    afx_msg void OnUpdateTogglePBTop(CCmdUI* pCmdUI);
    afx_msg void OnUpdateTogglePBAS(CCmdUI* pCmdUI);
    afx_msg void OnUpdateTogglePBTime(CCmdUI* pCmdUI);
    afx_msg void OnUpdateTogglePBBat(CCmdUI* pCmdUI);
    afx_msg void OnExportBmk();
    afx_msg void OnUpdateExportBmk(CCmdUI* pCmdUI);
    afx_msg void OnNewColorProfile();
    afx_msg void OnUpdateNewColorProfile(CCmdUI *pCmdUI);
    afx_msg void OnDelColorProfile();
    afx_msg void OnUpdateDelColorProfile(CCmdUI *pCmdUI);
    afx_msg void OnNextColorProfile();
    afx_msg void OnUpdateNextColorProfile(CCmdUI *pCmdUI);
    //}}AFX_MSG
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg LRESULT OnHotkey(WPARAM, LPARAM);
    afx_msg void OnSelColor(UINT cmd);
    afx_msg void OnUpdateSelColor(CCmdUI *pCmdUI);
    afx_msg LRESULT OnPower(WPARAM wParam, LPARAM lParam) {
        if (wParam == PBT_APMQUERYSUSPEND)
            return m_AS.timer && m_hWnd == ::GetFocus() ? BROADCAST_QUERY_DENY : TRUE;
        return TRUE;
    }
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TVIEW_H__BF90ABF1_7A00_4E3D_8E68_A4B0C27E2A62__INCLUDED_)
