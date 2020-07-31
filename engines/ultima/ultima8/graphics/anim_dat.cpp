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

#include "ultima/ultima8/misc/pent_include.h"

#include "ultima/ultima8/graphics/anim_dat.h"

#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/world/actors/actor_anim.h"
#include "ultima/ultima8/world/actors/anim_action.h"
#include "ultima/ultima8/world/actors/animation.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/kernel/core_app.h"
#include "ultima/ultima8/games/game_info.h"

namespace Ultima {
namespace Ultima8 {

AnimDat::AnimDat() {
}

AnimDat::~AnimDat() {
	for (unsigned int i = 0; i < _anims.size(); i++)
		delete _anims[i];
}

const ActorAnim *AnimDat::getAnim(uint32 shape) const {
	if (shape >= _anims.size())
		return nullptr;

	return _anims[shape];
}

const AnimAction *AnimDat::getAnim(uint32 shape, uint32 action) const {
	if (shape >= _anims.size())
		return nullptr;
	if (_anims[shape] == 0)
		return nullptr;

	return _anims[shape]->getAction(action);
}

uint32 AnimDat::getActionNumberForSequence(Animation::Sequence action, const Actor *actor) {
	if (GAME_IS_U8) {
		return static_cast<uint32>(action);
	} else {
		bool smallwpn = (actor && actor->activeWeaponIsSmall());
		// For crusader the actions have different IDs.  Rather than
		// rewrite everything, we just translate them here for all the ones
		// we want to use programmatically.  There are more, but they are
		// called from usecode so don't need translation.
		//
		// TODO: Also handle kneeling weapon animations
		switch (action) {
		case Animation::stand:
			return 0;
		case Animation::step:
			return 1; // Same as walk in crusader.
		case Animation::walk:
			return 1;
		case Animation::retreat:
			return 2; // TODO: 28 is also a retreat move, which is right?
		case Animation::run:
			return 3;
		case Animation::combatStand:
			return (smallwpn ? 4 : 37);
		// Note: 5, 6, 9, 10 == nothing (for avatar)?
		case Animation::unreadyWeapon:
			return (smallwpn ? 11: 16);
		case Animation::readyWeapon:
			return (smallwpn ? 7 : 12);
		case Animation::attack:
			return (smallwpn ? 8 : 13);
		// Note: 14, 17, 21, 22, 29 == nothing for avatar
		case Animation::fallBackwards:
			return 18;
		case Animation::die:
			return 20; // maybe? falls over forwards
		case Animation::advance:
			return 36; // TODO: 44 is also advance
		case Animation::startKneeling:
			return 40;
		case Animation::stopKneeling:
			return 41;
		case Animation::kneel:
			return 46; // 47 is knee with a larger weapon
		// 48 is nothing for avatar
		case Animation::lookLeft:
			return 0;
		case Animation::lookRight:
			return 0;
		case Animation::teleportInReplacement:
			return Animation::teleportIn;
		case Animation::teleportOutReplacement:
			return Animation::teleportOut;
		default:
			return static_cast<uint32>(action);;
		}
	}
}

void AnimDat::load(Common::SeekableReadStream *rs) {
	AnimFrame f;

	// CONSTANT !
	_anims.resize(2048);

	unsigned int actioncount = 64;

	if (GAME_IS_CRUSADER) {
		actioncount = 256;
	}

	for (unsigned int shape = 0; shape < _anims.size(); shape++) {
		rs->seek(4 * shape);
		uint32 offset = rs->readUint32LE();

		if (offset == 0) {
			_anims[shape] = nullptr;
			continue;
		}

		ActorAnim *a = new ActorAnim();

		// CONSTANT !
		a->_actions.resize(actioncount);

		for (unsigned int action = 0; action < actioncount; action++) {
			rs->seek(offset + action * 4);
			uint32 actionoffset = rs->readUint32LE();

			if (actionoffset == 0) {
				a->_actions[action] = 0;
				continue;
			}

			a->_actions[action] = new AnimAction();

			a->_actions[action]->_shapeNum = shape;
			a->_actions[action]->_action = action;

			rs->seek(actionoffset);
			uint32 actionsize = rs->readByte();
			a->_actions[action]->_size = actionsize;
			a->_actions[action]->_flags = rs->readByte();
			a->_actions[action]->_frameRepeat = rs->readByte();
			a->_actions[action]->_flags |= rs->readByte() << 8;

			unsigned int dirCount = 8;
			if (GAME_IS_CRUSADER &&
			        (a->_actions[action]->_flags & AnimAction::AAF_CRUS_16DIRS)) {
				dirCount = 16;
			}
			a->_actions[action]->_dirCount = dirCount;

			for (unsigned int dir = 0; dir < dirCount; dir++) {
				a->_actions[action]->_frames[dir].clear();

				for (unsigned int j = 0; j < actionsize; j++) {
					if (GAME_IS_U8) {
						f._frame = rs->readByte(); // & 0x7FF;
						uint8 x = rs->readByte();
						f._frame += (x & 0x7) << 8;
						f._deltaZ = rs->readSByte();
						f._sfx = rs->readByte();
						f._deltaDir = rs->readSByte();
						f._flags = rs->readByte();
						f._flags += (x & 0xF8) << 8;
						f._unk1 = 0;
						f._unk2 = 0;
					} else if (GAME_IS_CRUSADER) {
						// byte 0: low byte of frame
						f._frame = rs->readByte();
						// byte 1: low nibble part of frame
						uint8 x = rs->readByte();
						f._frame += (x & 0xF) << 8;
						// byte 2: delta z
						f._deltaZ = rs->readByte();
						// byte 3: sfx
						f._sfx = rs->readByte();
						// byte 4: deltadir (signed) - convert to pixels
						f._deltaDir = rs->readSByte();
						// byte 5: flags? TODO: Ensure "flipped" flag is mapped correctly
						f._flags = rs->readByte();
						f._flags += (x & 0xF0) << 8;
						// byte 6, 7: unknown
						f._unk2 = rs->readSint16LE();
					}
					a->_actions[action]->_frames[dir].push_back(f);
				}
			}
		}

		_anims[shape] = a;
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
