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
 * Based on the original engine code by EA Kids / Storm Software (1994).
 *
 */

#ifndef EEM_EEM_H
#define EEM_EEM_H

#include "common/platform.h"
#include "common/random.h"
#include "common/scummsys.h"

#include "engines/advancedDetector.h"
#include "engines/engine.h"

#include "eem/resource.h"

namespace EEM {

class Console;

/**
 * Screen IDs used by the original ScreenDriver dispatch table at 1a35:0e5e.
 * The table holds 14 (id, handler) entries; the loop iterates until it finds
 * a matching id and calls its handler. ID 0xFFFF is the exit sentinel.
 */
enum ScreenId {
	kScreenInvalid    = 0xFFFF,
	kScreenTitle      = 0x0B,  ///< _ShowTitlePage @ 1a35:06b7
	kScreenNext       = 0x08   ///< follow-up after title (case selection); to be confirmed
};

class EEMEngine : public Engine {
public:
	EEMEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~EEMEngine() override;

	Common::Error run() override;

	const char *getGameId() const;
	Common::Platform getPlatform() const;

	const ADGameDescription *_gameDescription;

	DBDArchive &getPics() { return _picsArchive; }

	/**
	 * Upload palette index @p num (one of 40 stored in SITEPALS) to the
	 * screen, with the VGA-DAC 6-bit-to-8-bit shift. Mirrors _GetPalette
	 * @ 172b:0e80 followed by _setmany @ 1000:0930.
	 */
	void setSitePalette(uint num);

	/** Blit @p pic to the screen at (0,0), expecting a 320x200 picture. */
	void blitFullScreen(const Picture &pic);

private:
	/**
	 * Central dispatch loop matching the original _ScreenDriver @ 1a35:0dc1.
	 * Each iteration restores video mode and calls the screen handler that
	 * matches _nextScreen. Handlers update _lastScreen / _nextScreen and
	 * return; the loop exits when _nextScreen == kScreenInvalid.
	 */
	void screenDriver();

	bool pollEvents();

	Console *_console;
	Common::RandomSource _rng;

	DBDArchive _picsArchive;  ///< PICS.DBD/.DBX (mouse, buttons, markers, balloons sprites)
	Common::Array<byte> _sitePals; ///< 40 x 768 bytes of 6-bit VGA palettes from SITEPALS

	uint16 _lastScreen;  ///< Mirrors _LastScreen @ 2d5d:3f24
	uint16 _nextScreen;  ///< Mirrors _NextScreen @ 2d5d:3f26
};

} // End of namespace EEM

#endif
