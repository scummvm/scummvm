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

#ifndef FREESCAPE_GFX_TINYGL_H_
#define FREESCAPE_GFX_TINYGL_H_

#include "common/rect.h"
#include "math/vector3d.h"

#include "engines/freescape/gfx.h"

namespace Freescape {

class TinyGLRenderer : public Renderer {
public:
	TinyGLRenderer(OSystem *_system);
	virtual ~TinyGLRenderer();

	virtual void init() override;
	virtual void clear() override;
	virtual void setViewport(const Common::Rect &rect) override;
	virtual void positionCamera(const Math::Vector3d &pos, const Math::Vector3d &interest) override;
	virtual void updateProjectionMatrix(float fov, float nearClipPlane, float farClipPlane) override;

	Texture *createTexture(const Graphics::Surface *surface) override;
	void freeTexture(Texture *texture) override;
	virtual void drawTexturedRect2D(const Common::Rect &screenRect, const Common::Rect &textureRect, Texture *texture) override;

	virtual void renderCrossair(byte color) override;
	virtual void renderShoot(byte color) override;
	virtual void renderCube(const Math::Vector3d &position, const Math::Vector3d &size, Common::Array<uint8> *colours) override;
	virtual void renderRectangle(const Math::Vector3d &position, const Math::Vector3d &size, Common::Array<uint8> *colours) override;
	virtual void renderPolygon(const Math::Vector3d &origin, const Math::Vector3d &size, const Common::Array<uint16> *ordinates, Common::Array<uint8> *colours) override;
	virtual void renderPyramid(const Math::Vector3d &origin, const Math::Vector3d &size, const Common::Array<uint16> *ordinates, Common::Array<uint8> *colours, int type);
    virtual void renderFace(const Common::Array<Math::Vector3d> &vertices) override;

	virtual void flipBuffer() override;
	virtual void drawSky(uint8 color) override;
	virtual void drawFloor(uint8 color) override;
};

} // End of namespace Freescape

#endif
