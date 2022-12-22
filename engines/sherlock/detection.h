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

#ifndef SHERLOCK_DETECTION_H
#define SHERLOCK_DETECTION_H

#include "engines/advancedDetector.h"

namespace Sherlock {

enum GameType {
	GType_SerratedScalpel = 0,
	GType_RoseTattoo = 1
};

struct SherlockGameDescription {
	ADGameDescription desc;

	GameType gameID;
};

#define GAMEOPTION_ORIGINAL_SAVES	GUIO_GAMEOPTIONS1
#define GAMEOPTION_FADE_STYLE		GUIO_GAMEOPTIONS2
#define GAMEOPTION_HELP_STYLE		GUIO_GAMEOPTIONS3
#define GAMEOPTION_PORTRAITS_ON		GUIO_GAMEOPTIONS4
#define GAMEOPTION_WINDOW_STYLE		GUIO_GAMEOPTIONS5
#define GAMEOPTION_TRANSPARENT_WINDOWS	GUIO_GAMEOPTIONS6

#ifdef USE_TTS
#define GAMEOPTION_TTS_NARRATOR 	GUIO_GAMEOPTIONS7
#endif

} // End of namespace Sherlock

#endif // SHERLOCK_DETECTION_H
