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

ActorAnim *AnimDat::getAnim(uint32 shape) const {
	if (shape >= _anims.size())
		return nullptr;

	return _anims[shape];
}

AnimAction *AnimDat::getAnim(uint32 shape, uint32 action) const {
	if (shape >= _anims.size())
		return nullptr;
	if (_anims[shape] == 0)
		return nullptr;

	return _anims[shape]->getAction(action);
}


void AnimDat::load(IDataSource *ds) {
	AnimFrame f;

	// CONSTANT !
	_anims.resize(2048);

	unsigned int actioncount = 64;
	if (GAME_IS_CRUSADER)
		actioncount = 256;

	for (unsigned int shape = 0; shape < _anims.size(); shape++) {
		ds->seek(4 * shape);
		uint32 offset = ds->read4();

		if (offset == 0) {
			_anims[shape] = nullptr;
			continue;
		}

		ActorAnim *a = new ActorAnim();

		// CONSTANT !
		a->_actions.resize(actioncount);

		for (unsigned int action = 0; action < actioncount; action++) {
			ds->seek(offset + action * 4);
			uint32 actionoffset = ds->read4();

			if (actionoffset == 0) {
				a->_actions[action] = 0;
				continue;
			}

			a->_actions[action] = new AnimAction();

			a->_actions[action]->_shapeNum = shape;
			a->_actions[action]->_action = action;

			ds->seek(actionoffset);
			uint32 actionsize = ds->read1();
			a->_actions[action]->_size = actionsize;
			a->_actions[action]->_flags = ds->read1();
			a->_actions[action]->_frameRepeat = ds->read1();
			a->_actions[action]->_flags |= ds->read1() << 8;

			unsigned int dirCount = 8;
			if (GAME_IS_CRUSADER &&
			        (a->_actions[action]->_flags & AnimAction::AAF_CRUS_16DIRS)) {
				dirCount = 16;
			}
			a->_actions[action]->_dirCount = dirCount;

			for (unsigned int dir = 0; dir < dirCount; dir++) {
				a->_actions[action]->frames[dir].clear();

				for (unsigned int j = 0; j < actionsize; j++) {
					if (GAME_IS_U8) {
						f._frame = ds->read1(); // & 0x7FF;
						uint8 x = ds->read1();
						f._frame += (x & 0x7) << 8;
						f._deltaZ = ds->readXS(1);
						f._sfx = ds->read1();
						f._deltaDir = ds->readXS(1);
						f._flags = ds->read1();
						f._flags += (x & 0xF8) << 8;
					} else if (GAME_IS_CRUSADER) {
						// byte 0: low byte of frame
						f._frame = ds->read1();
						// byte 1: low nibble part of frame
						uint8 x = ds->read1();
						f._frame += (x & 0xF) << 8;
						// byte 2, 3: unknown; byte 3 might contain flags
						ds->skip(2);
						// byte 4: deltadir (signed)
						f._deltaDir = ds->readXS(1);
						// byte 5: flags?
						f._flags = ds->read1();
						// byte 6, 7: unknown
						ds->skip(2);

						f._deltaZ = 0;
						f._sfx = 0;
					}
					a->_actions[action]->frames[dir].push_back(f);
				}
			}
		}

		_anims[shape] = a;
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
