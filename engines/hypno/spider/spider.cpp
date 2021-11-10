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

	Scene *sc;
	ChangeLevel *cl;

	loadSceneLevel("mainmenu.mi_", "", prefix);
	loadSceneLevel("tryagain.mi_", "", prefix);
	loadSceneLevel("options.mi_", "", prefix);
	loadSceneLevel("levels.mi_", "mv0t.mi_", prefix);
	loadSceneLevel("combmenu.mi_", "", prefix);

	loadSceneLevel("mv0t.mi_", "roof.mi_", prefix);
	_levels["mv0t.mi_"]->intros.push_back("cine/ints001s.smk");

	loadSceneLevel("roof.mi_", "decide1.mi_", prefix);
	loadSceneLevel("decide1.mi_", "", prefix);
	sc = (Scene *) _levels["decide1.mi_"];
	cl = new ChangeLevel("bank.mi_");
	sc->hots[1].actions.push_back(cl);
	cl = new ChangeLevel("c1.mi_");
	sc->hots[4].actions.push_back(cl);

	loadSceneLevel("bank.mi_", "", prefix);
	loadSceneLevel("busintro.mi_", "buspuz.mi_", prefix);
	loadSceneLevel("buspuz.mi_", "", prefix);

	loadArcadeLevel("c1", "", "spider");
	loadArcadeLevel("c2", "", "spider");
	loadArcadeLevel("c3", "", "spider");
	loadArcadeLevel("c4", "", "spider");
	loadArcadeLevel("c5", "", "spider");
	//loadArcadeLevel("c6", "", "spider");
	// No c7 level?
	loadArcadeLevel("c8", "", "spider");
	loadArcadeLevel("c9", "", "spider");
	loadArcadeLevel("c10", "", "spider");
	loadArcadeLevel("c11", "", "spider");
	loadArcadeLevel("c12", "", "spider");
	loadArcadeLevel("c13", "", "spider");

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

	sc = (Scene *) _levels["combmenu.mi_"]; 

	cl = new ChangeLevel("options.mi_");
	sc->hots[1].actions.push_back(cl);

	cl = new ChangeLevel("c1.mi_");
	sc->hots[2].actions.push_back(cl);

	cl = new ChangeLevel("c2.mi_");
	sc->hots[3].actions.push_back(cl);

	cl = new ChangeLevel("c5.mi_");
	sc->hots[6].actions.push_back(cl);

	cl = new ChangeLevel("c8.mi_");
	sc->hots[7].actions.push_back(cl);
	
	cl = new ChangeLevel("c9.mi_");
	sc->hots[8].actions.push_back(cl);
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

	cl = new ChangeLevel("sixdemo/puz_matr");
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
	matrix->name = "sixdemo/puz_matr";
	matrix->intros.push_back("spiderman/demo/aleyc01s.smk");
	matrix->levelIfWin = "sixdemo/mis/demo.mis";
	matrix->levelIfLose = "sixdemo/mis/demo.mis";
	_levels["sixdemo/puz_matr"] = matrix;
	_soundPath = "c_misc/sound.lib/";
}

void SpiderEngine::runCode(Code *code) {
	if (code->name == "sixdemo/puz_matr")
		runMatrix(code);
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

	loadImage("sixdemo/puz_matr/matrixbg.smk", 0, 0, false);
	MVideo v("sixdemo/puz_matr/matintro.smk", Common::Point(0, 0), false, false, false);
	playVideo(v);
	while (!shouldQuit()) {

		while (g_system->getEventManager()->pollEvent(event)) {
			mousePos = g_system->getEventManager()->getMousePos();
			// Events
			switch (event.type) {

			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				break;

			case Common::EVENT_LBUTTONDOWN:
				playSound("sixdemo/demo/sound.lib/matrix.raw", 1);
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
			playSound("sixdemo/demo/sound.lib/matrix_2.raw", 1);
			_nextLevel = code->levelIfWin;
			return;
		}

		if (v.decoder->needsUpdate()) {
			updateScreen(v);
		}

		drawScreen();
		g_system->delayMillis(10);
	}
}

} // End of namespace Hypno
