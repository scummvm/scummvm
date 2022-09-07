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

#ifndef FREESCAPE_GFX_H
#define FREESCAPE_GFX_H

#include "common/rect.h"
#include "common/system.h"

#include "math/frustum.h"
#include "math/matrix4.h"
#include "math/vector3d.h"
#include "graphics/tinygl/pixelbuffer.h"

namespace Freescape {

typedef Common::Array<byte*> ColorMap;

class Renderer;

class Texture {
public:
	uint width;
	uint height;
	Graphics::PixelFormat format;

	virtual void update(const Graphics::Surface *surface) = 0;
	virtual void updatePartial(const Graphics::Surface *surface, const Common::Rect &rect) = 0;

	static const Graphics::PixelFormat getRGBAPixelFormat();
protected:
	Texture() {}
	virtual ~Texture() {}
};

class Renderer {
public:
	Renderer(OSystem *system);
	virtual ~Renderer();

	Graphics::PixelFormat _currentPixelFormat;
	Graphics::PixelFormat _originalPixelFormat;
	Graphics::PixelFormat _palettePixelFormat;

    /**
	 *   Convert from paletted surface
     */
	Graphics::Surface *convertFromPalette(Graphics::PixelBuffer *rawsurf);


	virtual void init() = 0;
	virtual void clear() = 0;
	virtual void setViewport(const Common::Rect &rect) = 0;

	typedef enum {
		EastPyramid = 4,
		WestPyramid = 5,
		UpPyramid = 6,
		DownPyramid = 7,
		NorthPyramid = 8,
		SouthPyramid = 9,
	} PyramidType;

	/**
	 *  Swap the buffers, making the drawn screen visible
	 */
	virtual void flipBuffer() { }

	virtual Texture *createTexture(const Graphics::Surface *surface) = 0;
	virtual void freeTexture(Texture *texture) = 0;
	virtual void drawTexturedRect2D(const Common::Rect &screenRect, const Common::Rect &textureRect, Texture *texture) = 0;
	virtual void drawRect2D(const Common::Rect &rect, uint8 a, uint8 r, uint8 g, uint8 b) = 0;

	virtual void renderCrossair(byte color) = 0;
	virtual void renderShoot(byte color) = 0;
	virtual void renderCube(const Math::Vector3d &position, const Math::Vector3d &size, Common::Array<uint8> *colours) = 0;
	virtual void renderRectangle(const Math::Vector3d &position, const Math::Vector3d &size, Common::Array<uint8> *colours) = 0;
	virtual void renderPolygon(const Math::Vector3d &origin, const Math::Vector3d &size, const Common::Array<uint16> *ordinates, Common::Array<uint8> *colours) = 0;
	virtual void renderPyramid(const Math::Vector3d &origin, const Math::Vector3d &size, const Common::Array<uint16> *ordinates, Common::Array<uint8> *colours, int type) = 0;
	virtual void renderFace(const Common::Array<Math::Vector3d> &vertices) = 0;

	virtual void setSkyColor(uint8 color) = 0;
	virtual void drawFloor(uint8 color) = 0;

	Common::Rect viewport() const;

	// palette
	bool getRGBAt(uint8 index, uint8 &r, uint8 &g, uint8 &b);
	Graphics::PixelBuffer *_palette;
	ColorMap *_colorMap;
	int _keyColor;

	/**
	 * Select the window where to render
	 *
	 * This also sets the viewport
	 */

	virtual void positionCamera(const Math::Vector3d &pos, const Math::Vector3d &interest) = 0;
	virtual void updateProjectionMatrix(float fov, float nearClipPlane, float farClipPlane) = 0;

	Math::Matrix4 getMvpMatrix() const { return _mvpMatrix; }

	void flipVertical(Graphics::Surface *s);

	static const int kOriginalWidth = 320;
	static const int kOriginalHeight = 200;
	static const int kFrameHeight = 200;

	void computeScreenViewport();

protected:
	OSystem *_system;
	Common::Rect _screenViewport;

	Math::Matrix4 _projectionMatrix;
	Math::Matrix4 _modelViewMatrix;
	Math::Matrix4 _mvpMatrix;

	Math::Frustum _frustum;

	Math::Matrix4 makeProjectionMatrix(float fov, float nearClipPlane, float farClipPlane) const;
};

/**
 * A framerate limiter
 *
 * Ensures the framerate does not exceed the specified value
 * by delaying until all of the timeslot allocated to the frame
 * is consumed.
 * Allows to curb CPU usage and have a stable framerate.
 */
class FrameLimiter {
public:
	FrameLimiter(OSystem *system, const uint framerate);

	void startFrame();
	void delayBeforeSwap();
private:
	OSystem *_system;

	bool _enabled;
	uint _speedLimitMs;
	uint _startFrameTime;
};

Renderer *CreateGfxOpenGL(OSystem *system);
Renderer *CreateGfxOpenGLShader(OSystem *system);
Renderer *CreateGfxTinyGL(OSystem *system);
Renderer *createRenderer(OSystem *system);

} // End of namespace Freescape

#endif // GFX_H_
