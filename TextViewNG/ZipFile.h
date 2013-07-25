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
* $Id: ZipFile.h,v 1.9.2.3 2004/07/07 12:04:47 mike Exp $
*
*/

#if !defined(AFX_ZIPFILE_H__CDE86113_3C6F_4FE5_BCEF_F512CA03E3D3__INCLUDED_)
#define AFX_ZIPFILE_H__CDE86113_3C6F_4FE5_BCEF_F512CA03E3D3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "zlib.h"
#include "RFile.h"

class ZipFile : public RFile
{
	struct ZFile {
		ZFile	*parent;
		int		off; // offset into zip file, -1 for directories
		union {
			struct {
				// for plain files
				int	csize; // compressed size, -1 if file is not compressed
				int	size; // uncompressed size, -1 if compression type unsupported
			};
			CMapStringToPtr	*children; // for directories
		};
		~ZFile();
		bool isdir() { return off < 0; }
		bool iscmp() { return csize >= 0; }
		void fixup_encoding(int enc);
	};
public:
	ZipFile(const CString& fn);
	virtual ~ZipFile();

	// file operations
	virtual DWORD	  size();
	virtual DWORD	  read(void *buf);
	virtual void	  seek(DWORD pos);

	// compression
	virtual CString CompressionInfo();

	// zip file specific methods
	bool		  SetDir(const CString& dir); // set current zip dir
	bool		  GetNextFileInfo(CString& fname, bool& isdir, __int64& size);
	void		  Reset(); // reset iterator
	bool		  ReadZip(); // load zip file directory
	bool		  Open(const CString& filename); // open file in current dir
	bool		  IsSingleFile(CString *s);
protected:
	ZFile		  m_rootdir;
	ZFile		  *m_curdir;
	POSITION	  m_curpos;
	ZFile		  *m_curfile;
	z_stream	  *m_zs;
	Buffer<Bytef>	  m_in;
	DWORD		  m_ptr;

	void		  rewind();
};

#endif // !defined(AFX_ZIPFILE_H__CDE86113_3C6F_4FE5_BCEF_F512CA03E3D3__INCLUDED_)
