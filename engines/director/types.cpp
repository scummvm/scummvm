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

#include "common/array.h"
#include "common/str.h"
#include "director/types.h"

namespace Director {

const char *scriptTypes[] = {
	"ScoreScript",
	"CastScript",
	"MovieScript",
	"EventScript",
	"TestScript"
};

const char *scriptType2str(ScriptType scr) {
	if (scr < 0)
		return "NoneScript";

	if (scr > kMaxScriptType)
		return "<unknown>";

	return scriptTypes[scr];
}

Common::String castTypeToString(const CastType &type) {
	Common::String res;
	switch (type) {
	case kCastBitmap:
		res = "bitmap";
		break;
	case kCastPalette:
		res = "palette";
		break;
	case kCastButton:
		res = "button";
		break;
	case kCastPicture:
		res = "picture";
		break;
	case kCastDigitalVideo:
		res = "digitalVideo";
		break;
	case kCastLingoScript:
		res = "script";
		break;
	case kCastShape:
		res = "shape";
		break;
	case kCastFilmLoop:
		res = "filmLoop";
		break;
	case kCastSound:
		res = "sound";
		break;
	case kCastMovie:
		res = "movie";
		break;
	case kCastText:
		res = "text";
		break;
	default:
		res = "empty";
		break;
	}
	return res;
}


} // End of namespace Director
