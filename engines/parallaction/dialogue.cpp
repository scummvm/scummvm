/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

#include "common/stdafx.h"

#include "parallaction/commands.h"
#include "parallaction/parallaction.h"
#include "parallaction/graphics.h"
#include "parallaction/disk.h"
#include "parallaction/inventory.h"
#include "parallaction/parser.h"
#include "parallaction/zone.h"

#include "common/events.h"

namespace Parallaction {

#define SKIPPED_ANSWER		   1000

#define MAX_BALLOON_WIDTH			130

#define MAX_PASSWORD_LENGTH 		 7

#define QUESTION_BALLOON_X			140
#define QUESTION_BALLOON_Y			10
#define QUESTION_CHARACTER_X		  190
#define QUESTION_CHARACTER_Y		  80

#define ANSWER_CHARACTER_X			10
#define ANSWER_CHARACTER_Y			80


void enterDialogue();
void exitDialogue();

int16 selectAnswer(Question *q, StaticCnv*);
int16 getHoverAnswer(int16 x, int16 y, Question *q);

int16 _answerBalloonX[10] = { 80, 120, 150, 150, 150, 0, 0, 0, 0, 0 };
int16 _answerBalloonY[10] = { 10, 70, 130, 0, 0, 0, 0, 0, 0, 0 };
int16 _answerBalloonW[10] = { 0 };
int16 _answerBalloonH[10] = { 0 };



Dialogue *Parallaction::parseDialogue(Script &script) {
//	printf("parseDialogue()\n");
	uint16 num_questions = 0;
	uint16 v50[20];
	Table _questions_names(20);
	Question *_questions[20];

	for (uint16 _si = 0; _si < 20; _si++) {
		v50[_si] = 0;
	}

	fillBuffers(script, true);

	while (scumm_stricmp(_tokens[0], "enddialogue")) {
		if (scumm_stricmp(_tokens[0], "Question")) continue;

		_questions[num_questions] = new Dialogue;
		Dialogue *vB4 = _questions[num_questions];

		_questions_names.addData(_tokens[1]);

		vB4->_text = parseDialogueString(script);
//		printf("Question: '%s'\n", vB4->_text);

		fillBuffers(script, true);
		vB4->_mood = atoi(_tokens[0]);

		uint16 _di = 0;

		fillBuffers(script, true);
		while (scumm_stricmp(_tokens[0], "endquestion")) {	// parse answers

			if (_tokens[1][0]) {

				Table* v60 = _localFlagNames;
				uint16 v56 = 1;

				if (!scumm_stricmp(_tokens[1], "global")) {
					v56 = 2;
					v60 = _globalTable;
					vB4->_yesFlags[_di] |= kFlagsGlobal;
				}

				do {

					if (!scumm_strnicmp(_tokens[v56], "no", 2)) {
						byte _al = v60->lookup(_tokens[v56]+2);
						vB4->_noFlags[_di] |= 1 << (_al - 1);
					} else {
						byte _al = v60->lookup(_tokens[v56]);
						vB4->_yesFlags[_di] |= 1 << (_al - 1);
					}

					v56++;

				} while (!scumm_stricmp(_tokens[v56++], "|"));

			}

			vB4->_answers[_di] = parseDialogueString(script);

//			printf("answer[%i]: '%s'\n", _di, vB4->_answers[_di]);

			fillBuffers(script, true);
			vB4->_answer_moods[_di] = atoi(_tokens[0]);
			vB4->_following._names[_di] = parseDialogueString(script);

			fillBuffers(script, true);
			if (!scumm_stricmp(_tokens[0], "commands")) {
				vB4->_commands[_di] = parseCommands(script);
				fillBuffers(script, true);
			}

			_di++;
		}

		fillBuffers(script, true);
		num_questions++;

	}

	for (uint16 _si = 0; _si <num_questions; _si++) {

		for (uint16 v5A = 0; v5A < 5; v5A++) {
			if (_questions[_si]->_answers[v5A] == 0) continue;

			int16 v58 = _questions_names.lookup(_questions[_si]->_following._names[v5A]);
			free(_questions[_si]->_following._names[v5A]);

			if (v58 == -1) {
				_questions[_si]->_following._questions[v5A] = 0;
			} else {
				_questions[_si]->_following._questions[v5A] = _questions[v58-1];

				if (v50[v58]) {
					_questions[_si]->_answer_moods[v5A] |= 0x10;
				}

				v50[v58] = 1;
			}
		}
	}

	return _questions[0];
}


char *Parallaction::parseDialogueString(Script &script) {

	char vC8[200];
	char *vD0 = NULL;
	do {

		vD0 = script.readLine(vC8, 200);
		if (vD0 == 0) return NULL;

		vD0 = Common::ltrim(vD0);

	} while (strlen(vD0) == 0);

	vD0[strlen(vD0)-1] = '\0';	// deletes the trailing '0xA' inserted by parseNextLine
								// this is critical for Gfx::displayBalloonString to work properly

	char *vCC = (char*)malloc(strlen(vD0)+1);
	strcpy(vCC, vD0);

	return vCC;
}


void Parallaction::freeDialogue(Dialogue *d) {

	if (!d) return;

	uint16 _si;
	for (_si = 0; _si < NUM_ANSWERS; _si++) {
		if (d->_answer_moods[_si] & 0x10)
			freeDialogue(d->_following._questions[_si]);
	}

	for (_si = 0; _si < NUM_ANSWERS; _si++) {
		freeCommands(d->_commands[_si]);
		free(d->_answers[_si]);
	}

	free(d->_text);
	delete d;

	return;
}


uint16 askDialoguePassword(Dialogue *q, StaticCnv *face) {
	debugC(1, kDebugDialogue, "checkDialoguePassword()");

	char password[100];
	uint16 passwordLen = 0;

	while (true) {
		strcpy(password, ".......");
		_vm->_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);

		Common::Rect r(_answerBalloonW[0], _answerBalloonH[0]);
		r.moveTo(_answerBalloonX[0], _answerBalloonY[0]);

		_vm->_gfx->drawBalloon(r, 1);
		_vm->_gfx->displayWrappedString(q->_answers[0], _answerBalloonX[0], _answerBalloonY[0], MAX_BALLOON_WIDTH, 3);
		_vm->_gfx->flatBlitCnv(face, ANSWER_CHARACTER_X, ANSWER_CHARACTER_Y,	Gfx::kBitFront);
		_vm->_gfx->displayBalloonString(_answerBalloonX[0] + 5,	_answerBalloonY[0] + _answerBalloonH[0] - 15, "> ", 0);

		Common::Event e;
		while (e.kbd.ascii != 0xD && passwordLen < MAX_PASSWORD_LENGTH) {

			// FIXME: see comment for updateInput()
			if (!g_system->getEventManager()->pollEvent(e)) continue;
			if (e.type != Common::EVENT_KEYDOWN) continue;
			if (e.type != Common::EVENT_QUIT) g_system->quit();
			if (!isdigit(e.kbd.ascii)) continue;

			password[passwordLen] = e.kbd.ascii;
			passwordLen++;
			password[passwordLen] = '\0';

			_vm->_gfx->displayBalloonString(_answerBalloonX[0] + 5, _answerBalloonY[0] + _answerBalloonH[0] - 15, password, 0);

			g_system->delayMillis(20);
		}

		if ((!scumm_stricmp(_vm->_characterName, _doughName) && !scumm_strnicmp(password, "1732461", 7)) ||
			(!scumm_stricmp(_vm->_characterName, _donnaName) && !scumm_strnicmp(password, "1622", 4)) ||
			(!scumm_stricmp(_vm->_characterName, _dinoName) && !scumm_strnicmp(password, "179", 3))) {

			break;

		}

	}

	return 0;

}

bool _askPassword;

bool displayAnswer(Dialogue *q, uint16 i) {

	uint32 v28 = _localFlags[_vm->_currentLocationIndex];
	if (q->_yesFlags[i] & kFlagsGlobal)
		v28 = _commandFlags | kFlagsGlobal;

	// display suitable answers
	if (((q->_yesFlags[i] & v28) == q->_yesFlags[i]) && ((q->_noFlags[i] & ~v28) == q->_noFlags[i])) {

		_vm->_gfx->getStringExtent(q->_answers[i], MAX_BALLOON_WIDTH, &_answerBalloonW[i], &_answerBalloonH[i]);

		Common::Rect r(_answerBalloonW[i], _answerBalloonH[i]);
		r.moveTo(_answerBalloonX[i], _answerBalloonY[i]);

		_vm->_gfx->drawBalloon(r, 1);

		_answerBalloonY[i+1] = 10 + _answerBalloonY[i] + _answerBalloonH[i];
		_askPassword = _vm->_gfx->displayWrappedString(q->_answers[i], _answerBalloonX[i], _answerBalloonY[i], MAX_BALLOON_WIDTH, 3);

		return true;
	}

	return false;

}

bool displayAnswers(Dialogue *q) {

	bool displayed = false;

	uint16 i = 0;

	while (i < NUM_ANSWERS && q->_answers[i]) {
		if (displayAnswer(q, i)) {
			displayed = true;
		} else {
			_answerBalloonY[i+1] = _answerBalloonY[i];
			_answerBalloonY[i] = SKIPPED_ANSWER;
		}
		i++;
	}

	return displayed;
}

void displayQuestion(Dialogue *q, Cnv *cnv) {

	int16 w = 0, h = 0;

	if (!scumm_stricmp(q->_text, "NULL")) return;

	StaticCnv face;
	face._width = cnv->_width;
	face._height = cnv->_height;
	face._data0 = cnv->getFramePtr(q->_mood & 0xF);
	face._data1 = NULL; // cnv->field_8[v60->_mood & 0xF];

	_vm->_gfx->flatBlitCnv(&face, QUESTION_CHARACTER_X, QUESTION_CHARACTER_Y, Gfx::kBitFront);
	_vm->_gfx->getStringExtent(q->_text, MAX_BALLOON_WIDTH, &w, &h);

	Common::Rect r(w, h);
	r.moveTo(QUESTION_BALLOON_X, QUESTION_BALLOON_Y);

	_vm->_gfx->drawBalloon(r, q->_mood & 0x10);
	_vm->_gfx->displayWrappedString(q->_text, QUESTION_BALLOON_X, QUESTION_BALLOON_Y, MAX_BALLOON_WIDTH, 0);

	waitUntilLeftClick();

	_vm->_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);

	return;
}

uint16 getDialogueAnswer(Dialogue *q, Cnv *cnv) {

	uint16 answer = 0;

	StaticCnv face;
	face._width = cnv->_width;
	face._height = cnv->_height;
	face._data0 = cnv->getFramePtr(0);
	face._data1 = NULL; // cnv->field_8[0];

	_vm->_gfx->flatBlitCnv(&face, ANSWER_CHARACTER_X, ANSWER_CHARACTER_Y, Gfx::kBitFront);

	if (_askPassword == false) {
		answer = selectAnswer(q, &face);
	} else {
		answer = askDialoguePassword(q, &face);
	}

	_vm->_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);	// erase answer screen

	debugC(1, kDebugDialogue, "runDialogue: user selected answer #%i", answer);

	return answer;
}

void Parallaction::runDialogue(SpeakData *data) {
	debugC(1, kDebugDialogue, "runDialogue: starting dialogue '%s'", data->_name);

	enterDialogue();

	_vm->_gfx->setFont("comic");

	bool isNpc = scumm_stricmp(data->_name, "yourself") && data->_name[0] != '\0';
	Cnv *face = isNpc ? _vm->_disk->loadTalk(data->_name) : _vm->_char._talk;

	_askPassword = false;
	uint16 answer = 0;
	Command *cmdlist = NULL;

	Dialogue *q = data->_dialogue;
	while (q) {

		displayQuestion(q, face);
		if (q->_answers[0] == NULL) break;

		_answerBalloonY[0] = 10;

		if (scumm_stricmp(q->_answers[0], "NULL")) {

			if (!displayAnswers(q)) break;
			answer = getDialogueAnswer(q, _vm->_char._talk);
			cmdlist = q->_commands[answer];
		}

		q = (Dialogue*)q->_following._questions[answer];
	}

	debugC(1, kDebugDialogue, "runDialogue: out of dialogue loop");

	_vm->_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);

	if (isNpc) {
		delete face;
	}

	exitDialogue();
	runCommands(cmdlist);

	return;

}

int16 selectAnswer(Question *q, StaticCnv *cnv) {

	int16 numAvailableAnswers = 0;
	int16 _si = 0;
	int16 _di = 0;

	int16 i = 0;
	for (; q->_answers[i]; i++) {
		if (_answerBalloonY[i] == SKIPPED_ANSWER) continue;

		_di = i;
		numAvailableAnswers++;
	}
	_answerBalloonY[i] = 2000;

	if (numAvailableAnswers == 1) {
		_vm->_gfx->displayWrappedString(q->_answers[_di], _answerBalloonX[_di], _answerBalloonY[_di], MAX_BALLOON_WIDTH, 0);
		cnv->_data0 = _vm->_char._talk->getFramePtr(q->_answer_moods[_di] & 0xF);
//		cnv->_data1 = _vm->_char._talk->field_8[q->_answer_moods[_di] & 0xF];
		_vm->_gfx->flatBlitCnv(cnv, ANSWER_CHARACTER_X,	ANSWER_CHARACTER_Y, Gfx::kBitFront);
		waitUntilLeftClick();
		return _di;
	}

	int16 v2 = -1;

	_mouseButtons = kMouseNone;
	while (_mouseButtons != kMouseLeftUp) {

		_vm->updateInput();
		_si = getHoverAnswer(_vm->_mousePos.x, _vm->_mousePos.y, q);

		if (_si != v2) {
			if (v2 != -1)
				_vm->_gfx->displayWrappedString(q->_answers[v2], _answerBalloonX[v2], _answerBalloonY[v2], MAX_BALLOON_WIDTH, 3);

			_vm->_gfx->displayWrappedString(q->_answers[_si], _answerBalloonX[_si],	_answerBalloonY[_si], MAX_BALLOON_WIDTH, 0);
			cnv->_data0 = _vm->_char._talk->getFramePtr(q->_answer_moods[_si] & 0xF);
//			cnv->_data1 = _vm->_char._talk->field_8[q->_answer_moods[_si] & 0xF];
			_vm->_gfx->flatBlitCnv(cnv, ANSWER_CHARACTER_X, ANSWER_CHARACTER_Y, Gfx::kBitFront);
		}

		g_system->delayMillis(30);
		v2 = _si;
	}

	return _si;
}


//
//	finds out which answer is currently selected
//
int16 getHoverAnswer(int16 x, int16 y, Question *q) {

	int16 top = 1000;
	int16 bottom = 1000;

	for (int16 _si = 0; _si < NUM_ANSWERS; _si++) {
		if (q->_answers[_si] == NULL) break;

		if (_answerBalloonY[_si] != SKIPPED_ANSWER) {
			top = _answerBalloonY[_si];
		}

		int16 _di = _si + 1;
		for (; _answerBalloonY[_di] == SKIPPED_ANSWER; _di++) ;

		bottom = _answerBalloonY[_di];

		// mouse position is compared only with y coordinates
		if (y > top && y < bottom) return _si;

	}

	return 0;

}


void enterDialogue() {

	return;
}

//	rebuilds inventory
//
void exitDialogue() {

	refreshInventory(_vm->_characterName);

	return;
}


} // namespace Parallaction
