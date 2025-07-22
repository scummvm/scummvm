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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SHERLOCK_SETTINGS_H
#define SHERLOCK_SETTINGS_H

#include "common/events.h"
#include "common/scummsys.h"

#include "sherlock/sherlock.h"

namespace Sherlock {

class SherlockEngine;

namespace Scalpel {

class Settings {
private:
	SherlockEngine *_vm;

	Settings(SherlockEngine *vm) : _vm(vm) {
		_actionExit = kActionNone;
		_actionMusic = kActionNone;
		_actionPortraits = kActionNone;
		_actionNewFontStyle = kActionNone;
		_actionSoundEffects = kActionNone;
		_actionWindows = kActionNone;
		_actionAutoHelp = kActionNone;
		_actionVoices = kActionNone;
		_actionFade = kActionNone;

		memset(_actionsIndexed, kActionNone, sizeof(_actionsIndexed));
	}

	Common::CustomEventType _actionExit;
	Common::CustomEventType _actionMusic;
	Common::CustomEventType _actionPortraits;
	Common::CustomEventType _actionNewFontStyle;
	Common::CustomEventType _actionSoundEffects;
	Common::CustomEventType _actionWindows;
	Common::CustomEventType _actionAutoHelp;
	Common::CustomEventType _actionVoices;
	Common::CustomEventType _actionFade;

	Common::CustomEventType _actionsIndexed[12];

	/**
	 * Draws the interface for the settings window
	 */
	void drawInterface(bool flag);

	/**
	 * Draws the buttons for the settings dialog
	 */
	int drawButtons(const Common::Point &pt, Common::CustomEventType action);

	Common::Rect getButtonRect(int num) const;
	Common::Point getButtonTextPoint(int num) const;
	void makeButtonNum(int num, const Common::String &s);
	void makeButtonNumDisabled(int num, const Common::String &s);
	bool doesButtonExist(int num) const;

public:
	/**
	 * Handles input when the settings window is being shown
	 * @remarks		Whilst this would in theory be better in the Journal class, since it displays in
	 *		the user interface, it uses so many internal UI fields, that it sort of made some sense
	 *		to put it in the UserInterface class.
	 */
	static void show(SherlockEngine *vm);
};

} // End of namespace Scalpel

} // End of namespace Sherlock

#endif
