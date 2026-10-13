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

#ifndef HOLLYWOOD_RUNAWAY_STRINGS_H
#define HOLLYWOOD_RUNAWAY_STRINGS_H

#include "common/language.h"
#include "common/ustr.h"

namespace Hollywood {

// Interface labels are embedded in the original executables. Dialogue and
// subtitles remain in RESOURCE.003. Use the detected game's language here.
class RunawayStrings {
public:
	enum TextId {
		kLook, kTake, kUse, kPutAway, kWith, kInventory, kResume,
		kOpen, kClose, kMove, kLookThrough, kUsePhone, kTakeFromShelf,
		kUnplugCooler, kShirt, kWetCloth, kDirtyCloth, kTextCount
	};
	explicit RunawayStrings(Common::Language language) :
		_languageIndex(language == Common::FR_FRA ? 1 : language == Common::ES_ESP ? 2 : 0) {}
	Common::U32String text(TextId id) const;
	Common::U32String itemName(byte item) const;
	Common::U32String shackHotspotName(byte hotspot) const;

private:
	uint _languageIndex;
};

} // End of namespace Hollywood

#endif
