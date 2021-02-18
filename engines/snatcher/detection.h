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

#ifndef SNATCHER_DETECTION_H
#define SNATCHER_DETECTION_H

#include "engines/advancedDetector.h"

namespace Snatcher {

enum {
	GI_SNATCHER = 0
};

struct GameDescription {
	byte gameID;
	int soundOptions;
	Common::Language lang;
	Common::Platform platform;

	// language overwrites of fan translations (only needed for multilingual games)
	Common::Language fanLang;
	Common::Language replacedLang;
	
};

} // End of namespace Snatcher

namespace {

struct SnatcherGameDescription {
	AD_GAME_DESCRIPTION_HELPERS(ad_desc);
	ADGameDescription ad_desc;
	Snatcher::GameDescription ex_desc;
};

} // End of anonymous namespace

#endif // SNATCHER_DETECTION_H
