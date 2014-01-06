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
* $Id: PDBFile.h,v 1.6.4.2 2004/07/07 12:04:47 mike Exp $
*
*/


#if !defined(AFX_PDBFILE_H__777AC4E9_AB31_44B8_9A41_80B433927455__INCLUDED_)
#define AFX_PDBFILE_H__777AC4E9_AB31_44B8_9A41_80B433927455__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "RFile.h"

// TODO: the decoder is twice slower when block sizes do not evenly divide
// BSZ
class PDBFile : public RFile
{
public:
    PDBFile(const CString& fn);

    // generic file operations
    virtual DWORD   size() { return m_length; }
    virtual DWORD   read(void *buf);
    virtual void   seek(DWORD pos);

    // compression
    virtual CString CompressionInfo();

    // check if this is a pdb file
    static bool   IsPDB(RFile *fp);
protected:
    DWORD    m_length;
    DWORD    m_ptr;
    DWORD    m_rsz;
    bool    m_comp;
    struct Rec {
        DWORD   usize;
        DWORD   uoff;
        DWORD   off;
        DWORD   csize;
    };
    Buffer<Rec>   m_blocks;

    static bool   CheckPDB(RFile *fp, struct PDBHdr&, struct PDBRec0&);
    int    findblock(DWORD uoff);
};

#endif // !defined(AFX_PDBFILE_H__777AC4E9_AB31_44B8_9A41_80B433927455__INCLUDED_)
