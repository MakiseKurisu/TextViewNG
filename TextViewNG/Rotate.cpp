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
* $Id: Rotate.cpp,v 1.1.2.3 2003/10/12 15:11:21 mike Exp $
*
*/

#pragma warning(disable:4100)
#define _WIN32_WINNT	_WIN32_WINNT_MAXVER

#include <afxwin.h>
#include "Rotate.h"

static void Transform_R_0(RECT& r, const RECT& cli) {
    // do nothing
}

static void Transform_R_90(RECT& r, const RECT& cli) {
    int	  tmp = r.left;
    r.left = r.top;
    r.top = cli.bottom - r.right;
    r.right = r.bottom;
    r.bottom = cli.bottom - tmp;
}

static void Transform_R_180(RECT& r, const RECT& cli) {
    int	tmp = r.left;
    r.left = cli.right - r.right;
    r.right = cli.right - tmp;
    tmp = r.top;
    r.top = cli.bottom - r.bottom;
    r.bottom = cli.bottom - tmp;
}

static void Transform_R_270(RECT& r, const RECT& cli) {
    int	tmp = r.left;
    r.left = cli.right - r.bottom;
    r.bottom = r.right;
    r.right = cli.right - r.top;
    r.top = tmp;
}

static void Transform_P_0(POINT& pt, const RECT& cli) {
    // do nothing
}

static void Transform_P_90(POINT& pt, const RECT& cli) {
    int tmp = pt.x;
    pt.x = pt.y;
    pt.y = cli.bottom - tmp - 1;
}

static void Transform_P_180(POINT& pt, const RECT& cli) {
    pt.x = cli.right - pt.x;
    pt.y = cli.bottom - pt.y;
}

static void Transform_P_270(POINT& pt, const RECT& cli) {
    int tmp = pt.x;
    pt.x = cli.right - pt.y;
    pt.y = tmp;
}

static void R_Transform_P_0(POINT& pt, const RECT& cli) {
    // do nothing
}

static void R_Transform_P_90(POINT& pt, const RECT& cli) {
    int	tmp = pt.x;
    pt.x = cli.bottom - pt.y;
    pt.y = tmp;
}

static void R_Transform_P_180(POINT& pt, const RECT& cli) {
    pt.x = cli.right - pt.x;
    pt.y = cli.bottom - pt.y;
}

static void R_Transform_P_270(POINT& pt, const RECT& cli) {
    int	tmp = pt.x;
    pt.x = pt.y;
    pt.y = cli.right - tmp;
}

void  CRotate::SetRotAngle(int angle) {
    m_angle = angle;
    switch (angle) {
    case 2700:
        m_tr = Transform_R_270;
        m_tp = Transform_P_270;
        m_rtp = R_Transform_P_270;
        break;
    case 1800:
        m_tr = Transform_R_180;
        m_tp = Transform_P_180;
        m_rtp = R_Transform_P_180;
        break;
    case 900:
        m_tr = Transform_R_90;
        m_tp = Transform_P_90;
        m_rtp = R_Transform_P_90;
        break;
    default:
        m_tr = Transform_R_0;
        m_tp = Transform_P_0;
        m_rtp = R_Transform_P_0;
        break;
    }
}

void CRotate::System2Window(RECT& rc, const RECT& cli) {
    POINT	p1, p2;
    p1.x = rc.left; p1.y = rc.top;
    p2.x = rc.right; p2.y = rc.bottom;
    System2Window(p1, cli);
    System2Window(p2, cli);
    if (p1.x < p2.x) {
        rc.left = p1.x;
        rc.right = p2.x;
    }
    else {
        rc.left = p2.x;
        rc.right = p1.x;
    }
    if (p1.y < p2.y) {
        rc.top = p1.y;
        rc.bottom = p2.y;
    }
    else {
        rc.top = p2.y;
        rc.bottom = p1.y;
    }
}

void CRotate::TDrawText(HDC dc, int x, int y, const RECT& cli, const RECT& r,
    const wchar_t *p, int lp, const int *dx, int flags)
{
    RECT	  tmp = r;
    POINT	  pt;
    pt.x = x;
    pt.y = y;
    Window2System(tmp, cli);
    Window2System(pt, cli);
    // adjust org a bit
    switch (m_angle) {
    case 900: --pt.x; break;
    case 1800: ++pt.y; break;
    case 2700: ++pt.x; break;
    }
    ExtTextOutW(dc, pt.x, pt.y, flags, &tmp, p, lp, dx);
}

void  CRotate::TDrawLine(HDC dc, const RECT& cli, int x1, int y1, int x2, int y2) {
    POINT	pt[2];
    pt[0].x = x1; pt[0].y = y1;
    pt[1].x = x2; pt[1].y = y2;
    Window2System(pt[0], cli);
    Window2System(pt[1], cli);
    Polyline(dc, pt, 2);
}

void  CRotate::TDrawLine(HDC dc, const RECT& cli, const RECT& rc) {
    POINT	pt[2];
    pt[0].x = rc.left; pt[0].y = rc.top;
    pt[1].x = rc.right; pt[1].y = rc.bottom;
    Window2System(pt[0], cli);
    Window2System(pt[1], cli);
    Polyline(dc, pt, 2);
}

void  CRotate::TDrawPolyLine(HDC dc, const RECT& cli, POINT *pt, int npt) {
    for (int i = 0; i < npt; ++i)
        Window2System(pt[i], cli);
    Polyline(dc, pt, npt);
}

// bitmaps support in windows gdi is horrible
void  CRotate::TDrawBitmap(HDC dc, HBITMAP hBmp, int x, int y, int sh,
    const RECT& cli, int bmw, int bmh, int bmy)
{
    int	  u, v, z, t, rw, rh;
    switch (m_angle) {
    case 2700:
        u = bmh - bmy - sh;
        v = 0;
        z = cli.right - y - sh;
        t = x;
        rw = sh;
        rh = bmw;
        break;
    case 1800:
        u = 0;
        v = bmh - bmy - sh;
        z = cli.right - x - bmw;
        t = cli.bottom - y - sh;
        rw = bmw;
        rh = sh;
        break;
    case 900:
        u = bmy;
        v = 0;
        z = y;
        t = cli.bottom - x - bmw;
        rw = sh;
        rh = bmw;
        break;
    default:
        u = 0;
        v = bmy;
        z = x;
        t = y;
        rw = bmw;
        rh = sh;
        break;
    }
    if (!m_hCompatDC)
        m_hCompatDC = CreateCompatibleDC(dc);
    HGDIOBJ tempbm = SelectObject(m_hCompatDC, hBmp);
    BitBlt(dc, z, t, rw, rh, m_hCompatDC, u, v, SRCCOPY);
    SelectObject(m_hCompatDC, tempbm);
}
