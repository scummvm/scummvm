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
	ResourceManager();
	~ResourceManager();

	void init();
	void kill();

	void setData(Common::File *readStream);
	void setFileIndices(uint, uint);
	Common::SeekableReadStream *getData() { return _bigDataFile; }

	uint openFileFromNum(int num);
	bool openSubSlice(int num);
	bool openObjectSlice(int num);
	Common::String getNumberedString(int value);

	// Access control flag
	bool startAccess();
	void finishAccess();

	// Resource names
	void readResourceNames(Common::SeekableReadStream *readStream);
	const Common::String resourceNameFromNum(int i);
	bool hasResourceNames() { return !_allResourceNames.empty(); }

private:
	bool _sliceBusy;
	Common::File *_bigDataFile;
	uint32 _startOfDataIndex, _startOfTextIndex, _startOfSubIndex, _startOfObjectIndex;
	int32 _startIndex;

	Common::Array<Common::String> _allResourceNames;

private:
	static uint32 _cp1250ToUTF32[128];
	Common::String convertString(const Common::String &s);
};

} // End of namespace Sludge

#endif
