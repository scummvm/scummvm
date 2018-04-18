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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "lilliput/lilliput.h"
#include "lilliput/sound.h"

#include "common/debug.h"

namespace Lilliput {

LilliputSound::LilliputSound(LilliputEngine *vm) : _vm(vm) {
	_unpackedFiles = nullptr;
	_unpackedSizes = nullptr;
}

LilliputSound::~LilliputSound() {
	if (_unpackedFiles) {
		for (int i = 0; i < _fileNumb; i++)
			free(_unpackedFiles[i]);
	}
	free(_unpackedFiles);
	free(_unpackedSizes);
}

byte LilliputSound::readByte(const byte *data, uint32 offset) {
	uint16 al = data[0x201 + (offset >> 1)];
	return data[1 + (offset & 1) + (al << 1)];
}

uint32 LilliputSound::decode(const byte *src, byte *dst, uint32 len) {
	uint32 i = 0;
	for (; i < len; ++i) {
		*dst++ = readByte(src, i);
	}
	return i;
}

void LilliputSound::loadMusic(Common::String filename) {
	debugC(1, kDebugSound, "loadMusic(%s)", filename.c_str());

	Common::File f;

	if (!f.open(filename))
		error("Missing music file %s", filename.c_str());

	_fileNumb = f.readUint16LE();

	int *fileSizes = new int[_fileNumb + 1];
	for (int i = 0; i < _fileNumb; ++i)
		fileSizes[i] = f.readUint16LE();
	f.seek(0, SEEK_END);
	fileSizes[_fileNumb] = f.pos();

	_unpackedFiles = new byte *[_fileNumb];
	_unpackedSizes = new uint16[_fileNumb];
	int pos = (_fileNumb + 1) * 2; // file number + file sizes
	for (int i = 0; i < _fileNumb; ++i) {
		int packedSize = fileSizes[i + 1] - fileSizes[i];
		byte *srcBuf = new byte[packedSize];
		f.seek(pos, SEEK_SET);
		f.read(srcBuf, packedSize);
		if (srcBuf[0] == 'c' || srcBuf[0] == 'C') {
			int shift = (srcBuf[0] == 'c') ? 1 : 0;
			_unpackedSizes[i] = (1 + packedSize - 0x201) * 2 - shift;
			byte *dstBuf = new byte[_unpackedSizes[i]];
			decode(srcBuf + shift, dstBuf, _unpackedSizes[i]);
			_unpackedFiles[i] = dstBuf;
		} else {
			_unpackedSizes[i] = packedSize;
			byte *dstBuf = new byte[packedSize];
			for (int j = 0; j < packedSize; ++j)
				dstBuf[j] = srcBuf[j];
			_unpackedFiles[i] = dstBuf;
		}
		delete srcBuf;
		pos += packedSize;
	}

	delete fileSizes;
	f.close();

	// Debug code
	for (int i = 0; i < _fileNumb; ++i) {
		Common::DumpFile dmp;
		Common::String name = Common::String::format("dmp%d.mid", i);
		dmp.open(name);
		dmp.write(_unpackedFiles[i], _unpackedSizes[i]);
		dmp.close();
	}
	//
}

// Used during initialization
void LilliputSound::init() {
	debugC(1, kDebugSound, "LilliputSound::init()");

	loadMusic("ROBIN.MUS");
}

void LilliputSound::refresh() {
	debugC(1, kDebugSound, "LilliputSound::refresh()");
}

void LilliputSound::play(int var1, Common::Point var2, Common::Point var3, Common::Point var4) {
	debugC(1, kDebugSound, "LilliputSound::play(%d, %d - %d, %d - %d, %d - %d)", var1, var2.x, var2.y, var3.x, var3.y, var4.x, var4.y);
}

void LilliputSound::stop(Common::Point pos) {
	debugC(1, kDebugSound, "LilliputSound::stop(%d - %d)", pos.x, pos.y);
}

void LilliputSound::toggleOnOff() {
	debugC(1, kDebugSound, "LilliputSound::toggleOnOff()");
}

void LilliputSound::update() {
	debugC(1, kDebugSound, "LilliputSound::update()");
}

void LilliputSound::remove() {
	debugC(1, kDebugSound, "Lilliput::remove()");
}

} // End of namespace
