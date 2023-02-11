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

#ifndef MM_SHARED_XEEN_XSURFACE_H
#define MM_SHARED_XEEN_XSURFACE_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/rect.h"
#include "graphics/managed_surface.h"

namespace MM {
namespace Shared {
namespace Xeen {

class BaseSurface : public Graphics::ManagedSurface {
public:
	void addDirtyRect(const Common::Rect &r) override {
		Graphics::ManagedSurface::addDirtyRect(r);
	}
public:
	BaseSurface() : Graphics::ManagedSurface() {}
	BaseSurface(int width, int height) : Graphics::ManagedSurface(width, height) {}
	~BaseSurface() override {}
};

class XSurface : public BaseSurface {
public:
	XSurface() : BaseSurface() {}
	XSurface(int width, int height) : BaseSurface(width, height) {}
	~XSurface() override {}
};

} // End of namespace Xeen
} // End of namespace Shared
} // End of namespace MM

#endif
