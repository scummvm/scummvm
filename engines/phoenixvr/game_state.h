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

#ifndef PHOENIXVR_GAME_STATE_H
#define PHOENIXVR_GAME_STATE_H

#include "common/array.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/stream.h"

namespace Graphics {
struct Surface;
struct PixelFormat;
} // namespace Graphics

namespace PhoenixVR {

struct GameState {
	Common::String script;
	Common::String game;
	Common::String info;
	Common::Array<byte> dibHeader;
	int16 thumbWidth;
	int16 thumbHeight;
	Common::Array<byte> thumbnail;
	Common::Array<byte> state;

	static GameState load(Common::SeekableReadStream &stream);
	void save(Common::WriteStream &stream) const;

	Graphics::Surface *getThumbnail(const Graphics::PixelFormat &fmt, int newWidth = 0);
};
} // namespace PhoenixVR

#endif
