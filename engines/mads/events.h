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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef MADS_EVENTS_H
#define MADS_EVENTS_H

#include "common/scummsys.h"
#include "common/events.h"
#include "common/stack.h"
#include "mads/assets.h"
#include "mads/sprites.h"

namespace MADS {

enum CursorType { CURSOR_NONE = 0, CURSOR_ARROW = 1, CURSOR_WAIT = 2, CURSOR_GO_DOWN = 3,
	CURSOR_GO_UP = 4, CURSOR_GO_LEFT = 5, CURSOR_GO_RIGHT = 6 };

#define GAME_FRAME_RATE 50
#define GAME_FRAME_TIME (1000 / GAME_FRAME_RATE)

class MADSEngine;

class EventTarget {
public:
	virtual ~EventTarget() {}
	virtual bool onEvent(Common::Event &event) { return false; }
};

class EventsManager {
private:
	MADSEngine *_vm;
	uint32 _frameCounter;
	uint32 _priorFrameTime;
	Common::Point _mousePos;
	Common::Point _currentPos;
	EventTarget *_eventTarget;

	/**
	 * Updates the cursor image when the current cursor changes
	 */
	void changeCursor();
public:
	SpriteAsset *_cursorSprites;
	CursorType _cursorId;
	CursorType _newCursorId;
	bool _mouseClicked;
	bool _mouseReleased;
	byte _mouseButtons;
	bool _rightMousePressed;
	int _mouseStatus;
	int _strokeGoing;
	int _mouseStatusCopy;
	bool _mouseMoved;
	Common::Stack<Common::KeyState> _pendingKeys;
public:
	/**
	 * Constructor
	 */
	EventsManager(MADSEngine *vm);

	/**
	 * Destructor
	 */
	~EventsManager();

	/**
	 * Loads the sprite set containing the cursors
	 */
	void loadCursors(const Common::String &spritesName);

	/**
	 * Sets the cursor
	 */
	void setCursor(CursorType cursorId);

	/**
	 * Sets the cursor
	 */
	void setCursor2(CursorType cursorId);

	/**
	 * Show the mouse cursor
	 */
	void showCursor();

	/**
	 * Hide the mouse cursor
	 */
	void hideCursor();

	/**
	 * Returns if the mouse cursor is visible
	 */
	bool isCursorVisible();

	/**
	 * Shows the wait cursor
	 */
	void waitCursor();

	/**
	 * Free currently loaded cursors
	 */
	void freeCursors();

	/**
	 * Poll any pending events
	 */
	void pollEvents();

	/**
	 * Sets an event handler other than the events manager
	 */
	void setEventTarget(EventTarget *target) { _eventTarget = target; }

	/**
	 * Return the current mouse position
	 */
	Common::Point mousePos() const { return _mousePos; }

	/**
	* Return the current mouse position
	*/
	Common::Point currentPos() const { return _currentPos; }

	/**
	 * Delay for a given number of frames
	 */
	void delay(int amount);

	/**
	 * Wait for the next frame
	 */
	void waitForNextFrame();

	/**
	* Checks for whether the next game frame number has been reached.
	*/
	bool checkForNextFrameCounter();

	/**
	 * Gets the current frame counter
	 */
	uint32 getFrameCounter() const { return _frameCounter; }

	void initVars();

	/**
	 * Clears all currently pending keypresses
	 */
	void clearEvents();

	/**
	 * Returns true if there's any pending keys to be processed
	 */
	bool isKeyPressed() const { return !_pendingKeys.empty(); }

	/**
	 * Gets the next pending keypress
	 */
	Common::KeyState getKey() { return _pendingKeys.pop(); }
};

} // End of namespace MADS

#endif /* MADS_EVENTS_H */
