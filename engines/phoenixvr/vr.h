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

#include "common/array.h"
#include "common/random.h"
#include "common/stream.h"
#include "graphics/managed_surface.h"
#include "graphics/pixelformat.h"

namespace Graphics {
struct Surface;
class Screen;
} // namespace Graphics

namespace PhoenixVR {
class RegionSet;
class VR {
	Common::ScopedPtr<Graphics::ManagedSurface> _pic;
	bool _vr = false;
	struct Animation {
		struct Frame {
			Common::Array<byte> blockData;
			int restartAtFrame = -1;
			void render(Graphics::Surface &pic) const;
		};

		Common::String name;
		Common::Array<Frame> frames;

		bool active = false;
		float t = 0;
		float speed = 25.0f;

		unsigned frameIndex = 0;

		Common::String variable;
		int variableValue = 0;
		void renderNextFrame(Graphics::Surface &pic);
		void render(Graphics::Surface &pic, float dt);
	};
	Common::Array<Animation> _animations;
	Common::RandomSource _rnd = {"vr"};

public:
	static VR loadStatic(const Graphics::PixelFormat &format, Common::SeekableReadStream &s);
	void render(Graphics::Screen *screen, float ax, float ay, float fov, float dt, RegionSet *regSet);
	bool isVR() const { return _vr; }
	void playAnimation(const Common::String &name, const Common::String &variable, int value, float speed);
	Graphics::Surface &getSurface() { return *_pic->surfacePtr(); }
};
} // namespace PhoenixVR

#endif
