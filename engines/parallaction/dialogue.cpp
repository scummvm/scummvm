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
 * $URL$
 * $Id$
 *
 */

#include "common/events.h"

#include "parallaction/input.h"
#include "parallaction/parallaction.h"



namespace Parallaction {

#define MAX_PASSWORD_LENGTH			 7
/*
#define QUESTION_BALLOON_X			140
#define QUESTION_BALLOON_Y			10
#define QUESTION_CHARACTER_X		  190
#define QUESTION_CHARACTER_Y		  80

#define ANSWER_CHARACTER_X			10
#define ANSWER_CHARACTER_Y			80
*/
struct BalloonPositions {
	Common::Point	_questionBalloon;
	Common::Point	_questionChar;

	Common::Point	_answerChar;
};

BalloonPositions _balloonPositions_NS = {
	Common::Point(140, 10),
	Common::Point(190, 80),
	Common::Point(10, 80)
};

BalloonPositions _balloonPositions_BR = {
	Common::Point(0, 0),
	Common::Point(380, 80),
	Common::Point(10, 80)
};


class DialogueManager {

	enum {
		RUN_QUESTION,
		RUN_ANSWER,
		NEXT_QUESTION,
		NEXT_ANSWER,
		DIALOGUE_OVER
	} _state;

	Parallaction	*_vm;
	Dialogue		*_dialogue;

	bool			_askPassword;
	int				_passwordLen;
	bool			_passwordChanged;

	bool			isNpc;
	GfxObj			*_questioner;
	GfxObj			*_answerer;

	Question		*_q;

	uint16			_visAnswers[5];
	int			_numVisAnswers;

	int			_answerId;

	int		_selection, _oldSelection;

	uint32			_mouseButtons;
	Common::Point	_mousePos;
	bool			_isKeyDown;
	uint16			_downKey;

	BalloonPositions	_ballonPos;

public:
	DialogueManager(Parallaction *vm, ZonePtr z);
	~DialogueManager();

	bool isOver() {
		return _state == DIALOGUE_OVER;
	}
	void run();

	ZonePtr			_z;
	CommandList *_cmdList;

protected:
	bool displayQuestion();
	bool displayAnswers();
	bool displayAnswer(uint16 i);

	int16 selectAnswer1();
	int16 selectAnswerN();
	int16 askPassword();
	int16 getHoverAnswer(int16 x, int16 y);

	void runQuestion();
	void runAnswer();
	void nextQuestion();
	void nextAnswer();

	bool checkPassword();
	void resetPassword();
	void accumPassword(uint16 ascii);
};

DialogueManager::DialogueManager(Parallaction *vm, ZonePtr z) : _vm(vm), _z(z) {
	int gtype = vm->getGameType();
	if (gtype == GType_Nippon) {
		_ballonPos = _balloonPositions_NS;
	} else
	if (gtype == GType_BRA) {
		_ballonPos = _balloonPositions_BR;
	} else
		error("unsupported game in DialogueManager");

	_dialogue = _z->u.speak->_dialogue;
	isNpc = scumm_stricmp(_z->u.speak->_name, "yourself") && _z->u.speak->_name[0] != '\0';
	_questioner = isNpc ? _vm->_disk->loadTalk(_z->u.speak->_name) : _vm->_char._talk;
	_answerer = _vm->_char._talk;

	_askPassword = false;
	_q = _dialogue->_questions[0];

	_cmdList = 0;
	_answerId = 0;

	_state = displayQuestion() ? RUN_QUESTION : NEXT_ANSWER;
}

DialogueManager::~DialogueManager() {
	if (isNpc) {
		delete _questioner;
	}
	_z = nullZonePtr;
}

bool DialogueManager::displayAnswer(uint16 i) {

	Answer *a = _q->_answers[i];

	uint32 flags = _vm->getLocationFlags();
	if (a->_yesFlags & kFlagsGlobal)
		flags = _globalFlags | kFlagsGlobal;

	// display suitable answers
	if (((a->_yesFlags & flags) == a->_yesFlags) && ((a->_noFlags & ~flags) == a->_noFlags)) {

		int id = _vm->_balloonMan->setDialogueBalloon(a->_text, 1, 3);
		assert(id >= 0);
		_visAnswers[id] = i;

		_askPassword = (strstr(a->_text, "%P") != NULL);
		_numVisAnswers++;

		return true;
	}

	return false;
}

bool DialogueManager::displayAnswers() {

	_numVisAnswers = 0;

	for (int i = 0; i < NUM_ANSWERS && _q->_answers[i]; i++) {
		displayAnswer(i);
	}

	if (_askPassword) {
		resetPassword();
//		_vm->_balloonMan->setDialogueBalloon(_q->_answers[0]->_text, 1, 3);
		int id = _vm->_gfx->setItem(_answerer, _ballonPos._answerChar.x, _ballonPos._answerChar.y);
		_vm->_gfx->setItemFrame(id, 0);
	} else
	if (_numVisAnswers == 1) {
		int id = _vm->_gfx->setItem(_answerer, _ballonPos._answerChar.x, _ballonPos._answerChar.y);
		_vm->_gfx->setItemFrame(id, _q->_answers[0]->_mood & 0xF);
		_vm->_balloonMan->setBalloonText(0, _q->_answers[_visAnswers[0]]->_text, 0);
	} else
	if (_numVisAnswers > 1) {
		int id = _vm->_gfx->setItem(_answerer, _ballonPos._answerChar.x, _ballonPos._answerChar.y);
		_vm->_gfx->setItemFrame(id, _q->_answers[_visAnswers[0]]->_mood & 0xF);
		_oldSelection = -1;
		_selection = 0;
	}

	return _numVisAnswers > 0;
}

bool DialogueManager::displayQuestion() {
	if (!scumm_stricmp(_q->_text, "NULL")) return false;

	_vm->_balloonMan->setSingleBalloon(_q->_text, _ballonPos._questionBalloon.x, _ballonPos._questionBalloon.y, _q->_mood & 0x10, 0);
	int id = _vm->_gfx->setItem(_questioner, _ballonPos._questionChar.x, _ballonPos._questionChar.y);
	_vm->_gfx->setItemFrame(id, _q->_mood & 0xF);

	return true;
}


bool DialogueManager::checkPassword() {
	return ((!scumm_stricmp(_vm->_char.getBaseName(), _doughName) && !scumm_strnicmp(_password, "1732461", 7)) ||
		   (!scumm_stricmp(_vm->_char.getBaseName(), _donnaName) && !scumm_strnicmp(_password, "1622", 4)) ||
		   (!scumm_stricmp(_vm->_char.getBaseName(), _dinoName) && !scumm_strnicmp(_password, "179", 3)));
}

void DialogueManager::resetPassword() {
	_passwordLen = 0;
	_password[0] = '\0';
	_passwordChanged = true;
}

void DialogueManager::accumPassword(uint16 ascii) {
	if (!isdigit(ascii)) {
		return;
	}

	_password[_passwordLen] = ascii;
	_passwordLen++;
	_password[_passwordLen] = '\0';
	_passwordChanged = true;
}

int16 DialogueManager::askPassword() {

	if (_isKeyDown) {
		accumPassword(_downKey);
	}

	if (_passwordChanged) {
		_vm->_balloonMan->setBalloonText(0, _q->_answers[0]->_text, 3);
		_passwordChanged = false;
	}

	if ((_passwordLen == MAX_PASSWORD_LENGTH) || ((_isKeyDown) && (_downKey == Common::KEYCODE_RETURN))) {
		if (checkPassword()) {
			return 0;
		} else {
			resetPassword();
		}
	}

	return -1;
}

int16 DialogueManager::selectAnswer1() {

	if (_mouseButtons == kMouseLeftUp) {
		return 0;
	}

	return -1;
}

int16 DialogueManager::selectAnswerN() {

	_selection = _vm->_balloonMan->hitTestDialogueBalloon(_mousePos.x, _mousePos.y);

	if (_selection != _oldSelection) {
		if (_oldSelection != -1) {
			_vm->_balloonMan->setBalloonText(_oldSelection, _q->_answers[_visAnswers[_oldSelection]]->_text, 3);
		}

		if (_vm->quit())
			return -1;

		if (_selection != -1) {
			_vm->_balloonMan->setBalloonText(_selection, _q->_answers[_visAnswers[_selection]]->_text, 0);
			_vm->_gfx->setItemFrame(0, _q->_answers[_visAnswers[_selection]]->_mood & 0xF);
		}
	}

	_oldSelection = _selection;

	if ((_mouseButtons == kMouseLeftUp) && (_selection != -1)) {
		return _visAnswers[_selection];
	}

	return -1;
}

void DialogueManager::runQuestion() {
	debugC(9, kDebugDialogue, "runQuestion\n");

	if (_mouseButtons == kMouseLeftUp) {
		_vm->hideDialogueStuff();
		_state = NEXT_ANSWER;
	}

}


void DialogueManager::nextAnswer() {
	debugC(9, kDebugDialogue, "nextAnswer\n");

	if (_q->_answers[0] == NULL) {
		_state = DIALOGUE_OVER;
		return;
	}

	if (!scumm_stricmp(_q->_answers[0]->_text, "NULL")) {
		_answerId = 0;
		_state = NEXT_QUESTION;
		return;
	}

	_state = displayAnswers() ? RUN_ANSWER : DIALOGUE_OVER;
}

void DialogueManager::runAnswer() {
	debugC(9, kDebugDialogue, "runAnswer\n");

	if (_askPassword) {
		_answerId = askPassword();
	} else
	if (_numVisAnswers == 1) {
		_answerId = selectAnswer1();
	} else {
		_answerId = selectAnswerN();
	}

	if (_answerId != -1) {
		_cmdList = &_q->_answers[_answerId]->_commands;
		_vm->hideDialogueStuff();
		_state = NEXT_QUESTION;
	}
}

void DialogueManager::nextQuestion() {
	debugC(9, kDebugDialogue, "nextQuestion\n");

	_q = _q->_answers[_answerId]->_following._question;
	if (_q == 0) {
		_state = DIALOGUE_OVER;
	} else {
		_state = displayQuestion() ? RUN_QUESTION : NEXT_ANSWER;
	}
}

void DialogueManager::run() {

	// cache event data
	_mouseButtons = _vm->_input->getLastButtonEvent();
	_vm->_input->getCursorPos(_mousePos);
	_isKeyDown = _vm->_input->getLastKeyDown(_downKey);

	switch (_state) {
	case RUN_QUESTION:
		runQuestion();
		break;

	case NEXT_ANSWER:
		nextAnswer();
		break;

	case NEXT_QUESTION:
		nextQuestion();
		break;

	case RUN_ANSWER:
		runAnswer();
		break;

	case DIALOGUE_OVER:
		break;

	default:
		error("unknown state in DialogueManager");

	}

}

void Parallaction::enterDialogueMode(ZonePtr z) {
	debugC(1, kDebugDialogue, "Parallaction::enterDialogueMode(%s)", z->u.speak->_name);
	_dialogueMan = new DialogueManager(this, z);
	_input->_inputMode = Input::kInputModeDialogue;
}

void Parallaction::exitDialogueMode() {
	debugC(1, kDebugDialogue, "Parallaction::exitDialogueMode()");
	_input->_inputMode = Input::kInputModeGame;

	if (_dialogueMan->_cmdList) {
		_vm->_cmdExec->run(*_dialogueMan->_cmdList);
	}

	// The current instance of _dialogueMan must be destroyed before the zone commands
	// are executed, because they may create another instance of _dialogueMan that
	// overwrite the current one. This would cause headaches (and it did, actually).
	ZonePtr z = _dialogueMan->_z;
	delete _dialogueMan;
	_dialogueMan = 0;

	_cmdExec->run(z->_commands, z);
}

void Parallaction::runDialogueFrame() {
	if (_input->_inputMode != Input::kInputModeDialogue) {
		return;
	}

	_dialogueMan->run();

	if (_dialogueMan->isOver()) {
		exitDialogueMode();
	}

	return;
}


} // namespace Parallaction
