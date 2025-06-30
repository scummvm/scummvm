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

#ifndef BAGEL_MFC_GFX_CLIPPED_SURFACE_H
#define BAGEL_MFC_GFX_CLIPPED_SURFACE_H

#include "graphics/managed_surface.h"

namespace Bagel {
namespace MFC {
namespace Gfx {

/**
 * Simple derived surface that disables standard
 * screen dirty rect handling. This is to support
 * painting, where only a designated update rect
 * will be actually be copied to the screen.
 */
class ClippedSurface : public Graphics::ManagedSurface {
private:
	Common::Rect _clipRect;

public:
	~ClippedSurface() override {}

	void create(int16 width, int16 height) override {
		Graphics::ManagedSurface::create(width, height);
		resetClip();
	}
	void create(int16 width, int16 height, const Graphics::PixelFormat &pixelFormat) override {
		Graphics::ManagedSurface::create(width, height, pixelFormat);
		resetClip();
	}
	void create(ManagedSurface &surf, const Common::Rect &bounds) override {
		Graphics::ManagedSurface::create(surf, bounds);
		resetClip();
	}

	void setClipRect(const Common::Rect &r) {
		_clipRect = r;
	}

	void resetClip() {
		_clipRect = Common::Rect(0, 0, this->w, this->h);
	}

	void addDirtyRect(const Common::Rect &r) override {
		Common::Rect dirtyRect = r;
		dirtyRect.clip(_clipRect);
		Graphics::ManagedSurface::addDirtyRect(dirtyRect);
	}
};

} // namespace Gfx
} // namespace MFC
} // namespace Bagel

#endif
