/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#define FORBIDDEN_SYMBOL_EXCEPTION_putenv

#include "common/scummsys.h"

#if defined(SDL_BACKEND)

#include "backends/graphics/surfacesdl/surfacesdl-graphics.h"
#include "backends/events/sdl/sdl-events.h"
#include "backends/platform/sdl/sdl.h"
#include "common/config-manager.h"
#include "common/mutex.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "common/util.h"
#ifdef USE_RGB_COLOR
#include "common/list.h"
#endif
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/scaler.h"
#include "graphics/surface.h"
#include "graphics/pixelbuffer.h"
#include "gui/EventRecorder.h"

#ifdef USE_OPENGL
#include "graphics/opengl/extensions.h"
#endif

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{0, 0, 0}
};

SurfaceSdlGraphicsManager::SurfaceSdlGraphicsManager(SdlEventSource *sdlEventSource, SdlWindow *window)
	:
	SdlGraphicsManager(sdlEventSource, window),
#if SDL_VERSION_ATLEAST(2, 0, 0)
	_renderer(nullptr), _screenTexture(nullptr),
#endif
	_screen(0),
	_subScreen(0),
	_overlayVisible(false),
	_overlayscreen(0),
	_overlayWidth(0), _overlayHeight(0),
	_overlayDirty(true),
	_screenChangeCount(0),
	_lockAspectRatio(true),
	_gameRect()
#ifdef USE_OPENGL
	, _opengl(false), _overlayNumTex(0), _overlayTexIds(0)
	, _frameBuffer(nullptr)
#endif
#ifdef USE_OPENGL_SHADERS
	, _boxShader(nullptr), _boxVerticesVBO(0)
#endif
	{
		const SDL_VideoInfo *vi = SDL_GetVideoInfo();
		_desktopW = vi->current_w;
		_desktopH = vi->current_h;
		_sideSurfaces[0] = _sideSurfaces[1] = nullptr;
#ifdef USE_OPENGL
		_sideTextures[0] = _sideTextures[1] = nullptr;
#endif
}

SurfaceSdlGraphicsManager::~SurfaceSdlGraphicsManager() {
	closeOverlay();
}

void SurfaceSdlGraphicsManager::activateManager() {
	SdlGraphicsManager::activateManager();

	// Register the graphics manager as a event observer
	g_system->getEventManager()->getEventDispatcher()->registerObserver(this, 10, false);
}

void SurfaceSdlGraphicsManager::deactivateManager() {
	// Unregister the event observer
	if (g_system->getEventManager()->getEventDispatcher()) {
		g_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);
	}

	SdlGraphicsManager::deactivateManager();
}

void SurfaceSdlGraphicsManager::resetGraphicsScale() {
	setGraphicsMode(0);
}

bool SurfaceSdlGraphicsManager::hasFeature(OSystem::Feature f) {
	return
		(f == OSystem::kFeatureFullscreenMode) ||
#ifdef USE_OPENGL
		(f == OSystem::kFeatureOpenGL) ||
		(f == OSystem::kFeatureAspectRatioCorrection);
#else
	false;
#endif
}

void SurfaceSdlGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
	switch (f) {
	case OSystem::kFeatureFullscreenMode:
		_fullscreen = enable;
		break;
	case OSystem::kFeatureAspectRatioCorrection:
		_lockAspectRatio = enable;
		break;
	default:
		break;
	}
}

bool SurfaceSdlGraphicsManager::getFeatureState(OSystem::Feature f) {
	switch (f) {
		case OSystem::kFeatureFullscreenMode:
			return _fullscreen;
		case OSystem::kFeatureAspectRatioCorrection:
			return _lockAspectRatio;
		break;
		default:
			return false;
	}
}

const OSystem::GraphicsMode *SurfaceSdlGraphicsManager::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}

int SurfaceSdlGraphicsManager::getDefaultGraphicsMode() const {
	return 0;// ResidualVM: not use it
}

void SurfaceSdlGraphicsManager::beginGFXTransaction() {
	// ResidualVM: not use it
}

OSystem::TransactionError SurfaceSdlGraphicsManager::endGFXTransaction() {
	// ResidualVM: not use it
	return OSystem::kTransactionSuccess;
}

#ifdef USE_RGB_COLOR
Common::List<Graphics::PixelFormat> SurfaceSdlGraphicsManager::getSupportedFormats() const {
	// ResidualVM: not use it
	return _supportedFormats;
}
#endif

bool SurfaceSdlGraphicsManager::setGraphicsMode(int mode) {
	// ResidualVM: not use it
	return true;
}

int SurfaceSdlGraphicsManager::getGraphicsMode() const {
	// ResidualVM: not use it
	return 0;
}

void SurfaceSdlGraphicsManager::initSize(uint w, uint h, const Graphics::PixelFormat *format) {
	// ResidualVM: not use it
}

void SurfaceSdlGraphicsManager::launcherInitSize(uint w, uint h) {
	closeOverlay();
	setupScreen(w, h, false, false);
}

Graphics::PixelBuffer SurfaceSdlGraphicsManager::setupScreen(uint screenW, uint screenH, bool fullscreen, bool accel3d) {
	uint32 sdlflags;
	int bpp;

	closeOverlay();

#ifdef USE_OPENGL
	_opengl = accel3d;
	_antialiasing = 0;
#endif
	_fullscreen = fullscreen;

	ConfMan.registerDefault("fullscreen_res", "desktop");
	const Common::String &fsres = ConfMan.get("fullscreen_res");
	if (fsres != "desktop") {
		uint newW, newH;
		int converted = sscanf(fsres.c_str(), "%ux%u", &newW, &newH);
		if (converted == 2) {
			_desktopW = newW;
			_desktopH = newH;
		} else {
			warning("Could not parse 'fullscreen_res' option: expected WWWxHHH, got %s", fsres.c_str());
		}
	}

	ConfMan.registerDefault("aspect_ratio", true);
	_lockAspectRatio = ConfMan.getBool("aspect_ratio");
	uint fbW = screenW;
	uint fbH = screenH;
	_gameRect = Math::Rect2d(Math::Vector2d(0, 0), Math::Vector2d(1, 1));

#ifdef USE_OPENGL
	bool framebufferSupported = false;

	// Use the desktop resolution for fullscreen when possible
	if (_fullscreen) {
		if (g_engine->hasFeature(Engine::kSupportsArbitraryResolutions)) {
			// If the game supports arbitrary resolutions, use the desktop mode as the game mode
			screenW = _desktopW;
			screenH = _desktopH;
		} else if (_opengl) {
			// If available, draw to a framebuffer and scale it to the desktop resolution
#ifndef AMIGAOS
			// Spawn a 32x32 window off-screen
			SDL_putenv(const_cast<char *>("SDL_VIDEO_WINDOW_POS=9000,9000"));
			SDL_SetVideoMode(32, 32, 0, SDL_OPENGL);
			SDL_putenv(const_cast<char *>("SDL_VIDEO_WINDOW_POS=centered"));
			OpenGL::initExtensions();
			framebufferSupported = OpenGL::isExtensionSupported("GL_EXT_framebuffer_object");
			if (_fullscreen && framebufferSupported) {
				screenW = _desktopW;
				screenH = _desktopH;

				if (_lockAspectRatio) {
					float scale = MIN(_desktopH / float(fbH), _desktopW / float(fbW));
					float scaledW = scale * (fbW / float(_desktopW));
					float scaledH = scale * (fbH / float(_desktopH));
					_gameRect = Math::Rect2d(
						Math::Vector2d(0.5 - (0.5 * scaledW), 0.5 - (0.5 * scaledH)),
						Math::Vector2d(0.5 + (0.5 * scaledW), 0.5 + (0.5 * scaledH))
					);
				}
			}
#endif
		}
	}

	if (_opengl) {
		if (ConfMan.hasKey("antialiasing"))
			_antialiasing = ConfMan.getInt("antialiasing");

		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		setAntialiasing(true);

		sdlflags = SDL_OPENGL;
		bpp = 24;
	} else
#endif
	{
		bpp = 16;
		sdlflags = SDL_SWSURFACE;
	}

	if (_fullscreen && !accel3d && _lockAspectRatio) {
		screenW = _desktopW;
		screenH = _desktopH;
		_gameRect = Math::Rect2d(
			Math::Vector2d((_desktopW - fbW) / 2, (_desktopH - fbH) / 2),
			Math::Vector2d((_desktopW + fbW) / 2, (_desktopH + fbH) / 2)
		);
	}

	if (_fullscreen)
		sdlflags |= SDL_FULLSCREEN;

	_screen = SDL_SetVideoMode(screenW, screenH, bpp, sdlflags);
#ifdef USE_OPENGL
	// If 32-bit with antialiasing failed, try 32-bit without antialiasing
	if (!_screen && _opengl && _antialiasing) {
		warning("Couldn't create 32-bit visual with AA, trying 32-bit without AA");
		setAntialiasing(false);
		_screen = SDL_SetVideoMode(screenW, screenH, bpp, sdlflags);
	}

	// If 32-bit failed, try 16-bit
	if (!_screen && _opengl) {
		warning("Couldn't create 32-bit visual, trying 16-bit");
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 1);
		setAntialiasing(true);
		_screen = SDL_SetVideoMode(screenW, screenH, 0, sdlflags);
	}

	// If 16-bit with antialiasing failed, try 16-bit without antialiasing
	if (!_screen && _opengl && _antialiasing) {
		warning("Couldn't create 16-bit visual with AA, trying 16-bit without AA");
		setAntialiasing(false);
		_screen = SDL_SetVideoMode(screenW, screenH, 0, sdlflags);
	}

	// If 16-bit with alpha failed, try 16-bit without alpha
	if (!_screen && _opengl) {
		warning("Couldn't create 16-bit visual with alpha, trying without alpha");
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
		setAntialiasing(true);
		_screen = SDL_SetVideoMode(screenW, screenH, 0, sdlflags);
	}

	// If 16-bit without alpha and with antialiasing didn't work, try without antialiasing
	if (!_screen && _opengl && _antialiasing) {
		warning("Couldn't create 16-bit visual with AA, trying 16-bit without AA");
		setAntialiasing(false);
		_screen = SDL_SetVideoMode(screenW, screenH, 0, sdlflags);
	}
#endif

	if (!_screen) {
		warning("Error: %s", SDL_GetError());
		g_system->quit();
	}

#ifdef USE_OPENGL
	if (_opengl) {
		int glflag;
		const GLubyte *str;

		// apply atribute again for sure based on SDL docs
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

		str = glGetString(GL_VENDOR);
		debug("INFO: OpenGL Vendor: %s", str);
		str = glGetString(GL_RENDERER);
		debug("INFO: OpenGL Renderer: %s", str);
		str = glGetString(GL_VERSION);
		debug("INFO: OpenGL Version: %s", str);
		SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &glflag);
		debug("INFO: OpenGL Red bits: %d", glflag);
		SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &glflag);
		debug("INFO: OpenGL Green bits: %d", glflag);
		SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &glflag);
		debug("INFO: OpenGL Blue bits: %d", glflag);
		SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &glflag);
		debug("INFO: OpenGL Alpha bits: %d", glflag);
		SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &glflag);
		debug("INFO: OpenGL Z buffer depth bits: %d", glflag);
		SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &glflag);
		debug("INFO: OpenGL Double Buffer: %d", glflag);
		SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &glflag);
		debug("INFO: OpenGL Stencil buffer bits: %d", glflag);

#ifdef USE_GLEW
		debug("INFO: GLEW Version: %s", glewGetString(GLEW_VERSION));
		GLenum err = glewInit();
		if (err != GLEW_OK) {
			warning("Error: %s", glewGetErrorString(err));
			g_system->quit();
		}
#endif

#ifdef USE_OPENGL_SHADERS
		debug("INFO: GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

		const GLfloat vertices[] = {
			0.0, 0.0,
			1.0, 0.0,
			0.0, 1.0,
			1.0, 1.0,
		};

		// Setup the box shader used to render the overlay
		const char* attributes[] = { "position", "texcoord", NULL };
		_boxShader = Graphics::Shader::fromStrings("box", Graphics::BuiltinShaders::boxVertex, Graphics::BuiltinShaders::boxFragment, attributes);
		_boxVerticesVBO = Graphics::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(vertices), vertices);
		_boxShader->enableVertexAttribute("position", _boxVerticesVBO, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), 0);
		_boxShader->enableVertexAttribute("texcoord", _boxVerticesVBO, 2, GL_FLOAT, GL_TRUE, 2 * sizeof(float), 0);
#endif

		OpenGL::initExtensions();

	}
#endif

	_overlayWidth = screenW;
	_overlayHeight = screenH;

#ifdef USE_OPENGL
	if (_opengl) {
		uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0x00001f00;
		gmask = 0x000007e0;
		bmask = 0x000000f8;
		amask = 0x00000000;
#else
		rmask = 0x0000f800;
		gmask = 0x000007e0;
		bmask = 0x0000001f;
		amask = 0x00000000;
#endif
		_overlayscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _overlayWidth, _overlayHeight, 16,
						rmask, gmask, bmask, amask);
		_overlayScreenGLFormat = GL_UNSIGNED_SHORT_5_6_5;
	} else
#endif
	{
		_overlayscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _overlayWidth, _overlayHeight, 16,
					_screen->format->Rmask, _screen->format->Gmask, _screen->format->Bmask, _screen->format->Amask);
	}

	if (!_overlayscreen) {
		warning("Error: %s", SDL_GetError());
		g_system->quit();
	}

	/*_overlayFormat.bytesPerPixel = _overlayscreen->format->BytesPerPixel;

// 	For some reason the values below aren't right, at least on my system
	_overlayFormat.rLoss = _overlayscreen->format->Rloss;
	_overlayFormat.gLoss = _overlayscreen->format->Gloss;
	_overlayFormat.bLoss = _overlayscreen->format->Bloss;
	_overlayFormat.aLoss = _overlayscreen->format->Aloss;

	_overlayFormat.rShift = _overlayscreen->format->Rshift;
	_overlayFormat.gShift = _overlayscreen->format->Gshift;
	_overlayFormat.bShift = _overlayscreen->format->Bshift;
	_overlayFormat.aShift = _overlayscreen->format->Ashift;*/

	_overlayFormat = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);

	_screenChangeCount++;

	SDL_PixelFormat *f = _screen->format;
	_screenFormat = Graphics::PixelFormat(f->BytesPerPixel, 8 - f->Rloss, 8 - f->Gloss, 8 - f->Bloss, 0,
										f->Rshift, f->Gshift, f->Bshift, f->Ashift);

#if defined(USE_OPENGL) && !defined(AMIGAOS)
	if (_opengl && _fullscreen
			&& !g_engine->hasFeature(Engine::kSupportsArbitraryResolutions)
			&& framebufferSupported) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		_frameBuffer = new OpenGL::FrameBuffer(fbW, fbH);
		_frameBuffer->attach();
	}
#endif
	if (_fullscreen && !accel3d) {
		_subScreen = SDL_CreateRGBSurface(SDL_SWSURFACE, fbW, fbH, bpp, _screen->format->Rmask, _screen->format->Gmask, _screen->format->Bmask, _screen->format->Amask);
		return Graphics::PixelBuffer(_screenFormat, (byte *)_subScreen->pixels);
	}
	return Graphics::PixelBuffer(_screenFormat, (byte *)_screen->pixels);
}

#ifdef USE_OPENGL

#define BITMAP_TEXTURE_SIZE 256

void SurfaceSdlGraphicsManager::updateOverlayTextures() {
	if (!_overlayscreen)
		return;

	// remove if already exist
	if (_overlayNumTex > 0) {
		glDeleteTextures(_overlayNumTex, _overlayTexIds);
		delete[] _overlayTexIds;
		_overlayNumTex = 0;
	}

	_overlayNumTex = ((_overlayWidth + (BITMAP_TEXTURE_SIZE - 1)) / BITMAP_TEXTURE_SIZE) *
					((_overlayHeight + (BITMAP_TEXTURE_SIZE - 1)) / BITMAP_TEXTURE_SIZE);
	_overlayTexIds = new GLuint[_overlayNumTex];
	glGenTextures(_overlayNumTex, _overlayTexIds);
	for (int i = 0; i < _overlayNumTex; i++) {
		glBindTexture(GL_TEXTURE_2D, _overlayTexIds[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, BITMAP_TEXTURE_SIZE, BITMAP_TEXTURE_SIZE, 0, GL_RGB, _overlayScreenGLFormat, NULL);
	}

	int bpp = _overlayscreen->format->BytesPerPixel;

	glPixelStorei(GL_UNPACK_ALIGNMENT, bpp);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, _overlayWidth);

	int curTexIdx = 0;
	for (int y = 0; y < _overlayHeight; y += BITMAP_TEXTURE_SIZE) {
		for (int x = 0; x < _overlayWidth; x += BITMAP_TEXTURE_SIZE) {
			int t_width = (x + BITMAP_TEXTURE_SIZE >= _overlayWidth) ? (_overlayWidth - x) : BITMAP_TEXTURE_SIZE;
			int t_height = (y + BITMAP_TEXTURE_SIZE >= _overlayHeight) ? (_overlayHeight - y) : BITMAP_TEXTURE_SIZE;
			glBindTexture(GL_TEXTURE_2D, _overlayTexIds[curTexIdx]);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, t_width, t_height, GL_RGB, _overlayScreenGLFormat,
				(byte *)_overlayscreen->pixels + (y * _overlayscreen->pitch) + (bpp * x));
			curTexIdx++;
		}
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}

void SurfaceSdlGraphicsManager::drawOverlayOpenGL() {
	if (!_overlayscreen)
		return;

	// Save current state
	glPushAttrib(GL_TRANSFORM_BIT | GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_SCISSOR_BIT);

	// prepare view
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, _overlayWidth, _overlayHeight, 0, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_SCISSOR_TEST);

	glScissor(0, 0, _overlayWidth, _overlayHeight);

	int curTexIdx = 0;
	for (int y = 0; y < _overlayHeight; y += BITMAP_TEXTURE_SIZE) {
		for (int x = 0; x < _overlayWidth; x += BITMAP_TEXTURE_SIZE) {
			glBindTexture(GL_TEXTURE_2D, _overlayTexIds[curTexIdx]);
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex2i(x, y);
			glTexCoord2f(1.0f, 0.0f);
			glVertex2i(x + BITMAP_TEXTURE_SIZE, y);
			glTexCoord2f(1.0f, 1.0f);
			glVertex2i(x + BITMAP_TEXTURE_SIZE, y + BITMAP_TEXTURE_SIZE);
			glTexCoord2f(0.0f, 1.0f);
			glVertex2i(x, y + BITMAP_TEXTURE_SIZE);
			glEnd();
			curTexIdx++;
		}
	}

	// Restore previous state
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glPopAttrib();
}

void SurfaceSdlGraphicsManager::drawSideTexturesOpenGL() {
	if (_fullscreen && _lockAspectRatio) {
		const Math::Vector2d nudge(1.0 / float(_screen->w), 0);
		if (_sideTextures[0] != nullptr) {
			float left = _gameRect.getBottomLeft().getX() - (float(_screen->h) / float(_sideTextures[0]->getHeight())) * _sideTextures[0]->getWidth() / float(_screen->w);
			drawTexture(*_sideTextures[0], Math::Vector2d(left, 0.0), _gameRect.getBottomLeft() + nudge, true);
		}

		if (_sideTextures[1] != nullptr) {
			float right = _gameRect.getTopRight().getX() + (float(_screen->h) / float(_sideTextures[1]->getHeight())) * _sideTextures[1]->getWidth() / float(_screen->w);
			drawTexture(*_sideTextures[1], _gameRect.getTopRight() - nudge, Math::Vector2d(right, 1.0), true);
		}
	}
}

void SurfaceSdlGraphicsManager::drawTexture(const OpenGL::Texture &tex, const Math::Vector2d &topLeft, const Math::Vector2d &bottomRight, bool flip) {
#ifndef USE_OPENGL_SHADERS
	// Save current state
	glPushAttrib(GL_TRANSFORM_BIT | GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_SCISSOR_BIT);

	// prepare view
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 1.0, 1.0, 0, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);
	glDepthMask(GL_FALSE);

	float texcropX = tex.getWidth() / float(tex.getTexWidth());
	float texcropY = tex.getHeight() / float(tex.getTexHeight());
	float texTop    = flip ? 0.0 : texcropY;
	float texBottom = flip ? texcropY : 0.0;

	float offsetX = topLeft.getX();
	float offsetY = topLeft.getY();
	float sizeX   = fabsf(topLeft.getX() - bottomRight.getX());
	float sizeY   = fabsf(topLeft.getY() - bottomRight.getY());

	glColor4f(1.0, 1.0, 1.0, 1.0);

	glBindTexture(GL_TEXTURE_2D, tex.getTextureName());
	glBegin(GL_QUADS);
	glTexCoord2f(0, texTop);
	glVertex2f(offsetX, offsetY);
	glTexCoord2f(texcropX, texTop);
	glVertex2f(offsetX + sizeX, offsetY);
	glTexCoord2f(texcropX, texBottom);
	glVertex2f(offsetX + sizeX, offsetY + sizeY);
	glTexCoord2f(0.0, texBottom);
	glVertex2f(offsetX, offsetY + sizeY);
	glEnd();

	// Restore previous state
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glPopAttrib();
#else
	glBindTexture(GL_TEXTURE_2D, tex.getTextureName());

	_boxShader->use();
	float texcropX = tex.getWidth() / float(tex.getTexWidth());
	float texcropY = tex.getHeight() / float(tex.getTexHeight());
	_boxShader->setUniform("texcrop", Math::Vector2d(texcropX, texcropY));
	_boxShader->setUniform("flipY", flip);

	_boxShader->setUniform("offsetXY", topLeft);
	_boxShader->setUniform("sizeWH", Math::Vector2d(fabsf(topLeft.getX() - bottomRight.getX()), fabsf(topLeft.getY() - bottomRight.getY())));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindTexture(GL_TEXTURE_2D, 0);
	_boxShader->unbind();
#endif
}

#ifdef USE_OPENGL_SHADERS
void SurfaceSdlGraphicsManager::drawOverlayOpenGLShaders() {
	if (!_overlayscreen)
		return;

	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_SCISSOR_TEST);

	glScissor(0, 0, _overlayWidth, _overlayHeight);

	_boxShader->use();
	_boxShader->setUniform("sizeWH", Math::Vector2d(BITMAP_TEXTURE_SIZE / (float)_overlayWidth, BITMAP_TEXTURE_SIZE / (float)_overlayHeight));
	_boxShader->setUniform("flipY", true);
	_boxShader->setUniform("texcrop", Math::Vector2d(1.0, 1.0));

	int curTexIdx = 0;
	for (int y = 0; y < _overlayHeight; y += BITMAP_TEXTURE_SIZE) {
		for (int x = 0; x < _overlayWidth; x += BITMAP_TEXTURE_SIZE) {
			_boxShader->setUniform("offsetXY", Math::Vector2d(x / (float)_overlayWidth, y / (float)_overlayHeight));

			glBindTexture(GL_TEXTURE_2D, _overlayTexIds[curTexIdx]);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			curTexIdx++;
		}
	}

	_boxShader->unbind();
}

#endif
#endif

void SurfaceSdlGraphicsManager::drawSideTextures() {
	if (_fullscreen && _lockAspectRatio) {
		if (_sideSurfaces[0]) {
			SDL_Rect dstrect;
			dstrect.x = _gameRect.getTopLeft().getX() - _sideSurfaces[0]->w;
			dstrect.y = _gameRect.getTopLeft().getY();
			dstrect.w = _sideSurfaces[0]->w;
			dstrect.h = _sideSurfaces[0]->h;
			SDL_BlitSurface(_sideSurfaces[0], NULL, _screen, &dstrect);
		}
		if (_sideSurfaces[1]) {
			SDL_Rect dstrect;
			dstrect.x = _gameRect.getTopRight().getX();
			dstrect.y = _gameRect.getTopLeft().getY();
			dstrect.w = _sideSurfaces[1]->w;
			dstrect.h = _sideSurfaces[1]->h;
			SDL_BlitSurface(_sideSurfaces[1], NULL, _screen, &dstrect);
		}
	}
}

void SurfaceSdlGraphicsManager::drawOverlay() {
	if (!_overlayscreen)
		return;

	SDL_LockSurface(_screen);
	SDL_LockSurface(_overlayscreen);
	Graphics::PixelBuffer srcBuf(_overlayFormat, (byte *)_overlayscreen->pixels);
	Graphics::PixelBuffer dstBuf(_screenFormat, (byte *)_screen->pixels);
	int h = _overlayHeight;

	do {
		dstBuf.copyBuffer(0, _overlayWidth, srcBuf);

		srcBuf.shiftBy(_overlayWidth);
		dstBuf.shiftBy(_overlayWidth);
	} while (--h);
	SDL_UnlockSurface(_screen);
	SDL_UnlockSurface(_overlayscreen);
}

void SurfaceSdlGraphicsManager::updateScreen() {
#ifdef USE_OPENGL
	if (_opengl) {
		if (_frameBuffer) {
			_frameBuffer->detach();
			glViewport(0, 0, _screen->w, _screen->h);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			drawSideTexturesOpenGL();
			drawTexture(*_frameBuffer, _gameRect.getTopLeft(), _gameRect.getBottomRight());
		}

		if (_overlayVisible) {
			if (_overlayDirty) {
				updateOverlayTextures();
			}

#ifndef USE_OPENGL_SHADERS
			drawOverlayOpenGL();
#else
			drawOverlayOpenGLShaders();
#endif
		}

		SDL_GL_SwapBuffers();

		if (_frameBuffer) {
			_frameBuffer->attach();
		}
	} else
#endif
	{
		SDL_Rect dstrect;
		dstrect.x = _gameRect.getTopLeft().getX();
		dstrect.y = _gameRect.getTopLeft().getY();
		dstrect.w = _gameRect.getWidth();
		dstrect.h = _gameRect.getHeight();
		SDL_BlitSurface(_subScreen, NULL, _screen, &dstrect);
		if (_overlayVisible) {
			drawOverlay();
		}
		drawSideTextures();
		SDL_Flip(_screen);
	}
}

void SurfaceSdlGraphicsManager::copyRectToScreen(const void *src, int pitch, int x, int y, int w, int h) {
	// ResidualVM: not use it
}

Graphics::Surface *SurfaceSdlGraphicsManager::lockScreen() {
	return NULL; // ResidualVM: not use it
}

void SurfaceSdlGraphicsManager::unlockScreen() {
	// ResidualVM: not use it
}

void SurfaceSdlGraphicsManager::fillScreen(uint32 col) {
	// ResidualVM: not use it
}

int16 SurfaceSdlGraphicsManager::getHeight() {
	// ResidualVM specific
#ifdef USE_OPENGL
	if (_frameBuffer)
		return _frameBuffer->getHeight();
	else
#endif
	if (_subScreen)
		return _subScreen->h;
	else
		return _screen->h;
}

int16 SurfaceSdlGraphicsManager::getWidth() {
	// ResidualVM specific
#ifdef USE_OPENGL
	if (_frameBuffer)
		return _frameBuffer->getWidth();
	else
#endif
	if (_subScreen)
		return _subScreen->w;
	else
		return _screen->w;
}

void SurfaceSdlGraphicsManager::setPalette(const byte *colors, uint start, uint num) {
	// ResidualVM: not use it
}

void SurfaceSdlGraphicsManager::grabPalette(byte *colors, uint start, uint num) {
	// ResidualVM: not use it
}

void SurfaceSdlGraphicsManager::setCursorPalette(const byte *colors, uint start, uint num) {
	// ResidualVM: not use it
}

void SurfaceSdlGraphicsManager::setShakePos(int shake_pos) {
	// ResidualVM: not use it
}

void SurfaceSdlGraphicsManager::setFocusRectangle(const Common::Rect &rect) {
	// ResidualVM: not use it
}

void SurfaceSdlGraphicsManager::clearFocusRectangle() {
	// ResidualVM: not use it
}

#pragma mark -
#pragma mark --- Overlays ---
#pragma mark -

void SurfaceSdlGraphicsManager::showOverlay() {
	if (_overlayVisible)
		return;

	_overlayVisible = true;

	clearOverlay();
}

void SurfaceSdlGraphicsManager::hideOverlay() {
	if (!_overlayVisible)
		return;

	_overlayVisible = false;

	clearOverlay();
}

void SurfaceSdlGraphicsManager::clearOverlay() {
	if (!_overlayscreen)
		return;

	if (!_overlayVisible)
		return;

#ifdef USE_OPENGL
	if (_opengl) {
		SDL_Surface *tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, _overlayWidth, _overlayHeight,
				_overlayscreen->format->BytesPerPixel * 8,
				_overlayscreen->format->Rmask, _overlayscreen->format->Gmask,
				_overlayscreen->format->Bmask, _overlayscreen->format->Amask);

		SDL_LockSurface(tmp);
		SDL_LockSurface(_overlayscreen);

		glReadPixels(0, 0, _overlayWidth, _overlayHeight, GL_RGB, _overlayScreenGLFormat, tmp->pixels);

		// Flip pixels vertically
		byte *src = (byte *)tmp->pixels;
		byte *buf = (byte *)_overlayscreen->pixels + (_overlayHeight - 1) * _overlayscreen->pitch;
		int h = _overlayHeight;
		do {
			memcpy(buf, src, _overlayWidth * _overlayscreen->format->BytesPerPixel);
			src += tmp->pitch;
			buf -= _overlayscreen->pitch;
		} while (--h);

		SDL_UnlockSurface(_overlayscreen);
		SDL_UnlockSurface(tmp);

		SDL_FreeSurface(tmp);
	} else
#endif
	{
		SDL_LockSurface(_screen);
		SDL_LockSurface(_overlayscreen);
		Graphics::PixelBuffer srcBuf(_screenFormat, (byte *)_screen->pixels);
		Graphics::PixelBuffer dstBuf(_overlayFormat, (byte *)_overlayscreen->pixels);
		int h = _overlayHeight;

		do {
			dstBuf.copyBuffer(0, _overlayWidth, srcBuf);

			srcBuf.shiftBy(_overlayWidth);
			dstBuf.shiftBy(_overlayWidth);
		} while (--h);
		SDL_UnlockSurface(_screen);
		SDL_UnlockSurface(_overlayscreen);
	}
	_overlayDirty = true;
}

void SurfaceSdlGraphicsManager::setSideTextures(Graphics::Surface *left, Graphics::Surface *right) {
#ifdef USE_OPENGL
	if (_opengl) {
		delete _sideTextures[0];
		_sideTextures[0] = nullptr;
		delete _sideTextures[1];
		_sideTextures[1] = nullptr;
		if (left) {
			_sideTextures[0] = new OpenGL::Texture(*left);
		}
		if (right) {
			_sideTextures[1] = new OpenGL::Texture(*right);
		}
	} else
#endif
	{
		delete _sideSurfaces[0];
		_sideSurfaces[0] = nullptr;
		delete _sideSurfaces[1];
		_sideSurfaces[1] = nullptr;
		if (left) {
			_sideSurfaces[0] = SDL_CreateRGBSurface(SDL_SWSURFACE, left->w, left->h, 32, 0xff << left->format.rShift, 0xff << left->format.gShift, 0xff << left->format.bShift, 0xff << left->format.aShift);
			memcpy(_sideSurfaces[0]->pixels, left->getPixels(), left->w * left->h * 4);
		}
		if (right) {
			_sideSurfaces[1] = SDL_CreateRGBSurface(SDL_SWSURFACE, right->w, right->h, 32, 0xff << right->format.rShift, 0xff << right->format.gShift, 0xff << right->format.bShift, 0xff << right->format.aShift);
			memcpy(_sideSurfaces[1]->pixels, right->getPixels(), right->w * right->h * 4);
		}
	}
}

void SurfaceSdlGraphicsManager::grabOverlay(void *buf, int pitch) {
	if (_overlayscreen == NULL)
		return;

	if (SDL_LockSurface(_overlayscreen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	byte *src = (byte *)_overlayscreen->pixels;
	byte *dst = (byte *)buf;
	int h = _overlayHeight;
	do {
		memcpy(dst, src, _overlayWidth * _overlayscreen->format->BytesPerPixel);
		src += _overlayscreen->pitch;
		dst += pitch;
	} while (--h);

	SDL_UnlockSurface(_overlayscreen);
}

void SurfaceSdlGraphicsManager::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
	if (_overlayscreen == NULL)
		return;

	const byte *src = (const byte *)buf;

	// Clip the coordinates
	if (x < 0) {
		w += x;
		src -= x * _overlayscreen->format->BytesPerPixel;
		x = 0;
	}

	if (y < 0) {
		h += y;
		src -= y * pitch;
		y = 0;
	}

	if (w > _overlayWidth - x) {
		w = _overlayWidth - x;
	}

	if (h > _overlayHeight - y) {
		h = _overlayHeight - y;
	}

	if (w <= 0 || h <= 0)
		return;

	if (SDL_LockSurface(_overlayscreen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	byte *dst = (byte *)_overlayscreen->pixels + y * _overlayscreen->pitch + x * _overlayscreen->format->BytesPerPixel;
	do {
		memcpy(dst, src, w * _overlayscreen->format->BytesPerPixel);
		dst += _overlayscreen->pitch;
		src += pitch;
	} while (--h);

	SDL_UnlockSurface(_overlayscreen);
}

void SurfaceSdlGraphicsManager::closeOverlay() {
	SDL_FreeSurface(_sideSurfaces[0]);
	SDL_FreeSurface(_sideSurfaces[1]);
	_sideSurfaces[0] = _sideSurfaces[1] = nullptr;
#ifdef USE_OPENGL
	delete _sideTextures[0];
	delete _sideTextures[1];
	_sideTextures[0] = _sideTextures[1] = nullptr;
#endif
	if (_overlayscreen) {
		SDL_FreeSurface(_overlayscreen);
		_overlayscreen = NULL;
#ifdef USE_OPENGL
		if (_opengl) {
			if (_overlayNumTex > 0) {
				glDeleteTextures(_overlayNumTex, _overlayTexIds);
				delete[] _overlayTexIds;
				_overlayNumTex = 0;
			}

			if (_frameBuffer) {
				delete _frameBuffer;
				_frameBuffer = nullptr;
			}

#ifdef USE_OPENGL_SHADERS
			glDeleteBuffers(1, &_boxVerticesVBO);
			_boxVerticesVBO = 0;

			delete _boxShader;
			_boxShader = nullptr;
#endif
		} else if (_subScreen) {
			SDL_FreeSurface(_subScreen);
			_subScreen = nullptr;
		}
#endif
	}
}

#pragma mark -
#pragma mark --- Mouse ---
#pragma mark -

bool SurfaceSdlGraphicsManager::showMouse(bool visible) {
	SDL_ShowCursor(visible);
	return true;
}

// ResidualVM specific method
bool SurfaceSdlGraphicsManager::lockMouse(bool lock) {
	if (lock)
		SDL_WM_GrabInput(SDL_GRAB_ON);
	else
		SDL_WM_GrabInput(SDL_GRAB_OFF);
	return true;
}

void SurfaceSdlGraphicsManager::warpMouse(int x, int y) {
	//ResidualVM specific
#ifdef USE_OPENGL
	if (_frameBuffer) {
		// Scale from game coordinates to screen coordinates
		x = (x * _gameRect.getWidth() * _screen->w) / _frameBuffer->getWidth();
		y = (y * _gameRect.getHeight() * _screen->h) / _frameBuffer->getHeight();

		x += _gameRect.getTopLeft().getX() * _screen->w;
		y += _gameRect.getTopLeft().getY() * _screen->h;
	} else
#endif
	if (_subScreen) {
		// Scale from game coordinates to screen coordinates
		x = (x * _gameRect.getWidth()) / _subScreen->w;
		y = (y * _gameRect.getHeight()) / _subScreen->h;

		x += _gameRect.getTopLeft().getX();
		y += _gameRect.getTopLeft().getY();
	}

	SDL_WarpMouse(x, y);
}

void SurfaceSdlGraphicsManager::setMouseCursor(const void *buf, uint w, uint h, int hotspot_x, int hotspot_y, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format) {
	// ResidualVM: not use it
}

#pragma mark -
#pragma mark --- On Screen Display ---
#pragma mark -

#ifdef USE_OSD
void SurfaceSdlGraphicsManager::displayMessageOnOSD(const char *msg) {
	// ResidualVM: not use it
}
#endif


#ifdef USE_OPENGL
void SurfaceSdlGraphicsManager::setAntialiasing(bool enable) {
	// Antialiasing works without setting MULTISAMPLEBUFFERS, but as SDL's official
	// tests set both values, this seems to be the standard way to do it. It could
	// just be that in current OpenGL implementations setting SDL_GL_MULTISAMPLESAMPLES
	// implicitly sets SDL_GL_MULTISAMPLEBUFFERS as well.
	if (_antialiasing && enable) {
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, _antialiasing);
	} else {
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
	}
}
#endif

bool SurfaceSdlGraphicsManager::notifyEvent(const Common::Event &event) {
	//ResidualVM specific:
	switch ((int)event.type) {
	case Common::EVENT_KEYDOWN:
		break;
	case Common::EVENT_KEYUP:
		break;
	default:
		break;
	}

	return false;
}

void SurfaceSdlGraphicsManager::notifyVideoExpose() {
	_forceFull = true;
	//ResidualVM specific:
	updateScreen();
}

void SurfaceSdlGraphicsManager::transformMouseCoordinates(Common::Point &point) {
	bool frames = _subScreen
#ifdef USE_OPENGL
		|| _frameBuffer
#endif
	;
	if (_overlayVisible || !frames)
		return;

#ifdef USE_OPENGL
	if (_frameBuffer) {
		// Scale from screen coordinates to game coordinates
		point.x -= _gameRect.getTopLeft().getX() * _screen->w;
		point.y -= _gameRect.getTopLeft().getY() * _screen->h;

		point.x = (point.x * _frameBuffer->getWidth())  / (_gameRect.getWidth() * _screen->w);
		point.y = (point.y * _frameBuffer->getHeight()) / (_gameRect.getHeight() * _screen->h);

		// Make sure we only supply valid coordinates.
		point.x = CLIP<int16>(point.x, 0, _frameBuffer->getWidth() - 1);
		point.y = CLIP<int16>(point.y, 0, _frameBuffer->getHeight() - 1);
	} else
#endif
	{
		// Scale from screen coordinates to game coordinates
		point.x -= _gameRect.getTopLeft().getX();
		point.y -= _gameRect.getTopLeft().getY();

		point.x = (point.x * _subScreen->w) / _gameRect.getWidth();
		point.y = (point.y * _subScreen->h) / _gameRect.getHeight();

		// Make sure we only supply valid coordinates.
		point.x = CLIP<int16>(point.x, 0, _subScreen->w - 1);
		point.y = CLIP<int16>(point.y, 0, _subScreen->h - 1);
	}
}

void SurfaceSdlGraphicsManager::notifyMousePos(Common::Point mouse) {
	transformMouseCoordinates(mouse);
	// ResidualVM: not use that:
	//setMousePos(mouse.x, mouse.y);
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
void SurfaceSdlGraphicsManager::deinitializeRenderer() {
	SDL_DestroyTexture(_screenTexture);
	_screenTexture = nullptr;

	SDL_DestroyRenderer(_renderer);
	_renderer = nullptr;

	_window->destroyWindow();
}
#endif // SDL_VERSION_ATLEAST(2, 0, 0)

#endif
