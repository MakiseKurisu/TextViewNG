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
* $Id: Rotate.h,v 1.1.2.2 2003/10/12 15:11:21 mike Exp $
*
*/

#ifndef ROTATE_H
#define ROTATE_H

class CRotate {
public:
    CRotate() : m_hCompatDC(NULL) { SetRotAngle(0); }
    ~CRotate() { if (m_hCompatDC) ::DeleteDC(m_hCompatDC); }

    void Window2System(POINT& pt, const RECT& cli) { m_tp(pt, cli); }
    void Window2System(RECT& rc, const RECT& cli) { m_tr(rc, cli); }
    void System2Window(POINT& pt, const RECT& cli) { m_rtp(pt, cli); }
    void System2Window(RECT& rc, const RECT& cli);

    void SetRotAngle(int angle);

    // drawing services
    void  TDrawText(HDC dc, int x, int y, const RECT& cli, const RECT& r,
        const wchar_t *p, int lp, const int *dx, int flags = ETO_OPAQUE);

    void TDrawLine(HDC dc, const RECT& cli, int x1, int y1, int x2, int y2);
    void TDrawLine(HDC dc, const RECT& cli, const RECT& rc);

    // warning, this will transform the coordinates in place!
    void TDrawPolyLine(HDC dc, const RECT& cli, POINT *pt, int npt);

    void TDrawBitmap(HDC dc, HBITMAP hBmp, int x, int y, int sh,
        const RECT& cli, int bmw, int bmh, int bmy);

private:
    // transforms
    typedef void(*TransformRectType)(RECT& rc, const RECT& cli);
    typedef void(*TransformPointType)(POINT& pt, const RECT& cli);

    TransformRectType m_tr;
    TransformPointType m_tp;
    TransformPointType m_rtp;
    int   m_angle;
    HDC   m_hCompatDC;
};

#endif