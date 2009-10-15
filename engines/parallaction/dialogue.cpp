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
#include "parallaction/exec.h"
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

	bool			isNpc;
	GfxObj			*_questioner;
	GfxObj			*_answerer;
	int				_faceId;

	Question		*_q;

	int			_answerId;

	int		_selection, _oldSelection;

	uint32			_mouseButtons;
	Common::Point	_mousePos;

protected:
	BalloonPositions	_ballonPos;
	struct VisibleAnswer {
		Answer	*_a;
		int		_index;		// index into Question::_answers[]
		int		_balloon;
	} _visAnswers[5];
	int			_numVisAnswers;
	bool			_isKeyDown;
	uint16			_downKey;

public:
	DialogueManager(Parallaction *vm, ZonePtr z);
	virtual ~DialogueManager();

	void start();

	bool isOver() {
		return _state == DIALOGUE_OVER;
	}
	void run();

	ZonePtr			_z;
	CommandList *_cmdList;

protected:
	bool displayQuestion();
	bool displayAnswers();
	bool testAnswerFlags(Answer *a);
	virtual void addVisibleAnswers(Question *q) = 0;
	virtual int16 selectAnswer() = 0;

	int16 selectAnswer1();
	int16 selectAnswerN();
	int16 getHoverAnswer(int16 x, int16 y);

	void runQuestion();
	void runAnswer();
	void nextQuestion();
	void nextAnswer();
};

DialogueManager::DialogueManager(Parallaction *vm, ZonePtr z) : _vm(vm), _z(z) {
	_dialogue = _z->u._speakDialogue;
	isNpc = !_z->u._filename.empty() && _z->u._filename.compareToIgnoreCase("yourself");
	_questioner = isNpc ? _vm->_disk->loadTalk(_z->u._filename.c_str()) : _vm->_char._talk;
	_answerer = _vm->_char._talk;

	_cmdList = 0;
	_answerId = 0;
}

void DialogueManager::start() {
	assert(_dialogue);
	_q = _dialogue->_questions[0];
	_state = displayQuestion() ? RUN_QUESTION : NEXT_ANSWER;
}


DialogueManager::~DialogueManager() {
	if (isNpc) {
		delete _questioner;
	}
	_z.reset();
}


bool DialogueManager::testAnswerFlags(Answer *a) {
	uint32 flags = _vm->getLocationFlags();
	if (a->_yesFlags & kFlagsGlobal)
		flags = _globalFlags | kFlagsGlobal;
	return ((a->_yesFlags & flags) == a->_yesFlags) && ((a->_noFlags & ~flags) == a->_noFlags);
}

bool DialogueManager::displayAnswers() {

	addVisibleAnswers(_q);
	if (_numVisAnswers == 0) {
		return false;
	}

	// create balloons
	int id;
	for (int i = 0; i < _numVisAnswers; ++i) {
		id = _vm->_balloonMan->setDialogueBalloon(_visAnswers[i]._a->_text.c_str(), 1, BalloonManager::kUnselectedColor);
		assert(id >= 0);
		_visAnswers[i]._balloon = id;

	}

	int mood = 0;
	if (_numVisAnswers == 1) {
		mood = _visAnswers[0]._a->_mood & 0xF;
		_vm->_balloonMan->setBalloonText(_visAnswers[0]._balloon, _visAnswers[0]._a->_text.c_str(), BalloonManager::kNormalColor);
	} else
	if (_numVisAnswers > 1) {
		mood = _visAnswers[0]._a->_mood & 0xF;
		_oldSelection = -1;
		_selection = 0;
	}

	_faceId = _vm->_gfx->setItem(_answerer, _ballonPos._answerChar.x, _ballonPos._answerChar.y);
	_vm->_gfx->setItemFrame(_faceId, mood);

	return true;
}

int16 DialogueManager::selectAnswer1() {
	if (!_visAnswers[0]._a->_text.compareToIgnoreCase("null")) {
		return _visAnswers[0]._index;
	}

	if (_mouseButtons == kMouseLeftUp) {
		return _visAnswers[0]._index;
	}

	return -1;
}

int16 DialogueManager::selectAnswerN() {

	_selection = _vm->_balloonMan->hitTestDialogueBalloon(_mousePos.x, _mousePos.y);

	if (_selection != _oldSelection) {
		if (_oldSelection != -1) {
			_vm->_balloonMan->setBalloonText(_visAnswers[_oldSelection]._balloon, _visAnswers[_oldSelection]._a->_text.c_str(), BalloonManager::kUnselectedColor);
		}

		if (_selection != -1) {
			_vm->_balloonMan->setBalloonText(_visAnswers[_selection]._balloon, _visAnswers[_selection]._a->_text.c_str(), BalloonManager::kSelectedColor);
			_vm->_gfx->setItemFrame(_faceId, _visAnswers[_selection]._a->_mood & 0xF);
		}
	}

	_oldSelection = _selection;

	if ((_mouseButtons == kMouseLeftUp) && (_selection != -1)) {
		return _visAnswers[_selection]._index;
	}

	return -1;
}

bool DialogueManager::displayQuestion() {
	if (!_q->_text.compareToIgnoreCase("NULL")) return false;

	_vm->_balloonMan->setSingleBalloon(_q->_text.c_str(), _ballonPos._questionBalloon.x, _ballonPos._questionBalloon.y, _q->_mood & 0x10, BalloonManager::kNormalColor);
	_faceId = _vm->_gfx->setItem(_questioner, _ballonPos._questionChar.x, _ballonPos._questionChar.y);
	_vm->_gfx->setItemFrame(_faceId, _q->_mood & 0xF);

	return true;
}

void DialogueManager::runQuestion() {
	debugC(9, kDebugDialogue, "runQuestion\n");

	if (_mouseButtons == kMouseLeftUp) {
		_vm->_gfx->freeDialogueObjects();
		_state = NEXT_ANSWER;
	}

}


void DialogueManager::nextAnswer() {
	debugC(9, kDebugDialogue, "nextAnswer\n");

	if (_q->_answers[0] == NULL) {
		_state = DIALOGUE_OVER;
		return;
	}

	if (!_q->_answers[0]->_text.compareToIgnoreCase("NULL")) {
		_answerId = 0;
		_state = NEXT_QUESTION;
		return;
	}

	_state = displayAnswers() ? RUN_ANSWER : DIALOGUE_OVER;
}

void DialogueManager::runAnswer() {
	debugC(9, kDebugDialogue, "runAnswer\n");

	_answerId = selectAnswer();
	if (_answerId != -1) {
		_cmdList = &_q->_answers[_answerId]->_commands;
		_vm->_gfx->freeDialogueObjects();
		_state = NEXT_QUESTION;
	}
}

void DialogueManager::nextQuestion() {
	debugC(9, kDebugDialogue, "nextQuestion\n");

	_q = _dialogue->findQuestion(_q->_answers[_answerId]->_followingName);
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



class DialogueManager_ns : public DialogueManager {
protected:
	Parallaction_ns *_vm;
	bool			_passwordChanged;
	bool			_askPassword;

	bool checkPassword() {
		return ((!scumm_stricmp(_vm->_char.getBaseName(), _doughName) && _vm->_password.hasPrefix("1732461")) ||
			   (!scumm_stricmp(_vm->_char.getBaseName(), _donnaName) && _vm->_password.hasPrefix("1622")) ||
			   (!scumm_stricmp(_vm->_char.getBaseName(), _dinoName) && _vm->_password.hasPrefix("179")));
	}

	void resetPassword() {
		_vm->_password.clear();
		_passwordChanged = true;
	}

	void accumPassword(uint16 ascii) {
		if (!isdigit(ascii)) {
			return;
		}

		_vm->_password += ascii;
		_passwordChanged = true;
	}

	int16 askPassword() {

		if (_isKeyDown) {
			accumPassword(_downKey);
		}

		if (_passwordChanged) {
			_vm->_balloonMan->setBalloonText(_visAnswers[0]._balloon, _visAnswers[0]._a->_text.c_str(), BalloonManager::kNormalColor);
			_passwordChanged = false;
		}

		if ((_vm->_password.size() == MAX_PASSWORD_LENGTH) || ((_isKeyDown) && (_downKey == Common::KEYCODE_RETURN))) {
			if (checkPassword()) {
				return 0;
			} else {
				resetPassword();
			}
		}

		return -1;
	}

public:
	DialogueManager_ns(Parallaction_ns *vm, ZonePtr z) : DialogueManager(vm, z), _vm(vm) {
		_ballonPos = _balloonPositions_NS;
	}

	bool canDisplayAnswer(Answer *a) {
		return testAnswerFlags(a);
	}

	virtual void addVisibleAnswers(Question *q) {
		_askPassword = false;
		_numVisAnswers = 0;
		for (int i = 0; i < NUM_ANSWERS && q->_answers[i]; i++) {
			Answer *a = q->_answers[i];
			if (!canDisplayAnswer(a)) {
				continue;
			}

			if (a->_text.contains("%P")) {
				_askPassword = true;
			}

			_visAnswers[_numVisAnswers]._a = a;
			_visAnswers[_numVisAnswers]._index = i;
			_numVisAnswers++;
		}

		resetPassword();
	}

	virtual int16 selectAnswer() {
		int ans = -1;
		if (_askPassword) {
			ans = askPassword();
		} else
		if (_numVisAnswers == 1) {
			ans = selectAnswer1();
		} else {
			ans = selectAnswerN();
		}
		return ans;
	}
};

class DialogueManager_br : public DialogueManager {
	Parallaction_br *_vm;

public:
	DialogueManager_br(Parallaction_br *vm, ZonePtr z) : DialogueManager(vm, z), _vm(vm) {
		_ballonPos = _balloonPositions_BR;
	}

	bool canDisplayAnswer(Answer *a) {
		if (!a)
			return false;

		if (a->_hasCounterCondition) {
			_vm->testCounterCondition(a->_counterName, a->_counterOp, a->_counterValue);
			return (_vm->getLocationFlags() & kFlagsTestTrue) != 0;
		}

		return testAnswerFlags(a);
	}

	virtual void addVisibleAnswers(Question *q) {
		_numVisAnswers = 0;
		for (int i = 0; i < NUM_ANSWERS && q->_answers[i]; i++) {
			Answer *a = q->_answers[i];
			if (!canDisplayAnswer(a)) {
				continue;
			}
			_visAnswers[_numVisAnswers]._a = a;
			_numVisAnswers++;
		}
	}

	virtual int16 selectAnswer() {
		int16 ans = -1;
		if (_numVisAnswers == 1) {
			ans = selectAnswer1();
		} else {
			ans = selectAnswerN();
		}
		return ans;
	}
};


void Parallaction::enterDialogueMode(ZonePtr z) {
	debugC(1, kDebugDialogue, "Parallaction::enterDialogueMode(%s)", z->u._filename.c_str());
	_dialogueMan = createDialogueManager(z);
	assert(_dialogueMan);
	_dialogueMan->start();
	_input->_inputMode = Input::kInputModeDialogue;
}

void Parallaction::exitDialogueMode() {
	debugC(1, kDebugDialogue, "Parallaction::exitDialogueMode()");
	_input->_inputMode = Input::kInputModeGame;

	/* Since the current instance of _dialogueMan must be destroyed before the
	   zone commands are executed, as they may create a new instance of _dialogueMan that
	   would overwrite the current, we need to save the references to the command lists.
    */ 
	CommandList *_cmdList = _dialogueMan->_cmdList;
	ZonePtr z = _dialogueMan->_z;

	// destroy the _dialogueMan here
	delete _dialogueMan;
	_dialogueMan = 0;

	// run the lists saved
	if (_cmdList) {
		_cmdExec->run(*_cmdList);
	}
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

DialogueManager *Parallaction_ns::createDialogueManager(ZonePtr z) {
	return new DialogueManager_ns(this, z);
}

DialogueManager *Parallaction_br::createDialogueManager(ZonePtr z) {
	return new DialogueManager_br(this, z);
}

} // namespace Parallaction
