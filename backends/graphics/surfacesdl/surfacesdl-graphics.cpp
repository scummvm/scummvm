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
#include "graphics/opengl/context.h"
#endif

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{0, 0, 0}
};

SurfaceSdlGraphicsManager::SurfaceSdlGraphicsManager(SdlEventSource *sdlEventSource, SdlWindow *window)
	:
	SdlGraphicsManager(sdlEventSource, window),
#if SDL_VERSION_ATLEAST(2, 0, 0)
	_renderer(nullptr), _screenTexture(nullptr),
	_glContext(nullptr),
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
	, _opengl(false)
	, _frameBuffer(nullptr)
	, _surfaceRenderer(nullptr)
#endif
	{
		ConfMan.registerDefault("fullscreen_res", "desktop");
		ConfMan.registerDefault("aspect_ratio", true);
		ConfMan.registerDefault("antialiasing", 0);

		detectDesktopResolution();

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

void SurfaceSdlGraphicsManager::detectDesktopResolution() {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_DisplayMode di;
	if (SDL_GetCurrentDisplayMode(0, &di) != 0) {
		warning("Error: %s", SDL_GetError());
		g_system->quit();
	}
	_desktopW = di.w;
	_desktopH = di.h;
#else
	const SDL_VideoInfo *vi = SDL_GetVideoInfo();
	_desktopW = vi->current_w;
	_desktopH = vi->current_h;
#endif
}

void SurfaceSdlGraphicsManager::setupScreen(uint gameWidth, uint gameHeight, bool fullscreen, bool accel3d) {
	closeOverlay();

#ifdef USE_OPENGL
	_opengl = accel3d;
	_antialiasing = ConfMan.getInt("antialiasing");
#endif
	_fullscreen = fullscreen;
	_lockAspectRatio = ConfMan.getBool("aspect_ratio");

	bool engineSupportsArbitraryResolutions = g_engine && g_engine->hasFeature(Engine::kSupportsArbitraryResolutions);

	// Detecting if OpenGL framebuffers are available relies on spawning an offscreen window
	// thus it is only done when framebuffers may be used.
	bool framebufferSupported = accel3d && fullscreen && detectFramebufferSupport();

	// Select how the game screen is going to be drawn
	GameRenderTarget gameRenderTarget = selectGameRenderTarget(_fullscreen, accel3d, engineSupportsArbitraryResolutions,
	                                                           framebufferSupported, _lockAspectRatio);

	// Choose the effective window size or fullscreen mode
	uint effectiveWidth;
	uint effectiveHeight;
	if (_fullscreen && canUsePreferredResolution(gameRenderTarget, engineSupportsArbitraryResolutions)) {
		Common::Rect fullscreenResolution = getPreferredFullscreenResolution();
		effectiveWidth = fullscreenResolution.width();
		effectiveHeight = fullscreenResolution.height();
	} else {
		effectiveWidth = gameWidth;
		effectiveHeight = gameHeight;
	}

	// Compute the rectangle where to draw the game inside the effective screen
	_gameRect = computeGameRect(gameRenderTarget, gameWidth, gameHeight, effectiveWidth, effectiveHeight);

#ifdef USE_OPENGL
	if (_opengl) {
		if (!createScreenOpenGL(effectiveWidth, effectiveHeight, gameRenderTarget)) {
			warning("Error: %s", SDL_GetError());
			g_system->quit();
		}

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
#endif

		initializeOpenGLContext();
		_surfaceRenderer = OpenGL::createBestSurfaceRenderer();
	} else
#endif
	{
		uint32 sdlflags = SDL_SWSURFACE;
		if (_fullscreen)
			sdlflags |= SDL_FULLSCREEN;

		_screen = SDL_SetVideoMode(effectiveWidth, effectiveHeight, 16, sdlflags);
		if (!_screen) {
			warning("Error: %s", SDL_GetError());
			g_system->quit();
		}
	}

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
		_overlayscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, effectiveWidth, effectiveHeight, 16,
						rmask, gmask, bmask, amask);
		_overlayFormat = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
	} else
#endif
	{
		SDL_PixelFormat *f = _screen->format;
		_overlayscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, effectiveWidth, effectiveHeight, f->BitsPerPixel,
		                                      f->Rmask, f->Gmask, f->Bmask, f->Amask);

		_overlayFormat = Graphics::PixelFormat(f->BytesPerPixel, 8 - f->Rloss, 8 - f->Gloss, 8 - f->Bloss, 0,
		                                      f->Rshift, f->Gshift, f->Bshift, f->Ashift);
	}

	if (!_overlayscreen) {
		warning("Error: %s", SDL_GetError());
		g_system->quit();
	}

	_overlayWidth = effectiveWidth;
	_overlayHeight = effectiveHeight;
	_screenFormat = _overlayFormat;

	_screenChangeCount++;

#if defined(USE_OPENGL) && !defined(AMIGAOS)
	if (gameRenderTarget == kFramebuffer) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		_frameBuffer = createFramebuffer(gameWidth, gameHeight);
		_frameBuffer->attach();
	}
#endif
	if (gameRenderTarget == kSubScreen) {
		SDL_PixelFormat *f = _screen->format;
		_subScreen = SDL_CreateRGBSurface(SDL_SWSURFACE, gameWidth, gameHeight, f->BitsPerPixel, f->Rmask, f->Gmask, f->Bmask, f->Amask);
	}
}

SurfaceSdlGraphicsManager::GameRenderTarget SurfaceSdlGraphicsManager::selectGameRenderTarget(bool fullscreen,
                                                                                              bool accel3d,
                                                                                              bool engineSupportsArbitraryResolutions,
                                                                                              bool framebufferSupported,
                                                                                              bool lockAspectRatio) {
	if (!fullscreen) {
		return kScreen;
	}

	if (!accel3d && lockAspectRatio) {
		return kSubScreen;
	}

	if (!engineSupportsArbitraryResolutions && framebufferSupported) {
		return kFramebuffer;
	}

	return kScreen;
}

Math::Rect2d SurfaceSdlGraphicsManager::computeGameRect(GameRenderTarget gameRenderTarget, uint gameWidth, uint gameHeight,
                                                        uint effectiveWidth, uint effectiveHeight) {
	switch (gameRenderTarget) {
		case kScreen:
			// The game occupies the whole screen
			return Math::Rect2d(Math::Vector2d(0, 0), Math::Vector2d(1, 1));
		case kSubScreen:
			// The game is centered on the screen
			return Math::Rect2d(
					Math::Vector2d((effectiveWidth - gameWidth) / 2, (effectiveHeight - gameHeight) / 2),
					Math::Vector2d((effectiveWidth + gameWidth) / 2, (effectiveHeight + gameHeight) / 2)
			);
		case kFramebuffer:
			if (_lockAspectRatio) {
				// The game is scaled to fit the screen, keeping the same aspect ratio
				float scale = MIN(effectiveHeight / float(gameHeight), effectiveWidth / float(gameWidth));
				float scaledW = scale * (gameWidth / float(effectiveWidth));
				float scaledH = scale * (gameHeight / float(effectiveHeight));
				return Math::Rect2d(
						Math::Vector2d(0.5 - (0.5 * scaledW), 0.5 - (0.5 * scaledH)),
						Math::Vector2d(0.5 + (0.5 * scaledW), 0.5 + (0.5 * scaledH))
				);
			} else {
				// The game occupies the whole screen
				return Math::Rect2d(Math::Vector2d(0, 0), Math::Vector2d(1, 1));
			}
		default:
			error("Unhandled game render target '%d'", gameRenderTarget);
	}
}

bool SurfaceSdlGraphicsManager::detectFramebufferSupport() {
	bool framebufferSupported = false;
#if defined(USE_GLES2)
	// Framebuffers are always available with GLES2
	framebufferSupported = true;
#elif defined(USE_OPENGL) && !defined(AMIGAOS)
	// Spawn a 32x32 window off-screen with a GL context to test if framebuffers are supported
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_Window *window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 32, 32, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
	if (window) {
		SDL_GLContext glContext = SDL_GL_CreateContext(window);
		if (glContext) {
			initializeOpenGLContext();
			framebufferSupported = OpenGLContext.framebufferObjectSupported;
			SDL_GL_DeleteContext(glContext);
		}
		SDL_DestroyWindow(window);
	}
#else
	SDL_putenv(const_cast<char *>("SDL_VIDEO_WINDOW_POS=9000,9000"));
	SDL_SetVideoMode(32, 32, 0, SDL_OPENGL);
	SDL_putenv(const_cast<char *>("SDL_VIDEO_WINDOW_POS=center"));
	initializeOpenGLContext();
	framebufferSupported = OpenGLContext.framebufferObjectSupported;
#endif
#endif
	return framebufferSupported;
}

bool SurfaceSdlGraphicsManager::canUsePreferredResolution(GameRenderTarget gameRenderTarget,
                                                          bool engineSupportsArbitraryResolutions) {
	switch (gameRenderTarget) {
		case kScreen:
			// If the game supports arbitrary resolutions, use the preferred mode as the game mode
			return engineSupportsArbitraryResolutions;
		case kSubScreen:
		case kFramebuffer:
			return true;
		default:
			error("Unhandled game render target '%d'", gameRenderTarget);
	}
}

Common::Rect SurfaceSdlGraphicsManager::getPreferredFullscreenResolution() {
	// Default to the desktop resolution ...
	uint preferredWidth = _desktopW;
	uint preferredHeight = _desktopH;

	// ... unless the user has set a resolution in the configuration file
	const Common::String &fsres = ConfMan.get("fullscreen_res");
	if (fsres != "desktop") {
		uint newW, newH;
		int converted = sscanf(fsres.c_str(), "%ux%u", &newW, &newH);
		if (converted == 2) {
			preferredWidth = newW;
			preferredHeight = newH;
		} else {
			warning("Could not parse 'fullscreen_res' option: expected WWWxHHH, got %s", fsres.c_str());
		}
	}

	return Common::Rect(preferredWidth, preferredHeight);
}

Graphics::PixelBuffer SurfaceSdlGraphicsManager::getScreenPixelBuffer() {
#ifdef USE_OPENGL
	if (_opengl) {
		error("Direct screen buffer access is not allowed when using OpenGL");
	}
#endif

	if (_subScreen) {
		return Graphics::PixelBuffer(_screenFormat, (byte *)_subScreen->pixels);
	}

	return Graphics::PixelBuffer(_screenFormat, (byte *)_screen->pixels);
}

#ifdef USE_OPENGL

void SurfaceSdlGraphicsManager::initializeOpenGLContext() const {
	OpenGL::ContextType type;

#ifdef USE_GLES2
	type = OpenGL::kContextGLES2;
#else
	type = OpenGL::kContextGL;
#endif

	OpenGLContext.initialize(type);
}

SurfaceSdlGraphicsManager::OpenGLPixelFormat::OpenGLPixelFormat(uint screenBytesPerPixel, uint red, uint blue, uint green, uint alpha, int samples) :
		bytesPerPixel(screenBytesPerPixel),
		redSize(red),
		blueSize(blue),
		greenSize(green),
		alphaSize(alpha),
		multisampleSamples(samples) {

}

bool SurfaceSdlGraphicsManager::createScreenOpenGL(uint effectiveWidth, uint effectiveHeight, GameRenderTarget gameRenderTarget) {
	// Build a list of OpenGL pixel formats usable by ResidualVM
	Common::Array<OpenGLPixelFormat> pixelFormats;
	if (_antialiasing > 0 && gameRenderTarget == kScreen) {
		// Don't enable screen level multisampling when rendering to a framebuffer
		pixelFormats.push_back(OpenGLPixelFormat(32, 8, 8, 8, 8, _antialiasing));
		pixelFormats.push_back(OpenGLPixelFormat(16, 5, 5, 5, 1, _antialiasing));
		pixelFormats.push_back(OpenGLPixelFormat(16, 5, 6, 5, 0, _antialiasing));
	}
	pixelFormats.push_back(OpenGLPixelFormat(32, 8, 8, 8, 8, 0));
	pixelFormats.push_back(OpenGLPixelFormat(16, 5, 5, 5, 1, 0));
	pixelFormats.push_back(OpenGLPixelFormat(16, 5, 6, 5, 0, 0));

	// Unfortunatly, SDL does not provide a list of valid pixel formats
	// for the current OpenGL implementation and hardware.
	// SDL may not be able to create a screen with the preferred pixel format.
	// Try all the pixel formats in the list until SDL returns a valid screen.
	Common::Array<OpenGLPixelFormat>::const_iterator it = pixelFormats.begin();
	for (; it != pixelFormats.end(); it++) {
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, it->redSize);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, it->greenSize);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, it->blueSize);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, it->alphaSize);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, it->multisampleSamples > 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, it->multisampleSamples);
#if SDL_VERSION_ATLEAST(2, 0, 0)
#ifdef USE_GLES2
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
#endif
#endif

#if SDL_VERSION_ATLEAST(2, 0, 0)
		uint32 sdlflags = SDL_WINDOW_OPENGL;
		if (_fullscreen)
			sdlflags |= SDL_WINDOW_FULLSCREEN;

		if (_window->createWindow(effectiveWidth, effectiveHeight, sdlflags)) {
			_glContext = SDL_GL_CreateContext(_window->getSDLWindow());
			if (_glContext) {
				break;
			}
		}

		_window->destroyWindow();
#else
		uint32 sdlflags = SDL_OPENGL;
		if (_fullscreen)
			sdlflags |= SDL_FULLSCREEN;

		SDL_Surface *screen = SDL_SetVideoMode(effectiveWidth, effectiveHeight, it->bytesPerPixel, sdlflags);
		if (screen) {
			break;
		}
#endif
	}

	// Display a warning if the effective pixel format is not the preferred one
	if (it != pixelFormats.begin() && it != pixelFormats.end()) {
		bool wantsAA = pixelFormats.front().multisampleSamples > 0;
		bool gotAA = it->multisampleSamples > 0;

		warning("Couldn't create a %d-bit visual%s, using to %d-bit%s instead",
		        pixelFormats.front().bytesPerPixel,
		        wantsAA && !gotAA ? " with AA" : "",
		        it->bytesPerPixel,
		        wantsAA && !gotAA ? " without AA" : "");
	}

	return it != pixelFormats.end();
}

#define BITMAP_TEXTURE_SIZE 256

void SurfaceSdlGraphicsManager::updateOverlayTextures() {
	if (!_overlayscreen)
		return;

	// remove if already exist
	for (uint i = 0; i < _overlayTextures.size(); i++) {
		delete _overlayTextures[i];
	}
	_overlayTextures.clear();

	Graphics::Surface overlaySurface;
	overlaySurface.w = _overlayscreen->w;
	overlaySurface.h = _overlayscreen->h;
	overlaySurface.pitch = _overlayscreen->pitch;
	overlaySurface.format = getOverlayFormat();
	overlaySurface.setPixels(_overlayscreen->pixels);

	for (int y = 0; y < _overlayHeight; y += BITMAP_TEXTURE_SIZE) {
		for (int x = 0; x < _overlayWidth; x += BITMAP_TEXTURE_SIZE) {
			int t_width = (x + BITMAP_TEXTURE_SIZE >= _overlayWidth) ? (_overlayWidth - x) : BITMAP_TEXTURE_SIZE;
			int t_height = (y + BITMAP_TEXTURE_SIZE >= _overlayHeight) ? (_overlayHeight - y) : BITMAP_TEXTURE_SIZE;

			Common::Rect textureArea = Common::Rect(t_width, t_height);
			textureArea.translate(x, y);

			const Graphics::Surface subSurface = overlaySurface.getSubArea(textureArea);
			_overlayTextures.push_back(new OpenGL::Texture(subSurface));
		}
	}
}

void SurfaceSdlGraphicsManager::drawOverlayOpenGL() {
	if (!_overlayscreen)
		return;

	glViewport(0, 0, _overlayWidth, _overlayHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	_surfaceRenderer->prepareState();

	int curTexIdx = 0;
	for (int y = 0; y < _overlayHeight; y += BITMAP_TEXTURE_SIZE) {
		for (int x = 0; x < _overlayWidth; x += BITMAP_TEXTURE_SIZE) {
			OpenGL::Texture *texture = _overlayTextures[curTexIdx];

			Common::Rect textureArea = Common::Rect(texture->getWidth(), texture->getHeight());
			textureArea.translate(x, y);

			Math::Vector2d topLeft = Math::Vector2d(textureArea.left / (float)_overlayWidth, textureArea.top / (float)_overlayHeight);
			Math::Vector2d bottomRight = Math::Vector2d(textureArea.right / (float)_overlayWidth, textureArea.bottom / (float)_overlayHeight);

			_surfaceRenderer->render(texture, Math::Rect2d(topLeft, bottomRight), true);

			curTexIdx++;
		}
	}

	_surfaceRenderer->restorePreviousState();
}

void SurfaceSdlGraphicsManager::drawSideTexturesOpenGL() {
	if (_fullscreen && _lockAspectRatio) {
		const Math::Vector2d nudge(1.0 / float(_overlayWidth), 0);
		if (_sideTextures[0] != nullptr) {
			float left = _gameRect.getBottomLeft().getX() - (float(_overlayHeight) / float(_sideTextures[0]->getHeight())) * _sideTextures[0]->getWidth() / float(_overlayWidth);
			Math::Rect2d leftRect(Math::Vector2d(left, 0.0), _gameRect.getBottomLeft() + nudge);
			_surfaceRenderer->render(_sideTextures[0], leftRect, true);
		}

		if (_sideTextures[1] != nullptr) {
			float right = _gameRect.getTopRight().getX() + (float(_overlayHeight) / float(_sideTextures[1]->getHeight())) * _sideTextures[1]->getWidth() / float(_overlayWidth);
			Math::Rect2d rightRect(_gameRect.getTopRight() - nudge, Math::Vector2d(right, 1.0));
			_surfaceRenderer->render(_sideTextures[1], rightRect, true);
		}
	}
}

#ifndef AMIGAOS
OpenGL::FrameBuffer *SurfaceSdlGraphicsManager::createFramebuffer(uint width, uint height) {
#if !defined(USE_GLES2)
	if (_antialiasing && OpenGLContext.framebufferObjectMultisampleSupported) {
		return new OpenGL::MultiSampleFrameBuffer(width, height, _antialiasing);
	} else
#endif
	{
		return new OpenGL::FrameBuffer(width, height);
	}
}
#endif // AMIGAOS

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
			glViewport(0, 0, _overlayWidth, _overlayHeight);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			_surfaceRenderer->prepareState();
			drawSideTexturesOpenGL();
			_surfaceRenderer->render(_frameBuffer, _gameRect);
			_surfaceRenderer->restorePreviousState();
		}

		if (_overlayVisible) {
			if (_overlayDirty) {
				updateOverlayTextures();
			}

			drawOverlayOpenGL();
		}

#if SDL_VERSION_ATLEAST(2, 0, 0)
		SDL_GL_SwapWindow(_window->getSDLWindow());
#else
		SDL_GL_SwapBuffers();
#endif

		if (_frameBuffer) {
			_frameBuffer->attach();
		}
	} else
#endif
	{
		if (_subScreen) {
			SDL_Rect dstrect;
			dstrect.x = _gameRect.getTopLeft().getX();
			dstrect.y = _gameRect.getTopLeft().getY();
			dstrect.w = _gameRect.getWidth();
			dstrect.h = _gameRect.getHeight();
			SDL_BlitSurface(_subScreen, NULL, _screen, &dstrect);
		}
		if (_overlayVisible) {
			drawOverlay();
		}
		drawSideTextures();

#if SDL_VERSION_ATLEAST(2, 0, 0)
		SDL_UpdateTexture(_screenTexture, nullptr, _screen->pixels, _screen->pitch);

		SDL_RenderClear(_renderer);
		SDL_RenderCopy(_renderer, _screenTexture, nullptr, nullptr);
		SDL_RenderPresent(_renderer);
#else
		SDL_Flip(_screen);
#endif
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
		return _overlayHeight;
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
		return _overlayWidth;
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

		glReadPixels(0, 0, _overlayWidth, _overlayHeight, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, tmp->pixels);

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
		_overlayscreen = nullptr;
	}
	if (_subScreen) {
		SDL_FreeSurface(_subScreen);
		_subScreen = nullptr;
	}
	if (_screen) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
		SDL_FreeSurface(_screen);
#endif
		_screen = nullptr;
	}

#ifdef USE_OPENGL
	delete _surfaceRenderer;
	_surfaceRenderer = nullptr;

	for (uint i = 0; i < _overlayTextures.size(); i++) {
		delete _overlayTextures[i];
	}
	_overlayTextures.clear();

	delete _frameBuffer;
	_frameBuffer = nullptr;

	OpenGL::Context::destroy();
#endif

#if SDL_VERSION_ATLEAST(2, 0, 0)
	deinitializeRenderer();
#endif
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
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (lock)
		SDL_SetRelativeMouseMode(SDL_TRUE);
	else
		SDL_SetRelativeMouseMode(SDL_FALSE);
#else
	if (lock)
		SDL_WM_GrabInput(SDL_GRAB_ON);
	else
		SDL_WM_GrabInput(SDL_GRAB_OFF);
#endif
	return true;
}

void SurfaceSdlGraphicsManager::warpMouse(int x, int y) {
	//ResidualVM specific
#ifdef USE_OPENGL
	if (_frameBuffer) {
		// Scale from game coordinates to screen coordinates
		x = (x * _gameRect.getWidth() * _overlayWidth) / _frameBuffer->getWidth();
		y = (y * _gameRect.getHeight() * _overlayHeight) / _frameBuffer->getHeight();

		x += _gameRect.getTopLeft().getX() * _overlayWidth;
		y += _gameRect.getTopLeft().getY() * _overlayHeight;
	} else
#endif
	if (_subScreen) {
		// Scale from game coordinates to screen coordinates
		x = (x * _gameRect.getWidth()) / _subScreen->w;
		y = (y * _gameRect.getHeight()) / _subScreen->h;

		x += _gameRect.getTopLeft().getX();
		y += _gameRect.getTopLeft().getY();
	}

	_window->warpMouseInWindow(x, y);
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
		point.x -= _gameRect.getTopLeft().getX() * _overlayWidth;
		point.y -= _gameRect.getTopLeft().getY() * _overlayHeight;

		point.x = (point.x * _frameBuffer->getWidth())  / (_gameRect.getWidth() * _overlayWidth);
		point.y = (point.y * _frameBuffer->getHeight()) / (_gameRect.getHeight() * _overlayHeight);

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
#ifdef USE_OPENGL
	SDL_GL_DeleteContext(_glContext);
	_glContext = nullptr;
#endif

	SDL_DestroyTexture(_screenTexture);
	_screenTexture = nullptr;

	SDL_DestroyRenderer(_renderer);
	_renderer = nullptr;

	_window->destroyWindow();
}

SDL_Surface *SurfaceSdlGraphicsManager::SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags) {
	deinitializeRenderer();

	uint32 createWindowFlags = 0;
#ifdef USE_SDL_RESIZABLE_WINDOW
	createWindowFlags |= SDL_WINDOW_RESIZABLE;
#endif
	if ((flags & SDL_FULLSCREEN) != 0) {
		createWindowFlags |= SDL_WINDOW_FULLSCREEN;
	}

	if (!_window->createWindow(width, height, createWindowFlags)) {
		return nullptr;
	}

	_renderer = SDL_CreateRenderer(_window->getSDLWindow(), -1, 0);
	if (!_renderer) {
		deinitializeRenderer();
		return nullptr;
	}

	_screenTexture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, width, height);
	if (!_screenTexture) {
		deinitializeRenderer();
		return nullptr;
	}

	SDL_Surface *screen = SDL_CreateRGBSurface(0, width, height, 16, 0xF800, 0x7E0, 0x1F, 0);
	if (!screen) {
		deinitializeRenderer();
		return nullptr;
	} else {
		return screen;
	}
}
#endif // SDL_VERSION_ATLEAST(2, 0, 0)

#endif
