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
	Window(BuriedEngine *vm, Window *parent, bool visible = true);
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
	void invalidateWindow(bool erase = true) { invalidateRect(_rect, erase); }
	Window *getParent() const { return _parent; }
	const Common::Rect &getRect() const { return _rect; }
	Common::Rect getClientRect() const;
	Common::Rect getAbsoluteRect() const;
	void updateWindow();
	void enableWindow(bool enable);
	bool isWindowEnabled() const;
	void setWindowPos(Window *insertAfter, int x, int y, int width, int height, uint flags);

	// The subset of show modes we'll accept
	enum WindowShowMode {
		kWindowShow,
		kWindowHide,
		kWindowShowNormal
	};

	void showWindow(WindowShowMode showMode);
	bool isWindowVisible() const { return _visible; }

	// The subset of flags we'll accept
	enum WindowPosFlags {
		kWindowPosNoFlags = 0,

		kWindowPosNoSize = (1 << 0),
		kWindowPosNoZOrder = (1 << 1),
		kWindowPosHideWindow = (1 << 2),
		kWindowPosShowWindow = (1 << 3),
		kWindowPosNoMove = (1 << 4),
		kWindowPosNoActivate = (1 << 5)
	};

	Window *setFocus();

	// TODO:
	// ShowWindow
	// BeginPaint (?)
	// EndPaint (?)
	// Create
	// ...

	void sendMessage(Message *message);
	void postMessage(Message *message);

	Window *findWindowAtPoint(const Common::Point &point);

protected:
	BuriedEngine *_vm;

	Window *_parent;
	Common::Rect _rect;

	uint setTimer(uint elapse);
	bool killTimer(uint timer);

	Common::Rect makeAbsoluteRect(const Common::Rect &rect) const;

private:
	bool _enabled, _visible;
	bool _needsErase;

	typedef Common::List<Window *> WindowList;
	WindowList _children, _topMostChildren;
};

// A subset of the special insert after Window handles
// (Values declared in window.cpp)
extern const Window *kWindowPosTop;
extern const Window *kWindowPosTopMost;

} // End of namespace Buried

#endif
