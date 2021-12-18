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

#include "common/events.h"

#include "hypno/hypno.h"

namespace Hypno {

void SpiderEngine::runCode(Code *code) {
	if (code->name == "<puz_matr>")
		runMatrix(code);
	else if (code->name == "<note>")
		runNote(code);
	else if (code->name == "<fuse_panel>")
		runFusePanel(code);
	else if (code->name == "<recept>")
		runRecept(code);
	else if (code->name == "<credits>")
		showCredits();
	else
		error("invalid puzzle");
}

void SpiderEngine::runMatrix(Code *code) {
	changeScreenMode("640x480");
	Common::Point mousePos;
	Common::Event event;

	defaultCursor();
	bool data[10][10] = {};
	bool solution[10][10] = {
		{0, 0, 0, 1, 1, 1, 1, 0, 0, 0},
		{0, 0, 1, 1, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 1, 1, 1, 1, 1, 1, 0},
		{0, 1, 1, 0, 0, 0, 0, 1, 1, 0},
		{0, 1, 1, 1, 1, 1, 1, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 1, 1, 0, 0},
		{0, 0, 1, 1, 0, 0, 1, 1, 0, 0},
		{0, 0, 1, 1, 0, 0, 1, 1, 0, 0},
		{0, 0, 0, 1, 0, 0, 1, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	};
	Common::Rect matrix(175, 96, 461, 385);
	Common::Rect cell(0, 0, 27, 27);
	uint32 activeColor = _pixelFormat.RGBToColor(0, 130, 0);
	uint32 deactiveColor = _pixelFormat.RGBToColor(0, 0, 0);

	MVideo *v;

	if (isDemo()) {
		loadImage("sixdemo/puz_matr/matrixbg.smk", 0, 0, false);
		v = new MVideo("sixdemo/puz_matr/matintro.smk", Common::Point(0, 0), false, false, false);
	} else {
		loadImage("spider/puz_ally/matrixbg.smk", 0, 0, false);
		v = new MVideo("spider/puz_ally/matintro.smk", Common::Point(0, 0), false, false, false);
	}

	playVideo(*v);
	while (!shouldQuit()) {

		while (g_system->getEventManager()->pollEvent(event)) {
			mousePos = g_system->getEventManager()->getMousePos();
			// Events
			switch (event.type) {

			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				break;

			case Common::EVENT_LBUTTONDOWN:
				if (isDemo())
					playSound("sixdemo/demo/sound.lib/matrix.raw", 1);
				else
					playSound("spider/sound.lib/matrix.raw", 1);

				if (matrix.contains(mousePos)) {
					int x = (mousePos.x - 175) / 29;
					int y = (mousePos.y - 96) / 29;
					cell.moveTo(175 + 29 * x + 1, 96 + 29 * y + 1);
					_compositeSurface->fillRect(cell, data[x][y] ? deactiveColor : activeColor);
					data[x][y] = !data[x][y];
				}
				break;

			default:
				break;
			}
		}

		bool found = true;
		for (int x = 0; x < 10; x++) {
			for (int y = 0; y < 10; y++) {
				if (data[x][y] != solution[y][x]) {
					found = false;
					break;
				}
			}
			if (!found)
				break;
		}

		if (found) {
			if (isDemo())
				playSound("sixdemo/demo/sound.lib/matrix_2.raw", 1);
			else {
				MVideo video("spider/cine/shv001s.smk", Common::Point(0, 0), false, false, false);
				runIntro(video);
			}

			_nextLevel = code->levelIfWin;
			return;
		}

		if (v->decoder->needsUpdate()) {
			updateScreen(*v);
		}

		drawScreen();
		g_system->delayMillis(10);
	}
}

void SpiderEngine::runNote(Code *code) {
	const char alphaES[] = "abcdefghijklmnopqrstuvwxyz~";
	const char alphaEN[] = "abcdefghijklmnopqrstuvwxyz";

	const char solEasyES1[] = "hable cpn el svtp z talwe a";
	const char solEasyES2[] = "masz jane";
	char placeEasyES[] = "????? ??? ?? ???? ? ????? ?";

	const char solEasyEN1[] = "talk with the russian and save";
	const char solEasyEN2[] = "mary jane";
	char placeEasyEN[] = "????? ???? ??????? ????? ??? ????";

	char placeEasy2[] = "???? ????";

	const char solHardES1[] = "encvenuse a tmesdzakpw p tv";
	const char solHardES2[] = "mvjes mpsisa";
	char placeHardES[] = "????????? ? ?????????? ? ??";
	char placeHardES2[] = "????? ??????";

	const char solHardEN1[] = "find smerdyakov or your wife";
	const char solHardEN2[] = "dies";
	char placeHardEN[] = "???? ?????????? ?? ???? ??? ????";
	char placeHardEN2[] = "???? ????";

	changeScreenMode("640x480");
	Common::Point mousePos;
	Common::Event event;

	defaultCursor();
	Common::String alpha;
	Common::String selected = " ";
	char *firstSentence;
	char *secondSentence;
	Common::String firstSolution;
	Common::String secondSolution;

	switch (_language) {
	case Common::EN_USA:
		alpha = alphaEN;
		if (_sceneState["GS_PUZZLELEVEL"] == 0) { // easy
			firstSentence = (char*) &placeEasyEN;
			secondSentence = (char*) &placeEasy2;
			firstSolution = solEasyEN1;
			secondSolution = solEasyEN2;
		} else { // hard 
			firstSentence = (char*) &placeHardEN;
			secondSentence = (char*) &placeHardEN2;
			firstSolution = solHardEN1;
			secondSolution = solHardEN2;
		}
	break;
	
	case Common::ES_ESP:
		alpha = alphaES;
		if (_sceneState["GS_PUZZLELEVEL"] == 0) { // easy
			firstSentence = (char*) &placeEasyES;
			secondSentence = (char*) &placeEasy2;
			firstSolution = solEasyES1; 
			secondSolution = solEasyES2;
		} else { // hard 
			firstSentence = (char*) &placeHardES;
			secondSentence = (char*) &placeHardES2;
			firstSolution = solHardES1;
			secondSolution = solHardES2;
		}
	break;
	default:
		error("Unsupported language");
	break;
	}

	float firstSentenceLength = strlen(firstSentence);
	float secondSentenceLength = strlen(secondSentence);
	Common::Rect letterBox(22, 442, 554, 455); 
	Common::Rect firstSentenceBox(21, 140, 560, 162); 
	Common::Rect secondSentenceBox(21, 140, 196, 201); 

	Frames letters = decodeFrames("spider/int_ball/letters.smk");
	Common::Point size(letters[0]->w, letters[0]->h); 

	if (_sceneState["GS_PUZZLELEVEL"] == 0) { // easy
		MVideo v("spider/int_ball/ppv007es.smk", Common::Point(0, 0), false, false, false);
		runIntro(v);
		loadImage("spider/int_ball/enote.smk", 0, 0, false);
	} else { // hard
		MVideo v("spider/int_ball/ppv007hs.smk", Common::Point(0, 0), false, false, false);
		runIntro(v);
		loadImage("spider/int_ball/hnote.smk", 0, 0, false);
	}
	
	while (!shouldQuit()) {

		while (g_system->getEventManager()->pollEvent(event)) {
			mousePos = g_system->getEventManager()->getMousePos();
			uint32 o1x = 21;
			uint32 o1y = 140;

			uint32 o2x = 21;
			uint32 o2y = 180;

			// Events
			switch (event.type) {

			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				break;

			case Common::EVENT_LBUTTONDOWN:

				if (letterBox.contains(mousePos)) {
					uint32 idx = (mousePos.x - 21) / (letterBox.width() / (alpha.size()-1));
					selected = alpha[idx];
					//debug("%s", selected.c_str());
				} else if (firstSentenceBox.contains(mousePos)) {
					if (!selected.empty()) {
						uint32 idx = float(mousePos.x - 21) / (firstSentenceBox.width() / firstSentenceLength);
						//debug("idx: %d", idx);
						if (firstSentence[idx] != ' ') {
							firstSentence[idx] = selected[0];
							//debug("%s", firstSentence);
						}
					}
				} else if (secondSentenceBox.contains(mousePos)) {
					if (!selected.empty()) {
						uint32 idx = float(mousePos.x - 21) / (secondSentenceBox.width() / secondSentenceLength);
						//debug("idx: %d", idx);
						if (secondSentence[idx] != ' ') {
							secondSentence[idx] = selected[0];
							//debug("%s", secondSentence);
						}
					}
				}

				for (uint i = 0; i < strlen(firstSentence); i++) {
					if (firstSentence[i] != '?' && firstSentence[i] != ' ') {
						if (firstSentence[i] == '~')
							drawImage(*letters[26], o1x, o1y, false); // ñ
						else  
							drawImage(*letters[firstSentence[i]-97], o1x, o1y, false);

					}
					o1x = o1x + size.x;
				}

				for (uint i = 0; i < strlen(secondSentence); i++) {
					if (secondSentence[i] != '?' && secondSentence[i] != ' ') {
						if (secondSentence[i] == '~')
							drawImage(*letters[26], o2x, o2y, false); // ñ
						else  
							drawImage(*letters[secondSentence[i]-97], o2x, o2y, false);
					}
					o2x = o2x + size.x;
				}

				break;

			default:
				break;
			}
		}

		if (firstSentence == firstSolution && secondSentence == secondSolution) {
			if (_sceneState["GS_PUZZLELEVEL"] == 0 && Common::File::exists("spider/int_ball/ppv008es.smk")) {
				MVideo v("spider/int_ball/ppv008es.smk", Common::Point(0, 0), false, false, false);
				runIntro(v);
			} else if (_sceneState["GS_PUZZLELEVEL"] == 1 && Common::File::exists("spider/int_ball/ppv008hs.smk")) {
				MVideo v("spider/int_ball/ppv008hs.smk", Common::Point(0, 0), false, false, false);
				runIntro(v);
			}

			_nextLevel = code->levelIfWin;
			return;
		}

		drawScreen();
		g_system->delayMillis(10);
	}
}

void SpiderEngine::runRecept(Code *code) {

	if (!_sceneState["GS_SWITCH3"]) { // lights off
		MVideo v("spider/cine/recdark.smk", Common::Point(0, 0), false, false, false);
		runIntro(v);
		_nextLevel = "int_roof.mi_";
		return;
	}
	
	if (_sceneState["GS_SWITCH2"]) { // camera on
		MVideo v("spider/cine/iobs001s.smk", Common::Point(0, 0), false, true, false);
		runIntro(v);
		_nextLevel = "<over_apt_5>";
		return;
	}

	_nextLevel = "recept.mi_";
}

void SpiderEngine::runFusePanel(Code *code) {
	changeScreenMode("640x480");
	Common::Point mousePos;
	Common::Event event;

	defaultCursor();
	Common::Rect fuses(363, 52, 598, 408);
	Common::Rect back(0, 446, 640, 480);

	//MVideo *v;
	loadImage("spider/int_alof/fuse.smk", 0, 0, false);

	//playVideo(*v);
	while (!shouldQuit()) {

		while (g_system->getEventManager()->pollEvent(event)) {
			mousePos = g_system->getEventManager()->getMousePos();
			// Events
			switch (event.type) {

			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				break;

			case Common::EVENT_LBUTTONDOWN:
				if (fuses.contains(mousePos)) {
				    int x = (mousePos.x - 364) / (235 / 2.);
				    int y = (mousePos.y - 54) / (355 / 10.);
					int s = 10* x + y + 1;

					if (s == 1) {
						_sceneState["GS_SWITCH1"] = !_sceneState["GS_SWITCH1"]; 
					} else if (s == 2) {
						_sceneState["GS_SWITCH2"] = !_sceneState["GS_SWITCH2"];
					} else if (s == 12) {
						_sceneState["GS_SWITCH4"] = !_sceneState["GS_SWITCH4"];
					} else if (s == 13) {
						_sceneState["GS_SWITCH5"] = !_sceneState["GS_SWITCH5"];
					}

				} else if (back.contains(mousePos)) {
					_nextLevel = code->levelIfWin;
					return;
				}
				break;

			default:
				break;
			}
		}

		drawScreen();
		g_system->delayMillis(10);
	}
}


void SpiderEngine::showCredits() {
	changeScreenMode("640x480");
	MVideo video("cine/credits.smk", Common::Point(0, 0), false, false, false);
	runIntro(video);
}

} // End of namespace Hypno