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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/events.h"

#include "graphics/renderer.h"

#include "engines/util.h"

#include "playground3d/playground3d.h"

namespace Playground3d {

bool Playground3dEngine::hasFeature(EngineFeature f) const {
	// The TinyGL renderer does not support arbitrary resolutions for now
	Common::String rendererConfig = ConfMan.get("renderer");
	Graphics::RendererType desiredRendererType = Graphics::Renderer::parseTypeCode(rendererConfig);
	Graphics::RendererType matchingRendererType = Graphics::Renderer::getBestMatchingAvailableType(desiredRendererType,
#if defined(USE_OPENGL_GAME)
			Graphics::kRendererTypeOpenGL |
#endif
#if defined(USE_OPENGL_SHADERS)
			Graphics::kRendererTypeOpenGLShaders |
#endif
#if defined(USE_TINYGL)
			Graphics::kRendererTypeTinyGL |
#endif
			0);
	bool softRenderer = matchingRendererType == Graphics::kRendererTypeTinyGL;

	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsArbitraryResolutions && !softRenderer);
}

Playground3dEngine::Playground3dEngine(OSystem *syst)
		: Engine(syst), _system(syst), _gfx(nullptr), _frameLimiter(nullptr),
		_rotateAngleX(0), _rotateAngleY(0), _rotateAngleZ(0),
		_clearColor(0.0f, 0.0f, 0.0f, 1.0f), _fade(1.0f), _fadeIn(false),
		_rgbaTexture(nullptr), _rgbTexture(nullptr), _rgb565Texture(nullptr),
		_rgba5551Texture(nullptr), _rgba4444Texture(nullptr) {
}

Playground3dEngine::~Playground3dEngine() {
	delete _frameLimiter;
	delete _gfx;
}

Common::Error Playground3dEngine::run() {
	_gfx = createRenderer(_system);
	_gfx->init();

	_frameLimiter = new Graphics::FrameLimiter(_system, ConfMan.getInt("engine_speed"));

	_system->showMouse(true);

	// 1 - rotated colorfull cube
	// 2 - rotated two triangles with depth offset
	// 3 - fade in/out
	// 4 - moving filled rectangle in viewport
	// 5 - drawing RGBA pattern texture to check endian correctness
	int testId = 1;

	switch (testId) {
		case 1:
			_clearColor = Math::Vector4d(0.5f, 0.5f, 0.5f, 1.0f);
			_rotateAngleX = 45, _rotateAngleY = 45, _rotateAngleZ = 10;
			break;
		case 2:
			_clearColor = Math::Vector4d(0.5f, 0.5f, 0.5f, 1.0f);
			break;
		case 3:
			_clearColor = Math::Vector4d(1.0f, 0.0f, 0.0f, 1.0f);
			break;
		case 4:
			_clearColor = Math::Vector4d(0.5f, 0.5f, 0.5f, 1.0f);
			break;
		case 5: {
			_clearColor = Math::Vector4d(0.5f, 0.5f, 0.5f, 1.0f);
#if defined(SCUMM_LITTLE_ENDIAN)
			Graphics::PixelFormat pixelFormatRGBA(4, 8, 8, 8, 8, 0, 8, 16, 24);
			Graphics::PixelFormat pixelFormatRGB(3, 8, 8, 8, 0, 0, 8, 16, 0);
#else
			Graphics::PixelFormat pixelFormatRGBA(4, 8, 8, 8, 8, 24, 16, 8, 0);
			Graphics::PixelFormat pixelFormatRGB(3, 8, 8, 8, 0, 16, 8, 0, 0);
#endif
			Graphics::PixelFormat pixelFormatRGB565(2, 5, 6, 5, 0, 11, 5, 0, 0);
			Graphics::PixelFormat pixelFormatRGB5551(2, 5, 5, 5, 1, 11, 6, 1, 0);
			Graphics::PixelFormat pixelFormatRGB4444(2, 4, 4, 4, 4, 12, 8, 4, 0);
			_rgbaTexture = generateRgbaTexture(120, 120, pixelFormatRGBA);
			_rgbTexture = _rgbaTexture->convertTo(pixelFormatRGB);
			_rgb565Texture = generateRgbaTexture(120, 120, pixelFormatRGB565);
			_rgba5551Texture = generateRgbaTexture(120, 120, pixelFormatRGB5551);
			_rgba4444Texture = generateRgbaTexture(120, 120, pixelFormatRGB4444);
			break;
		}
		default:
			assert(false);
	}

	while (!shouldQuit()) {
		processInput();
		drawFrame(testId);
	}

	delete _rgbaTexture;
	delete _rgbTexture;
	delete _rgb565Texture;
	delete _rgba5551Texture;
	delete _rgba4444Texture;
	_gfx->deinit();
	_system->showMouse(false);

	return Common::kNoError;
}

void Playground3dEngine::processInput() {
	Common::Event event;

	while (getEventManager()->pollEvent(event)) {
		if (event.type == Common::EVENT_SCREEN_CHANGED) {
			_gfx->computeScreenViewport();
		}
	}
}

Graphics::Surface *Playground3dEngine::generateRgbaTexture(int width, int height, Graphics::PixelFormat format) {
	Graphics::Surface *surface = new Graphics::Surface;
	surface->create(width, height, format);
	const int barW = width / 4;
	Common::Rect r(0, 0, barW, height);
	uint32 pixel = format.ARGBToColor(255, 255, 0, 0);
	surface->fillRect(r, pixel);
	r.left += barW;
	r.right += barW;
	pixel = format.ARGBToColor(255, 0, 255, 0);
	surface->fillRect(r, pixel);
	r.left += barW;
	r.right += barW;
	pixel = format.ARGBToColor(255, 0, 0, 255);
	surface->fillRect(r, pixel);
	r.left += barW;
	r.right += barW;
	pixel = format.ARGBToColor(128, 0, 0, 0);
	surface->fillRect(r, pixel);
	return surface;
}

void Playground3dEngine::drawAndRotateCube() {
	Math::Vector3d pos = Math::Vector3d(0.0f, 0.0f, 6.0f);
	_gfx->drawCube(pos, Math::Vector3d(_rotateAngleX, _rotateAngleY, _rotateAngleZ));
	_rotateAngleX += 0.25;
	_rotateAngleY += 0.50;
	_rotateAngleZ += 0.10;
	if (_rotateAngleX >= 360)
		_rotateAngleX = 0;
	if (_rotateAngleY >= 360)
		_rotateAngleY = 0;
	if (_rotateAngleZ >= 360)
		_rotateAngleZ = 0;
}

void Playground3dEngine::drawPolyOffsetTest() {
	Math::Vector3d pos = Math::Vector3d(0.0f, 0.0f, 6.0f);
	_gfx->drawPolyOffsetTest(pos, Math::Vector3d(0, _rotateAngleY, 0));
	_rotateAngleY += 0.10;
	if (_rotateAngleY >= 360)
		_rotateAngleY = 0;
}

void Playground3dEngine::dimRegionInOut() {
	_gfx->dimRegionInOut(_fade);
	if (_fadeIn)
		_fade += 0.01f;
	else
		_fade -= 0.01f;
	if (_fade > 1.0f) {
		_fade = 1;
		_fadeIn = false;
	} else if (_fade < 0.0f) {
		_fade = 0;
		_fadeIn = true;
	}
}

void Playground3dEngine::drawInViewport() {
	_gfx->drawInViewport();
}

void Playground3dEngine::drawRgbaTexture() {
	_gfx->drawRgbaTexture();
}

void Playground3dEngine::drawFrame(int testId) {
	_gfx->clear(_clearColor);

	float pitch = 0.0f;
	float heading = 0.0f;
	float fov = 45.0f;
	_gfx->setupCameraPerspective(pitch, heading, fov);

	Common::Rect vp = _gfx->viewport();
	_gfx->setupViewport(vp.left, _system->getHeight() - vp.top - vp.height(), vp.width(), vp.height());

	switch (testId) {
		case 1:
			drawAndRotateCube();
			break;
		case 2:
			drawPolyOffsetTest();
			break;
		case 3:
			dimRegionInOut();
			break;
		case 4:
			_gfx->setupViewport(vp.left + 40, _system->getHeight() - vp.top - vp.height() + 40, vp.width() - 80, vp.height() - 80);
			drawInViewport();
			break;
		case 5:
			_gfx->loadTextureRGBA(_rgbaTexture);
			_gfx->loadTextureRGB(_rgbTexture);
			_gfx->loadTextureRGB565(_rgb565Texture);
			_gfx->loadTextureRGBA5551(_rgba5551Texture);
			_gfx->loadTextureRGBA4444(_rgba4444Texture);
			drawRgbaTexture();
			break;
		default:
			assert(false);
	}

	_gfx->flipBuffer();

	_frameLimiter->delayBeforeSwap();
	_system->updateScreen();
	_frameLimiter->startFrame();
}

} // End of namespace Playground3d
