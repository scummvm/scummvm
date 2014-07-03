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

#include "engines/myst3/gfx.h"
#include "graphics/tinygl/zgl.h"

namespace Myst3 {

class TinyGLRenderer : public BaseRenderer {
public:
	TinyGLRenderer(OSystem *_system);
	virtual ~TinyGLRenderer();

	virtual void init(Graphics::PixelBuffer &screenBuffer);

	virtual void clear();
	virtual void setupCameraOrtho2D();
	virtual void setupCameraPerspective(float pitch, float heading, float fov);

	Texture *createTexture(const Graphics::Surface *surface);
	void freeTexture(Texture *texture);

	virtual void drawRect2D(const Common::Rect &rect, uint32 color);
	virtual void drawTexturedRect2D(const Common::Rect &screenRect, const Common::Rect &textureRect, Texture *texture, float transparency = -1.0, bool additiveBlending = false);
	virtual void drawTexturedRect3D(const Math::Vector3d &topLeft, const Math::Vector3d &bottomLeft,
	                                const Math::Vector3d &topRight, const Math::Vector3d &bottomRight,
	                                Texture *texture);

	virtual void drawCube(Texture **textures);
	virtual void draw2DText(const Common::String &text, const Common::Point &position);

	virtual Graphics::Surface *getScreenshot();

	virtual void screenPosToDirection(const Common::Point screen, float &pitch, float &heading);

private:
	void blitScreen(Texture *texture, int dstX, int dstY, int srcX, int srcY, int width, int height, float transparency, bool invertY = false);

	TinyGL::FrameBuffer *_fb;
	int _cubeViewport[4];
	double _cubeProjectionMatrix[16];
	double _cubeModelViewMatrix[16];

	bool _nonPowerOfTwoTexSupport;
};

} // end of namespace Myst3

#endif /* GFX_H_ */
