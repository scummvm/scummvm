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

#ifndef GRIM_MPEG_PLAYER_H
#define GRIM_MPEG_PLAYER_H

#include "engines/grim/movie/movie.h"

#ifdef USE_MPEG2

namespace Grim {

class MpegPlayer : public MoviePlayer {
public:
	MpegPlayer();
private:
	bool loadFile(const Common::String &filename) override;
	//bool _demo;
};

} // end of namespace Grim

#endif // USE_MPEG2

#endif
