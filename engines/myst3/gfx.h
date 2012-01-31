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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GFX_H_
#define GFX_H_

#include "common/rect.h"
#include "common/system.h"
#include "math/vector3d.h"

namespace Myst3 {

class Drawable {
public:
	virtual void draw() {}
	virtual void drawOverlay() {}
	virtual ~Drawable() {}
};

class Texture {
public:
	uint width;
	uint height;
	Graphics::PixelFormat format;

	virtual void update(const Graphics::Surface *surface) = 0;
protected:
	Texture() {}
	virtual ~Texture() {}
};

class Renderer {
public:
	Renderer(OSystem *_system);
	virtual ~Renderer();

	void init();
	void initFont(const Graphics::Surface *surface);

	void clear();
	void setupCameraOrtho2D();
	void setupCameraPerspective(float pitch, float heading, float fov);

	Texture *createTexture(const Graphics::Surface *surface);
	void freeTexture(Texture *texture);

	void drawRect2D(const Common::Rect &rect, uint32 color);
	void drawTexturedRect2D(const Common::Rect &screenRect, const Common::Rect &textureRect, Texture *texture, float transparency = -1.0);
	void drawTexturedRect3D(const Math::Vector3d &topLeft, const Math::Vector3d &bottomLeft,
			const Math::Vector3d &topRight, const Math::Vector3d &bottomRight, Texture *texture);

	void drawCube(Texture **textures);
	void draw2DText(const Common::String &text, const Common::Point &position);

	Graphics::Surface *getScreenshot();

	void screenPosToDirection(const Common::Point screen, float &pitch, float &heading);

	static const int kOriginalWidth = 640;
	static const int kOriginalHeight = 480;
	static const int kTopBorderHeight = 30;
	static const int kBottomBorderHeight = 90;
	static const int kFrameHeight = 360;

protected:
	OSystem *_system;
	Texture *_font;

	int _cubeViewport[4];
	double _cubeProjectionMatrix[16];
	double _cubeModelViewMatrix[16];

	Common::Rect getFontCharacterRect(uint8 character);
};

} // end of namespace Myst3

#endif /* GFX_H_ */
