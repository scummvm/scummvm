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
#include "common/str.h"

namespace Voyeur {

class VoyeurEngine;
class BoltFile;
class BoltGroup;
class BoltEntry;
#define DECOMPRESS_SIZE 0x7000

typedef void (BoltFile::*BoltMethodPtr)();

class BoltFile {
private:
	static BoltFile *_curLibPtr;
	static BoltGroup *_curGroupPtr;
	static BoltEntry *_curMemberPtr;
	static byte *_curMemInfoPtr;
	static int _fromGroupFlag;
	static byte _xorMask;
	static bool _encrypt;
	// TODO: Move decompression statics and methods into BoltEntry
	static int _curFilePosition;
	static int _bufferEnd;
	static int _bufferBegin;
	static int _bytesLeft;
	static int _bufSize;
	static byte *_bufStart;
	static byte *_bufPos;
	static byte _decompressBuf[DECOMPRESS_SIZE];
	static int _historyIndex;
	static byte _historyBuffer[0x200];
	static int _runLength;
	static int _decompState;
	static int _runType;
	static int _runValue;
	static int _runOffset;
	static const BoltMethodPtr _fnInitType[25];
private:
	Common::File _curFd;
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

	// Decompression
	byte *decompress(byte *buf, int size, int mode);
	void nextBlock();
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
	BoltFile();
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
public:
	BoltEntry(Common::SeekableReadStream *f);
	virtual ~BoltEntry();

	void load();
};

class FilesManager {
private:
	int _decompressSize;
public:
	BoltFile *_curLibPtr;
public:
	FilesManager();

	bool openBoltLib(const Common::String &filename, BoltFile *&boltFile);
};

} // End of namespace Voyeur

#endif /* VOYEUR_FILES_H */
