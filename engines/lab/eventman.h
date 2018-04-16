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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#ifndef LAB_EVENTMAN_H
#define LAB_EVENTMAN_H

#include "common/events.h"

namespace Lab {

class LabEngine;
class Image;

struct IntuiMessage {
	MessageClass _msgClass;
	uint16 _code; // KeyCode or Button Id
	uint16 _qualifier;
	Common::Point _mouse;
};

class EventManager {
private:
	LabEngine *_vm;

	bool _leftClick;
	bool _rightClick;
	bool _buttonHit;

	Common::Point _mousePos;
	Common::KeyState _keyPressed;

public:
	EventManager (LabEngine *vm);

	IntuiMessage *getMsg();

	/**
	 * Initializes the mouse.
	 */
	void initMouse();

	/**
	 * Shows the mouse.
	 */
	void mouseShow();

	/**
	 * Hides the mouse.
	 */
	void mouseHide();
	void processInput();

	/**
	 * Moves the mouse to new co-ordinates.
	 */
	void setMousePos(Common::Point pos);
	Common::Point updateAndGetMousePos();

	/**
	 * Simulates an event for the game main loop, when a game is
	 * loaded or when the user teleports to a scene
	 */
	void simulateEvent();
};

} // End of namespace Lab

#endif // LAB_EVENTMAN_H
