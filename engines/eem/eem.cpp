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

#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/path.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "engines/util.h"

#include "graphics/paletteman.h"

#include "eem/console.h"
#include "eem/detection.h"
#include "eem/eem.h"

namespace {
const uint kPalSize = 768;     ///< 256 colors * 3 bytes
const uint kNumSitePals = 40;  ///< SITEPALS holds 40 palettes (40 * 768 = 30720)
} // anonymous namespace

namespace EEM {

EEMEngine::EEMEngine(OSystem *syst, const ADGameDescription *gameDesc)
	: Engine(syst), _gameDescription(gameDesc), _console(nullptr),
	  _rng("eem"), _lastScreen(kScreenInvalid), _nextScreen(kScreenTitle) {
}

EEMEngine::~EEMEngine() {
	// _console is owned by the Engine base class.
}

Common::Error EEMEngine::run() {
	// Original _main @ 1a35:0f59 enters mode 13h via _SetMode13X (320x200x256).
	initGraphics(320, 200);

	_console = new Console(this);
	setDebugger(_console);

	// _main's startup paints the screen black via _AllBlack @ 172b:0d4b before
	// the first screen handler runs; we do the same here.
	byte palette[3 * 256] = { 0 };
	g_system->getPaletteManager()->setPalette(palette, 0, 256);

	// Mirrors _main's `_picsFile = _fopen("PICS.DBD", ...)` plus
	// _InitGraphicsSystem's PICS.DBX index parse (172b:0145).
	if (!_picsArchive.open(Common::Path("PICS.DBD"), Common::Path("PICS.DBX"))) {
		return Common::Error(Common::kReadingFailed, "PICS archive missing");
	}

	// Mirrors _ReadPalettes @ 172b:0d89 — slurp SITEPALS in one read.
	Common::File palFile;
	if (!palFile.open(Common::Path("SITEPALS"))) {
		return Common::Error(Common::kReadingFailed, "SITEPALS missing");
	}
	_sitePals.resize(palFile.size());
	if (palFile.read(_sitePals.data(), _sitePals.size()) != _sitePals.size()) {
		return Common::Error(Common::kReadingFailed, "SITEPALS short read");
	}
	palFile.close();
	debugC(1, kDebugGfx, "Loaded %u SITEPALS palettes", (uint)(_sitePals.size() / kPalSize));

	debugC(1, kDebugGeneral, "EEM engine starting; first screen = 0x%02X", _nextScreen);

	// Show the first intro image (EA Kids logo) — mirrors the opening of
	// _ShowEAKids @ 2520:05f0: GetPicture(0x54), MemoryCopy to 0xa000:0,
	// GetPalette(0x25), setmany(_fpal, 0). Skipped: color-cycle loop.
	{
		Picture eakids;
		if (!_picsArchive.getPicture(0x54, eakids)) {
			return Common::Error(Common::kReadingFailed, "EA Kids logo (picture #0x54) load failed");
		}
		debugC(1, kDebugGfx, "EA Kids logo: %dx%d", eakids.surface.w, eakids.surface.h);
		blitFullScreen(eakids);
		setSitePalette(0x25);
		g_system->updateScreen();

		// Hold the image for up to 3 s or until the user clicks/keys/quits.
		const uint32 startMs = g_system->getMillis();
		while (g_system->getMillis() - startMs < 3000) {
			Common::Event event;
			bool stop = false;
			while (g_system->getEventManager()->pollEvent(event)) {
				if (event.type == Common::EVENT_QUIT ||
					event.type == Common::EVENT_RETURN_TO_LAUNCHER ||
					event.type == Common::EVENT_LBUTTONDOWN ||
					event.type == Common::EVENT_KEYDOWN) {
					stop = true;
					break;
				}
			}
			if (stop)
				break;
			g_system->updateScreen();
			g_system->delayMillis(10);
		}
	}

	screenDriver();

	debugC(1, kDebugGeneral, "EEM engine exiting");
	return Common::kNoError;
}

void EEMEngine::screenDriver() {
	// Mirrors _ScreenDriver @ 1a35:0dc1. The original walks a 14-entry table at
	// 1a35:0e5e of (id, handler) pairs; we use a switch as we port handlers in.
	while (_nextScreen != kScreenInvalid && !shouldQuit()) {
		ScreenId next = static_cast<ScreenId>(_nextScreen);
		switch (next) {
		case kScreenTitle:
			// TODO(M3): port _ShowTitlePage @ 1a35:06b7
			warning("Screen 0x%02X (title) not implemented yet", next);
			_lastScreen = _nextScreen;
			_nextScreen = kScreenInvalid;
			break;
		default:
			warning("Unknown screen id 0x%02X; exiting", next);
			_nextScreen = kScreenInvalid;
			break;
		}

		// Until handlers run their own event loops, pump events here so the
		// engine remains responsive and the user can quit.
		if (!pollEvents())
			break;
	}
}

void EEMEngine::setSitePalette(uint num) {
	if (num >= kNumSitePals || _sitePals.size() < (num + 1) * kPalSize) {
		warning("setSitePalette: index %u out of range", num);
		return;
	}
	// SITEPALS stores 6-bit VGA-DAC values (0..63); ScummVM expects 8-bit
	// (0..255), so left-shift by 2 like the original VGA hardware did.
	const byte *src = _sitePals.data() + num * kPalSize;
	byte expanded[kPalSize];
	for (uint i = 0; i < kPalSize; i++) {
		expanded[i] = (byte)(src[i] << 2);
	}
	g_system->getPaletteManager()->setPalette(expanded, 0, 256);
}

void EEMEngine::blitFullScreen(const Picture &pic) {
	// _MemoryCopy(0, 0xa000, srcOff, srcSeg) in _ShowEAKids dumps the picture
	// straight into VGA's 320x200 framebuffer.
	g_system->copyRectToScreen(pic.surface.getPixels(), pic.surface.pitch,
							   0, 0, pic.surface.w, pic.surface.h);
}

bool EEMEngine::pollEvents() {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			return false;
		default:
			break;
		}
	}
	g_system->updateScreen();
	g_system->delayMillis(10);
	return true;
}

} // End of namespace EEM
