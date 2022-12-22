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

#include "common/rendermode.h"
#include "common/rect.h"

#include "graphics/pixelformat.h"
#include "graphics/renderer.h"
#include "math/frustum.h"
#include "math/vector3d.h"

namespace Freescape {

#define kVertexArraySize 20
#define kCoordsArraySize 4

typedef Common::Array<byte *> ColorMap;
typedef Common::HashMap<int, int> ColorReMap;

class Renderer;

class Texture {
public:
	Texture(){ _width = 0; _height = 0; };
	virtual ~Texture(){};

	uint _width;
	uint _height;
	Graphics::PixelFormat _format;

	virtual void update(const Graphics::Surface *surface) = 0;
	virtual void updatePartial(const Graphics::Surface *surface, const Common::Rect &rect) = 0;

	static const Graphics::PixelFormat getRGBAPixelFormat();
};

class Renderer {
public:
	Renderer(int screenW, int screenH, Common::RenderMode renderMode);
	virtual ~Renderer();

	Graphics::PixelFormat _currentPixelFormat;
	Graphics::PixelFormat _palettePixelFormat;
	Graphics::PixelFormat _texturePixelFormat;
	bool _isAccelerated;

	virtual void init() = 0;
	virtual void setViewport(const Common::Rect &rect) = 0;

	/**
	 *  Swap the buffers, making the drawn screen visible
	 */
	virtual void flipBuffer() {}
	virtual void useColor(uint8 r, uint8 g, uint8 b) = 0;
	virtual void polygonOffset(bool enabled) = 0;

	virtual Texture *createTexture(const Graphics::Surface *surface) = 0;
	void convertImageFormatIfNecessary(Graphics::Surface *surface);

	virtual void freeTexture(Texture *texture) = 0;
	virtual void drawTexturedRect2D(const Common::Rect &screenRect, const Common::Rect &textureRect, Texture *texture) = 0;

	virtual void renderSensorShoot(byte color, const Math::Vector3d sensor, const Math::Vector3d player, const Common::Rect viewPort) = 0;
	virtual void renderPlayerShoot(byte color, const Common::Point position, const Common::Rect viewPort) = 0;
	virtual void renderCube(const Math::Vector3d &position, const Math::Vector3d &size, Common::Array<uint8> *colours);
	virtual void renderRectangle(const Math::Vector3d &position, const Math::Vector3d &size, Common::Array<uint8> *colours);
	virtual void renderPolygon(const Math::Vector3d &origin, const Math::Vector3d &size, const Common::Array<uint16> *ordinates, Common::Array<uint8> *colours);
	virtual void renderPyramid(const Math::Vector3d &origin, const Math::Vector3d &size, const Common::Array<uint16> *ordinates, Common::Array<uint8> *colours, int type);
	virtual void renderFace(const Common::Array<Math::Vector3d> &vertices) = 0;

	void setColorRemaps(ColorReMap *colorRemaps);
	virtual void clear(uint8 color) = 0;
	virtual void drawFloor(uint8 color) = 0;

	Common::Rect viewport() const;

	// palette
	void readFromPalette(uint8 index, uint8 &r, uint8 &g, uint8 &b);
	uint8 indexFromColor(uint8 r, uint8 g, uint8 b);
	bool getRGBAt(uint8 index, uint8 &r1, uint8 &g1, uint8 &b1, uint8 &r2, uint8 &g2, uint8 &b2);
	bool getRGBAtCGA(uint8 index, uint8 &r1, uint8 &g1, uint8 &b1, uint8 &r2, uint8 &g2, uint8 &b2);
	bool getRGBAtEGA(uint8 index, uint8 &r1, uint8 &g1, uint8 &b1, uint8 &r2, uint8 &g2, uint8 &b2);

	virtual void useStipple(bool enabled) {};
	byte *_palette;
	ColorMap *_colorMap;
	ColorReMap *_colorRemaps;
	int _keyColor;

	/**
	 * Select the window where to render
	 *
	 * This also sets the viewport
	 */

	virtual void positionCamera(const Math::Vector3d &pos, const Math::Vector3d &interest) = 0;
	virtual void updateProjectionMatrix(float fov, float nearClipPlane, float farClipPlane) = 0;

	Math::Matrix4 getMvpMatrix() const { return _mvpMatrix; }
	virtual Graphics::Surface *getScreenshot() = 0;
	void flipVertical(Graphics::Surface *s);

	int _screenW;
	int _screenH;
	Common::RenderMode _renderMode;

	bool computeScreenViewport();

protected:
	Common::Rect _screenViewport;
	Common::Rect _viewport;
	Common::Rect _unscaledViewport;

	Math::Matrix4 _projectionMatrix;
	Math::Matrix4 _modelViewMatrix;
	Math::Matrix4 _mvpMatrix;

	Math::Frustum _frustum;

	Math::Matrix4 makeProjectionMatrix(float fov, float nearClipPlane, float farClipPlane) const;
};

Graphics::RendererType determinateRenderType();
Renderer *CreateGfxOpenGL(int screenW, int screenH, Common::RenderMode renderMode);
Renderer *CreateGfxOpenGLShader(int screenW, int screenH, Common::RenderMode renderMode);
Renderer *CreateGfxTinyGL(int screenW, int screenH, Common::RenderMode renderMode);
Renderer *createRenderer(int screenW, int screenH, Common::RenderMode renderMode);

} // End of namespace Freescape

#endif // FREESCAPE_GFX_H
