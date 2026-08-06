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

#ifndef RIPPER_DIAGNOSTICS_MILESTONE_OVERLAY_H
#define RIPPER_DIAGNOSTICS_MILESTONE_OVERLAY_H

#include "common/array.h"
#include "common/queue.h"
#include "common/rect.h"
#include "common/str.h"

#include "ripper/diagnostics/screen_presenter.h"
#include "ripper/milestones.h"

namespace Ripper {

class MilestoneOverlayQueue {
public:
	enum {
		kLifetimeMs = 5000,
		kFadeMs = 250
	};

	MilestoneOverlayQueue();

	void setEnabled(bool enabled);
	bool isEnabled() const { return _enabled; }
	void pause(uint32 now);
	void resume(uint32 now);
	void enqueue(uint flag, const Common::String &label, bool value);
	bool update(uint32 now);
	byte opacity(uint32 now) const;

	bool hasActiveNotification() const { return _active; }
	const Common::String &message() const { return _message; }
	uint pendingCount() const { return (uint)_pending.size(); }

private:
	struct Notification {
		uint flag;
		Common::String label;
		bool value;
	};

	void startNext(uint32 now);

	bool _enabled;
	bool _active;
	bool _paused;
	uint32 _startedAt;
	uint32 _pausedAt;
	Common::Queue<Notification> _pending;
	Common::String _message;
};

class MilestoneOverlay : public MilestoneObserver, public ScreenPresenter {
public:
	explicit MilestoneOverlay(Milestones &milestones);
	~MilestoneOverlay() override;

	void setEnabled(bool enabled);
	bool isEnabled() const { return _queue.isEnabled(); }
	void pause(bool paused);
	void presentScreen() override;

	void onMilestoneChanged(uint flag, bool value) override;

private:
	bool rebuildMessageSurface();
	void resetMessageSurface();

	Milestones &_milestones;
	MilestoneOverlayQueue _queue;
	Common::Rect _bounds;
	Common::Array<byte> _backing;
	Common::Array<byte> _messagePixels;
	Common::String _renderedMessage;
	int _messageWidth;
	int _messageHeight;
};

} // End of namespace Ripper

#endif // RIPPER_DIAGNOSTICS_MILESTONE_OVERLAY_H
