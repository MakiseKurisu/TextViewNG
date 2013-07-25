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
* $Id: Dictionary.h,v 1.14.2.2 2004/07/07 12:04:47 mike Exp $
*
*/

#pragma warning(disable:4100)

#if !defined(AFX_DICTIONARY_H__C28A6875_F537_4ED2_9B6B_1B37FDC74AAD__INCLUDED_)
#define AFX_DICTIONARY_H__C28A6875_F537_4ED2_9B6B_1B37FDC74AAD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Dictionary interface
class IDict {
public:
	virtual ~IDict() { }

	virtual int	  NumWords() = 0;
	virtual Buffer<wchar_t> GetWordW(int index) = 0;
	virtual bool	  Find(const wchar_t *word, int& index, int& found) = 0;
	virtual bool	  Valid() = 0;
	virtual int	  GetStartPofWord(int index) = 0;
	virtual int	  GetWordFromP(int para) = 0;
	virtual int	  GetNumP() = 0;

	CString	  GetWord(int index);

	static IDict	*Create(const CString& filename, CString *errmsg = NULL);

	enum { SIGSIZE = 4 };

	class DictInit {
		DictInit	      *m_next;
		char	      m_sig[SIGSIZE];
		IDict	      *(*m_create)(RFile *fp);
		static DictInit   *m_head;
		friend class IDict;
	protected:
		DictInit(const char *sig, IDict *(*create)(RFile *fp));
	};
};

class DictParser : public TextParser {
	auto_ptr<IDict>	m_dict;
	DictParser(auto_ptr<IDict> dict) : TextParser(NULL, NULL, NULL, NULL), m_dict(dict) { }
	Buffer<wchar_t>	GetParagraphImp(int para, bool& isfirst);
public:

	// paragraphs
	virtual Paragraph	GetParagraph(int docid, int para);
	virtual int		GetPLength(int docid, int para);
	virtual int		Length(int docid) { return m_dict->GetNumP(); }

	virtual int		GetPStart(int docid, int para) { return para; }
	virtual int		GetTotalLength(int docid) { return Length(docid); }
	virtual int		LookupParagraph(int docid, int charpos) { return charpos; }

	// links
	virtual bool		LookupReference(const wchar_t *rname, FilePos& dest);

	// create
	static DictParser	*OpenDict(const CString& path, CString *errmsg = NULL);
};

#endif // !defined(AFX_DICTIONARY_H__C28A6875_F537_4ED2_9B6B_1B37FDC74AAD__INCLUDED_)
