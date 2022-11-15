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

#ifndef CINE_DETECTION_H
#define CINE_DETECTION_H

#include "engines/advancedDetector.h"

namespace Cine {

enum CineGameType {
	GType_FW = 1,
	GType_OS
};

enum CineGameFeatures {
	GF_CD =   1 << 0,
	GF_DEMO = 1 << 1,
	GF_ALT_FONT = 1 << 2,
	GF_CRYPTED_BOOT_PRC = 1 << 3
};

struct CINEGameDescription {
	ADGameDescription desc;

	int gameType;
	uint32 features;
};

#define GAMEOPTION_ORIGINAL_SAVELOAD   GUIO_GAMEOPTIONS1
#define GAMEOPTION_TRANSPARENT_DIALOG_BOXES   GUIO_GAMEOPTIONS2

} // End of namespace Cine

#endif // CINE_DETECTION_H
