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

#ifndef GRIM_SMUSH_PLAYER_H
#define GRIM_SMUSH_PLAYER_H

#include "engines/grim/movie/movie.h"

#include "video/video_decoder.h"

namespace Grim {

class SmushDecoder;

class SmushPlayer : public MoviePlayer {
public:
	SmushPlayer(bool demo);
	virtual ~SmushPlayer();
	void restore(SaveGame *state) override;

private:
	bool loadFile(const Common::String &filename) override;
	void handleFrame() override;
	void postHandleFrame() override;
	void init() override;
	bool _demo;
	bool _currentVideoIsTheora = false;
	SmushDecoder *_smushDecoder;
	Video::VideoDecoder *_theoraDecoder; // HACK for now, move to other class later?
};

} // end of namespace Grim

#endif
