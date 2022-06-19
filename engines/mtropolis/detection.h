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

#ifndef MTROPOLIS_DETECTION_H
#define MTROPOLIS_DETECTION_H

#include "engines/advancedDetector.h"

namespace MTropolis {

enum MTropolisGameID {
	GID_OBSIDIAN			= 0,
	GID_LEARNING_MTROPOLIS	= 1,
};

struct MTropolisGameDescription {
	ADGameDescription desc;

	int gameID;
	int gameType;
	uint16 version;
};

} // End of namespace MTropolis

#endif // MTROPOLIS_DETECTION_H
