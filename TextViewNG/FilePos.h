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
* $Id: FilePos.h,v 1.3.2.1 2003/04/12 22:52:32 mike Exp $
*
*/

#ifndef FILEPOS_H
#define FILEPOS_H

struct FilePos {
	int	  para;
	int	  off;
	int	  docid;

	FilePos() : para(0), off(0), docid(0) { }
	FilePos(int p, int o, int did) : para(p), off(o), docid(did) { }
	FilePos(const FilePos& p) : para(p.para), off(p.off), docid(p.docid) { }
	FilePos& operator=(const FilePos& p) { para = p.para; off = p.off; docid = p.docid; return *this; }
	bool operator==(int p) const { return para == p && off == 0 && docid == 0; }
	bool operator!=(int p) const { return !operator==(p); }
	bool operator==(const FilePos& p) const { return para == p.para && off == p.off && docid == p.docid; }
	bool operator!=(const FilePos& p) const { return !operator==(p); }
	bool operator < (const FilePos& p) const {
		return docid < p.docid || (docid == p.docid &&
			(para < p.para || (para == p.para && off < p.off)));
	}
	bool operator > (const FilePos& p) const { return !operator < (p) && !operator==(p); }
	bool operator<=(const FilePos& p) const { return operator < (p) || operator==(p); }
	bool operator>=(const FilePos& p) const { return !operator < (p); }
	FilePos operator+(int i) { return FilePos(para, off + i, docid); }
};

#endif
