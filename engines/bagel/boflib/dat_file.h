
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
#define CDF_SAVEFILE CBF_SAVEFILE

#define CDF_MEMORY 0x00010000     // *header/footer should stay in memory
#define CDF_ENCRYPT 0x00020000    // Specifies if data should use encryption
#define CDF_KEEPOPEN 0x00040000   // File should be kept open after construction
#define CDF_COMPRESSED 0x00080000 // *Specifies if data should be compressed
// * = indicates feature not yet implemented

#define CDF_DEFAULT (CDF_MEMORY | CDF_ENCRYPT | CDF_SHARED | CDF_KEEPOPEN | CDF_READONLY)

#define MAX_PW_LEN 32 // Max Password length

class HEADER_REC {
public:
	int32 m_lOffset;
	int32 m_lLength;
	uint32 m_lCrc;
	uint32 m_lKey;
};

class CBofDataFile : public CBofFile {
private:
	char m_szPassWord[MAX_PW_LEN];
	int32 m_lHeaderLength = 0;
	int32 m_lHeaderStart = 0;
	int32 m_lNumRecs = 0;
	HEADER_REC *m_pHeader = nullptr;

	bool m_bHeaderDirty;

protected:
	/**
	 * Read the header (actually a footer) from the data-file.
	 * @return                  Error return code
	 */
	ErrorCode ReadHeader();

	/**
	 * Writes the header (actually a footer) to the data-file.
	 * @return                  Error return code
	 */
	ErrorCode WriteHeader();

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

	CBofDataFile(const char *pszFileName, uint32 lFlags = CDF_DEFAULT, const char *pPassword = nullptr);

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
	ErrorCode SetFile(const char *pszFileName, uint32 lFlags = CDF_DEFAULT, const char *pPassword = nullptr);

	/**
	 * Free memory used by this object
	 * @return                  Error return code
	 */
	ErrorCode ReleaseFile();

	/**
	 * Retrieves size of specified record.
	 * @param lRecNum           Index of record to get size of
	 * @return                  Size of specified record
	 */
	int32 GetRecSize(int32 lRecNum);
	int32 GetNumberOfRecs() const {
		return m_lNumRecs;
	}

	/**
	 * Retrieves size of the largest record.
	 * @return                  Size of largest record in the data-file
	 */
	int32 GetMaxRecSize() const;

	/**
	 * Opens an existsing data-file, or creates a new one.
	 * @return          Error return code
	 */
	ErrorCode Open();

	/**
	 * Closes current data-file, if it's not already closed
	 * @return          Error return code
	 */
	ErrorCode Close();

	/**
	 * Destroys current data-file, if any, and starts a new empty one
	 * @return          Error return code
	 */
	ErrorCode Create();

	/**
	 * Reads specified record from data-file.
	 * @param lRecNum   Record number to read
	 * @param pBuf      Buffer to store record
	 * @return          Error return code
	 */
	ErrorCode ReadRecord(int32 lRecNum, void *pBuf);

	/**
	 * Read a set number of bytes from the beginning of a file,
	 * don't bother with a CRC, but decrypt if necessary.  This is dependant upon
	 * the decryption being based on a single byte ordering scheme.
	 */
	ErrorCode ReadFromFile(int32 lRecNum, void *pBuf, int32 lBytes);

	/**
	 * Writes specified to data-file.
	 * @param lRecordNum        Record number to read
	 * @param pBuf              Buffer to write data from
	 * @param lRecSize          Size of buffer
	 * @param bUpdateHeader     true if header is to be commited to disk
	 * @return                  Error return code
	 */
	ErrorCode WriteRecord(int32 lRecNum, void *pBuf, int32 lRecSize = -1, bool bUpdateHeader = false, uint32 lKey = 0xFFFFFFFF);

	/**
	 * Verifies specified record in data-file.
	 * @param lRecNum           Record number to verify
	 * @return                  Error return code
	 */
	ErrorCode VerifyRecord(int32 lRecNum);

	/**
	 * Verifies all records in this file
	 * @return                  Error return code
	 */
	ErrorCode VerifyAllRecords();

	/**
	 * Adds a new record to the data-file.
	 * @param pBuf              Buffer to write data from
	 * @param lRecSize          Size of buffer
	 * @param bUpdateHeader     true if header is to be commited to disk
	 * @param key               Key
	 * @return                  Error return code
	 */
	ErrorCode AddRecord(void *pBuf, int32 lRecSize, bool bUpdateHeader = false, uint32 lKey = 0xFFFFFFFF);

	/**
	 * Deletes specified record from data-file.
	 * @param lRecNum           Record number to read
	 * @param bUpdateHeader     true if header is to be commited to disk
	 * @return                  Error return code
	 */
	ErrorCode DeleteRecord(int32 lRecNum, bool bUpdateHeader = false);

	/**
	 * Finds record by it's key.
	 * @param lKey              Key to search records with
	 * @return                  Index of record matching key, or -1
	 */
	int32 FindRecord(uint32 lKey);

	/**
	 * Sets encryption password
	 * @param pszPassword       New password
	 */
	void SetPassword(const char *pszPassword);
	const char *GetPassword() const {
		return m_szPassWord;
	}
};

} // namespace Bagel

#endif
