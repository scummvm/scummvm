/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SCUMM_FILE_H
#define SCUMM_FILE_H

#include "common/file.h"
#include "common/stream.h"

#include "scumm/detection.h"

namespace Scumm {

class BaseScummFile : public Common::File {
protected:
	byte _encbyte;

public:
	BaseScummFile() : _encbyte(0) {}
	void setEnc(byte value) { _encbyte = value; }

	bool open(const Common::String &filename) override = 0;
	virtual bool openSubFile(const Common::String &filename) = 0;

	int32 pos() const override = 0;
	int32 size() const override = 0;
	bool seek(int32 offs, int whence = SEEK_SET) override = 0;

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

	void setSubfileRange(int32 start, int32 len);
	void resetSubfile();

public:
	ScummFile();

	bool open(const Common::String &filename) override;
	bool openSubFile(const Common::String &filename) override;

	void clearErr() override { _myEos = false; BaseScummFile::clearErr(); }

	bool eos() const override;
	int32 pos() const override;
	int32 size() const override;
	bool seek(int32 offs, int whence = SEEK_SET) override;
	uint32 read(void *dataPtr, uint32 dataSize) override;
};

class ScummDiskImage : public BaseScummFile {
private:
	Common::SeekableReadStream *_stream;
	byte _roomDisks[59], _roomTracks[59], _roomSectors[59];

	byte *_buf;

	const GameSettings _game;

	const Common::String _disk1, _disk2;
	int _openedDisk;

	int _numGlobalObjects;
	int _numRooms;
	int _numCostumes;
	int _numScripts;
	int _numSounds;
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

	bool open(const Common::String &filename) override;
	bool openSubFile(const Common::String &filename) override;

	void close() override;
	bool eos() const override { return _stream->eos(); }
	int32 pos() const override { return _stream->pos(); }
	int32 size() const override { return _stream->size(); }
	bool seek(int32 offs, int whence = SEEK_SET) override { return _stream->seek(offs, whence); }
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

	bool openWithSubRange(const Common::String &filename, int32 subFileStart, int32 subFileLen);
public:
	ScummSteamFile(const SteamIndexFile &indexFile) : ScummFile(), _indexFile(indexFile) {}

	bool open(const Common::String &filename) override;
};

} // End of namespace Scumm

#endif
