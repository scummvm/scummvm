/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
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
#include "common/scummsys.h"

#if defined(SDL_BACKEND)

#include "backends/graphics/psp2sdl/psp2sdl-graphics.h"
#include "backends/events/sdl/sdl-events.h"
#include "backends/platform/sdl/sdl.h"
#include "common/config-manager.h"
#include "common/mutex.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "common/util.h"
#include "common/frac.h"
#ifdef USE_RGB_COLOR
#include "common/list.h"
#endif
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/scaler.h"
#include "graphics/scaler/aspect.h"
#include "graphics/surface.h"
#include "gui/EventRecorder.h"

#include <vita2d_fbo.h>
#include <lcd3x_v.h>
#include <lcd3x_f.h>
#include <texture_v.h>
#include <texture_f.h>
#include <advanced_aa_v.h>
#include <advanced_aa_f.h>
#include <scale2x_f.h>
#include <scale2x_v.h>
#include <sharp_bilinear_f.h>
#include <sharp_bilinear_v.h>
#include <sharp_bilinear_simple_f.h>
#include <sharp_bilinear_simple_v.h>

#define GFX_SHADER_NONE 0
#define GFX_SHADER_LCD3X 1
#define GFX_SHADER_SHARP 2
#define GFX_SHADER_SHARP_SCAN 3
#define GFX_SHADER_AAA 4
#define GFX_SHADER_SCALE2X 5

static const OSystem::GraphicsMode s_supportedShadersPSP2[] = {
	{"NONE", "Normal (no shader)", GFX_SHADER_NONE},
	{"LCD", "LCD", GFX_SHADER_LCD3X},
	{"Sharp", "Sharp", GFX_SHADER_SHARP},
	{"Scan", "Scan", GFX_SHADER_SHARP_SCAN},
	{"AAA", "Super2xSAI", GFX_SHADER_AAA},
	{"Scale", "Scale", GFX_SHADER_SCALE2X},
	{0, 0, 0}
};

PSP2SdlGraphicsManager::PSP2SdlGraphicsManager(SdlEventSource *sdlEventSource, SdlWindow *window)
	:
	SurfaceSdlGraphicsManager(sdlEventSource, window),
	_vitatex_hwscreen(nullptr),
	_sdlpixels_hwscreen(nullptr) {

	// do aspect ratio correction in hardware on the Vita
	if (_videoMode.aspectRatioCorrection == true) {
		_hardwareAspectRatioCorrection = true;
	} else {
		_hardwareAspectRatioCorrection = false;
	}
	_videoMode.aspectRatioCorrection = false;

	// shader number 0 is the entry NONE (no shader)
	const OSystem::GraphicsMode *p = s_supportedShadersPSP2;
	_numShaders = 0;
	while (p->name) { 
		_numShaders++;
		p++;
	}
	_currentShader = ConfMan.getInt("shader");
	if (_currentShader < 0 || _currentShader >= _numShaders) {
		_currentShader = 0;
	}

	_shaders[0] = NULL;
}

PSP2SdlGraphicsManager::~PSP2SdlGraphicsManager() {
	if (_vitatex_hwscreen) {
		vita2d_free_texture(_vitatex_hwscreen);
		for (int i = 0; i < 6; i++) {
			vita2d_free_shader(_shaders[i]);
			_shaders[i] = NULL;
		}
		_vitatex_hwscreen = NULL;
	}
	if (_hwscreen) {
		_hwscreen->pixels = _sdlpixels_hwscreen;
	}
	_sdlpixels_hwscreen = nullptr;
}

OSystem::TransactionError PSP2SdlGraphicsManager::endGFXTransaction() {
		OSystem::TransactionError returnValue = SurfaceSdlGraphicsManager::endGFXTransaction();

		// force update of filtering on Vita
		PSP2_UpdateFiltering();

		return returnValue;
}

void PSP2SdlGraphicsManager::setGraphicsModeIntern() {
	SurfaceSdlGraphicsManager::setGraphicsModeIntern();
	PSP2_UpdateFiltering();
}

void PSP2SdlGraphicsManager::PSP2_UpdateFiltering() {
	if (_vitatex_hwscreen) {
		if (_videoMode.filtering) {
			vita2d_texture_set_filters(_vitatex_hwscreen, SCE_GXM_TEXTURE_FILTER_LINEAR, SCE_GXM_TEXTURE_FILTER_LINEAR);
		} else {
			vita2d_texture_set_filters(_vitatex_hwscreen, SCE_GXM_TEXTURE_FILTER_POINT, SCE_GXM_TEXTURE_FILTER_POINT);
		}
	}
}

const OSystem::GraphicsMode *PSP2SdlGraphicsManager::getSupportedShaders() const {
	return s_supportedShadersPSP2;
}

void PSP2SdlGraphicsManager::unloadGFXMode() {
	if (_screen) {
		SDL_FreeSurface(_screen);
		_screen = NULL;
	}

	deinitializeRenderer();

	if (_hwscreen) {
		if (_vitatex_hwscreen) {
			vita2d_free_texture(_vitatex_hwscreen);
			for (int i = 0; i < 6; i++) {
				vita2d_free_shader(_shaders[i]);
				_shaders[i] = NULL;
			}
			_vitatex_hwscreen = NULL;
		}
		_hwscreen->pixels = _sdlpixels_hwscreen;
	}
	SurfaceSdlGraphicsManager::unloadGFXMode();
}

bool PSP2SdlGraphicsManager::hotswapGFXMode() {
	if (!_screen)
		return false;

	// Release the HW screen surface
	if (_hwscreen) {
		if (_vitatex_hwscreen) {
			vita2d_free_texture(_vitatex_hwscreen);
			for (int i = 0; i < 6; i++) {
				vita2d_free_shader(_shaders[i]);
				_shaders[i] = NULL;
			}
			_vitatex_hwscreen = NULL;
		}
		_hwscreen->pixels = _sdlpixels_hwscreen;
	}
	return SurfaceSdlGraphicsManager::hotswapGFXMode();
}

void PSP2SdlGraphicsManager::updateShader() {
// shader init code goes here
// currently only used on Vita port
// the user-selected shaderID should be obtained via ConfMan.getInt("shader")
// and the corresponding shader should then be activated here
// this way the user can combine any software scaling (scalers)
// with any hardware shading (shaders). The shaders could provide
// scanline masks, overlays, but could also serve for
// hardware-based up-scaling (sharp-bilinear-simple, etc.)
	if (_vitatex_hwscreen) {
		if (_shaders[0] == NULL) { 
			// load shaders
			_shaders[GFX_SHADER_NONE] = vita2d_create_shader((const SceGxmProgram *)texture_v, (const SceGxmProgram *)texture_f);
			_shaders[GFX_SHADER_LCD3X] = vita2d_create_shader((const SceGxmProgram *)lcd3x_v, (const SceGxmProgram *)lcd3x_f);
			_shaders[GFX_SHADER_SHARP] = vita2d_create_shader((const SceGxmProgram *)sharp_bilinear_simple_v, (const SceGxmProgram *)sharp_bilinear_simple_f);
			_shaders[GFX_SHADER_SHARP_SCAN] = vita2d_create_shader((const SceGxmProgram *)sharp_bilinear_v, (const SceGxmProgram *)sharp_bilinear_f);
			_shaders[GFX_SHADER_AAA] = vita2d_create_shader((const SceGxmProgram *)advanced_aa_v, (const SceGxmProgram *)advanced_aa_f);
			_shaders[GFX_SHADER_SCALE2X] = vita2d_create_shader((const SceGxmProgram *)scale2x_v, (const SceGxmProgram *)scale2x_f);
		}
		if (_currentShader >= 0 && _currentShader < _numShaders) {
			vita2d_texture_set_program(_shaders[_currentShader]->vertexProgram, _shaders[_currentShader]->fragmentProgram);
			vita2d_texture_set_wvp(_shaders[_currentShader]->wvpParam);
			vita2d_texture_set_vertexInput(&_shaders[_currentShader]->vertexInput);
			vita2d_texture_set_fragmentInput(&_shaders[_currentShader]->fragmentInput);
		}
	}
}

void PSP2SdlGraphicsManager::internUpdateScreen() {
	SDL_Surface *srcSurf, *origSurf;
	int height, width;
	ScalerProc *scalerProc;
	int scale1;

	// definitions not available for non-DEBUG here. (needed this to compile in SYMBIAN32 & linux?)
#if defined(DEBUG)
	assert(_hwscreen != NULL);
	assert(_hwscreen->map->sw_data != NULL);
#endif

	// If the shake position changed, fill the dirty area with blackness
	if (_currentShakePos != _newShakePos ||
		(_mouseNeedsRedraw && _mouseBackup.y <= _currentShakePos)) {
		SDL_Rect blackrect = {0, 0, (Uint16)(_videoMode.screenWidth * _videoMode.scaleFactor), (Uint16)(_newShakePos * _videoMode.scaleFactor)};

		if (_videoMode.aspectRatioCorrection && !_overlayVisible)
			blackrect.h = real2Aspect(blackrect.h - 1) + 1;

		SDL_FillRect(_hwscreen, &blackrect, 0);

		_currentShakePos = _newShakePos;

		_forceFull = true;
	}

	// Check whether the palette was changed in the meantime and update the
	// screen surface accordingly.
	if (_screen && _paletteDirtyEnd != 0) {
		SDL_SetColors(_screen, _currentPalette + _paletteDirtyStart,
			_paletteDirtyStart,
			_paletteDirtyEnd - _paletteDirtyStart);

		_paletteDirtyEnd = 0;

		_forceFull = true;
	}

	if (!_overlayVisible) {
		origSurf = _screen;
		srcSurf = _tmpscreen;
		width = _videoMode.screenWidth;
		height = _videoMode.screenHeight;
		scalerProc = _scalerProc;
		scale1 = _videoMode.scaleFactor;
	} else {
		origSurf = _overlayscreen;
		srcSurf = _tmpscreen2;
		width = _videoMode.overlayWidth;
		height = _videoMode.overlayHeight;
		scalerProc = Normal1x;

		scale1 = 1;
	}

	// Add the area covered by the mouse cursor to the list of dirty rects if
	// we have to redraw the mouse.
	if (_mouseNeedsRedraw)
		undrawMouse();

#ifdef USE_OSD
	updateOSD();
#endif

	// Force a full redraw if requested
	if (_forceFull) {
		_numDirtyRects = 1;
		_dirtyRectList[0].x = 0;
		_dirtyRectList[0].y = 0;
		_dirtyRectList[0].w = width;
		_dirtyRectList[0].h = height;
	}

	// Only draw anything if necessary
	if (_numDirtyRects > 0 || _mouseNeedsRedraw) {
		SDL_Rect *r;
		SDL_Rect dst;
		uint32 srcPitch, dstPitch;
		SDL_Rect *lastRect = _dirtyRectList + _numDirtyRects;

		for (r = _dirtyRectList; r != lastRect; ++r) {
			dst = *r;
			dst.x++;	// Shift rect by one since 2xSai needs to access the data around
			dst.y++;	// any pixel to scale it, and we want to avoid mem access crashes.

			if (SDL_BlitSurface(origSurf, r, srcSurf, &dst) != 0)
				error("SDL_BlitSurface failed: %s", SDL_GetError());
		}

		SDL_LockSurface(srcSurf);
		srcPitch = srcSurf->pitch;
		dstPitch = _hwscreen->pitch;

		for (r = _dirtyRectList; r != lastRect; ++r) {
			register int dst_y = r->y + _currentShakePos;
			register int dst_h = 0;
#ifdef USE_SCALERS
			register int orig_dst_y = 0;
#endif
			register int rx1 = r->x * scale1;

			if (dst_y < height) {
				dst_h = r->h;
				if (dst_h > height - dst_y)
					dst_h = height - dst_y;

#ifdef USE_SCALERS
				orig_dst_y = dst_y;
#endif
				dst_y = dst_y * scale1;

				if (_videoMode.aspectRatioCorrection && !_overlayVisible)
					dst_y = real2Aspect(dst_y);

				assert(scalerProc != NULL);
				scalerProc((byte *)srcSurf->pixels + (r->x * 2 + 2) + (r->y + 1) * srcPitch, srcPitch,
					(byte *)_hwscreen->pixels + rx1 * 2 + dst_y * dstPitch, dstPitch, r->w, dst_h);
			}

			r->x = rx1;
			r->y = dst_y;
			r->w = r->w * scale1;
			r->h = dst_h * scale1;

#ifdef USE_SCALERS
			if (_videoMode.aspectRatioCorrection && orig_dst_y < height && !_overlayVisible)
				r->h = stretch200To240((uint8 *) _hwscreen->pixels, dstPitch, r->w, r->h, r->x, r->y, orig_dst_y * scale1);
#endif
		}
		SDL_UnlockSurface(srcSurf);
		// Readjust the dirty rect list in case we are doing a full update.
		// This is necessary if shaking is active.
		if (_forceFull) {
			_dirtyRectList[0].y = 0;
			_dirtyRectList[0].h = effectiveScreenHeight();
		}

		drawMouse();

#ifdef USE_OSD
		drawOSD();
#endif

#ifdef USE_SDL_DEBUG_FOCUSRECT
		// We draw the focus rectangle on top of everything, to assure it's easily visible.
		// Of course when the overlay is visible we do not show it, since it is only for game
		// specific focus.
		if (_enableFocusRect && !_overlayVisible) {
			int y = _focusRect.top + _currentShakePos;
			int h = 0;
			int x = _focusRect.left * scale1;
			int w = _focusRect.width() * scale1;

			if (y < height) {
				h = _focusRect.height();
				if (h > height - y)
					h = height - y;

				y *= scale1;

				if (_videoMode.aspectRatioCorrection && !_overlayVisible)
					y = real2Aspect(y);

				if (h > 0 && w > 0) {
					// Use white as color for now.
					Uint32 rectColor = SDL_MapRGB(_hwscreen->format, 0xFF, 0xFF, 0xFF);

					// First draw the top and bottom lines
					// then draw the left and right lines
					if (_hwscreen->format->BytesPerPixel == 2) {
						uint16 *top = (uint16 *)((byte *)_hwscreen->pixels + y * _hwscreen->pitch + x * 2);
						uint16 *bottom = (uint16 *)((byte *)_hwscreen->pixels + (y + h) * _hwscreen->pitch + x * 2);
						byte *left = ((byte *)_hwscreen->pixels + y * _hwscreen->pitch + x * 2);
						byte *right = ((byte *)_hwscreen->pixels + y * _hwscreen->pitch + (x + w - 1) * 2);

						while (w--) {
							*top++ = rectColor;
							*bottom++ = rectColor;
						}

						while (h--) {
							*(uint16 *)left = rectColor;
							*(uint16 *)right = rectColor;

							left += _hwscreen->pitch;
							right += _hwscreen->pitch;
						}
					} else if (_hwscreen->format->BytesPerPixel == 4) {
						uint32 *top = (uint32 *)((byte *)_hwscreen->pixels + y * _hwscreen->pitch + x * 4);
						uint32 *bottom = (uint32 *)((byte *)_hwscreen->pixels + (y + h) * _hwscreen->pitch + x * 4);
						byte *left = ((byte *)_hwscreen->pixels + y * _hwscreen->pitch + x * 4);
						byte *right = ((byte *)_hwscreen->pixels + y * _hwscreen->pitch + (x + w - 1) * 4);

						while (w--) {
							*top++ = rectColor;
							*bottom++ = rectColor;
						}

						while (h--) {
							*(uint32 *)left = rectColor;
							*(uint32 *)right = rectColor;

							left += _hwscreen->pitch;
							right += _hwscreen->pitch;
						}
					}
				}
			}
		}
#endif

		// Finally, blit all our changes to the screen
		if (!_displayDisabled) {
			PSP2_UpdateRects(_hwscreen, _numDirtyRects, _dirtyRectList);
		}
	}

	_numDirtyRects = 0;
	_forceFull = false;
	_mouseNeedsRedraw = false;
}

void PSP2SdlGraphicsManager::setAspectRatioCorrection(bool enable) {
	Common::StackLock lock(_graphicsMutex);

	if (_oldVideoMode.setup && _hardwareAspectRatioCorrection == enable)
		return;

	if (_transactionMode == kTransactionActive) {
		_videoMode.aspectRatioCorrection = false;
		_hardwareAspectRatioCorrection = enable;
		// erase the screen for both buffers
		if (_vitatex_hwscreen) {
			for (int i = 0; i <= 10; i++) {
				vita2d_start_drawing();
				vita2d_clear_screen();
				vita2d_end_drawing();
				vita2d_swap_buffers();
			}
		}
	}
}

SDL_Surface *PSP2SdlGraphicsManager::SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags) {
	SDL_Surface *screen = SurfaceSdlGraphicsManager::SDL_SetVideoMode(width, height, bpp, flags);
	
	if (screen != nullptr) {
		vita2d_set_vblank_wait(true);
		_vitatex_hwscreen = vita2d_create_empty_texture_format(width, height, SCE_GXM_TEXTURE_FORMAT_R5G6B5);
		_sdlpixels_hwscreen = screen->pixels; // for SDL_FreeSurface...
		screen->pixels = vita2d_texture_get_datap(_vitatex_hwscreen);
		updateShader();
	}
	return screen;
}

void PSP2SdlGraphicsManager::PSP2_UpdateRects(SDL_Surface *screen, int numrects, SDL_Rect *rects) {
	int x, y, w, h;
	float sx, sy;
	float ratio = (float)screen->w / (float)screen->h;

	if ((_videoMode.screenHeight == 200 || _videoMode.screenHeight == 400) && _hardwareAspectRatioCorrection) {
		ratio = ratio * (200.0f / 240.0f);
	}

	if (_videoMode.fullscreen || screen->h >= 544) {
		h = 544; 
		w = h * ratio;
	} else {
		if (screen->h <= 277 && screen->w <= 480) {
			// Use Vita hardware 2x scaling if the picture is really small
			// this uses the current shader and filtering mode
			h = screen->h * 2;
			w = screen->w * 2;
		} else {
			h = screen->h;
			w = screen->w;
		}
		if ((_videoMode.screenHeight == 200 || _videoMode.screenHeight == 400) && _hardwareAspectRatioCorrection) {
			// stretch the height only if it fits, otherwise make the width smaller
			if (((float)w * (1.0f / ratio)) <= 544.0f) {
				h = w * (1.0f / ratio);
			} else {
				w = h * ratio;
			}
		}
	}
	
	x = (960 - w) / 2; y = (544 - h) / 2;
	sx = (float)w / (float)screen->w;
	sy = (float)h / (float)screen->h;
	if (_vitatex_hwscreen) {
		vita2d_start_drawing();
		vita2d_draw_texture_scale(_vitatex_hwscreen, x, y, sx, sy);
		vita2d_end_drawing();
		vita2d_swap_buffers();
	}
}

#endif
