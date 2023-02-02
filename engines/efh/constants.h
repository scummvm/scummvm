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

#ifndef EFH_CONSTANTS_H
#define EFH_CONSTANTS_H

#include "common/scummsys.h"

namespace Efh {
enum EfhMenuItems {
	kEfhMenuEquip = 0,
	kEfhMenuUse = 1,
	kEfhMenuGive = 2,
	kEfhMenuTrade = 3,
	kEfhMenuDrop = 4,
	kEfhMenuInfo = 5,
	kEfhMenuPassive = 6,
	kEfhMenuActive = 7,
	kEfhMenuLeave = 8,
	kEfhMenuInvalid = 9
};

enum EfhReactionType {
	kEfhReactionReels = 0,
	kEfhReactionCriesOut = 1,
	kEfhReactionFalters = 2,
	kEfhReactionWinces = 3,
	kEfhReactionScreams = 4,
	kEfhReactionChortles = 5,
	kEfhReactionLaughs = 6
};

enum EfhStatusType {
	kEfhStatusNormal = 0,
	kEfhStatusSleeping = 1,
	kEfhStatusFrozen = 2
};

struct Font {
	uint8 _lines[8];
};

struct Encounter {
	char _name[14];
	uint8 _animId;
	uint16 _pictureRef;
	uint16 _xpGiven;
	uint16 _dropItemId[5];
	uint8 _dropOccurrencePct;
	uint8 _nameArticle;
};

#define kDefaultNoteDuration 616

extern const uint8 kFontWidthArray[96];
extern const uint8 kFontExtraLinesArray[96];
extern const Font kFontData[96];
extern const Encounter kEncounters[];
extern const char kSkillArray[37][20];
extern const uint8 kByte2C7D0[60];
extern const char kPossessive[3][4];
extern const char kPersonal[3][4];
extern const char kAttackVerbs[51][20];
extern const int16 kSoundFrequency[72];

} // End of namespace Efh

#endif // EFH_CONSTANTS_H
