/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PLAYGROUND3D_GFX_TINYGL_H
#define PLAYGROUND3D_GFX_TINYGL_H

#include "common/rect.h"
#include "common/system.h"

#include "math/vector3d.h"

#include "graphics/tinygl/zgl.h"

#include "engines/playground3d/gfx.h"

namespace Playground3d {

class TinyGLRenderer : public Renderer {
public:
	TinyGLRenderer(OSystem *_system);
	virtual ~TinyGLRenderer();

	virtual void init() override;

	virtual void clear(const Math::Vector4d &clearColor) override;

	virtual void setupViewport(int x, int y, int width, int height) override;
	virtual void drawCube(const Math::Vector3d &pos, const Math::Vector3d &roll) override;
	virtual void drawPolyOffsetTest(const Math::Vector3d &pos, const Math::Vector3d &roll) override;
	virtual void dimRegionInOut(float fade) override;
	virtual void drawInViewport() override;

	virtual void flipBuffer() override;

private:
	TinyGL::FrameBuffer *_fb;
	Math::Vector3d _pos;

	void drawFace(uint face);
};

} // End of namespace Playground3d

#endif // PLAYGROUND3D_GFX_TINYGL_H
