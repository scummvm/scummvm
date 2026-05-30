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

/*
 * Based on ScottFree interpreter version 1.14 developed by Swansea
 * University Computer Society without disassembly of any other game
 * drivers, only of game databases as permitted by EEC law (for purposes
 * of compatibility).
 *
 * Licensed under GPLv2
 *
 * https://github.com/angstsmurf/spatterlight/tree/master/terps/scott
 */

#include "common/formats/spectrum_tape.h"
#include "common/md5.h"
#include "common/memstream.h"
#include "common/system.h"
#include "graphics/managed_surface.h"
#include "glk/window_graphics.h"
#include "glk/scott/decompress_z80.h"
#include "glk/scott/globals.h"
#include "glk/scott/load_zx_spectrum.h"
#include "glk/scott/zx_spectrum.h"
#include "glk/scott/resource.h"
#include "glk/scott/scott.h"
#include "image/scr.h"

namespace Glk {
namespace Scott {

#define TITLE_SCREEN g_globals->_spectrumTitleScreen

static void loadZXSpectrumGame(const Common::String &md5) {
	int offset;
	DictionaryType dict_type = getId(&offset);
	if (dict_type == NOT_A_GAME)
		return;

	int index = _G(_md5Index)[md5];
	if (tryLoading(_G(_games)[index], offset, 0)) {
		_G(_game) = &_G(_games)[index];
	}
}

void loadZXSpectrum(Common::SeekableReadStream *f, const Common::String &md5) {
	TITLE_SCREEN.clear();

	_G(_entireFile) = new uint8_t[_G(_fileLength)];
	size_t result = f->read(_G(_entireFile), _G(_fileLength));
	if (result != _G(_fileLength))
		g_scott->fatal("File empty or read error!");

	uint8_t *uncompressed = decompressZ80(_G(_entireFile), _G(_fileLength));
	if (uncompressed != nullptr) {
		delete[] _G(_entireFile);
		_G(_entireFile) = uncompressed;
		_G(_fileLength) = 0xc000;
	}

	loadZXSpectrumGame(md5);
}

void loadZXSpectrumTape(Common::SeekableReadStream *f) {
	TITLE_SCREEN.clear();

	ZXSpectrumTapeData tape;
	if (!extractZXSpectrumTapeData(*f, tape))
		return;

	TITLE_SCREEN = tape.screen;

	_G(_fileLength) = tape.code.size();
	_G(_entireFile) = new uint8_t[_G(_fileLength)];
	memcpy(_G(_entireFile), tape.code.data(), _G(_fileLength));

	Common::MemoryReadStream codeStream(_G(_entireFile), _G(_fileLength));
	Common::String md5 = Common::computeStreamMD5AsString(codeStream);

	loadZXSpectrumGame(md5);
}

static Graphics::ManagedSurface *decodeZXSpectrumTitleScreen() {
	if (TITLE_SCREEN.size() != 6912)
		return nullptr;

	Common::Array<byte> titleScreen = TITLE_SCREEN;
	if (CURRENT_GAME == HULK) {
		byte *attrs = titleScreen.data() + 6144;
		for (int row = 22; row < 24; row++) {
			if (((attrs[row * 32] >> 3) & 0x07) == 7) {
				for (int col = 0; col < 32; col++)
					attrs[row * 32 + col] &= ~0x38;
			}
		}
	}

	Common::MemoryReadStream stream(titleScreen.data(), titleScreen.size());
	::Image::ScrDecoder decoder;
	if (!decoder.loadStream(stream))
		return nullptr;

	const Graphics::Surface *decoded = decoder.getSurface();
	Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
	surface->create(320, 200, g_system->getScreenFormat());
	surface->fillRect(Common::Rect(0, 0, surface->w, surface->h), surface->format.RGBToColor(0, 0, 0));
	surface->simpleBlitFrom(*decoded, Common::Point((surface->w - decoded->w) / 2, (surface->h - decoded->h) / 2),
		Graphics::FLIP_NONE, false, 255, &decoder.getPalette());
	return surface;
}

static void drawZXSpectrumTitleScreen(winid_t win, const Graphics::ManagedSurface &surface, int visibleLines) {
	GraphicsWindow *gfxWin = dynamic_cast<GraphicsWindow *>(win);
	if (!gfxWin || !gfxWin->_surface)
		return;

	g_scott->glk_window_clear(win);

	const int srcWidth = surface.w;
	const int srcHeight = surface.h;
	int width = gfxWin->_w;
	int height = width * srcHeight / srcWidth;
	if (height > (int)gfxWin->_h) {
		height = gfxWin->_h;
		width = height * srcWidth / srcHeight;
	}

	if (width <= 0 || height <= 0)
		return;

	const int x = ((int)gfxWin->_w - width) / 2;
	const int y = ((int)gfxWin->_h - height) / 2;
	const int visibleHeight = CLIP<int>(visibleLines, 0, srcHeight) * height / srcHeight;
	if (visibleHeight <= 0)
		return;

	if (width == srcWidth && height == srcHeight) {
		gfxWin->_surface->blitFrom(surface, Common::Rect(0, 0, srcWidth, visibleHeight), Common::Point(x, y));
	} else {
		Graphics::ManagedSurface *scaled = surface.scale(width, height);
		if (scaled) {
			gfxWin->_surface->blitFrom(*scaled, Common::Rect(0, 0, width, visibleHeight), Common::Point(x, y));
			scaled->free();
			delete scaled;
		}
	}
}

void showZXSpectrumTapeTitleScreen() {
	if (!g_scott->glk_gestalt(gestalt_Graphics, 0) || TITLE_SCREEN.empty())
		return;

	Graphics::ManagedSurface *surface = decodeZXSpectrumTitleScreen();
	if (!surface)
		return;

	winid_t win = g_scott->glk_window_open(_G(_bottomWindow), winmethod_Above | winmethod_Proportional,
		100, wintype_Graphics, GLK_GRAPHICS_ROCK);
	if (!win) {
		surface->free();
		delete surface;
		return;
	}

	g_scott->glk_window_set_background_color(win, 0);
	uint graphWidth, graphHeight;
	g_scott->glk_window_get_size(win, &graphWidth, &graphHeight);
	uint targetHeight = graphWidth * surface->h / surface->w;
	if (targetHeight > graphHeight)
		targetHeight = graphHeight;
	winid_t parent = g_scott->glk_window_get_parent(win);
	if (parent)
		g_scott->glk_window_set_arrangement(parent, winmethod_Above | winmethod_Fixed, targetHeight, win);
	int visibleLines = 0;
	drawZXSpectrumTitleScreen(win, *surface, visibleLines);
	g_scott->glk_request_char_event(_G(_bottomWindow));
	g_scott->glk_request_mouse_event(win);
	g_scott->glk_request_timer_events(15);

	event_t ev;
	int frames = 0;
	while (frames < 60 * 6 && !g_vm->shouldQuit()) {
		g_scott->glk_select(&ev);
		if (ev.type == evtype_CharInput || ev.type == evtype_MouseInput)
			break;
		if (ev.type == evtype_Arrange)
			drawZXSpectrumTitleScreen(win, *surface, visibleLines);
		else if (ev.type == evtype_Timer) {
			if (visibleLines < surface->h) {
				visibleLines = MIN<int>(visibleLines + 3, surface->h);
				drawZXSpectrumTitleScreen(win, *surface, visibleLines);
			}
			frames++;
		} else
			g_scott->updates(ev);
	}

	g_scott->glk_request_timer_events(0);
	g_scott->glk_cancel_char_event(_G(_bottomWindow));
	g_scott->glk_cancel_mouse_event(win);
	g_scott->glk_window_close(win, nullptr);

	surface->free();
	delete surface;
}

} // End of namespace Scott
} // End of namespace Glk
