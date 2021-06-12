/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef DIRECTOR_DETECTION_H
#define DIRECTOR_DETECTION_H

#include "engines/advancedDetector.h"

namespace Director {

enum DirectorGameGID {
	GID_GENERIC,
	GID_TEST,
	GID_TESTALL
};

struct DirectorGameDescription {
	ADGameDescription desc;

	DirectorGameGID gameGID;
	uint16 version;
};

extern const char *directoryGlobs[];

} // End of namespace Director

#endif // DIRECTOR_DETECTION_H
