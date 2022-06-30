/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef GFX_TINYGL_H_
#define GFX_TINYGL_H_

#include "common/rect.h"
#include "common/system.h"
#include "math/vector3d.h"

#include "engines/freescape/gfx.h"
#include "graphics/tinygl/zgl.h"

namespace Freescape {

class TinyGLRenderer : public Renderer {
public:
	TinyGLRenderer(OSystem *_system);
	virtual ~TinyGLRenderer();

	virtual void init() override;
	virtual void clear() override;
	virtual void scale(const Math::Vector3d &scale) override;
	virtual void positionCamera(const Math::Vector3d &pos, const Math::Vector3d &interest) override;
	virtual void updateProjectionMatrix(float fov, float nearClipPlane, float farClipPlane) override;

	Texture *createTexture(const Graphics::Surface *surface) override;
	void freeTexture(Texture *texture) override;

	virtual void drawRect2D(const Common::Rect &rect, uint8 a, uint8 r, uint8 g, uint8 b) override;
	virtual void drawTexturedRect2D(const Common::Rect &screenRect, const Common::Rect &textureRect, Texture *texture,
	                                float transparency = -1.0, bool additiveBlending = false) override;
	virtual void draw2DText(const Common::String &text, const Common::Point &position) override;

	virtual void renderCube(const Math::Vector3d &position, const Math::Vector3d &size, Common::Array<uint8> *colours) override;
	virtual void renderRectangle(const Math::Vector3d &position, const Math::Vector3d &size, Common::Array<uint8> *colours) override;
	virtual void renderPolygon(const Math::Vector3d &origin, const Math::Vector3d &size, const Common::Array<uint16> *ordinates, Common::Array<uint8> *colours) override;
	virtual void renderPyramid(const Math::Vector3d &origin, const Math::Vector3d &size, const Common::Array<uint16> *ordinates, Common::Array<uint8> *colours, int type);

    virtual void renderFace(const Common::Array<Math::Vector3d> &vertices) override;

	virtual void flipBuffer() override;
	virtual void drawSky(uint8 color) override;
	virtual void drawFloor(uint8 color) override;
};

} // End of namespace Myst3

#endif // GFX_H_
