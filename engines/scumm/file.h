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

#ifndef SCUMM_FILE_H
#define SCUMM_FILE_H

#include "common/file.h"
#include "common/stream.h"

#include "scumm/detection.h"

namespace Scumm {

class ScummEngine;

class BaseScummFile : public Common::SeekableReadStream {
protected:
	byte _encbyte;
	Common::ScopedPtr<Common::SeekableReadStream> _baseStream;
	Common::String _debugName;

public:
	BaseScummFile() : _encbyte(0) {}
	void setEnc(byte value) { _encbyte = value; }

	virtual bool open(const Common::Path &filename) = 0;
	virtual bool openSubFile(const Common::Path &filename) = 0;
	virtual void close();

	int64 pos() const override = 0;
	int64 size() const override = 0;
	bool seek(int64 offs, int whence = SEEK_SET) override = 0;

	Common::String getDebugName() const { return _debugName; }

	bool isOpen() const { return !!_baseStream; }

// Unused
#if 0
	virtual bool eos() const = 0;
	virtual uint32 read(void *dataPtr, uint32 dataSize) = 0;
#endif
};

class ScummFile : public BaseScummFile {
protected:
	int32	_subFileStart;
	int32	_subFileLen;
	bool	_myEos; // Have we read past the end of the subfile?
	bool    _isMac;

	void setSubfileRange(int32 start, int32 len);
	void resetSubfile();

public:
	explicit ScummFile(const ScummEngine *vm);

	bool open(const Common::Path &filename) override;
	bool openSubFile(const Common::Path &filename) override;

	void clearErr() override { _myEos = false; BaseScummFile::clearErr(); }

	bool eos() const override;
	int64 pos() const override;
	int64 size() const override;
	bool seek(int64 offs, int whence = SEEK_SET) override;
	uint32 read(void *dataPtr, uint32 dataSize) override;
};

class ScummDiskImage : public BaseScummFile {
private:
	Common::SeekableReadStream *_stream;
	byte _roomDisks[59] = {}, _roomTracks[59] = {}, _roomSectors[59] = {};

	byte *_buf = nullptr;

	const GameSettings _game;

	const Common::String _disk1, _disk2;
	int _openedDisk = 0;

	int _numGlobalObjects = 0;
	int _numRooms = 0;
	int _numCostumes = 0;
	int _numScripts = 0;
	int _numSounds = 0;
	const int *_resourcesPerFile;

	bool openDisk(char num);

	bool generateIndex();
	bool generateResource(int res);

	uint16 extractIndex(Common::WriteStream *out);
	uint16 extractResource(Common::WriteStream *out, int res);

	byte fileReadByte();
	uint16 fileReadUint16LE();

public:
	ScummDiskImage(const char *disk1, const char *disk2, GameSettings game);

	bool open(const Common::Path &filename) override;
	bool openSubFile(const Common::Path &filename) override;

	void close() override;
	bool eos() const override { return _stream->eos(); }
	int64 pos() const override { return _stream->pos(); }
	int64 size() const override { return _stream->size(); }
	bool seek(int64 offs, int whence = SEEK_SET) override { return _stream->seek(offs, whence); }
	uint32 read(void *dataPtr, uint32 dataSize) override;
};

struct SteamIndexFile {
	byte id;
	Common::Platform platform;
	const char *pattern;
	const char *indexFileName;
	const char *executableName;
	int32 start;
	int32 len;
};

class ScummSteamFile : public ScummFile {
private:
	const SteamIndexFile &_indexFile;

	bool openWithSubRange(const Common::Path &filename, int32 subFileStart, int32 subFileLen);
public:
	ScummSteamFile(const ScummEngine *vm, const SteamIndexFile &indexFile) : ScummFile(vm), _indexFile(indexFile) {}

	bool open(const Common::Path &filename) override;
};

} // End of namespace Scumm

#endif
