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

#include "bagel/spacebar/boflib/dat_file.h"
#include "bagel/spacebar/boflib/crc.h"
#include "bagel/spacebar/boflib/debug.h"
#include "bagel/boflib/log.h"
#include "bagel/boflib/misc.h"
#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/file_functions.h"

namespace Bagel {
namespace SpaceBar {

// Local prototypes
static uint32 CreateHashCode(const byte *);

void HeaderRec::synchronize(Common::Serializer &s) {
	s.syncAsSint32LE(_lOffset);
	s.syncAsSint32LE(_lLength);
	s.syncAsUint32LE(_lCrc);
	s.syncAsUint32LE(_lKey);
}

void HeadInfo::synchronize(Common::Serializer &s) {
	s.syncAsSint32LE(_lNumRecs);
	s.syncAsSint32LE(_lAddress);
	s.syncAsUint32LE(_lFlags);
	s.syncAsUint32LE(_lFootCrc);
}


CBofDataFile::CBofDataFile() {
	_szFileName[0] = '\0';
	_szPassWord[0] = '\0';
	_lHeaderLength = 0;
	_lNumRecs = 0;
	_pHeader = nullptr;
	_bHeaderDirty = false;
}

ErrorCode CBofDataFile::setFile(const char *pszFileName, uint32 lFlags) {
	assert(isValidObject(this));

	// Validate input
	assert(pszFileName != nullptr);
	assert(strlen(pszFileName) < MAX_FNAME);

	// Release any previous data-file
	releaseFile();

	// All data files are binary, so force it
	lFlags |= CBF_BINARY;

	// Remember the flags
	_lFlags = lFlags;

	if (fileGetFullPath(_szFileName, pszFileName) != nullptr) {
		if (open() == ERR_NONE) {

			// Read header block
			readHeader();

			// Close data file if we are not keeping it open
			if (!(_lFlags & CDF_KEEPOPEN)) {
				close();
			}
		} else
			reportError(ERR_FOPEN, "Could not open file %s", _szFileName);

	} else {
		reportError(ERR_FFIND, "Could not build full path to %s", pszFileName);
	}

	return _errCode;
}

CBofDataFile::~CBofDataFile() {
	assert(isValidObject(this));

	releaseFile();
}

ErrorCode CBofDataFile::releaseFile() {
	assert(isValidObject(this));

	// If header was modified
	if (_bHeaderDirty) {
		// Write header to disk
		writeHeader();
	}

	close();

	// Free header buffer
	delete[] _pHeader;
	_pHeader = nullptr;

	return _errCode;
}

ErrorCode CBofDataFile::create() {
	assert(isValidObject(this));

	HeadInfo stHeaderInfo;

	// Only continue if there is no current error
	if (_errCode == ERR_NONE) {
		if (_stream != nullptr) {
			close();
		}

		// Re-initialize
		delete[] _pHeader;
		_pHeader = nullptr;

		_stream = nullptr;
		_lHeaderLength = 0;
		_bHeaderDirty = false;

		stHeaderInfo._lNumRecs = _lNumRecs = 0;
		stHeaderInfo._lAddress = HeadInfo::size();

		// Create the file
		if (CBofFile::create(_szFileName, _lFlags) == ERR_NONE) {
			// Write empty header info
			if (write(stHeaderInfo) != ERR_NONE) {
				_errCode = ERR_FWRITE;
			}

			seek(0);

		} else {
			_errCode = ERR_FOPEN;
		}
	}

	return _errCode;
}

ErrorCode CBofDataFile::open() {
	assert(isValidObject(this));

	// Only continue if there is no current error
	if (_errCode == ERR_NONE && _stream == nullptr) {
		if (!(_lFlags & CDF_READONLY)) {
			if (_lFlags & CDF_SAVEFILE) {
				if (_lFlags & CDF_CREATE)
					create();
			} else if (!fileExists(_szFileName))
				create();
		}

		if (_stream == nullptr) {
			// Open data file
			CBofFile::open(_szFileName, _lFlags);
		}
	}

	return _errCode;
}

ErrorCode CBofDataFile::close() {
	assert(isValidObject(this));

	if (_stream != nullptr) {
		if (_bHeaderDirty) {
			writeHeader();
		}

		CBofFile::close();
	}

	return _errCode;
}

ErrorCode CBofDataFile::readHeader() {
	assert(isValidObject(this));

	// Only continue if there is no current error
	if (_errCode == ERR_NONE) {
		if (_stream == nullptr) {
			open();
		}

		if (!errorOccurred()) {
			// Determine number of records in file
			HeadInfo stHeaderInfo;
			if (read(stHeaderInfo) == ERR_NONE) {
				_lNumRecs = stHeaderInfo._lNumRecs;
				_lHeaderStart = stHeaderInfo._lAddress;

				// Length of header is number of records * header-record size
				_lHeaderLength = _lNumRecs * HeaderRec::size();

				Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(_stream);
				assert(rs);
				int32 lfileLength = rs->size();

				// Make sure header contains valid info
				if ((_lHeaderStart >= HeadInfo::size()) &&
				        (_lHeaderStart <= lfileLength) && (_lHeaderLength >= 0) &&
				        (_lHeaderLength < lfileLength)) {

					// Force Encrypted, and Compress if existing file has them
					_lFlags |= stHeaderInfo._lFlags & CDF_ENCRYPT;
					_lFlags |= stHeaderInfo._lFlags & CDF_COMPRESSED;

					if (_lHeaderLength != 0) {
						// Allocate buffer to hold header
						_pHeader = new HeaderRec[(int)_lNumRecs];

						// Seek to start of header
						seek(_lHeaderStart);

						// Read header
						ErrorCode errorCode = ERR_NONE;
						for (int i = 0; i < _lNumRecs && errorCode == ERR_NONE; ++i) {
							errorCode = read(_pHeader[i]);
						}

						if (errorCode == ERR_NONE) {
							uint32 lCrc = calculateCRC(&_pHeader->_lOffset, 4 * _lNumRecs);

							if (lCrc != stHeaderInfo._lFootCrc) {
								logError(buildString("Error: '%s' has invalid footer", _szFileName));
								_errCode = ERR_CRC;
							}

						} else {
							logError(buildString("Error: Could not read footer in file '%s'", _szFileName));
							_errCode = ERR_FREAD;
						}
					}

				} else {
					logError(buildString("Error: '%s' has invalid header", _szFileName));
					_errCode = ERR_FTYPE;
				}

			} else {
				logError(buildString("Error: Could not read header in file '%s'", _szFileName));
				_errCode = ERR_FREAD;
			}
		}
	}

	return _errCode;
}

ErrorCode CBofDataFile::writeHeader() {
	assert(isValidObject(this));

	// Only continue if there is no current error
	if (_errCode == ERR_NONE) {
		// Open the data file if it's not already open
		if (_stream == nullptr) {
			open();
		}

		if (_errCode == ERR_NONE) {
			// Header starts at the end of the last record
			HeaderRec *pRec = &_pHeader[_lNumRecs - 1];
			_lHeaderStart = pRec->_lOffset + pRec->_lLength;

			HeadInfo stHeaderInfo;
			stHeaderInfo._lNumRecs = _lNumRecs;
			stHeaderInfo._lAddress = _lHeaderStart;
			stHeaderInfo._lFlags = _lFlags;
			stHeaderInfo._lFootCrc = calculateCRC(&_pHeader->_lOffset, 4 * _lNumRecs);

			// Seek to front of file to write header info
			seekToBeginning();

			if (write(stHeaderInfo) == ERR_NONE) {
				// Seek to start of where header is to be written
				seek(_lHeaderStart);

				// Write header to data file
				ErrorCode errorCode = ERR_NONE;
				for (int i = 0; i < _lNumRecs && errorCode == ERR_NONE; ++i) {
					errorCode = write(_pHeader[i]);
				}

				if (errorCode == ERR_NONE) {
					// Header is now clean
					_bHeaderDirty = false;

				} else {
					logError(buildString("Error writing footer to file '%s'", _szFileName));
					_errCode = ERR_FWRITE;
				}

			} else {
				logError(buildString("Error writing header to file '%s'", _szFileName));
				_errCode = ERR_FWRITE;
			}
		}
	}

	return _errCode;
}

ErrorCode CBofDataFile::readRecord(int32 lRecNum, void *pBuf) {
	assert(isValidObject(this));

	// Only continue if there is no current error
	if (_errCode == ERR_NONE) {
		// Can't write to nullptr pointers
		assert(pBuf != nullptr);

		// Validate record number
		assert(lRecNum >= 0 && lRecNum < _lNumRecs);

		// Make sure we have a valid header
		assert(_pHeader != nullptr);
		// Get info about address of where record starts
		// and how large the record is.
		HeaderRec *pRecInfo = &_pHeader[(int)lRecNum];

		// Open the data file if it's not already open
		if (_stream == nullptr) {
			open();
		}

		if (_errCode == ERR_NONE) {
			// Seek to that point in the file
			seek(pRecInfo->_lOffset);

			// Read in the record
			if (read(pBuf, pRecInfo->_lLength) == ERR_NONE) {
				// If this file is encrypted, then decrypt it
				if (_lFlags & CDF_ENCRYPT) {
					decrypt(pBuf, (int)pRecInfo->_lLength, _szPassWord);
				}

				// Calculate and verify this record's CRC value
				uint32 lCrc = calculateCRC(pBuf, (int)pRecInfo->_lLength);

				if (lCrc != pRecInfo->_lCrc) {
					_errCode = ERR_CRC;
				}

			} else {
				logError(buildString("Error reading record %d in file '%s'", lRecNum, _szFileName));
				_errCode = ERR_FREAD;
			}
		}
	}

	return _errCode;
}

ErrorCode CBofDataFile::readFromFile(int32 lRecNum, void *pBuf, int32 lBytes) {
	assert(isValidObject(this));

	// Only continue if there is no current error
	if (_errCode == ERR_NONE) {
		// Can't write to nullptr pointers
		assert(pBuf != nullptr);

		// Validate record number
		assert(lRecNum >= 0 && lRecNum < _lNumRecs);

		// Make sure we have a valid header
		assert(_pHeader != nullptr);

		// Get info about address of where record starts
		// and how large the record is.
		HeaderRec *pRecInfo = &_pHeader[(int)lRecNum];

		// Open the data file if it's not already open
		if (_stream == nullptr) {
			open();
		}

		if (_errCode == ERR_NONE) {
			// Seek to that point in the file
			seek(pRecInfo->_lOffset);

			// Read in the requested bytes...
			if (read(pBuf, lBytes) == ERR_NONE) {
				// If this file is encrypted, then decrypt it
				if (_lFlags & CDF_ENCRYPT) {
					decryptPartial(pBuf, (int32)pRecInfo->_lLength, (int32)lBytes, _szPassWord);
				}

				// Don't bother with a CRC as this chunk of input won't generate a proper
				// CRC anyway.
			} else {
				logError(buildString("Error reading record %u in file '%s'", lRecNum, _szFileName));
				_errCode = ERR_FREAD;
			}
		}
	}

	return _errCode;
}

ErrorCode CBofDataFile::writeRecord(int32 lRecNum, void *pBuf, int32 lSize, bool bUpdateHeader, uint32 lKey) {
	assert(isValidObject(this));

	// Only continue if there is no current error
	if (_errCode == ERR_NONE) {
		// Validate record number
		assert(lRecNum >= 0 && lRecNum < _lNumRecs);

		// Validate input buffer
		assert(pBuf != nullptr);

		// There must already be a valid header
		assert(_pHeader != nullptr);

		if (lSize == -1)
			lSize = _pHeader[(int)lRecNum]._lLength;

		int32 lPrevOffset = HeadInfo::size();
		int32 lPrevLength = 0;

		if (lRecNum != 0) {
			lPrevOffset = _pHeader[(int)lRecNum - 1]._lOffset;
			lPrevLength = _pHeader[(int)lRecNum - 1]._lLength;
		}

		HeaderRec *pRecInfo = &_pHeader[(int)lRecNum];

		// Header needs to updated
		_bHeaderDirty = true;

		if (_stream == nullptr) {
			open();
		}

		// This record starts at the end of the last record
		pRecInfo->_lOffset = lPrevOffset + lPrevLength;

		// Seek to where we want to write this record
		seek(pRecInfo->_lOffset);

		// Calculate new hash code based on this records key
		pRecInfo->_lKey = lKey;
		if (lKey == 0xFFFFFFFF) {
			pRecInfo->_lKey = CreateHashCode((const byte *)pBuf);
		}

		// Calculate this record's CRC value
		pRecInfo->_lCrc = calculateCRC(pBuf, lSize);

		if (_lFlags & CDF_ENCRYPT) {
			encrypt(pBuf, lSize, _szPassWord);
		}

		// If new record is larger then original
		if (lSize > pRecInfo->_lLength) {
			// How many bytes back do we have to write?
			int32 lDiff = lSize - pRecInfo->_lLength;

			//
			// Move the rest of file back that many bytes
			//

			// Read the rest of the file in chunks (of 200k or less),
			// and write each chunk back in it's new position.
			//
			int32 lBufLength = getLength() - (pRecInfo->_lOffset + pRecInfo->_lLength);
			int32 lChunkSize = MIN(lBufLength, (int32)200000);

			// Allocate a buffer big enough for one chunk
			byte *pTmpBuf = (byte *)bofAlloc(lChunkSize);

			// While there is data to move
			while (lBufLength > 0) {
				// Seek to beginning of the source for this chunk
				setPosition(pRecInfo->_lOffset + pRecInfo->_lLength + lBufLength - lChunkSize);

				// Read the chunk
				read(pTmpBuf, lChunkSize);

				// Seek to this chunks new position (offset by 'lDiff' bytes)
				setPosition(pRecInfo->_lOffset + pRecInfo->_lLength + lBufLength - lChunkSize + lDiff);

				// Write chunk to new position
				write(pTmpBuf, lChunkSize);

				// That much less to do next time through
				lBufLength -= lChunkSize;

				// Last chunk is lBufLength
				lChunkSize = MIN(lBufLength, lChunkSize);
			}

			// Don't need that temp buffer anymore
			bofFree(pTmpBuf);

			// Tell the rest of the records that they moved
			for (int i = lRecNum + 1; i < getNumberOfRecs(); i++) {
				_pHeader[i]._lOffset += lDiff;
			}

			// Remember it's new length
			pRecInfo->_lLength = lSize;

			// Seek to where we want to write this record
			seek(pRecInfo->_lOffset);

			// Write this record
			write(pBuf, lSize);

			// If we are to update the header now
			if (bUpdateHeader) {
				writeHeader();
			}

		} else {
			// Write this record
			if (write(pBuf, lSize) == ERR_NONE) {
				// If this record got smaller
				if (pRecInfo->_lLength > lSize) {
					// Remember it's length
					pRecInfo->_lLength = lSize;

					int bufferSize = getMaxRecSize();
					if (bufferSize <= 0)
						fatalError(ERR_FREAD, "Invalid size read in header data");

					// Allocate a buffer that could hold the largest record
					byte *pTmpBuf = (byte *)bofAlloc(bufferSize);

					for (int i = (int)lRecNum + 1; i < (int)_lNumRecs - 1; i++) {
						_errCode = readRecord(i, pTmpBuf);
						if (_errCode != ERR_NONE)
							break;

						_errCode = writeRecord(i + 1, pTmpBuf);
						if (_errCode != ERR_NONE)
							break;
					}

					bofFree(pTmpBuf);
				}

				// If we are to update the header now
				if (bUpdateHeader) {
					writeHeader();
				}

			} else {
				_errCode = ERR_FWRITE;
			}
		}

		// If this record is encrypted the decrypt it
		if (_lFlags & CDF_ENCRYPT) {
			decrypt(pBuf, (int)pRecInfo->_lLength, _szPassWord);
		}
	}

	return _errCode;
}

ErrorCode CBofDataFile::verifyRecord(int32 lRecNum) {
	assert(isValidObject(this));

	if (_errCode == ERR_NONE) {
		// Validate record number
		assert(lRecNum >= 0 && lRecNum < _lNumRecs);

		// Allocate space to hold this record
		void *pBuf = bofAlloc((int)getRecSize(lRecNum));

		_errCode = readRecord(lRecNum, pBuf);
		bofFree(pBuf);
	}

	return _errCode;
}

ErrorCode CBofDataFile::verifyAllRecords() {
	assert(isValidObject(this));

	if (_errCode == ERR_NONE) {
		int32 n = getNumberOfRecs();
		for (int32 i = 0; i < n; i++) {
			_errCode = verifyRecord(i);
			if (_errCode != ERR_NONE) {
				break;
			}
		}
	}

	return _errCode;
}

ErrorCode CBofDataFile::addRecord(void *pBuf, int32 lLength, bool bUpdateHeader, uint32 lKey) {
	assert(isValidObject(this));

	// Only continue if there is no current error
	if (_errCode == ERR_NONE) {

		// Validate input
		assert(pBuf != nullptr);
		assert(lLength > 0);

		if (lLength > 0) {
			if (_stream == nullptr) {
				open();
			}

			if (_errCode == ERR_NONE) {
				_lNumRecs++;

				HeaderRec *pTmpHeader = new HeaderRec[(int)_lNumRecs];

				for (int i = 0; i < _lNumRecs; ++i) {
					pTmpHeader[i]._lOffset = pTmpHeader[i]._lLength = 0;
					pTmpHeader[i]._lCrc = pTmpHeader[i]._lKey = 0;
				}

				if (_pHeader != nullptr) {
					memcpy(pTmpHeader, _pHeader, (size_t)(HeaderRec::size() * (_lNumRecs - 1)));
					delete[] _pHeader;
				}

				_pHeader = pTmpHeader;

				int32 lRecNum = _lNumRecs - 1;
				HeaderRec *pCurRec = &_pHeader[lRecNum];
				int32 lPrevLength = HeadInfo::size();
				int32 lPrevOffset = 0;

				if (lRecNum != 0) {
					lPrevLength = _pHeader[lRecNum - 1]._lLength;
					lPrevOffset = _pHeader[lRecNum - 1]._lOffset;
				}

				pCurRec->_lLength = lLength;
				pCurRec->_lOffset = lPrevOffset + lPrevLength;

				writeRecord(lRecNum, pBuf, lLength, bUpdateHeader, lKey);
			}
		}
	}

	return _errCode;
}

int32 CBofDataFile::findRecord(uint32 lKey) {
	assert(isValidObject(this));

	// Assume no match
	int32 lRecNum = -1;

	// Only continue if there is no current error
	if (_errCode == ERR_NONE) {
		// Scan the header for the key matching the hash code
		for (int32 i = 0; i < _lNumRecs; i++) {
			// Header records must be valid
			assert(_pHeader != nullptr);

			if (_pHeader[i]._lKey == lKey) {
				lRecNum = i;
				break;
			}
		}
	}

	return lRecNum;
}

int32 CBofDataFile::getRecSize(int32 lRecNum) {
	assert(isValidObject(this));

	int32 lSize = -1;

	// Only continue if there is no current error
	if (_errCode == ERR_NONE) {
		// Validate record number
		assert(lRecNum >= 0 && lRecNum < _lNumRecs);

		assert(_pHeader != nullptr);

		lSize = _pHeader[lRecNum]._lLength;
	}

	return lSize;
}

int32 CBofDataFile::getMaxRecSize() const {
	assert(isValidObject(this));

	int32 lLargest = -1;

	// Only continue if there is no current error
	if (_errCode == ERR_NONE) {
		// Validate header
		assert(_pHeader != nullptr);

		for (int i = 0; i < (int)_lNumRecs; i++) {
			lLargest = MAX(lLargest, _pHeader[i]._lLength);
		}
	}

	return lLargest;
}

void CBofDataFile::setPassword(const char *pszPassword) {
	assert(isValidObject(this));
	_szPassWord[0] = '\0';

	if (pszPassword != nullptr) {
		assert(strlen(pszPassword) < MAX_PW_LEN);

		Common::strcpy_s(_szPassWord, pszPassword);
	}
}

ErrorCode CBofDataFile::read(void *pDestBuf, int32 lBytes) {
	return CBofFile::read(pDestBuf, lBytes);
}

ErrorCode CBofDataFile::read(HeadInfo &rec) {
	byte buf[16];
	ErrorCode errorCode = read(&buf[0], 16);

	Common::MemoryReadStream mem(buf, 16);
	Common::Serializer s(&mem, nullptr);
	rec.synchronize(s);

	return errorCode;
}

ErrorCode CBofDataFile::read(HeaderRec &rec) {
	byte buf[16];
	ErrorCode errorCode = read(&buf[0], 16);

	Common::MemoryReadStream mem(buf, 16);
	Common::Serializer s(&mem, nullptr);
	rec.synchronize(s);

	return errorCode;
}

ErrorCode CBofDataFile::write(const void *pSrcBuf, int32 lBytes) {
	return CBofFile::write(pSrcBuf, lBytes);
}

ErrorCode CBofDataFile::write(HeadInfo &rec) {
	byte buf[16];

	Common::MemoryWriteStream mem(buf, 16);
	Common::Serializer s(nullptr, &mem);
	rec.synchronize(s);

	return write(&buf[0], 16);
}

ErrorCode CBofDataFile::write(HeaderRec &rec) {
	byte buf[16];

	Common::MemoryWriteStream mem(buf, 16);
	Common::Serializer s(nullptr, &mem);
	rec.synchronize(s);

	return write(&buf[0], 16);
}

/**
 * Builds a Hash code based on a key.
 * @param pKey          Key
 * @return              Hash code
 */
uint32 CreateHashCode(const byte *pKey) {
	// validate input
	assert(pKey != nullptr);

	uint32 lCode = ((uint32) * pKey << 24) | ((uint32) * (pKey + 1) << 16) | ((uint32) * (pKey + 2) << 8) | *(pKey + 3);

	return lCode;
}

void SwapHeadInfo(HeadInfo *stHI) {
	// Macintosh is big endian, so we must swap our bytes
	stHI->_lNumRecs = SWAPLONG(stHI->_lNumRecs);
	stHI->_lAddress = SWAPLONG(stHI->_lAddress);
	stHI->_lFlags = SWAPLONG(stHI->_lFlags);
	stHI->_lFootCrc = SWAPLONG(stHI->_lFootCrc);
}

void SwapHeaderRec(HeaderRec *stHR, int nRecords) {
	HeaderRec *p = stHR;
	for (int i = 0; i < nRecords; i++) {
		p->_lOffset = SWAPLONG(p->_lOffset);
		p->_lLength = SWAPLONG(p->_lLength);
		p->_lCrc = SWAPLONG(p->_lCrc);
		p->_lKey = SWAPLONG(p->_lKey);

		p++;
	}
}

} // namespace SpaceBar
} // namespace Bagel
