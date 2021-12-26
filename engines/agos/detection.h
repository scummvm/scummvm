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

#ifndef AGOS_DETECTION_H
#define AGOS_DETECTION_H

#include "engines/advancedDetector.h"

namespace AGOS {

enum SIMONGameType {
	GType_PN = 0,
	GType_ELVIRA1 = 1,
	GType_ELVIRA2 = 2,
	GType_WW = 3,
	GType_SIMON1 = 4,
	GType_SIMON2 = 5,
	GType_FF = 6,
	GType_PP = 7
};

struct AGOSGameDescription {
	ADGameDescription desc;

	int gameType;
	int gameId;
	uint32 features;
};

} // End of namespace AGOS

#endif // AGOS_DETECTION_H
