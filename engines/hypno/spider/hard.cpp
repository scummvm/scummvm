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

#include "hypno/hypno.h"

namespace Hypno {

void SpiderEngine::runCode(Code *code) {
	if (code->name == "<puz_matr>")
		runMatrix(code);
	else if (code->name == "<add_ingredient>")
		addIngredient(code);
	else if (code->name == "<check_mixture>")
		checkMixture(code);
	else if (code->name == "<note>")
		runNote(code);
	else if (code->name == "<fuse_panel>")
		runFusePanel(code);
	else if (code->name == "<recept>")
		runRecept(code);
	else if (code->name == "<office>")
		runOffice(code);
	else if (code->name == "<file_cabinet>") 
		runFileCabinet(code);
	else if (code->name == "<lock>") 
		runLock(code);
	else if (code->name == "<fuse_box>")
		runFuseBox(code);
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
	uint32 activeColor = 2;
	uint32 deactiveColor = 0;

	MVideo *v;

	if (isDemo()) {
		loadImage("puz_matr/matrixbg.smk", 0, 0, false, true);
		v = new MVideo("puz_matr/matintro.smk", Common::Point(0, 0), false, false, false);
	} else {
		loadImage("puz_ally/matrixbg.smk", 0, 0, false, true);
		v = new MVideo("puz_ally/matintro.smk", Common::Point(0, 0), false, false, false);
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
					playSound("sound.lib/matrix.raw", 1);

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
				MVideo video("cine/shv001s.smk", Common::Point(0, 0), false, false, false);
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

void SpiderEngine::addIngredient(Code *code) {
	if (_sceneState["GS_SWITCH0"]) { // wrong ingredient
		ingredients[0] = true;
	} else if (_sceneState["GS_SWITCH1"]) {
		ingredients[1] = true;
	} else if (_sceneState["GS_SWITCH2"]) {
		ingredients[2] = true;
	} else if (_sceneState["GS_SWITCH3"]) {
		ingredients[3] = true;
	} else if (_sceneState["GS_SWITCH4"]) {
		ingredients[4] = true;
	} else if (_sceneState["GS_SWITCH5"]) {
		ingredients[5] = true;
	} else if (_sceneState["GS_SWITCH6"]) {
		ingredients[6] = true;
	}
	resetSceneState();
	_nextLevel = "bushard2.mi_";
}

void SpiderEngine::checkMixture(Code *code) {
	_nextLevel = "bushard2.mi_";
	if (ingredients[0]) {
		MVideo video("spider/cine/blcs005s.smk", Common::Point(0, 0), false, true, false);
		runIntro(video);
		memset(ingredients, 0, 7);
		return;
	}

	bool nothing = true;
	for (int i = 1; i < 7; i++) {
		if (ingredients[i])
			nothing = false;
	}

	if (nothing)
		return;

	for (int i = 1; i < 7; i++) {
		if (!ingredients[i]) {
			MVideo video("spider/cine/blcs005s.smk", Common::Point(0, 0), false, true, false);
			runIntro(video);
			memset(ingredients, 0, 7);
			return;
		}
	}
	_nextLevel = "<after_bus_hard>";
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
		loadImage("spider/int_ball/enote.smk", 0, 0, false, true);
	} else { // hard
		MVideo v("spider/int_ball/ppv007hs.smk", Common::Point(0, 0), false, false, false);
		runIntro(v);
		loadImage("spider/int_ball/hnote.smk", 0, 0, false, true);
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

void SpiderEngine::runOffice(Code *code) {

	if (!_sceneState["GS_SWITCH6"]) { // lights off
		MVideo v("spider/cine/toodark.smk", Common::Point(0, 0), false, false, false);
		runIntro(v);
		_nextLevel = "recept.mi_";
		return;
	}
	_nextLevel = "<alveroff_selector>";
}

void SpiderEngine::runFusePanel(Code *code) {
	changeScreenMode("640x480");
	Common::Point mousePos;
	Common::Event event;

	defaultCursor();
	Common::Rect fuses(363, 52, 598, 408);
	Common::Rect back(0, 446, 640, 480);

	if (_sceneState["GS_PUZZLELEVEL"]) { // hard
		if (isFuseRust) {
			Common::String intro = "spider/cine/spv029s.smk"; 
			if (!_intros.contains(intro)) {
				MVideo v(intro, Common::Point(0, 0), false, false, false);
				runIntro(v);
				_intros[intro] = true;
			}

			loadImage("spider/int_alof/fuserust.smk", 0, 0, false, true);
		} else if (isFuseUnreadable)
			loadImage("spider/int_alof/fuseclea.smk", 0, 0, false, true);
		else
			loadImage("spider/int_alof/fuseread.smk", 0, 0, false, true);

	} else {
		isFuseRust = false;
		isFuseUnreadable = false;
		loadImage("spider/int_alof/fuse.smk", 0, 0, false, true);
	}

	while (!shouldQuit()) {

		while (g_system->getEventManager()->pollEvent(event)) {
			mousePos = g_system->getEventManager()->getMousePos();
			// Events
			switch (event.type) {

			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				break;

			case Common::EVENT_LBUTTONDOWN:
				if (back.contains(mousePos)) {
					_nextLevel = code->levelIfWin;
					return;
				}

				if (isFuseRust && _sceneState["GS_SWITCH8"]) {
					MVideo v("spider/cine/spv031s.smk", Common::Point(0, 0), false, false, false);
					runIntro(v);
					isFuseRust = false;
					isFuseUnreadable = true;
					loadImage("spider/int_alof/fuseclea.smk", 0, 0, false, true);
				} else if (isFuseUnreadable && _sceneState["GS_SWITCH9"]) {
					MVideo v("spider/cine/spv032s.smk", Common::Point(0, 0), false, false, false);
					runIntro(v);
					isFuseRust = false;
					isFuseUnreadable = false;
					loadImage("spider/int_alof/fuseread.smk", 0, 0, false, true);
				}

				if (isFuseRust || isFuseUnreadable)
					break;

				if (fuses.contains(mousePos)) {
					int x = (mousePos.x - 364) / (235 / 2.);
					int y = (mousePos.y - 54) / (355 / 10.);
					int s = 10* x + y + 1;

					if (s == 1) {
						_sceneState["GS_SWITCH1"] = !_sceneState["GS_SWITCH1"]; 
					} else if (s == 2) {
						_sceneState["GS_SWITCH2"] = !_sceneState["GS_SWITCH2"];
					} else if (s == 18) {
						_sceneState["GS_SWITCH3"] = !_sceneState["GS_SWITCH3"];
					} else if (s == 12) {
						_sceneState["GS_SWITCH4"] = !_sceneState["GS_SWITCH4"];
					} else if (s == 13) {
						_sceneState["GS_SWITCH5"] = !_sceneState["GS_SWITCH5"];
					} else if (s == 10) {
						_sceneState["GS_SWITCH6"] = !_sceneState["GS_SWITCH6"];
					}

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

void SpiderEngine::runFileCabinet(Code *code) {
	changeScreenMode("640x480");
	Common::Point mousePos;
	Common::Event event;

	uint32 comb[6] = {};
	Common::Rect sel[6];

	sel[0] = Common::Rect(16, 176, 91, 301);
	sel[1] = Common::Rect(108, 176, 183, 301);

	sel[2] = Common::Rect(232, 176, 306, 301);
	sel[3] = Common::Rect(324, 176, 400, 301);

	sel[4] = Common::Rect(453, 176, 526, 301);
	sel[5] = Common::Rect(545, 176, 618, 301);

	Common::String intro = "spider/cine/spv040s.smk"; 
	if (!_intros.contains(intro)) {
		MVideo v(intro, Common::Point(0, 0), false, false, false);
		runIntro(v);
		_intros[intro] = true;
	}

	Frames nums = decodeFrames("spider/int_alof/combo.smk");
	if (nums.size() != 10)
		error("Invalid number of digits: %d", nums.size());

	defaultCursor();
	Common::Rect back(0, 446, 640, 480);
	loadImage("spider/int_alof/combobg.smk", 0, 0, false, true);
	for (int i = 0; i < 6; i++) {
		drawImage(*nums[comb[i]], sel[i].left, sel[i].top, true);
	}

	while (!shouldQuit()) {

		while (g_system->getEventManager()->pollEvent(event)) {
			mousePos = g_system->getEventManager()->getMousePos();
			// Events
			switch (event.type) {

			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				break;

			case Common::EVENT_LBUTTONDOWN:
				if (back.contains(mousePos)) {
					if (comb[0] == 3 && comb[1] == 2 && comb[2] == 5 && comb[3] == 7 && comb[4] == 0 && comb[5] == 1) {
						MVideo v("spider/cine/file0000.smk", Common::Point(0, 0), false, false, false);
						runIntro(v);
						_sceneState["GS_SWITCH0"] = 1;
					}

					_nextLevel = code->levelIfWin;
					return;
				}

				for (int i = 0; i < 6; i++) {
					if (sel[i].contains(mousePos))
						comb[i] = (comb[i] + 1) % 10;
				}

				loadImage("spider/int_alof/combobg.smk", 0, 0, false, true);
				for (int i = 0; i < 6; i++) {
					drawImage(*nums[comb[i]], sel[i].left, sel[i].top, true);
				}
				break;

			case Common::EVENT_RBUTTONDOWN:
				for (int i = 0; i < 6; i++)
					if (sel[i].contains(mousePos)) {
						if (comb[i] == 0)
							comb[i] = 9;
						else
							comb[i] = comb[i] - 1;
					}

				loadImage("spider/int_alof/combobg.smk", 0, 0, false, true);
				for (int i = 0; i < 6; i++) {
					drawImage(*nums[comb[i]], sel[i].left, sel[i].top, true);
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

void SpiderEngine::runLock(Code *code) {
	changeScreenMode("640x480");
	Common::Point mousePos;
	Common::Event event;

	uint32 comb[5] = {};
	Common::Rect sel[5];

	sel[0] = Common::Rect(276, 57, 324, 134);
	sel[1] = Common::Rect(348, 57, 396, 134);
	sel[2] = Common::Rect(416, 57, 464, 134);
	sel[3] = Common::Rect(484, 57, 532, 134);
	sel[4] = Common::Rect(552, 57, 601, 134);
	Common::Rect act(345, 337, 537, 404);

	if (_sceneState["GS_PUZZLELEVEL"] == 0) { // easy
		MVideo v("spider/cine/spv051s.smk", Common::Point(0, 0), false, false, false);
		runIntro(v);
		loadImage("spider/factory/elockbg.smk", 0, 0, false, true);
	} else {
		MVideo v("spider/cine/spv051as.smk", Common::Point(0, 0), false, false, false);
		runIntro(v);
		loadImage("spider/factory/hlockbg.smk", 0, 0, false, true);
	}

	Frames nums = decodeFrames("spider/factory/button.smk");
	if (nums.size() != 5)
		error("Invalid number of colors: %d", nums.size());

	defaultCursor();
	for (int i = 0; i < 5; i++) {
		drawImage(*nums[comb[i]], sel[i].left, sel[i].top, true);
	}

	while (!shouldQuit()) {

		while (g_system->getEventManager()->pollEvent(event)) {
			mousePos = g_system->getEventManager()->getMousePos();
			// Events
			switch (event.type) {

			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				break;

			case Common::EVENT_LBUTTONDOWN:
				if (act.contains(mousePos)) {
					if (comb[0] == 1 && comb[1] == 1 && comb[2] == 1 && comb[3] == 1 && comb[4] == 1) {
					 	_nextLevel = code->levelIfWin;
					 	return;
					}
				}

				for (int i = 0; i < 5; i++) {
					if (sel[i].contains(mousePos))
						comb[i] = (comb[i] + 1) % 5;
				}

				if (_sceneState["GS_PUZZLELEVEL"] == 0) // easy
					loadImage("spider/factory/elockbg.smk", 0, 0, false, true);
				else 
					loadImage("spider/factory/hlockbg.smk", 0, 0, false, true);

				for (int i = 0; i < 5; i++) {
					drawImage(*nums[comb[i]], sel[i].left, sel[i].top, true);
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


void SpiderEngine::runFuseBox(Code *code) {
	changeScreenMode("640x480");
	Common::Point mousePos;
	Common::Event event;

	defaultCursor();

	bool hdata[8][9] = {};
	bool vdata[9][8] = {};

	bool vsol[9][8] = {
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 1, 1, 1, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 1, 1, 1, 0},
		{0, 1, 1, 1, 1, 1, 1, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 1, 1, 1, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
	};

	bool hsol[8][9] = {
		{0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 1, 0, 0, 1, 0, 0, 1, 0},
		{0, 1, 0, 0, 1, 0, 0, 1, 0},
		{0, 1, 0, 0, 1, 0, 0, 1, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 1, 0, 0, 1, 0, 0, 0, 0},
		{0, 1, 0, 0, 1, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0},
	};

	Common::Rect matrix(289, 89, 551, 351);
	Common::Point fuse(292, 87);
	Common::Point vz(289, 89);
	Common::Point hz(289, 89);

	uint32 dxVert = 412 - 380;
	uint32 dyVert = 120 - 88;

	uint32 dxHoriz = 359 - 327;
	uint32 dyHoriz = 146 - 114;

	Common::Rect vcell(0, 0, 8, 32);
	Common::Rect hcell(0, 0, 32, 8);

	if (_sceneState["GS_PUZZLELEVEL"] == 0) { // easy
		MVideo v("spider/cine/ppv011es.smk", Common::Point(0, 0), false, false, false);
		runIntro(v);
		loadImage("spider/movie2/efusebg.smk", 0, 0, false, true);
	} else { // hard
		MVideo v("spider/cine/ppv011hs.smk", Common::Point(0, 0), false, false, false);
		runIntro(v);
		loadImage("spider/movie2/hfusebg.smk", 0, 0, false, true);
	}

	Frames fuses = decodeFrames("spider/movie2/onoffuse.smk");

	while (!shouldQuit()) {

		while (g_system->getEventManager()->pollEvent(event)) {
			mousePos = g_system->getEventManager()->getMousePos();
			// Events
			switch (event.type) {

			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				break;

			case Common::EVENT_LBUTTONDOWN:
				if (matrix.contains(mousePos)) {
					if (_sceneState["GS_PUZZLELEVEL"] == 0) { // easy
						loadImage("spider/movie2/efusebg.smk", 0, 0, false, true);
					} else { // hard
						loadImage("spider/movie2/hfusebg.smk", 0, 0, false, true);
					}

					debug("\nvdata:");
					for (int i = 0; i < 9; i++) {
						for (int j = 0; j < 8; j++) {
							vcell.moveTo(vz.x + i*dxVert, vz.y + j*dyVert);
							if (vcell.contains(mousePos.x, mousePos.y)) {
								vdata[i][j] = !vdata[i][j];
							}
							debugN("%d, ", vdata[i][j]);
						}
						debugN("\n");
					}

					debug("\nhdata:");
					for (int i = 0; i < 8; i++) {
						for (int j = 0; j < 9; j++) {
							hcell.moveTo(hz.x + i*dxHoriz, hz.y + j*dyHoriz);
							if (hcell.contains(mousePos.x, mousePos.y)) {
								hdata[i][j] = !hdata[i][j];
							}
							debugN("%d, ", hdata[i][j]);
						}
						debugN("\n");
					}

					for (int i = 0; i < 9; i++) {
						for (int j = 0; j < 8; j++) {
							if (vdata[i][j]) {
								vcell.moveTo(i*dxVert, j*dyVert);
								Graphics::Surface sub = fuses[0]->getSubArea(vcell);
								drawImage(sub, vz.x + i*dxVert, vz.y + j*dyVert, true);
							}
						}
					}

					for (int i = 0; i < 8; i++) {
						for (int j = 0; j < 9; j++) {	
							if (hdata[i][j]) {
								hcell.moveTo(i*dxHoriz, j*dyHoriz);
								Graphics::Surface sub = fuses[0]->getSubArea(hcell);
								drawImage(sub, hz.x + i*dxHoriz, hz.y + j*dyHoriz, true);
								//debug("Found horizontal fuse between %d, %d and %d, %d", i, j, i + 1, j);
							}
						}
					}
				}
				break;

			default:
				break;
			}
		}

		bool hfound = true;
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 9; j++) {
				if (hdata[i][j] != hsol[i][j]) {
					hfound = false;
					break;
				}
			}
			if (!hfound)
				break;
		}

		bool vfound = true;
		for (int i = 0; i < 9; i++) {
			for (int j = 0; j < 8; j++) {
				if (vdata[i][j] != vsol[i][j]) {
					vfound = false;
					break;
				}
			}
			if (!vfound)
				break;
		}

		if (hfound && vfound) {
			_nextLevel = code->levelIfWin;
			return;
		}

		drawScreen();
		g_system->delayMillis(10);
	}
}

void SpiderEngine::showCredits() {
	if (_cheatsEnabled && !_arcadeMode.empty()) {
		_skipLevel = true;
		return;
	}

	if (!isDemo()) { // No credits in demo
		changeScreenMode("640x480");
		MVideo video("cine/credits.smk", Common::Point(0, 0), false, true, false);
		runIntro(video);
		_nextLevel = "mainmenu.mi_";
	}
}

} // End of namespace Hypno