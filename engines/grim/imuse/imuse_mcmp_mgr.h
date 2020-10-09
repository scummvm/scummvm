/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef GRIM_MCMP_MGR_H
#define GRIM_MCMP_MGR_H

namespace Grim {

class McmpMgr {
private:

	struct CompTable {
		byte codec;
		int32 decompSize;
		int32 compSize;
		int32 offset;
	};

	CompTable *_compTable;
	int16 _numCompItems;
	int _curSample;
	Common::SeekableReadStream *_file;
	byte _compOutput[0x2000];
	byte *_compInput;
	int _outputSize;
	int _lastBlock;

public:

	McmpMgr();
	~McmpMgr();

	bool openSound(const char *filename, Common::SeekableReadStream *data, int &offsetData);
	int32 decompressSample(int32 offset, int32 size, byte **comp_final);
};

} // end of namespace Grim

#endif
