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

#ifndef AGI_DETECTION_H
#define AGI_DETECTION_H

#include "engines/advancedDetector.h"

namespace Agi {

struct AGIGameDescription {
	AD_GAME_DESCRIPTION_HELPERS(desc);

	ADGameDescription desc;

	int gameID;
	int gameType;
	uint32 features;
	uint16 version;
};

#define GAMEOPTION_ORIGINAL_SAVELOAD           GUIO_GAMEOPTIONS1
#define GAMEOPTION_AMIGA_ALTERNATIVE_PALETTE   GUIO_GAMEOPTIONS2
#define GAMEOPTION_ENABLE_MOUSE                GUIO_GAMEOPTIONS3
#define GAMEOPTION_USE_HERCULES_FONT           GUIO_GAMEOPTIONS4
#define GAMEOPTION_COMMAND_PROMPT_WINDOW       GUIO_GAMEOPTIONS5
#define GAMEOPTION_APPLE2GS_ADD_SPEED_MENU     GUIO_GAMEOPTIONS6
#define GAMEOPTION_COPY_PROTECTION             GUIO_GAMEOPTIONS7
#define GAMEOPTION_ENABLE_PREDICTIVE_FOR_MOUSE GUIO_GAMEOPTIONS8

} // End of namespace Agi

#endif // AGI_DETECTION_H
