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

#ifndef ALG_DETECTION_H
#define ALG_DETECTION_H

#include "engines/advancedDetector.h"

namespace Alg {

enum AlgGameType {
	GType_CRIME_PATROL,
	GType_DRUG_WARS,
	GType_WSJR,
	GType_LAST_BOUNTY_HUNTER,
	GType_MADDOG,
	GType_MADDOG2,
	GType_SPACE_PIRATES,
};

struct AlgGameDescription {
	AD_GAME_DESCRIPTION_HELPERS(desc);

	ADGameDescription desc;
	uint8 gameType;

};

#define GAMEOPTION_SINGLE_SPEED_VERSION		GUIO_GAMEOPTIONS1

} // End of namespace Alg

#endif // ALG_DETECTION_H
