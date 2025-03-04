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

#ifndef HODJNPODJ_VIEWS_GFX_SURFACE_H
#define HODJNPODJ_VIEWS_GFX_SURFACE_H

#include "graphics/managed_surface.h"
#include "graphics/font.h"
#include "graphics/palette.h"

namespace Bagel {
namespace HodjNPodj {

class MinigameView;

class GfxSurface : public Graphics::ManagedSurface {
private:
	Graphics::ManagedSurface _cellsSource; // Used with loadCels
	size_t _cellWidth = 0;
	int _cellIndex = 0;

public:
	GfxSurface() : Graphics::ManagedSurface() {}
	GfxSurface(Graphics::ManagedSurface &surf, const Common::Rect &bounds) :
		Graphics::ManagedSurface(surf, bounds) {
	}

	void floodFill(int x, int y, byte color);
	void floodFill(const Common::Point &pt, byte color) {
		floodFill(pt.x, pt.y, color);
	}

	void loadBitmap(const char *filename);
	void loadCels(const char *filename, size_t numCells);
	void convertTo(const byte *palette, int count = PALETTE_COUNT);

	void setCel(size_t cellNum);
	int getCelIndex() const {
		return _cellIndex;
	}

	void writeString(const Common::String &text,
		const Common::Point &pos, int color = 0);
	void writeString(const Common::String &text,
		const Common::Rect &bounds, int color = 0,
		Graphics::TextAlign justify = Graphics::kTextAlignLeft);
};

class Sprite : public GfxSurface, public Common::Point {
private:
	MinigameView *_minigame = nullptr;

public:
	Sprite(MinigameView *minigame) : GfxSurface(),
			_minigame(minigame) {
		assert(_minigame);
	}

	bool isLinked() const;
	void linkSprite();
	void unlinkSprite();

	Common::Point getPosition() const {
		return *this;
	}
	void setPosition(int xx, int yy) {
		this->x = xx;
		this->y = yy;
	}
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
