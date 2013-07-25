// Written by Andrey Chicherov, uart@os2.ru

#define _WIN32_WINNT	_WIN32_WINNT_MAXVER

#include <afxwin.h>

#include "ptr.h"
#include "TCRFile.h"
#include "TextViewNG.h"

#include <string.h>


// ----------------------------------------------------------------------------------------------------------------

CString TCRFile::CompressionInfo() {

	CString ret; ret.Format(_T("TCR, packed (%.2f)"),
		(double) RFile::size() / (double) m_length
		);

	return ret;

}

// ----------------------------------------------------------------------------------------------------------------

bool TCRFile::CheckTCR(RFile *fp) {

	char sign[9]; fp->seek(0);
	if (fp->read2(sign, 9) != 9) return false;

	if (memcmp(sign, "!!8-Bit!!", 9))
		return false;

	return true;

}

// ----------------------------------------------------------------------------------------------------------------

bool TCRFile::IsTCR(RFile *fp) {

	bool ret = CheckTCR(fp);
	fp->seek(0);
	return ret;

}

// ----------------------------------------------------------------------------------------------------------------

TCRFile::TCRFile(const CString& fn) : RFile(fn), m_length(0), m_ptr(0) {

	if (Reopen() && CheckTCR(this)) {
		int i;

		DWORD offset = 9; unsigned char ch; for (i = 0; i < 256; i++) {

			if (RFile::read2(&ch, 1) != 1) goto fail; Dictonary[i] = Buffer<unsigned char>(ch + 1);
			Dictonary[i][0] = ch; if (RFile::read2(Dictonary[i] + 1, ch) != ch) goto fail;
			offset += ch + 1;

		}

		int nr = (int) ((RFile::size() - offset + 4095) / 4096); m_blocks = Buffer<Block>(nr);
		Buffer<unsigned char> tmp(4096); for (i = 0; i < nr; i++) {

			m_blocks[i].off = offset; m_blocks[i].size = RFile::read2(tmp, 4096);
			m_blocks[i].uoff = m_length; m_blocks[i].usize = 0;
			for (DWORD j = 0; j < m_blocks[i].size; j++)
				m_blocks[i].usize += Dictonary[tmp[j]][0];

			m_length += m_blocks[i].usize;
			offset += m_blocks[i].size;

		}

		return;

	}

fail:

	CTVApp::Barf(_T("Invalid or unsupported TCR file"));

}

// ----------------------------------------------------------------------------------------------------------------

void TCRFile::seek(DWORD pos) {

	if (pos >= m_length)
		m_ptr = m_length;
	else
		m_ptr = pos&BMASK;

}

// ----------------------------------------------------------------------------------------------------------------

DWORD TCRFile::read(void *buf) {

	unsigned char *out = (unsigned char*) buf; DWORD n = BSZ; for (int i = 0; i < m_blocks.size() && n; i++)
		if (m_ptr >= m_blocks[i].uoff && m_ptr < m_blocks[i].uoff + m_blocks[i].usize) {

			Buffer<unsigned char> src(m_blocks[i].size);  RFile::seek(m_blocks[i].off);
			if (RFile::read2(src, m_blocks[i].size) != m_blocks[i].size) return 0;

			Buffer<unsigned char> dst(m_blocks[i].usize); unsigned char *ptr = dst;
			for (DWORD j = 0; j < m_blocks[i].size; j++) {

				unsigned char len = Dictonary[src[j]][0];
				memcpy(ptr, Dictonary[src[j]] + 1, len);
				ptr += len;

			}

			DWORD len = m_blocks[i].usize - (m_ptr - m_blocks[i].uoff); if (len > n) len = n;
			memcpy(out, dst + m_ptr - m_blocks[i].uoff, len); out += len;
			m_ptr += len; n -= len;

		}

		return BSZ - n;

}
