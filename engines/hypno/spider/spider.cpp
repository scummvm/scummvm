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
	if (language == Common::EN_USA) {
		if (!_installerArchive.open("DATA.Z"))
			error("Failed to open DATA.Z");

		SearchMan.add("DATA.Z", (Common::Archive *) &_installerArchive, 0, false);
	}

	Common::ArchiveMemberList files;
	LibFile *missions = loadLib("", "spider/c_misc/missions.lib", true);
	if (missions == nullptr || missions->listMembers(files) == 0)
		error("Failed to load any file from missions.lib");

	parseScene("", "mainmenu.mi_");
	_levels["mainmenu.mi_"].scene.prefix = "spider";

	parseScene("", "options.mi_");
	_levels["options.mi_"].scene.prefix = "spider";

	parseScene("", "levels.mi_");
	_levels["levels.mi_"].scene.prefix = "spider";
	_levels["levels.mi_"].scene.levelIfWin = "mv0t.mi_";

	parseScene("", "combmenu.mi_");
	_levels["combmenu.mi_"].scene.prefix = "spider";

	parseScene("", "mv0t.mi_");
	_levels["mv0t.mi_"].scene.prefix = "spider";

	// start level
	Level start;
	start.trans.level = "mainmenu.mi_";
	start.trans.intros.push_back("spider/cine/dcine1.smk");
	start.trans.intros.push_back("spider/cine/dcine2.smk");
	_levels["<start>"] = start;

	ChangeLevel *cl = new ChangeLevel("levels.mi_");
	_levels["mainmenu.mi_"].scene.hots[1].actions.push_back(cl);
	
	cl = new ChangeLevel("options.mi_");
	_levels["mainmenu.mi_"].scene.hots[4].actions.push_back(cl);

	cl = new ChangeLevel("<quit>");
	_levels["mainmenu.mi_"].scene.hots[5].actions.push_back(cl);

	cl = new ChangeLevel("combmenu.mi_");
	_levels["options.mi_"].scene.hots[1].actions.push_back(cl);
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
	Level start;
	start.trans.level = "sixdemo/mis/demo.mis";
	start.trans.intros.push_back("sixdemo/demo/dcine1.smk");
	start.trans.intros.push_back("sixdemo/demo/dcine2.smk");
	_levels["<start>"] = start;

	Common::String arc;
	Common::String list;
	Common::String arclevel = files.front()->getName();
	Common::SeekableReadStream *file = files.front()->createReadStream();

	while (!file->eos()) {
		byte x = file->readByte();
		arc += x;
		if (x == 'X') {
			while (!file->eos()) {
				x = file->readByte();
				if (x == 'Y')
					break;
				list += x;
			}
			break; // No need to keep parsing
		}
	}
	delete file;

	arclevel = "sixdemo/c_misc/missions.lib/" + arclevel;
	parseArcadeShooting("sixdemo", arclevel, arc);
	_levels[arclevel].arcade.shootSequence = parseShootList(arclevel, list);
	_levels[arclevel].arcade.levelIfWin = "sixdemo/mis/demo.mis";
	_levels[arclevel].arcade.levelIfLose = "sixdemo/mis/demo.mis";

	loadLib("", "sixdemo/c_misc/fonts.lib", true);
	loadLib("sixdemo/c_misc/sound.lib/", "sixdemo/c_misc/sound.lib", true);
	loadLib("sixdemo/demo/sound.lib/", "sixdemo/demo/sound.lib", true);

	// Read assets from mis files
	parseScene("sixdemo", "mis/demo.mis");
	ChangeLevel *cl = new ChangeLevel("sixdemo/c_misc/missions.lib/c1.mi_");
	_levels["sixdemo/mis/demo.mis"].scene.hots[1].actions.push_back(cl);

	cl = new ChangeLevel("sixdemo/mis/alley.mis");
	_levels["sixdemo/mis/demo.mis"].scene.hots[2].actions.push_back(cl);

	cl = new ChangeLevel("sixdemo/puz_matr");
	_levels["sixdemo/mis/demo.mis"].scene.hots[3].actions.push_back(cl);

	cl = new ChangeLevel("sixdemo/mis/shoctalk.mis");
	_levels["sixdemo/mis/demo.mis"].scene.hots[4].actions.push_back(cl);

	cl = new ChangeLevel("sixdemo/mis/order.mis");
	_levels["sixdemo/mis/demo.mis"].scene.hots[5].actions.push_back(cl);
	_levels["sixdemo/mis/demo.mis"].scene.sound = "demo/sound.lib/menu_mus.raw";

	parseScene("sixdemo", "mis/order.mis");
	cl = new ChangeLevel("<quit>");
	_levels["sixdemo/mis/order.mis"].scene.hots[1].actions.push_back(cl);

	parseScene("sixdemo", "mis/alley.mis");
	_levels["sixdemo/mis/alley.mis"].scene.intro = "demo/aleyc01s.smk";
	_levels["sixdemo/mis/alley.mis"].scene.sound = "demo/sound.lib/alleymus.raw";
	_levels["sixdemo/mis/alley.mis"].scene.levelIfWin = "sixdemo/mis/demo.mis";
	_levels["sixdemo/mis/alley.mis"].scene.levelIfLose = "sixdemo/mis/demo.mis";

	parseScene("sixdemo", "mis/shoctalk.mis");

	Level matrix;
	matrix.code.name = "sixdemo/puz_matr";
	matrix.code.intros.push_back("spiderman/demo/aleyc01s.smk");
	matrix.code.levelIfWin = "sixdemo/mis/demo.mis";
	matrix.code.levelIfLose = "sixdemo/mis/demo.mis";
	_levels["sixdemo/puz_matr"] = matrix;
	_soundPath = "c_misc/sound.lib/";
}

void SpiderEngine::runCode(Code code) {
	if (code.name == "sixdemo/puz_matr")
		runMatrix(code);
	else
		error("invalid puzzle");
}

void SpiderEngine::runMatrix(Code code) {
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
			_nextLevel = code.levelIfWin;
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
