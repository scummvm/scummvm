
/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BAGEL_BOFLIB_FILE_H
#define BAGEL_BOFLIB_FILE_H

#include "common/stream.h"
#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/object.h"
#include "bagel/boflib/error.h"

namespace Bagel {

#define CBF_TEXT 0x00000001
#define CBF_BINARY 0x00000002
#define CBF_READONLY 0x00000004
#define CBF_OVERWRITE 0x00000008
#define CBF_SHARED 0x00000010
#define CBF_CREATE 0x00000020

#define CBF_DEFAULT (CBF_BINARY | CBF_READONLY)

#define CBOFFILE_TEXT CBF_TEXT
#define CBOFFILE_READONLY CBF_READONLY
#define CBOFFILE_OVERWRITE CBF_OVERWRITE

#define CBOFFILE_DEFAULT CBF_DEFAULT

class CBofFile : public CBofObject, public CBofError {
protected:
	CHAR m_szFileName[MAX_FNAME];
	Common::Stream *_stream = nullptr;
	ULONG m_lFlags = CBF_DEFAULT;

public:
	/**
	 * Default constructor
	 */
	CBofFile();

	/**
	 * Open a specified file for access
	 * @param pszFileName		Filename
	 * @param lFlags			Access flags
	 */
	CBofFile(const CHAR *pszFileName, ULONG lFlags = CBF_DEFAULT);

	/**
	 * Destructor
	 */
	virtual ~CBofFile();

	/**
	 * Open specified file into this object
	 * @param pszFileName		Filename
	 * @param lFlags			Access flags
	 */
	ERROR_CODE Open(const CHAR *pszFileName, ULONG lFlags = CBF_DEFAULT);

	/**
	 * Creates specified file
	 * @param pszFileName		Filename
	 * @param lFlags			Access flags
	 */
	ERROR_CODE Create(const CHAR *pszFileName, ULONG lFlags = CBF_DEFAULT | CBF_CREATE);

	/**
	 * Close a currently open file
	 */
	 VOID Close();

	/**
	 * Read from a currently open file
	 * @param pDestBuf		Destination buffer
	 * @param lBytes		Number of bytes
	 * @return				Error code
	 */
	ERROR_CODE Read(VOID *pDestBuf, LONG lBytes);

	/**
	 * Write to a currently open file
	 * @param pSrcBuf		Source buffer
	 * @param lBytes		Number of bytes
	 * @return				Error code
	 */
	ERROR_CODE Write(const VOID *pSrcBuf, LONG lBytes);

	/**
	 * Flushes I/O stream
	 */
	VOID Commit();

	//static VOID CommitAll() { _flushall(); }

	/**
	 * Seek to a specified location in the file
	 * @return		Error code
	 */
	ERROR_CODE Seek(ULONG lPos) { return (SetPosition(lPos)); }

	/**
	 * Sets the file pointer to the beginning of the file
	 * @return		Error code
	 */
	ERROR_CODE SeekToBeginning() { return (SetPosition(0)); }

	/**
	 * Sets the file pointer to the end of the file
	 * @return		Error code
	 */
	ERROR_CODE SeekToEnd();

	/**
	 * Sets the current file-seek position to that specified
	 * @param lPos		New position
	 */
	ERROR_CODE SetPosition(ULONG lPos);

	/**
	 * Retrieves the current seek position
	 */
	ULONG GetPosition();

	/**
	 * Set the length of a file
	 */
	ERROR_CODE SetLength(ULONG lNewLength);

	/**
	 * Get the length of a file
	 */
	ULONG GetLength();

	operator Common::SeekableReadStream* () const {
		return dynamic_cast<Common::SeekableReadStream *>(_stream);
	}
};

} // namespace Bagel

#endif
