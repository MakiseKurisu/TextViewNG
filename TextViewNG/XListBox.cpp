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
* $Id: XListBox.cpp,v 1.1.2.33 2004/10/21 14:53:27 mike Exp $
*
*/

#define _WIN32_WINNT _WIN32_WINNT_MAXVER

#include <afxwin.h>
#include <afxcmn.h>

#include "resource.h"
#include "config.h"
#include "XListBox.h"

#define DEFAULT_FONT_SIZE 11
#define UICON_PAD 1
#define COLOR_BLACK RGB(0,0,0)
#define COLOR_LIGHT_BLUE RGB(166,218,255)
#define COLOR_GRAY RGB(128,128,128)
#define COLOR_WHITE RGB(255,255,255)
#define COLOR_BLUE RGB(0,0,255)

#define INDENT_PER_LEVEL 5

// string compare
#define CmpI(s1,s2) \
    (::CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, \
    (s1), -1, (s2), -1) - 2)

struct XLBItem
{
    TCHAR *text1;
    TCHAR *text2;
    int icon_index;
    int level;
    int flags;
    LONG user_data;
};

#define XIF_COLLAPSED 1

struct XLB
{
    HWND hWnd;

    int num_items;
    int max_items;
    XLBItem *items;

    int visible_items;
    XLBItem **vitems;

    int selection; // selection is I
    int top_offset;
    int scroll_page;

    int item_height;
    int line_height;

    HIMAGELIST icons;
    bool icons_shared;

    HFONT font;
    int dots_width;

    HIMAGELIST tree_icons;

    int flags;

    XLB_GetText gtf;
    void *gtdata;

    int uicon_w;
    int ticon_w;
};

#define XLF_TREE 1

static bool XLB_GrowList(XLB *x)
{
    int incr = x->num_items;

    if (incr < 16)
        incr = 16;
    if (incr > 4096)
        incr = 4096;

    int newsize = x->num_items + incr;

    XLBItem *newitems = (XLBItem*) realloc(x->items, newsize*sizeof(XLBItem));
    if (newitems == NULL)
        return false;

    x->items = newitems;

    XLBItem **newvitems = (XLBItem**) realloc(x->vitems, newsize*sizeof(XLBItem*));
    if (newvitems == NULL)
        return false;

    x->vitems = newvitems;

    x->max_items = newsize;

    return true;
}

static void XLB_UpdateVisibleItems(XLB *x)
{
    XLBItem *ii = x->items;
    XLBItem *jj = ii + x->num_items;
    XLBItem **kk = x->vitems;

    int level = 0;
    int flags = 0;

    while (ii<jj)
    {
        if (ii->level > level)
            flags = XLF_TREE;

        level = ii->level;

        *kk++ = ii;

        if (ii++->flags & XIF_COLLAPSED)
        {
            // skip all following children
            while (ii<jj && ii->level>level)
            {
                ++ii;
                flags = XLF_TREE;
            }
        }
    }

    x->visible_items = kk - x->vitems;

    x->flags |= flags;
}

static int XLB_GetIFromV(XLB *x, int v)
{
    return x->vitems[v] - x->items;
}

static int XLB_GetVFromI(XLB *x, int i)
{
    int top = x->visible_items - 1;
    int bottom = 0;
    XLBItem *ii = &x->items[i];
    XLBItem **kk = x->vitems;

    while (bottom <= top)
    {
        int middle = (bottom + top) >> 1;

        if (kk[middle] == ii)
            return middle;

        if (kk[middle] > ii)
            top = middle - 1;
        else
            bottom = middle + 1;
    }

    return -1;
}

static int XLB_GetParent(XLB *x, int i)
{
    XLBItem *jj = x->items;
    XLBItem *ii = jj + i;
    int level = ii->level;

    while (ii-- > jj)
        if (ii->level < level)
            return ii - jj;

    return -1;
}

static void XLB_UpdateScrollbar(XLB *x);

static void XLB_Restore(XLB *x, int item)
{
    bool changed = false;

    for (;;)
    {
        item = XLB_GetParent(x, item);

        if (item < 0)
            break;

        if (x->items[item].flags & XIF_COLLAPSED)
        {
            x->items[item].flags &= ~XIF_COLLAPSED;
            changed = true;
        }
    }

    if (changed)
        XLB_UpdateVisibleItems(x);
}

static bool XLB_HasChildren(XLB *x, int item)
{
    return item < x->num_items - 1 && x->items[item + 1].level > x->items[item].level;
}

#define SPC(x) ((x)==_T(' ') || (x)==_T('\t') || (x)==_T('\r') || (x)==_T('\n'))

static void XLB_PaintItem(XLB *x, int item, int vitem, HDC hDC, RECT& rc)
{
    // cache an item pointer
    XLBItem *ii = &x->items[item];

    // query text1 and text2 if they are NULL
    const TCHAR *text1 = ii->text1;
    const TCHAR *text2 = ii->text2;

    if (text1 == NULL && x->gtf)
    {
        CString t(x->gtf(x->gtdata, 0, item, ii->user_data));
        text1 = ii->text1 = _tcsdup(t);
    }

    if (text2 == NULL && x->gtf)
    {
        CString t(x->gtf(x->gtdata, 1, item, ii->user_data));
        text2 = ii->text2 = _tcsdup(t);
    }

    // set text and bg colors
    COLORREF text1_color, text2_color, bk_color;
    if (item == x->selection)
    {
        text1_color = COLOR_WHITE;
        text2_color = COLOR_WHITE;
        bk_color = COLOR_BLUE;
    }
    else
    {
        text1_color = COLOR_BLACK;
        text2_color = COLOR_GRAY;
        if (vitem & 1)
        {
            bk_color = COLOR_WHITE;
        }
        else
        {
            bk_color = COLOR_LIGHT_BLUE;
        }
    }
    ::SetBkColor(hDC, bk_color);

    // calculate indentation and paint it
    int indent = ii->level*INDENT_PER_LEVEL;
    if (indent > (rc.right - rc.left) / 2)
        indent = (rc.right - rc.left) / 2;

    RECT rci = rc;
    rci.right = rci.left;

    if (indent)
        rci.right += indent;

    if (x->flags & XLF_TREE)
        rci.right += x->ticon_w;

    if (ii->icon_index >= 0)
        rci.right += x->uicon_w;

    if (rci.right != rci.left)
        ::ExtTextOut(hDC, rci.left, rci.top, ETO_OPAQUE, &rci, NULL, 0, NULL);

    // top and bottom text rectangles
    RECT rt, rb;
    rt = rc;
    rt.bottom = rt.top + x->line_height;
    rt.left += indent;
    rb = rt;
    rb.top += x->line_height;
    rb.bottom += x->line_height;

    // paint a tree icon
    if (x->flags & XLF_TREE)
    {
        // tree mode
        // reduce text width
        rci = rc;
        rci.left = rt.left;
        rci.right = rci.left + x->ticon_w;

        rt.left += x->ticon_w;
        rb.left += x->ticon_w;

        // paint the icon itself
        ::ImageList_DrawEx(x->tree_icons,
            XLB_HasChildren(x, item) ? (ii->flags & XIF_COLLAPSED ? 0 : 1) : 2,
            hDC,
            rci.left, rci.top, 0, 0,
            bk_color, CLR_NONE,
            ILD_NORMAL);
    }

    // paint an icon
    if (ii->icon_index >= 0)
    {
        // calc icon rectangle
        rci = rc;
        rci.left += rt.left;
        rci.right = rci.left + x->uicon_w;

        // reduce text width by icon size
        rt.left += x->uicon_w;
        rb.left += x->uicon_w;

        // paint the icon itself
        ::ImageList_DrawEx(x->icons,
            ii->icon_index,
            hDC,
            rci.left + UICON_PAD, rci.top + UICON_PAD, 0, 0,
            bk_color, CLR_NONE,
            ILD_NORMAL);
    }

    // paint text2
    if (text2)
    {
        int tl = _tcslen(text2);

        // get text2 width
        SIZE sz;
        ::GetTextExtentPoint(hDC, text2, tl, &sz);

        // calc text box
        rci = rb;

        // adjust bottom rectangle
        if (sz.cx < rb.right - rb.left)
            rb.right -= sz.cx;
        else
            rb.right = rb.left;
        rb.right -= 5;

        if (rb.right < rb.left)
            rb.right = rb.left;

        rci.left = rb.right;

        // paint text
        ::SetTextColor(hDC, text2_color);
        ::ExtTextOut(hDC, rci.left + 3, rci.top, ETO_OPAQUE | ETO_CLIPPED, &rci, text2, tl, NULL);
    }

    int tl = text1 ? _tcslen(text1) : 0;

    // get size of top part
    SIZE sz;
    int top_len = 0;
    ::GetTextExtentExPoint(hDC, text1, tl, rt.right - rt.left, &top_len, NULL, &sz);

    // do some simple word wrapping
    int i;
    for (i = 0; i < top_len; ++i)
        if (text1[i] == _T('\n') || text1[i] == _T('\r'))
            break;
    top_len = i;
    if (top_len != tl && !SPC(text1[top_len]))
    {
        while (i > 0 && !SPC(text1[i - 1]))
            --i;
        if (i > 0)
            top_len = i;
    }

    // back off until spaces end
    while (top_len > 0 && SPC(text1[top_len - 1]))
        --top_len;

    // set text1 color
    ::SetTextColor(hDC, text1_color);

    // paint top part
    ::ExtTextOut(hDC, rt.left, rt.top, ETO_OPAQUE | ETO_CLIPPED, &rt, text1, top_len, NULL);

    // skip spaces here
    while (top_len < tl && SPC(text1[top_len]))
        ++top_len;

    // paint bottom part
    if (rb.right > rb.left)
    {
        // check size
        ::GetTextExtentExPoint(hDC, text1 + top_len, tl - top_len, rb.right - rb.left, &i, NULL, &sz);
        if (i != tl - top_len)
        {
            // overflow, will need to append "..."
            ::GetTextExtentExPoint(hDC, text1 + top_len, tl - top_len, rb.right - rb.left - x->dots_width, &i, NULL, &sz);
            ::GetTextExtentPoint(hDC, text1 + top_len, i, &sz);
        }
        ::ExtTextOut(hDC, rb.left, rb.top, ETO_OPAQUE | ETO_CLIPPED, &rb, text1 + top_len, i, NULL);
        if (i != tl - top_len)
            ::ExtTextOut(hDC, rb.left + sz.cx, rb.top, 0, NULL, _T("..."), 3, NULL);
    }
}

static void XLB_PaintAll(XLB *x, HDC hDC)
{
    // load tree icons if we are in tree mode
    if (x->flags & XLF_TREE && !x->tree_icons)
    {
        x->tree_icons = ::ImageList_LoadBitmap(
            ::AfxGetResourceHandle(),
            MAKEINTRESOURCE(IDB_TREE),
            13, // XXX hardcoded
            0,
            RGB(255, 0, 255));
        int dummy;
        ::ImageList_GetIconSize(x->tree_icons, &x->ticon_w, &dummy);
    }

    // select our font
    HGDIOBJ obj = NULL;
    if (x->font)
        obj = ::SelectObject(hDC, x->font);

    // get client rect
    RECT cli;
    ::GetClientRect(x->hWnd, &cli);

    // get update rect
    RECT dirty;
    if (!::GetUpdateRect(x->hWnd, &dirty, FALSE))
        dirty = cli;

    // shift update rect to (0,0)
    dirty.top -= cli.top;
    dirty.bottom -= cli.top;
    dirty.left -= cli.left;
    dirty.right -= cli.left;

    // repaint all items from top to bottom
    int top = (x->top_offset + dirty.top) / x->item_height;
    int bottom = (x->top_offset + dirty.bottom - 1) / x->item_height;
    cli.top += top*x->item_height - x->top_offset;
    for (int i = top; i <= bottom; ++i, cli.top += x->item_height)
    {
        cli.bottom = cli.top + x->item_height;
        if (i >= 0 && i < x->visible_items)
            XLB_PaintItem(x, XLB_GetIFromV(x, i), i, hDC, cli);
        else
        {
            // draw a blank rectangle
            ::SetBkColor(hDC, COLOR_WHITE);
            ::ExtTextOut(hDC, cli.left, cli.top, ETO_OPAQUE, &cli, NULL, 0, NULL);
        }
    }

    // restore font
    if (obj)
        ::SelectObject(hDC, obj);
}

static void XLB_InitFont(XLB *x, HDC hDC)
{
    LOGFONT lf;

    memset(&lf, 0, sizeof(lf));
    lf.lfHeight = -(DEFAULT_FONT_SIZE*GetDeviceCaps(hDC, LOGPIXELSY)) / 96;
    lf.lfWeight = FW_BOLD;
    lf.lfCharSet = ANSI_CHARSET;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;
    _tcscpy_s(lf.lfFaceName, LF_FACESIZE, _T("Tahoma"));

    x->font = ::CreateFontIndirect(&lf);

    TEXTMETRIC tm;
    HGDIOBJ obj = ::SelectObject(hDC, x->font);
    ::GetTextMetrics(hDC, &tm);
    SIZE sz;
    ::GetTextExtentPoint(hDC, _T("..."), 3, &sz);
    ::SelectObject(hDC, obj);

    x->line_height = tm.tmAscent + tm.tmDescent;
    x->item_height = x->line_height * 2;
    x->dots_width = sz.cx;
}

static void XLB_ScrollTo(XLB *x, int top)
{
    int delta = x->top_offset - top;

    x->top_offset = top;

    ::SetScrollPos(x->hWnd, SB_VERT, top, TRUE);

    ::ScrollWindowEx(x->hWnd, 0, delta, NULL, NULL, NULL, NULL, SW_INVALIDATE);
}

static void XLB_EnsureVisible2(XLB *x, int item)
{
    int v = XLB_GetVFromI(x, item);

    int cvis = x->scroll_page / x->item_height; // completely visible items
    if (cvis == 0)
        cvis = 1;

    int item_offset = v*x->item_height - x->top_offset;
    // if the item is not fully visible, then
    // scroll and repaint completely
    if (item_offset < 0)
    {
        x->top_offset = v*x->item_height;
        ::SetScrollPos(x->hWnd, SB_VERT, x->top_offset, TRUE);
    }
    else if (item_offset + x->item_height > x->scroll_page)
    {
        x->top_offset = (v - cvis + 1)*x->item_height;
        ::SetScrollPos(x->hWnd, SB_VERT, x->top_offset, TRUE);
    }
}

static LRESULT CALLBACK XLB_WndProc(HWND hWnd, UINT uMsg,
    WPARAM wParam, LPARAM lParam)
{
    XLB *x = (XLB*) ::GetWindowLong(hWnd, 0);
    int i, j;
    HDC hDC;

    switch (uMsg)
    {
    case WM_CREATE:
        x = (XLB*) malloc(sizeof(*x));
        if (x == NULL)
            return -1;
        memset(x, 0, sizeof(*x));
        ::SetWindowLong(hWnd, 0, (LONG) x);

        x->hWnd = hWnd;
        x->selection = -1;

        hDC = ::GetDC(hWnd);
        XLB_InitFont(x, hDC);
        ::ReleaseDC(hWnd, hDC);

        break;

    case WM_DESTROY:
        if (x->icons && !x->icons_shared)
            ::ImageList_Destroy(x->icons);
        if (x->tree_icons)
            ::ImageList_Destroy(x->tree_icons);
        if (x->font)
            ::DeleteObject(x->font);
        for (i = 0; i < x->num_items; ++i)
        {
            free(x->items[i].text1);
            free(x->items[i].text2);
        }
        free(x->items);
        free(x->vitems);
        free(x);
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        hDC = ::BeginPaint(hWnd, &ps);
        XLB_PaintAll(x, hDC);
        ::EndPaint(hWnd, &ps);
        break; }

    case WM_VSCROLL:
        i = -1;
        j = x->visible_items*x->item_height - x->scroll_page;
        switch (LOWORD(wParam))
        {
        case SB_BOTTOM:
            i = j;
            break;
        case SB_LINEDOWN:
            i = min(x->top_offset + x->item_height, j);
            break;
        case SB_LINEUP:
            i = max(x->top_offset - x->item_height, 0);
            break;
        case SB_PAGEDOWN:
            i = min(x->top_offset + x->scroll_page, j);
            break;
        case SB_PAGEUP:
            i = max(x->top_offset - x->scroll_page, 0);
            break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK: {
            SCROLLINFO si;
            si.cbSize = sizeof(si);
            si.fMask = SIF_TRACKPOS;
            ::GetScrollInfo(hWnd, SB_VERT, &si);
            i = min(si.nTrackPos, j);
            break; }
        case SB_TOP:
            i = 0;
            break;
        }
        if (i >= 0 && i <= j && i != x->top_offset)
            XLB_ScrollTo(x, i);
        break;

    case WM_KEYDOWN:
        i = -1;
        j = XLB_GetVFromI(x, x->selection);
        switch (wParam)
        {
        case VK_RIGHT: case VK_ADD:
            // expand item
            if (x->flags & XLF_TREE && x->selection >= 0 &&
                x->items[x->selection].flags & XIF_COLLAPSED &&
                XLB_HasChildren(x, x->selection))
            {
                x->items[x->selection].flags &= ~XIF_COLLAPSED;
                XLB_UpdateVisibleItems(x);
                XLB_UpdateScrollbar(x);
                ::InvalidateRect(x->hWnd, NULL, FALSE);
            }
            return 0;
        case VK_LEFT: case VK_SUBTRACT:
            // collapse item
            if (x->flags & XLF_TREE && x->selection >= 0 &&
                !(x->items[x->selection].flags & XIF_COLLAPSED) &&
                XLB_HasChildren(x, x->selection))
            {
                x->items[x->selection].flags |= XIF_COLLAPSED;
                XLB_UpdateVisibleItems(x);
                XLB_UpdateScrollbar(x);
                ::InvalidateRect(x->hWnd, NULL, FALSE);
            }
            return 0;
        case VK_UP:
            i = j - 1;
            break;
        case VK_DOWN:
            i = j + 1;
            break;
        case VK_PRIOR:
            i = max(j - (x->scroll_page + x->item_height - 1) / x->item_height, 0);
            break;
        case VK_NEXT:
            i = min(j + (x->scroll_page + x->item_height - 1) / x->item_height, x->visible_items - 1);
            break;
        case VK_HOME:
            i = 0;
            break;
        case VK_END:
            i = x->visible_items - 1;
            break;
        }
        if (i >= 0 && i < x->visible_items && i != j)
        {
            j = i*x->item_height - x->top_offset;
            // if the item is not fully visible, then
            // scroll and repaint completely
            if (j<0 || j + x->item_height>x->scroll_page)
            {
                x->selection = XLB_GetIFromV(x, i);
                XLB_EnsureVisible2(x, x->selection);
                ::InvalidateRect(x->hWnd, NULL, FALSE);
            }
            else
                XLB_SetSelection(hWnd, XLB_GetIFromV(x, i));
        }
        break;

    case WM_LBUTTONDOWN:
        i = (HIWORD(lParam) + x->top_offset) / x->item_height;
        if (i >= 0 && i < x->visible_items)
        {
            j = XLB_GetIFromV(x, i);
            XLB_SetSelection(hWnd, j);

            // if we are in tree mode and hit a control icon,
            // then collapse/uncollapse an item
            if (x->flags & XLF_TREE && x->tree_icons)
            {
                int iw = 0, ih = 0;
                ::ImageList_GetIconSize(x->tree_icons, &iw, &ih);

                if (LOWORD(lParam) < x->items[j].level*INDENT_PER_LEVEL + iw &&
                    XLB_HasChildren(x, j))
                {
                    x->items[j].flags ^= XIF_COLLAPSED;
                    XLB_UpdateVisibleItems(x);
                    XLB_UpdateScrollbar(x);
                    ::InvalidateRect(x->hWnd, NULL, FALSE);
                }
            }
        }
        break;

    case WM_LBUTTONUP:
        i = (HIWORD(lParam) + x->top_offset) / x->item_height;
        if (i >= 0 && i < x->visible_items && x->selection >= 0)
            ::SendMessage(::GetParent(hWnd), XLM_CLICK, lParam, (LPARAM) hWnd);
        break;

    case WM_RBUTTONDOWN:
        i = (HIWORD(lParam) + x->top_offset) / x->item_height;
        if (i >= 0 && i < x->visible_items)
        {
            XLB_SetSelection(hWnd, XLB_GetIFromV(x, i));
            ::SendMessage(::GetParent(hWnd), XLM_CONTEXTMENU, lParam, (LPARAM) hWnd);
        }
        break;

    case WM_LBUTTONDBLCLK:
        i = (HIWORD(lParam) + x->top_offset) / x->item_height;
        if (i >= 0 && i < x->visible_items)
            ::SendMessage(::GetParent(hWnd), XLM_DBLCLK, lParam, (LPARAM) hWnd);
        break;

    case WM_GETDLGCODE:
        return DLGC_WANTARROWS | DLGC_WANTCHARS;

#if defined(WM_MOUSEWHEEL) && defined(SPI_GETWHEELSCROLLLINES) && defined(WHEEL_DELTA)
    case WM_MOUSEWHEEL: {
        UINT lines = 3;
        ::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &lines, 0);
        int scrl = (short) HIWORD(wParam) * (int) lines * x->item_height;
        scrl /= WHEEL_DELTA;
        j = x->visible_items*x->item_height - x->scroll_page;
        i = max(min(x->top_offset - scrl, j), 0);
        if (i != x->top_offset)
            XLB_ScrollTo(x, i);
        break; }
#endif

    default:
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

void XLB_SetImageList(HWND hWnd, HIMAGELIST hIml, bool shared)
{
    XLB *x = (XLB*) ::GetWindowLong(hWnd, 0);
    if (!x)
        return;

    if (x->icons && !x->icons_shared)
        ::ImageList_Destroy(x->icons);

    x->icons = hIml;
    x->icons_shared = shared;

    if (hIml)
    {
        int dummy;
        ::ImageList_GetIconSize(hIml, &x->uicon_w, &dummy);
        x->uicon_w += UICON_PAD * 2;
    }
    else
        x->uicon_w = 0;
}

void XLB_UpdateScrollbar(XLB *x)
{
    RECT rc;
    ::GetClientRect(x->hWnd, &rc);

    x->scroll_page = rc.bottom - rc.top;

    SCROLLINFO si;
    memset(&si, 0, sizeof(si));
    si.cbSize = sizeof(si);

    si.fMask = SIF_PAGE | SIF_RANGE;
    si.nMin = 0;
    si.nMax = x->visible_items*x->item_height - 1;
    if (si.nMax < x->scroll_page)
    {
        si.nMax = 0;
        si.nPage = 0;
    }
    else
        si.nPage = x->scroll_page;
    ::SetScrollInfo(x->hWnd, SB_VERT, &si, TRUE);
}

struct XLB_Handle *XLB_GetHandle(HWND hWnd)
{
    return (XLB_Handle *) ::GetWindowLong(hWnd, 0);
}

bool XLB_AppendItem(struct XLB_Handle *handle,
    const TCHAR *text1, const TCHAR *text2,
    int icon, int level,
    LONG user_data)
{
    XLB *x = (XLB*) handle;

    if (!x)
        return false;

    if (x->num_items >= x->max_items && !XLB_GrowList(x))
        return false;

    XLBItem *ii = &x->items[x->num_items];

    ii->text1 = ii->text2 = NULL;

    if (text1)
    {
        ii->text1 = _tcsdup(text1);
        if (ii->text1 == NULL)
            return false;
    }

    if (text2)
    {
        ii->text2 = _tcsdup(text2);
        if (ii->text2 == NULL)
        {
            free(ii->text1);
            return false;
        }
    }

    ii->icon_index = icon;
    ii->level = level;
    ii->user_data = user_data;
    ii->flags = 0;

    x->num_items++;

    return true;
}

int XLB_GetSelection(HWND hWnd)
{
    XLB *x = (XLB*) ::GetWindowLong(hWnd, 0);

    if (!x)
        return -1;

    return x->selection;
}

static void XLB_GetItemRect(XLB *x, int item, RECT& r)
{
    memset(&r, 0, sizeof(r));

    if (item < 0 || item >= x->num_items)
        return;

    item = XLB_GetVFromI(x, item);

    if (item < 0)
        return;

    ::GetClientRect(x->hWnd, &r);

    r.top += x->item_height*item;
    r.top -= x->top_offset;
    r.bottom = r.top + x->item_height;
}

static void XLB_InvalidateItem(XLB *x, int item)
{
    RECT ri;

    XLB_GetItemRect(x, item, ri);
    ::InvalidateRect(x->hWnd, &ri, FALSE);
}

void XLB_SetSelection(HWND hWnd, int sel)
{
    XLB *x = (XLB*) ::GetWindowLong(hWnd, 0);

    if (!x || sel < 0 || sel >= x->num_items || sel == x->selection)
        return;

    if (XLB_GetVFromI(x, sel) < 0)
    {
        XLB_Restore(x, sel);
        XLB_UpdateScrollbar(x);
    }

    if (x->selection != -1)
        XLB_InvalidateItem(x, x->selection);
    if (sel != -1)
        XLB_InvalidateItem(x, sel);

    x->selection = sel;
}

int XLB_GetItemCount(HWND hWnd)
{
    XLB *x = (XLB*) ::GetWindowLong(hWnd, 0);

    if (!x)
        return 0;

    return x->num_items;
}

LONG XLB_GetData(HWND hWnd, int item)
{
    XLB *x = (XLB*) ::GetWindowLong(hWnd, 0);

    if (!x || item < 0 || item >= x->num_items)
        return 0;

    return x->items[item].user_data;
}

void XLB_EnsureVisible(HWND hWnd, int item, bool middle)
{
    XLB *x = (XLB*) ::GetWindowLong(hWnd, 0);

    if (!x || item < 0 || item >= x->num_items)
        return;

    int v = XLB_GetVFromI(x, item);

    if (v < 0)
    {
        // item is not currently visble
        XLB_Restore(x, item);
        XLB_UpdateScrollbar(x);
        v = XLB_GetVFromI(x, item);
    }

    int cvis = x->scroll_page / x->item_height; // completely visible items
    if (cvis == 0)
        cvis = 1;

    if (middle)
    {
        // scroll the view so the item is in the middle
        int new_offset = (v - cvis / 2)*x->item_height;
        if (new_offset + x->scroll_page > x->visible_items*x->item_height)
            new_offset = x->visible_items*x->item_height - x->scroll_page;
        if (new_offset < 0)
            new_offset = 0;

        if (x->top_offset != new_offset)
            XLB_ScrollTo(x, new_offset);
        return;
    }

    int item_offset = v*x->item_height - x->top_offset;
    // if the item is not fully visible, then
    // scroll and repaint completely
    if (item_offset < 0)
        XLB_ScrollTo(x, v*x->item_height);
    else if (item_offset + x->item_height > x->scroll_page)
        XLB_ScrollTo(x, (v - cvis + 1)*x->item_height);
}

// TODO: avoid unneeded redraws
void XLB_DeleteItem(HWND hWnd, int item)
{
    XLB *x = (XLB*) ::GetWindowLong(hWnd, 0);

    if (!x || item < 0 || item >= x->num_items)
        return;

    free(x->items[item].text1);
    free(x->items[item].text2);

    memmove(&x->items[item], &x->items[item + 1], sizeof(XLBItem) *(x->num_items - item - 1));

    x->num_items--;

    if (x->selection >= x->num_items)
        x->selection = x->num_items - 1;

    XLB_UpdateVisibleItems(x);

    XLB_EnsureVisible(hWnd, x->selection);
    ::InvalidateRect(hWnd, NULL, FALSE);
    XLB_UpdateScrollbar(x);
}

void XLB_SetItemText1(HWND hWnd, int item, const TCHAR *text)
{
    XLB *x = (XLB*) ::GetWindowLong(hWnd, 0);

    if (!x || item < 0 || item >= x->num_items)
        return;

    free(x->items[item].text1);
    x->items[item].text1 = _tcsdup(text);

    XLB_InvalidateItem(x, item);
}

void XLB_DeleteAllItems(HWND hWnd)
{
    XLB *x = (XLB*) ::GetWindowLong(hWnd, 0);

    if (!x)
        return;

    for (int i = 0; i < x->num_items; ++i)
    {
        free(x->items[i].text1);
        free(x->items[i].text2);
    }

    x->num_items = 0;
    x->visible_items = 0;
    x->selection = -1;
    x->top_offset = 0;
    ::SetScrollPos(hWnd, SB_VERT, 0, TRUE);
    XLB_UpdateScrollbar(x);
}

void XLB_UpdateState(HWND hWnd)
{
    XLB *x = (XLB*) ::GetWindowLong(hWnd, 0);

    if (!x)
        return;

    XLB_UpdateVisibleItems(x);
    XLB_UpdateScrollbar(x);
}

void XLB_CollapseLevel(HWND hWnd, int level)
{
    XLB *x = (XLB*) ::GetWindowLong(hWnd, 0);

    if (!x)
        return;

    if (level < 0)
        level = 0x7fffffff; // XXX should be maxint

    XLBItem *ii = x->items;
    XLBItem *jj = ii + x->num_items;

    while (ii < jj)
    {
        if (ii->level >= level)
            ii->flags |= XIF_COLLAPSED;
        else
            ii->flags &= ~XIF_COLLAPSED;
        ++ii;
    }

    if (x->top_offset >= x->visible_items)
        x->top_offset = 0; // XXX can do something better

    XLB_UpdateState(hWnd);
    ::InvalidateRect(hWnd, NULL, FALSE); // XXX can be avoided sometimes
}

// custom sort function
static int compare_items(const void *v1, const void *v2)
{
    const XLBItem *i1 = (XLBItem*) v1;
    const XLBItem *i2 = (XLBItem*) v2;

    int i = i1->user_data - i2->user_data;
    if (i != 0)
        return i;
    bool e1 = !i1->text1 || !*i1->text1;
    bool e2 = !i2->text1 || !*i2->text1;
    if (e1)
        return e2 ? 0 : -1;
    if (e2)
        return 1;
    return CmpI(i1->text1, i2->text1);
}

void XLB_SortItems(HWND hWnd)
{
    XLB *x = (XLB*) ::GetWindowLong(hWnd, 0);

    if (!x)
        return;

    qsort(x->items, x->num_items, sizeof(XLBItem), compare_items);
}

const TCHAR *XLB_GetItemText1(HWND hWnd, int item)
{
    XLB *x = (XLB*) ::GetWindowLong(hWnd, 0);

    if (!x || item < 0 || item >= x->num_items)
        return NULL;

    return x->items[item].text1;
}

void XLB_SetGTFunc(HWND hWnd, XLB_GetText fn, void *ugtdata)
{
    XLB *x = (XLB*) ::GetWindowLong(hWnd, 0);

    if (!x)
        return;

    x->gtf = fn;
    x->gtdata = ugtdata;
}


void XLB_Init()
{
    WNDCLASS wc;

    memset(&wc, 0, sizeof(wc));

    wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = XLB_WndProc;
    wc.cbWndExtra = sizeof(XLB*);
    wc.hInstance = ::GetModuleHandle(NULL);
    // assume the SDK always defines this via macro
#if defined(LoadCursor)
    wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
#endif
    wc.lpszClassName = _T("XListBox");

    ::RegisterClass(&wc);
}