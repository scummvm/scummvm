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
#ifndef SLUDGE_FILESET_H
#define SLUDGE_FILESET_H

#include "common/file.h"

namespace Sludge {

class ResourceManager {

public:
	ResourceManager():
		_sliceBusy(true),
		_bigDataFile(0),
		_startOfDataIndex(0),
		_startOfTextIndex(0),
		_startOfSubIndex(0),
		_startOfObjectIndex(0),
		_startIndex(0) {}

	void setData(Common::File *readStream);
	void setFileIndices(uint, uint);
	Common::SeekableReadStream *getData() { return _bigDataFile; }

	uint openFileFromNum(int num);
	bool openSubSlice(int num);
	bool openObjectSlice(int num);
	Common::String getNumberedString(int value);

	bool startAccess();
	void finishAccess();

private:
	bool _sliceBusy;
	Common::File *_bigDataFile;
	uint32 _startOfDataIndex, _startOfTextIndex, _startOfSubIndex, _startOfObjectIndex;
	int32 _startIndex;

private:
	Common::String convertString(const Common::String &s);
};

} // End of namespace Sludge

#endif
