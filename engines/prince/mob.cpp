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

#include "prince/mob.h"

#include "common/stream.h"

namespace Prince {

bool Mob::loadFromStream(Common::SeekableReadStream &stream) {
	int32 pos = stream.pos();

	uint16 visible = stream.readUint16LE();

	if (visible == 0xFFFF)
		return false;

	_visible = visible;
	_type = stream.readUint16LE();
	_rect.left = stream.readUint16LE();
	_rect.top = stream.readUint16LE();
	_rect.right = stream.readUint16LE();
	_rect.bottom = stream.readUint16LE();

	stream.skip(6 * sizeof(uint16));
	uint32 nameOffset = stream.readUint32LE();
	uint32 examTextOffset = stream.readUint32LE();

	byte c;
	stream.seek(nameOffset);
	_name.clear();
	while ((c = stream.readByte()))
		_name += c;

	stream.seek(examTextOffset);
	_examText.clear();
	while ((c = stream.readByte()))
		_examText += c;
	stream.seek(pos + 32);

	return true;
}

}

/* vim: set tabstop=4 noexpandtab: */
