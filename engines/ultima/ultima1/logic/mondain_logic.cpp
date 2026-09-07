
/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ultima/ultima1/logic/mondain_logic.h"
#include "ultima/ultima1/data/map.h"
#include "ultima/ultima1/metaengine.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

MondainLogic::MondainLogic() {
	_G(map)._mapType = Data::MAPTYPE_MONDAIN;
}

void MondainLogic::entering() {
	_G(savegame)._locationPosition = Common::Point(3, 4);
	_G(savegame)._mondainPos = Common::Point(15, 4);
}

bool MondainLogic::move(Data::Direction dir) {
	// TODO
	return true;
}

void MondainLogic::updateCreatures() {
	// TODO
}

bool MondainLogic::get() {
	return true;
}

bool MondainLogic::inform() {
	if (_G(savegame)._gemDestroyedFlag) {
		writeString("Inform and Search\n");
		writeString("Mondain's magical aura doth\n");
		writeString("seem substantially diminished\n");
		writeString("in the absence of the gem.\n");
	} else {
		writeString("Inform and Search...it looks\n");
		writeString("as if he is creating the evil\n");
		writeString("gem!\n");
	}

	return true;
}

bool MondainLogic::quit() {
	writeString("Quit...is not allowed!\n");
	playFX(1);
	return true;
}

bool MondainLogic::steal() {
	writeString("Steal...he's watching,\n");
	writeString("That would be foolish!\n");
	playFX(1);
	return true;
}

bool MondainLogic::transact() {
	writeString("Transact...Mondain will\n");
	writeString("not negotiate!\n");
	playFX(1);
	return true;
}

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima
