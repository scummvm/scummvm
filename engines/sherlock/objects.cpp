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

#include "sherlock/objects.h"

namespace Sherlock {

/**
 * Reset the data for the sprite
 */
void Sprite::clear() {
	_name = "";
	_description = "";
	_examine.clear();
	_pickUp = "";
	_sequences = nullptr;
	_images = nullptr;
	_imageFrame = nullptr;
	_walkCount = 0;
	_allow = 0;
	_frameNumber = _sequenceNumber = 0;
	_position.x = _position.y = 0;
	_movement.x = _movement.y = 0;
	_oldPosition.x = _oldPosition.y = 0;
	_oldSize.x = _oldSize.y = 0;
	_goto.x = _goto.y = 0;
	_type = INVALID;
	_pickUp.clear();
	_noShapeSize.x = _noShapeSize.y = 0;
	_status = 0;
	_misc = 0;
	_numFrames = 0;
}

/*----------------------------------------------------------------*/

void ActionType::synchronize(Common::SeekableReadStream &s) {
	char buffer[12];
	
	_cAnimNum = s.readByte();
	_cAnimSpeed = s.readByte();

	for (int idx = 0; idx < 4; ++idx) {
		s.read(buffer, 12);
		_names[idx] = Common::String(buffer);
	}
}

/*----------------------------------------------------------------*/

void UseType::synchronize(Common::SeekableReadStream &s) {
	char buffer[12];

	_cAnimNum = s.readByte();
	_cAnimSpeed = s.readByte();

	for (int idx = 0; idx < 4; ++idx) {
		s.read(buffer, 12);
		_names[idx] = Common::String(buffer);
	}

	_useFlag = s.readUint16LE();
	_dFlag[0] = s.readUint16LE();
	_lFlag[0] = s.readUint16LE();
	_lFlag[1] = s.readUint16LE();

	s.read(buffer, 12);
	_target = Common::String(buffer);
}

/*----------------------------------------------------------------*/

void Object::synchronize(Common::SeekableReadStream &s) {
	char buffer[50];

	s.read(buffer, 12);
	_name = Common::String(buffer);
	s.read(buffer, 41);
	_description = Common::String(buffer);

	_examine.clear();
	_sequences = nullptr;
	_images = nullptr;
	_imageFrame = nullptr;
	s.seek(16, SEEK_CUR);

	_walkCount = s.readByte();
	_allow = s.readByte();
	_frameNumber = s.readSint16LE();
	_sequenceNumber = s.readSint16LE();
	_position.x = s.readSint16LE();
	_position.y = s.readSint16LE();
	_movement.x = s.readSint16LE();
	_movement.y = s.readSint16LE();
	_type = (SpriteType)s.readUint16LE();
	_oldPosition.x = s.readSint16LE();
	_oldPosition.y = s.readSint16LE();
	_oldSize.x = s.readUint16LE();
	_oldSize.y = s.readUint16LE();
	_goto.x = s.readSint16LE();
	_goto.y = s.readSint16LE();
	
	_pickup = s.readByte();
	_defaultCommand = s.readByte();
	_lookFlag = s.readUint16LE();
	_pickupFlag = s.readUint16LE();
	_requiredFlag = s.readUint16LE();
	_noShapeSize.x = s.readUint16LE();
	_noShapeSize.y = s.readUint16LE();
	_status = s.readUint16LE();
	_misc = s.readByte();
	_maxFrames = s.readUint16LE();
	_flags = s.readByte();	
	_aOpen.synchronize(s);
	_aType = s.readByte();
	_lookFrames = s.readByte();
	_seqcounter = s.readByte();
	_lookPosition.x = s.readUint16LE();
	_lookPosition.y = s.readByte();
	_lookFacing = s.readByte();
	_lookcAnim = s.readByte();
	_aClose.synchronize(s);
	_seqStack = s.readByte();
	_seqTo = s.readByte();
	_descOfs = s.readUint16LE();
	_seqcounter2 = s.readByte();
	_seqSize = s.readUint16LE();
	s.skip(1);
	_aMove.synchronize(s);
	s.skip(8);
	
	for (int idx = 0; idx < 4; ++idx)
		_use[idx].synchronize(s);
}

} // End of namespace Sherlock
