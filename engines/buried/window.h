/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BURIED_WINDOW_H
#define BURIED_WINDOW_H

#include "common/queue.h"

namespace Common {
struct KeyState;
struct Point;
}

namespace Buried {

struct Message;

class Window {
public:
	virtual ~Window() {}

	// The message types used by Buried in Time's windows
	virtual bool onEraseBackground() { return false; }
	virtual void onKeyDown(const Common::KeyState &key, uint flags) {}
	virtual void onKeyUp(const Common::KeyState &key, uint flags) {}
	virtual void onTimer(uint timer) {}
	virtual void onKillFocus(Window *newWindow) {}
	virtual void onSetFocus(Window *oldWindow) {}
	virtual bool onQueryNewPalette() { return false; }
	virtual void onPaint() {}
	virtual void onLButtonUp(const Common::Point &point, uint flags) {}
	virtual void onLButtonDown(const Common::Point &point, uint flags) {}
	virtual void onLButtonDoubleClick(const Common::Point &point, uint flags) {}
	virtual void onMouseMove(const Common::Point &point, uint flags) {}
	virtual void onMButtonUp(const Common::Point &point, uint flags) {}
	virtual void onRButtonUp(const Common::Point &point, uint flags) {}
	virtual void onRButtonDown(const Common::Point &point, uint flags) {}
	virtual bool onSetCursor(Window *window, uint cursor) { return false; }
	virtual void onEnable(bool enable) {}

	// TODO:
	// SetTimer
	// ShowWindow
	// UpdateWindow
	// GetClientRect
	// KillTimer
	// InvalidateRect
	// BeginPaint (?)
	// EndPaint (?)
	// Create
	// GetParent
	// ...

	void sendMessage(Message *message) { _queue.push(message); }
	void dispatchMessage();

private:
	Common::Queue<Message *> _queue;

	// TODO: Something about children? Parents?
};

} // End of namespace Buried

#endif
