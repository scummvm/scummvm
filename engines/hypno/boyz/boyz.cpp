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

#include "hypno/grammar.h"
#include "hypno/hypno.h"

#include "common/events.h"

namespace Hypno {

BoyzEngine::BoyzEngine(OSystem *syst, const ADGameDescription *gd) : HypnoEngine(syst, gd) {
	_screenW = 320;
	_screenH = 200;
	_lives = 2;
	_currentWeapon = 0;
	_currentActor = 0;
	_currentMode = NonInteractive;
	_crosshairsPalette = nullptr;

	for (int i = 0; i < 6; i++) {
		_ammoTeam[i] = 0;
		_weaponMaxAmmo[i] = 0;
	}
}

static const char *selectBoyz = "\
MENU preload\\slct_boy.smk\n\
HOTS /BBOX= 19  3   66  199\n\
SOND tollop1.raw 22K\n\
HOTS /BBOX= 69  3   116 199\n\
SOND tollam.raw  22K\n\
HOTS /BBOX= 119 3   166 199\n\
SOND tolvas1.raw 22K\n\
HOTS /BBOX= 169 3   216 199\n\
SOND tolbut1.raw 22K\n\
HOTS /BBOX= 219 3   266 199\n\
SOND tolbro1.raw 22K\n\
HOTS /BBOX= 269 3   316 170\n\
SOND tolmon1.raw 22K\n\
HOTS /BBOX= 262 172 312 194\n\
SOND pstlfire.raw\n\
END\n";

void BoyzEngine::loadAssets() {
	LibFile *missions = loadLib("", "preload/missions.lib", true);
	Common::ArchiveMemberList files;
	if (missions->listMembers(files) == 0)
		error("Failed to load any files from missions.lib");

	Transition *logos = new Transition("<main_menu>");
	logos->intros.push_back("intro/dclogos.smk");
	logos->intros.push_back("intro/mplogos.smk");
	logos->intros.push_back("intro/hyplogos.smk");
	logos->intros.push_back("intro/sblogos.smk");
	_levels["<start>"] = logos;

	Code *menu = new Code("<main_menu>");
	_levels["<main_menu>"] = menu;
	_levels["<main_menu>"]->levelIfWin = "<select_boyz>";

	loadArcadeLevel("c19.mi_", "c11.mi_", "??", "");
	loadArcadeLevel("c11.mi_", "c12.mi_", "??", "");
	loadArcadeLevel("c12.mi_", "c14.mi_", "??", "");
	loadArcadeLevel("c14.mi_", "c13.mi_", "??", "");
	loadArcadeLevel("c13.mi_", "c15.mi_", "??", "");
	loadArcadeLevel("c15.mi_", "c16.mi_", "??", "");
	loadArcadeLevel("c16.mi_", "c17.mi_", "??", "");
	loadArcadeLevel("c17.mi_", "c18.mi_", "??", "");
	loadArcadeLevel("c18.mi_", "c21.mi_", "??", "");

	loadArcadeLevel("c21.mi_", "c22.mi_", "??", "");
	loadArcadeLevel("c22.mi_", "c23.mi_", "??", "");
	loadArcadeLevel("c31.mi_", "c32.mi_", "??", "");
	loadArcadeLevel("c32.mi_", "c33.mi_", "??", "");
	loadArcadeLevel("c33.mi_", "c34.mi_", "??", "");
	loadArcadeLevel("c34.mi_", "c35.mi_", "??", "");
	loadArcadeLevel("c35.mi_", "c352.mi_", "??", "");
	loadArcadeLevel("c352.mi_", "c353.mi_", "??", "");
	loadArcadeLevel("c353.mi_", "c354.mi_", "??", "");
	loadArcadeLevel("c354.mi_", "c355.mi_", "??", "");
	loadArcadeLevel("c355.mi_", "c36.mi_", "??", "");
	loadArcadeLevel("c36.mi_", "c41.mi_", "??", "");
	loadArcadeLevel("c41.mi_", "c42.mi_", "??", "");
	//loadArcadeLevel("c42.mi_", "c51.mi_", "??", "");

	loadArcadeLevel("c51.mi_", "c52.mi_", "??", "");
	loadArcadeLevel("c52.mi_", "c53.mi_", "??", "");
	loadArcadeLevel("c53.mi_", "c54.mi_", "??", "");
	loadArcadeLevel("c54.mi_", "c55.mi_", "??", "");
	loadArcadeLevel("c55.mi_", "c56.mi_", "??", "");
	loadArcadeLevel("c56.mi_", "c57.mi_", "??", "");
	loadArcadeLevel("c57.mi_", "c58.mi_", "??", "");
	loadArcadeLevel("c58.mi_", "c59.mi_", "??", "");
	loadArcadeLevel("c59.mi_", "<credits>", "??", "");

	loadSceneLevel(selectBoyz, "<select_boyz>", "", "");
	Scene *sc = (Scene *) _levels["<select_boyz>"];

	ChangeLevel *cl = new ChangeLevel("c19.mi_");
	sc->hots[7].actions.push_back(cl);

	loadLib("sound/", "misc/sound.lib", true);

	_weaponShootSound[0] = "";
	_weaponShootSound[1] = "pstlfire.raw";
	_weaponShootSound[2] = "ak47fire.raw";
	_weaponShootSound[3] = "dblfire.raw";
	_weaponShootSound[4] = "m16fire.raw";
	_weaponShootSound[5] = "shotfire.raw";
	_weaponShootSound[6] = "glm60fr.raw";

	Graphics::Surface *targets = decodeFrame("preload/targets.smk", 0, &_crosshairsPalette);

	Common::Rect cursorBox;

	// Pistol?
	cursorBox = Common::Rect(62, 6, 83, 26);
	_crosshairsInactive[1].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsInactive[1].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(62, 38, 83, 58);
	_crosshairsActive[1].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsActive[1].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(62, 70, 83, 90);
	_crosshairsTarget[1].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsTarget[1].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(16, 8, 32, 24);
	_crosshairsInactive[3].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsInactive[3].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(16, 40, 32, 56);
	_crosshairsActive[3].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsActive[3].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(16, 72, 32, 88);
	_crosshairsTarget[3].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsTarget[3].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(163, 11, 173, 21);
	_crosshairsInactive[4].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsInactive[4].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(163, 43, 173, 53);
	_crosshairsActive[4].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsActive[4].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(163, 75, 173, 85);
	_crosshairsTarget[4].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsTarget[4].copyRectToSurface(*targets, 0, 0, cursorBox);

	// Shotgun
	cursorBox = Common::Rect(104, 7, 136, 25);
	_crosshairsInactive[5].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsInactive[5].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(104, 39, 136, 57);
	_crosshairsActive[5].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsActive[5].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(104, 71, 136, 83);
	_crosshairsTarget[5].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsTarget[5].copyRectToSurface(*targets, 0, 0, cursorBox);

    // Double small
	/*
	cursorBox = Common::Rect(203, 11, 229, 21);
	_crosshairsInactive[4].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsInactive[4].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(203, 43, 229, 53);
	_crosshairsActive[4].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsActive[4].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(203, 75, 229, 85);
	_crosshairsTarget[4].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsTarget[4].copyRectToSurface(*targets, 0, 0, cursorBox);

	// Granade

	cursorBox = Common::Rect(256, 2, 273, 29);
	_crosshairsInactive[5].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsInactive[5].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(256, 34, 273, 61);
	_crosshairsActive[5].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsActive[5].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(256, 66, 273, 93);
	_crosshairsTarget[5].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsTarget[5].copyRectToSurface(*targets, 0, 0, cursorBox);

	// Large crossair

	cursorBox = Common::Rect(56, 98, 88, 125);
	_crosshairsInactive[6].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsInactive[6].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(57, 130, 88, 158);
	_crosshairsActive[6].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsActive[6].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(57, 162, 88, 189);
	_crosshairsTarget[6].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsTarget[6].copyRectToSurface(*targets, 0, 0, cursorBox);
	*/

	_weaponMaxAmmo[0] = 0;
	_weaponMaxAmmo[1] = 10;
	_weaponMaxAmmo[2] = 2; // large shotgun
	_weaponMaxAmmo[3] = 8;
	_weaponMaxAmmo[4] = 6; // pistol?
	_weaponMaxAmmo[5] = 6; // small shotgun
	_weaponMaxAmmo[6] = 0;

	_maxHealth = 100;
	_civiliansShoot = 0;
	_warningAnimals = "warnings/w01s.smk";
	_warningCivilians.push_back("warnings/w02s.smk");
	_warningCivilians.push_back("warnings/w03s.smk");
	_warningCivilians.push_back("warnings/w04s.smk");
	_warningCivilians.push_back("warnings/w05s.smk");

	// Set initial health for the team
	for (int i = 0; i < 7; i++) {
		_healthTeam[i] = _maxHealth;
	}

	targets->free();
	delete targets;

	loadLib("", "misc/fonts.lib", true);
	loadFonts();

	_nextLevel = "c11.mi_";
}

void BoyzEngine::loadFonts() {
	Common::File file;

	if (!file.open("block05.fgx"))
		error("Cannot open font");

	byte *font = (byte *)malloc(file.size());
	file.read(font, file.size());

	_font05.set_size(file.size()*8);
	_font05.set_bits((byte *)font);

	file.close();
	free(font);
	if (!file.open("scifi08.fgx"))
		error("Cannot open font");

	font = (byte *)malloc(file.size());
	file.read(font, file.size());

	_font08.set_size(file.size()*8);
	_font08.set_bits((byte *)font);

	free(font);
}

void BoyzEngine::drawString(const Common::String &font, const Common::String &str, int x, int y, int w, uint32 color) {
	int offset = 0;
	if (font == "block05.fgx") {
		for (uint32 c = 0; c < str.size(); c++) {

			offset = 0;
			if (str[c] == ':')
				offset = 1;
			else if (str[c] == '.')
				offset = 4;

			for (int i = 0; i < 5; i++) {
				for (int j = 0; j < 5; j++) {
					if (!_font05.get(275 + 40*str[c] + j*8 + i))
						_compositeSurface->setPixel(x + 5 - i + 6*c, offset + y + j, color);
				}
			}
		}
	} else if (font == "scifi08.fgx") {
		for (uint32 c = 0; c < str.size(); c++) {
			if (str[c] == 0)
				continue;
			assert(str[c] >= 32);
			offset = 0;
			if (str[c] == 't')
				offset = 0;
			else if (str[c] == 'i' || str[c] == '%')
				offset = 1;
			else if (Common::isLower(str[c]) || str[c] == ':')
				offset = 2;

			for (int i = 0; i < 6; i++) {
				for (int j = 0; j < 8; j++) {
					if (!_font08.get(1554 + 72*(str[c]-32) + j*8 + i))
						_compositeSurface->setPixel(x + 6 - i + 7*c, offset + y + j, color);
				}
			}
		}
	} else
		error("Invalid font: '%s'", font.c_str());
}


Common::String BoyzEngine::findNextLevel(const Transition *trans) {
	if (trans->nextLevel.empty())
		error("Invalid transition!");
	return trans->nextLevel;
}

Common::String BoyzEngine::findNextLevel(const Common::String &level) { return level; }

} // namespace Hypno
