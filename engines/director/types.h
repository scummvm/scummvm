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

#ifndef DIRECTOR_TYPES_H
#define DIRECTOR_TYPES_H

namespace Director {

enum CastType {
	kCastTypeNull = 0,
	kCastBitmap = 1,
	kCastFilmLoop = 2,
	kCastText = 3,
	kCastPalette = 4,
	kCastPicture = 5,
	kCastSound = 6,
	kCastButton = 7,
	kCastShape = 8,
	kCastMovie = 9,
	kCastDigitalVideo = 10,
	kCastLingoScript = 11,
	kCastRTE = 12
};

enum ScriptType {
	kMovieScript = 0,
	kSpriteScript = 1,
	kFrameScript = 2,
	kCastScript = 3,
	kGlobalScript = 4,
	kNoneScript = -1,
	kMaxScriptType = 4	// Sync with score.cpp:45, array scriptTypes[]
};

const char *scriptType2str(ScriptType scr);

} // End of namespace Director

#endif
