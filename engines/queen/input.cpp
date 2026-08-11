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


#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "queen/queen.h"
#include "queen/input.h"

namespace Queen {

const verbAction Input::_verbActions[] = {
	{ VERB_OPEN, kActionOpen },
	{ VERB_CLOSE, kActionClose },
	{ VERB_MOVE, kActionMove },
	{ VERB_GIVE, kActionGive },
	{ VERB_LOOK_AT, kActionLook },
	{ VERB_PICK_UP, kActionPickUp },
	{ VERB_TALK_TO, kActionTalk },
	{ VERB_USE, kActionUse }
};

Input::Input(Common::Language language, OSystem *system) :
	_system(system), _eventMan(system->getEventManager()), _fastMode(false),
	_keyVerb(VERB_NONE), _cutawayRunning(false), _canQuit(false),
	_cutawayQuit(false), _dialogueRunning(false), _talkQuit(false),
	_quickSave(false), _quickLoad(false), _inKey(kActionNone),
	_mouseButton(0), _idleTime(0) {
}

void Input::delay(uint amount) {
	if (_fastMode && amount > DELAY_SHORT) {
		amount = DELAY_SHORT;
	}
	if (_idleTime < DELAY_SCREEN_BLANKER) {
		_idleTime += amount;
	}
	uint32 end = _system->getMillis() + amount;
	do {
		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			_idleTime = 0;
			switch (event.type) {
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				if (event.customType == kActionFastMode) {
					_fastMode = !_fastMode;
				} else {
					_inKey = event.customType;
				}
				break;

			case Common::EVENT_LBUTTONDOWN:
				_mouseButton |= MOUSE_LBUTTON;
				break;

			case Common::EVENT_RBUTTONDOWN:
				_mouseButton |= MOUSE_RBUTTON;
				if (_dialogueRunning)
					_talkQuit = true;
				break;
			case Common::EVENT_RETURN_TO_LAUNCHER:
			case Common::EVENT_QUIT:
				if (_cutawayRunning)
					_cutawayQuit = true;
				// Allow using close button while dialogue is running
				if (_dialogueRunning)
					_talkQuit = true;
				return;

			default:
				break;
			}
		}

		_system->updateScreen();

		if (amount == 0)
			break;

		_system->delayMillis((amount > 10) ? 10 : amount);
	} while (_system->getMillis() < end);
}

void Input::checkKeys() {

	if (_inKey)
		debug(6, "[Input::checkKeys] _inKey = %i", _inKey);

	switch (_inKey) {
	case kActionSkipText:
		_keyVerb = VERB_SKIP_TEXT;
		break;
	case kActionScrollUp:
		_keyVerb = VERB_SCROLL_UP;
		break;
	case kActionScrollDown:
		_keyVerb = VERB_SCROLL_DOWN;
		break;
	case kActionInvSlot1:
		_keyVerb = VERB_DIGIT_1;
		break;
	case kActionInvSlot2:
		_keyVerb = VERB_DIGIT_2;
		break;
	case kActionInvSlot3:
		_keyVerb = VERB_DIGIT_3;
		break;
	case kActionInvSlot4:
		_keyVerb = VERB_DIGIT_4;
		break;
	case kActionSkipCutaway: // skip cutaway / dialogue
		if (_canQuit) {
			if (_cutawayRunning) {
				debug(6, "[Input::checkKeys] Setting _cutawayQuit to true");
				_cutawayQuit = true;
			}
			if (_dialogueRunning)
				_talkQuit = true;
		}
		break;
	case kActionJournal: // use Journal
		if (_cutawayRunning) {
			if (_canQuit) {
				_keyVerb = VERB_USE_JOURNAL;
				_cutawayQuit = _talkQuit = true;
			}
		} else {
			_keyVerb = VERB_USE_JOURNAL;
			if (_canQuit)
				_talkQuit = true;
		}
		break;
	case kActionSave: // quicksave
		_quickSave = true;
		break;
	case kActionLoad: // quickload
		_quickLoad = true;
		break;
	default:
		for (int i = 0; i < ARRAYSIZE(_verbActions); ++i) {
			if (_inKey == _verbActions[i]._action) {
				_keyVerb = _verbActions[i]._verb;
				break;
			}
		}
		break;
	}

	_inKey = kActionNone;	// reset
}

Common::Point Input::getMousePos() const {
	return _eventMan->getMousePos();
}

} // End of namespace Queen
