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

	// shader number 0 is the entry NONE (no shader)
	const OSystem::GraphicsMode *p = s_supportedShadersPSP2;
	_numShaders = 0;
	while (p->name) {
		_numShaders++;
		p++;
	}
	_currentShader = GFX_SHADER_NONE;

	_shaders[0] = NULL;

	/* Vita display size is always 960x544 (that's just the hardware) */
	handleResize(960, 544, 90, 90);
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
	if (_hwScreen) {
		_hwScreen->pixels = _sdlpixels_hwscreen;
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

int PSP2SdlGraphicsManager::getDefaultShader() const {
	return GFX_SHADER_SHARP;
}

int PSP2SdlGraphicsManager::getShader() const {
	return _currentShader;
}

bool PSP2SdlGraphicsManager::setShader(int id) {
	assert(id >= 0 && id < _numShaders);
	_currentShader = id;
	updateShader();
	return true;
}

void PSP2SdlGraphicsManager::unloadGFXMode() {
	if (_screen) {
		SDL_FreeSurface(_screen);
		_screen = NULL;
	}

	deinitializeRenderer();

	if (_hwScreen) {
		if (_vitatex_hwscreen) {
			vita2d_free_texture(_vitatex_hwscreen);
			for (int i = 0; i < 6; i++) {
				vita2d_free_shader(_shaders[i]);
				_shaders[i] = NULL;
			}
			_vitatex_hwscreen = NULL;
		}
		_hwScreen->pixels = _sdlpixels_hwscreen;
	}
	SurfaceSdlGraphicsManager::unloadGFXMode();
}

bool PSP2SdlGraphicsManager::hotswapGFXMode() {
	if (!_screen)
		return false;

	// Release the HW screen surface
	if (_hwScreen) {
		if (_vitatex_hwscreen) {
			vita2d_free_texture(_vitatex_hwscreen);
			for (int i = 0; i < 6; i++) {
				vita2d_free_shader(_shaders[i]);
				_shaders[i] = NULL;
			}
			_vitatex_hwscreen = NULL;
		}
		_hwScreen->pixels = _sdlpixels_hwscreen;
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

SDL_Surface *PSP2SdlGraphicsManager::SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags) {

	SDL_Surface *screen = SurfaceSdlGraphicsManager::SDL_SetVideoMode(width, height, bpp, flags);

	if (screen != nullptr) {
		vita2d_set_vblank_wait(true);
		_vitatex_hwscreen = vita2d_create_empty_texture_format(width, height, SCE_GXM_TEXTURE_FORMAT_R5G6B5);
		_sdlpixels_hwscreen = screen->pixels; // for SDL_FreeSurface...
		screen->pixels = vita2d_texture_get_datap(_vitatex_hwscreen);
		screen->pitch = vita2d_texture_get_stride(_vitatex_hwscreen);
		updateShader();
	}
	return screen;
}

void PSP2SdlGraphicsManager::SDL_UpdateRects(SDL_Surface *screen, int numrects, SDL_Rect *rects) {
	int screenH = screen->h;
	int screenW = screen->w;

	int x = _activeArea.drawRect.left;
	int y = _activeArea.drawRect.top;
	float sx = _activeArea.drawRect.width() / (float)screenW;
	float sy = _activeArea.drawRect.height() / (float)screenH;
	if (_vitatex_hwscreen) {
		vita2d_start_drawing();
		vita2d_clear_screen();
		vita2d_draw_texture_scale(_vitatex_hwscreen, x, y, sx, sy);
		vita2d_end_drawing();
		vita2d_swap_buffers();
	}
}

#endif
