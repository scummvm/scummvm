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

#ifndef GROOVIE_DETECTION_H
#define GROOVIE_DETECTION_H

#include "engines/advancedDetector.h"

namespace Groovie {

enum EngineVersion {
	kGroovieT7G,
	kGroovieT11H,
	kGroovieCDY,
	kGroovieUHP,
	kGroovieTLC
};

struct GroovieGameDescription {
	ADGameDescription desc;

	EngineVersion version; // Version of the engine
};

#define GAMEOPTION_T7G_FAST_MOVIE_SPEED GUIO_GAMEOPTIONS1
#define GAMEOPTION_ORIGINAL_SAVELOAD GUIO_GAMEOPTIONS2
#define GAMEOPTION_EASIER_AI GUIO_GAMEOPTIONS3
#define GAMEOPTION_FINAL_HOUR GUIO_GAMEOPTIONS4
#define GAMEOPTION_SPEEDRUN GUIO_GAMEOPTIONS5
#define GAMEOPTION_EASIER_AI_DEFAULT GUIO_GAMEOPTIONS6
#define GAMEOPTION_SLIMHOTSPOTS GUIO_GAMEOPTIONS7

} // End of namespace Groovie

#endif
