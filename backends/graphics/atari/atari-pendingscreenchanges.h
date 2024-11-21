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

#ifndef BACKENDS_GRAPHICS_ATARI_PENDINGSCREENCHANGES_H
#define BACKENDS_GRAPHICS_ATARI_PENDINGSCREENCHANGES_H

#include <utility>

class AtariGraphicsManager;
class Screen;
namespace Graphics {
class Surface;
}

class PendingScreenChanges {
public:
	PendingScreenChanges(const AtariGraphicsManager *manager)
		: _manager(manager) {
	}

	void clearTransaction() {
		_changes &= ~kTransaction;
	}

	void setScreenSurface(Graphics::Surface *surface) {
		_surface = surface;
	}

	void queueVideoMode() {
		_changes |= kVideoMode;
	}
	void queueAspectRatioCorrection() {
		_changes |= kAspectRatioCorrection;
	}
	void queuePalette() {
		_changes |= kPalette;
	}
	void queueShakeScreen() {
		_changes |= kShakeScreen;
	}
	void queueAll();

	int get() const {
		return _changes;
	}
	bool empty() const {
		return _changes == kNone;
	}

	Graphics::Surface *screenSurface() const {
		return _surface;
	}
	const std::pair<int, bool>& aspectRatioCorrectionYOffset() const {
		return _aspectRatioCorrectionYOffset;
	}
	const std::pair<bool, bool>& screenOffsets() const {
		return _setScreenOffsets;
	}
	const std::pair<bool, bool>& shrinkVidelVisibleArea() const {
		return _shrinkVidelVisibleArea;
	}

	void applyBeforeVblLock(const Screen &screen);
	void applyAfterVblLock(const Screen &screen);

private:
	void processAspectRatioCorrection(const Screen &screen);
	void processVideoMode(const Screen &screen);

	enum Change {
		kNone                  = 0,
		kVideoMode             = 1<<0,
		kAspectRatioCorrection = 1<<1,
		kPalette               = 1<<2,
		kShakeScreen           = 1<<3,
		kTransaction           = kVideoMode | kAspectRatioCorrection,
		kAll                   = kTransaction | kPalette | kShakeScreen
	};
	int _changes = kNone;

	const AtariGraphicsManager *_manager;

	Graphics::Surface *_surface = nullptr;

	int _mode;
	bool _resetSuperVidel;
	bool _switchToBlackPalette;

	// <value, set> ... std::optional would be so much better!
	std::pair<int, bool> _aspectRatioCorrectionYOffset;
	std::pair<bool, bool> _setScreenOffsets;
	std::pair<bool, bool> _shrinkVidelVisibleArea;

};

#endif // ATARI-PENDINGSCREENCHANGES_H
