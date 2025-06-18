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

#include "common/hashmap.h"
#include "common/rendermode.h"
#include "common/rect.h"

#include "graphics/pixelformat.h"
#include "graphics/managed_surface.h"
#include "graphics/renderer.h"
#include "math/frustum.h"
#include "math/vector3d.h"

namespace Freescape {

#define kVertexArraySize 128
#define kCoordsArraySize 4

typedef Common::Array<byte *> ColorMap;
typedef Common::HashMap<int, int> ColorReMap;

class Renderer;

const Graphics::PixelFormat getRGBAPixelFormat();

class Texture {
public:
	Texture(){ _width = 0; _height = 0; };
	virtual ~Texture(){};

	uint _width;
	uint _height;
	Graphics::PixelFormat _format;

	virtual void update(const Graphics::Surface *surface) = 0;
	virtual void updatePartial(const Graphics::Surface *surface, const Common::Rect &rect) = 0;
};

class Renderer {
public:
	Renderer(int screenW, int screenH, Common::RenderMode renderMode, bool authenticGraphics);
	virtual ~Renderer();

	Graphics::PixelFormat _currentPixelFormat;
	Graphics::PixelFormat _texturePixelFormat;
	bool _isAccelerated;
	bool _authenticGraphics;

	virtual void init() = 0;
	virtual void setViewport(const Common::Rect &rect) = 0;

	/**
	 *  Swap the buffers, making the drawn screen visible
	 */
	virtual void flipBuffer() {}
	virtual void useColor(uint8 r, uint8 g, uint8 b) = 0;
	virtual void depthTesting(bool enabled) {};
	virtual void polygonOffset(bool enabled) = 0;

	virtual Texture *createTexture(const Graphics::Surface *surface, bool is3D = false) = 0;
	Graphics::Surface *convertImageFormatIfNecessary(Graphics::ManagedSurface *surface);

	virtual void freeTexture(Texture *texture) = 0;
	virtual void drawTexturedRect2D(const Common::Rect &screenRect, const Common::Rect &textureRect, Texture *texture) = 0;

	virtual void renderSensorShoot(byte color, const Math::Vector3d sensor, const Math::Vector3d player, const Common::Rect &viewPort) = 0;
	virtual void renderPlayerShootBall(byte color, const Common::Point &position, int frame, const Common::Rect &viewPort) = 0;
	virtual void renderPlayerShootRay(byte color, const Common::Point &position, const Common::Rect &viewPort) = 0;

	virtual void renderCrossair(const Common::Point &crossairPosition) = 0;

	virtual void renderCube(const Math::Vector3d &position, const Math::Vector3d &size, Common::Array<uint8> *colours, Common::Array<uint8> *ecolours, float offset = 0.0);
	virtual void renderRectangle(const Math::Vector3d &position, const Math::Vector3d &size, Common::Array<uint8> *colours, Common::Array<uint8> *ecolours, float offset = 0.0);
	virtual void renderPolygon(const Math::Vector3d &origin, const Math::Vector3d &size, const Common::Array<float> *ordinates, Common::Array<uint8> *colours, Common::Array<uint8> *ecolours, float offset = 0.0);
	virtual void renderPyramid(const Math::Vector3d &origin, const Math::Vector3d &size, const Common::Array<float> *ordinates, Common::Array<uint8> *colours, Common::Array<uint8> *ecolours, int type);
	virtual void renderFace(const Common::Array<Math::Vector3d> &vertices) = 0;

	void setColorRemaps(ColorReMap *colorRemaps);
	virtual void clear(uint8 r, uint8 g, uint8 b, bool ignoreViewport = false) = 0;
	virtual void drawFloor(uint8 color) = 0;
	virtual void drawBackground(uint8 color);

	void drawEclipse(uint8 color1, uint8 color2, float difference);
	virtual void drawSkybox(Texture *texture, Math::Vector3d camera) {};
	virtual void drawCelestialBody(Math::Vector3d position, float radius, uint8 color) {};

	Common::Rect viewport() const;
	virtual Common::Point nativeResolution() { return Common::Point(_screenW, _screenH); }

	// palette
	void readFromPalette(uint8 index, uint8 &r, uint8 &g, uint8 &b);
	void setPaletteValue(uint8 index, uint8 r, uint8 g, uint8 b);
	uint8 indexFromColor(uint8 r, uint8 g, uint8 b);
	uint8 mapEGAColor(uint8 index);

	bool getRGBAt(uint8 index, uint8 ecolor, uint8 &r1, uint8 &g1, uint8 &b1, uint8 &r2, uint8 &g2, uint8 &b2, byte *&stipple);
	bool getRGBAtC64(uint8 index, uint8 &r1, uint8 &g1, uint8 &b1, uint8 &r2, uint8 &g2, uint8 &b2, byte *&stipple);
	bool getRGBAtCGA(uint8 index, uint8 &r1, uint8 &g1, uint8 &b1, uint8 &r2, uint8 &g2, uint8 &b2, byte *&stipple);
	bool getRGBAtCPC(uint8 index, uint8 &r1, uint8 &g1, uint8 &b1, uint8 &r2, uint8 &g2, uint8 &b2, byte *&stipple);
	bool getRGBAtEGA(uint8 index, uint8 &r1, uint8 &g1, uint8 &b1, uint8 &r2, uint8 &g2, uint8 &b2);
	bool getRGBAtZX(uint8 index, uint8 &r1, uint8 &g1, uint8 &b1, uint8 &r2, uint8 &g2, uint8 &b2, byte *&stipple);
	bool getRGBAtHercules(uint8 index, uint8 &r1, uint8 &g1, uint8 &b1, uint8 &r2, uint8 &g2, uint8 &b2, byte *&stipple);
	void extractCPCIndexes(uint8 cm1, uint8 cm2, uint8 &i1, uint8 &i2);
	void extractC64Indexes(uint8 cm1, uint8 cm2, uint8 &i1, uint8 &i2);

	void selectColorFromFourColorPalette(uint8 index, uint8 &r1, uint8 &g1, uint8 &b1);

	// Stipple
	virtual void setStippleData(byte *data) {};
	virtual void useStipple(bool enabled) {};
	void scaleStipplePattern(byte originalPattern[128], byte newPattern[128]);

	byte _defaultStippleArray[128] = {
		0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
		0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
		0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
		0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
		0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
		0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
		0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
		0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
		0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
		0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
		0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
		0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
		0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
		0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
		0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
		0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
	};

	byte *_variableStippleArray;

	float _skyNormals[16][3] = {
		{ 0.0, 0.0, 1.0 }, //front //0
		{ 0.0, 0.0, 1.0 },		//1
		{ 0.0, 0.0, 1.0 },		//2
		{ 0.0, 0.0, 1.0 },		//3
		{ 0.0, 0.0, -1.0 }, //back //0
		{ 0.0, 0.0, -1.0 },		//1
		{ 0.0, 0.0, -1.0 },		//2
		{ 0.0, 0.0, -1.0 },		//3
		{ -1.0, 0.0, 0.0 }, //left
		{ -1.0, 0.0, 0.0 },
		{ -1.0, 0.0, 0.0 },
		{ -1.0, 0.0, 0.0 },
		{ 1.0, 0.0, 0.0 }, //right
		{ 1.0, 0.0, 0.0 },
		{ 1.0, 0.0, 0.0 },
		{ 1.0, 0.0, 0.0 }
	};

	float _skyUvs1008[16][2] = {
		{ 0.0f, 0.0f }, //1
		{ 0.0f, 2.0f }, //2
		{ 0.4f, 2.0f }, //3
		{ 0.4f, 0.0f }, //front //4

		{ 0.0f, 2.0f }, //back //1
		{ 0.4f, 2.0f }, //2
		{ 0.4f, 0.0f }, //3
		{ 0.0f, 0.0f }, //4

		{ 0.0f, 0.0f }, //left //1
		{ 0.4f, 0.0f }, //2
		{ 0.4f, 2.0f }, //3
		{ 0.0f, 2.0f }, //4

		{ 0.4f, 0.0f }, //right //1
		{ 0.0f, 0.0f }, //2
		{ 0.0f, 2.0f }, //3
		{ 0.4f, 2.0f }, //4
	};

	float _skyUvs128[16][2] = {
		{ 0.0f, 0.0f }, //1
		{ 0.0f, 2.0f }, //2
		{ 2.5f, 2.0f }, //3
		{ 2.5f, 0.0f }, //front //4

		{ 0.0f, 2.0f }, //back //1
		{ 2.5f, 2.0f }, //2
		{ 2.5f, 0.0f }, //3
		{ 0.0f, 0.0f }, //4

		{ 0.0f, 0.0f }, //left //1
		{ 2.5f, 0.0f }, //2
		{ 2.5f, 2.0f }, //3
		{ 0.0f, 2.0f }, //4

		{ 2.5f, 0.0f }, //right //1
		{ 0.0f, 0.0f }, //2
		{ 0.0f, 2.0f }, //3
		{ 2.5f, 2.0f }, //4
	};

	float _skyVertices[16][3] = {
		{ -81280.0, 8128.0, 81280.0 },          //1     // Vertex #0 front
		{ -81280.0, -8128.0, 81280.0 }, //2     // Vertex #1
		{ 81280.0,  -8128.0, 81280.0 }, //3     // Vertex #2
		{ 81280.0,  8128.0, 81280.0 },          //4     // Vertex #3

		{ 81280.0f, -8128.0f, -81280.0f }, // 1
		{ -81280.0f, -8128.0f, -81280.0f }, // 2
		{ -81280.0f, 8128.0f, -81280.0f }, // 3
		{ 81280.0f, 8128.0f, -81280.0f }, // 4

		{ -81280.0f,  8128.0f,  81280.0f }, //left //1
		{ -81280.0f,  8128.0f, -81280.0f }, //2
		{ -81280.0f, -8128.0f, -81280.0f }, //3
		{ -81280.0f, -8128.0f,  81280.0f }, //4

		{ 81280.0f,  8128.0f, -81280.0f }, //right //1
		{ 81280.0f,  8128.0f,  81280.0f }, //2
		{ 81280.0f, -8128.0f,  81280.0f },//3
		{ 81280.0f, -8128.0f, -81280.0f },//4
	};

	unsigned int _skyIndices[24] = {
		0, 1, 2, 	// front
		0, 2, 3,

		4, 5, 6, 	// back
		4, 6, 7,

		8, 9, 10, 	// left
		8, 10, 11,

		12, 13, 14, // right
		12, 14, 15,
	};

	byte *_palette;
	void setColorMap(ColorMap *colorMap_);
	ColorMap *_colorMap;
	ColorReMap *_colorRemaps;
	void clearColorPairArray();
	void fillColorPairArray();
	byte _colorPair[16];
	int _keyColor;
	int _inkColor;
	int _paperColor;
	int _underFireBackgroundColor;
	Common::Point _shakeOffset;
	byte _stipples[16][128];

	int _scale;

	/**
	 * Select the window where to render
	 *
	 * This also sets the viewport
	 */

	virtual void positionCamera(const Math::Vector3d &pos, const Math::Vector3d &interest, float rollAngle = 0.0f) = 0;
	virtual void updateProjectionMatrix(float fov, float aspectRatio, float nearClipPlane, float farClipPlane) = 0;

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
Renderer *CreateGfxOpenGL(int screenW, int screenH, Common::RenderMode renderMode, bool authenticGraphics);
Renderer *CreateGfxOpenGLShader(int screenW, int screenH, Common::RenderMode renderMode, bool authenticGraphics);
Renderer *CreateGfxTinyGL(int screenW, int screenH, Common::RenderMode renderMode);
Renderer *createRenderer(int screenW, int screenH, Common::RenderMode renderMode, bool authenticGraphics);

} // End of namespace Freescape

#endif // FREESCAPE_GFX_H
