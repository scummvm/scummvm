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

#ifndef MTROPOLIS_RENDER_H
#define MTROPOLIS_RENDER_H

#include "common/ptr.h"
#include "common/scummsys.h"

#include "graphics/pixelformat.h"

namespace Graphics {

class ManagedSurface;

} // End of namespace Graphics

namespace MTropolis {

class Runtime;
class Project;

class Window {
public:
	Window(Runtime *runtime, int32 x, int32 y, int16 width, int16 height, const Graphics::PixelFormat &format);
	~Window();

	int32 getX() const;
	int32 getY() const;
	void setPosition(int32 x, int32 y);

	const Common::SharedPtr<Graphics::ManagedSurface> &getSurface() const;
	const Graphics::PixelFormat &getPixelFormat() const;

	void close();
	void detachFromRuntime();

private:
	int32 _x;
	int32 _y;
	Common::SharedPtr<Graphics::ManagedSurface> _surface;
	Runtime *_runtime;
};

namespace Render {

uint32 resolveRGB(uint8 r, uint8 g, uint8 b, const Graphics::PixelFormat &fmt);
void renderProject(Runtime *runtime, Window *mainWindow);

} // End of namespace Render

} // End of namespace MTropolis

#endif
