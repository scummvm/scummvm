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

#include "atari-pendingscreenchanges.h"

#include <cassert>
#include <mint/falcon.h>

#include "backends/platform/atari/atari-debug.h"
#include "graphics/surface.h"

#include "atari-graphics.h"
#include "atari-graphics-superblitter.h"

void PendingScreenChanges::queueAll() {
	_changes |= kAll;

	if (_manager->_tt)
		_changes &= ~kAspectRatioCorrection;
}

/*
 * VsetRGB() - stores the palette in a buffer and applyBeforeVblLock in nearest VBL
 * EsetPalette() - immediatelly applies the palette
 * (V)SetScreen() - immediatelly sets physbase/logbase but explicitly calls Vsync() for resolution changes
 * VsetMode() - explicitly calls Vsync()
 */

void PendingScreenChanges::applyBeforeVblLock(const Screen &screen) {
	_mode = screen.mode;	// avoid modifying 'Screen' content
	_resetSuperVidel = false;
	_switchToBlackPalette = (_changes & kVideoMode);

	_aspectRatioCorrectionYOffset.second = false;
	_setScreenOffsets.second = false;
	_shrinkVidelVisibleArea.second = false;

	if (_changes & kAspectRatioCorrection) {
		processAspectRatioCorrection(screen);
		_changes &= ~kAspectRatioCorrection;
	}

	_switchToBlackPalette |= _resetSuperVidel;

	if (_changes & kVideoMode) {
		processVideoMode(screen);
		// don't reset kVideoMode yet
	}
}

void PendingScreenChanges::applyAfterVblLock(const Screen &screen) {
	// VBL doesn't process new palette nor screen address updates

	if (_changes & kShakeScreen) {
		_setScreenOffsets = std::make_pair(true, true);
		_changes &= ~kShakeScreen;
	}

	// restore current (kVideoMode) or set new (kPalette) palette
	if (_changes & (kVideoMode | kPalette)) {
		if (_switchToBlackPalette || (_changes & kPalette)) {
			if (_manager->_tt) {
				if (_changes & kPalette)
					Vsync();
				EsetPalette(0, screen.palette->entries, screen.palette->tt);
			} else {
				VsetRGB(0, screen.palette->entries, screen.palette->falcon);
				if (_changes & kPalette)
					Vsync();
			}
		}

		_changes &= ~(kVideoMode | kPalette);
	}

	assert(_changes == kNone);
}

void PendingScreenChanges::processAspectRatioCorrection(const Screen &screen) {
	assert(!_manager->_tt);
	assert(_mode != -1);

	if (_manager->_aspectRatioCorrection && _manager->_currentState.height == 200 && !_manager->isOverlayVisible()) {
		// apply machine-specific aspect ratio correction
		if (!_manager->_vgaMonitor) {
			_mode &= ~PAL;
			// 60 Hz
			_mode |= NTSC;
			_changes |= kVideoMode;
		} else {
			_aspectRatioCorrectionYOffset =
				std::make_pair((screen.surf.h - 2*MAX_V_SHAKE - screen.offsettedSurf->h) / 2, true);
			_shrinkVidelVisibleArea = std::make_pair(true, true);
		}
	} else {
		// reset back to default mode
		if (!_manager->_vgaMonitor) {
			_mode &= ~NTSC;
			// 50 Hz
			_mode |= PAL;
			_changes |= kVideoMode;
		} else {
			_aspectRatioCorrectionYOffset = std::make_pair(0, true);
			_shrinkVidelVisibleArea = std::make_pair(false, true);

			if (hasSuperVidel())
				_resetSuperVidel = true;

			// kPendingVideoMode will reset the shrunken Videl area
			_changes |= kVideoMode;
		}
	}

	// for VsetMode() and/or _aspectRatioCorrectionYOffset
	_setScreenOffsets = std::make_pair(true, true);
}

void PendingScreenChanges::processVideoMode(const Screen &screen) {
	// changing video mode implies an additional Vsync(): there's no way to change resolution
	// and set new screen address (and/or shake offsets etc) in one go
	if (screen.rez != -1) {
		if (_switchToBlackPalette) {
			static uint16 black[256];
			EsetPalette(0, screen.palette->entries, black);
		}

		// unfortunately this reinitializes VDI, too
		Setscreen(SCR_NOCHANGE, SCR_NOCHANGE, screen.rez);
	} else if (_mode != -1) {
		if (_switchToBlackPalette) {
			static _RGB black[256];
			VsetRGB(0, screen.palette->entries, black);
		}

		  // VsetMode() must be called first: it resets all hz/v, scrolling and line width registers
		if (_resetSuperVidel)
			VsetMode(SVEXT | SVEXT_BASERES(0) | COL80 | BPS8C);	// resync to proper 640x480

		atari_debug("VsetMode: %04x", _mode);
		VsetMode(_mode);
	}
}
