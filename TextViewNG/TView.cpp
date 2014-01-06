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
* $Id: TView.cpp,v 1.164.2.70 2007/04/02 02:10:45 mike Exp $
*
*/

#define _WIN32_WINNT _WIN32_WINNT_MAXVER

#include <afxcmn.h>
#include <afxtempl.h>
#include <afxext.h>
#include <shlobj.h>
#include <shellapi.h>
#include <mmsystem.h>

#include "config.h"
#include "resource.h"

#include "ptr.h"
#include "FilePos.h"
#include "Colors.h"
#include "OptionsDialog.h"
#include "FileFormatDialog.h"
#include "InputBox.h"
#include "ColorSelector.h"
#include "StylesDlg.h"
#include "MiscOptDlg.h"
#include "Keys.h"
#include "Unicode.h"
#include "TextFile.h"
#include "TextFormatter.h"
#include "TextViewNG.h"
#include "TVFrame.h"
#include "Rotate.h"
#include "TView.h"
#include "Bookmarks.h"
#include "ContentsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef CSIDL_STARTMENU
#define CSIDL_STARTMENU 0x000b
#endif

#define DO_CLIP    0
#define PROGRESS_M   3
#define PROGRESS_A   11
#define PROGRESS_C   10
#define PROGRESS_F   6
#define FRAME_SIZE   3

// colors
ColorItem    g_colors[] = {
    { _T("Text"), RGB(0, 0, 0) },
    { _T("Highlight 1"), RGB(0, 0, 192) },
    { _T("Highlight 2"), RGB(129, 0, 0) },
    { _T("Highlight 3"), RGB(192, 0, 192) },
    { _T("Highlight 4"), RGB(0, 112, 0) },
    { _T("Highlight 5"), RGB(0, 112, 112) },
    { _T("Highlight 6"), RGB(0, 192, 0) },
    { _T("Highlight 7"), RGB(128, 128, 128) },
    { _T("Background"), RGB(255, 255, 255) },
    { _T("Highlighted Background"), RGB(16, 192, 255) },
    { _T("Gauge"), RGB(150, 150, 150) },
    { _T("Section Tick"), RGB(0, 0, 0) },
    { _T("Bookmark Tick"), RGB(255, 0, 0) },
    { _T("User input color"), RGB(0, 128, 0) },
    { _T("User input background"), RGB(203, 224, 199) },
    { _T("Autoscroll underline"), RGB(0, 0, 0) },
    { NULL }
};

int     g_color_profile;

enum {
    TM_SAVEINFO = 1,
    TM_USERTEXT,
    TM_AS,
    TM_PD,
};

enum {
    MAXUSERINPUTWIDTH = 400,
    USERINPUTFLAGS = Attr::BOLD,
    USERINPUTSIZE = 3
};

COLORREF  CTView::v_C_BG() { return m_textfile->IsImage() ? RGB(0, 0, 0) : C_BG; }

/////////////////////////////////////////////////////////////////////////////
// CTView

CTView::CTView()
{
    LoadColors();

    // read other params
    CFDC::SetCacheSize(CTVApp::GetInt(_T("FontCacheSize"), 6));
    SetRotAngle(m_TextDisp.angle);
}

// colors support
void  SaveColors() {
    AfxGetApp()->WriteProfileInt(_T("Colors"), _T("Profile"), g_color_profile);

    if (g_color_profile != 0) {
        CString   name;
        for (int i = 0; g_colors[i].name; ++i) {
            name.Format(_T("%s_%d"), g_colors[i].name, g_color_profile);
            AfxGetApp()->WriteProfileInt(_T("Colors"), name, g_colors[i].value);
        }
    }
    else
        for (int i = 0; g_colors[i].name; ++i)
            AfxGetApp()->WriteProfileInt(_T("Colors"), g_colors[i].name,
            g_colors[i].value);
}

void  LoadColors() {
    // read color profile id
    g_color_profile = AfxGetApp()->GetProfileInt(_T("Colors"), _T("Profile"), 0);

    // read colors
    if (g_color_profile != 0) {
        CString name;
        for (int i = 0; g_colors[i].name; ++i) {
            name.Format(_T("%s_%d"), g_colors[i].name, g_color_profile);
            g_colors[i].value = AfxGetApp()->GetProfileInt(_T("Colors"),
                name, g_colors[i].value);
        }
    }
    else
        for (int i = 0; g_colors[i].name; ++i)
            g_colors[i].value = AfxGetApp()->GetProfileInt(_T("Colors"),
            g_colors[i].name, g_colors[i].value);
}

bool  NextColorProfile() {
    HKEY hKey = AfxGetApp()->GetSectionKey(_T("Colors"));
    if (hKey == NULL)
        return false;

    int   next_id = 0;
    bool   found = false;

    TCHAR name[16];
    for (DWORD idx = 0;; ++idx) {
        DWORD   namesize = sizeof(name) / sizeof(name[0]);
        DWORD   type;
        if (RegEnumValue(hKey, idx, name, &namesize, 0, &type, NULL, NULL) != ERROR_SUCCESS)
            break;

        int     id;
        if (_stscanf_s(name, _T("%d"), &id) != 1)
            continue;

        if (found) {
            next_id = id;
            break;
        }

        if (g_color_profile == id)
            found = true;

        if (g_color_profile == 0) {
            next_id = id;
            break;
        }
    }

    RegCloseKey(hKey);

    if (next_id == g_color_profile)
        return false;

    AfxGetApp()->WriteProfileInt(_T("Colors"), _T("Profile"), next_id);

    LoadColors();

    return true;
}

void  AddColorProfileNames(void *vmenu, int startpos) {
    CMenu *menu = (CMenu *)vmenu;

    menu->InsertMenu(startpos, MF_BYPOSITION | MF_STRING, COLORS_BASE, _T("Default"));
    if (0 == g_color_profile)
        menu->CheckMenuRadioItem(startpos, startpos, startpos, MF_BYPOSITION);
    ++startpos;

    HKEY hKey = AfxGetApp()->GetSectionKey(_T("Colors"));
    if (hKey == NULL)
        return;

    TCHAR name[16];
    for (DWORD idx = 0;; ++idx) {
        DWORD   namesize = sizeof(name) / sizeof(name[0]);
        DWORD   type;
        if (RegEnumValue(hKey, idx, name, &namesize, 0, &type, NULL, NULL) != ERROR_SUCCESS)
            break;

        int     id;
        if (_stscanf_s(name, _T("%d"), &id) != 1)
            continue;

        TCHAR   data[64];
        DWORD   dsize = sizeof(data);
        if (RegQueryValueEx(hKey, name, 0, &type, (BYTE*)data, &dsize) == ERROR_SUCCESS && type == REG_SZ) {
            menu->InsertMenu(startpos, MF_BYPOSITION | MF_STRING, COLORS_BASE + id, data);
            if (id == g_color_profile)
                menu->CheckMenuRadioItem(startpos, startpos, startpos, MF_BYPOSITION);
            ++startpos;
        }
    }

    RegCloseKey(hKey);
}

void CTView::xx_Window::LoadSettings() {
    columns = CTVApp::GetInt(_T("Columns"), DEF_COLUMNS);
    rotbuttons = CTVApp::GetInt(_T("RotateButtons"), DEF_ROTB) != 0;
    autorepeatlimit = CTVApp::GetInt(_T("AutoRepeatLimit"), DEF_AUTOREPEATLIMIT) != 0;
    showprog = CTVApp::GetInt(_T("ShowProgress"), DEF_SHOWPROGRESS) != 0;
    pb.flags = CTVApp::GetInt(_T("ProgressBar"), DEF_PROGRESSBAR);
    if (columns < 1 || columns>4)
        columns = 1;
}

void  CTView::xx_Window::SaveSettings() {
    CTVApp::SetInt(_T("ShowProgress"), showprog);
    CTVApp::SetInt(_T("ProgressBar"), pb.flags);
    CTVApp::SetInt(_T("Columns"), columns);
    CTVApp::SetInt(_T("RotateButtons"), rotbuttons);
    CTVApp::SetInt(_T("AutoRepeatLimit"), autorepeatlimit);
}

void  CTView::xx_TextDisp::LoadSettings() {
    angle = CTVApp::GetInt(_T("Orientation"), DEF_ORIENTATION);
    margin_width = CTVApp::GetInt(_T("Margins"), DEF_MARGINS);
    justify = CTVApp::GetInt(_T("Justify"), DEF_JUSTIFY) != 0;
    hyphenate = CTVApp::GetInt(_T("Hyphenate"), DEF_HYPHENATE) != 0;
    SetFont(CTVApp::GetStr(_T("FontFace"), DEF_FACE),
        CTVApp::GetInt(_T("FontBold"), DEF_BOLD) != 0,
        CTVApp::GetInt(_T("FontSize"), DEF_SIZE),
        CTVApp::GetInt(_T("FontCleartype"), DEF_CLEARTYPE));
}

void CTView::xx_TextDisp::SaveSettings()
{
    CTVApp::SetStr(_T("FontFace"), fontface);
    CTVApp::SetInt(_T("FontSize"), fontsize);
    CTVApp::SetInt(_T("FontBold"), bold);
    CTVApp::SetInt(_T("FontCleartype"), cleartype);
    CTVApp::SetInt(_T("Orientation"), angle);
    CTVApp::SetInt(_T("Margins"), margin_width);
    CTVApp::SetInt(_T("Justify"), justify);
    CTVApp::SetInt(_T("Hyphenate"), hyphenate);
}

void CTView::xx_TextDisp::SetFont(const CString &face, bool pbold,
    int size, int pct)
{
    bold = pbold;
    cleartype = pct;
    fontface = face;
    fontsize = size;
    SetDefFont();
}

void CTView::xx_TextDisp::SetDefFont()
{
    CFDC::SetDefaultFont(fontface, fontsize, bold, cleartype, angle);
}

void  CTView::xx_AS::LoadSettings() {
    delay = CTVApp::GetInt(_T("AutoScrollDelay"), DEF_AS_DELAY);
}

void  CTView::xx_AS::SaveSettings() {
    CTVApp::SetInt(_T("AutoScrollDelay"), delay);
}

CTView::~CTView() {
    SaveInfo();
    Bookmarks::CleanupRegistry(CTVApp::GetInt(_T("NumBookmarks"), DEF_BOOKMARKS));
}

void  CTView::SaveInfo() {
    if (m_textfile.get() && m_formatter.get())
        m_textfile->SaveBookmarks(CurFilePos());
}

void  CTView::Init() {
    GetClientRect(&m_Window.cli);
    CalcSizes();
    Keys::SetWindow(m_hWnd);
    m_timer = SetTimer(TM_SAVEINFO, DEF_SAVEINTERVAL, 0);
    StartWindowPDTimer();
    // initialize progress bar height
    CFDC dc(m_hWnd);
    dc.SelectFontAbs(MulDiv(PROGRESS_F, GetDeviceCaps(dc.DC(), LOGPIXELSY), 72), CFDC::FORCENORMALWEIGHT | CFDC::FORCETAHOMA, true);
    int dummy;
    dc.GetFontSize(m_Window.progress_height, dummy);
    m_Window.progress_height -= 2; // XXX no inter-line spacing
}

void  CTView::SetFile(auto_ptr<TextFile> tfile) {
    if (!tfile.get())
        return;
    SaveInfo();
    m_textfile = tfile;
    m_Window.progoverride = !m_textfile->IsImage();
    m_formatter = new TextFormatter(m_textfile.get());
    m_formatter->SetJustified(m_TextDisp.justify);
    m_formatter->SetHyphenate(m_TextDisp.hyphenate);
    m_formatter->SetTop(m_textfile->bmk().StartPos());
    m_Search.matchpos = m_formatter->Eof();
    m_History.Clear();
    SaveInfo();
    // and format the page
    CalcSizes();
    QueueRepaint();
    StartWindowPDTimer();
}

BEGIN_MESSAGE_MAP(CTView, CWnd)
    //{{AFX_MSG_MAP(CTView)
    ON_WM_PAINT()
    ON_WM_SIZE()
    ON_WM_KEYDOWN()
    ON_WM_ERASEBKGND()
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    ON_UPDATE_COMMAND_UI(ID_DISP_OPTIONS, OnUpdateOptions)
    ON_COMMAND(ID_DISP_OPTIONS, OnOptions)
    ON_UPDATE_COMMAND_UI(ID_FILEFORMAT, OnUpdateFileformat)
    ON_COMMAND(ID_FILEFORMAT, OnFileformat)
    ON_UPDATE_COMMAND_UI(ID_BACK, OnUpdateBack)
    ON_COMMAND(ID_BACK, OnBack)
    ON_WM_LBUTTONDOWN()
    ON_UPDATE_COMMAND_UI(ID_FIND, OnUpdateFind)
    ON_COMMAND(ID_FIND, OnFind)
    ON_COMMAND(ID_FINDNEXT, OnFindnext)
    ON_UPDATE_COMMAND_UI(ID_FINDNEXT, OnUpdateFindnext)
    ON_UPDATE_COMMAND_UI(ID_COLORS, OnUpdateColors)
    ON_COMMAND(ID_COLORS, OnColors)
    ON_WM_LBUTTONDBLCLK()
    ON_COMMAND(ID_ADD_BMK, OnAddBmk)
    ON_UPDATE_COMMAND_UI(ID_ADD_BMK, OnUpdateAddBmk)
    ON_COMMAND(ID_BOOKMARKS, OnBookmarks)
    ON_UPDATE_COMMAND_UI(ID_BOOKMARKS, OnUpdateBookmarks)
    ON_COMMAND(ID_LINE_UP, OnLineUp)
    ON_COMMAND(ID_LINE_DOWN, OnLineDown)
    ON_COMMAND(ID_PAGE_UP, OnPageUp)
    ON_COMMAND(ID_PAGE_DOWN, OnPageDown)
    ON_COMMAND(ID_START_OF_FILE, OnStartFile)
    ON_COMMAND(ID_END_OF_FILE, OnEndFile)
    ON_COMMAND(ID_KEYS, OnKeys)
    ON_UPDATE_COMMAND_UI(ID_KEYS, OnUpdateKeys)
    ON_WM_DESTROY()
    ON_COMMAND(ID_STYLES, OnStyles)
    ON_UPDATE_COMMAND_UI(ID_STYLES, OnUpdateStyles)
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_COMMAND(ID_MISCOPT, OnMiscopt)
    ON_UPDATE_COMMAND_UI(ID_MISCOPT, OnUpdateMiscopt)
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
    ON_COMMAND(ID_ROTATE, OnRotate)
    ON_WM_TIMER()
    ON_COMMAND(ID_NEXTCH, OnNextSection)
    ON_COMMAND(ID_PREVCH, OnPrevSection)
    ON_COMMAND(ID_NEXTBM, OnNextBm)
    ON_COMMAND(ID_PREVBM, OnPrevBm)
    ON_WM_CHAR()
    ON_COMMAND(ID_FORWARD, OnForward)
    ON_UPDATE_COMMAND_UI(ID_FORWARD, OnUpdateForward)
    ON_COMMAND(ID_DO_FIND, DoFind)
    ON_COMMAND(IDOK, OnOK)
    ON_WM_KILLFOCUS()
    ON_WM_SETFOCUS()
    ON_COMMAND(ID_AS_STOP, StopAS)
    ON_COMMAND(ID_AS_START, StartAS)
    ON_COMMAND(ID_AS_SLOWER, SlowerAS)
    ON_COMMAND(ID_AS_FASTER, FasterAS)
    ON_COMMAND(ID_AS_SLOWER_FINE, SlowerASFine)
    ON_COMMAND(ID_AS_FASTER_FINE, FasterASFine)
    ON_COMMAND(ID_AS_TOGGLE, ToggleAS)
    ON_UPDATE_COMMAND_UI(ID_PB_TOGGLE, OnUpdateTogglePB)
    ON_COMMAND(ID_PB_TOGGLE, OnTogglePB)
    ON_UPDATE_COMMAND_UI(ID_PB_POS, OnUpdateTogglePBPos)
    ON_COMMAND(ID_PB_POS, OnTogglePBPos)
    ON_UPDATE_COMMAND_UI(ID_PB_CHAPTER, OnUpdateTogglePBChapter)
    ON_COMMAND(ID_PB_CHAPTER, OnTogglePBChapter)
    ON_UPDATE_COMMAND_UI(ID_PB_TOP, OnUpdateTogglePBTop)
    ON_COMMAND(ID_PB_TOP, OnTogglePBTop)
    ON_UPDATE_COMMAND_UI(ID_PB_TIME, OnUpdateTogglePBTime)
    ON_COMMAND(ID_PB_TIME, OnTogglePBTime)
    ON_UPDATE_COMMAND_UI(ID_PB_AS, OnUpdateTogglePBAS)
    ON_COMMAND(ID_PB_AS, OnTogglePBAS)
    ON_UPDATE_COMMAND_UI(ID_PB_BATTERY, OnUpdateTogglePBBat)
    ON_COMMAND(ID_PB_BATTERY, OnTogglePBBat)
    ON_UPDATE_COMMAND_UI(ID_FILE_EXPORTBMK, OnUpdateExportBmk)
    ON_COMMAND(ID_FILE_EXPORTBMK, OnExportBmk)
    ON_UPDATE_COMMAND_UI(ID_COLORS_ADDP, OnUpdateNewColorProfile)
    ON_COMMAND(ID_COLORS_ADDP, OnNewColorProfile)
    ON_UPDATE_COMMAND_UI(ID_COLORS_DELP, OnUpdateDelColorProfile)
    ON_COMMAND(ID_COLORS_DELP, OnDelColorProfile)
    ON_UPDATE_COMMAND_UI(ID_NEXT_PROFILE, OnUpdateNextColorProfile)
    ON_COMMAND(ID_NEXT_PROFILE, OnNextColorProfile)
    //}}AFX_MSG_MAP
    ON_WM_MOUSEWHEEL()
    ON_MESSAGE(WM_HOTKEY, OnHotkey)
    ON_COMMAND_RANGE(COLORS_BASE, COLORS_BASE + COLORS_MAX, OnSelColor)
    ON_UPDATE_COMMAND_UI_RANGE(COLORS_BASE, COLORS_BASE + COLORS_MAX, OnUpdateSelColor)
    ON_MESSAGE(WM_POWERBROADCAST, OnPower)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTView message handlers
BOOL CTView::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CWnd::PreCreateWindow(cs))
        return FALSE;

    cs.style &= ~WS_BORDER;
    cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
        NULL, NULL, NULL);

    return TRUE;
}

struct FormatterGetLine : public CTView::IGetLine {
    TextFormatter   *m_tf;
    int    m_page;
    int    m_off;
    int    m_len;

    FormatterGetLine(TextFormatter *tf, int page) : m_tf(tf), m_page(page), m_off(0) {
        for (int i = 0; i < page; ++i)
            m_off += tf->PageLength(i);
        m_len = tf->PageLength(page);
    }
    int    Length() { return m_len; }
    const Line&   At(int i) { return m_tf->GetLine(m_off + i); }
};

static inline bool Overlap(const RECT& r1, const RECT& r2) {
    return !(
        r1.right <= r2.left ||
        r1.left >= r2.right ||
        r1.bottom <= r2.top ||
        r1.top >= r2.bottom
        );
}

void CTView::OnPaint()
{
    RECT     update_rect;
    if (!GetUpdateRect(&update_rect))
        return;
    System2Window(update_rect, m_Window.cli);

    PAINTSTRUCT ps;
    {
        // do a quick check for bookmarks
        bool    checkbmk = false;
        if (m_Window.showprogress() && m_textfile->bmk().BookmarksInRange(
            m_formatter->Top(), m_formatter->Bottom()))
            checkbmk = true;
        // draw stuff
        CFDC     fdc(m_hWnd, &ps);
        RECT     col;
        col.left = col.top = 0;
        col.right = m_Window.width;
        col.bottom = m_Window.rheight;
        if (m_Window.showprogress())
            col.bottom -= m_Window.progress_height;
        for (int column = 0; column < m_Window.columns; ++column) {
            if (Overlap(col, update_rect)) {
                FormatterGetLine   gl(m_formatter.get(), column);
                PaintColumn(fdc, update_rect, col, m_Window.cli,
                    &gl, m_TextDisp.margin_width, checkbmk);
            }
            col.left += m_Window.width;
            col.right += m_Window.width;
        }
        if (m_Window.showprogress()) {
            col.left = 0; col.top = m_Window.rheight - m_Window.progress_height;
            col.right = m_Window.rwidth; col.bottom = m_Window.rheight;
            if (Overlap(col, update_rect)) {
                UpdateWindowPD();
                PaintProgressBar(fdc, col, m_Window.cli);
            }
        }
        if (m_UI.visible && Overlap(m_UI.rc, update_rect))
            PaintUserInput(fdc, m_UI.rc, m_Window.cli, Unicode::ToWCbuf(m_UI.inp));
        if (m_BP.visible && Overlap(m_BP.rc, update_rect))
            PaintBookmarkPopup(fdc, m_BP.rc, m_Window.cli);
    }
}

void  CTView::RedrawProgressBar() {
    if (!m_Window.showprogress())
        return;

    CFDC fdc(m_hWnd);
    RECT col;
    col.left = 0; col.top = m_Window.rheight - m_Window.progress_height;
    col.right = m_Window.rwidth; col.bottom = m_Window.rheight;
    UpdateWindowPD();
    PaintProgressBar(fdc, col, m_Window.cli);
}

void CTView::PaintSingleLine(int column, int line, COLORREF underline) {
    if (column < 0 || column >= m_Window.columns)
        return;

    if (line < 0 || line >= m_formatter->PageLength(column))
        return;

    FormatterGetLine   gl(m_formatter.get(), column);

    RECT   line_rect;

    line_rect.left = m_Window.width*column;
    line_rect.right = line_rect.left + m_Window.width;

    line_rect.top = 0;

    for (int i = 0; i < line; ++i)
        line_rect.top += gl.At(i).height;

    const Line& line_data = gl.At(line);

    line_rect.bottom = line_rect.top + line_data.height;

    RECT   cli;
    GetClientRect(&cli);

    CFDC   dc(m_hWnd);
    dc.SetTextColor(C_NORM);
    dc.SetBkColor(v_C_BG());

    RECT   tmp = line_rect;
    PaintLine(dc, cli, tmp, m_TextDisp.margin_width, line_data);

    dc.SetColor(C_TOCBM);

    HighlightBookmarks(dc, cli, line_rect.left, m_TextDisp.margin_width,
        line_rect.top, line_data, line_data.pos, gl.At(line + 1).pos);

    if (underline != CLR_DEFAULT) {
        tmp = line_rect;
        tmp.top = tmp.bottom = tmp.bottom - 1;
        dc.SetColor(underline);
        TDrawLine(dc.DC(), cli, tmp);
    }

    // if this is the last line in column, then paint till column bottom
    if (line == gl.Length() - 1) {
        line_rect.top = line_rect.bottom;
        line_rect.bottom = m_Window.rheight;
        TDrawText(dc.DC(), line_rect.left, line_rect.top, cli, line_rect, NULL, 0, NULL);
        if (m_Window.showprogress()) {
            tmp.left = 0; tmp.top = m_Window.rheight - m_Window.progress_height;
            tmp.right = m_Window.rwidth; tmp.bottom = m_Window.rheight;
            UpdateWindowPD();
            PaintProgressBar(dc, tmp, m_Window.cli);
        }
    }
}

void CTView::PaintSbItem(CFDC& dc, const wchar_t *text, int len, const RECT& rc,
    const RECT& cli, int& pb_width)
{
    if (len < 0)
        len = wcslen(text);

    RECT       ii;
    SIZE       sz;
    int       nch = 0;

    dc.GetTextExtent(text, len, rc.right - rc.left, nch, NULL, sz);
    if (pb_width > 0)
        sz.cx += 3; // XXX padding
    ii.top = rc.top - 2;
    ii.bottom = rc.bottom;
    ii.left = rc.right - sz.cx - pb_width - 1;
    ii.right = ii.left + sz.cx;
    TDrawText(dc.DC(), ii.left, ii.top, cli, ii, text, len, NULL, ETO_OPAQUE);
    pb_width += sz.cx;
}

bool CTView::UpdateWindowPD() {
    bool   upd = false;

    // calculate current position
    int   top = m_textfile->GetTotalLength(m_formatter->DocId());
    int   cur = m_formatter->DocId() < 0 ? m_textfile->GetPStart(m_formatter->DocId(), m_formatter->Top().para) :
        m_textfile->AbsPos(m_formatter->Bottom());
    m_Window.pd.cc = cur;
    if (m_formatter->DocId() >= 0)
        cur = (cur + 1) >> 11;
    else
        cur = -(top ? MulDiv(100, cur, top) : 100) - 1;

    if (cur != m_Window.pd.pos) {
        if (m_Window.pb.position)
            upd = true;
        m_Window.pd.pos = cur;
    }

    if (top != m_Window.pd.top) {
        if (m_Window.pb.top && m_formatter->DocId() >= 0)
            upd = true;
        m_Window.pd.top = top;
    }

    // find current chapter title
    if (m_Window.pb.chapter) {
        int idx = m_textfile->bmk().BFind(m_formatter->Top(), Bookmarks::SPREVCH);
        if (idx < m_textfile->bmk().GetSize() &&
            m_textfile->bmk().Ref(idx).docid == m_formatter->Top().docid)
        {
            CString textcur = m_textfile->bmk().Text(idx, m_textfile.get());
            if (textcur != m_Window.pd.title) {
                upd = true;
                m_Window.pd.title = textcur;
            }
        }
        else if (!m_Window.pd.title.IsEmpty()) {
            m_Window.pd.title.Empty();
            upd = true;
        }
    }

    cur = m_AS.delay / 1000;
    if (cur != m_Window.pd.as) {
        if (m_Window.pb.as_delay)
            upd = true;
        m_Window.pd.as = cur;
    }

    SYSTEMTIME stm;
    ::GetLocalTime(&stm);
    cur = (stm.wHour << 8) | stm.wMinute;
    if (cur != m_Window.pd.tm) {
        if (m_Window.pb.time)
            upd = true;
        m_Window.pd.tm = cur;
    }

    SYSTEM_POWER_STATUS     pws;
    BOOL      ok = GetSystemPowerStatus(&pws);
    if (ok && pws.BatteryLifePercent >= 0 && pws.BatteryLifePercent <= 100) {
        if (pws.ACLineStatus == 1)
            cur = 101;
        else
            cur = pws.BatteryLifePercent;
    }
    else
        cur = -1;

    if (cur != m_Window.pd.bat) {
        if (m_Window.pb.battery)
            upd = true;
        m_Window.pd.bat = cur;
    }

    return upd;
}

void CTView::StartWindowPDTimer() {
    if (m_Window.showprogress() && (m_Window.pb.battery || m_Window.pb.time))
        m_Window.pd_timer = SetTimer(TM_PD, 2000, NULL);
    else {
        KillTimer(m_Window.pd_timer);
        m_Window.pd_timer = 0;
    }
}

void CTView::UpdateProgressBar() {
    if (UpdateWindowPD()) {
        CFDC fdc(m_hWnd);
        RECT col;
        col.left = 0; col.top = m_Window.rheight - m_Window.progress_height;
        col.right = m_Window.rwidth; col.bottom = m_Window.rheight;
        PaintProgressBar(fdc, col, m_Window.cli);
    }
}

void CTView::PaintProgressBar(CFDC& dc, const RECT& rc, const RECT& cli) {
    RECT       col;
    wchar_t     buf[128];
    int       fonthdpi = MulDiv(PROGRESS_F, GetDeviceCaps(dc.DC(), LOGPIXELSY), 72);
    dc.SelectFontAbs(fonthdpi, CFDC::FORCENORMALWEIGHT | CFDC::FORCETAHOMA);
    ::SetBkMode(dc.DC(), OPAQUE);

    // clear area
    dc.SetBkColor(v_C_BG());
    TDrawText(dc.DC(), rc.left, rc.top, m_Window.cli, rc, NULL, 0, NULL);
    m_Window.pb_width = 0;

    if (m_Window.pb.battery && m_Window.pd.bat >= 0) {
        if (m_Window.pd.bat > 100)
            wcscpy_s(buf, sizeof(buf) / sizeof(buf[0]), L"AC");
        else
            swprintf(buf, sizeof(buf) / sizeof(buf[0]), L"%d%%", m_Window.pd.bat);
        PaintSbItem(dc, buf, -1, rc, m_Window.cli, m_Window.pb_width);
    }

    if (m_Window.pb.time) {
        swprintf(buf, sizeof(buf) / sizeof(buf[0]), L"%02d:%02d", m_Window.pd.tm >> 8, m_Window.pd.tm & 0xff);
        PaintSbItem(dc, buf, -1, rc, m_Window.cli, m_Window.pb_width);
    }

    if (m_Window.pb.as_delay && m_AS.timer) {
        swprintf(buf, sizeof(buf) / sizeof(buf[0]), L"%d", m_Window.pd.as);
        PaintSbItem(dc, buf, -1, rc, m_Window.cli, m_Window.pb_width);
    }

    // draw vpage number for normal docs, or a back button for dictionary
    if (m_Window.pb.position || (m_Window.pb.top && m_Window.pd.pos >= 0)) {
        if (m_Window.pd.pos >= 0) {
            if (m_Window.pb.top)
                swprintf(buf, sizeof(buf) / sizeof(buf[0]), L"%d/%d", m_Window.pd.pos, m_Window.pd.top >> 11);
            else
                swprintf(buf, sizeof(buf) / sizeof(buf[0]), L"%d", m_Window.pd.pos);
        }
        else
            swprintf(buf, sizeof(buf) / sizeof(buf[0]), L"%d%%", -m_Window.pd.pos - 1);
        PaintSbItem(dc, buf, -1, rc, m_Window.cli, m_Window.pb_width);
    }


    if (m_Window.pb.chapter && !m_Window.pd.title.IsEmpty()) {
        // draw chapter title
        RECT  rc2 = rc;
        rc2.top -= 2;
        rc2.left += 3; // XXX padding
        rc2.right -= m_Window.pb_width + 2; // XXX padding
        SIZE    sz;
        int     nch = 0;
        CString tmp(m_Window.pd.title);
        dc.SelectFontAbs(fonthdpi, CFDC::FORCENORMALWEIGHT | CFDC::FORCETAHOMA, true);
        dc.GetTextExtent(tmp, tmp.GetLength(), rc2.right - rc2.left, nch, NULL, sz);
        if (nch < tmp.GetLength()) {
            // doesnt fit
            int     nch2;
            wchar_t ch = 0x2026;
            dc.GetTextExtent(&ch, 1, 8192, nch2, NULL, sz);
            dc.GetTextExtent(tmp, tmp.GetLength(), rc2.right - rc2.left - sz.cx, nch, NULL, sz);
            if (nch > 0) {
                tmp.Delete(nch, tmp.GetLength() - nch);
                tmp += ch;
            }
            else
                tmp.Empty();
        }
        dc.SelectFontAbs(fonthdpi, CFDC::FORCENORMALWEIGHT | CFDC::FORCETAHOMA);
        TDrawText(dc.DC(), rc2.left, rc2.top, cli, rc2, tmp, tmp.GetLength(), NULL, ETO_OPAQUE | ETO_CLIPPED);
    }
    else {
        // draw position bar
        RECT  rc2 = rc;
        rc2.top += 1 + (m_Window.progress_height - 5) / 2;

        int   bw = rc2.right - rc2.left - 2 * PROGRESS_M - 2 * PROGRESS_A - m_Window.pb_width;
        int   pos = m_Window.pd.top ? MulDiv(bw, m_Window.pd.cc, m_Window.pd.top) : bw;
        col.left = rc2.left + PROGRESS_M + PROGRESS_A; col.top = rc2.top + 1;
        col.right = col.left + pos; col.bottom = col.top;
        dc.SetColor(C_GAUGE);
        TDrawLine(dc.DC(), m_Window.cli, col);
        col.top += 2;
        col.bottom = col.top;
        TDrawLine(dc.DC(), m_Window.cli, col);
        // draw a thin black line
        col.left = rc2.left + PROGRESS_M; col.top = col.bottom = rc2.top + 2;
        col.right = rc2.right - PROGRESS_M - m_Window.pb_width;
        dc.SetColor(C_TOCL0); // XXX
        TDrawLine(dc.DC(), m_Window.cli, col);
        // draw arrows
        col.top = rc2.top + 1;
        col.bottom = col.top + 3;
        col.left = col.right = rc2.left + PROGRESS_M + 1;
        TDrawLine(dc.DC(), m_Window.cli, col);
        col.left = col.right = rc2.left + PROGRESS_M + 4;
        TDrawLine(dc.DC(), m_Window.cli, col);
        col.left = col.right = rc2.right - PROGRESS_M - m_Window.pb_width - 5;
        TDrawLine(dc.DC(), m_Window.cli, col);
        col.left = col.right = rc2.right - PROGRESS_M - m_Window.pb_width - 2;
        TDrawLine(dc.DC(), m_Window.cli, col);
        col.top = rc2.top;
        col.bottom = col.top + 5;
        col.left = col.right = rc2.left + PROGRESS_M + 2;
        TDrawLine(dc.DC(), m_Window.cli, col);
        col.left = col.right = rc2.left + PROGRESS_M + 5;
        TDrawLine(dc.DC(), m_Window.cli, col);
        col.left = col.right = rc2.right - PROGRESS_M - m_Window.pb_width - 6;
        TDrawLine(dc.DC(), m_Window.cli, col);
        col.left = col.right = rc2.right - PROGRESS_M - m_Window.pb_width - 3;
        TDrawLine(dc.DC(), m_Window.cli, col);
        // draw chapter ticks
        if (m_Window.pd.top) {
            Bookmarks& bm = m_textfile->bmk();
            bool  doall = bm.GetSize() < (rc2.right - rc2.left) / 3;
            bool  doch = bm.NumTopMarks() < (rc2.right - rc2.left) / 3;
            bool  dobm = bm.NumBookmarks() < (rc2.right - rc2.left) / 3;
            if (doch || dobm || doall) {
                for (int ii = 0; ii < bm.GetSize(); ++ii) {
                    if (bm.Ref(ii).docid != m_formatter->DocId())
                        continue;
                    COLORREF   color = C_TOCL0;
                    col.top = rc2.top;
                    col.bottom = col.top + 5;
                    if (bm.Flags(ii)&Bookmarks::BMK) {
                        if (!dobm)
                            continue;
                        color = C_TOCBM;
                    }
                    else if (bm.Level(ii) > 0) {
                        if (!doall)
                            continue;
                        col.top = rc2.top + 1;
                        col.bottom = col.top + 3;
                    }
                    else if (!doch)
                        continue;
                    col.left = col.right = rc2.left + PROGRESS_M + PROGRESS_A + MulDiv(bw,
                        m_textfile->AbsPos(bm.Ref(ii)), m_Window.pd.top);
                    dc.SetColor(color);
                    TDrawLine(dc.DC(), m_Window.cli, col);
                }
            }
        }
    }
}

void CTView::PaintBookmarkPopup(CFDC& dc, const RECT& rc, const RECT& cli) {
    // bascially it's a rectangle+paintcolumn
    // draw frame first
    POINT   pt[5];
    pt[0].x = rc.left; pt[0].y = rc.top;
    pt[1].x = rc.right - 1; pt[1].y = rc.top;
    pt[2].x = rc.right - 1; pt[2].y = rc.bottom - 1;
    pt[3].x = rc.left; pt[3].y = rc.bottom - 1;
    pt[4] = pt[0];
    dc.SetColor(C_NORM);
    TDrawPolyLine(dc.DC(), cli, pt, 5);
    // shrink rc a bit to avoid overwriting frame
    RECT   nrc = rc;
    nrc.left++; nrc.right--;
    nrc.top++; nrc.bottom--;
    PaintColumn(dc, nrc, nrc, cli, &m_BP, FRAME_SIZE - 1, false);
}

void CTView::OnSize(UINT nType, int cx, int cy) {
    CWnd::OnSize(nType, cx, cy);
    if (cx == 0 || cy == 0) // don't bother with invalid sizes
        return;
    GetClientRect(&m_Window.cli);
    CalcSizes();
}

void CTView::PaintLine(CFDC& dc, const RECT& cli, RECT& line,
    int margin, const Line& l)
{
    int   x = margin + l.ispace;
    if (l.flags&Line::image) {
        Image img;
        if (m_textfile->GetImage(l.href, dc.DC(), l.base, l.imageheight,
            m_TextDisp.angle, img))
        {
            // clean left and right margins
            RECT  rma = line;
            if (x) {
                rma.right = rma.left + x;
                TDrawText(dc.DC(), rma.left, rma.top, cli, rma, NULL, 0, NULL);
            }
            rma.left = line.left + x + img.width;
            rma.right = line.right;
            if (rma.right != rma.left)
                TDrawText(dc.DC(), rma.left, rma.top, cli, rma, NULL, 0, NULL);
            TDrawBitmap(dc.DC(), img.hBmp, line.left + x, line.top,
                l.height, cli, img.width, img.height, l.yoffset);
        }
        else
            TDrawText(dc.DC(), line.left, line.top, cli, line, NULL, 0, NULL);
    }
    else if (l.flags&Line::defstyle) {
        int   fh, fa; // fontheight,fontascent

        dc.SelectFont(0, 0);
        dc.GetFontSize(fh, fa);
        TDrawText(dc.DC(), x + line.left, line.top + l.base - fa, cli, line,
            l.str, l.str.size(), l.dx);
    }
    else {
        ComplexLine(dc, cli, line, x, l);
    }
}

void  CTView::HighlightBookmarks(CFDC& dc, const RECT& cli, int left,
    int margin, int y, const Line& l,
    FilePos bmkstart, FilePos bmkend)
{
    RECT     r;
    int     j, nc;

    while (m_textfile->bmk().BookmarkFind(bmkstart, bmkend)) {
        r.top = y;
        r.left = left + margin + l.ispace;
        nc = bmkstart.off - l.pos.off;
        for (j = 0; j < nc; ++j)
            r.left += l.dx[j];
        r.right = r.left;
        r.bottom = y + l.height;
        TDrawLine(dc.DC(), cli, r);
        r.right = r.left + 3;
        r.bottom = r.top;
        TDrawLine(dc.DC(), cli, r);
        bmkstart.off++;
    }
}

void CTView::PaintColumn(CFDC &dc, const RECT& update,
    const RECT& col, const RECT& cli, IGetLine *gl,
    int margin, bool chkbmk)
{
    RECT   line;
    dc.SetTextColor(C_NORM);
    dc.SetBkColor(v_C_BG());

    line.top = col.top;

    int i;
    int pl = gl->Length();

    for (i = 0; i < pl; ++i, line.top = line.bottom) {
        line.left = col.left;
        line.right = col.right;
        const Line& l = gl->At(i);
        line.bottom = line.top + l.height;
        if (!Overlap(line, update))
            continue;
        PaintLine(dc, cli, line, margin, l);
    }
    line.left = col.left;
    line.right = col.right;
    line.bottom = col.bottom;
    TDrawText(dc.DC(), line.left, line.top, cli, line, NULL, 0, NULL);
    if (chkbmk && pl) {
        // now paint bookmarks if any
        FilePos bmkstart, bmkend;
        bmkend = gl->At(0).pos;
        dc.SetColor(C_TOCBM);
        int     y = col.top;
        for (i = 0; i < pl; ++i) {
            const Line& l = gl->At(i);
            bmkstart = bmkend;
            bmkend = gl->At(i + 1).pos;
            HighlightBookmarks(dc, cli, col.left, margin, y, l, bmkstart, bmkend);
            y += l.height;
        }
    }
}

/*
Generic line structure:

+-----------------+------+----------------------------+----+------------+
|                 +  LM  +       text                 + RM +            +
+-----------------+------+----------------------------+----+------------+
|                                        +---
|           this is the
|                                         currently
|                                         painted coulmn
+--------------------------------------------

LM is the left margin an can be 0, same for RM

normally we iterate over text extracting segments with the same
formatting and painting them, but initial LM is handled separately.
if LM is not zero, and the first text segment has a different
backgound, then we reset the segment and paint only LM, otherwise
LM is attached to the first text segment. As a shortcut, when the
entire line has the same default formatting (as is the case for
99% of all text), the painting is done with the single
TDrawText call in the PaintColumn main loop.
*/

void  CTView::ComplexLine(CFDC& dc, const RECT& cli, RECT& line,
    int x, const Line& l)
{
    int    flags = ETO_OPAQUE;
    int    len = l.str.size();
    int    off = 0;
    int    fh, fa;
    int    x0 = line.left;
    int    width = line.right - line.left;
    // check if the backgound is the same
    for (int i = 0; i < len; ++i)
        if (l.attr[i].hibg)
            goto normal_draw;
    // all bg is normal, so we clear the entire line and use transparent mode
    dc.SetBkColor(v_C_BG());
    TDrawText(dc.DC(), line.left, line.top, cli, line, NULL, 0, NULL);
    flags = 0;
normal_draw:
    while (len > 0) {
        line.right = x0 + x;
        Attr      attr = l.attr[off];
        int       run = 0;
        if (attr.img) {
            line.right += l.dx[off];
            run = 1;
        }
        else
        while (run < len && attr.wa == l.attr[off + run].wa) {
            line.right += l.dx[off + run];
            ++run;
        }
        if (run == len && !attr.hibg) // fill till the end
            line.right = x0 + width;
        dc.SelectFont(attr.fsize, attr.fontattr());
        dc.GetFontSize(fh, fa);
        // non-default backgound, left margin present, and starting at the
        // very left edge of the column, then discard fisrt segment
        if (attr.hibg && x && line.left == x0) {
            line.right = x0 + x;
            run = 0;
            attr.wa = 0;
        }
        else
            dc.SetTextColor(C_TCOLOR(attr.color));
        dc.SetBkColor(attr.hibg ? C_HBG : v_C_BG());
        const wchar_t   *txt = attr.img ? L" " : l.str + off;
        TDrawText(dc.DC(), x0 + x, line.top + l.base - fa, cli, line, txt, run, l.dx + off, flags);
        if (attr.img) {
            // slow, but we were adding the feature later, and
            // the code wasn't designed for this in the first
            // place
            Paragraph p(m_textfile->GetParagraph(l.pos.docid, l.pos.para));
            int idx = l.str[off];
            Image img;
            if (idx >= 0 && idx < p.links.size() &&
                m_textfile->GetImage(p.links[idx].target, dc.DC(),
                l.dx[off],
                l.height, m_TextDisp.angle, img))
                TDrawBitmap(dc.DC(), img.hBmp, x0 + x, line.top + l.base - img.height,
                l.height, cli, img.width, img.height, 0);
        }
        len -= run;
        off += run;
        line.left = line.right;
        x = line.left - x0;
    }
    if (x < width && flags) {
        // fill right part
        line.right = x0 + width;
        dc.SetBkColor(v_C_BG());
        TDrawText(dc.DC(), line.left, line.top, cli, line, NULL, 0, NULL);
    }
    dc.SetTextColor(C_NORM);
    dc.SetBkColor(v_C_BG());
}

void  CTView::PaintUserInput(CFDC& dc, const RECT& rc, const RECT& cli,
    const Buffer<wchar_t>& text)
{
    dc.SelectFont(USERINPUTSIZE, USERINPUTFLAGS);
    dc.SetBkColor(C_UINPBG);
    dc.SetTextColor(C_UINP);
    TDrawText(dc.DC(), rc.left + 2, rc.top + 2, cli, rc, text, text.size(), NULL);
}

void CTView::CalcSizes()
{
    if (m_TextDisp.angle == 900 || m_TextDisp.angle == 2700) {
        m_Window.rheight = m_Window.cli.right - m_Window.cli.left;
        m_Window.rwidth = m_Window.cli.bottom - m_Window.cli.top;
    }
    else {
        m_Window.rheight = m_Window.cli.bottom - m_Window.cli.top;
        m_Window.rwidth = m_Window.cli.right - m_Window.cli.left;
    }
    m_Window.width = m_Window.rwidth;
    m_Window.height = m_Window.rheight;
    if (m_Window.showprogress())
        m_Window.height -= m_Window.progress_height;
    m_Window.width /= m_Window.columns;
    if (m_Window.width == 0)
        m_Window.width = 1;
    if (m_Window.height == 0)
        m_Window.height = 1;
    CFDC fdc(m_hWnd);
    // calculate userinput window position
    fdc.SelectFont(3, 0);
    int   height, ascent;
    fdc.GetFontSize(height, ascent);
    int   uwidth = m_Window.rwidth;
    if (uwidth > MAXUSERINPUTWIDTH)
        uwidth = MAXUSERINPUTWIDTH;
    m_UI.rc.left = (m_Window.rwidth - uwidth) / 2;
    m_UI.rc.right = m_UI.rc.left + uwidth;
    m_UI.rc.top = 20; // XXX
    m_UI.rc.bottom = m_UI.rc.top + height + 4;
    if (m_formatter.get()) {
        m_formatter->SetSize(m_Window.width, m_TextDisp.margin_width,
            m_Window.height, m_Window.columns, m_TextDisp.angle);
        m_formatter->Reformat(fdc);
    }
    m_BP.visible = false;
}

void CTView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    UINT cmd;

    // Check for key chatter and ignore event
    DWORD now = ::GetTickCount();
    if (m_Window.autorepeatlimit && now - m_Window.lastkeypress < REPEAT_THRESHOLD)
        return;
    m_Window.lastkeypress = now;

    // spaces are handled separately in OnChar
    if (nChar != VK_SPACE && Keys::TranslateKey(nChar, cmd,
        m_Window.rotbuttons ? m_TextDisp.angle : 0))
    {
        // repeated key
        if (nFlags & 0x4000 && CTVApp::TopQueuedCmd() == cmd)
            return;
        CTVApp::QueueCmd(cmd);
        return;
    }

    CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CTView::OnLineUp() { Move(mBack, mLine); }
void CTView::OnLineDown() { Move(mFwd, mLine); }
void CTView::OnPageUp() { Move(mBack, mPage); }
void CTView::OnPageDown() { Move(mFwd, mPage); }
void CTView::OnStartFile() { Move(mBack, mFile); }
void CTView::OnEndFile() { Move(mFwd, mFile); }

void CTView::Move(int dir, int amount)
{
    if (dir == mBack) {
        if (m_formatter->AtTop())
            return;
    }
    else {
        if (m_formatter->AtEof())
            return;
    }
    Line     l;
    CFDC     fdc(m_hWnd);
    if (dir == mBack)
        switch (amount) {
        case mLine:
            m_formatter->FormatBack(fdc, m_formatter->GetLine(
                m_formatter->Length() - 1).pos, m_formatter->Top());
            break;
        case mFile:
            PushPos();
            m_formatter->FormatFwd(fdc, m_formatter->Sof());
            break;
        default:
        case mPage:
            m_formatter->FormatBack(fdc, m_formatter->Top(), FilePos());
            break;
    }
    else
        switch (amount) {
        case mLine:
            m_formatter->FormatFwd(fdc, m_formatter->GetLine(1).pos);
            break;
        case mFile:
            PushPos();
            m_formatter->FormatBack(fdc, m_formatter->Eof(), FilePos());
            break;
        default:
        case mPage:
            m_formatter->FormatFwdAdj(fdc);
            break;
    }
    QueueRepaint();

    m_BP.bmkidx = -1;
}

BOOL CTView::OnEraseBkgnd(CDC* pDC)
{
    return FALSE; // we'll erase it in OnPaint
}

class CAboutDialog : public CDialog {
public:
    CAboutDialog(UINT id, CWnd *parent = NULL) : CDialog(id, parent) { }
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM, LPARAM);
    CString   m_info;
    bool     m_finfo;
};

#include "buildnum.h"

#define FILE_INFO_FORMAT \
    _T("File: %s\r\nSize: %d byte(s), %d paragraph(s)\r\n\r\nFormat: %s\r\nEncoding: %s\r\nCompression: %s\r\n\r\nPosition: %d:%d")

#define ABOUT_FORMAT1  \
    _T("This program is based on Haali Reader 2.0b264\r\n\tCopyright (C) 2001-2007 Mike Matsnev.\r\n")

#define ABOUT_FORMAT2  \
    _T("TextViewNG v2.0b%d %s\r\nCopyright (C)\t2001-2007 Mike Matsnev@haali.su\r\n\t\t2012-2014 MakiseKurisu@Github")

BOOL CAboutDialog::OnInitDialog() {
    CDialog::OnInitDialog();
    CString ver;
    ver.Format(ABOUT_FORMAT2, BUILD_NUM, BUILD_DATE);
    SetDlgItemText(IDC_ABOUTVER, ver);
    SetDlgItemText(IDC_ABOUTINFO, m_info);
    m_finfo = true;
    return TRUE;
}

BOOL CAboutDialog::OnCommand(WPARAM wParam, LPARAM lParam) {
    if (LOWORD(wParam) == IDC_ABOUT_TOGGLE) {
        if (m_finfo) {
            SetDlgItemText(IDC_ABOUTINFO, ABOUT_FORMAT1);
            SetDlgItemText(IDC_ABOUT_TOGGLE, _T("File Info"));
        }
        else {
            SetDlgItemText(IDC_ABOUTINFO, m_info);
            SetDlgItemText(IDC_ABOUT_TOGGLE, _T("Copyrights"));
        }
        m_finfo = !m_finfo;
    }
    return CDialog::OnCommand(wParam, lParam);
}

void CTView::OnAppAbout()
{
    CAboutDialog   dlg(IDD_ABOUTBOX, this);
    CString   fmt, enc;
    FilePos   p = CurFilePos();

    if (m_textfile->GetFormat() < 0) {
        // auto
        fmt.Format(_T("Auto, detected: %s"),
            m_textfile->GetFormatName(m_textfile->GetRealFormat()));
    }
    else {
        // set by user
        fmt = m_textfile->GetFormatName(m_textfile->GetFormat());
    }
    if (m_textfile->GetEncoding() < 0) {
        // auto
        enc.Format(_T("Auto, detected: %s"),
            m_textfile->GetEncodingName(m_textfile->GetRealEncoding()));
    }
    else {
        enc = m_textfile->GetEncodingName(m_textfile->GetEncoding());
    }
    dlg.m_info.Format(FILE_INFO_FORMAT,
        (LPCTSTR)m_textfile->Name(), m_textfile->ByteLength(),
        m_textfile->Length(CurFilePos().docid), (LPCTSTR)fmt, (LPCTSTR)enc,
        (LPCTSTR)m_textfile->CompressionInfo(),
        p.para, p.off);
    dlg.DoModal();
}

void CTView::OnUpdateOptions(CCmdUI* pCmdUI) {
    pCmdUI->Enable();
}

void CTView::OnOptions()
{
    COptionsDialog    opt;

    opt.m_bold = m_TextDisp.bold;
    opt.m_cleartype = m_TextDisp.cleartype;
    opt.m_justify = m_TextDisp.justify;
    opt.m_margins = m_TextDisp.margin_width;
    opt.m_size = m_TextDisp.fontsize;
    opt.m_face = m_TextDisp.fontface;
    opt.m_hyphenate = m_TextDisp.hyphenate;
    opt.m_angle = m_TextDisp.angle / 900;
    opt.m_columns = m_Window.columns - 1;

    if (opt.DoModal() == IDOK) {
        // try to apply settings
        bool    update = false;
        if (m_TextDisp.fontface != opt.m_face || m_TextDisp.fontsize != opt.m_size ||
            m_TextDisp.bold != (opt.m_bold != 0) || m_TextDisp.cleartype != opt.m_cleartype ||
            m_TextDisp.angle != opt.m_angle * 900 || m_TextDisp.margin_width != opt.m_margins ||
            m_TextDisp.justify != (opt.m_justify != 0) ||
            m_TextDisp.hyphenate != (opt.m_hyphenate != 0))
        {
            m_TextDisp.hyphenate = (opt.m_hyphenate != 0);
            m_formatter->SetHyphenate(m_TextDisp.hyphenate);
            m_TextDisp.justify = (opt.m_justify != 0);
            m_formatter->SetJustified(m_TextDisp.justify);
            m_TextDisp.angle = opt.m_angle * 900;
            m_TextDisp.margin_width = opt.m_margins;
            m_TextDisp.SetFont(opt.m_face, opt.m_bold != 0, opt.m_size, opt.m_cleartype);
            m_TextDisp.SaveSettings();
            SetRotAngle(m_TextDisp.angle);
            update = true;
        }
        if (m_Window.columns != opt.m_columns + 1)
        {
            m_Window.columns = opt.m_columns + 1;
            m_Window.SaveSettings();
            update = true;
        }
        if (update) {
            CalcSizes();
            QueueRepaint();
        }
    }
}

void CTView::OnUpdateFileformat(CCmdUI* pCmdUI) {
    pCmdUI->Enable(m_formatter->DocId() >= 0);
}

void CTView::OnFileformat()
{
    if (m_formatter->DocId() < 0)
        return;
    CFileFormatDialog fmt;

    fmt.m_format = m_textfile->GetFormat() + 1;
    fmt.m_encoding = m_textfile->GetEncoding() + 1;
    fmt.m_defencoding = CTVApp::GetInt(_T("DefEncoding"), -1) + 1;

    if (fmt.DoModal() == IDOK) {
        if (fmt.m_defencoding != CTVApp::GetInt(_T("DefEncoding"), -1) + 1)
            CTVApp::SetInt(_T("DefEncoding"), fmt.m_defencoding - 1);
        m_textfile->SetFormatEncoding(fmt.m_format - 1, fmt.m_encoding - 1);
        m_formatter->SetTop(FilePos());
        CFDC   fdc(m_hWnd);
        m_formatter->Reformat(fdc);
        QueueRepaint();
        m_Search.matchpos = m_formatter->Eof();
    }
}

void  CTView::PushPos() {
    while (m_History.stacktop) {
        POSITION  tmp = m_History.stacktop;
        m_History.pstack.GetNext(m_History.stacktop);
        m_History.pstack.RemoveAt(tmp);
    }
    FilePos   top = m_formatter->Top();
    m_History.pstack.AddTail(top);
    if (m_History.pstack.GetCount() > 100)
        m_History.pstack.RemoveHead();
}

static CString GetNextFile(const CString& filename, bool fNext) {
    CString     path(filename);
    int      p1 = path.ReverseFind(_T('\\'));
    int      p2 = path.ReverseFind(_T('/'));
    p1 = max(p1, p2) + 1;
    path.Delete(p1, path.GetLength() - p1);
    path += _T("*");

    CString     file(filename);
    file.Delete(0, p1);

    CString     prev, first;
    WIN32_FIND_DATA   fd;
    HANDLE     hFind = ::FindFirstFile(path, &fd);
    BOOL      fFind = hFind != INVALID_HANDLE_VALUE;

    for (; fFind; fFind = ::FindNextFile(hFind, &fd)) {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;

        const TCHAR   *ext = fd.cFileName + _tcslen(fd.cFileName);
        while (ext > fd.cFileName && ext[-1] != _T('.'))
            --ext;
        if (_tcsicmp(ext, _T("jpg")) && _tcsicmp(ext, _T("png")) &&
            _tcsicmp(ext, _T("jpeg")))
            continue;

        if (first.IsEmpty())
            first = fd.cFileName;

        if (fNext) {
            if (file.CompareNoCase(prev) == 0) {
                prev = fd.cFileName;
                break;
            }
        }
        else {
            if (file.CompareNoCase(fd.cFileName) == 0)
                break;
        }
        prev = fd.cFileName;
    }

    if (hFind != INVALID_HANDLE_VALUE)
        ::FindClose(hFind);

    if (!fFind) {
        // looped over all files and didnt find a match
        if (fNext && file.CompareNoCase(prev) == 0)
            prev = first;
    }

    path.Delete(path.GetLength() - 1); // remove last '*'

    return prev.IsEmpty() ? prev : path + prev;
}

static TextFile  *OpenNextImage(const CString& cur, bool fNext) {
    for (CString next(GetNextFile(cur, fNext)); !next.IsEmpty() && next != cur; next = GetNextFile(next, fNext)) {
        TextFile  *tf = TextFile::Open(next);
        if (tf)
            return tf;
    }

    return NULL;
}

void CTView::OnUpdateBack(CCmdUI* pCmdUI) {
    if (m_textfile->IsImage()) {
        pCmdUI->Enable();
        return;
    }

    pCmdUI->Enable(!m_History.pstack.IsEmpty() &&
        m_History.stacktop != m_History.pstack.GetHeadPosition());
}

void CTView::OnBack() {
    if (m_textfile->IsImage()) {
        TextFile  *tf = OpenNextImage(m_textfile->Name(), false);
        if (tf) {
            AfxGetMainWnd()->SetWindowText(FileName(tf->Name()));
            auto_ptr<TextFile> text(tf);
            SetFile(text);
        }
        return;
    }

    if (!m_History.pstack.IsEmpty() &&
        m_History.stacktop != m_History.pstack.GetHeadPosition())
    {
        FilePos   cur = m_formatter->Top();
        if (m_History.stacktop == NULL)
            m_History.stacktop = m_History.pstack.GetTailPosition();
        else
            m_History.pstack.GetPrev(m_History.stacktop);
        MoveAbs(m_History.pstack.GetAt(m_History.stacktop));
        m_History.pstack.SetAt(m_History.stacktop, cur);
    }
}

void  CTView::OnNextSection() {
    Bookmarks& bm = m_textfile->bmk();
    FilePos cur = m_formatter->Bottom();
    if (cur < m_formatter->Eof()) {
        int   index = bm.BFind(cur, Bookmarks::SNEXTCH);
        if (index < bm.GetSize()) {
            if (cur == bm.Ref(index) && index < bm.GetSize())
                ++index;
            if (cur < bm.Ref(index)) {
                PushPos();
                EnsureVisible(bm.Ref(index));
            }
        }
    }
}

void  CTView::OnPrevSection() {
    Bookmarks& bm = m_textfile->bmk();
    FilePos cur = m_formatter->Top();
    if (cur > FilePos()) {
        int   index = bm.BFind(cur, Bookmarks::SPREVCH);
        if (index < bm.GetSize()) {
            if (cur == bm.Ref(index) && index>0)
                do --index; while (index >= 0 && bm.Flags(index)&Bookmarks::BMK);
            if (index < 0)
                OnStartFile();
            else
                if (bm.Ref(index) < cur) {
                    PushPos();
                    EnsureVisible(bm.Ref(index));
                }
        }
    }
}

void  CTView::DisplayBookmarkPopup(int index) {
    Bookmarks& bm = m_textfile->bmk();

    if (bm.Flags(index) & Bookmarks::BMK) {
        POINT pt;
        if (LookupPoint(bm.Ref(index), pt))
            DisplayBookmarkPopup(pt, Unicode::ToWCbuf(bm.Text(index, m_textfile.get())));
    }

    m_BP.bmkidx = index;
}

void  CTView::OnNextBm() {
    Bookmarks& bm = m_textfile->bmk();
    int  index;

    if (m_BP.bmkidx >= 0 && m_BP.bmkidx < bm.GetSize() - 1) {
        index = m_BP.bmkidx + 1;
    }
    else {
        if (m_formatter->AtEof())
            return;

        FilePos cur = m_formatter->Bottom();
        index = bm.BFind(cur, Bookmarks::SNEXTANY);
        if (index >= bm.GetSize())
            return;

        if (cur == bm.Ref(index) && index < bm.GetSize() - 1)
            ++index;

        if (cur >= bm.Ref(index))
            return;
    }

    PushPos();
    EnsureVisible(bm.Ref(index));

    DisplayBookmarkPopup(index);
}

void  CTView::OnPrevBm() {
    Bookmarks& bm = m_textfile->bmk();

    int  index;

    if (m_BP.bmkidx > 0 && m_BP.bmkidx < bm.GetSize()) {
        index = m_BP.bmkidx - 1;
    }
    else {
        if (m_formatter->AtTop())
            return;

        FilePos cur = m_formatter->Top();
        index = bm.BFind(cur, Bookmarks::SPREVANY);
        if (index >= bm.GetSize())
            return;

        if (cur == bm.Ref(index) && index > 0)
            --index;

        if (bm.Ref(index) >= cur)
            return;
    }

    PushPos();
    EnsureVisible(bm.Ref(index));

    DisplayBookmarkPopup(index);
}

void CTView::HandleMouseDown(CPoint point) {
    if (m_BP.visible) {
        HideBookmarkPopup();
        return;
    }

    POINT     pt;
    FilePos   pos;

    pt.x = point.x;
    pt.y = point.y;

    // transform point into virtual coords
    System2Window(pt, m_Window.cli);

    // check if we are in a progress bar
    if (pt.y >= m_Window.rheight - PROGRESS_C && pt.y < m_Window.rheight) {
        if (pt.x < PROGRESS_M + PROGRESS_A) {
            // prev section
            OnPrevSection();
        }
        else if (pt.x >= m_Window.rwidth - PROGRESS_M - PROGRESS_A - m_Window.pb_width &&
            pt.x < m_Window.rwidth - m_Window.pb_width)
        {
            // next section
            OnNextSection();
        }
        else if (pt.x < m_Window.rwidth - PROGRESS_M - PROGRESS_A - m_Window.pb_width) {
            // move absolute
            PushPos();
            int   charpos = MulDiv(
                m_textfile->GetTotalLength(m_formatter->Top().docid),
                pt.x - PROGRESS_M - PROGRESS_A,
                m_Window.rwidth - 2 * PROGRESS_M - 2 * PROGRESS_A - m_Window.pb_width
                );
            int   para = m_textfile->LookupParagraph(m_formatter->DocId(), charpos);
            EnsureVisible(
                FilePos(
                para,
                charpos - m_textfile->GetPStart(m_formatter->DocId(), para),
                m_formatter->DocId()
                )
                );
        }
        else {
            // right part, back in dict mode
            if (m_formatter->DocId() < 0)
                OnBack();
        }
        return;
    }

    if (LookupAddr(pt, pos)) {
        // check if there is a bookmark nearby
        int     bmk = m_textfile->bmk().BFind(pos, Bookmarks::SPREVBMK);
        if (bmk >= 0) {
            FilePos bp(m_textfile->bmk().Ref(bmk));
            if (bp.docid == pos.docid && bp.para == pos.para && bp.off >= pos.off - 2) {
                DisplayBookmarkPopup(pt, Unicode::ToWCbuf(m_textfile->bmk().Text(bmk, m_textfile.get())));
                return;
            }
        }

        // check if it is a link
        Paragraph p(m_textfile->GetParagraph(pos.docid, pos.para));
        for (int link = 0; link<p.links.size(); ++link)
            if (p.links[link].off <= (DWORD)pos.off && p.links[link].off + p.links[link].len>(DWORD) pos.off)
            {
                // found a link
                // for now only local links are supported
                if (p.links[link].target[0] == _T('#')) {
                    FilePos   dest;
                    if (m_textfile->LookupReference(p.links[link].target + 1, dest)) {
                        PushPos();
                        MoveAbs(dest);
                    }
                    else
                        goto beep;
                }
                else
                beep:
                MessageBeep(MB_OK);
                return;
            }
    }

    // didn't have an action yet, interpret as movement
    if (pt.y < (m_Window.rheight - PROGRESS_C) / 2)
        Move(mBack, mPage);
    else
        Move(mFwd, mPage);
}

void CTView::OnLButtonDblClk(UINT nFlags, CPoint point) {
    FilePos   pos;

    System2Window(point, m_Window.cli);

    if (point.y < m_Window.rheight - m_Window.progress_height)
        CTVApp::QueueCmd(ID_FULLSCREEN);
}

void CTView::MoveAbs(FilePos pos) {
    CFDC  fdc(m_hWnd);
    m_formatter->FormatFwd(fdc, pos);
    QueueRepaint();
}

void CTView::EnsureVisible(FilePos pos) {
    CFDC  fdc(m_hWnd);
    if (m_formatter->EnsureVisible(fdc, pos))
        QueueRepaint();
}

void CTView::OnUpdateFind(CCmdUI* pCmdUI) {
    pCmdUI->Enable();
}

void CTView::OnFind() {
    CFindDlg    dlg(this);
    Buffer<wchar_t> sel;
    if (GetSelText(sel))
        dlg.m_text = Unicode::ToCS(sel);
    else
        dlg.m_text = Unicode::ToCS(m_Search.searchstr);
    dlg.m_matchcase = m_Search.matchcase;
    dlg.m_fromcurpos = m_Search.fromcurpos;
    if (dlg.DoModal() == IDOK && dlg.m_text.GetLength() > 0) {
        m_Search.matchpos = dlg.m_fromcurpos ? m_formatter->Top() : m_formatter->Sof();
        m_Search.matchcase = dlg.m_matchcase != 0;
        m_Search.fromcurpos = dlg.m_fromcurpos != 0;
        m_Search.searchstr = Unicode::ToWCbuf(dlg.m_text);
        if (!m_Search.matchcase)
            m_Search.searchstr = Unicode::Lower(m_Search.searchstr);
        CTVApp::QueueCmd(ID_DO_FIND);
    }
}

void CTView::OnFindnext() {
    if (m_Search.searchstr.size() > 0 &&
        m_Search.matchpos + m_Search.searchstr.size() < m_formatter->Eof()) {
        m_Search.matchpos.off += m_Search.searchstr.size();
        DoFind();
    }
}

void CTView::OnUpdateFindnext(CCmdUI* pCmdUI) {
    pCmdUI->Enable(m_Search.searchstr.size() > 0 &&
        m_Search.matchpos + m_Search.searchstr.size() < m_formatter->Eof());
}

static Buffer<int>  kmptable(const Buffer<wchar_t>& s) {
    Buffer<int> b(s.size() + 1);

    if (s.size() > 0) {
        int  i, j;

        i = 0;
        j = b[0] = -1;
        while (i<s.size()) {
            while (j>-1 && s[i] != s[j])
                j = b[j];
            ++i;
            ++j;
            if (i < s.size() && j < s.size() && s[i] == s[j])
                b[i] = b[j];
            else
                b[i] = j;
        }
    }
    return b;
}

static int   kmpfind(const wchar_t *s, int len, int off, const wchar_t *pat,
    int patlen, int *tab)
{
    int   i = 0, j = off;
    while (j<len) {
        while (i>-1 && pat[i] != s[j])
            i = tab[i];
        ++i;
        ++j;
        if (i >= patlen)
            return j - i;
    }
    return -1;
}

void CTView::DoFind() {
    if (m_Search.matchpos.para < m_textfile->Length(m_Search.matchpos.docid)) {
        CWaitCursor wait;
        Buffer<int> tab(kmptable(m_Search.matchcase ? m_Search.searchstr :
            Unicode::Lower(m_Search.searchstr)));
        while (m_Search.matchpos.para < m_textfile->Length(m_Search.matchpos.docid)) {
            Paragraph       para(m_textfile->GetParagraph(m_Search.matchpos.docid,
                m_Search.matchpos.para));
            Buffer<wchar_t> text(m_Search.matchcase ? para.str : Unicode::Lower(para.str));
            int       pp = kmpfind(text, text.size(),
                m_Search.matchpos.off, m_Search.searchstr, m_Search.searchstr.size(), tab);
            if (pp >= 0) {
                m_Search.matchpos.off = pp;
                if (m_Search.matchpos < m_formatter->Top() ||
                    m_Search.matchpos >= m_formatter->Bottom())
                {
                    PushPos();
                    EnsureVisible(m_Search.matchpos);
                }
                SetSelection(m_Search.matchpos, m_Search.searchstr.size());
                QueueRepaint();
                return;
            }
            ++m_Search.matchpos.para;
            m_Search.matchpos.off = 0;
        }
    }
    // didnt find anything
    SetSelection(FilePos(), 0);
    QueueRepaint();
    MessageBeep(MB_OK);
}

void CTView::OnUpdateColors(CCmdUI* pCmdUI) {
    pCmdUI->Enable();
}

void CTView::OnColors() {
    int gamma = CTVApp::GetInt(_T("Gamma"), DEF_GAMMA);
    if (myChooseColors(g_colors, &gamma, this)) {
        SaveColors();
        CTVApp::SetInt(_T("Gamma"), gamma);
        m_textfile->InvalidateImageCache();
        QueueRepaint();
    }
}

void CTView::OnAddBmk() {
    CAddBmDialog   dlg(this);
    FilePos   pos(CurFilePos());
    Buffer<wchar_t> sel;
    if (m_Sel.start.docid == pos.docid && GetSelText(sel))
        pos = m_Sel.start;
    dlg.m_text = Unicode::ToCS(sel);
    if (dlg.DoModal() == IDOK) {
        int nidx = m_textfile->bmk().Add(dlg.m_text, pos);
        if (nidx <= m_BP.bmkidx)
            ++m_BP.bmkidx;
        QueueRepaint();
    }
}

void CTView::OnUpdateAddBmk(CCmdUI* pCmdUI) {
    pCmdUI->Enable(m_formatter->DocId() >= 0);
}

void CTView::OnBookmarks() {
    if (m_textfile->bmk().GetSize() <= 0 && m_textfile->GetSubDocCount() <= 1)
        return;

    CContentsDlg    dlg(m_textfile->bmk(), m_textfile.get(), CurFilePos(), this);
    if (dlg.DoModal() != IDOK)
        return;

    dlg.m_index = m_textfile->bmk().Commit(dlg.m_index);
    if (dlg.m_index >= 0) {
        PushPos();
        EnsureVisible(m_textfile->bmk().Ref(dlg.m_index));
        DisplayBookmarkPopup(dlg.m_index);
    }
    else if (-dlg.m_index <= m_textfile->GetSubDocCount()) {
        PushPos();
        EnsureVisible(FilePos(0, 0, -dlg.m_index - 1));
    }
}

void CTView::OnUpdateBookmarks(CCmdUI* pCmdUI) {
    pCmdUI->Enable(m_textfile->bmk().GetSize() > 0);
}

void CTView::OnKeys() {
    Keys::SetupKeys(this);
}

void CTView::OnUpdateKeys(CCmdUI* pCmdUI) {
    pCmdUI->Enable();
}

void CTView::OnDestroy()
{
    Keys::SetWindow(0);
    KillTimer(m_timer);
    KillTimer(m_UI.timer);
    KillTimer(m_AS.timer);
    KillTimer(m_Window.pd_timer);
    CWnd::OnDestroy();
}

LRESULT CTView::OnHotkey(WPARAM wp, LPARAM lp) {
    UINT cmd;
    if (Keys::TranslateKey(wp, cmd))
        CTVApp::QueueCmd(cmd);
    return 0;
}

void CTView::OnStyles() {
    CStylesDlg dlg(this);

    if (dlg.DoModal() == IDOK) {
        if (dlg.SaveChanges()) {
            //XMLParser::SaveStyles();
            m_textfile->Reparse();
            CFDC   fdc(m_hWnd);
            m_formatter->Reformat(fdc);
            QueueRepaint();
        }
    }
}

void CTView::OnUpdateStyles(CCmdUI* pCmdUI) {
    pCmdUI->Enable();
}

bool CTView::LookupAddr(const POINT& vp, FilePos& p) {
    int   column = vp.x / m_Window.width;
    if (column < 0 || column >= m_Window.columns) {
        if (column < 0)
            p = m_formatter->Top();
        else
            p = m_formatter->Bottom();
        return false;
    }
    int   off = 0;
    for (int col = 0; col < column; ++col)
        off += m_formatter->PageLength(col);
    int   line = 0;
    int   cury = 0;
    if (vp.y < 0) {
        if (m_formatter->PageLength(column)>0)
            p = m_formatter->GetLine(off).pos;
        else
            p = m_formatter->Bottom();
        return false;
    }
    for (; line < m_formatter->PageLength(column); ++line) {
        const Line& l = m_formatter->GetLine(off + line);
        if (vp.y >= cury && vp.y < cury + l.height) {
            // found line
            if (l.flags&Line::image) {
                // don't bother selecting this
                p = l.pos;
                return false;
            }
            int x = vp.x - m_Window.width*column - l.ispace - m_TextDisp.margin_width;
            int curx = 0;
            int sym = 0;
            if (x < 0) {
                p = l.pos;
                return false;
            }
            for (; sym < l.real_len; ++sym) {
                if (x >= curx && x < curx + l.dx[sym]) {
                    // found character
                    p = l.pos;
                    p.off += sym;
                    return true;
                }
                curx += l.dx[sym];
            }
            p = l.pos;
            p.off += sym;
            return false;
        }
        cury += l.height;
    }
    if (m_formatter->PageLength(column) > 0) {
        const Line& l = m_formatter->GetLine(off + m_formatter->PageLength(column) - 1);
        p = l.pos;
        p.off += l.real_len;
    }
    else
        p = m_formatter->Bottom();
    return false;
}

bool  CTView::LookupPoint(FilePos p, POINT& pt) {
    if (p < m_formatter->Top() || p >= m_formatter->Bottom())
        return false;

    for (int column = 0, max = 0, i = 0, x = 0; column < m_Window.columns; ++column, x += m_Window.width) {
        max += m_formatter->PageLength(column);
        for (int y = 0; i < max; ++i) {
            const Line& l1 = m_formatter->GetLine(i);
            const Line& l2 = m_formatter->GetLine(i + 1);
            if (p >= l1.pos && p < l2.pos) {
                x += l1.ispace + m_TextDisp.margin_width;
                int dist = p.off - l1.pos.off;
                for (int n = 0; n < dist; ++n)
                    x += l1.dx[n];
                pt.x = x;
                pt.y = y;
                return true;
            }
            y += l1.height;
        }
    }

    return false;
}

void CTView::OnLButtonDown(UINT nFlags, CPoint point)
{
    StopAS();

    m_Mouse.start = m_Mouse.last = m_Mouse.end = point;
    m_Mouse.trackmouse = true;
    m_Mouse.dmove = m_Sel.len > 0;
    SetCapture();
    TrackMouse();
}

void CTView::OnLButtonUp(UINT nFlags, CPoint point) {
    StopAS();

    if (m_Mouse.trackmouse) {
        m_Mouse.end = point;
        TrackMouse();
        m_Mouse.trackmouse = false;
        ReleaseCapture();
        int len;
        FilePos start;
        CalcSelection(start, len);
        if (!m_Mouse.dmove && !len) // position didnt change
            HandleMouseDown(point);
        else if (len)
            DisplaySelectionMenu(point);
    }
}

class CCustomMenu : public CWnd, public CRotate
{
public:
    CCustomMenu() { }

    void   AppendMenu(const CString& title, UINT id) {
        m_title.Add(title);
        m_id.Add(id);
    }

    UINT   TrackPopupMenu(CWnd *parent, int reqx, int reqy, int angle);

protected:
    enum { FSZ = 13, PAD = 2, BORDER = 1 };

    CStringArray   m_title;
    CUIntArray   m_id;
    int    m_selection, m_fh, m_fa, m_w, m_h;

    void MenuSelect() {
#if 0
        PlaySound(_T("hwandsw"), NULL, SND_ASYNC | SND_NODEFAULT); // XXX hack
#endif
        EndModalLoop(m_selection < 0 ? 0 : m_id[m_selection]);
    }

    DECLARE_MESSAGE_MAP()
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC) { return FALSE; }
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnCaptureChanged(CWnd *pWnd) { EndModalLoop(0); }
    afx_msg void OnKillFocus(CWnd* pNewWnd) { EndModalLoop(0); }
};

BEGIN_MESSAGE_MAP(CCustomMenu, CWnd)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_KEYDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_CAPTURECHANGED()
    ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

void  CCustomMenu::OnMouseMove(UINT nFlags, CPoint point) {
    RECT cli;
    GetClientRect(&cli);
    System2Window(point, cli);
    int sel = -1;
    if (point.x >= BORDER && point.x < m_w - BORDER && point.y >= BORDER && point.y < m_h - BORDER)
        sel = (point.y - BORDER) / m_fh;
    if (sel != m_selection) {
        m_selection = sel;
        InvalidateRect(NULL);
    }
}

void  CCustomMenu::OnLButtonDown(UINT nFlags, CPoint point) {
    RECT cli;
    GetClientRect(&cli);
    System2Window(point, cli);
    if (point.x < BORDER || point.x >= m_w - BORDER || point.y < BORDER || point.y >= m_h - BORDER)
        EndModalLoop(0);
    else {
        m_selection = (point.y - BORDER) / m_fh;
        InvalidateRect(NULL);
    }
}

void  CCustomMenu::OnLButtonUp(UINT nFlags, CPoint point) {
    RECT cli;
    GetClientRect(&cli);
    System2Window(point, cli);
    if (point.x >= BORDER && point.x < m_w - BORDER && point.y >= BORDER && point.y < m_h - BORDER) {
        m_selection = (point.y - BORDER) / m_fh;
        MenuSelect();
    }
}

void CCustomMenu::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
    switch (nChar) {
    case VK_RETURN:
        if (m_selection >= 0)
            MenuSelect();
        break;
    case VK_ESCAPE:
        EndModalLoop(0);
        break;
    case VK_UP:
        if (--m_selection < 0)
            m_selection = m_id.GetUpperBound();
        InvalidateRect(NULL);
        break;
    case VK_DOWN:
        if (++m_selection >= m_id.GetSize())
            m_selection = 0;
        InvalidateRect(NULL);
        break;
    case VK_HOME:
        if (m_selection != 0) {
            m_selection = 0;
            InvalidateRect(NULL);
        }
        break;
    case VK_END:
        if (m_selection != m_id.GetUpperBound()) {
            m_selection = m_id.GetUpperBound();
            InvalidateRect(NULL);
        }
        break;
    }
}

void  CCustomMenu::OnPaint() {
    COLORREF
        cMenuText = ::GetSysColor(COLOR_MENUTEXT),
        cMenu = ::GetSysColor(COLOR_MENU),
        cHlBg = ::GetSysColor(COLOR_HIGHLIGHT),
        cHlText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);

    PAINTSTRUCT ps;
    {
        CFDC     fdc(m_hWnd, &ps);
        RECT     rc, cli;

        GetClientRect(&cli);
        rc.left = BORDER; rc.right = m_w - BORDER;

        fdc.SelectFontAbs((FSZ*GetDeviceCaps(fdc.DC(), LOGPIXELSY)) / 96, CFDC::FORCENORMALWEIGHT | CFDC::FORCETAHOMA);

        for (int i = 0; i < m_title.GetSize(); ++i) {
            if (i == m_selection) {
                fdc.SetTextColor(cHlText);
                fdc.SetBkColor(cHlBg);
            }
            else {
                fdc.SetTextColor(cMenuText);
                fdc.SetBkColor(cMenu);
            }
            rc.top = m_fh*i + BORDER; rc.bottom = rc.top + m_fh;
            const CString& s = m_title[i];
            TDrawText(fdc.DC(), rc.left + PAD, rc.top + PAD, cli, rc, s, s.GetLength(), NULL);
        }

        // paint border now
        fdc.SetColor(RGB(32, 48, 160)); // XXX hardcoded
        POINT   pt[5];
        for (int j = 0; j < BORDER; ++j) {
            pt[0].x = pt[3].x = pt[4].x = cli.left + j;
            pt[0].y = pt[1].y = pt[4].y = cli.top + j;
            pt[1].x = pt[2].x = cli.right - j - 1;
            pt[2].y = pt[3].y = cli.bottom - j - 1;
            Polyline(fdc.DC(), pt, 5);
        }
    }
}

UINT  CCustomMenu::TrackPopupMenu(CWnd *parent, int reqx, int reqy, int angle) {
    SetRotAngle(angle);

    // calculate window size
    m_w = 0;
    {
        // borrow parent's DC
        CFDC   dc(parent->m_hWnd);

        dc.SelectFontAbs((FSZ*GetDeviceCaps(dc.DC(), LOGPIXELSY)) / 96, CFDC::FORCENORMALWEIGHT | CFDC::FORCETAHOMA, true);

        dc.GetFontSize(m_fh, m_fa);
        m_fh += PAD * 2;
        m_h = m_fh*m_title.GetSize();

        for (int i = 0; i < m_title.GetSize(); ++i) {
            SIZE    sz;
            int     nch = m_title[i].GetLength();
            dc.GetTextExtent(m_title[i], nch, 8192, nch, NULL, sz);

            if (sz.cx > m_w)
                m_w = sz.cx;
        }
    }
    m_w += PAD * 2 + BORDER * 2;
    m_h += BORDER * 2;

    int sw = m_w, sh = m_h;

    if (angle == 900 || angle == 2700)
        sw = m_h, sh = m_w;

    RECT rc;
    parent->GetClientRect(&rc);

    int mw = ((rc.right - rc.left) * 7) >> 3;
    int mh = ((rc.bottom - rc.top) * 7) >> 3;

    if (sw > mw)
        sw = mw;
    if (sh > mh)
        sh = mh;

    if (angle == 900 || angle == 2700)
        m_w = sh, m_h = sw;
    else
        m_w = sw, m_h = sh;

    // align window's top left corner with requested point
    int tlx, tly;

    switch (angle) {
    case 900: tlx = reqx; tly = reqy - sh; break;
    case 1800: tlx = reqx - sw; tly = reqy - sh; break;
    case 2700: tlx = reqx - sw; tly = reqy; break;
    default: tlx = reqx; tly = reqy; break;
    }

    if (tlx < rc.left)
        tlx = rc.left;
    if (tly < rc.top)
        tly = rc.top;
    if (tlx + sw > rc.right)
        tlx = rc.right - sw;
    if (tly + sh > rc.bottom)
        tly = rc.bottom - sh;

    rc.left = tlx; rc.top = tly;
    rc.right = tlx + sw; rc.bottom = tly + sh;

    // initialize
    m_selection = -1;

    HWND hFocus = ::GetFocus();

    // create&show window
    Create(NULL, NULL, WS_CHILD, rc, parent, 0);
    ShowWindow(SW_SHOW);

    SetCapture();
    SetFocus();

#if 0
    PlaySound(_T("MenuPop"), NULL, SND_ASYNC | SND_NODEFAULT);
#endif

    int code = RunModalLoop(); // run a nested message loop

    ShowWindow(SW_HIDE);

    ReleaseCapture();

    ::SetFocus(hFocus);

    // cleanup
    DestroyWindow();

    // return command
    return code;
}

void CTView::DisplaySelectionMenu(CPoint point) {
    CCustomMenu menu;

    menu.AppendMenu(_T("Copy"), ID_EDIT_COPY);
    menu.AppendMenu(_T("Add bookmark..."), ID_ADD_BMK);

    UINT code = menu.TrackPopupMenu(this, point.x, point.y, m_TextDisp.angle);

    if (code)
        CTVApp::QueueCmd(code);
}

void CTView::OnMouseMove(UINT nFlags, CPoint point) {
    if (m_Mouse.trackmouse) {
        m_Mouse.end = point;
        TrackMouse();
        m_Mouse.last = point;
    }
}

void CTView::TrackMouse() {
    FilePos start;
    int len;
    CalcSelection(start, len);
    SetSelection(start, len);
    if (abs(m_Mouse.end.x - m_Mouse.start.x) > 4 ||
        abs(m_Mouse.end.y - m_Mouse.start.y) > 4)
        m_Mouse.dmove = true;
}

void CTView::OnMiscopt() {
    CMiscOptDlg dlg;

    dlg.m_allowmulti = CTVApp::GetInt(_T("AllowOnlyOneInstance"), 1) == 0;
    dlg.m_rotb = m_Window.rotbuttons;
    dlg.m_autorepeatlimit = m_Window.autorepeatlimit;
    dlg.m_fcsize = CTVApp::GetInt(_T("FontCacheSize"), DEF_FONTCACHE);
    dlg.m_fbsize = CTVApp::GetInt(_T("FileBufSize"), DEF_FBUF) >> 10;
    dlg.m_savetofiles = CTVApp::GetInt(_T("SaveToFiles"), DEF_SAVETOFILES) != 0;
    dlg.m_lastfiles = CTVApp::GetInt(_T("NumBookmarks"), DEF_BOOKMARKS);
    if (dlg.DoModal() == IDOK) {
        if (dlg.m_allowmulti != (CTVApp::GetInt(_T("AllowOnlyOneInstance"), 1) == 0))
            CTVApp::SetInt(_T("AllowOnlyOneInstance"), !dlg.m_allowmulti);
        if ((dlg.m_rotb != 0) != m_Window.rotbuttons ||
            (dlg.m_autorepeatlimit != 0) != m_Window.autorepeatlimit)
        {
            m_Window.rotbuttons = dlg.m_rotb != 0;
            m_Window.autorepeatlimit = dlg.m_autorepeatlimit != 0;
            m_Window.SaveSettings();
        }
        if (dlg.m_fcsize < 2)
            dlg.m_fcsize = 2;
        if (dlg.m_fcsize > 32)
            dlg.m_fcsize = 32;
        if (dlg.m_fcsize != CTVApp::GetInt(_T("FontCacheSize"), DEF_FONTCACHE)) {
            CTVApp::SetInt(_T("FontCacheSize"), dlg.m_fcsize);
            CFDC::SetCacheSize(dlg.m_fcsize);
        }
        if (dlg.m_fbsize < 8)
            dlg.m_fbsize = 8;
        if (dlg.m_fbsize > 1024)
            dlg.m_fbsize = 1024;
        dlg.m_fbsize <<= 10;
        int   fbs = 8192;
        while ((fbs << 1) <= dlg.m_fbsize)
            fbs <<= 1;
        if (fbs != CTVApp::GetInt(_T("FileBufSize"), DEF_FBUF))
            CTVApp::SetInt(_T("FileBufSize"), fbs);
        if ((dlg.m_savetofiles != 0) != (CTVApp::GetInt(_T("SaveToFiles"), DEF_SAVETOFILES) != 0))
            CTVApp::SetInt(_T("SaveToFiles"), dlg.m_savetofiles != 0);
        if (dlg.m_lastfiles != CTVApp::GetInt(_T("NumBookmarks"), DEF_BOOKMARKS))
            CTVApp::SetInt(_T("NumBookmarks"), dlg.m_lastfiles);
    }
}

void CTView::OnUpdateMiscopt(CCmdUI* pCmdUI) {
    pCmdUI->Enable();
}

void  CTView::CopyToClipboard(const wchar_t *text, int length, HWND hWnd) {
    HGLOBAL   hMem = GlobalAlloc(GMEM_MOVEABLE, sizeof(wchar_t)*(length + 1));
    if (!hMem)
        return;
    wchar_t   *result = (wchar_t*)GlobalLock(hMem);
    if (!result) {
        GlobalFree(hMem);
        return;
    }
    memcpy(result, text, sizeof(wchar_t)*length);
    result[length] = L'\0';

    GlobalUnlock(hMem);

    if (::OpenClipboard(hWnd)) {
        if (EmptyClipboard())
            if (::SetClipboardData(CF_UNICODETEXT, hMem)) {
                CloseClipboard();
                return;
            }
        CloseClipboard();
    }

    GlobalFree(hMem);
}

void CTView::OnEditCopy() {
    // wastes memory, but easy to implement
    Buffer<wchar_t>   str;
    if (GetSelText(str))
        CopyToClipboard(str, str.size(), m_hWnd);
}

void CTView::OnUpdateEditCopy(CCmdUI* pCmdUI) {
    pCmdUI->Enable(m_Sel.len > 0);
}

void CTView::CalcSelection(FilePos& p, int& len) {
    FilePos   start, end;
    POINT     a, b;
    a = m_Mouse.start; b = m_Mouse.end;
    System2Window(a, m_Window.cli);
    System2Window(b, m_Window.cli);
    LookupAddr(a, start);
    LookupAddr(b, end);
    int     dist = m_formatter->Distance(start, end);
    if (dist < 0) {
        p = end;
        len = -dist;
    }
    else {
        p = start;
        len = dist;
    }
}

void CTView::OnRotate() {
    m_TextDisp.angle += 900;
    if (m_TextDisp.angle > 2700)
        m_TextDisp.angle = 0;
    SetRotAngle(m_TextDisp.angle);
    CTVApp::SetInt(_T("Orientation"), m_TextDisp.angle);
    m_TextDisp.SetDefFont();
    CalcSizes();
    QueueRepaint();
}

int   CTView::GetSelParagraphCount() {
    if (m_Sel.len == 0)
        return 0;
    int   len = m_Sel.len;
    int   pcount = 0;
    FilePos p(m_Sel.start);
    FilePos eof(p);
    eof.para = m_textfile->Length(p.docid);
    eof.off = 0;
    while (len > 0 && p < eof) {
        int   plen = m_textfile->GetPLength(p.docid, p.para) - p.off;
        if (plen < 0)
            plen = 0;
        if (plen > len)
            plen = len;
        len -= plen;
        p.off = 0;
        p.para++;
        pcount++;
    }
    return pcount;
}

bool  CTView::GetSelText(Buffer<wchar_t>& str) {
    if (m_Sel.len <= 0)
        return false;

    FilePos p(m_Sel.start);
    FilePos eof(p);
    eof.para = m_textfile->Length(p.docid);
    eof.off = 0;
    int   pl = GetSelParagraphCount();
    int   len = m_Sel.len;

    Buffer<wchar_t>   tmp(len + 2 * pl);
    wchar_t       *bp = tmp;
    Paragraph pp(m_textfile->GetParagraph(p.docid, p.para));
    int   ll = pp.str.size() - p.off;
    if (ll < 0)
        ll = 0;
    if (ll > len)
        ll = len;
    memcpy(bp, pp.str + p.off, ll*sizeof(wchar_t));
    bp += ll;
    len -= ll;
    p.off = 0;
    ++p.para;
    while (len > 0 && p < eof) {
        *bp++ = L'\r'; *bp++ = L'\n';
        pp = m_textfile->GetParagraph(p.docid, p.para);
        ll = len;
        if (ll > pp.str.size())
            ll = pp.str.size();
        memcpy(bp, pp.str, ll*sizeof(wchar_t));
        bp += ll;
        ++p.para;
        len -= ll;
    }
    tmp.setsize(bp - tmp);
    str = tmp;
    return true;
}

void CTView::OnTimer(UINT nIDEvent) {
    switch (nIDEvent) {
    case TM_SAVEINFO:
        SaveInfo();
        break;
    case TM_USERTEXT:
        HideText();
        break;
    case TM_AS:
        StepAS();
        break;
    case TM_PD:
        UpdateProgressBar();
        break;
    }
    CWnd::OnTimer(nIDEvent);
}

void  CTView::MovePercent(int p) {
    if (p >= 0 && p <= 100) {
        FilePos   pp;
        pp.para = p*m_formatter->Eof().para / 100;
        PushPos();
        MoveAbs(pp);
    }
}

static int ParsePosSpec(const CString& s, int& v1, int& v2) {
    int i;

    v1 = v2 = 0;

    for (i = 0; i < s.GetLength(); ++i) {
        if (s[i]<_T('0') || s[i]>_T('9'))
            break;
        v1 = v1 * 10 + s[i] - _T('0');
    }
    if (i == 0)
        return 0;
    if (i >= s.GetLength())
        return 1;
    if (s[i] != _T(':') && s[i] != _T(';'))
        goto end;
    ++i;
    for (; i < s.GetLength(); ++i) {
        if (s[i]<_T('0') || s[i]>_T('9'))
            break;
        v2 = v2 * 10 + s[i] - _T('0');
    }
    if (i >= s.GetLength())
        return 1;
end:
    if (i != s.GetLength() - 1)
        return 0;
    if (s[i] == _T('l') || s[i] == _T('L'))
        return 4;
    if (s[i] == _T('p') || s[i] == _T('P') || s[i] == _T('%'))
        return 3;
    if (s[i] == _T('=') || s[i] == _T('g') || s[i] == _T('G'))
        return 2;
    return 0;
}

bool CTView::CanAddUIChar(TCHAR ch) {
    CFDC dc(m_hWnd);
    dc.SelectFont(USERINPUTSIZE, USERINPUTFLAGS);
    m_UI.inp += ch;
    int nch = m_UI.inp.GetLength();
    SIZE sz;
    dc.GetTextExtent(m_UI.inp, nch, m_UI.rc.right - m_UI.rc.left, nch, NULL, sz);
    m_UI.inp.Delete(m_UI.inp.GetLength() - 1);
    return nch > m_UI.inp.GetLength() && sz.cx < m_UI.rc.right - m_UI.rc.left;
}

void CTView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) {
    if (nChar == VK_BACK) {
        if (m_UI.inp.GetLength() > 0)
            m_UI.inp.Delete(m_UI.inp.GetLength() - 1);
        else {
            // act as back
            CTVApp::QueueCmd(ID_BACK);
            return;
        }
    }
    else if (nChar == VK_TAB) {
        m_UI.inp.Empty();
    }
    else if (nChar == VK_SPACE) {
        UINT  cmd;
        if (m_UI.inp.GetLength() > 0) {
            if (CanAddUIChar(' '))
                m_UI.inp += _T(' ');
        }
        else if (Keys::TranslateKey(nChar, cmd, m_Window.rotbuttons ? m_TextDisp.angle : 0)) {
            // repeated key
            if (nFlags & 0x4000 && CTVApp::TopQueuedCmd() == cmd)
                return;
            CTVApp::QueueCmd(cmd);
            return;
        }
    }
    else {
        if (nChar == '/' && m_UI.inp.GetLength() == 0) {
            CTVApp::QueueCmd(ID_FIND);
            return;
        }
        if (nChar >= 32 && CanAddUIChar((TCHAR)nChar))
            m_UI.inp += (TCHAR)nChar;
        else
            return;
    }
    StopAS();
    if (m_UI.inp.GetLength() > 0) {
        // check if this is a valid pos spec
        FilePos   p(m_formatter->Top());
        switch (ParsePosSpec(m_UI.inp, p.para, p.off)) {
        case 1: // partial spec
            break;
        case 2: // complete spec, abs pos
            PushPos();
            m_formatter->AdjustPos(p);
            MoveAbs(p);
            m_UI.inp.Empty();
            HideText();
            return;
        case 3: // complete spec, percentage
            MovePercent(p.para);
            m_UI.inp.Empty();
            HideText();
            return;
        case 4: // complete spec, virt. page
            MoveAbs(FilePos(m_textfile->LookupParagraph(m_formatter->DocId(), p.para << 11), 0, m_formatter->DocId()));
            m_UI.inp.Empty();
            HideText();
            return;
        case 0: // not a valid spec at all, try lookup
            if (m_formatter->DocId() < 0) {
                // inside dictionary
                Buffer<wchar_t>   tmp(Unicode::ToWCbufZ(m_UI.inp));
                if (m_textfile->LookupDict(tmp, p)) {
                    MoveAbs(p);
                    m_Dict.lastdictlookup = tmp;
                }
            }
            break;
        }
        ShowText();
    }
    else
        HideText();
}

FilePos   CTView::CurFilePos() {
    FilePos   p = m_formatter->Top();
    if (p.docid == -1) {
        // skip dictionary positions
        POSITION sp = m_History.stacktop;
        if (sp)
            m_History.pstack.GetPrev(sp);
        else
            sp = m_History.pstack.GetTailPosition();
        while (sp && p.docid == -1)
            p = m_History.pstack.GetPrev(sp);
    }
    if (p.docid == -1)
        return FilePos();
    return p;
}

void CTView::OnForward() {
    if (m_textfile->IsImage()) {
        TextFile  *tf = OpenNextImage(m_textfile->Name(), true);
        if (tf) {
            AfxGetMainWnd()->SetWindowText(FileName(tf->Name()));
            auto_ptr<TextFile> text(tf);
            SetFile(text);
        }
        return;
    }

    if (m_History.stacktop != NULL) {
        FilePos   next = m_History.pstack.GetAt(m_History.stacktop);
        FilePos   top = m_formatter->Top();
        m_History.pstack.SetAt(m_History.stacktop, top);
        m_History.pstack.GetNext(m_History.stacktop);
        MoveAbs(next);
    }
}

void CTView::OnUpdateForward(CCmdUI* pCmdUI) {
    pCmdUI->Enable(m_textfile->IsImage() || m_History.stacktop != NULL);
}

void  CTView::ShowText() {
    if (m_UI.timer)
        KillTimer(m_UI.timer);
    m_UI.timer = SetTimer(TM_USERTEXT, 2000, NULL);
    m_UI.visible = true;
    // check for popup
    if (m_BP.visible)
        QueueRepaint(m_UI.rc);
    else {
        // otherwise repaint directly
        CFDC fdc(m_hWnd);
        PaintUserInput(fdc, m_UI.rc, m_Window.cli, Unicode::ToWCbuf(m_UI.inp));
    }
}

void  CTView::HideText() {
    if (!m_UI.visible)
        return;
    if (m_UI.timer) {
        KillTimer(m_UI.timer);
        m_UI.timer = 0;
    }
    m_UI.visible = false;
    QueueRepaint(m_UI.rc);
}

void  CTView::OnOK() {
    // rollback to the most recent !dict view
    if (m_formatter->DocId() < 0 && !m_History.pstack.IsEmpty()) {
        // inside dict
        POSITION  p = m_History.stacktop;
        FilePos   cur(m_formatter->Top());
        while (cur.docid < 0 && p != m_History.pstack.GetHeadPosition()) {
            if (p == NULL)
                p = m_History.pstack.GetTailPosition();
            else
                m_History.pstack.GetPrev(p);
            FilePos np(m_History.pstack.GetAt(p));
            m_History.pstack.SetAt(p, cur);
            cur = np;
        }
        m_History.stacktop = p;
        MoveAbs(cur);
    }
}

void  CTView::FormatBookmarkPopup(CFDC& dc, const POINT& spot) {
    // format the text using maximum width
    int width = m_Window.width - 21; // XXX
    // don't make it too wide
    if (width > 400) // XXX
        width = 400;
    int height = m_Window.height - 21;
    m_formatter->FormatPlainText(dc, width, height, -2,
        m_BP.text, m_BP.text.size(), m_BP.lines);
    // now width and height contain real require w&h, and lines hold formatted text
    // however sizes do not include margins
    // add a couple of extra pixels for frame
    width += FRAME_SIZE * 2;
    height += 1 + FRAME_SIZE;
    // we need to center the popup around the spot now
    int left = spot.x - (width >> 1);
    int top = spot.y - (height >> 1);
    // and check that it doesnt appear out of bounds
    if (left < 10)
        left = 10;
    if (top < 10)
        top = 10;
    if (left + width > m_Window.rwidth - 10)
        left = m_Window.rwidth - 10 - width;
    if (top + height > m_Window.rheight - 10)
        top = m_Window.rheight - 10 - height;
    // and store the thing
    m_BP.rc.left = left;
    m_BP.rc.top = top;
    m_BP.rc.right = left + width;
    m_BP.rc.bottom = top + height;
}

void  CTView::DisplayBookmarkPopup(const POINT& spot, const Buffer<wchar_t>& text) {
    if (m_BP.visible) // repaint if we already have a bookmark
        InvalidateRect(m_BP.rc);
    m_BP.text = text;
    CFDC dc(m_hWnd);
    FormatBookmarkPopup(dc, spot);
    m_BP.visible = true;
    PaintBookmarkPopup(dc, m_BP.rc, m_Window.cli);
}

void  CTView::Advance(FilePos& p, int len) {
    while (len > 0 && p.para < m_textfile->Length(p.docid)) {
        int pl = m_textfile->GetPLength(p.docid, p.para);
        if (p.off + len >= pl) {
            len -= pl - p.off;
            p.off = 0;
            p.para++;
        }
        else {
            p.off += len;
            len = 0;
        }
    }
}

void  CTView::InvalidateRange(const FilePos& x, const FilePos& y) {
    if (y <= x || y <= m_formatter->GetLine(0).pos) // completely out of range
        return;
    int nlines = m_formatter->Length();
    int column = 0;
    int line = 0;
    while (line < nlines) {
        int   col_last = line + m_formatter->PageLength(column);
        int   ystart = -1, yend = 0;
        int   cury = 0;
        while (line < col_last) {
            const Line&   ll = m_formatter->GetLine(line);
            FilePos     le(ll.pos);
            le.off += ll.real_len;
            if (x < le && ll.pos < y) {
                // in range
                if (ystart < 0)
                    ystart = cury;
                yend = cury + ll.height;
            }
            cury += ll.height;
            ++line;
        }
        if (ystart >= 0) {
            RECT    rc;
            rc.left = column*m_Window.width;
            rc.right = rc.left + m_Window.width;
            rc.top = ystart;
            rc.bottom = yend;
            InvalidateRect(rc);
        }
        ++column;
    }
}

void  CTView::SetSelection(const FilePos& p, int len) {
    if (p != m_Sel.start || len != m_Sel.len) {
        if (m_formatter->SetHighlight(p, len)) {
            FilePos e(p);
            Advance(e, len);
            FilePos ce(m_Sel.start);
            Advance(ce, m_Sel.len);
            if (e <= m_Sel.start || ce <= p) {
                // do not intersect, invalidate both
                InvalidateRange(p, e);
                InvalidateRange(m_Sel.start, ce);
            }
            else {
                // avoid updating intersection
                if (p < m_Sel.start)
                    InvalidateRange(p, m_Sel.start);
                else
                    InvalidateRange(m_Sel.start, p);
                if (ce < e)
                    InvalidateRange(ce, e);
                else
                    InvalidateRange(e, ce);
            }
        }
        m_Sel.start = p;
        m_Sel.len = len;
    }
}

void  CTView::InvalidateRect(const RECT& rc) {
    RECT tmp = rc;
    Window2System(tmp, m_Window.cli);
    CWnd::InvalidateRect(&tmp, FALSE);
}

void  CTView::QueueRepaint(const RECT& rc) {
    HideBookmarkPopup();
    InvalidateRect(rc);
}

void  CTView::HideBookmarkPopup() {
    if (m_BP.visible) {
        InvalidateRect(m_BP.rc);
        m_BP.visible = false;
    }
}

BOOL CTView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) {
    // just pretend it's up/down
    zDelta *= 5;
    if (zDelta > 0) {
        while (zDelta > 0) {
            OnLineUp();
            zDelta -= WHEEL_DELTA;
        }
    }
    else if (zDelta < 0) {
        while (zDelta < 0) {
            OnLineDown();
            zDelta += WHEEL_DELTA;
        }
    }
    return TRUE;
}

void  CTView::StartAS() {
    if (m_AS.timer)
        return; // already running

    if (m_BP.visible || m_UI.visible) {
        HideBookmarkPopup();
        HideText();
        CTVApp::QueueCmd(ID_AS_START);
        return;
    }

    if (m_AS.column < 0 || m_AS.line < 0 || m_AS.top_pos != m_formatter->Top()) {
        // try to start near the end of page
        int   tc = m_formatter->Length() - 5;
        if (tc < 0)
            tc = 0;
        m_AS.column = 0;
        while (tc > m_formatter->PageLength(m_AS.column))
            tc -= m_formatter->PageLength(m_AS.column++);
        m_AS.line = tc;
    }
    m_AS.top_pos = m_formatter->Top();
    m_AS.timer = SetTimer(TM_AS, m_AS.delay / 1000, NULL);

    if (m_Window.showprogress() && m_AS.timer && m_Window.pb.as_delay)
        RedrawProgressBar();
}

void  CTView::StopAS() {
    if (!m_AS.timer)
        return;

    KillTimer(m_AS.timer);

    // repaint everything
    QueueRepaint();

    m_AS.timer = 0;
    m_AS.column = m_AS.line = -1;
}

void  CTView::RestartASTimer() {
    if (m_AS.timer) {
        KillTimer(m_AS.timer);
        m_AS.timer = SetTimer(TM_AS, m_AS.delay / 1000, NULL);
    }
}

void  CTView::StepAS() {
    // only perform steps if the view has focus
    if (::GetFocus() != m_hWnd) {
        KillTimer(m_AS.timer);
        return;
    }

    // cancel AS if user paged through the file on his own
    if (m_AS.top_pos != m_formatter->Top()) {
        StopAS();
        return;
    }

    // currently highlighted line is in m_AS
    // here we a) repaint it, b) advance it, and c) underline the next line

    // repaint
    if (m_AS.column >= 0 && m_AS.column < m_Window.columns &&
        m_AS.line >= 0 && m_AS.line < m_formatter->PageLength(m_AS.column))
    {
        PaintSingleLine(m_AS.column, m_AS.line, CLR_DEFAULT);
        // advance
        if (++m_AS.line >= m_formatter->PageLength(m_AS.column)) {
            m_AS.line = 0;
            if (++m_AS.column >= m_Window.columns) {
                CFDC  dc(m_hWnd);
                if (!m_formatter->FormatFwdAdj(dc)) {
                    StopAS();
                    return;
                }
                m_AS.top_pos = m_formatter->Top();
                m_AS.column = 0;
            }
        }
    }
    else
        // somehow the position was completely invalid, start from top
        m_AS.line = m_AS.column = 0;

    // underline
    PaintSingleLine(m_AS.column, m_AS.line, C_AS);
}

void  CTView::FasterAS() {
    if (m_AS.delay < 50000)
        return;

    m_AS.delay -= 25000; // 25ms
    RestartASTimer();
    m_AS.SaveSettings();
    UpdateProgressBar();
}

void  CTView::SlowerAS() {
    m_AS.delay += 25000; // 25ms
    RestartASTimer();
    m_AS.SaveSettings();
    UpdateProgressBar();
}

void  CTView::FasterASFine() {
    if (m_AS.delay < 50000)
        return;

    m_AS.delay -= 5000; // 5ms
    RestartASTimer();
    m_AS.SaveSettings();
    UpdateProgressBar();
}

void  CTView::SlowerASFine() {
    m_AS.delay += 5000; // 5ms
    RestartASTimer();
    m_AS.SaveSettings();
    UpdateProgressBar();
}

BOOL CTView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
    if (nCode == CN_COMMAND || nCode == CN_EVENT) {
        if (nID != ID_AS_START && nID != ID_AS_STOP && nID != ID_AS_FASTER &&
            nID != ID_AS_SLOWER && nID != ID_AS_FASTER_FINE &&
            nID != ID_AS_SLOWER_FINE && nID != ID_AS_TOGGLE)
            StopAS();
    }

    return CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CTView::OnKillFocus(CWnd* pNewWnd)
{
    CWnd::OnKillFocus(pNewWnd);

    KillTimer(m_AS.timer); // stop timer, but don't clear values
}

void CTView::OnSetFocus(CWnd* pOldWnd)
{
    CWnd::OnSetFocus(pOldWnd);

    RestartASTimer();
}

void CTView::ToggleAS() {
    if (m_AS.timer)
        StopAS();
    else
        StartAS();
}

void CTView::OnTogglePB() {
    m_Window.showprog = !m_Window.showprog;
    m_Window.SaveSettings();
    QueueRepaint();
    StartWindowPDTimer();
}
void CTView::OnTogglePBChapter() {
    m_Window.pb.chapter = !m_Window.pb.chapter;
    m_Window.SaveSettings();
    RedrawProgressBar();
    StartWindowPDTimer();
}
void CTView::OnTogglePBPos() {
    m_Window.pb.position = !m_Window.pb.position;
    m_Window.SaveSettings();
    RedrawProgressBar();
    StartWindowPDTimer();
}
void CTView::OnTogglePBTop() {
    m_Window.pb.top = !m_Window.pb.top;
    m_Window.SaveSettings();
    RedrawProgressBar();
    StartWindowPDTimer();
}
void CTView::OnTogglePBAS() {
    m_Window.pb.as_delay = !m_Window.pb.as_delay;
    m_Window.SaveSettings();
    RedrawProgressBar();
    StartWindowPDTimer();
}
void CTView::OnTogglePBTime() {
    m_Window.pb.time = !m_Window.pb.time;
    m_Window.SaveSettings();
    RedrawProgressBar();
    StartWindowPDTimer();
}
void CTView::OnTogglePBBat() {
    m_Window.pb.battery = !m_Window.pb.battery;
    m_Window.SaveSettings();
    RedrawProgressBar();
    StartWindowPDTimer();
}
void CTView::OnUpdateTogglePB(CCmdUI* pCmdUI) {
    pCmdUI->SetCheck(m_Window.showprog);
    pCmdUI->Enable();
}
void CTView::OnUpdateTogglePBPos(CCmdUI* pCmdUI) {
    pCmdUI->SetCheck(m_Window.pb.position);
    pCmdUI->Enable();
}
void CTView::OnUpdateTogglePBChapter(CCmdUI* pCmdUI) {
    pCmdUI->SetCheck(m_Window.pb.chapter);
    pCmdUI->Enable();
}
void CTView::OnUpdateTogglePBTop(CCmdUI* pCmdUI) {
    pCmdUI->SetCheck(m_Window.pb.top);
    pCmdUI->Enable();
}
void CTView::OnUpdateTogglePBAS(CCmdUI* pCmdUI) {
    pCmdUI->SetCheck(m_Window.pb.as_delay);
    pCmdUI->Enable();
}
void CTView::OnUpdateTogglePBTime(CCmdUI* pCmdUI) {
    pCmdUI->SetCheck(m_Window.pb.time);
    pCmdUI->Enable();
}
void CTView::OnUpdateTogglePBBat(CCmdUI* pCmdUI) {
    pCmdUI->SetCheck(m_Window.pb.battery);
    pCmdUI->Enable();
}

void CTView::OnUpdateExportBmk(CCmdUI* pCmdUI) {
    pCmdUI->Enable();
}
void CTView::OnExportBmk() {
    CString filename;

    OPENFILENAME ofn;

    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);

    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFilter = _T("Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0");
    ofn.lpstrDefExt = _T("txt");
    ofn.nMaxFile = 1024;
    ofn.lpstrFile = filename.GetBuffer(ofn.nMaxFile);
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

    if (!GetSaveFileName(&ofn))
        return;

    filename.ReleaseBuffer();

    SaveInfo();

    Bookmarks::ExportAllBookmarks(filename);
}

void CTView::OnNewColorProfile() {
    CString pn;
    if (GetUserInput(_T("Profile name:"), _T("New color profile"), pn, this)) {
        SaveColors();

        // allocate new profile id and switch to it
        int     id;
        CString name;
        HKEY    hKey = AfxGetApp()->GetSectionKey(_T("Colors"));

        for (id = 1;; ++id) {
            name.Format(_T("%d"), id);
            if (RegQueryValueEx(hKey, name, 0, 0, 0, 0) != ERROR_SUCCESS)
                break;
        }

        RegCloseKey(hKey);

        AfxGetApp()->WriteProfileString(_T("Colors"), name, pn);
        AfxGetApp()->WriteProfileInt(_T("Colors"), _T("Profile"), id);

        LoadColors();
        SaveColors();
        QueueRepaint();
    }
}

void CTView::OnUpdateNewColorProfile(CCmdUI *pCmdUI) {
    pCmdUI->Enable();
}

void CTView::OnDelColorProfile() {
    int id = g_color_profile;

    if (id == 0 || !NextColorProfile())
        return;

    HKEY hKey = AfxGetApp()->GetSectionKey(_T("Colors"));
    if (hKey == NULL)
        return;

    CString name;
    name.Format(_T("%d"), id);
    RegDeleteValue(hKey, name);

    for (int i = 0; g_colors[i].name; ++i) {
        name.Format(_T("%s_%d"), g_colors[i].name, id);
        RegDeleteValue(hKey, name);
    }

    RegCloseKey(hKey);

    QueueRepaint();
}

void CTView::OnUpdateDelColorProfile(CCmdUI *pCmdUI) {
    pCmdUI->Enable(g_color_profile != 0);
}

void CTView::OnSelColor(UINT cmd) {
    AfxGetApp()->WriteProfileInt(_T("Colors"), _T("Profile"), cmd - COLORS_BASE);
    LoadColors();
    QueueRepaint();
}

void CTView::OnUpdateSelColor(CCmdUI *pCmdUI) {
    pCmdUI->Enable();
}

void CTView::OnUpdateNextColorProfile(CCmdUI *pCmdUI) {
    pCmdUI->Enable();
}

void CTView::OnNextColorProfile() {
    if (NextColorProfile())
        QueueRepaint();
}