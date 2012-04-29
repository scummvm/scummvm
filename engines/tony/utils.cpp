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

#include "tony/utils.h"
#include "mpal/lzo.h"

namespace Tony {

/****************************************************************************\
*       Resource Update
\****************************************************************************/

RMResUpdate::RMResUpdate() {
	_infos = NULL;
}

RMResUpdate::~RMResUpdate() {
	if (_infos) {
		delete[] _infos;
		_infos = NULL;
	}

	if (_hFile.isOpen())
		_hFile.close();
}

void RMResUpdate::Init(const Common::String &fileName) {
	// Open the resource update file
	if (!_hFile.open(fileName))
		// It doesn't exist, so exit immediately
		return;

	uint8 version;
	uint32 i;

	version = _hFile.readByte();
	_numUpd = _hFile.readUint32LE();

	_infos = new ResUpdInfo[_numUpd];

	// Load the index of the resources in the file
	for (i=0; i<_numUpd; ++i) {
		ResUpdInfo &info = _infos[i];

		info.dwRes = _hFile.readUint32LE();
		info.offset = _hFile.readUint32LE();
		info.size = _hFile.readUint32LE();
		info.cmpSize = _hFile.readUint32LE();
	}
}

const byte *RMResUpdate::QueryResource(uint32 dwRes) {
	// If there isn't an update file, return NULL
	if (!_hFile.isOpen())
		return NULL;

	uint32 i;
	for (i=0; i < _numUpd; ++i)
		if (_infos[i].dwRes == dwRes)
			// Found the index
			break;

	if (i==_numUpd)
		// Couldn't find a matching resource, so return NULL
		return NULL;

	const ResUpdInfo &info = _infos[i];
	byte *cmpBuf = new byte[info.cmpSize];
	uint32 dwRead;

	// Move to the correct offset and read in the compressed data
	_hFile.seek(info.offset);
	dwRead = _hFile.read(cmpBuf, info.cmpSize);

	if (info.cmpSize > dwRead) {
		// Error occurred reading data, so return NULL
		delete[] cmpBuf;
		return NULL;
	}

	// Allocate space for the output resource
	byte *lpDestBuf = new byte[info.size];
	uint32 dwSize;

	// Decompress the data
	MPAL::lzo1x_decompress(cmpBuf, info.cmpSize, lpDestBuf, &dwSize);

	// Delete buffer for compressed data
	delete [] cmpBuf;

	// Return the resource
	return lpDestBuf;
}

} // End of namespace Tony
