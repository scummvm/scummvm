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

#ifndef PHOENIXVR_VR_H
#define PHOENIXVR_VR_H

#include "common/stream.h"
#include "graphics/pixelformat.h"

namespace Graphics {
class Surface;
class Screen;
} // namespace Graphics

namespace PhoenixVR {
class VR {
	Common::ScopedPtr<Graphics::Surface> _pic;
	bool _vr = false;

public:
	~VR();
	VR() = default;
	VR(VR &&) = default;
	VR &operator=(VR &&) = default;

	static VR loadStatic(const Graphics::PixelFormat &format, Common::SeekableReadStream &s);
	void render(Graphics::Screen *screen, float ax, float ay, float fov);
	bool isVR() const { return _vr; }
};
} // namespace PhoenixVR

#endif
