
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

#include "common/serializer.h"
#include "bagel/spacebar/boflib/file.h"

namespace Bagel {
namespace SpaceBar {

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

struct HeaderRec {
public:
	int32 _lOffset;
	int32 _lLength;
	uint32 _lCrc;
	uint32 _lKey;

	void synchronize(Common::Serializer &s);
	static int size() {
		return 16;
	}
};

struct HeadInfo {
	int32 _lNumRecs;  // Number of records in this file
	int32 _lAddress;  // starting address of footer
	uint32 _lFlags;   // contains flags for this file
	uint32 _lFootCrc; // CRC of the footer

	void synchronize(Common::Serializer &s);
	static int size() {
		return 16;
	}
};

class CBofDataFile : public CBofFile {
private:
	char _szPassWord[MAX_PW_LEN];
	int32 _lHeaderLength = 0;
	int32 _lHeaderStart = 0;
	int32 _lNumRecs = 0;
	HeaderRec *_pHeader = nullptr;

	bool _bHeaderDirty;

protected:
	/**
	 * Read the header (actually a footer) from the data-file.
	 * @return                  Error return code
	 */
	ErrorCode readHeader();

	/**
	 * Writes the header (actually a footer) to the data-file.
	 * @return                  Error return code
	 */
	ErrorCode writeHeader();

public:
	/**
	 * Constructor
	*/
	CBofDataFile();

	/**
	 * Destructor
	 */
	virtual ~CBofDataFile();

	/**
	 * Initializes a CBofDataFile with specified info
	 * @param pszFileName       Name of .DAT file
	 * @param lFlags            Flags for open, and encryption, etc.
	 * @return                  Error return code
	 */
	ErrorCode setFile(const char *pszFileName, uint32 lFlags);

	/**
	 * Free memory used by this object
	 * @return                  Error return code
	 */
	ErrorCode releaseFile();

	/**
	 * Retrieves size of specified record.
	 * @param lRecNum           Index of record to get size of
	 * @return                  Size of specified record
	 */
	int32 getRecSize(int32 lRecNum);
	int32 getNumberOfRecs() const {
		return _lNumRecs;
	}

	/**
	 * Retrieves size of the largest record.
	 * @return                  Size of largest record in the data-file
	 */
	int32 getMaxRecSize() const;

	/**
	 * Opens an existing data-file, or creates a new one.
	 * @return          Error return code
	 */
	ErrorCode open();

	/**
	 * Closes current data-file, if it's not already closed
	 * @return          Error return code
	 */
	ErrorCode close() override;

	/**
	 * Destroys current data-file, if any, and starts a new empty one
	 * @return          Error return code
	 */
	ErrorCode create();

	/**
	 * Reads specified record from data-file.
	 * @param lRecNum   Record number to read
	 * @param pBuf      Buffer to store record
	 * @return          Error return code
	 */
	ErrorCode readRecord(int32 lRecNum, void *pBuf);

	/**
	 * Read a set number of bytes from the beginning of a file,
	 * don't bother with a CRC, but decrypt if necessary.  This is dependent upon
	 * the decryption being based on a single byte ordering scheme.
	 */
	ErrorCode readFromFile(int32 lRecNum, void *pBuf, int32 lBytes);

	/**
	 * Writes specified to data-file.
	 * @param lRecNum           Record number to read
	 * @param pBuf              Buffer to write data from
	 * @param lSize             Size of buffer
	 * @param bUpdateHeader     True if header is to be committed to disk
	 * @param lKey              Hash key
	 * @return                  Error return code
	 */
	ErrorCode writeRecord(int32 lRecNum, void *pBuf, int32 lSize = -1, bool bUpdateHeader = false, uint32 lKey = 0xFFFFFFFF);

	/**
	 * Verifies specified record in data-file.
	 * @param lRecNum           Record number to verify
	 * @return                  Error return code
	 */
	ErrorCode verifyRecord(int32 lRecNum);

	/**
	 * Verifies all records in this file
	 * @return                  Error return code
	 */
	ErrorCode verifyAllRecords();

	/**
	 * Adds a new record to the data-file.
	 * @param pBuf              Buffer to write data from
	 * @param lLength           Size of buffer
	 * @param bUpdateHeader     true if header is to be committed to disk
	 * @param lKey              hash Key
	 * @return                  Error return code
	 */
	ErrorCode addRecord(void *pBuf, int32 lLength, bool bUpdateHeader = false, uint32 lKey = 0xFFFFFFFF);

	/**
	 * Finds record by it's key.
	 * @param lKey              Key to search records with
	 * @return                  Index of record matching key, or -1
	 */
	int32 findRecord(uint32 lKey);

	/**
	 * Sets encryption password
	 * @param pszPassword       New password
	 */
	void setPassword(const char *pszPassword);
	const char *getPassword() const {
		return _szPassWord;
	}

	/**
	 * Read from a currently open file
	 * @param pDestBuf      Destination buffer
	 * @param lBytes        Number of bytes
	 * @return              Error code
	 */
	ErrorCode read(void *pDestBuf, int32 lBytes) override;
	ErrorCode read(HeaderRec &rec);
	ErrorCode read(HeadInfo &rec);

	/**
	 * Write to a currently open file
	 * @param pSrcBuf       Source buffer
	 * @param lBytes        Number of bytes
	 * @return              Error code
	 */
	ErrorCode write(const void *pSrcBuf, int32 lBytes) override;
	ErrorCode write(HeaderRec &rec);
	ErrorCode write(HeadInfo &rec);
};

} // namespace SpaceBar
} // namespace Bagel

#endif
