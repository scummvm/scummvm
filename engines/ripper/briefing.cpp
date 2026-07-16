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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/briefing.h"

#include "common/debug.h"
#include "common/system.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/ripper.h"

namespace Ripper {

static const int kBriefingRightEdge = 635;
static const int kBriefingSceneOriginY = 50;
static const int kBriefingControlOffsetY = 305;
static const uint kBriefingCursor = 16;
static const uint32 kDosTickMillis = 55;
static const uint32 kBriefingFrameInterval = 3 * kDosTickMillis;
static const uint kBriefingAlertVolume = 34;

BriefingManager::BriefingManager(RipperEngine *engine) : _engine(engine),
		_lastFrameMillis(0), _frameIndex(0), _selector(0), _armed(false),
		_initialized(false) {
	memset(_announcedSelectors, 0, sizeof(_announcedSelectors));
}

bool BriefingManager::initialize(ResourceManager &resources) {
	// LoadFrontEndToolbarResources at 0x18678 stores toolbar action four at
	// g_frontEndToolbarResources+0xc. ArmBriefingMediaTrigger at 0x1929a
	// reuses that same frame set for the lower-band message control.
	if (!resources.loadInterfaceBitmapSequence("toolbar4.pl", _frames) ||
			_frames.frames.empty())
		return false;

	const BitmapAssetFrame &frame = _frames.frames.front();
	for (uint i = 1; i < _frames.frames.size(); ++i) {
		if (_frames.frames[i].width != frame.width ||
				_frames.frames[i].height != frame.height)
			return false;
	}
	_bounds = Common::Rect(kBriefingRightEdge - frame.width,
		kBriefingSceneOriginY + kBriefingControlOffsetY,
		kBriefingRightEdge,
		kBriefingSceneOriginY + kBriefingControlOffsetY + frame.height);
	_initialized = true;
	debugC(1, kDebugScene,
		"Ripper: initialized briefing trigger frames=%u rect=%d,%d,%d,%d cursor=%u",
		_frames.frames.size(), _bounds.left, _bounds.top, _bounds.width(),
		_bounds.height(), kBriefingCursor);
	return true;
}

bool BriefingManager::arm(uint selector, bool playNotification) {
	if (!_initialized || selector == 0 || selector >= ARRAYSIZE(_announcedSelectors)) {
		warning("Ripper: invalid briefing selector %u", selector);
		return false;
	}
	if (_announcedSelectors[selector]) {
		debugC(2, kDebugScene,
			"Ripper: briefing selector=%u already announced; trigger not rearmed", selector);
		return true;
	}

	_announcedSelectors[selector] = true;
	_selector = selector;
	_frameIndex = 0;
	_lastFrameMillis = g_system->getMillis(true);
	_armed = true;
	draw();
	g_system->updateScreen();
	debugC(1, kDebugScene,
		"Ripper: armed briefing trigger selector=%u control=0x4e1 frame=%u rect=%d,%d,%d,%d",
		_selector, _frameIndex, _bounds.left, _bounds.top, _bounds.width(), _bounds.height());

	if (playNotification && !_engine->getMedia()->playBlockingAudio("wacicon0.wav"))
		return false;
	return true;
}

void BriefingManager::restore(bool armed, uint selector) {
	_armed = armed && _initialized && selector != 0 &&
		selector < ARRAYSIZE(_announcedSelectors);
	_selector = _armed ? selector : 0;
	_frameIndex = 0;
	_lastFrameMillis = g_system->getMillis(true);
	if (_armed) {
		_announcedSelectors[_selector] = true;
		debugC(2, kDebugSaveLoad,
			"Ripper: restored armed briefing trigger selector=%u control=0x4e1",
			_selector);
	}
}

void BriefingManager::clear() {
	if (!_armed)
		return;
	_engine->getMedia()->stopSoundEffect(_alertHandle);
	debugC(1, kDebugScene,
		"Ripper: cleared briefing trigger selector=%u control=0x4e1", _selector);
	_armed = false;
	_selector = 0;
	_frameIndex = 0;
}

void BriefingManager::advanceAnimation(uint32 now) {
	if (!_armed || now < _lastFrameMillis + kBriefingFrameInterval)
		return;

	_frameIndex = (_frameIndex + 1) % _frames.frames.size();
	_lastFrameMillis = now;
	if (_frameIndex == 0) {
		_engine->getMedia()->playSoundEffect("wacicon1.wav", _alertHandle,
			kBriefingAlertVolume);
		debugC(2, kDebugAudio,
			"Ripper: briefing trigger selector=%u animation wrapped alert='wacicon1.wav' volume=%u",
			_selector, kBriefingAlertVolume);
	}
	debugC(3, kDebugScene,
		"Ripper: briefing trigger selector=%u frame=%u control=0x4e1",
		_selector, _frameIndex);
}

bool BriefingManager::service(const MouseState &mouse) {
	if (!_armed)
		return false;
	advanceAnimation(g_system->getMillis(true));
	draw();
	if (!_bounds.contains(mouse.position))
		return false;

	_engine->getCursor()->setVisible(true);
	_engine->getCursor()->update(kBriefingCursor);
	return true;
}

void BriefingManager::draw() const {
	if (!_armed || _frames.frames.empty())
		return;

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	const BitmapAssetFrame &frame = _frames.frames[_frameIndex];
	for (uint row = 0; row < frame.height; ++row) {
		byte *destination = (byte *)screen->getBasePtr(_bounds.left, _bounds.top + row);
		const byte *source = frame.pixels.data() + row * frame.width;
		for (uint column = 0; column < frame.width; ++column) {
			if (source[column] != frame.transparentColor)
				destination[column] = source[column];
		}
	}
	g_system->unlockScreen();
}

} // End of namespace Ripper
