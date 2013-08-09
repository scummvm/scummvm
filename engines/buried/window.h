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

#include "common/rect.h"
#include "common/queue.h"

namespace Common {
struct KeyState;
}

namespace Buried {

class BuriedEngine;
struct Message;

class Window {
public:
	Window(BuriedEngine *vm);
	virtual ~Window();

	// The message types used by Buried in Time's windows
	virtual bool onEraseBackground() { return false; }
	virtual void onKeyDown(const Common::KeyState &key, uint flags) {}
	virtual void onKeyUp(const Common::KeyState &key, uint flags) {}
	virtual void onTimer(uint timer) {}
	virtual void onKillFocus(Window *newWindow) {}
	virtual void onSetFocus(Window *oldWindow) {}
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

	void invalidateRect(const Common::Rect &rect, bool erase = true);
	void createChild(const Common::Rect &rect, Window *parent);
	Window *getParent() const { return _parent; }
	const Common::Rect &getRect() const { return _rect; }
	Common::Rect getClientRect() const;
	void updateWindow() { onPaint(); }
	void enableWindow(bool enable);
	bool isWindowEnabled() const;

	// TODO:
	// SetTimer
	// ShowWindow
	// KillTimer
	// BeginPaint (?)
	// EndPaint (?)
	// Create
	// ...

	void sendMessage(Message *message) { _queue.push(message); }
	void dispatchAllMessages();

protected:
	BuriedEngine *_vm;

	Window *_parent;
	Common::Rect _rect;

private:
	Common::Queue<Message *> _queue;
	bool _enabled;
};

} // End of namespace Buried

#endif
