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
	kOverlayRed = 4
};

const byte kFadeDither[16] = {
	0,  8,  2, 10,
	12, 4, 14,  6,
	3, 11,  1,  9,
	15, 7, 13,  5
};

} // End of anonymous namespace

MilestoneOverlayQueue::MilestoneOverlayQueue() :
		_enabled(false), _active(false), _startedAt(0) {
}

void MilestoneOverlayQueue::setEnabled(bool enabled) {
	if (_enabled == enabled)
		return;

	_enabled = enabled;
	_active = false;
	_startedAt = 0;
	_pending.clear();
	_message.clear();
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
	if (!_enabled)
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
		_milestones(milestones), _messageWidth(0), _messageHeight(0), _drawn(false) {
	_milestones.setChangeObserver(this);
}

MilestoneOverlay::~MilestoneOverlay() {
	_milestones.setChangeObserver(nullptr);
}

void MilestoneOverlay::setEnabled(bool enabled) {
	if (isEnabled() == enabled)
		return;

	prepareFrame();
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

void MilestoneOverlay::prepareFrame() {
	if (!_drawn)
		return;

	g_system->copyRectToScreen(_backing.data(), _bounds.width(), _bounds.left,
		_bounds.top, _bounds.width(), _bounds.height());
	_backing.clear();
	_drawn = false;
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

void MilestoneOverlay::drawFrame() {
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
	if (!_queue.hasActiveNotification())
		return;
	if (_renderedMessage != _queue.message() && !rebuildMessageSurface()) {
		warning("Ripper: could not prepare milestone overlay text");
		return;
	}

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 ||
			_bounds.right > screen->w || _bounds.bottom > screen->h) {
		if (screen)
			g_system->unlockScreen();
		warning("Ripper: milestone overlay requires the 640x400 indexed display");
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
	_drawn = true;
}

} // End of namespace Ripper
