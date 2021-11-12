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
	credits->name = "credits";
	credits->prefix = prefix;
	_levels["credits"] = credits;

	Scene *sc;
	ChangeLevel *cl;

	loadSceneLevel("mainmenu.mi_", "", prefix);
	loadSceneLevel("tryagain.mi_", "", prefix);

	cl = new ChangeLevel("mainmenu.mi_");
	sc = (Scene *) _levels["tryagain.mi_"];
	sc->hots[1].actions.push_back(cl);

	loadSceneLevel("options.mi_", "", prefix);
	loadSceneLevel("levels.mi_", "mv0t.mi_", prefix);
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
	cl = new ChangeLevel("c1.mi_");
	sc->hots[4].actions.push_back(cl);

	loadSceneLevel("bank.mi_", "", prefix);
	_levels["bank.mi_"]->intros.push_back("cine/swcs001s.smk");
	_levels["bank.mi_"]->levelIfWin = "alley.mi_";

	loadSceneLevel("busintro.mi_", "buspuz.mi_", prefix);
	loadSceneLevel("buspuz.mi_", "", prefix);
	_levels["buspuz.mi_"]->intros.push_back("cine/apts004s.smk");
	_levels["buspuz.mi_"]->intros.push_back("cine/blcs001s.smk");
	_levels["buspuz.mi_"]->intros.push_back("cine/ppv001s.smk");

	loadSceneLevel("alley.mi_", "<bank_easy>", prefix);
	_levels["alley.mi_"]->music = "alleymus.raw";
	_levels["alley.mi_"]->intros.push_back("cine/aleyc01s.smk");

	loadArcadeLevel("c1", "<trans_apt_1>", prefix);
	_levels["c1.mi_"]->intros.push_back("cine/ross002s.smk");
	_levels["c1.mi_"]->levelIfLose = "<over_apt_1>";

	Transition *trans_apt_1 = new Transition();
	trans_apt_1->level = "busintro.mi_";
	trans_apt_1->intros.push_back("spider/cine/apts002s.smk");
	_levels["<trans_apt_1>"] = trans_apt_1;

	Transition *bankEasy = new Transition();
	bankEasy->level = "buspuz.mi_";
	bankEasy->intros.push_back("spider/cine/dia002s.smk");
	_levels["<bank_easy>"] = bankEasy;

	// Transition *bankHard = new Transition();
	// bankHard->level = "buspuz.mi_";
	// bankHard->intros.push_back("spider/cine/bals003s.smk");
	// _levels["<bank_hard>"] = bankHard;

	// Easy arcade levels
	loadArcadeLevel("c2", "", prefix);
	loadArcadeLevel("c3", "", prefix);
	loadArcadeLevel("c4", "", prefix);
	loadArcadeLevel("c5", "", prefix);
	//loadArcadeLevel("c6", "", "spider");
	// No c7 level?
	loadArcadeLevel("c8", "", prefix);
	loadArcadeLevel("c9", "", prefix);
	loadArcadeLevel("c10", "", prefix);
	loadArcadeLevel("c11", "", prefix);
	loadArcadeLevel("c12", "", prefix);
	loadArcadeLevel("c13", "", prefix);

	// // Hard arcade levels
	// loadArcadeLevel("c1h", "", prefix);
	// loadArcadeLevel("c2h", "", prefix);
	// loadArcadeLevel("c3h", "", prefix);
	// loadArcadeLevel("c4h", "", prefix);
	// loadArcadeLevel("c5h", "", prefix);
	// //loadArcadeLevel("c6h", "", "spider");
	// // No c7h level?
	// loadArcadeLevel("c8h", "", prefix);
	// loadArcadeLevel("c9h", "", prefix);
	// loadArcadeLevel("c10h", "", prefix);
	// loadArcadeLevel("c11h", "", prefix);
	// loadArcadeLevel("c12h", "", prefix);
	// loadArcadeLevel("c13h", "", prefix);


	// // Hardcoded levels
	// Code *matrix = new Code();
	// matrix->name = "<puz_matr>";
	// matrix->intros.push_back("spider/cine/aleyc01s.smk");
	// matrix->levelIfWin = "";
	// _levels["<puz_matr>"] = matrix;

	// start level
	Transition *start = new Transition();
	start->level = "mainmenu.mi_";
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

	cl = new ChangeLevel("c1.mi_");
	sc->hots[2].actions.push_back(cl);

	cl = new ChangeLevel("c2.mi_");
	sc->hots[3].actions.push_back(cl);

	cl = new ChangeLevel("c3.mi_");
	sc->hots[4].actions.push_back(cl);

	cl = new ChangeLevel("c4.mi_");
	sc->hots[5].actions.push_back(cl);

	cl = new ChangeLevel("c5.mi_");
	sc->hots[6].actions.push_back(cl);

	cl = new ChangeLevel("c8.mi_");
	sc->hots[7].actions.push_back(cl);
	
	cl = new ChangeLevel("c9.mi_");
	sc->hots[8].actions.push_back(cl);

	cl = new ChangeLevel("c10.mi_");
	sc->hots[9].actions.push_back(cl);

	cl = new ChangeLevel("c11.mi_");
	sc->hots[10].actions.push_back(cl);

	cl = new ChangeLevel("c12.mi_");
	sc->hots[11].actions.push_back(cl);

	// Game overs
	Transition *over_apt_1 = new Transition();
	over_apt_1->level = "tryagain.mi_";
	over_apt_1->intros.push_back("spider/cine/apts01as.smk");
	_levels["<over_apt_1>"] = over_apt_1;
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
	Transition *start = new Transition();
	start->level = "sixdemo/mis/demo.mis";
	start->intros.push_back("sixdemo/demo/dcine1.smk");
	start->intros.push_back("sixdemo/demo/dcine2.smk");
	_levels["<start>"] = start;

	loadArcadeLevel("c1", "sixdemo/mis/demo.mis", "sixdemo");

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

	cl = new ChangeLevel("puz_matr");
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
	matrix->intros.push_back("spiderman/demo/aleyc01s.smk");
	matrix->levelIfWin = "sixdemo/mis/demo.mis";
	matrix->levelIfLose = "sixdemo/mis/demo.mis";
	_levels["<puz_matr>"] = matrix;
	_soundPath = "c_misc/sound.lib/";
}

void SpiderEngine::runCode(Code *code) {
	if (code->name == "<puz_matr>")
		runMatrix(code);
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

void SpiderEngine::showCredits() {
	MVideo video("cine/credits.smk", Common::Point(0, 0), false, false, false);
	runIntro(video);
	_nextLevel = "mainmenu.mi_";
}

} // End of namespace Hypno
