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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef VOYEUR_FILES_H
#define VOYEUR_FILES_H

#include "common/scummsys.h"
#include "common/file.h"
#include "common/rect.h"
#include "common/str.h"

namespace Voyeur {

class VoyeurEngine;
class BoltFile;
class BoltGroup;
class BoltEntry;
class PictureResource;
#define DECOMPRESS_SIZE 0x7000

typedef void (BoltFile::*BoltMethodPtr)();

class BoltFilesState {
public:
	BoltFile *_curLibPtr;
	BoltGroup *_curGroupPtr;
	BoltEntry *_curMemberPtr;
	byte *_curMemInfoPtr;
	int _fromGroupFlag;
	byte _xorMask;
	bool _encrypt;
	int _curFilePosition;
	int _bufferEnd;
	int _bufferBegin;
	int _bytesLeft;
	int _bufSize;
	byte *_bufStart;
	byte *_bufPos;
	byte _decompressBuf[DECOMPRESS_SIZE];
	int _historyIndex;
	byte _historyBuffer[0x200];
	int _runLength;
	int _decompState;
	int _runType;
	int _runValue;
	int _runOffset;
	Common::File _curFd;
public:
	BoltFilesState();

	byte *decompress(byte *buf, int size, int mode);
	void nextBlock();
};

class BoltFile {
private:
	static const BoltMethodPtr _fnInitType[25];
private:
	BoltFilesState &_state;
	Common::Array<BoltGroup> _groups;

	// initType method table
	void initDefault();
	void sInitPic();
	void vInitCMap();
	void vInitCycl();
	void initViewPort();
	void initViewPortList();
	void initFontInfo();
	void initSoundMap();
private:
	void resolveAll() {}
	byte *getBoltMember(uint32 id);

	void initType() {}
	void termType() {}
	void initMem(int id) {}
	void termMem() {}
	void initGro() {}
	void termGro() {}
public:
	BoltFile(BoltFilesState &state);
	~BoltFile();

	bool getBoltGroup(uint32 id);
	byte *memberAddr(uint32 id);
};

class BoltGroup {
private:
	Common::SeekableReadStream *_file;
public:
	byte _loaded;
	bool _processed;
	bool _callInitGro;
	int _count;
	int _fileOffset;
	Common::Array<BoltEntry> _entries;
public:
	BoltGroup(Common::SeekableReadStream *f); 

	void load();
};


class BoltEntry {
private:
	Common::SeekableReadStream *_file;
public:
	byte _mode;
	byte _field1;
	byte _initMethod;
	int _fileOffset;
	byte _xorMask;
	int _size;
	byte *_data;

	PictureResource *_picResource;
public:
	BoltEntry(Common::SeekableReadStream *f);
	virtual ~BoltEntry();

	void load();
};

class FilesManager {
private:
	int _decompressSize;
public:
	BoltFilesState _boltFilesState;
	BoltFile *_curLibPtr;
public:
	FilesManager();

	bool openBoltLib(const Common::String &filename, BoltFile *&boltFile);
};

class PictureResource {
	uint16 _flags;
	byte _select;
	byte _pick;
	byte _onOff;
	byte _depth;
	Common::Point _offset;
	int _width;
	int _height;
	uint32 _maskData;
	uint16 _planeSize;

	byte *_imgData;
public:
	PictureResource(BoltFilesState &state, const byte *src);
	virtual ~PictureResource();
};

} // End of namespace Voyeur

#endif /* VOYEUR_FILES_H */
