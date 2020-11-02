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

#include "ultima/ultima8/graphics/wpn_ovlay_dat.h"

#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/world/actors/weapon_overlay.h"
#include "ultima/ultima8/filesys/raw_archive.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/main_shape_archive.h"
#include "ultima/ultima8/world/actors/anim_action.h"

namespace Ultima {
namespace Ultima8 {

WpnOvlayDat::WpnOvlayDat() {
}

WpnOvlayDat::~WpnOvlayDat() {
	for (unsigned int i = 0; i < _overlay.size(); i++)
		delete _overlay[i];
}

const WeaponOverlayFrame *WpnOvlayDat::getOverlayFrame(uint32 action, int type,
        Direction direction,
        int frame) const {
	if (action >= _overlay.size())
		return nullptr;
	if (!_overlay[action])
		return nullptr;
	return _overlay[action]->getFrame(type, direction, frame);
}


void WpnOvlayDat::load(RawArchive *overlaydat) {
	MainShapeArchive *msf = GameData::get_instance()->getMainShapes();
	assert(msf);

	_overlay.resize(overlaydat->getCount());

	for (unsigned int action = 0; action < _overlay.size(); action++) {
		Common::SeekableReadStream *rs = overlaydat->get_datasource(action);
		_overlay[action] = nullptr;

		if (rs && rs->size()) {
			// get Avatar's animation
			const AnimAction *anim = msf->getAnim(1, action);
			if (!anim) {
				perr << "Skipping wpnovlay action " << action << " because avatar animation doesn't exist." << Std::endl;
				continue;
			}

			AnimWeaponOverlay *awo = new AnimWeaponOverlay;
			_overlay[action] = awo;

			unsigned int animlength = anim->getSize();
			unsigned int dircount = anim->getDirCount();

			unsigned int typecount = rs->size() / (4 * dircount * animlength);
			awo->_overlay.resize(typecount);

			for (unsigned int type = 0; type < typecount; type++) {
				awo->_overlay[type]._dirCount = dircount;
				awo->_overlay[type]._frames =
				    new Std::vector<WeaponOverlayFrame>[dircount];
				for (unsigned int dir = 0; dir < dircount; dir++) {
					awo->_overlay[type]._frames[dir].resize(animlength);
					for (unsigned int frame = 0; frame < animlength; frame++) {
						WeaponOverlayFrame f;
						f._xOff = rs->readSByte();
						f._yOff = rs->readSByte();
						f._frame = rs->readUint16LE();

						awo->_overlay[type]._frames[dir][frame] = f;
					}
				}
			}
		}

		delete rs;
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
