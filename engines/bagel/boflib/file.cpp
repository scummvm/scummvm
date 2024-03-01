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

#include "common/file.h"
#include "common/system.h"
#include "common/savefile.h"
#include "bagel/boflib/file.h"
#include "bagel/boflib/debug.h"

namespace Bagel {


#define CHUNK_SIZE 0x00007FFF

CBofFile::CBofFile() {
	m_szFileName[0] = '\0';
}

CBofFile::CBofFile(const CHAR *pszFileName, ULONG lFlags) {
	m_szFileName[0] = '\0';
	Assert(pszFileName != nullptr);

	// Open now?
	if (pszFileName != nullptr) {
		Open(pszFileName, lFlags);
	}
}

CBofFile::~CBofFile() {
	Assert(IsValidObject(this));

	Close();
}

ERROR_CODE CBofFile::Create(const CHAR *pszFileName, ULONG lFlags) {
	Assert(IsValidObject(this));
	Assert(pszFileName != nullptr);
	Assert(strlen(pszFileName) < MAX_DIRPATH);
	Assert(*pszFileName != '\0');

	// can't create a read-only file
	Assert(!(lFlags & CBF_READONLY));

	m_lFlags = lFlags;

	// Remember this files name
	Common::strcpy_s(m_szFileName, pszFileName);

	// Create the file
	if ((_stream = g_system->getSavefileManager()->openForSaving(pszFileName, false)) != nullptr) {
#if BOF_DEBUG
		if (g_pDebugOptions != nullptr && g_pDebugOptions->m_bShowIO) {
			LogInfo(BuildString("Creating file '%s'", m_szFileName));
		}
#endif

	} else {
		ReportError(ERR_FOPEN, "Unable to create %s", m_szFileName);
	}

	return m_errCode;
}

ERROR_CODE CBofFile::Open(const CHAR *pszFileName, ULONG lFlags) {
	Assert(IsValidObject(this));
	Assert(pszFileName != nullptr);
	Assert(strlen(pszFileName) < MAX_DIRPATH);
	Assert(*pszFileName != '\0');

	// can't open for both Text and Binary modes
	Assert(!((lFlags & CBF_TEXT) && (lFlags & CBF_BINARY)));

	// can't overwrite a readonly file
	Assert(!((lFlags & CBF_READONLY) && (lFlags & CBF_OVERWRITE)));

	// can't create a new file to be readonly (there would be nothing to read!)
	Assert(!((lFlags & CBF_READONLY) && (lFlags & CBF_CREATE)));

	// keep a copy of these flags
	m_lFlags = lFlags;

	// Change the order of evaluation, this causes file exists to only
	// be called in the instance when we were requested to create something, which
	// happens only 8 times during the startup of the game (whereas this code is
	// called 164 times).
	if ((lFlags & CBF_CREATE) && !Common::File::exists(pszFileName)) {
		Create(pszFileName, lFlags);

	} else {
		// Remember this files name
		Common::strcpy_s(m_szFileName, pszFileName);
		Common::File *f = new Common::File();

		if (f->open(pszFileName)) {
			_stream = f;
#if BOF_DEBUG
			if (g_pDebugOptions != nullptr && g_pDebugOptions->m_bShowIO) {
				LogInfo(BuildString("Opened file '%s'", m_szFileName));
			}
#endif
		} else {
			delete f;
			ReportError(ERR_FOPEN, "Could not open %s", pszFileName);
		}
	}

	return m_errCode;
}

VOID CBofFile::Close() {
	Assert(IsValidObject(this));

	if (_stream != nullptr) {
#if BOF_DEBUG
		if (g_pDebugOptions != nullptr && g_pDebugOptions->m_bShowIO) {
			LogInfo(BuildString("Closed file '%s'", m_szFileName));
		}
#endif
		delete _stream;
		_stream = nullptr;
	}
}

ERROR_CODE CBofFile::Read(VOID HUGE *pDestBuf, LONG lBytes) {
	Assert(IsValidObject(this));
	Assert(pDestBuf != nullptr);
	Assert(lBytes >= 0);

	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(_stream);
	assert(rs);

	if (!ErrorOccurred()) {
		if (rs != nullptr) {
			UBYTE HUGE *pBuf;
			INT nLength;
			INT nTotalBytes = 0;

			pBuf = (UBYTE HUGE *)pDestBuf;

			while (lBytes > 0) {
				nLength = (INT)min(lBytes, CHUNK_SIZE);
				lBytes -= CHUNK_SIZE;

				if ((int)rs->read(pBuf, nLength) != nLength) {
					ReportError(ERR_FREAD, "Unable to read %d bytes from %s", nLength, m_szFileName);
				}

				pBuf += nLength;
				nTotalBytes += nLength;
			}

		} else {
			error("Attempt to read from a file that is not open for reading: %s", m_szFileName);
		}
	}

	return m_errCode;
}

ERROR_CODE CBofFile::Write(const VOID HUGE *pSrcBuf, LONG lBytes) {
	Assert(IsValidObject(this));

	Common::WriteStream *ws = dynamic_cast<Common::WriteStream *>(_stream);

	if (ws != nullptr) {
		// As long as this file is not set for readonly, then write the buffer
		if (!(m_lFlags & CBF_READONLY)) {
			const UBYTE HUGE *pBuf;
			INT nLength;
			pBuf = (const UBYTE HUGE *)pSrcBuf;

			while (lBytes > 0) {
				nLength = (INT)min(lBytes, CHUNK_SIZE);
				lBytes -= CHUNK_SIZE;

				if ((int)ws->write(pBuf, nLength) != nLength) {
					ReportError(ERR_FWRITE, "Unable to write %d bytes to %s", nLength, m_szFileName);
				}

				pBuf += nLength;
			}

			// Flush this file's buffer back out right now
			Commit();

		} else {
			LogWarning(BuildString("Attempted to write to the READONLY file '%s'", m_szFileName));
		}

	} else {
		LogWarning("Attempt to write to a file that is not open");
	}

	return (m_errCode);
}

ERROR_CODE CBofFile::SetPosition(ULONG lPos) {
	Assert(IsValidObject(this));

	// only supports files up to 2Gig
	Assert(lPos < 0x80000000);

	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(_stream);
	Common::SeekableWriteStream *ws = dynamic_cast<Common::SeekableWriteStream *>(_stream);

	if (rs) {
		if (!rs->seek(lPos))
			ReportError(ERR_FSEEK, "Unable to seek to %ld", lPos);
	} else if (ws) {
		if (!ws->seek(lPos))
			ReportError(ERR_FSEEK, "Unable to seek to %ld", lPos);
	} else {
		ReportError(ERR_FSEEK, "Unable to seek to %ld", lPos);
	}

	return m_errCode;
}

ULONG CBofFile::GetPosition() {
	Assert(IsValidObject(this));

	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(_stream);
	Common::SeekableWriteStream *ws = dynamic_cast<Common::SeekableWriteStream *>(_stream);

	if (rs)
		return rs->pos();
	if (ws)
		return ws->pos();

	error("GetPosition on closed file");
}

ERROR_CODE CBofFile::SeekToEnd() {
	Assert(IsValidObject(this));

	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(_stream);
	Common::SeekableWriteStream *ws = dynamic_cast<Common::SeekableWriteStream *>(_stream);

	if (rs)
		rs->seek(0, SEEK_END);
	else if (ws)
		ws->seek(0, SEEK_END);
	else
		error("Seek in closed file");

	return m_errCode;
}

ERROR_CODE CBofFile::SetLength(ULONG /*lNewLength*/) {
	Assert(IsValidObject(this));

	LogWarning("CBofFile::SetLength() is not yet supported");

	return m_errCode;
}

ULONG CBofFile::GetLength() {
	Assert(IsValidObject(this));

	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(_stream);
	Common::SeekableWriteStream *ws = dynamic_cast<Common::SeekableWriteStream *>(_stream);

	if (rs)
		return rs->size();
	if (ws)
		return ws->size();

	error("GetLength in closed file");
}

VOID CBofFile::Commit() {
	Assert(IsValidObject(this));

	Common::SeekableWriteStream *ws = dynamic_cast<Common::SeekableWriteStream *>(_stream);
	if (ws)
		ws->finalize();
}

} // namespace Bagel
