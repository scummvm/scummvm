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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "image/bmp.h"
#include "common/events.h"
#include "common/formats/winexe.h"
#include "common/formats/winexe_pe.h"
#include "graphics/paletteman.h"
#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/qdcore/util/splash_screen.h"
#include "qdengine/qdengine.h"

namespace QDEngine {

bool SplashScreen::create(int bitmapResID) {
	if (!create_window()) return false;

	Common::PEResources r;
	Common::WinResourceID resid(bitmapResID);
	Image::BitmapDecoder decoder;

	if (r.loadFromEXE("shveik.exe")) {
		Common::SeekableReadStream *stream = r.getResource(Common::kWinBitmap, resid);
		if (decoder.loadStream(*stream)) {
			_splash = new Graphics::Surface();
			_splash->copyFrom(*decoder.getSurface());
			_paletteCount = decoder.getPaletteColorCount();
			_palette = new byte[_paletteCount * 3];

			memcpy(_palette, decoder.getPalette(), _paletteCount * 3);
		}
	}

	return true;
}

bool SplashScreen::create(const char *bitmap_file) {
	return true;
}

bool SplashScreen::set_mask(int mask_resid) {
	return true;
}

bool SplashScreen::set_mask(const char *mask_file) {
	return true;
}

bool SplashScreen::destroy() {
	delete _splash;
	delete[] _palette;
	return true;
}

void SplashScreen::show() {
	g_system->fillScreen(0);
	g_system->getPaletteManager()->setPalette(_palette, 0, _paletteCount);

	int x = (640 - _splash->w) / 2;
	int y = (480 - _splash->h) / 2;
	g_system->copyRectToScreen(_splash->getPixels(), _splash->pitch, x, y, _splash->w, _splash->h);
	g_system->updateScreen();

	start_time_ = g_system->getMillis();
}

void SplashScreen::wait(int time) {
	if (g_system->getMillis() - start_time_ < time)
		g_system->delayMillis(time - (g_system->getMillis() - start_time_));
}

void SplashScreen::hide() {
}

bool SplashScreen::create_window() {
	return true;
}

} // namespace QDEngine
