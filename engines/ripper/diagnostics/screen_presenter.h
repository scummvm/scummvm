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

#ifndef RIPPER_DIAGNOSTICS_SCREEN_PRESENTER_H
#define RIPPER_DIAGNOSTICS_SCREEN_PRESENTER_H

#include "common/array.h"
#include "common/queue.h"
#include "common/rect.h"
#include "common/str.h"

namespace Ripper {

class ScreenMessageQueue {
public:
	enum {
		kLifetimeMs = 5000,
		kFadeMs = 250
	};

	ScreenMessageQueue();

	void enqueue(const Common::String &message);
	void clear();
	void pause(uint32 now);
	void resume(uint32 now);
	bool update(uint32 now);
	byte opacity(uint32 now) const;

	bool hasActiveMessage() const { return _active; }
	const Common::String &message() const { return _message; }
	uint pendingCount() const { return (uint)_pending.size(); }

private:
	void startNext(uint32 now);

	bool _active;
	bool _paused;
	uint32 _startedAt;
	uint32 _pausedAt;
	Common::Queue<Common::String> _pending;
	Common::String _message;
};

class ScreenPresenter {
public:
	ScreenPresenter();
	~ScreenPresenter();

	void showMessage(const Common::String &message);
	void clearMessages();
	void pause(bool paused);
	void presentScreen();
	uint pendingMessageCount() const { return _queue.pendingCount(); }

private:
	bool rebuildMessageSurface();
	void resetMessageSurface();

	ScreenMessageQueue _queue;
	Common::Rect _bounds;
	Common::Array<byte> _backing;
	Common::Array<byte> _messagePixels;
	Common::String _renderedMessage;
	int _messageWidth;
	int _messageHeight;
};

void showScreenMessage(const Common::String &message);
void presentScreen();

} // End of namespace Ripper

#endif // RIPPER_DIAGNOSTICS_SCREEN_PRESENTER_H
