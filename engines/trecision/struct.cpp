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

#include "trecision/struct.h"

namespace Trecision {

void SRoom::loadRoom(Common::File *file) {
	file->read(&_baseName, 4);
	_flag = file->readByte();
	file->readByte(); // Padding
	_bkgAnim = file->readUint16LE();
	for (int j = 0; j < MAXOBJINROOM; ++j)
		_object[j] = file->readUint16LE();
	for (int j = 0; j < MAXSOUNDSINROOM; ++j)
		_sounds[j] = file->readUint16LE();
	for (int j = 0; j < MAXACTIONINROOM; ++j)
		_actions[j] = file->readUint16LE();
}

void SRoom::syncGameStream(Common::Serializer &ser) {
	ser.syncBytes((byte *)_baseName, 4);
	for (int i = 0; i < MAXACTIONINROOM; i++)
		ser.syncAsUint16LE(_actions[i]);
	ser.syncAsByte(_flag);
	ser.syncAsUint16LE(_bkgAnim);
}

} // namespace Trecision
