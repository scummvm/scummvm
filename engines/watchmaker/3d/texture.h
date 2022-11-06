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

#ifndef WATCHMAKER_TEXTURE_H
#define WATCHMAKER_TEXTURE_H

#include "common/array.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "watchmaker/3d/dds_header.h"
#include "graphics/surface.h"
#include "watchmaker/rect.h"
#include "watchmaker/surface.h"

namespace Watchmaker {

class WGame;
// Texture structs
struct gTexture {
	Common::String			name;
	Texture 			   *texture = nullptr;
	int                     RealDimX = 0;               // original dimensions
	int                     RealDimY = 0;               // original dimensions
	int                     DimX = 0;                   // current dimensions
	int                     DimY = 0;                   // current dimensions
	int                     ID = 0;                     // id
	int                     Flags = 0;                  // Flags

	bool isEmpty() {
		return texture == nullptr;
	}
	void clear() {
		// TODO: This will only work for the back-surface
		warning("Clearing %d", _blitsOnTop.size());
		_blitsOnTop.clear();
	}
	void render(WGame &game, Rect src, Rect dst);
	void blitInto(gTexture *texture, Rect src, Rect dst) {
		_blitsOnTop.push_back({texture, src, dst});
	}
private:
	struct Blit {
		gTexture *texture;
		Rect src;
		Rect dst;
	};
	Common::Array<Blit> _blitsOnTop;
};

} // End of namespace Watchmaker

#endif // WATCHMAKER_TEXTURE_H
