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


char *parseDialogueString();

Dialogue *parseDialogue(ArchivedFile *file) {
//	printf("parseDialogue()\n");
	uint16 num_questions = 0;
	uint16 v50[20];
	char *_questions_names[20];
	Question *_questions[20];

	for (uint16 _si = 0; _si < 20; _si++) {
		v50[_si] = 0;
	}

	parseFillBuffers();

	while (scumm_stricmp(_tokens[0], "enddialogue")) {
		if (scumm_stricmp(_tokens[0], "Question")) continue;

		_questions[num_questions] = (Dialogue*)memAlloc(sizeof(Dialogue));
		Dialogue *vB4 = _questions[num_questions];
		memset(_questions[num_questions], 0, sizeof(Dialogue));

		_questions_names[num_questions] = (char*)memAlloc(strlen(_tokens[1])+1);
		strcpy(_questions_names[num_questions], _tokens[1]);

		vB4->_text = parseDialogueString();
//		printf("Question: '%s'\n", vB4->_text);

		parseFillBuffers();
		vB4->_mood = atoi(_tokens[0]);

		uint16 _di = 0;

		parseFillBuffers();
		while (scumm_stricmp(_tokens[0], "endquestion")) {	// parse answers

			char** v60 = _localFlagNames;
			uint16 v56 = 1;

			if (_tokens[1][0]) {

				if (!scumm_stricmp(_tokens[1], "global")) {
					v56 = 2;
					v60 = _globalTable;
					vB4->_yesFlags[_di] |= kFlagsGlobal;
				}

				do {

					if (!scumm_strnicmp(_tokens[v56], "no", 2)) {
						byte _al = _vm->searchTable(_tokens[v56]+2, v60);
						vB4->_noFlags[_di] |= 1 << (_al - 1);
					} else {
						byte _al = _vm->searchTable(_tokens[v56]+2, v60);
						vB4->_yesFlags[_di] |= 1 << (_al - 1);
					}

					v56++;

				} while (!scumm_stricmp(_tokens[v56], "|"));

			}

			vB4->_answers[_di] = parseDialogueString();

//			printf("answer[%i]: '%s'\n", _di, vB4->_answers[_di]);

			parseFillBuffers();
			vB4->_answer_moods[_di] = atoi(_tokens[0]);
			vB4->_following._names[_di] = parseDialogueString();

			parseFillBuffers();
			if (!scumm_stricmp(_tokens[0], "commands")) {
				vB4->_commands[_di] = parseCommands(file);
				parseFillBuffers();
			}

			_di++;
		}

		parseFillBuffers();
		num_questions++;

	}

	_questions_names[num_questions] = NULL;

	for (uint16 _si = 0; _si <num_questions; _si++) {

		for (uint16 v5A = 0; v5A < 5; v5A++) {
			if (_questions[_si]->_answers[v5A] == 0) continue;

			int16 v58 = _vm->searchTable(_questions[_si]->_following._names[v5A], _questions_names);
			memFree(_questions[_si]->_following._names[v5A]);

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
		memFree(_questions_names[_si]);

	return _questions[0];
}


char *parseDialogueString() {

	char vC8[200];
	char *vD0 = NULL;
	do {

		vD0 = parseNextLine(vC8, 200);
		if (vD0 == 0) return NULL;

		vD0 = skip_whitespace(vD0);

	} while (strlen(vD0) == 0);

	vD0[strlen(vD0)-1] = '\0';	// deletes the trailing '0xA' inserted by parseNextLine
								// this is critical for Graphics::displayBalloonString to work properly

	char *vCC = (char*)memAlloc(strlen(vD0)+1);
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
		memFree(d->_answers[_si]);
	}

	memFree(d->_text);
	memFree(d);

	return;
}



void runDialogue(SpeakData *data) {

	enterDialogue();

	if (!scumm_stricmp(_location, "museum")) {
		_vm->_graphics->freeCnv( &_tempFrames );
	}

	char v20[PATH_LEN];
	char *v24 = _vm->_characterName;
	if (!scumm_strnicmp(v24, "mini", 4)) {
		v24+=4;
	}

	if (_engineFlags & kEngineMiniDonna) {
		sprintf(v20, "%stta", v24);
	} else {
		sprintf(v20, "%stal", v24);
	}

	_vm->_graphics->loadExternalCnv(v20, &_characterFace);
	_vm->_graphics->loadExternalCnv("comiccnv", &Graphics::_font);

	Cnv v6E;
	StaticCnv v5C, v48;

	if (!scumm_stricmp(data->_name, "yourself") || data->_name[0] == '\0') {
		memcpy(&v6E, &_characterFace, sizeof(Cnv));
	} else {
		_vm->_graphics->loadCnv(data->_name, &v6E);
	}

	v5C._width = v6E._width;
	v5C._height = v6E._height;

	v48._width = _characterFace._width;
	v48._height = _characterFace._height;

	bool displayedAnswers = false;
	int16 question_width = 0, question_height = 0;
	bool askPassword = false;
	uint16 _di = 0;
	Command *v34 = NULL;

//	printf("enter Dialogue\n");

	Dialogue *v60 = data->_dialogue;
	while (v60) {

		v5C._data0 = v6E._array[v60->_mood & 0xF];
//		v5C._data1 = v6E.field_8[v60->_mood & 0xF];
		v48._data0 = _characterFace._array[v60->_mood & 0xF];
//		v48._data1 = _characterFace.field_8[v60->_mood & 0xF];

		// display Question if any
		if (scumm_stricmp(v60->_text, "NULL")) {
			_vm->_graphics->flatBlitCnv(
				&v5C,
				QUESTION_CHARACTER_X,
				QUESTION_CHARACTER_Y,
				Graphics::kBitFront,
				v5C._data1
			);

			_vm->_graphics->getStringExtent(
				v60->_text,
				MAX_BALLOON_WIDTH,
				&question_width,
				&question_height
			);

			_vm->_graphics->drawBalloon(
				QUESTION_BALLOON_X,
				QUESTION_BALLOON_Y,
				question_width,
				question_height,
				v60->_mood & 0x10
			);

			_vm->_graphics->displayWrappedString(
				v60->_text,
				QUESTION_BALLOON_X,
				QUESTION_BALLOON_Y,
				MAX_BALLOON_WIDTH,
				0
			);

			waitUntilLeftClick();
			_vm->_graphics->copyScreen(Graphics::kBitBack, Graphics::kBitFront);
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

					_vm->_graphics->getStringExtent(
						v60->_answers[_si],
						MAX_BALLOON_WIDTH,
						&_answerBalloonW[_si],
						&_answerBalloonH[_si]
					);

					_vm->_graphics->drawBalloon(
						_answerBalloonX[_si],
						_answerBalloonY[_si],
						_answerBalloonW[_si],
						_answerBalloonH[_si],
						1
					);

					_answerBalloonY[_si+1] = 10 + _answerBalloonY[_si] + _answerBalloonH[_si];

					askPassword = _vm->_graphics->displayWrappedString(
						v60->_answers[_si],
						_answerBalloonX[_si],
						_answerBalloonY[_si],
						MAX_BALLOON_WIDTH,
						3
					);

					displayedAnswers = true;
				} else {
					_answerBalloonY[_si+1] = _answerBalloonY[_si];
					_answerBalloonY[_si] = SKIPPED_ANSWER;
				}

				_si++;

			}

			if (displayedAnswers == true) {

				_vm->_graphics->flatBlitCnv(
					&v48,
					ANSWER_CHARACTER_X,
					ANSWER_CHARACTER_Y,
					Graphics::kBitFront,
					v48._data1
				);

				if (askPassword == false) {

					_di = selectAnswer(v60, &v48);

				} else {

					char password[100];
					uint16 passwordLen = 0;

					while (askPassword == true) {
						strcpy(password, ".......");
						_vm->_graphics->copyScreen(Graphics::kBitBack, Graphics::kBitFront);

						_vm->_graphics->drawBalloon(
							_answerBalloonX[0],
							_answerBalloonY[0],
							_answerBalloonW[0],
							_answerBalloonH[0],
							1
						);

						_vm->_graphics->displayWrappedString(
							v60->_answers[0],
							_answerBalloonX[0],
							_answerBalloonY[0],
							MAX_BALLOON_WIDTH,
							3
						);

						_vm->_graphics->flatBlitCnv(
							&v48,
							ANSWER_CHARACTER_X,
							ANSWER_CHARACTER_Y,
							Graphics::kBitFront,
							v48._data1
						);

						_vm->_graphics->displayBalloonString(
							_answerBalloonX[0] + 5,
							_answerBalloonY[0] + _answerBalloonH[0] - 15,
							"> ",
							0
						);

						OSystem::Event e;

						while (e.kbd.ascii != 0xD && passwordLen < MAX_PASSWORD_LENGTH) {

							if (!g_system->pollEvent(e)) continue;
							if (e.type != OSystem::EVENT_KEYDOWN) continue;
							if (!isdigit(e.kbd.ascii)) continue;

							password[passwordLen] = e.kbd.ascii;
							passwordLen++;
							password[passwordLen] = '\0';

							_vm->_graphics->displayBalloonString(
								_answerBalloonX[0] + 5,
								_answerBalloonY[0] + _answerBalloonH[0] - 15,
								password,
								0
							);

							g_system->delayMillis(20);
						}

						if ((!scumm_stricmp(_vm->_characterName, "dough") && !scumm_strnicmp(password, "1732461", 7)) ||
							(!scumm_stricmp(_vm->_characterName, "donna") && !scumm_strnicmp(password, "1622", 4)) ||
							(!scumm_stricmp(_vm->_characterName, "dino") && !scumm_strnicmp(password, "179", 3))) {

							askPassword = false;

						}

						_di = 0;

					}

				}

				_vm->_graphics->copyScreen(Graphics::kBitBack, Graphics::kBitFront);

				v34 = v60->_commands[_di];
				v60 = (Dialogue*)v60->_following._questions[_di];

			} else {
				v60 = NULL;
			}
		} else {

			v60 = (Dialogue*)v60->_following._questions[_di];

		}

	}

//	printf("exit Dialogue\n");

	_vm->_graphics->copyScreen(Graphics::kBitBack, Graphics::kBitFront);
	_vm->_graphics->freeCnv(&_characterFace);

	if (scumm_stricmp(data->_name, "yourself") || data->_name[0] == '\0') {
		_vm->_graphics->freeCnv(&v6E);
	}

	_vm->_graphics->freeCnv(&Graphics::_font);
	exitDialogue();

	if (!scumm_stricmp(_location, "museum")) {

		closeArchive();
		strcpy(_vm->_disk, "disk1");
		openArchive(_vm->_disk);
		_vm->_graphics->loadCnv("dino", &_tempFrames);

		memcpy(&_yourself._cnv, &_tempFrames, sizeof(Cnv));

	}

	runCommands(v34);

	return;

}

int16 selectAnswer(Question *q, StaticCnv *cnv) {

	int16 v6 = 0;
	int16 numAvailableAnswers = 0;
	int16 _si = 0;
	int16 _di = 0;

	for (; q->_answers[v6]; v6++) {
		if (_answerBalloonY[v6] == SKIPPED_ANSWER) continue;

		_di = v6;
		numAvailableAnswers++;
	}

	_answerBalloonY[v6] = 2000;

	if (numAvailableAnswers == 1) {

		_vm->_graphics->displayWrappedString(
			q->_answers[_di],
			_answerBalloonX[_di],
			_answerBalloonY[_di],
			MAX_BALLOON_WIDTH,
			0
		);

		cnv->_data0 = _characterFace._array[q->_answer_moods[_di] & 0xF];
//		cnv->_data1 = _characterFace.field_8[q->_answer_moods[_di] & 0xF];

		_vm->_graphics->flatBlitCnv(
			cnv,
			ANSWER_CHARACTER_X,
			ANSWER_CHARACTER_Y,
			Graphics::kBitFront,
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
			if (_si != -1) {
				_vm->_graphics->displayWrappedString(
					q->_answers[v2],
					_answerBalloonX[v2],
					_answerBalloonY[v2],
					MAX_BALLOON_WIDTH,
					3
				);
			}

			_vm->_graphics->displayWrappedString(
				q->_answers[_si],
				_answerBalloonX[_si],
				_answerBalloonY[_si],
				MAX_BALLOON_WIDTH,
				0
			);

			cnv->_data0 = _characterFace._array[q->_answer_moods[_si] & 0xF];
//			cnv->_data1 = _characterFace.field_8[q->_answer_moods[_si] & 0xF];

			_vm->_graphics->flatBlitCnv(
				cnv,
				ANSWER_CHARACTER_X,
				ANSWER_CHARACTER_Y,
				Graphics::kBitFront,
				cnv->_data1
			);

		}

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

//	backups background mask and path
//
//
void enterDialogue() {

	_vm->_graphics->backupBackgroundMask(Graphics::kMask0);
	_vm->_graphics->backupBackgroundPath(Graphics::kPath0);

	return;
}

//	restores background mask and path
//	rebuilds inventory
//
void exitDialogue() {

	_vm->_graphics->restoreBackgroundMask(Graphics::kMask0);
	_vm->_graphics->restoreBackgroundPath(Graphics::kPath0);

	refreshInventory(_vm->_characterName);

	return;
}


} // namespace Parallaction
