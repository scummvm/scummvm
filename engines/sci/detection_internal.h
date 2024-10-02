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

#ifndef SCI_DETECTION_INTERNAL_H
#define SCI_DETECTION_INTERNAL_H

#include "common/path.h"
#include "common/platform.h"

#include "sci/detection.h"

namespace Sci {

struct GameIdStrToEnum {
	const char *gameidStr;
	const char *sierraIdStr;
	SciGameId gameidEnum;
	bool isSci32;
	SciVersion version;
};

extern const GameIdStrToEnum gameIdStrToEnum[];

Common::String customizeGuiOptions(Common::Path gamePath, Common::String guiOptions, Common::Platform platform, Common::String idStr, SciVersion version);

} // End of namespace Sci

#endif
