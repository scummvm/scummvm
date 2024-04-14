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

#include "bagel/boflib/dat_file.h"
#include "bagel/boflib/crc.h"
#include "bagel/boflib/debug.h"
#include "bagel/boflib/log.h"
#include "bagel/boflib/misc.h"
#include "bagel/boflib/stdinc.h"

namespace Bagel {

// Local prototypes
static uint32 CreateHashCode(const byte *);

void HEADER_REC::synchronize(Common::Serializer &s) {
	s.syncAsSint32LE(m_lOffset);
	s.syncAsSint32LE(m_lLength);
	s.syncAsUint32LE(m_lCrc);
	s.syncAsUint32LE(m_lKey);
}

void HEAD_INFO::synchronize(Common::Serializer &s) {
	s.syncAsSint32LE(m_lNumRecs);
	s.syncAsSint32LE(m_lAddress);
	s.syncAsUint32LE(m_lFlags);
	s.syncAsUint32LE(m_lFootCrc);
}


CBofDataFile::CBofDataFile() {
	m_szFileName[0] = '\0';
	m_szPassWord[0] = '\0';
	m_lHeaderLength = 0;
	m_lNumRecs = 0;
	m_pHeader = nullptr;
	m_bHeaderDirty = false;
}

CBofDataFile::CBofDataFile(const char *pszFileName, uint32 lFlags, const char *pPassword) {
	m_szFileName[0] = '\0';
	m_szPassWord[0] = '\0';
	m_lHeaderLength = 0;
	m_lNumRecs = 0;
	m_pHeader = nullptr;
	m_bHeaderDirty = false;

	SetFile(pszFileName, lFlags, pPassword);
}

ErrorCode CBofDataFile::SetFile(const char *pszFileName, uint32 lFlags, const char *pPassword) {
	Assert(IsValidObject(this));

	// Validate input
	Assert(pszFileName != nullptr);
	Assert(strlen(pszFileName) < MAX_FNAME);

	// Release any previous data-file
	ReleaseFile();

	// All data files are binary, so force it
	lFlags |= CBF_BINARY;

	// Store password locally
	//
	if (pPassword != nullptr) {
		// don't want to overwrite past our password buffer
		Assert(strlen(pPassword) < MAX_PW_LEN);

		Common::strcpy_s(m_szPassWord, pPassword);

		// Force encryption on since they supplied a password
		m_lFlags |= CDF_ENCRYPT;
	}

	// Remember the flags
	m_lFlags = lFlags;

	// Remember this file's name
	//
	// Hopefully this code won't stand... there are three places inside
	// of the following code that changes m_szFileName to a full path,
	// and each of them expects as input just the filename,
	// so carefully preserve the value of that buffer along the way.
#if BOF_MAC
	Common::strcpy_s(m_szFileName, pszFileName);
#else
	if (FileGetFullPath(m_szFileName, pszFileName) != nullptr) {
#endif
	if (Open() == ERR_NONE) {

		// Read header block
		ReadHeader();

		// Close data file if we are not keeping it open
		//
		if (!(m_lFlags & CDF_KEEPOPEN)) {
			Close();
		}
	}
#if !BOF_MAC
} else {
	ReportError(ERR_FFIND, "Could not build full path to %s", pszFileName);
}
#endif
	return m_errCode;
}

CBofDataFile::~CBofDataFile() {
	Assert(IsValidObject(this));

	ReleaseFile();
}

ErrorCode CBofDataFile::ReleaseFile() {
	Assert(IsValidObject(this));

	// If header was modified
	//
	if (m_bHeaderDirty) {
		// Write header to disk
		WriteHeader();
	}

	Close();

	// Free header buffer
	if (m_pHeader != nullptr) {
		delete[] m_pHeader;
		m_pHeader = nullptr;
	}

	return m_errCode;
}

ErrorCode CBofDataFile::Create() {
	Assert(IsValidObject(this));

	HEAD_INFO stHeaderInfo;

	// Only continue if there is no current error
	if (m_errCode == ERR_NONE) {
		if (_stream != nullptr) {
			Close();
		}

		// Re-initialize
		//
		if (m_pHeader != nullptr) {
			delete[] m_pHeader;
			m_pHeader = nullptr;
		}
		_stream = nullptr;
		m_lHeaderLength = 0;
		m_bHeaderDirty = false;

		stHeaderInfo.m_lNumRecs = m_lNumRecs = 0;
		stHeaderInfo.m_lAddress = HEAD_INFO::size();

#if BOF_WINMAC || BOF_MAC
		SwapHeadInfo(&stHeaderInfo);
#endif

		// create the file
		//
		if (CBofFile::Create(m_szFileName, m_lFlags) == ERR_NONE) {

			// write empty header info
			//
			if (Write(stHeaderInfo) == ERR_NONE) {

			} else {
				m_errCode = ERR_FWRITE;
			}

			Seek(0);

		} else {
			m_errCode = ERR_FOPEN;
		}
	}

	return m_errCode;
}

ErrorCode CBofDataFile::Open() {
#if BOF_MAC
	char pszFileName[MAX_FNAME];
#endif
	Assert(IsValidObject(this));

	// Only continue if there is no current error
	if (m_errCode == ERR_NONE) {
		if (_stream == nullptr) {
			if (!(m_lFlags & CDF_READONLY)) {
				if (m_lFlags & CDF_SAVEFILE) {
					if (m_lFlags & CDF_CREATE)
						Create();
				} else {
					if (!FileExists(m_szFileName))
						Create();
				}
			}

			if (_stream == nullptr) {
				// Open data file
				CBofFile::Open(m_szFileName, m_lFlags);
			}
		}
	}

	return m_errCode;
}

ErrorCode CBofDataFile::Close() {
	Assert(IsValidObject(this));

	if (_stream != nullptr) {
		if (m_bHeaderDirty) {
			WriteHeader();
		}

		CBofFile::Close();
	}

	return m_errCode;
}

ErrorCode CBofDataFile::ReadHeader() {
	Assert(IsValidObject(this));

	HEAD_INFO stHeaderInfo;
	uint32 lCrc;
	int32 lFileLength;

	// only continue if there is no current error
	//
	if (m_errCode == ERR_NONE) {

		if (_stream == nullptr) {
			Open();
		}

		if (!ErrorOccurred()) {

			// determine number of records in file
			//
			if (Read(stHeaderInfo) == ERR_NONE) {

#if BOF_WINMAC || BOF_MAC
				SwapHeadInfo(&stHeaderInfo);
#endif

				m_lNumRecs = stHeaderInfo.m_lNumRecs;
				m_lHeaderStart = stHeaderInfo.m_lAddress;

				// length of header is number of records * header-record size
				m_lHeaderLength = m_lNumRecs * HEADER_REC::size();

				auto *rs = dynamic_cast<Common::SeekableReadStream *>(_stream);
				assert(rs);
				lFileLength = rs->size();

				// Make sure header contains valid info
				if ((m_lHeaderStart >= HEAD_INFO::size()) &&
				        (m_lHeaderStart <= lFileLength) && (m_lHeaderLength >= 0) &&
				        (m_lHeaderLength < lFileLength)) {

					// force Encrypted, and Compress if existing file has them
					//
					m_lFlags |= stHeaderInfo.m_lFlags & CDF_ENCRYPT;
					m_lFlags |= stHeaderInfo.m_lFlags & CDF_COMPRESSED;

					if (m_lHeaderLength != 0) {

						// allocate buffer to hold header
						//
						// watch out!  Could be the metrowerks allocation bug
						// here! ???
						if ((m_pHeader = new HEADER_REC[(int)m_lNumRecs]) != nullptr) {

							// seek to start of header
							//
							Seek(m_lHeaderStart);

							// read header
							ErrorCode errCode = ERR_NONE;
							for (int i = 0; i < m_lNumRecs && errCode == ERR_NONE; ++i) {
								errCode = Read(m_pHeader[i]);
							}

							if (errCode == ERR_NONE) {
								lCrc = CalculateCRC(&m_pHeader->m_lOffset, 4 * m_lNumRecs);
#if BOF_WINMAC || BOF_MAC
								SwapHeaderRec(m_pHeader, m_lNumRecs);
#endif
								if (lCrc != stHeaderInfo.m_lFootCrc) {
									LogError(BuildString("Error: '%s' has invalid footer", m_szFileName));
									m_errCode = ERR_CRC;
								}

							} else {
								LogError(BuildString("Error: Could not read footer in file '%s'", m_szFileName));
								m_errCode = ERR_FREAD;
							}

						} else {
							LogError(BuildString("Error: Could not allocate footer for file '%s'", m_szFileName));
							m_errCode = ERR_MEMORY;
						}
					}

				} else {
					LogError(BuildString("Error: '%s' has invalid header", m_szFileName));
					m_errCode = ERR_FTYPE;
				}

			} else {
				LogError(BuildString("Error: Could not read header in file '%s'", m_szFileName));
				m_errCode = ERR_FREAD;
			}
		}
	}

	return m_errCode;
}

ErrorCode CBofDataFile::WriteHeader() {
	Assert(IsValidObject(this));

	HEAD_INFO stHeaderInfo;
	HEADER_REC *pRec;

	// only continue if there is no current error
	//
	if (m_errCode == ERR_NONE) {

		// open the data file if it's not already open
		//
		if (_stream == nullptr) {
			Open();
		}

		if (m_errCode == ERR_NONE) {

			// header starts at the end of the last record
			//
			pRec = &m_pHeader[m_lNumRecs - 1];
			m_lHeaderStart = pRec->m_lOffset + pRec->m_lLength;

			stHeaderInfo.m_lNumRecs = m_lNumRecs;
			stHeaderInfo.m_lAddress = m_lHeaderStart;
			stHeaderInfo.m_lFlags = m_lFlags;
#if BOF_WINMAC || BOF_MAC
			// Swap all the header recs before going to disk...
			SwapHeaderRec(m_pHeader, m_lNumRecs);
#endif
			stHeaderInfo.m_lFootCrc = CalculateCRC(&m_pHeader->m_lOffset, 4 * m_lNumRecs);

			// seek to front of file to write header info
			SeekToBeginning();

#if BOF_WINMAC || BOF_MAC
			// before going to disk, swap!
			SwapHeadInfo(&stHeaderInfo);
#endif
			if (Write(stHeaderInfo) == ERR_NONE) {

				// seek to start of where header is to be written
				//
				Seek(m_lHeaderStart);

				// Write header to data file
				ErrorCode errCode = ERR_NONE;
				for (int i = 0; i < m_lNumRecs && errCode == ERR_NONE; ++i) {
					errCode = Write(m_pHeader[i]);
				}

				if (errCode == ERR_NONE) {
					// Header is now clean
					m_bHeaderDirty = false;

				} else {
					LogError(BuildString("Error writing footer to file '%s'", m_szFileName));
					m_errCode = ERR_FWRITE;
				}

			} else {
				LogError(BuildString("Error writing header to file '%s'", m_szFileName));
				m_errCode = ERR_FWRITE;
			}
#if BOF_WINMAC || BOF_MAC
			// Swap all the header recs before going to disk...
			SwapHeaderRec(m_pHeader, m_lNumRecs);
#endif
		}
	}

	return m_errCode;
}

ErrorCode CBofDataFile::ReadRecord(int32 lRecNum, void *pBuf) {
	Assert(IsValidObject(this));

	HEADER_REC *pRecInfo;
	uint32 lCrc;

	// only continue if there is no current error
	//
	if (m_errCode == ERR_NONE) {

		// can't write to nullptr pointers
		Assert(pBuf != nullptr);

		// validate record number
		Assert(lRecNum >= 0 && lRecNum < m_lNumRecs);

		// make sure we have a valid header
		Assert(m_pHeader != nullptr);

		// get info about address of where record starts
		// and how large the record is.
		//
		pRecInfo = &m_pHeader[(int)lRecNum];

		// open the data file if it's not already open
		//
		if (_stream == nullptr) {

			Open();
		}

		if (m_errCode == ERR_NONE) {

			// seek to that point in the file
			//
			Seek(pRecInfo->m_lOffset);

			// read in the record
			//
			if (Read(pBuf, pRecInfo->m_lLength) == ERR_NONE) {

				// if this file is encrypted, then decrypt it
				//
				if (m_lFlags & CDF_ENCRYPT) {
					Decrypt(pBuf, (int)pRecInfo->m_lLength, m_szPassWord);
				}

				// calculate and verify this record's CRC value
				//
				lCrc = CalculateCRC(pBuf, (int)pRecInfo->m_lLength);

				if (lCrc != pRecInfo->m_lCrc) {
					m_errCode = ERR_CRC;
				}

			} else {
				LogError(BuildString("Error reading record %ld in file '%s'", lRecNum, m_szFileName));
				m_errCode = ERR_FREAD;
			}
		}
	}

	return m_errCode;
}

ErrorCode CBofDataFile::ReadFromFile(int32 lRecNum, void *pBuf, int32 lBytes) {
	Assert(IsValidObject(this));

	HEADER_REC *pRecInfo;

	// only continue if there is no current error
	//
	if (m_errCode == ERR_NONE) {

		// can't write to nullptr pointers
		Assert(pBuf != nullptr);

		// validate record number
		Assert(lRecNum >= 0 && lRecNum < m_lNumRecs);

		// make sure we have a valid header
		Assert(m_pHeader != nullptr);

		// get info about address of where record starts
		// and how large the record is.
		//
		pRecInfo = &m_pHeader[(int)lRecNum];

		// open the data file if it's not already open
		//
		if (_stream == nullptr) {

			Open();
		}

		if (m_errCode == ERR_NONE) {

			// seek to that point in the file
			//
			Seek(pRecInfo->m_lOffset);

			// read in the requested bytes...
			//
			if (Read(pBuf, lBytes) == ERR_NONE) {

				// if this file is encrypted, then decrypt it
				//
				if (m_lFlags & CDF_ENCRYPT) {
					DecryptPartial(pBuf, (int32)pRecInfo->m_lLength, (int32)lBytes, m_szPassWord);
				}

				// Don't bother with a CRC as this chunk of input won't generate a proper
				// CRC anyway.
			} else {
				LogError(BuildString("Error reading record %ld in file '%s'", lRecNum, m_szFileName));
				m_errCode = ERR_FREAD;
			}
		}
	}

	return m_errCode;
}

ErrorCode CBofDataFile::WriteRecord(int32 lRecNum, void *pBuf, int32 lSize, bool bUpdateHeader, uint32 lKey) {
	Assert(IsValidObject(this));

	HEADER_REC *pRecInfo;
	byte *pTmpBuf;
	int32 lPrevLength, lPrevOffset;
	int i;

	// only continue if there is no current error
	//
	if (m_errCode == ERR_NONE) {

		// validate record number
		Assert(lRecNum >= 0 && lRecNum < m_lNumRecs);

		// validate input buffer
		Assert(pBuf != nullptr);

		// there must already be a valid header
		Assert(m_pHeader != nullptr);

		if (lSize == -1)
			lSize = m_pHeader[(int)lRecNum].m_lLength;

		lPrevOffset = HEAD_INFO::size();
		lPrevLength = 0;

		if (lRecNum != 0) {
			lPrevOffset = m_pHeader[(int)lRecNum - 1].m_lOffset;
			lPrevLength = m_pHeader[(int)lRecNum - 1].m_lLength;
		}

		pRecInfo = &m_pHeader[(int)lRecNum];

		// header needs to updated
		m_bHeaderDirty = true;

		if (_stream == nullptr) {
			Open();
		}

		// this record starts at the end of the last record
		pRecInfo->m_lOffset = lPrevOffset + lPrevLength;

		// seek to where we want to write this record
		Seek(pRecInfo->m_lOffset);

		// calculate new hash code based on this records key
		//
		pRecInfo->m_lKey = lKey;
		if (lKey == 0xFFFFFFFF) {
			pRecInfo->m_lKey = CreateHashCode((const byte *)pBuf);
		}

		// calculate this record's CRC value
		pRecInfo->m_lCrc = CalculateCRC(pBuf, lSize);

		if (m_lFlags & CDF_ENCRYPT) {
			Encrypt(pBuf, lSize, m_szPassWord);
		}

		// If new record is larger then original
		//
		if (lSize > pRecInfo->m_lLength) {
			int32 lDiff;

			// How many bytes back do we have to write?
			lDiff = lSize - pRecInfo->m_lLength;

			int32 lBufLength, lChunkSize;

			//
			// Move the rest of file back that many bytes
			//

			// Read the rest of the file in chunks (of 200k or less),
			// and write each chunk back in it's new position.
			//
			lBufLength = GetLength() - (pRecInfo->m_lOffset + pRecInfo->m_lLength);
			lChunkSize = MIN(lBufLength, (int32)200000);

			// Allocate a buffer big enough for one chunk
			//
			if ((pTmpBuf = (byte *)BofAlloc(lChunkSize)) != nullptr) {

				// While there is data to move
				//
				while (lBufLength > 0) {

					// Seek to begining of the source for this chunk
					SetPosition(pRecInfo->m_lOffset + pRecInfo->m_lLength + lBufLength - lChunkSize);

					// Read the chunk
					Read(pTmpBuf, lChunkSize);

					// Seek to this chunks new positon (offset by 'lDiff' bytes)
					SetPosition(pRecInfo->m_lOffset + pRecInfo->m_lLength + lBufLength - lChunkSize + lDiff);

					// Write chunk to new position
					Write(pTmpBuf, lChunkSize);

					// That much less to do next time thru
					lBufLength -= lChunkSize;

					// Last chunk is lBufLength
					lChunkSize = MIN(lBufLength, lChunkSize);
				}

				// don't need that temp buffer anymore
				BofFree(pTmpBuf);

			} else {

				ReportError(ERR_MEMORY, "Unable to allocate %ld bytes to expand record size in WriteRecord()", lBufLength);
			}

			// Tell the rest of the records that they moved
			//
			for (i = lRecNum + 1; i < GetNumberOfRecs(); i++) {
				m_pHeader[i].m_lOffset += lDiff;
			}

			// remember it's new length
			pRecInfo->m_lLength = lSize;

			// seek to where we want to write this record
			Seek(pRecInfo->m_lOffset);

			// write this record
			//
			Write(pBuf, lSize);

			// if we are to update the header now
			//
			if (bUpdateHeader) {
				WriteHeader();
			}

		} else {

			// write this record
			//
			if (Write(pBuf, lSize) == ERR_NONE) {

				// if this record got smaller
				//
				if (pRecInfo->m_lLength > lSize) {

					// remember it's length
					pRecInfo->m_lLength = lSize;

					// allocate a buffer that could hold the largest record
					//
					if ((pTmpBuf = (byte *)BofAlloc((int)GetMaxRecSize())) != nullptr) {

						for (i = (int)lRecNum + 1; i < (int)m_lNumRecs - 1; i++) {

							if ((m_errCode = ReadRecord(i, pTmpBuf)) != ERR_NONE)
								break;

							if ((m_errCode = WriteRecord(i + 1, pTmpBuf)) != ERR_NONE)
								break;
						}

						BofFree(pTmpBuf);

					} else {
						m_errCode = ERR_MEMORY;
					}
				}

				// if we are to update the header now
				//
				if (bUpdateHeader) {
					WriteHeader();
				}

			} else {
				m_errCode = ERR_FWRITE;
			}
		}

		// if this record is encrypted the decrypt it
		//
		if (m_lFlags & CDF_ENCRYPT) {
			Decrypt(pBuf, (int)pRecInfo->m_lLength, m_szPassWord);
		}
	}

	return m_errCode;
}

ErrorCode CBofDataFile::VerifyRecord(int32 lRecNum) {
	Assert(IsValidObject(this));

	void *pBuf;

	if (m_errCode == ERR_NONE) {

		// validate record number
		Assert(lRecNum >= 0 && lRecNum < m_lNumRecs);

		// allocate space to hold this record
		//
		if ((pBuf = BofAlloc((int)GetRecSize(lRecNum))) != nullptr) {

			m_errCode = ReadRecord(lRecNum, pBuf);

			BofFree(pBuf);

		} else {
			m_errCode = ERR_MEMORY;
		}
	}

	return m_errCode;
}

ErrorCode CBofDataFile::VerifyAllRecords() {
	Assert(IsValidObject(this));

	int32 i, n;

	if (m_errCode == ERR_NONE) {

		n = GetNumberOfRecs();
		for (i = 0; i < n; i++) {

			if ((m_errCode = VerifyRecord(i)) != ERR_NONE) {
				break;
			}
		}
	}

	return m_errCode;
}

ErrorCode CBofDataFile::AddRecord(void *pBuf, int32 lLength, bool bUpdateHeader, uint32 lKey) {
	Assert(IsValidObject(this));

	HEADER_REC *pTmpHeader;
	HEADER_REC *pCurRec;
	int32 lRecNum;
	int32 lPrevLength;
	int32 lPrevOffset;

	// only continue if there is no current error
	//
	if (m_errCode == ERR_NONE) {

		// validate input
		Assert(pBuf != nullptr);
		Assert(lLength > 0);

		if (lLength > 0) {

			if (_stream == nullptr) {
				Open();
			}

			if (m_errCode == ERR_NONE) {

				m_lNumRecs++;

#if BOF_MAC && DEVELOPMENT && !__POWERPC__
				// there is a metrowerks code generation bug in the new
				// [] operator.  Just put a debug statement here so we're sure to examine
				// it next time we run this code.

				DebugStr("\ppTmpHeader = new HEADER_REC[(int)m_lNumRecs]");
#endif
				if ((pTmpHeader = new HEADER_REC[(int)m_lNumRecs]) != nullptr) {

					if (m_pHeader != nullptr) {

						BofMemCopy(pTmpHeader, m_pHeader, (size_t)(HEADER_REC::size() * (m_lNumRecs - 1)));

						delete[] m_pHeader;
					}

					m_pHeader = pTmpHeader;

					lRecNum = m_lNumRecs - 1;

					pCurRec = &m_pHeader[lRecNum];
					lPrevLength = HEAD_INFO::size();
					lPrevOffset = 0;
					if (lRecNum != 0) {
						lPrevLength = m_pHeader[lRecNum - 1].m_lLength;
						lPrevOffset = m_pHeader[lRecNum - 1].m_lOffset;
					}

					pCurRec->m_lLength = lLength;
					pCurRec->m_lOffset = lPrevOffset + lPrevLength;

					WriteRecord(lRecNum, pBuf, lLength, bUpdateHeader, lKey);

				} else {
					ReportError(ERR_MEMORY, "Could not allocate a data file header");
				}
			}
		}
	}

	return m_errCode;
}

ErrorCode CBofDataFile::DeleteRecord(int32 lRecNum, bool bUpdateHeader) {
	Assert(IsValidObject(this));

	//
	// I don't think this function works yet!
	//

	byte *pBuf;
	int i;

	// only continue if there is no current error
	//
	if (m_errCode == ERR_NONE) {

		// validate record number
		Assert(lRecNum >= 0 && lRecNum < m_lNumRecs);

		m_bHeaderDirty = true;

		// header has moved
		m_lHeaderStart -= m_pHeader[(int)lRecNum].m_lLength;

		// header has changed size
		BofMemMove(m_pHeader + lRecNum, m_pHeader + lRecNum + 1, (size_t)((m_lNumRecs - lRecNum - 1) * HEADER_REC::size()));

		// on less record
		m_lNumRecs--;

		m_lHeaderLength = m_lNumRecs * HEADER_REC::size();

		// open the data file if it's not already
		//
		if (_stream == nullptr) {
			Open();
		}

		if (m_errCode == ERR_NONE) {

			// allocate a buffer that could hold the largest record
			//
			if ((pBuf = (byte *)BofAlloc((int)GetMaxRecSize())) != nullptr) {

				// remove this record from the file
				//
				for (i = (int)lRecNum; i < (int)m_lNumRecs - 1; i++) {

					if ((m_errCode = ReadRecord(i + 1, pBuf)) != ERR_NONE)
						break;

					if ((m_errCode = WriteRecord(i, pBuf)) != ERR_NONE)
						break;
				}

				// if we are to update the header now
				//
				if (bUpdateHeader) {
					WriteHeader();
				}

				BofFree(pBuf);

			} else {
				m_errCode = ERR_MEMORY;
			}
		}
	}

	return m_errCode;
}

int32 CBofDataFile::FindRecord(uint32 lKey) {
	Assert(IsValidObject(this));

	int32 i, lRecNum;

	// assume no match
	lRecNum = -1;

	// only continue if there is no current error
	//
	if (m_errCode == ERR_NONE) {

		// scan the header for the key matching the hash code
		//
		for (i = 0; i < m_lNumRecs; i++) {

			// header records must be valid
			Assert(m_pHeader != nullptr);

			if (m_pHeader[i].m_lKey == lKey) {
				lRecNum = i;
				break;
			}
		}
	}

	return lRecNum;
}

int32 CBofDataFile::GetRecSize(int32 lRecNum) {
	Assert(IsValidObject(this));

	int32 lSize = -1;

	// only continue if there is no current error
	//
	if (m_errCode == ERR_NONE) {

		// validate record number
		Assert(lRecNum >= 0 && lRecNum < m_lNumRecs);

		Assert(m_pHeader != nullptr);

		lSize = m_pHeader[lRecNum].m_lLength;
	}

	return lSize;
}

int32 CBofDataFile::GetMaxRecSize() const {
	Assert(IsValidObject(this));

	int32 lLargest;
	int i;

	lLargest = -1;

	// only continue if there is no current error
	//
	if (m_errCode == ERR_NONE) {

		// validate header
		Assert(m_pHeader != nullptr);

		for (i = 0; i < (int)m_lNumRecs; i++) {
			lLargest = MAX(lLargest, m_pHeader[i].m_lLength);
		}
	}

	return lLargest;
}

void CBofDataFile::SetPassword(const char *pszPassword) {
	Assert(IsValidObject(this));

	m_szPassWord[0] = '\0';
	if (pszPassword != nullptr) {

		Assert(strlen(pszPassword) < MAX_PW_LEN);

		Common::strcpy_s(m_szPassWord, pszPassword);
	}
}

ErrorCode CBofDataFile::Read(void *pDestBuf, int32 lBytes) {
	return CBofFile::Read(pDestBuf, lBytes);
}

ErrorCode CBofDataFile::Read(HEAD_INFO &rec) {
	byte buf[16];
	ErrorCode result = Read(&buf[0], 16);

	Common::MemoryReadStream mem(buf, 16);
	Common::Serializer s(&mem, nullptr);
	rec.synchronize(s);

	return result;
}

ErrorCode CBofDataFile::Read(HEADER_REC &rec) {
	byte buf[16];
	ErrorCode result = Read(&buf[0], 16);

	Common::MemoryReadStream mem(buf, 16);
	Common::Serializer s(&mem, nullptr);
	rec.synchronize(s);

	return result;
}

ErrorCode CBofDataFile::Write(const void *pSrcBuf, int32 lBytes) {
	return CBofFile::Write(pSrcBuf, lBytes);
}

ErrorCode CBofDataFile::Write(HEAD_INFO &rec) {
	byte buf[16];

	Common::MemoryWriteStream mem(buf, 16);
	Common::Serializer s(nullptr, &mem);
	rec.synchronize(s);

	return Write(&buf[0], 16);
}

ErrorCode CBofDataFile::Write(HEADER_REC &rec) {
	byte buf[16];

	Common::MemoryWriteStream mem(buf, 16);
	Common::Serializer s(nullptr, &mem);
	rec.synchronize(s);

	return Write(&buf[0], 16);
}

/**
 * Builds a Hash code based on a key.
 * @param pKey          Key
 * @return              Hash code
 */
uint32 CreateHashCode(const byte *pKey) {
	uint32 lCode;

	// validate input
	Assert(pKey != nullptr);

	lCode = ((uint32) * pKey << 24) | ((uint32) * (pKey + 1) << 16) | ((uint32) * (pKey + 2) << 8) | *(pKey + 3);

	return lCode;
}

void SwapHeadInfo(HEAD_INFO *stHI) {
	// Macintosh is big endian, so we must swap our bytes
	//
	stHI->m_lNumRecs = SWAPLONG(stHI->m_lNumRecs);
	stHI->m_lAddress = SWAPLONG(stHI->m_lAddress);
	stHI->m_lFlags = SWAPLONG(stHI->m_lFlags);
	stHI->m_lFootCrc = SWAPLONG(stHI->m_lFootCrc);
}

void SwapHeaderRec(HEADER_REC *stHR, int nRecords) {
	HEADER_REC *p = stHR;
	for (int i = 0; i < nRecords; i++) {
		p->m_lOffset = SWAPLONG(p->m_lOffset);
		p->m_lLength = SWAPLONG(p->m_lLength);
		p->m_lCrc = SWAPLONG(p->m_lCrc);
		p->m_lKey = SWAPLONG(p->m_lKey);

		p++;
	}
}

} // namespace Bagel
