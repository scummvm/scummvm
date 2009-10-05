/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
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
 * $URL$
 * $Id$
 *
 */

#include "backends/platform/sdl/sdl.h"
#include "common/mutex.h"
#include "common/util.h"
#ifdef USE_RGB_COLOR
#include "common/list.h"
#endif
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/scaler.h"
#include "graphics/surface.h"



void OSystem_SDL::launcherInitSize(uint w, uint h) {
	closeOverlay();
	setupScreen(w, h, false, false);
}

byte *OSystem_SDL::setupScreen(int screenW, int screenH, bool fullscreen, bool accel3d) {
	uint32 sdlflags;
	int bpp;

	closeOverlay();

#ifdef USE_OPENGL
	_opengl = accel3d;
#endif
	_fullscreen = fullscreen;

#ifdef USE_OPENGL
	if (_opengl) {
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

		sdlflags = SDL_OPENGL;
		bpp = 24;
	} else
#endif
	{
		bpp = 16;
		sdlflags = SDL_HWSURFACE;
	}

	if (_fullscreen)
		sdlflags |= SDL_FULLSCREEN;

	_screen = SDL_SetVideoMode(screenW, screenH, bpp, sdlflags);
	if (!_screen)
		error("Could not initialize video");

#ifdef USE_OPENGL
	if (_opengl) {
		int glflag;

		// apply atribute again for sure based on SDL docs
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

		SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &glflag);
		warning("INFO: GL RED bits: %d", glflag);
		SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &glflag);
		warning("INFO: GL GREEN bits: %d", glflag);
		SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &glflag);
		warning("INFO: GL BLUE bits: %d", glflag);
		SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &glflag);
		warning("INFO: GL APLHA bits: %d", glflag);
		SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &glflag);
		warning("INFO: GL Z buffer depth bits: %d", glflag);
		SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &glflag);
		warning("INFO: GL Double Buffer: %d", glflag);
		SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &glflag);
		warning("INFO: GL Stencil buffer bits: %d", glflag);
	}
#endif

	_overlayWidth = screenW;
	_overlayHeight = screenH;

	Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0x00001f00;
	gmask = 0x000007e0;
	bmask = 0x000000f8;
	amask = 0x00000000;
#else
	rmask = 0x0000001f;
	gmask = 0x000007e0;
	bmask = 0x0000f800;
	amask = 0x00000000;
#endif
	_overlayscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _overlayWidth, _overlayHeight, 16,
						rmask, gmask, bmask, amask);

	if (!_overlayscreen)
		error("allocating _overlayscreen failed");

	_overlayFormat.bytesPerPixel = _overlayscreen->format->BytesPerPixel;

	_overlayFormat.rLoss = _overlayscreen->format->Rloss;
	_overlayFormat.gLoss = _overlayscreen->format->Gloss;
	_overlayFormat.bLoss = _overlayscreen->format->Bloss;
	_overlayFormat.aLoss = _overlayscreen->format->Aloss;

	_overlayFormat.rShift = _overlayscreen->format->Rshift;
	_overlayFormat.gShift = _overlayscreen->format->Gshift;
	_overlayFormat.bShift = _overlayscreen->format->Bshift;
	_overlayFormat.aShift = _overlayscreen->format->Ashift;

	return (byte *)_screen->pixels;
}

#define BITMAP_TEXTURE_SIZE 256

void OSystem_SDL::updateScreen() {
#ifdef USE_OPENGL
	if (_opengl) {
		if (_overlayVisible) {
			if (_overlayDirty) {
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
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, BITMAP_TEXTURE_SIZE, BITMAP_TEXTURE_SIZE, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
				}

				glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
				glPixelStorei(GL_UNPACK_ROW_LENGTH, _overlayWidth);

				int curTexIdx = 0;
				for (int y = 0; y < _overlayHeight; y += BITMAP_TEXTURE_SIZE) {
					for (int x = 0; x < _overlayWidth; x += BITMAP_TEXTURE_SIZE) {
						int t_width = (x + BITMAP_TEXTURE_SIZE >= _overlayWidth) ? (_overlayWidth - x) : BITMAP_TEXTURE_SIZE;
						int t_height = (y + BITMAP_TEXTURE_SIZE >= _overlayHeight) ? (_overlayHeight - y) : BITMAP_TEXTURE_SIZE;
						glBindTexture(GL_TEXTURE_2D, _overlayTexIds[curTexIdx]);
						glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, t_width, t_height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (byte *)_overlayscreen->pixels + (y * 2 * _overlayWidth) + (2 * x));
						curTexIdx++;
					}
				}
				glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
				glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			}

			// prepare view
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, _overlayWidth, _overlayHeight, 0, 0, 1);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glMatrixMode(GL_TEXTURE);
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
					glTexCoord2f(1.0, 0.0);
					glVertex2i(x + BITMAP_TEXTURE_SIZE, y);
					glTexCoord2f(1.0, 1.0);
					glVertex2i(x + BITMAP_TEXTURE_SIZE, y + BITMAP_TEXTURE_SIZE);
					glTexCoord2f(0.0, 1.0);
					glVertex2i(x, y + BITMAP_TEXTURE_SIZE);
					glEnd();
					curTexIdx++;
				}
			}

			glDisable(GL_SCISSOR_TEST);
			glDisable(GL_TEXTURE_2D);
			glDepthMask(GL_TRUE);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_LIGHTING);
		}
		SDL_GL_SwapBuffers();
	} else
#endif
	{
		if (_overlayVisible) {
			SDL_LockSurface(_screen);
			SDL_LockSurface(_overlayscreen);
			byte *src = (byte *)_overlayscreen->pixels;
			byte *buf = (byte *)_screen->pixels;
			int h = _overlayHeight;
			do {
				memcpy(buf, src, _overlayWidth * _overlayscreen->format->BytesPerPixel);
				src += _overlayscreen->pitch;
				buf += _screen->pitch;
			} while (--h);
			SDL_UnlockSurface(_screen);
			SDL_UnlockSurface(_overlayscreen);
		}
		SDL_Flip(_screen);
	}
}

int16 OSystem_SDL::getHeight() {
	return _screen->h;
}

int16 OSystem_SDL::getWidth() {
	return _screen->w;
}


#pragma mark -
#pragma mark --- Overlays ---
#pragma mark -

void OSystem_SDL::showOverlay() {
	if (_overlayVisible)
		return;

	_overlayVisible = true;

	clearOverlay();
}

void OSystem_SDL::hideOverlay() {
	if (!_overlayVisible)
		return;

	_overlayVisible = false;

	clearOverlay();
}

void OSystem_SDL::clearOverlay() {
	if (!_overlayVisible)
		return;

#ifdef USE_OPENGL
	if (_opengl) {
		SDL_LockSurface(_overlayscreen);
		glReadPixels(0, 0, _overlayWidth, _overlayWidth, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, _overlayscreen->pixels);
		SDL_UnlockSurface(_overlayscreen);
	} else
#endif
	{
		SDL_LockSurface(_screen);
		SDL_LockSurface(_overlayscreen);
		byte *src = (byte *)_screen->pixels;
		byte *buf = (byte *)_overlayscreen->pixels;
		int h = _overlayHeight;
		do {
			memcpy(buf, src, _overlayWidth * _overlayscreen->format->BytesPerPixel);
			src += _screen->pitch;
			buf += _overlayscreen->pitch;
		} while (--h);
		SDL_UnlockSurface(_screen);
		SDL_UnlockSurface(_overlayscreen);
	}
	_overlayDirty = true;
}

void OSystem_SDL::grabOverlay(OverlayColor *buf, int pitch) {
	if (_overlayscreen == NULL)
		return;

	if (SDL_LockSurface(_overlayscreen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	byte *src = (byte *)_overlayscreen->pixels;
	int h = _overlayHeight;
	do {
		memcpy(buf, src, _overlayWidth * _overlayscreen->format->BytesPerPixel);
		src += _overlayscreen->pitch;
		buf += pitch;
	} while (--h);

	SDL_UnlockSurface(_overlayscreen);
}

void OSystem_SDL::copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) {
	if (_overlayscreen == NULL)
		return;

	// Clip the coordinates
	if (x < 0) {
		w += x;
		buf -= x;
		x = 0;
	}

	if (y < 0) {
		h += y; buf -= y * pitch;
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
		memcpy(dst, buf, w * _overlayscreen->format->BytesPerPixel);
		dst += _overlayscreen->pitch;
		buf += pitch;
	} while (--h);

	SDL_UnlockSurface(_overlayscreen);
}


#pragma mark -
#pragma mark --- Mouse ---
#pragma mark -

bool OSystem_SDL::showMouse(bool visible) {
	return false;
}
void OSystem_SDL::closeOverlay() {
	if (_overlayscreen) {
		SDL_FreeSurface(_overlayscreen);
		_overlayscreen = NULL;
#ifdef USE_OPENGL
		if (_overlayNumTex > 0) {
			glDeleteTextures(_overlayNumTex, _overlayTexIds);
			delete[] _overlayTexIds;
			_overlayNumTex = 0;
		}
#endif
	}
}

void OSystem_SDL::warpMouse(int x, int y) {
	SDL_WarpMouse(x, y);
/* Residual doesn't support this
	int y1 = y;

	if (_videoMode.aspectRatioCorrection && !_overlayVisible)
		y1 = real2Aspect(y);

	if (_mouseCurState.x != x || _mouseCurState.y != y) {
		if (!_overlayVisible)
			SDL_WarpMouse(x * _videoMode.scaleFactor, y1 * _videoMode.scaleFactor);
		else
			SDL_WarpMouse(x, y1);

		// SDL_WarpMouse() generates a mouse movement event, so
		// setMousePos() would be called eventually. However, the
		// cannon script in CoMI calls this function twice each time
		// the cannon is reloaded. Unless we update the mouse position
		// immediately the second call is ignored, causing the cannon
		// to change its aim.

		setMousePos(x, y);
	}*/
}

void OSystem_SDL::setMouseCursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y, uint32 keycolor, int cursorTargetScale, const Graphics::PixelFormat *format) {
#ifdef USE_RGB_COLOR
	if (!format)
		_cursorFormat = Graphics::PixelFormat::createFormatCLUT8();
	else if (format->bytesPerPixel <= _screenFormat.bytesPerPixel)
		_cursorFormat = *format;
	keycolor &= (1 << (_cursorFormat.bytesPerPixel << 3)) - 1;
#else
	keycolor &= 0xFF;
#endif

	if (w == 0 || h == 0)
		return;
/* Residual doesn't support this
	_mouseCurState.hotX = hotspot_x;
	_mouseCurState.hotY = hotspot_y;

	_mouseKeyColor = keycolor;

	_cursorTargetScale = cursorTargetScale;

	if (_mouseCurState.w != (int)w || _mouseCurState.h != (int)h) {
		_mouseCurState.w = w;
		_mouseCurState.h = h;

		if (_mouseOrigSurface)
			SDL_FreeSurface(_mouseOrigSurface);

		// Allocate bigger surface because AdvMame2x adds black pixel at [0,0]
		_mouseOrigSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA,
						_mouseCurState.w + 2,
						_mouseCurState.h + 2,
						16,
						_hwscreen->format->Rmask,
						_hwscreen->format->Gmask,
						_hwscreen->format->Bmask,
						_hwscreen->format->Amask);

		if (_mouseOrigSurface == NULL)
			error("allocating _mouseOrigSurface failed");
		SDL_SetColorKey(_mouseOrigSurface, SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA, kMouseColorKey);
	}

	free(_mouseData);
#ifdef USE_RGB_COLOR
	_mouseData = (byte *)malloc(w * h * _cursorFormat.bytesPerPixel);
	memcpy(_mouseData, buf, w * h * _cursorFormat.bytesPerPixel);
#else
	_mouseData = (byte *)malloc(w * h);
	memcpy(_mouseData, buf, w * h);
#endif
	blitCursor();*/
}

void OSystem_SDL::toggleMouseGrab() {
	if (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_OFF)
		SDL_WM_GrabInput(SDL_GRAB_ON);
	else
		SDL_WM_GrabInput(SDL_GRAB_OFF);
}
