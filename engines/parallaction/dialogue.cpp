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

#define QUESTION_BALLOON_X			140
#define QUESTION_BALLOON_Y			10
#define QUESTION_CHARACTER_X		  190
#define QUESTION_CHARACTER_Y		  80

#define ANSWER_CHARACTER_X			10
#define ANSWER_CHARACTER_Y			80

class DialogueManager {

	Parallaction	*_vm;
	SpeakData		*_data;
	Dialogue		*_dialogue;

	bool			_askPassword;

	bool			isNpc;
	Frames			*_questioner;
	Frames			*_answerer;

	Question		*_q;

	uint16			_visAnswers[5];
	int			_numVisAnswers;

public:
	DialogueManager(Parallaction *vm, SpeakData *data) : _vm(vm), _data(data) {
		_dialogue = _data->_dialogue;
		isNpc = scumm_stricmp(_data->_name, "yourself") && _data->_name[0] != '\0';
		_questioner = isNpc ? _vm->_disk->loadTalk(_data->_name) : _vm->_char._talk;
		_answerer = _vm->_char._talk;
	}

	~DialogueManager() {
		if (isNpc) {
			delete _questioner;
		}
	}

	void run();

protected:
	void displayQuestion();
	bool displayAnswers();
	bool displayAnswer(uint16 i);

	uint16 getAnswer();
	int16 selectAnswer();
	uint16 askPassword();
	int16 getHoverAnswer(int16 x, int16 y);

};

uint16 DialogueManager::askPassword() {
	debugC(3, kDebugExec, "checkDialoguePassword()");

	uint16 passwordLen = 0;
	_password[0] = '\0';

	_vm->_gfx->setDialogueBalloon(_q->_answers[0]->_text, 1, 3);
	int id = _vm->_gfx->setItem(_answerer, ANSWER_CHARACTER_X, ANSWER_CHARACTER_Y);
	_vm->_gfx->setItemFrame(id, 0);

	Common::Event e;
	bool changed = true;	// force first refresh

	while (true) {
		e.kbd.ascii = 0;

		if (g_system->getEventManager()->pollEvent(e)) {
			if (e.type == Common::EVENT_QUIT) {
				_vm->_quit = true;
				break;
			}

			if ((e.type == Common::EVENT_KEYDOWN) && isdigit(e.kbd.ascii)) {
				_password[passwordLen] = e.kbd.ascii;
				passwordLen++;
				_password[passwordLen] = '\0';
				changed = true;
			}
		}

		if (changed) {
			_vm->_gfx->setBalloonText(0, _q->_answers[0]->_text, 3);
			_vm->_gfx->updateScreen();
			changed = false;
		}

		if ((passwordLen == MAX_PASSWORD_LENGTH) || (e.kbd.ascii == Common::KEYCODE_RETURN)) {

			if ((!scumm_stricmp(_vm->_char.getBaseName(), _doughName) && !scumm_strnicmp(_password, "1732461", 7)) ||
			   (!scumm_stricmp(_vm->_char.getBaseName(), _donnaName) && !scumm_strnicmp(_password, "1622", 4)) ||
			   (!scumm_stricmp(_vm->_char.getBaseName(), _dinoName) && !scumm_strnicmp(_password, "179", 3))) {

				break;

			} else {
				passwordLen = 0;
				_password[0] = '\0';
				changed = true;
			}

		}

		g_system->delayMillis(20);

	}

	_vm->_gfx->hideDialogueStuff();

	return 0;

}



bool DialogueManager::displayAnswer(uint16 i) {

	Answer *a = _q->_answers[i];

	uint32 flags = _vm->getLocationFlags();
	if (a->_yesFlags & kFlagsGlobal)
		flags = _commandFlags | kFlagsGlobal;

	// display suitable answers
	if (((a->_yesFlags & flags) == a->_yesFlags) && ((a->_noFlags & ~flags) == a->_noFlags)) {

		int id = _vm->_gfx->setDialogueBalloon(a->_text, 1, 3);
		assert(id >= 0);
		_visAnswers[id] = i;

		_askPassword = (strstr(a->_text, "%p") != NULL);
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

	return _numVisAnswers > 0;
}

void DialogueManager::displayQuestion() {

	if (!scumm_stricmp(_q->_text, "NULL")) return;

	_vm->_gfx->setSingleBalloon(_q->_text, QUESTION_BALLOON_X, QUESTION_BALLOON_Y, _q->_mood & 0x10, 0);
	int id = _vm->_gfx->setItem(_questioner, QUESTION_CHARACTER_X, QUESTION_CHARACTER_Y);
	_vm->_gfx->setItemFrame(id, _q->_mood & 0xF);

	_vm->_gfx->updateScreen();
	_vm->_input->waitUntilLeftClick();
	_vm->_gfx->hideDialogueStuff();

	return;
}

uint16 DialogueManager::getAnswer() {

	uint16 answer = 0;

	if (_askPassword == false) {
		answer = selectAnswer();
	} else {
		answer = askPassword();
	}

	debugC(3, kDebugExec, "runDialogue: user selected answer #%i", answer);

	return answer;
}

void DialogueManager::run() {

	_askPassword = false;
	CommandList *cmdlist = NULL;

	_q = _dialogue->_questions[0];
	int16 answer;

	while (_q) {

		answer = 0;

		displayQuestion();
		
		if (_vm->_quit)
			return;

		if (_q->_answers[0] == NULL) break;

		if (scumm_stricmp(_q->_answers[0]->_text, "NULL")) {
			if (!displayAnswers()) break;
			answer = getAnswer();

			if (_vm->_quit)
				return;

			cmdlist = &_q->_answers[answer]->_commands;
		}

		_q = _q->_answers[answer]->_following._question;
	}

	if (cmdlist)
		_vm->runCommands(*cmdlist);

}

int16 DialogueManager::selectAnswer() {

	int16 numAvailableAnswers = _numVisAnswers;

	int id = _vm->_gfx->setItem(_answerer, ANSWER_CHARACTER_X, ANSWER_CHARACTER_Y);
	_vm->_gfx->setItemFrame(id, _q->_answers[0]->_mood & 0xF);

	if (numAvailableAnswers == 1) {
		_vm->_gfx->setBalloonText(0, _q->_answers[0]->_text, 0);
		_vm->_input->waitUntilLeftClick();
		_vm->_gfx->hideDialogueStuff();
		return 0;
	}

	int oldSelection = -1;
	int selection;

	uint32 event;
	Common::Point p;
	while (!_vm->_quit) {

		_vm->_input->readInput();
		_vm->_input->getCursorPos(p);
		event = _vm->_input->getLastButtonEvent();
		selection = _vm->_gfx->hitTestDialogueBalloon(p.x, p.y);

		if (selection != oldSelection) {
			if (oldSelection != -1) {
				_vm->_gfx->setBalloonText(oldSelection, _q->_answers[_visAnswers[oldSelection]]->_text, 3);
			}

			if (selection != -1) {
				_vm->_gfx->setBalloonText(selection, _q->_answers[_visAnswers[selection]]->_text, 0);
				_vm->_gfx->setItemFrame(0, _q->_answers[_visAnswers[selection]]->_mood & 0xF);
			}
		}

		if ((selection != -1) && (event == kMouseLeftUp)) {
			break;
		}

		_vm->_gfx->updateScreen();
		g_system->delayMillis(20);

		oldSelection = selection;
	}

	_vm->_gfx->hideDialogueStuff();

	return _visAnswers[selection];
}


void Parallaction::runDialogue(SpeakData *data) {
	debugC(1, kDebugExec, "runDialogue: starting dialogue '%s'", data->_name);

	DialogueManager man(this, data);
	man.run();

	return;
}


} // namespace Parallaction
