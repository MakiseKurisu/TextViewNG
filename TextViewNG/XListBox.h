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
* $Id: XListBox.h,v 1.1.2.8 2003/07/09 19:50:13 mike Exp $
*
*/

#ifndef XLISTBOX_H
#define XLISTBOX_H

#define XLM_CONTEXTMENU (WM_USER+1)
#define XLM_DBLCLK (WM_USER+2)
#define XLM_CLICK (WM_USER+3)

void XLB_Init();
void XLB_SetImageList(HWND hWnd, HIMAGELIST hIml, bool shared = false);

struct XLB_Handle;
struct XLB_Handle *XLB_GetHandle(HWND hWnd);
bool XLB_AppendItem(struct XLB_Handle *handle,
 const TCHAR *text1, const TCHAR *text2,
 int icon, int level,
 LONG user_data);
void XLB_DeleteItem(HWND hWnd, int item);
void XLB_SetItemText1(HWND hWnd, int item, const TCHAR *text);
LONG XLB_GetData(HWND hWnd, int item);
int XLB_GetSelection(HWND hWnd);
void XLB_SetSelection(HWND hWnd, int sel);
int XLB_GetItemCount(HWND hWnd);
void XLB_EnsureVisible(HWND hWnd, int item, bool middle = false);
void XLB_DeleteAllItems(HWND hWnd);
const TCHAR *XLB_GetItemText1(HWND hWnd, int item);
void XLB_CollapseLevel(HWND hWnd, int level);

void XLB_UpdateState(HWND hWnd);

typedef CString(*XLB_GetText)(void *ugtdata, int num, int item, LONG data);
void XLB_SetGTFunc(HWND hWnd, XLB_GetText fn, void *ugtdata);

// the sort routine is specially tailored to
// control use in FileOpenDialog
void XLB_SortItems(HWND hWnd);

#endif