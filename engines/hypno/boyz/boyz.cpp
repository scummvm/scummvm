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
}

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
	_levels["<main_menu>"]->levelIfWin = "c19.mi_";

	loadArcadeLevel("c19.mi_", "c11.mi_", "??", "");
	loadArcadeLevel("c11.mi_", "c12.mi_", "??", "");
	loadArcadeLevel("c12.mi_", "c14.mi_", "??", "");
	//loadArcadeLevel("c13.mi_", "??", "??", "");
	loadArcadeLevel("c14.mi_", "c15.mi_", "??", "");
	loadArcadeLevel("c15.mi_", "c16.mi_", "??", "");
	loadArcadeLevel("c16.mi_", "c17.mi_", "??", "");
	loadArcadeLevel("c17.mi_", "c18.mi_", "??", "");
	loadArcadeLevel("c18.mi_", "c21.mi_", "??", "");

	loadLib("sound/", "misc/sound.lib", true);

	_weaponShootSound[0] = "pstlfire.raw";
	_weaponShootSound[1] = "ak47fire.raw";
	_weaponShootSound[2] = "dblfire.raw";
	_weaponShootSound[3] = "m16fire.raw";
	_weaponShootSound[4] = "shotfire.raw";
	_weaponShootSound[5] = "glm60fr.raw";

	Graphics::Surface *targets = decodeFrame("preload/targets.smk", 0, &_crosshairsPalette);

	Common::Rect cursorBox;

	// Pistol?
	cursorBox = Common::Rect(62, 6, 83, 26);
	_crosshairsInactive[0].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsInactive[0].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(62, 38, 83, 58);
	_crosshairsActive[0].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsActive[0].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(62, 70, 83, 90);
	_crosshairsTarget[0].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsTarget[0].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(16, 8, 32, 24);
	_crosshairsInactive[2].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsInactive[2].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(16, 40, 32, 56);
	_crosshairsActive[2].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsActive[2].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(16, 72, 32, 88);
	_crosshairsTarget[2].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsTarget[2].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(163, 11, 173, 21);
	_crosshairsInactive[3].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsInactive[3].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(163, 43, 173, 53);
	_crosshairsActive[3].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsActive[3].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(163, 75, 173, 85);
	_crosshairsTarget[3].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsTarget[3].copyRectToSurface(*targets, 0, 0, cursorBox);

	// Shotgun
	cursorBox = Common::Rect(104, 7, 136, 25);
	_crosshairsInactive[4].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsInactive[4].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(104, 39, 136, 57);
	_crosshairsActive[4].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsActive[4].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(104, 71, 136, 83);
	_crosshairsTarget[4].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsTarget[4].copyRectToSurface(*targets, 0, 0, cursorBox);

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

	targets->free();
	delete targets;

	loadLib("", "misc/fonts.lib", true);
	loadFonts();

	_nextLevel = "<start>";
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
