
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

#ifndef BAGEL_BOFLIB_DAT_FILE_H
#define BAGEL_BOFLIB_DAT_FILE_H

#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/file.h"

namespace Bagel {

#define CDF_NOFLAGS 0x00000000
#define CDF_READONLY CBF_READONLY   // Open for Read-only access
#define CDF_OVERWRITE CBF_OVERWRITE // *Overwrite any existing data-file
#define CDF_SHARED CBF_SHARED       // *Open for Shared access
#define CDF_CREATE CBF_CREATE       // *Create new file if not exist

#define CDF_MEMORY 0x00010000     // *header/footer should stay in memory
#define CDF_ENCRYPT 0x00020000    // Specifies if data should use encryption
#define CDF_KEEPOPEN 0x00040000   // File should be kept open after construction
#define CDF_COMPRESSED 0x00080000 // *Specifies if data should be compressed
// * = indicates feature not yet implemented

#define CDF_DEFAULT (CDF_MEMORY | CDF_ENCRYPT | CDF_SHARED | CDF_KEEPOPEN | CDF_READONLY)

#define MAX_PW_LEN 32 // Max Password length

class HEADER_REC {
public:
	LONG m_lOffset;
	LONG m_lLength;
	ULONG m_lCrc;
	ULONG m_lKey;
};

class CBofDataFile : public CBofFile {
private:
	CHAR m_szPassWord[MAX_PW_LEN];
	LONG m_lHeaderLength = 0;
	LONG m_lHeaderStart = 0;
	LONG m_lNumRecs = 0;
	HEADER_REC *m_pHeader = nullptr;

	BOOL m_bHeaderDirty;

protected:
	/**
	 * Read the header (actually a footer) from the data-file.
	 * @return                  Error return code
	 */
	ERROR_CODE ReadHeader();

	/**
	 * Writes the header (actually a footer) to the data-file.
	 * @return                  Error return code
	 */
	ERROR_CODE WriteHeader();

public:
	/**
	 * Constructor
	*/
	CBofDataFile();

	/**
	 * Constructor
	 * @param pszFileName       Name of .DAT file
	 * @param lFlags            Flags for open, and ecnryption, etc.
	 * @param pszPassword       Password for encryption
	 */

	CBofDataFile(const CHAR *pszFileName, ULONG lFlags = CDF_DEFAULT, const CHAR *pPassword = nullptr);

	/**
	 * Destructor
	 */
	virtual ~CBofDataFile();

	/**
	 * Initializes a CBofDataFile with specified info
	 * @param pszFileName       Name of .DAT file
	 * @param lFlags            Flags for open, and ecnryption, etc.
	 * @param pszPassword       Password for encryption
	 * @return                  Error return code
	 */
	ERROR_CODE SetFile(const CHAR *pszFileName, ULONG lFlags = CDF_DEFAULT, const CHAR *pPassword = nullptr);

	/**
	 * Free memory used by this object
	 * @return                  Error return code
	 */
	ERROR_CODE ReleaseFile();

	/**
	 * Retrieves size of specified record.
	 * @param lRecNum           Index of record to get size of
	 * @return                  Size of specified record
	 */
	LONG GetRecSize(LONG lRecNum);
	LONG GetNumberOfRecs() const {
		return (m_lNumRecs);
	}

	/**
	 * Retrieves size of the largest record.
	 * @return                  Size of largest record in the data-file
	 */
	LONG GetMaxRecSize() const;

	/**
	 * Opens an existsing data-file, or creates a new one.
	 * @return          Error return code
	 */
	ERROR_CODE Open();

	/**
	 * Closes current data-file, if it's not already closed
	 * @return          Error return code
	 */
	ERROR_CODE Close();

	/**
	 * Destroys current data-file, if any, and starts a new empty one
	 * @return          Error return code
	 */
	ERROR_CODE Create();

	/**
	 * Reads specified record from data-file.
	 * @param lRecNum   Record number to read
	 * @param pBuf      Buffer to store record
	 * @return          Error return code
	 */
	ERROR_CODE ReadRecord(LONG lRecNum, VOID *pBuf);

	/**
	 * Read a set number of bytes from the beginning of a file,
	 * don't bother with a CRC, but decrypt if necessary.  This is dependant upon
	 * the decryption being based on a single byte ordering scheme.
	 */
	ERROR_CODE ReadFromFile(LONG lRecNum, VOID *pBuf, LONG lBytes);

	/**
	 * Writes specified to data-file.
	 * @param lRecordNum        Record number to read
	 * @param pBuf              Buffer to write data from
	 * @param lRecSize          Size of buffer
	 * @param bUpdateHeader     TRUE if header is to be commited to disk
	 * @return                  Error return code
	 */
	ERROR_CODE WriteRecord(LONG lRecNum, VOID *pBuf, LONG lRecSize = -1, BOOL bUpdateHeader = FALSE, ULONG lKey = 0xFFFFFFFF);

	/**
	 * Verifies specified record in data-file.
	 * @param lRecNum           Record number to verify
	 * @return                  Error return code
	 */
	ERROR_CODE VerifyRecord(LONG lRecNum);

	/**
	 * Verifies all records in this file
	 * @return                  Error return code
	 */
	ERROR_CODE VerifyAllRecords();

	/**
	 * Adds a new record to the data-file.
	 * @param pBuf              Buffer to write data from
	 * @param lRecSize          Size of buffer
	 * @param bUpdateHeader     TRUE if header is to be commited to disk
	 * @param key               Key
	 * @return                  Error return code
	 */
	ERROR_CODE AddRecord(VOID *pBuf, LONG lRecSize, BOOL bUpdateHeader = FALSE, ULONG lKey = 0xFFFFFFFF);

	/**
	 * Deletes specified record from data-file.
	 * @param lRecNum           Record number to read
	 * @param bUpdateHeader     TRUE if header is to be commited to disk
	 * @return                  Error return code
	 */
	ERROR_CODE DeleteRecord(LONG lRecNum, BOOL bUpdateHeader = FALSE);

	/**
	 * Finds record by it's key.
	 * @param lKey              Key to search records with
	 * @return                  Index of record matching key, or -1
	 */
	LONG FindRecord(ULONG lKey);

	/**
	 * Sets encryption password
	 * @param pszPassword       New password
	 */
	VOID SetPassword(const CHAR *pszPassword);
	const CHAR *GetPassword() const {
		return m_szPassWord;
	}
};

} // namespace Bagel

#endif
