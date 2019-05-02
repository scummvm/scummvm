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

#ifndef SLUDGE_EVENT_H
#define SLUDGE_EVENT_H

#include "common/stream.h"

namespace Sludge {

struct FrozenStuffStruct;

struct InputType {
	bool leftClick, rightClick, justMoved, leftRelease, rightRelease;
	int mouseX, mouseY, keyPressed;
};

enum EventFunctions {
	kLeftMouse,
	kLeftMouseUp,
	kRightMouse,
	kRightMouseUp,
	kMoveMouse,
	kFocus,
	kSpace,
	EVENT_FUNC_NB
};

struct EventHandlers {
	int func[EVENT_FUNC_NB];
};

class SludgeEngine;

class EventManager {
public:
	EventManager(SludgeEngine *vm);
	virtual ~EventManager();

	void init();
	void kill();

	// Input
	void checkInput();
	bool handleInput();

	int mouseX() const { return _input.mouseX; }
	int mouseY() const { return _input.mouseY; }
	int &mouseX() { return _input.mouseX; }
	int &mouseY() { return _input.mouseY; }

	// Events
	void setEventFunction(EventFunctions event, int funcNum) { _currentEvents->func[event] = funcNum; };
	void loadHandlers(Common::SeekableReadStream *stream);
	void saveHandlers(Common::WriteStream *stream);
	bool freeze(FrozenStuffStruct *frozenStuff);
	void restore(FrozenStuffStruct *frozenStuff);

	// Quit
	void startGame() { _weAreDoneSoQuit = false; }
	void quitGame() { _weAreDoneSoQuit = true; /* _reallyWantToQuit = true; */ }
	bool quit() { return _weAreDoneSoQuit; }

private:
	SludgeEngine *_vm;
	InputType _input;

	int _weAreDoneSoQuit;
	bool _reallyWantToQuit;

	EventHandlers *_currentEvents;
};

} /* namespace Sludge */

#endif /* ENGINES_SLUDGE_EVENT_H_ */
