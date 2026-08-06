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

#include "ripper/diagnostics/milestone_overlay.h"

#include "common/debug.h"
#include "common/system.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "ripper/detection.h"
#include "ripper/display.h"

namespace Ripper {

namespace {

enum {
	kOverlayMargin = 8,
	kOverlayPadding = 4,
	kOverlayBlack = 0,
	kOverlayRed = 254
};

const byte kFadeDither[16] = {
	0,  8,  2, 10,
	12, 4, 14,  6,
	3, 11,  1,  9,
	15, 7, 13,  5
};

} // End of anonymous namespace

MilestoneOverlayQueue::MilestoneOverlayQueue() :
		_enabled(false), _active(false), _paused(false), _startedAt(0), _pausedAt(0) {
}

void MilestoneOverlayQueue::setEnabled(bool enabled) {
	if (_enabled == enabled)
		return;

	_enabled = enabled;
	_active = false;
	_paused = false;
	_startedAt = 0;
	_pausedAt = 0;
	_pending.clear();
	_message.clear();
}

void MilestoneOverlayQueue::pause(uint32 now) {
	if (!_enabled || _paused)
		return;
	_paused = true;
	_pausedAt = now;
}

void MilestoneOverlayQueue::resume(uint32 now) {
	if (!_paused)
		return;
	if (_active)
		_startedAt += now - _pausedAt;
	_paused = false;
	_pausedAt = 0;
}

void MilestoneOverlayQueue::enqueue(uint flag, const Common::String &label, bool value) {
	if (!_enabled)
		return;

	Notification notification;
	notification.flag = flag;
	notification.label = label.empty() ? "<unnamed>" : label;
	notification.value = value;
	_pending.push(notification);
}

void MilestoneOverlayQueue::startNext(uint32 now) {
	const Notification notification = _pending.pop();
	_message = Common::String::format("Milestone %u: %s [%s]", notification.flag,
		notification.label.c_str(), notification.value ? "set" : "unset");
	_startedAt = now;
	_active = true;
}

bool MilestoneOverlayQueue::update(uint32 now) {
	if (!_enabled || _paused)
		return false;

	bool changed = false;
	if (_active && now - _startedAt >= kLifetimeMs) {
		_active = false;
		_message.clear();
		changed = true;
	}
	if (!_active && !_pending.empty()) {
		startNext(now);
		changed = true;
	}
	return changed;
}

byte MilestoneOverlayQueue::opacity(uint32 now) const {
	if (!_active)
		return 0;

	if (_paused)
		now = _pausedAt;
	const uint32 elapsed = now - _startedAt;
	if (elapsed >= kLifetimeMs)
		return 0;
	if (elapsed < kFadeMs)
		return (byte)(elapsed * 255 / kFadeMs);
	const uint32 remaining = kLifetimeMs - elapsed;
	if (remaining < kFadeMs)
		return (byte)(remaining * 255 / kFadeMs);
	return 255;
}

MilestoneOverlay::MilestoneOverlay(Milestones &milestones) :
		_milestones(milestones), _messageWidth(0), _messageHeight(0) {
	_milestones.setChangeObserver(this);
	registerScreenPresenter(this);
}

MilestoneOverlay::~MilestoneOverlay() {
	unregisterScreenPresenter(this);
	_milestones.setChangeObserver(nullptr);
}

void MilestoneOverlay::setEnabled(bool enabled) {
	if (isEnabled() == enabled)
		return;

	_queue.setEnabled(enabled);
	resetMessageSurface();
	debugC(1, kDebugMilestones,
		"Ripper: milestone change overlay enabled=%d command=OVERLAY_MILESTONES",
		enabled);
}

void MilestoneOverlay::onMilestoneChanged(uint flag, bool value) {
	if (!isEnabled())
		return;

	_queue.enqueue(flag, _milestones.label(flag), value);
	debugC(2, kDebugMilestones,
		"Ripper: queued milestone overlay flag=%u label='%s' value=%d pending=%u",
		flag, _milestones.label(flag).c_str(), value, _queue.pendingCount());
}

void MilestoneOverlay::pause(bool paused) {
	if (paused)
		_queue.pause(g_system->getMillis());
	else
		_queue.resume(g_system->getMillis());
}

void MilestoneOverlay::resetMessageSurface() {
	_bounds = Common::Rect();
	_messagePixels.clear();
	_renderedMessage.clear();
	_messageWidth = 0;
	_messageHeight = 0;
}

bool MilestoneOverlay::rebuildMessageSurface() {
	const Graphics::Font *font =
		FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);
	if (!font)
		return false;

	_renderedMessage = _queue.message();
	_messageWidth = MIN<int>(kRipperScreenWidth - kOverlayMargin * 2,
		font->getStringWidth(_renderedMessage) + kOverlayPadding * 2);
	_messageHeight = font->getFontHeight() + kOverlayPadding * 2;
	_bounds = Common::Rect(kOverlayMargin,
		kRipperScreenHeight - kOverlayMargin - _messageHeight,
		kOverlayMargin + _messageWidth, kRipperScreenHeight - kOverlayMargin);
	_messagePixels.resize(_messageWidth * _messageHeight);
	memset(_messagePixels.data(), kOverlayBlack, _messagePixels.size());

	Graphics::Surface surface;
	surface.init(_messageWidth, _messageHeight, _messageWidth,
		_messagePixels.data(), Graphics::PixelFormat::createFormatCLUT8());
	font->drawString(&surface, _renderedMessage, kOverlayPadding, kOverlayPadding,
		_messageWidth - kOverlayPadding * 2, kOverlayRed);
	return true;
}

void MilestoneOverlay::presentScreen() {
	const uint32 now = g_system->getMillis();
	const bool notificationChanged = _queue.update(now);
	if (notificationChanged) {
		resetMessageSurface();
		if (_queue.hasActiveNotification()) {
			debugC(2, kDebugMilestones,
				"Ripper: presenting milestone overlay message='%s' durationMs=%u queued=%u",
				_queue.message().c_str(), MilestoneOverlayQueue::kLifetimeMs,
				_queue.pendingCount());
		} else {
			debugC(2, kDebugMilestones, "Ripper: milestone overlay queue drained");
		}
	}
	if (!_queue.hasActiveNotification()) {
		g_system->updateScreen();
		return;
	}
	if (_renderedMessage != _queue.message() && !rebuildMessageSurface()) {
		warning("Ripper: could not prepare milestone overlay text");
		g_system->updateScreen();
		return;
	}

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 ||
			_bounds.right > screen->w || _bounds.bottom > screen->h) {
		if (screen)
			g_system->unlockScreen();
		warning("Ripper: milestone overlay requires the 640x400 indexed display");
		g_system->updateScreen();
		return;
	}

	_backing.resize(_messageWidth * _messageHeight);
	for (int y = 0; y < _messageHeight; ++y) {
		memcpy(_backing.data() + y * _messageWidth,
			screen->getBasePtr(_bounds.left, _bounds.top + y), _messageWidth);
	}

	const uint coverage = (_queue.opacity(now) * 16 + 254) / 255;
	for (int y = 0; y < _messageHeight; ++y) {
		byte *destination = (byte *)screen->getBasePtr(_bounds.left, _bounds.top + y);
		const byte *source = _messagePixels.data() + y * _messageWidth;
		for (int x = 0; x < _messageWidth; ++x) {
			if (kFadeDither[(y & 3) * 4 + (x & 3)] < coverage)
				destination[x] = source[x];
		}
	}
	g_system->unlockScreen();

	g_system->updateScreen();

	screen = g_system->lockScreen();
	if (screen && screen->format.bytesPerPixel == 1 &&
			_bounds.right <= screen->w && _bounds.bottom <= screen->h) {
		for (int y = 0; y < _messageHeight; ++y) {
			memcpy(screen->getBasePtr(_bounds.left, _bounds.top + y),
				_backing.data() + y * _messageWidth, _messageWidth);
		}
	}
	if (screen)
		g_system->unlockScreen();
}

} // End of namespace Ripper
