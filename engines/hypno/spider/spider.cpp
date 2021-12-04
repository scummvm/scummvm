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
#include "common/config-manager.h"

#include "hypno/grammar.h"
#include "hypno/hypno.h"
#include "hypno/libfile.h"

namespace Hypno {

SpiderEngine::SpiderEngine(OSystem *syst, const ADGameDescription *gd) : HypnoEngine(syst, gd) {}

void SpiderEngine::loadAssets() {
	if (!isDemo())
		loadAssetsFullGame();
	else
		loadAssetsDemo();
}

void SpiderEngine::loadAssetsFullGame() {

	Common::Language language = Common::parseLanguage(ConfMan.get("language"));
	Common::String prefix = "spider";
	if (language == Common::EN_USA) {
		if (!_installerArchive.open("DATA.Z"))
			error("Failed to open DATA.Z");

		SearchMan.add("DATA.Z", (Common::Archive *) &_installerArchive, 0, false);
	}

	Common::ArchiveMemberList files;
	LibFile *missions = loadLib("", "spider/c_misc/missions.lib", true);
	if (missions == nullptr || missions->listMembers(files) == 0)
		error("Failed to load any file from missions.lib");

	//loadLib("", "c_misc/fonts.lib", true);
	loadLib("spider/sound.lib/", "spider/c_misc/sound.lib", true);

	Code *credits = new Code();
	credits->name = "<credits>";
	credits->prefix = prefix;
	_levels["<credits>"] = credits;
	credits->levelIfWin = "options.mi_";

	Scene *sc;
	ChangeLevel *cl;

	loadSceneLevel("mainmenu.mi_", "", prefix);
	loadSceneLevel("tryagain.mi_", "", prefix);

	cl = new ChangeLevel("mainmenu.mi_");
	sc = (Scene *) _levels["tryagain.mi_"];
	sc->hots[1].actions.push_back(cl);

	loadSceneLevel("options.mi_", "", prefix);
	loadSceneLevel("levels.mi_", "", prefix);
	loadSceneLevel("combmenu.mi_", "", prefix);

	loadSceneLevel("mv0t.mi_", "roof.mi_", prefix);
	_levels["mv0t.mi_"]->intros.push_back("cine/ints001s.smk");
	_levels["mv0t.mi_"]->music = "sound.lib/sm001.raw";

	loadSceneLevel("roof.mi_", "decide1.mi_", prefix);
	_levels["roof.mi_"]->music = "sound.lib/ros_mus.raw";

	loadSceneLevel("decide1.mi_", "", prefix);
	_levels["decide1.mi_"]->intros.push_back("cine/siren1s.smk");
	sc = (Scene *) _levels["decide1.mi_"];
	cl = new ChangeLevel("bank.mi_");
	sc->hots[2].actions.push_back(cl);
	cl = new ChangeLevel("c1"); // depens on the difficulty
	sc->hots[4].actions.push_back(cl);

	loadSceneLevel("bank.mi_", "", prefix);
	_levels["bank.mi_"]->intros.push_back("cine/swcs001s.smk");
	_levels["bank.mi_"]->levelIfWin = "<alley_selector>";

	Transition *alley_selector = new Transition("alley.mi_", "<puz_matr>");
	_levels["<alley_selector>"] = alley_selector;

	loadSceneLevel("alley.mi_", "<trans_apt_2>", prefix);
	_levels["alley.mi_"]->music = "alleymus.raw";
	_levels["alley.mi_"]->intros.push_back("cine/aleyc01s.smk");

	loadArcadeLevel("c1.mi_", "<trans_apt_1>", prefix);
	_levels["c1.mi_"]->intros.push_back("cine/ross002s.smk");
	_levels["c1.mi_"]->levelIfLose = "<over_apt_1>";

	loadArcadeLevel("c1h.mi_", "<trans_apt_1>", prefix);
	_levels["c1h.mi_"]->intros.push_back("cine/ross002s.smk");
	_levels["c1h.mi_"]->levelIfLose = "<over_apt_1>";

	// Hardcoded levels
	Code *matrix = new Code();
	matrix->name = "<puz_matr>";
	matrix->intros.push_back("spider/cine/aleyc01s.smk");
	matrix->levelIfWin = "<trans_apt_2>";
	_levels["<puz_matr>"] = matrix;

	// Transitions
	Transition *trans_apt_1 = new Transition("busintro.mi_");
	trans_apt_1->frameImage = "spider/cine/blcs00.smk";
	trans_apt_1->frameNumber = 0;
	trans_apt_1->intros.push_back("spider/cine/ross004s.smk");
	trans_apt_1->intros.push_back("spider/cine/apts002s.smk");
	_levels["<trans_apt_1>"] = trans_apt_1;

	Transition *trans_apt_2 = new Transition("busint.mi_");
	trans_apt_2->frameImage = "spider/cine/blcs00.smk";
	trans_apt_2->frameNumber = 0;
	trans_apt_2->intros.push_back("spider/cine/dia002s.smk");
	trans_apt_2->intros.push_back("spider/cine/apts004s.smk");
	_levels["<trans_apt_2>"] = trans_apt_2;

	loadSceneLevel("busint.mi_", "busintro.mi_", prefix);
	loadSceneLevel("busintro.mi_", "<bus_selector>", prefix);
	_levels["busintro.mi_"]->intros.push_back("cine/blcs001s.smk");

	Transition *bus_selector = new Transition("buspuz.mi_", "bushard1.mi_");
	_levels["<bus_selector>"] = bus_selector;

	loadSceneLevel("bushard1.mi_", "bushard1.mi_", prefix);
	loadSceneLevel("bushard2.mi_", "decide3.mi_", prefix);

	loadSceneLevel("buspuz.mi_", "decide3.mi_", prefix);
	loadSceneLevel("decide3.mi_", "", prefix);

	sc = (Scene *) _levels["decide3.mi_"];
	cl = new ChangeLevel("alofintr.mi_");
	sc->hots[2].actions.push_back(cl);
	cl = new ChangeLevel("c4"); // depens on the difficulty
	sc->hots[4].actions.push_back(cl);

	loadSceneLevel("int_roof.mi_", "", prefix);
	sc = (Scene *) _levels["int_roof.mi_"];
	Overlay *over = (Overlay*) sc->hots[0].actions[2];
	over->path = "int_alof\\ROOFB1.SMK"; // seems to be a bug?

	loadSceneLevel("alofintr.mi_", "<boil_selector>", prefix);
	
	Transition *boil_selector = new Transition("boiler.mi_", "boilhard.mi_");
	_levels["<boil_selector>"] = boil_selector;

	loadSceneLevel("boiler.mi_", "", prefix);
	sc = (Scene *) _levels["boiler.mi_"];
	over = (Overlay*) sc->hots[0].actions[2];
	over->path = "int_alof\\BOILB1.SMK"; // seems to be a bug?
	
	cl = new ChangeLevel("int_roof.mi_");
	sc->hots[2].actions.push_back(cl);

	loadSceneLevel("boilhard.mi_", "", prefix);
	sc = (Scene *) _levels["boilhard.mi_"];
	over = (Overlay*) sc->hots[0].actions[2];
	over->path = "int_alof\\BOILB1.SMK"; // seems to be a bug?

	cl = new ChangeLevel("int_roof.mi_");
	sc->hots[2].actions.push_back(cl);

	loadSceneLevel("alverofh.mi_", "", prefix);
	loadSceneLevel("recept.mi_", "", prefix);

	sc = (Scene *) _levels["recept.mi_"];
	over = (Overlay*) sc->hots[0].actions[2];
	over->path = "int_alof\\rec0B1.SMK"; // seems to be a bug?

	loadSceneLevel("alveroff.mi_", "", prefix);

	loadArcadeLevel("c4.mi_", "c2", prefix);
	loadArcadeLevel("c2.mi_", "decide4.mi_", prefix);
	loadArcadeLevel("c4h.mi_", "c2", prefix);
	loadArcadeLevel("c2h.mi_", "decide4.mi_", prefix);

	loadSceneLevel("decide4.mi_", "", prefix);
	sc = (Scene *) _levels["decide4.mi_"];
	cl = new ChangeLevel("ball1.mi_");
	sc->hots[2].actions.push_back(cl);
	cl = new ChangeLevel("c5"); // depens on the difficulty
	sc->hots[4].actions.push_back(cl);

	loadArcadeLevel("c5.mi_", "factory1.mi_", prefix);
	loadArcadeLevel("c5h.mi_", "factory1.mi_", prefix);

	loadSceneLevel("ball1.mi_", "<note>", prefix);
	loadSceneLevel("ball2.mi_", "balcony.mi_", prefix);
	loadSceneLevel("balcony.mi_", "factory1.mi_", prefix);

	Code *note = new Code();
	note->name = "<note>";
	note->levelIfWin = "ball2.mi_";
	_levels["<note>"] = note;

	loadSceneLevel("factory1.mi_", "intercom.mi_", prefix);
	loadSceneLevel("intercom.mi_", "c3", prefix);

	loadArcadeLevel("c3.mi_", "", prefix);
	loadArcadeLevel("c3h.mi_", "", prefix);

	loadSceneLevel("movie2.mi_", "", prefix);
	//_levels["buspuz.mi_"]->intros.push_back("cine/ppv001s.smk");

	// Transition *bus_transition = new Transition("buspuz.mi_");
	// bankEasy->intros.push_back("spider/cine/dia002s.smk");
	// _levels["<bus_transition>"] = bankEasy;

	// Transition *bankHard = new Transition();
	// bankHard->level = "buspuz.mi_";
	// bankHard->intros.push_back("spider/cine/bals003s.smk");
	// _levels["<bank_hard>"] = bankHard;

	// Easy arcade levels

	//loadArcadeLevel("c6.mi_", "", "spider");
	// No c7 level?
	loadArcadeLevel("c8.mi_", "", prefix);
	loadArcadeLevel("c9.mi_", "", prefix);
	loadArcadeLevel("c10.mi_", "", prefix);
	loadArcadeLevel("c11.mi_", "", prefix);
	loadArcadeLevel("c12.mi_", "", prefix);
	loadArcadeLevel("c13.mi_", "", prefix);

	// // Hard arcade levels

	//loadArcadeLevel("c6h.mi_", "", "spider");
	// No c7h level?
	loadArcadeLevel("c8h.mi_", "", prefix);
	loadArcadeLevel("c9h.mi_", "", prefix);
	loadArcadeLevel("c10h.mi_", "", prefix);
	loadArcadeLevel("c11h.mi_", "", prefix);
	loadArcadeLevel("c12h.mi_", "", prefix);
	loadArcadeLevel("c13h.mi_", "", prefix);

	// start level
	Transition *start = new Transition("mainmenu.mi_");
	start->intros.push_back("spider/cine/dcine1.smk");
	start->intros.push_back("spider/cine/dcine2.smk");
	_levels["<start>"] = start;

	sc = (Scene *) _levels["mainmenu.mi_"];
	cl = new ChangeLevel("levels.mi_");
	sc->hots[1].actions.push_back(cl);
	
	cl = new ChangeLevel("options.mi_");
	sc->hots[4].actions.push_back(cl);

	cl = new ChangeLevel("<quit>");
	sc->hots[5].actions.push_back(cl);

	sc = (Scene *) _levels["levels.mi_"];
	cl = new ChangeLevel("levels.mi_");
	sc->hots[1].actions.push_back(cl);
	sc->hots[2].actions.push_back(cl);
	sc->hots[3].actions.push_back(cl);
	sc->hots[4].actions.push_back(cl);

	cl = new ChangeLevel("mv0t.mi_");
	sc->hots[5].actions.push_back(cl);
	cl = new ChangeLevel("mainmenu.mi_");
	sc->hots[6].actions.push_back(cl);

	sc = (Scene *) _levels["options.mi_"]; 

	cl = new ChangeLevel("combmenu.mi_");
	sc->hots[1].actions.push_back(cl);

	cl = new ChangeLevel("mainmenu.mi_");
	sc->hots[4].actions.push_back(cl);

	cl = new ChangeLevel("<credits>");
	sc->hots[5].actions.push_back(cl);

	sc = (Scene *) _levels["combmenu.mi_"]; 

	cl = new ChangeLevel("options.mi_");
	sc->hots[1].actions.push_back(cl);

	cl = new ChangeLevel("c1h.mi_");
	sc->hots[2].actions.push_back(cl);

	cl = new ChangeLevel("c2h.mi_");
	sc->hots[3].actions.push_back(cl);

	cl = new ChangeLevel("c3h.mi_");
	sc->hots[4].actions.push_back(cl);

	cl = new ChangeLevel("c4h.mi_");
	sc->hots[5].actions.push_back(cl);

	cl = new ChangeLevel("c5h.mi_");
	sc->hots[6].actions.push_back(cl);

	cl = new ChangeLevel("c8h.mi_");
	sc->hots[7].actions.push_back(cl);
	
	cl = new ChangeLevel("c9h.mi_");
	sc->hots[8].actions.push_back(cl);

	cl = new ChangeLevel("c10h.mi_");
	sc->hots[9].actions.push_back(cl);

	cl = new ChangeLevel("c11h.mi_");
	sc->hots[10].actions.push_back(cl);

	cl = new ChangeLevel("c12h.mi_");
	sc->hots[11].actions.push_back(cl);

	// Game overs
	Transition *over_apt_1 = new Transition("tryagain.mi_");
	over_apt_1->intros.push_back("spider/cine/apts01as.smk");
	_levels["<over_apt_1>"] = over_apt_1;
	_nextLevel = "<start>";
}

void SpiderEngine::loadAssetsDemo() {

	if (!_installerArchive.open("DATA.Z"))
		error("Failed to open DATA.Z");

	SearchMan.add("DATA.Z", (Common::Archive *) &_installerArchive, 0, false);

	Common::ArchiveMemberList files;
	LibFile *missions = loadLib("", "sixdemo/c_misc/missions.lib", true);
	if (missions == nullptr || missions->listMembers(files) == 0)
		error("Failed to load any file from missions.lib");

	// start level
	Transition *start = new Transition("sixdemo/mis/demo.mis");
	start->intros.push_back("sixdemo/demo/dcine1.smk");
	start->intros.push_back("sixdemo/demo/dcine2.smk");
	_levels["<start>"] = start;

	loadArcadeLevel("c1.mi_", "sixdemo/mis/demo.mis", "sixdemo");

	loadLib("", "sixdemo/c_misc/fonts.lib", true);
	loadLib("sixdemo/c_misc/sound.lib/", "sixdemo/c_misc/sound.lib", true);
	loadLib("sixdemo/demo/sound.lib/", "sixdemo/demo/sound.lib", true);

	// Read assets from mis files
	loadSceneLevel("sixdemo/mis/demo.mis", "", "sixdemo");
	ChangeLevel *cl = new ChangeLevel("c1.mi_");

	Scene *sc = (Scene *) _levels["sixdemo/mis/demo.mis"];
	sc->hots[1].actions.push_back(cl);

	cl = new ChangeLevel("sixdemo/mis/alley.mis");
	sc->hots[2].actions.push_back(cl);

	cl = new ChangeLevel("<puz_matr>");
	sc->hots[3].actions.push_back(cl);

	cl = new ChangeLevel("sixdemo/mis/shoctalk.mis");
	sc->hots[4].actions.push_back(cl);

	cl = new ChangeLevel("sixdemo/mis/order.mis");
	sc->hots[5].actions.push_back(cl);
	sc->music = "demo/sound.lib/menu_mus.raw";

	loadSceneLevel("sixdemo/mis/order.mis", "", "sixdemo");
	sc = (Scene *) _levels["sixdemo/mis/order.mis"];
	cl = new ChangeLevel("<quit>");
	sc->hots[1].actions.push_back(cl);

	loadSceneLevel("sixdemo/mis/alley.mis", "", "sixdemo");
	sc = (Scene *) _levels["sixdemo/mis/alley.mis"];

	sc->intros.push_back("demo/aleyc01s.smk");
	sc->music = "demo/sound.lib/alleymus.raw";
	sc->levelIfWin = "sixdemo/mis/demo.mis";
	sc->levelIfLose = "sixdemo/mis/demo.mis";

	loadSceneLevel("sixdemo/mis/shoctalk.mis", "", "sixdemo");

	Code *matrix = new Code();
	matrix->name = "<puz_matr>";
	matrix->intros.push_back("sixdemo/demo/aleyc01s.smk");
	matrix->levelIfWin = "sixdemo/mis/demo.mis";
	matrix->levelIfLose = "sixdemo/mis/demo.mis";
	_levels["<puz_matr>"] = matrix;
	_soundPath = "c_misc/sound.lib/";
	_nextLevel = "<start>";
}

void SpiderEngine::runCode(Code *code) {
	if (code->name == "<puz_matr>")
		runMatrix(code);
	else if (code->name == "<note>")
		runNote(code);
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

void SpiderEngine::showCredits() {
	changeScreenMode("640x480");
	MVideo video("cine/credits.smk", Common::Point(0, 0), false, false, false);
	runIntro(video);
}

Common::String SpiderEngine::findNextLevel(const Transition *trans) { 
	if (trans->nextLevel.empty())
		return _sceneState["GS_PUZZLELEVEL"] == 0 ? trans->levelEasy : trans->levelHard;

	return trans->nextLevel;
}

Common::String SpiderEngine::findNextLevel(const Common::String &level) {
	if (Common::matchString(level.c_str(), "c#") || Common::matchString(level.c_str(), "c##"))
		return level + (_sceneState["GS_COMBATLEVEL"] == 0 ? "" : "h") + ".mi_";
	else {
		return level;
	}
}

} // End of namespace Hypno
