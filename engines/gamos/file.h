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

#ifndef GAMOS_FILE_H
#define GAMOS_FILE_H

#include "common/file.h"

namespace Gamos {

typedef Common::Array<byte> RawData;

struct GameFileDir {
	uint32 offset;
	byte id;
};

class GameFile : public Common::File {
public:
	GameFile();
	~GameFile() override;
	bool open(const Common::Path &name) override;

	uint16 getDirCount() const {
		return _dirCount;
	}

	int16 findDirByID(uint id) const {
		for (uint i = 0; i < _directories.size(); ++i) {
			if (_directories[i].id == id)
				return i;
		}

		return -1;
	}

	bool seekDir(uint id);

	int32 readPackedInt();

	RawData *readCompressedData();
	bool readCompressedData(RawData *out);

	static void decompress(RawData const *in, RawData *out);

	void setVersion(int v) {
		_version = v;
	}

public:

	uint32 _lastReadSize = 0;
	uint32 _lastReadDecompressedSize = 0;
	uint32 _lastReadDataOffset = 0;


private:
	int32 _dirOffset;

	byte _dirCount;
	uint32 _dataOffset;

	Common::Array<GameFileDir> _directories;

	int _version = 0x18;

	//bool _error;
};



} // namespace Gamos

#endif // GAMOS_FILE_H
