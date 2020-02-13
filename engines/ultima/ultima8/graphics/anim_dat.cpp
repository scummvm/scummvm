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
	for (unsigned int i = 0; i < anims.size(); i++)
		delete anims[i];
	anims.clear();
}

ActorAnim *AnimDat::getAnim(uint32 shape) const {
	if (shape >= anims.size()) return 0;

	return anims[shape];
}

AnimAction *AnimDat::getAnim(uint32 shape, uint32 action) const {
	if (shape >= anims.size()) return 0;
	if (anims[shape] == 0) return 0;

	return anims[shape]->getAction(action);
}


void AnimDat::load(IDataSource *ds) {
	AnimFrame f;

	// CONSTANT !
	anims.resize(2048);

	unsigned int actioncount = 64;
	if (GAME_IS_CRUSADER)
		actioncount = 256;

	for (unsigned int shape = 0; shape < anims.size(); shape++) {
		ds->seek(4 * shape);
		uint32 offset = ds->read4();

		if (offset == 0) {
			anims[shape] = 0;
			continue;
		}

		ActorAnim *a = new ActorAnim();

		// CONSTANT !
		a->actions.resize(actioncount);

		for (unsigned int action = 0; action < actioncount; action++) {
			ds->seek(offset + action * 4);
			uint32 actionoffset = ds->read4();

			if (actionoffset == 0) {
				a->actions[action] = 0;
				continue;
			}

			a->actions[action] = new AnimAction();

			a->actions[action]->shapenum = shape;
			a->actions[action]->action = action;

			ds->seek(actionoffset);
			uint32 actionsize = ds->read1();
			a->actions[action]->size = actionsize;
			a->actions[action]->flags = ds->read1();
			a->actions[action]->framerepeat = ds->read1();
			a->actions[action]->flags |= ds->read1() << 8;

			unsigned int dircount = 8;
			if (GAME_IS_CRUSADER &&
			        (a->actions[action]->flags & AnimAction::AAF_CRUS_16DIRS)) {
				dircount = 16;
			}
			a->actions[action]->dircount = dircount;

			for (unsigned int dir = 0; dir < dircount; dir++) {
				a->actions[action]->frames[dir].clear();

				for (unsigned int j = 0; j < actionsize; j++) {
					if (GAME_IS_U8) {
						f.frame = ds->read1(); // & 0x7FF;
						uint8 x = ds->read1();
						f.frame += (x & 0x7) << 8;
						f.deltaz = ds->readXS(1);
						f.sfx = ds->read1();
						f.deltadir = ds->readXS(1);
						f.flags = ds->read1();
						f.flags += (x & 0xF8) << 8;
					} else if (GAME_IS_CRUSADER) {
						// byte 0: low byte of frame
						f.frame = ds->read1();
						// byte 1: low nibble part of frame
						uint8 x = ds->read1();
						f.frame += (x & 0xF) << 8;
						// byte 2, 3: unknown; byte 3 might contain flags
						ds->skip(2);
						// byte 4: deltadir (signed)
						f.deltadir = ds->readXS(1);
						// byte 5: flags?
						f.flags = ds->read1();
						// byte 6, 7: unknown
						ds->skip(2);

						f.deltaz = 0;
						f.sfx = 0;
					}
					a->actions[action]->frames[dir].push_back(f);
				}
			}
		}

		anims[shape] = a;
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
