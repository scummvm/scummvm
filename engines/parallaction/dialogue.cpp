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
	char *_questions_names[20];
	Question *_questions[20];

	for (uint16 _si = 0; _si < 20; _si++) {
		v50[_si] = 0;
	}

	fillBuffers(script, true);

	while (scumm_stricmp(_tokens[0], "enddialogue")) {
		if (scumm_stricmp(_tokens[0], "Question")) continue;

		_questions[num_questions] = (Dialogue*)malloc(sizeof(Dialogue));
		Dialogue *vB4 = _questions[num_questions];
		memset(_questions[num_questions], 0, sizeof(Dialogue));

		_questions_names[num_questions] = (char*)malloc(strlen(_tokens[1])+1);
		strcpy(_questions_names[num_questions], _tokens[1]);

		vB4->_text = parseDialogueString(script);
//		printf("Question: '%s'\n", vB4->_text);

		fillBuffers(script, true);
		vB4->_mood = atoi(_tokens[0]);

		uint16 _di = 0;

		fillBuffers(script, true);
		while (scumm_stricmp(_tokens[0], "endquestion")) {	// parse answers

			const char** v60 = const_cast<const char **>(_localFlagNames);
			uint16 v56 = 1;

			if (_tokens[1][0]) {

				if (!scumm_stricmp(_tokens[1], "global")) {
					v56 = 2;
					v60 = const_cast<const char **>(_globalTable);
					vB4->_yesFlags[_di] |= kFlagsGlobal;
				}

				do {

					if (!scumm_strnicmp(_tokens[v56], "no", 2)) {
						byte _al = _vm->searchTable(_tokens[v56]+2, v60);
						vB4->_noFlags[_di] |= 1 << (_al - 1);
					} else {
						byte _al = _vm->searchTable(_tokens[v56], v60);
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

	_questions_names[num_questions] = NULL;

	for (uint16 _si = 0; _si <num_questions; _si++) {

		for (uint16 v5A = 0; v5A < 5; v5A++) {
			if (_questions[_si]->_answers[v5A] == 0) continue;

			int16 v58 = _vm->searchTable(_questions[_si]->_following._names[v5A], const_cast<const char **>(_questions_names));
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

	for (uint16 _si = 0; _si < num_questions; _si++)
		free(_questions_names[_si]);

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


void freeDialogue(Dialogue *d) {

	if (!d) return;

	uint16 _si;
	for (_si = 0; _si < 5; _si++) {
		if (d->_answer_moods[_si] & 0x10)
			freeDialogue(d->_following._questions[_si]);
	}

	for (_si = 0; _si < 5; _si++) {
		freeCommands(d->_commands[_si]);
		free(d->_answers[_si]);
	}

	free(d->_text);
	free(d);

	return;
}



void runDialogue(SpeakData *data) {
	debugC(1, kDebugDialogue, "runDialogue: starting dialogue '%s'", data->_name);

	enterDialogue();
	debugC(1, kDebugDialogue, "runDialogue: enterDialogue ok");
/*
	// the only character which can have a dialogue inside the museum location is Dino
	if (!scumm_stricmp(_name, "museum")) {
		_vm->_gfx->freeCnv( &_characterFrames );
		debugC(1, kDebugDialogue, "runDialogue: special trick for 'museum' location");
	}
*/
	_vm->_gfx->setFont("comic");

	Cnv v6E;
	StaticCnv v5C, v48;

	if (!scumm_stricmp(data->_name, "yourself") || data->_name[0] == '\0') {
		memcpy(&v6E, &_yourTalk, sizeof(Cnv));
		debugC(1, kDebugDialogue, "runDialogue: using default character head");
	} else {
		debugC(1, kDebugDialogue, "runDialogue: loading 2nd character head '%s'", _vm->_characterName);
		_vm->_disk->loadTalk(data->_name, &v6E);
		debugC(1, kDebugDialogue, "runDialogue: 2nd character head loaded");
	}

	v5C._width = v6E._width;
	v5C._height = v6E._height;

	v48._width = _yourTalk._width;
	v48._height = _yourTalk._height;

	bool displayedAnswers = false;
	int16 question_width = 0, question_height = 0;
	bool askPassword = false;
	uint16 _di = 0;
	Command *v34 = NULL;

	Dialogue *v60 = data->_dialogue;
	while (v60) {

		v5C._data0 = v6E._array[v60->_mood & 0xF];
		v5C._data1 = NULL; // v6E.field_8[v60->_mood & 0xF];
		v48._data0 = _yourTalk._array[0];
		v48._data1 = NULL; // _yourTalk.field_8[0];

		// display Question if any
		if (scumm_stricmp(v60->_text, "NULL")) {
			debugC(1, kDebugDialogue, "runDialogue: showing question '%s'", v60->_text);

			_vm->_gfx->flatBlitCnv(
				&v5C,
				QUESTION_CHARACTER_X,
				QUESTION_CHARACTER_Y,
				Gfx::kBitFront,
				v5C._data1
			);

			_vm->_gfx->getStringExtent(
				v60->_text,
				MAX_BALLOON_WIDTH,
				&question_width,
				&question_height
			);

			Common::Rect r(question_width, question_height);
			r.moveTo(QUESTION_BALLOON_X, QUESTION_BALLOON_Y);

			_vm->_gfx->drawBalloon(r, v60->_mood & 0x10);

			_vm->_gfx->displayWrappedString(
				v60->_text,
				QUESTION_BALLOON_X,
				QUESTION_BALLOON_Y,
				MAX_BALLOON_WIDTH,
				0
			);

			waitUntilLeftClick();
			_vm->_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);
		}

		if (v60->_answers[0] == NULL) break;

		_answerBalloonY[0] = 10;
		displayedAnswers = false;


		if (scumm_stricmp(v60->_answers[0], "NULL")) {

			uint16 _si = 0;
			while (_si < 5 && v60->_answers[_si]) {

				uint32 v28 = _localFlags[_vm->_currentLocationIndex];
				if (v60->_yesFlags[_si] & kFlagsGlobal) {
					v28 = _commandFlags | kFlagsGlobal;
				}

				// display suitable answers
				if (((v60->_yesFlags[_si] & v28) == v60->_yesFlags[_si]) && ((v60->_noFlags[_si] & ~v28) == v60->_noFlags[_si])) {

					_vm->_gfx->getStringExtent(
						v60->_answers[_si],
						MAX_BALLOON_WIDTH,
						&_answerBalloonW[_si],
						&_answerBalloonH[_si]
					);

					debugC(1, kDebugDialogue, "runDialogue: showing answer #%i '%s'",
						_si,
						v60->_answers[_si]
					);

					Common::Rect r(_answerBalloonW[_si], _answerBalloonH[_si]);
					r.moveTo(_answerBalloonX[_si], _answerBalloonY[_si]);

					_vm->_gfx->drawBalloon(r, 1);

					_answerBalloonY[_si+1] = 10 + _answerBalloonY[_si] + _answerBalloonH[_si];

					askPassword = _vm->_gfx->displayWrappedString(
						v60->_answers[_si],
						_answerBalloonX[_si],
						_answerBalloonY[_si],
						MAX_BALLOON_WIDTH,
						3
					);

					debugC(1, kDebugDialogue, "runDialogue: answer #%i shown at (%i, %i)+(%i, %i)",
						_si,
						_answerBalloonX[_si],
						_answerBalloonY[_si],
						_answerBalloonW[_si],
						_answerBalloonH[_si]
					);


					displayedAnswers = true;
				} else {
					debugC(1, kDebugDialogue, "runDialogue: skipping answer #%i", _si);

					_answerBalloonY[_si+1] = _answerBalloonY[_si];
					_answerBalloonY[_si] = SKIPPED_ANSWER;
				}

				_si++;

			}

			debugC(1, kDebugDialogue, "runDialogue: all suitable answers displayed");

			if (displayedAnswers == true) {

				debugC(1, kDebugDialogue, "runDialogue: showing answering face (%p)", (const void*)v48._data0);

				_vm->_gfx->flatBlitCnv(
					&v48,
					ANSWER_CHARACTER_X,
					ANSWER_CHARACTER_Y,
					Gfx::kBitFront,
					v48._data1
				);

				debugC(1, kDebugDialogue, "runDialogue: answering face shown");

				if (askPassword == false) {

					debugC(1, kDebugDialogue, "runDialogue: waiting for user to select answer");
					_di = selectAnswer(v60, &v48);
					debugC(1, kDebugDialogue, "runDialogue: user selected answer #%i", _di);

				} else {

					char password[100];
					uint16 passwordLen = 0;

					debugC(1, kDebugDialogue, "runDialogue: asking password");

					while (askPassword == true) {
						strcpy(password, ".......");
						_vm->_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);

						Common::Rect r(_answerBalloonW[0], _answerBalloonH[0]);
						r.moveTo(_answerBalloonX[0], _answerBalloonY[0]);

						_vm->_gfx->drawBalloon(r, 1);

						_vm->_gfx->displayWrappedString(
							v60->_answers[0],
							_answerBalloonX[0],
							_answerBalloonY[0],
							MAX_BALLOON_WIDTH,
							3
						);

						_vm->_gfx->flatBlitCnv(
							&v48,
							ANSWER_CHARACTER_X,
							ANSWER_CHARACTER_Y,
							Gfx::kBitFront,
							v48._data1
						);

						_vm->_gfx->displayBalloonString(
							_answerBalloonX[0] + 5,
							_answerBalloonY[0] + _answerBalloonH[0] - 15,
							"> ",
							0
						);

						OSystem::Event e;

						while (e.kbd.ascii != 0xD && passwordLen < MAX_PASSWORD_LENGTH) {

							// FIXME: see comment for updateInput()
							if (!g_system->getEventManager()->pollEvent(e)) continue;
							if (e.type != OSystem::EVENT_KEYDOWN) continue;
							if (e.type != OSystem::EVENT_QUIT) g_system->quit();
							if (!isdigit(e.kbd.ascii)) continue;

							password[passwordLen] = e.kbd.ascii;
							passwordLen++;
							password[passwordLen] = '\0';

							_vm->_gfx->displayBalloonString(
								_answerBalloonX[0] + 5,
								_answerBalloonY[0] + _answerBalloonH[0] - 15,
								password,
								0
							);

							g_system->delayMillis(20);
						}

						if ((!scumm_stricmp(_vm->_characterName, _doughName) && !scumm_strnicmp(password, "1732461", 7)) ||
							(!scumm_stricmp(_vm->_characterName, _donnaName) && !scumm_strnicmp(password, "1622", 4)) ||
							(!scumm_stricmp(_vm->_characterName, _dinoName) && !scumm_strnicmp(password, "179", 3))) {

							askPassword = false;

						}

						_di = 0;

					}

				}

				_vm->_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);

				v34 = v60->_commands[_di];
				v60 = (Dialogue*)v60->_following._questions[_di];

			} else {
				debugC(1, kDebugDialogue, "runDialogue: no suitable answers found");

				v60 = NULL;
			}
		} else {

			v60 = (Dialogue*)v60->_following._questions[_di];

		}

	}

	debugC(1, kDebugDialogue, "runDialogue: out of dialogue loop");
	_vm->_gfx->copyScreen(Gfx::kBitBack, Gfx::kBitFront);

	if (scumm_stricmp(data->_name, "yourself") || data->_name[0] == '\0') {
		_vm->_gfx->freeCnv(&v6E);
		debugC(1, kDebugDialogue, "runDialogue: 2nd character head free'd");
	}

	exitDialogue();
	debugC(1, kDebugDialogue, "runDialogue: exit dialogue ok");
/*
	// as mentioned in the comment at the beginning of this routine, the only
	// character which can have a dialogue inside the museum location is Dino,
	// that's why this hack (with hardcoded Dino) works
	if (!scumm_stricmp(_name, "museum")) {
		_vm->_disk->selectArchive("disk1");
		_vm->_disk->loadFrames(_dinoName, &_characterFrames);

		memcpy(&_yourself._cnv, &_characterFrames, sizeof(Cnv));

		debugC(1, kDebugDialogue, "runDialogue: special trick for 'museum' location ok");
	}
*/
	debugC(1, kDebugDialogue, "runDialogue: running zone commands");
	runCommands(v34);

	debugC(1, kDebugDialogue, "runDialogue: end");

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

		_vm->_gfx->displayWrappedString(
			q->_answers[_di],
			_answerBalloonX[_di],
			_answerBalloonY[_di],
			MAX_BALLOON_WIDTH,
			0
		);

		cnv->_data0 = _yourTalk._array[q->_answer_moods[_di] & 0xF];
//		cnv->_data1 = _yourTalk.field_8[q->_answer_moods[_di] & 0xF];

		_vm->_gfx->flatBlitCnv(
			cnv,
			ANSWER_CHARACTER_X,
			ANSWER_CHARACTER_Y,
			Gfx::kBitFront,
			cnv->_data1
		);

		waitUntilLeftClick();
		return _di;
	}

	int16 v2 = -1;

	_mouseButtons = kMouseNone;
	while (_mouseButtons != kMouseLeftUp) {

		_vm->updateInput();
		_si = getHoverAnswer(_mousePos._x, _mousePos._y, q);

		if (_si != v2) {
			if (v2 != -1) {
				_vm->_gfx->displayWrappedString(
					q->_answers[v2],
					_answerBalloonX[v2],
					_answerBalloonY[v2],
					MAX_BALLOON_WIDTH,
					3
				);
			}

			_vm->_gfx->displayWrappedString(
				q->_answers[_si],
				_answerBalloonX[_si],
				_answerBalloonY[_si],
				MAX_BALLOON_WIDTH,
				0
			);

			cnv->_data0 = _yourTalk._array[q->_answer_moods[_si] & 0xF];
//			cnv->_data1 = _yourTalk.field_8[q->_answer_moods[_si] & 0xF];

			_vm->_gfx->flatBlitCnv(
				cnv,
				ANSWER_CHARACTER_X,
				ANSWER_CHARACTER_Y,
				Gfx::kBitFront,
				cnv->_data1
			);

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

	for (int16 _si = 0; _si < 5; _si++) {
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
