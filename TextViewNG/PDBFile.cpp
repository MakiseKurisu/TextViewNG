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
* $Id: PDBFile.cpp,v 1.13.2.5 2004/09/29 08:27:12 mike Exp $
*
*/

#define _WIN32_WINNT _WIN32_WINNT_MAXVER

#include <afxwin.h>

#include "ptr.h"
#include "PDBFile.h"
#include "TextViewNG.h"

#include <string.h>

// pdb header
struct PDBHdr {
 char name[32];
 WORD attr;
 WORD ver;
 DWORD ctime;
 DWORD mtime;
 DWORD btime;
 DWORD mnum;
 DWORD appinfoid;
 DWORD aortinfoid;
 char type[4];
 char creator[4];
 DWORD idseed;
 DWORD nextreclist;
 WORD numrec;
};
#define PDBHDRSIZE 78

// record 0
struct PDBRec0 {
 WORD ver;
 WORD res1;
 DWORD usize;
 WORD nrec;
 WORD rsize;
 DWORD res2;
};
#define PDBR0SIZE 16

static DWORD dword(const DWORD& dw) {
 // convert from BE
 BYTE *b = (BYTE*)&dw;
 return ((DWORD)b[0] << 24) | ((DWORD)b[1] << 16) | ((DWORD)b[2] << 8) | b[3];
}

static WORD word(const WORD& w) {
 // convert from BE
 BYTE *b = (BYTE*)&w;
 return ((WORD)b[0] << 8) | b[1];
}

bool PDBFile::CheckPDB(RFile *fp, PDBHdr& hdr, PDBRec0& r0) {
 // we want to access original RFile methods, not the overridden
 // ones
 fp->RFile::seek(0);
 if (fp->RFile::read2(&hdr, PDBHDRSIZE) != PDBHDRSIZE)
 return false;
 if (memcmp(hdr.type, "TEXt", 4) || memcmp(hdr.creator, "REAd", 4))
 return false;
 DWORD off0;
 if (fp->RFile::read2(&off0, 4) != 4)
 return false;
 fp->RFile::seek(dword(off0));
 if (fp->RFile::read2(&r0, PDBR0SIZE) != PDBR0SIZE)
 return false;
 if (word(r0.ver) != 1 && word(r0.ver) != 2 && word(r0.ver) != 257 && word(r0.nrec) > 0)
 return false;
 return true;
}

static DWORD scan_size(BYTE *src, DWORD len) {
 DWORD ulen = 0;

 while (len--) {
 if (*src >= 1 && *src <= 8) {
 DWORD k = *src++;
 while (k-- && len--)
 src++, ulen++;
 }
 else if (*src <= 0x7f)
 src++, ulen++;
 else if (*src >= 0xc0)
 src++, ulen += 2;
 else if (len) {
 DWORD k = *src++;
 k <<= 8; k |= *src++;
 --len;
 ulen += (k & 7) + 3;
 }
 }
 return ulen;
}

PDBFile::PDBFile(const CString& fn) :
RFile(fn), m_length(0), m_ptr(0)
{
 PDBHdr hdr;
 PDBRec0 r0;
 if (Reopen() && CheckPDB(this, hdr, r0)) {
 m_comp = word(r0.ver) == 2;
 m_rsz = word(r0.rsize);
 DWORD nr = word(r0.nrec);
 m_blocks = Buffer<Rec>(nr);
 // fill in records table
 RFile::seek(PDBHDRSIZE + 8);
 for (DWORD j = 0; j < nr; ++j) {
 DWORD off[2];
 if (RFile::read2(off, 8) != 8)
 goto fail;
 m_blocks[j].off = dword(off[0]);
 if (j > 0) {
 m_blocks[j - 1].csize = m_blocks[j].off - m_blocks[j - 1].off;
 if (m_blocks[j - 1].csize > m_rsz)
 goto fail;
 }
 }
 if (nr + 1 < word(hdr.numrec)) {
 // minus rec0
 DWORD off[2];
 if (RFile::read2(off, 8) != 8)
 goto fail;
 m_blocks[nr - 1].csize = dword(off[0]) - m_blocks[nr - 1].off;
 }
 else
 m_blocks[nr - 1].csize = (DWORD)RFile::size() - m_blocks[nr - 1].off;
 if (m_blocks[nr - 1].csize > m_rsz)
 goto fail;
 if (m_comp) {
 // compressed
 Buffer<BYTE> tmp(m_rsz);
 DWORD uoff = 0;
 for (int i = 0; i < m_blocks.size(); ++i) {
 RFile::seek(m_blocks[i].off);
 if (RFile::read2(tmp, m_blocks[i].csize) != m_blocks[i].csize)
 goto fail;
 m_blocks[i].usize = scan_size(tmp, m_blocks[i].csize);
 if (m_blocks[i].usize > m_rsz)
 goto fail;
 m_blocks[i].uoff = uoff;
 uoff += m_blocks[i].usize;
 }
 m_length = uoff;
 }
 else {
 // uncompressed
 for (int i = 0; i < m_blocks.size(); ++i) {
 m_blocks[i].usize = m_blocks[i].csize;
 m_blocks[i].uoff = m_length;
 m_length += m_blocks[i].usize;
 }
 }
 return;
 }
fail:
 CTVApp::Barf(_T("Invalid or unsupported PDB file"));
}

static DWORD pdb_decompress(BYTE *source, DWORD srclen, BYTE *dest, DWORD destlen)
{
 BYTE *se = source + srclen;
 BYTE *de = dest + destlen;
 BYTE *dd = dest;

 while (source < se && dest < de) {
 DWORD c = *source++;
 if (c >= 1 && c <= 8) {
 // copy
 while (c-- && source < se && dest < de)
 *dest++ = *source++;
 }
 else if (c <= 0x7f) // this char
 *dest++ = (BYTE)c;
 else if (c >= 0xc0) {
 // space + c&0x7f
 *dest++ = ' ';
 if (dest < de)
 *dest++ = (BYTE)c & 0x7f;
 }
 else if (source < se) {
 // copy from decoded buf
 c = (c << 8) | *source++;
 int k = (c & 0x3fff) >> 3;
 c = 3 + (c & 7);
 if (dest - k<dd || dest + c>de) // invalid buffer
 break;
 while (c-- && dest < de) {
 *dest = dest[-k];
 ++dest;
 }
 }
 }
 return dest - dd;
}

void PDBFile::seek(DWORD pos) {
 if (pos >= m_length)
 m_ptr = m_length;
 else
 m_ptr = pos&BMASK;
}

DWORD PDBFile::read(void *buf) {
 if (m_ptr >= m_length)
 return 0;
 // ok, figure what block is needed
 int i;
 for (i = 0; i < m_blocks.size(); ++i)
 if (m_ptr >= m_blocks[i].uoff && m_ptr < m_blocks[i].uoff + m_blocks[i].usize)
 goto found;
 return 0;
found:
 Buffer<BYTE> tmp(m_rsz), tmp2(m_rsz);
 BYTE *p = (BYTE*)buf;
 DWORD n = BSZ;
 while (i < m_blocks.size() && n>0) {
 RFile::seek(m_blocks[i].off);
 if (m_comp) {
 if (RFile::read2(tmp2, m_blocks[i].csize) != m_blocks[i].csize)
 return 0;
 if (pdb_decompress(tmp2, m_blocks[i].csize, tmp, m_rsz) != m_blocks[i].usize)
 return 0;
 }
 else {
 if (RFile::read2(tmp, m_blocks[i].csize) != m_blocks[i].csize)
 return 0;
 }
 // now we have a decompressed block in tmp
 DWORD hb = m_blocks[i].uoff + m_blocks[i].usize - m_ptr;
 if (hb > n)
 hb = n;
 memcpy(p, tmp + m_ptr - m_blocks[i].uoff, hb);
 p += hb;
 m_ptr += hb;
 n -= hb;
 ++i;
 }
 return BSZ - n;
}

CString PDBFile::CompressionInfo() {
 if (m_comp && m_length > 0) {
 CString ret;
 ret.Format(_T("PDB, packed (%.2f)"),
 (double)RFile::size() / (double)m_length);
 return ret;
 }
 return _T("PDB, not packed");
}

bool PDBFile::IsPDB(RFile *fp) {
 PDBHdr hdr;
 PDBRec0 r0;
 bool ret = CheckPDB(fp, hdr, r0);
 fp->seek(0);
 return ret;
}
