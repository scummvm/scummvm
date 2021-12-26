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

#ifndef SAGA2_DETECTION_H
#define SAGA2_DETECTION_H

namespace Saga2 {

enum GameIds {
	GID_DINO,
	GID_FTA2
};

enum GameFileTypes {
	GAME_RESOURCEFILE     = 1 << 0,    // Game resources
	GAME_SCRIPTFILE       = 1 << 1,    // Game scripts
	GAME_SOUNDFILE        = 1 << 2,    // SFX, voices and MIDI music
	GAME_VOICEFILE        = 1 << 3,    // Voices
	GAME_IMAGEFILE        = 1 << 4,    // Game images
	GAME_OBJRESOURCEFILE  = 1 << 5,    // Game object data
	GAME_EXECUTABLE		  = 1 << 6
};

struct SAGA2GameDescription {
	ADGameDescription desc;

	int gameId;
};

} // End of namespace Saga2

#endif // SAGA2_DETECTION_H
