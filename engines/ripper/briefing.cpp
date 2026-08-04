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
#include "ripper/milestones.h"
#include "ripper/ripper.h"

namespace Ripper {

static const int kBriefingRightEdge = 635;
static const int kBriefingSceneOriginY = 50;
static const int kBriefingControlOffsetY = 305;
static const uint kBriefingCursor = 16;
static const uint32 kDosTickMillis = 55;
static const uint32 kBriefingFrameInterval = 3 * kDosTickMillis;
static const uint kBriefingAlertVolume = 35;

static bool isImplementedBriefingSelector(uint selector) {
	return selector == 1 || selector == 2 || selector == 3 || selector == 4 ||
		selector == 5 || selector == 6;
}

BriefingManager::BriefingManager(RipperEngine *engine) : _engine(engine),
		_lastFrameMillis(0), _frameIndex(0), _selector(0), _armed(false),
		_initialized(false), _hovered(false) {
	memset(_announcedSelectors, 0, sizeof(_announcedSelectors));
}

BriefingManager::~BriefingManager() {
	_engine->getMedia()->stopSoundEffect(_alertHandle);
}

bool BriefingManager::initialize(ResourceManager &resources) {
	// LoadFrontEndToolbarResources at 0x18678 stores toolbar action four at
	// g_frontEndToolbarResources+0xc. ArmBriefingMediaTrigger at 0x1929a
	// reuses that same frame set for the lower-band message control.
	if (!resources.loadInterfaceBitmapSequence("toolbar4.pl", _frames) ||
			_frames.frames.empty())
		return false;

	const BitmapAssetFrame &frame = _frames.frames.front();
	if (frame.width == 0 || frame.height == 0 || frame.width > kBriefingRightEdge ||
			kBriefingSceneOriginY + kBriefingControlOffsetY + frame.height > 400)
		return false;
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
	if (!_initialized) {
		warning("Ripper: briefing selector %u requested before initialization", selector);
		return false;
	}
	if (!isImplementedBriefingSelector(selector)) {
		warning("Ripper: unsupported briefing selector %u", selector);
		return false;
	}
	if (_announcedSelectors[selector]) {
		debugC(2, kDebugScene,
			"Ripper: briefing selector=%u already announced; trigger not rearmed", selector);
		return true;
	}
	if (_armed)
		clear();

	_announcedSelectors[selector] = true;
	_selector = selector;
	_frameIndex = 0;
	_lastFrameMillis = g_system->getMillis(true);
	_armed = true;
	_hovered = false;
	draw();
	g_system->updateScreen();
	debugC(1, kDebugScene,
		"Ripper: armed briefing trigger selector=%u control=0x4e1 frame=%u rect=%d,%d,%d,%d",
		_selector, _frameIndex, _bounds.left, _bounds.top, _bounds.width(), _bounds.height());

	if (playNotification && !_engine->getMedia()->playBlockingAudio("wacicon0.wav")) {
		clear();
		return false;
	}
	return true;
}

bool BriefingManager::restore(bool armed, uint selector) {
	_engine->getMedia()->stopSoundEffect(_alertHandle);
	_backing.clear();
	if (armed && !isImplementedBriefingSelector(selector)) {
		warning("Ripper: cannot restore unsupported briefing selector %u", selector);
		_armed = false;
		_selector = 0;
		_frameIndex = 0;
		_hovered = false;
		return false;
	}
	_armed = armed && _initialized && selector != 0 &&
		selector < ARRAYSIZE(_announcedSelectors);
	_selector = _armed ? selector : 0;
	_frameIndex = 0;
	_lastFrameMillis = g_system->getMillis(true);
	_hovered = false;
	if (_armed) {
		_announcedSelectors[_selector] = true;
		debugC(2, kDebugSaveLoad,
			"Ripper: restored armed briefing trigger selector=%u control=0x4e1",
			_selector);
	}
	return true;
}

void BriefingManager::clear() {
	if (!_armed)
		return;
	const bool alertStopped = _engine->getMedia()->stopSoundEffect(_alertHandle);
	debugC(2, kDebugAudio,
		"Ripper: briefing trigger stopped alert='wacicon1.wav' selector=%u active=%d",
		_selector, alertStopped);
	restoreBacking();
	debugC(1, kDebugScene,
		"Ripper: cleared briefing trigger selector=%u control=0x4e1", _selector);
	_armed = false;
	_selector = 0;
	_frameIndex = 0;
	_hovered = false;
}

void BriefingManager::prepareForSceneTransition() {
	if (!_armed)
		return;
	// A briefing armed by WMAP*.RUN survives the following scene handoff.
	// Discard its map-screen backing so the first destination frame captures
	// the pixels that should be restored when the player activates the icon.
	_backing.clear();
	debugC(3, kDebugScene,
		"Ripper: discarded briefing backing before scene transition selector=%u",
		_selector);
}

void BriefingManager::captureBacking() {
	if (!_backing.empty())
		return;
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		_backing.clear();
		return;
	}
	_backing.resize(_bounds.width() * _bounds.height());
	for (int row = 0; row < _bounds.height(); ++row) {
		memcpy(_backing.data() + row * _bounds.width(),
			screen->getBasePtr(_bounds.left, _bounds.top + row), _bounds.width());
	}
	g_system->unlockScreen();
}

void BriefingManager::restoreBacking() {
	if (_backing.size() != (uint)(_bounds.width() * _bounds.height()))
		return;
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		_backing.clear();
		return;
	}
	for (int row = 0; row < _bounds.height(); ++row) {
		memcpy(screen->getBasePtr(_bounds.left, _bounds.top + row),
			_backing.data() + row * _bounds.width(), _bounds.width());
	}
	g_system->unlockScreen();
	_backing.clear();
	g_system->updateScreen();
}

bool BriefingManager::activate() {
	const uint selector = _selector;
	clear();
	_engine->getCursor()->setVisible(false);
	debugC(1, kDebugScene,
		"Ripper: activated briefing trigger selector=%u control=0x4e1", selector);

	bool result = true;
	switch (selector) {
	case 1:
		result = _engine->getMedia()->play("cp0_1_p1.avi", true, 0, 0);
		if (result) {
			result = _engine->getMilestones()->set(kMilestoneLastTravelLocation, true,
				"briefing selector 1");
			debugC(1, kDebugScene,
				"Ripper: completed briefing selector=1 media='cp0_1_p1.avi' travelFlag=%u",
				(uint)kMilestoneLastTravelLocation);
		}
		break;
	case 2:
		// ServiceBriefingMediaTrigger at 0x1945b clears selector 2 without
		// presenting media or changing milestone state.
		debugC(1, kDebugScene,
			"Ripper: completed briefing selector=2 without media or state changes");
		break;
	case 3:
		// The selector jump-table entry at 0x19624 presents RIP_WAC2.AVI
		// without changing milestone state.
		result = _engine->getMedia()->play("rip_wac2.avi", true, 0, 0);
		if (result) {
			debugC(1, kDebugScene,
				"Ripper: completed briefing selector=3 media='rip_wac2.avi' without state changes");
		}
		break;
	case 4:
		result = _engine->getMedia()->play("sj_wacm.avi", true, 0, 0);
		if (result)
			result = _engine->getMilestones()->set(
				kMilestoneReceivedJordanWacMessage, true, "briefing selector 4");
		if (result)
			result = _engine->getMilestones()->set(
				kMilestoneWebRunnersLoftOpen, true, "briefing selector 4");
		if (result) {
			debugC(1, kDebugScene,
				"Ripper: completed briefing selector=4 media='sj_wacm.avi' "
				"messageFlag=%u travelFlag=%u",
				(uint)kMilestoneReceivedJordanWacMessage,
				(uint)kMilestoneWebRunnersLoftOpen);
		}
		break;
	case 5:
		// ServiceBriefingMediaTrigger at 0x1945b presents VF2_2_P1.AVI,
		// records milestone 0x133, and opens travel location 0x14.
		result = _engine->getMedia()->play("vf2_2_p1.avi", true, 0, 0);
		if (result)
			result = _engine->getMilestones()->set(
				kMilestoneGotFarleyWacMessage, true, "briefing selector 5");
		if (result)
			result = _engine->getMilestones()->set(
				kMilestoneFirstTravelLocation, true, "briefing selector 5");
		if (result) {
			debugC(1, kDebugScene,
				"Ripper: completed briefing selector=5 media='vf2_2_p1.avi' "
				"messageFlag=%u travelFlag=%u",
				(uint)kMilestoneGotFarleyWacMessage,
				(uint)kMilestoneFirstTravelLocation);
		}
		break;
	case 6:
		// ServiceBriefingMediaTrigger at 0x1945b presents RIP_WAC3.AVI
		// and records milestone 0x13a after the presentation returns.
		result = _engine->getMedia()->play("rip_wac3.avi", true, 0, 0);
		if (result)
			result = _engine->getMilestones()->set(
				kMilestonePlayedThirdRipperWacMessage, true, "briefing selector 6");
		if (result) {
			debugC(1, kDebugScene,
				"Ripper: completed briefing selector=6 media='rip_wac3.avi' messageFlag=%u",
				(uint)kMilestonePlayedThirdRipperWacMessage);
		}
		break;
	default:
		warning("Ripper: unsupported briefing selector %u", selector);
		result = false;
		break;
	}
	_engine->getInput()->discardMouseTransitions();
	return result;
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

BriefingServiceResult BriefingManager::service(const MouseState &mouse) {
	if (!_armed)
		return kBriefingIdle;
	advanceAnimation(g_system->getMillis(true));
	draw();
	const bool hovered = _bounds.contains(mouse.position);
	if (hovered != _hovered) {
		_hovered = hovered;
		debugC(2, kDebugScene,
			"Ripper: briefing trigger hover=%d selector=%u point=%d,%d control=0x4e1",
			_hovered, _selector, mouse.position.x, mouse.position.y);
	}
	if (!hovered)
		return kBriefingIdle;

	_engine->getCursor()->setVisible(true);
	_engine->getCursor()->update(kBriefingCursor);
	if ((mouse.pressed & kMouseButtonLeft) == 0)
		return kBriefingHovered;
	return activate() ? kBriefingActivated : kBriefingFailed;
}

void BriefingManager::draw() {
	if (!_armed || _frames.frames.empty())
		return;
	captureBacking();

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
