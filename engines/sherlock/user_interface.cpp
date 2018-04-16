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

#include "sherlock/user_interface.h"
#include "sherlock/scalpel/scalpel.h"
#include "sherlock/scalpel/scalpel_user_interface.h"
#include "sherlock/tattoo/tattoo.h"
#include "sherlock/tattoo/tattoo_user_interface.h"

namespace Sherlock {

UserInterface *UserInterface::init(SherlockEngine *vm) {
	if (vm->getGameID() == GType_SerratedScalpel)
		return new Scalpel::ScalpelUserInterface(vm);
	else
		return new Tattoo::TattooUserInterface(vm);
}

UserInterface::UserInterface(SherlockEngine *vm) : _vm(vm) {
	_menuMode = STD_MODE;
	_menuCounter = 0;
	_infoFlag = false;
	_windowOpen = false;
	_endKeyActive = true;
	_invLookFlag = 0;
	_slideWindows = true;
	_helpStyle = false;
	_windowBounds = Common::Rect(0, CONTROLS_Y1, SHERLOCK_SCREEN_WIDTH - 1, SHERLOCK_SCREEN_HEIGHT - 1);
	_lookScriptFlag = false;
	_exitZone = -1;

	_bgFound = _oldBgFound = -1;
	_key = _oldKey = '\0';
	_selector = _oldSelector = -1;
	_temp = _oldTemp = 0;
	_temp1 = 0;
	_lookHelp = 0;
}

void UserInterface::checkAction(ActionType &action, int objNum, FixedTextActionId fixedTextActionId) {
	Events &events = *_vm->_events;
	FixedText &fixedText = *_vm->_fixedText;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	Point32 pt(-1, -1);

	if (action._useFlag)
		// Automatically set the given flag
		_vm->setFlags(action._useFlag);

	if (IS_SERRATED_SCALPEL && objNum >= 1000)
		// Ignore actions done on characters
		return;

	if (IS_SERRATED_SCALPEL && !action._cAnimSpeed) {
		// Invalid action, to print error message
		_infoFlag = true;
		clearInfo();
		Common::String errorMessage = fixedText.getActionMessage(fixedTextActionId, action._cAnimNum);
		screen.print(Common::Point(0, INFO_LINE + 1), COL_INFO_FOREGROUND, "%s", errorMessage.c_str());
		_infoFlag = true;

		// Set how long to show the message
		_menuCounter = 30;
	} else {
		BaseObject *obj;
		if (objNum >= 1000)
			obj = &people[objNum - 1000];
		else
			obj = &scene._bgShapes[objNum];

		int cAnimNum;
		if (action._cAnimNum == 0)
			// Really a 10
			cAnimNum = 9;
		else
			cAnimNum = action._cAnimNum - 1;

		int dir = -1;
		if (action._cAnimNum != 99) {
			CAnim &anim = scene._cAnim[cAnimNum];

			if (action._cAnimNum != 99) {
				if (action._cAnimSpeed & REVERSE_DIRECTION) {
					pt = anim._teleport[0];
					dir = anim._teleport[0]._facing;
				} else {
					pt = anim._goto[0];
					dir = anim._goto[0]._facing;
				}
			}
		} else {
			pt = Point32(-1, -1);
			dir = -1;
		}

		// Has a value, so do action
		// Show wait cursor whilst walking to object and doing action
		events.setCursor(WAIT);
		bool printed = false;

		for (int nameIdx = 0; nameIdx < NAMES_COUNT; ++nameIdx) {
			if (action._names[nameIdx].hasPrefix("*") && action._names[nameIdx].size() >= 2
					&& toupper(action._names[nameIdx][1]) == 'W') {
				if (obj->checkNameForCodes(Common::String(action._names[nameIdx].c_str() + 2), fixedTextActionId)) {
					if (!talk._talkToAbort)
						printed = true;
				}
			}
		}

		bool doCAnim = true;
		for (int nameIdx = 0; nameIdx < NAMES_COUNT; ++nameIdx) {
			if (action._names[nameIdx].hasPrefix("*") && action._names[nameIdx].size() >= 2) {
				char ch = toupper(action._names[nameIdx][1]);

				if (ch == 'T' || ch == 'B') {
					printed = true;
					if (pt.x != -1)
						// Holmes needs to walk to object before the action is done
						people[HOLMES].walkToCoords(pt, dir);

					if (!talk._talkToAbort) {
						// Ensure Holmes is on the exact intended location
						people[HOLMES]._position = pt;
						people[HOLMES]._sequenceNumber = dir;
						people[HOLMES].gotoStand();

						talk.talkTo(action._names[nameIdx].c_str() + 2);
						if (ch == 'T')
							doCAnim = false;
					}
				}
			}
		}

		if (doCAnim && !talk._talkToAbort) {
			if (pt.x != -1)
				// Holmes needs to walk to object before the action is done
				people[HOLMES].walkToCoords(pt, dir);
		}

		for (int nameIdx = 0; nameIdx < NAMES_COUNT; ++nameIdx) {
			if (action._names[nameIdx].hasPrefix("*") && action._names[nameIdx].size() >= 2
					&& toupper(action._names[nameIdx][1]) == 'F') {
				if (obj->checkNameForCodes(action._names[nameIdx].c_str() + 2, fixedTextActionId)) {
					if (!talk._talkToAbort)
						printed = true;
				}
			}
		}

		if (doCAnim && !talk._talkToAbort && action._cAnimNum != 99)
			scene.startCAnim(cAnimNum, action._cAnimSpeed);

		if (!talk._talkToAbort) {
			for (int nameIdx = 0; nameIdx < NAMES_COUNT && !talk._talkToAbort; ++nameIdx) {
				if (obj->checkNameForCodes(action._names[nameIdx], fixedTextActionId)) {
					if (!talk._talkToAbort)
						printed = true;
				}
			}

			// Unless we're leaving the scene, print a "Done" message unless the printed flag has been set
			if (IS_SERRATED_SCALPEL && scene._goToScene != 1 && !printed && !talk._talkToAbort) {
				_infoFlag = true;
				clearInfo();
				screen.print(Common::Point(0, INFO_LINE + 1), COL_INFO_FOREGROUND, "Done...");

				// Set how long to show the message
				_menuCounter = 30;
			}
		}
	}

	// Reset cursor back to arrow
	events.setCursor(ARROW);
}

void UserInterface::reset() {
	_bgFound = _oldBgFound = -1;
	_oldKey = -1;
	_oldTemp = _temp = -1;
	_exitZone = -1;
}


} // End of namespace Sherlock
