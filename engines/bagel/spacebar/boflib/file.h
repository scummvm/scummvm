
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

#include "common/algorithm.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/object.h"
#include "bagel/boflib/error.h"

namespace Bagel {
namespace SpaceBar {

#define CBF_TEXT 0x00000001
#define CBF_BINARY 0x00000002
#define CBF_READONLY 0x00000004
#define CBF_OVERWRITE 0x00000008
#define CBF_SHARED 0x00000010
#define CBF_CREATE 0x00000020
#define CBF_SAVEFILE 0x100

#define CBF_DEFAULT (CBF_BINARY | CBF_READONLY)

#define CBOFFILE_TEXT CBF_TEXT
#define CBOFFILE_READONLY CBF_READONLY
#define CBOFFILE_OVERWRITE CBF_OVERWRITE

#define CBOFFILE_DEFAULT CBF_DEFAULT

class SaveReadStream : public Common::SeekableReadStream {
private:
	Common::MemoryWriteStreamDynamic *_owner;
public:
	SaveReadStream(Common::MemoryWriteStreamDynamic *owner) : _owner(owner) {
	}

	bool eos() const override {
		return _owner->pos() >= _owner->size();
	}
	uint32 read(void *dataPtr, uint32 dataSize) override {
		int bytesToCopy = MIN<int>(dataSize, _owner->size() - _owner->pos());
		const byte *src = _owner->getData() + _owner->pos();
		Common::copy(src, src + bytesToCopy, (byte *)dataPtr);
		seek(bytesToCopy, SEEK_CUR);
		return bytesToCopy;
	}
};

/**
 * Used as a wrapper for writing out original saves using the console,
 * since it also does reads from the stream whilst open
*/
class SaveReadWriteStream : public Common::MemoryWriteStreamDynamic, SaveReadStream {
private:
	Common::WriteStream *_save;

public:
	SaveReadWriteStream(Common::WriteStream *save) :
		Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES),
		SaveReadStream(this), _save(save) {
	}
	~SaveReadWriteStream() {
		_save->write(getData(), Common::MemoryWriteStreamDynamic::size());
		delete _save;
	}

	int64 pos() const override {
		return Common::MemoryWriteStreamDynamic::pos();
	}
	bool seek(int64 offset, int whence = SEEK_SET) override {
		return Common::MemoryWriteStreamDynamic::seek(offset, whence);
	}
	int64 size() const override {
		return Common::MemoryWriteStreamDynamic::size();
	}
};

class CBofFile : public CBofObject, public CBofError {
protected:
	char _szFileName[MAX_FNAME];
	Common::Stream *_stream = nullptr;
	uint32 _lFlags = CBF_DEFAULT;

public:
	/**
	 * Default constructor
	 */
	CBofFile();

	/**
	 * Open a specified file for access
	 * @param pszFileName       Filename
	 * @param lFlags            Access flags
	 */
	CBofFile(const char *pszFileName, uint32 lFlags = CBF_DEFAULT);

	/**
	 * Destructor
	 */
	virtual ~CBofFile();

	/**
	 * Open specified file into this object
	 * @param pszFileName       Filename
	 * @param lFlags            Access flags
	 */
	ErrorCode open(const char *pszFileName, uint32 lFlags = CBF_DEFAULT);

	/**
	 * Creates specified file
	 * @param pszFileName       Filename
	 * @param lFlags            Access flags
	 */
	ErrorCode create(const char *pszFileName, uint32 lFlags = CBF_DEFAULT | CBF_CREATE);

	/**
	 * Close a currently open file
	 */
	virtual ErrorCode close();

	/**
	 * Read from a currently open file
	 * @param pDestBuf      Destination buffer
	 * @param lBytes        Number of bytes
	 * @return              Error code
	 */
	virtual ErrorCode read(void *pDestBuf, int32 lBytes);

	/**
	 * Write to a currently open file
	 * @param pSrcBuf       Source buffer
	 * @param lBytes        Number of bytes
	 * @return              Error code
	 */
	virtual ErrorCode write(const void *pSrcBuf, int32 lBytes);

	/**
	 * Flushes I/O stream
	 */
	void commit();

	/**
	 * Seek to a specified location in the file
	 * @return      Error code
	 */
	ErrorCode seek(uint32 lPos) {
		return (setPosition(lPos));
	}

	/**
	 * Sets the file pointer to the beginning of the file
	 * @return      Error code
	 */
	ErrorCode seekToBeginning() {
		return (setPosition(0));
	}

	/**
	 * Sets the file pointer to the end of the file
	 * @return      Error code
	 */
	ErrorCode seekToEnd();

	/**
	 * Sets the current file-seek position to that specified
	 * @param lPos      New position
	 */
	ErrorCode setPosition(uint32 lPos);

	/**
	 * Retrieves the current seek position
	 */
	uint32 getPosition();

	/**
	 * Get the length of a file
	 */
	uint32 getLength();

	operator Common::SeekableReadStream *() const {
		return dynamic_cast<Common::SeekableReadStream *>(_stream);
	}
};

} // namespace SpaceBar
} // namespace Bagel

#endif
