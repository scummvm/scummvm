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
#include "ultima/ultima8/world/damage_info.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/kernel/kernel.h"

namespace Ultima {
namespace Ultima8 {

DamageInfo::DamageInfo(uint8 data[6]) {
	_flags = data[0];
	_sound = data[1];
	_data[0] = data[2];
	_data[1] = data[3];
	_data[2] = data[4];
	_damagePoints = data[5];
}

bool DamageInfo::applyToItem(Item *item, uint16 points) const {
	if (!item)
		return false;

	// The game does this.. it seems to be used to mean
	// "destroyed" (as distinct from broken?)
	if (item->hasFlags(Item::FLG_GUMP_OPEN))
		return false;

	uint8 itemPts = item->getDamagePoints();
	if (points < itemPts) {
		item->setDamagePoints(itemPts - points);
		return false;
	}
	item->setDamagePoints(0);
	item->setFlag(Item::FLG_GUMP_OPEN | Item::FLG_BROKEN);

	// Get some data out of the item before we potentially delete
	// it by explosion
	uint16 q = item->getQuality();
	int32 x, y , z;
	item->getLocation(x, y, z);
	int32 mapnum = item->getMapNum();

	if (explode()) {
		item->explode(explosionType(), explodeDestroysItem(), explodeWithDamage());
	}
	if (_sound) {
		AudioProcess *audio = AudioProcess::get_instance();
		if (audio) {
			audio->playSFX(_sound, 0x10, item->getObjId(), 1, true);
		}
	}
	if (replaceItem()) {
		uint16 replacementShape = getReplacementShape();
		uint8 replacementFrame = getReplacementFrame();
		Item *newitem = ItemFactory::createItem(replacementShape, replacementFrame, q, 0, 0, mapnum, 0, true);
		newitem->move(x, y, z);
	} else if (!explodeDestroysItem()) {
		assert(!explodeDestroysItem());
		if (frameDataIsAbsolute()) {
			int frameval = 1;
			if (_data[1])
				frameval++;
			if (_data[2])
				frameval++;
			item->setFrame(_data[getRandom() % frameval]);
		} else {
			int frameoff = 0;
			for (int i = 0; i < 3; i++)
				if (_data[i])
					frameoff++;
			if (!frameoff) {
				item->destroy();
			} else {
				uint32 frame = item->getFrame();
				item->setFrame(frame + _data[getRandom() % frameoff]);
			}
		}
	}
	return true;
}


} // End of namespace Ultima8
} // End of namespace Ultima
