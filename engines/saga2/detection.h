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

#ifndef SAGA2_DETECTION_H
#define SAGA2_DETECTION_H

namespace Saga2 {

enum GameIds {
	GID_DINO,
	GID_FTA2
};

enum GameFileTypes {
	// Common
	GAME_RESOURCEFILE     = 1 << 0,    // Game resources
	GAME_SCRIPTFILE       = 1 << 1,    // Game scripts
	GAME_SOUNDFILE        = 1 << 2,    // SFX (also contains voices and MIDI music in SAGA 2 games)
	GAME_VOICEFILE        = 1 << 3,    // Voices (also contains SFX in the ITE floppy version)
	// ITE specific
	GAME_DIGITALMUSICFILE = 1 << 4,    // ITE digital music, added by Wyrmkeep
	GAME_MACBINARY        = 1 << 5,    // ITE Mac CD Guild
	GAME_DEMOFILE         = 1 << 6,    // Early ITE demo
	GAME_SWAPENDIAN       = 1 << 7,    // Used to identify the BE voice file in the ITE combined version
	// IHNM specific
	GAME_MUSICFILE_FM     = 1 << 8,    // IHNM
	GAME_MUSICFILE_GM     = 1 << 9,    // IHNM, ITE Mac CD Guild
	GAME_PATCHFILE        = 1 << 10,   // IHNM patch file (patch.re_/patch.res)
	// SAGA 2 (Dinotopia, FTA2)
	GAME_IMAGEFILE        = 1 << 11,   // Game images
	GAME_OBJRESOURCEFILE  = 1 << 12,   // Game object data
	GAME_EXECUTABLE		  = 1 << 13
};

struct SAGA2GameDescription {
	ADGameDescription desc;

	int gameId;
};

} // End of namespace Saga2

#endif // SAGA2_DETECTION_H
