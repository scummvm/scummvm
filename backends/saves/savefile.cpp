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

#include "common/util.h"
#include "common/savefile.h"
#include "common/str.h"
#if defined(USE_CLOUD) && defined(USE_LIBCURL)
#include "backends/cloud/cloudmanager.h"
#endif

namespace Common {

OutSaveFile::OutSaveFile(WriteStream *w): _wrapped(w) {}

OutSaveFile::~OutSaveFile() {
	delete _wrapped;
#if defined(USE_CLOUD) && defined(USE_LIBCURL)
	CloudMan.syncSaves();
#endif
}

bool OutSaveFile::err() const { return _wrapped->err(); }

void OutSaveFile::clearErr() { _wrapped->clearErr(); }

void OutSaveFile::finalize() {
	_wrapped->finalize();
}

bool OutSaveFile::flush() { return _wrapped->flush(); }

uint32 OutSaveFile::write(const void *dataPtr, uint32 dataSize) {
	return _wrapped->write(dataPtr, dataSize);
}

int64 OutSaveFile::pos() const {
	return _wrapped->pos();
}

bool OutSaveFile::seek(int64 offset, int whence) {
	Common::SeekableWriteStream *sws =
		dynamic_cast<Common::SeekableWriteStream *>(_wrapped);

	if (sws) {
		return sws->seek(offset, whence);
	} else {
		warning("Seeking isn't supported for compressed save files");
		return false;
	}
}

int64 OutSaveFile::size() const {
	Common::SeekableWriteStream *sws =
		dynamic_cast<Common::SeekableWriteStream *>(_wrapped);

	if (sws) {
		return sws->size();
	} else {
		warning("Size isn't supported for compressed save files");
		return -1;
	}
}

bool SaveFileManager::copySavefile(const String &oldFilename, const String &newFilename, bool compress) {
	InSaveFile *inFile = nullptr;
	OutSaveFile *outFile = nullptr;
	uint32 size = 0;
	void *buffer = nullptr;
	bool success = false;

	inFile = openForLoading(oldFilename);

	if (inFile) {
		size = inFile->size();
		buffer = malloc(size);
		assert(buffer);

		outFile = openForSaving(newFilename, compress);

		if (buffer && outFile) {
			inFile->read(buffer, size);
			bool error = inFile->err();
			delete inFile;
			inFile = nullptr;

			if (!error) {
				outFile->write(buffer, size);
				outFile->finalize();

				success = !outFile->err();
			}
		}

		free(buffer);
		delete outFile;
		delete inFile;
	}

	return success;
}

bool SaveFileManager::renameSavefile(const String &oldFilename, const String &newFilename, bool compress) {
	if (!copySavefile(oldFilename, newFilename, compress))
		return false;

	return removeSavefile(oldFilename);
}

String SaveFileManager::popErrorDesc() {
	String err = _errorDesc;
	clearError();

	return err;
}

} // End of namespace Common
