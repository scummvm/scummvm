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

#include "common/macresman.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/savefile.h"
#include "bagel/bagel.h"
#include "bagel/spacebar/boflib/file.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/spacebar/boflib/debug.h"
#include "bagel/boflib/log.h"

namespace Bagel {
namespace SpaceBar {

#define CHUNK_SIZE 0x00007FFF

CBofFile::CBofFile() {
	_szFileName[0] = '\0';
}

CBofFile::CBofFile(const char *pszFileName, uint32 lFlags) {
	_szFileName[0] = '\0';
	assert(pszFileName != nullptr);

	// Open now?
	if (pszFileName != nullptr) {
		open(pszFileName, lFlags);
	}
}

CBofFile::~CBofFile() {
	assert(isValidObject(this));

	close();
}

ErrorCode CBofFile::create(const char *pszFileName, uint32 lFlags) {
	assert(isValidObject(this));
	assert(pszFileName != nullptr);
	assert(strlen(pszFileName) < MAX_DIRPATH);
	assert(*pszFileName != '\0');

	// Can't create a read-only file
	assert(!(lFlags & CBF_READONLY));

	_lFlags = lFlags;

	// Remember this files name
	Common::strcpy_s(_szFileName, pszFileName);

	// Create the file
	Common::OutSaveFile *save = g_system->getSavefileManager()->openForSaving(pszFileName, false);
	if (save != nullptr) {
		_stream = new SaveReadWriteStream(save);

		if (g_pDebugOptions != nullptr && g_pDebugOptions->_bShowIO) {
			logInfo(buildString("Creating file '%s'", _szFileName));
		}

	} else {
		reportError(ERR_FOPEN, "Unable to create %s", _szFileName);
	}

	return _errCode;
}

ErrorCode CBofFile::open(const char *pszFileName, uint32 lFlags) {
	assert(isValidObject(this));
	assert(pszFileName != nullptr);
	assert(strlen(pszFileName) < MAX_DIRPATH);
	assert(*pszFileName != '\0');

	// Can't open for both Text and Binary modes
	assert(!((lFlags & CBF_TEXT) && (lFlags & CBF_BINARY)));

	// Can't overwrite a readonly file
	assert(!((lFlags & CBF_READONLY) && (lFlags & CBF_OVERWRITE)));

	// Can't create a new file to be readonly (there would be nothing to read!)
	assert(!((lFlags & CBF_READONLY) && (lFlags & CBF_CREATE)));

	// Keep a copy of these flags
	_lFlags = lFlags;

	if (_stream)
		return _errCode;

	if ((lFlags & CBF_CREATE) && ((lFlags & CBF_SAVEFILE) ||
	                              !fileExists(pszFileName))) {
		create(pszFileName, lFlags);

	} else {
		// Remember this files' name
		Common::strcpy_s(_szFileName, pszFileName);

		if (lFlags & CBF_SAVEFILE) {
			_stream = g_system->getSavefileManager()->openForLoading(pszFileName);

			if (!_stream)
				reportError(ERR_FOPEN, "Could not open %s", pszFileName);

		} else {
			if (g_engine->getPlatform() == Common::kPlatformMacintosh) {
				_stream = Common::MacResManager::openFileOrDataFork(pszFileName);
			} else {
				_stream = SearchMan.createReadStreamForMember(pszFileName);
			}

			if (_stream) {
				if (g_pDebugOptions != nullptr && g_pDebugOptions->_bShowIO) {
					logInfo(buildString("Opened file '%s'", _szFileName));
				}
			} else {
				reportError(ERR_FOPEN, "Could not open %s", pszFileName);
			}
		}
	}

	return _errCode;
}

ErrorCode CBofFile::close() {
	assert(isValidObject(this));

	if (_stream != nullptr) {
		if (g_pDebugOptions != nullptr && g_pDebugOptions->_bShowIO) {
			logInfo(buildString("Closed file '%s'", _szFileName));
		}

		delete _stream;
		_stream = nullptr;
	}

	return ERR_NONE;
}

ErrorCode CBofFile::read(void *pDestBuf, int32 lBytes) {
	assert(isValidObject(this));
	assert(pDestBuf != nullptr);
	assert(lBytes >= 0);

	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(_stream);
	assert(rs);

	if (!errorOccurred()) {
		if (rs != nullptr) {
			byte *pBuf = (byte *)pDestBuf;

			while (lBytes > 0) {
				int nLength = (int)MIN(lBytes, (int32)CHUNK_SIZE);
				lBytes -= CHUNK_SIZE;

				if ((int)rs->read(pBuf, nLength) != nLength) {
					reportError(ERR_FREAD, "Unable to read %d bytes from %s", nLength, _szFileName);
				}

				pBuf += nLength;
			}

		} else {
			error("Attempt to read from a file that is not open for reading: %s", _szFileName);
		}
	}

	return _errCode;
}

ErrorCode CBofFile::write(const void *pSrcBuf, int32 lBytes) {
	assert(isValidObject(this));

	Common::WriteStream *ws = dynamic_cast<Common::WriteStream *>(_stream);

	if (ws != nullptr) {
		// As long as this file is not set for readonly, then write the buffer
		if (!(_lFlags & CBF_READONLY)) {
			const byte *pBuf = (const byte *)pSrcBuf;

			while (lBytes > 0) {
				int nLength = (int)MIN(lBytes, (int32)CHUNK_SIZE);
				lBytes -= CHUNK_SIZE;

				if ((int)ws->write(pBuf, nLength) != nLength) {
					reportError(ERR_FWRITE, "Unable to write %d bytes to %s", nLength, _szFileName);
				}

				pBuf += nLength;
			}

			// Flush this file's buffer back out right now
			commit();

		} else {
			logWarning(buildString("Attempted to write to the READONLY file '%s'", _szFileName));
		}

	} else {
		logWarning("Attempt to write to a file that is not open");
	}

	return _errCode;
}

ErrorCode CBofFile::setPosition(uint32 lPos) {
	assert(isValidObject(this));

	// Only supports files up to 2Gig
	assert(lPos < 0x80000000);

	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(_stream);
	Common::SeekableWriteStream *ws = dynamic_cast<Common::SeekableWriteStream *>(_stream);

	if (rs && !rs->seek(lPos)) {
		reportError(ERR_FSEEK, "Unable to seek to %u in rs", lPos);
	}

	if (ws && !ws->seek(lPos)) {
		reportError(ERR_FSEEK, "Unable to seek to %u in ws", lPos);
	}

	return _errCode;
}

uint32 CBofFile::getPosition() {
	assert(isValidObject(this));

	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(_stream);
	Common::SeekableWriteStream *ws = dynamic_cast<Common::SeekableWriteStream *>(_stream);

	if (rs)
		return rs->pos();

	if (ws)
		return ws->pos();

	error("getPosition on closed file");
}

ErrorCode CBofFile::seekToEnd() {
	assert(isValidObject(this));

	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(_stream);
	Common::SeekableWriteStream *ws = dynamic_cast<Common::SeekableWriteStream *>(_stream);

	if (rs)
		rs->seek(0, SEEK_END);
	else if (ws)
		ws->seek(0, SEEK_END);
	else
		error("Seek in closed file");

	return _errCode;
}

uint32 CBofFile::getLength() {
	assert(isValidObject(this));

	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(_stream);
	Common::SeekableWriteStream *ws = dynamic_cast<Common::SeekableWriteStream *>(_stream);

	if (rs)
		return rs->size();
	if (ws)
		return ws->size();

	error("getLength in closed file");
}

void CBofFile::commit() {
	assert(isValidObject(this));

	Common::SeekableWriteStream *ws = dynamic_cast<Common::SeekableWriteStream *>(_stream);
	if (ws)
		ws->finalize();
}

} // namespace SpaceBar
} // namespace Bagel
