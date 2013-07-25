// Written by Andrey Chicherov, uart@os2.ru

#ifndef TCRFILE_H
#define TCRFILE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "RFile.h"

class TCRFile : public RFile {

public:

	TCRFile(const CString& fn);

	// generic file operations

	virtual DWORD size() { return m_length; }
	virtual DWORD read(void *buf);
	virtual void seek(DWORD pos);

	// compression

	virtual CString CompressionInfo();

	// check if this is a tcr file

	static bool IsTCR(RFile *fp);

protected:

	struct Block { DWORD size, off, usize, uoff; };
	Buffer<unsigned char> Dictonary[256];
	Buffer<Block> m_blocks;
	DWORD m_length, m_ptr;

	static bool	CheckTCR(RFile *fp);
};

#endif
