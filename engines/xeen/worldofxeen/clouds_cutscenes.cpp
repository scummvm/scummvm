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

#include "xeen/worldofxeen/clouds_cutscenes.h"
#include "xeen/resources.h"
#include "xeen/sound.h"

namespace Xeen {

bool CloudsCutscenes::showCloudsTitle() {
	EventsManager &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;

	// Initial logo display
	screen.loadPalette("intro1.pal");
	screen.loadBackground("logobak.raw");
	screen.saveBackground();
	screen.update();
	screen.fadeIn(128);

	SpriteResource logo[2] = {
		SpriteResource("logo.vga"), SpriteResource("logo1.vga")
	};
sound.playFX(1);

for (int idx = 0; idx < 80; ++idx) {
	screen.restoreBackground();
	logo[idx / 65].draw(screen, idx % 65);
	screen.update();

	switch (idx) {
	case 37:
		sound.playFX(0);
		sound.playFX(53);
		sound.playSound("fire.voc");
		break;
	case 52:
	case 60:
		sound.playFX(3);
		break;
	case 64:
		sound.playFX(2);
		break;
	case 66:
		sound.playFX(52);
		sound.playSound("meangro&.voc");
		break;
	default:
		break;
	}

	WAIT(2);
}

screen.restoreBackground();
screen.update();
WAIT(30);

screen.fadeOut(8);
logo[0].clear();
logo[1].clear();

return true;
}

bool CloudsCutscenes::showCloudsIntro() {
	EventsManager &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	SpriteResource stars("stars.vga"), intro1("intro1.vga"),
		lake("lake.vga"), xeen("xeen.vga"), wizTower("wiztower.vga"),
		wizTower2("wiztwer2.vga"), lake2("lake2.vga"), lake3("lake3.vga"),
		xeen1("xeen1.vga");

	// Show the production splash screen
	sound.playSong("mm4theme.m");
	screen.loadBackground("jvc.raw");
	screen.loadPalette("mm4.pal");
	screen.update();
	screen.fadeIn();
	WAIT(30);
	screen.fadeOut(8);

	// Clouds of Xeen title with vertically scrolling background
	screen.loadPalette("intro.pal");
	screen.loadBackground("blank.raw");
	screen.saveBackground();
	stars.draw(screen, 0);
	stars.draw(screen, 1, Common::Point(160, 0));
	screen.loadPage(0);
	intro1.draw(screen, 0);
	screen.loadPage(1);

	bool fadeFlag = true;
	for (int yCtr = SCREEN_HEIGHT, yScroll = 0, xeenCtr = -1; yCtr > 0; --yCtr, ++yScroll) {
		screen.vertMerge(yScroll);
		if (yCtr < 160) {
			xeen.draw(screen, 0);
		}
		else if (yCtr < 100) {
			xeen.draw(screen, 0);
			if (++xeenCtr < 14)
				xeen1.draw(screen, xeenCtr);
		}
		screen.update();

		if (fadeFlag) {
			screen.fadeIn();
			fadeFlag = false;
		}

		WAIT(1);
	}

	// Remainder of vertical scrolling of background
	screen.restoreBackground();
	intro1.draw(screen, 0);
	screen.loadPage(0);
	lake.draw(screen, 0);
	screen.loadPage(1);

	bool drawFlag = false;
	for (int yCtr = SCREEN_HEIGHT - 1, yScroll = 1, lakeCtr = 0; yCtr > 0; --yCtr, ++yScroll) {
		screen.vertMerge(yScroll);

		if (yCtr < 95) {
			if (++lakeCtr >= 44 || drawFlag) {
				lakeCtr = 0;
				drawFlag = true;
			} else {
				lake3.draw(screen, lakeCtr, Common::Point(0, yCtr));
			}
		}
		
		xeen.draw(screen, 0);
		screen.update();
		WAIT(1);
	}

	screen.freePages();

	// Flying creatures moving horizontally
	lake.draw(screen, 0);
	screen.saveBackground();

	int frameNum = 0;
	events.updateGameCounter();
	for (int idx = 0; idx < 100; ++idx) {
		frameNum = (frameNum + 1) % 43;
		screen.restoreBackground();
		lake2.draw(screen, frameNum, Common::Point(0, 0), SPRFLAG_800);
		WAIT(1);
	}

	// Zoom in on a closeup of the wizardry tower
	const int XLIST1[16] = { 0, 5, 10, 15, 20, 25, 30, 35, 40, 44, 48, 52, 56, 60, 64, 68 };
	const int XLIST2[16] = { 160, 155, 150, 145, 140, 135, 130, 125, 120, 114, 108, 102, 96, 90, 84, 78 };
	const int YLIST[23] = { 0, 6, 12, 17, 20, 23, 26, 29, 32, 35, 38, 41, 44, 47, 50, 51 };

	for (int idx = 15; idx >= 0; --idx) {
		events.updateGameCounter();

		screen.restoreBackground();
		lake2.draw(screen, frameNum, Common::Point(0, 0), SPRFLAG_800);
		frameNum = (frameNum + 1) % 43;
		wizTower.draw(screen, 0, Common::Point(XLIST1[idx], YLIST[idx]), 0, idx);
		wizTower.draw(screen, 1, Common::Point(XLIST2[idx], YLIST[idx]), 0, idx);
		screen.update();
		WAIT(1);
	}

	// Cloaked figure walks horizontally
	wizTower.draw(screen, 0);
	wizTower.draw(screen, 1, Common::Point(160, 0));
	screen.saveBackground();

	for (int idx = 0; idx < 39; ++idx) {
		screen.restoreBackground();
		wizTower2.draw(screen, idx);
		screen.update();

		WAIT(2);
	}

	screen.fadeOut();
	lake2.clear();
	lake3.clear();
	xeen1.clear();

	//
	const char *const VOCS[14] = {
		"crodo1.voc", "crodo2.voc", "iamking.voc", "crodo3.voc",
		"ya1.voc", "crodo4a.voc", "crodo4b.voc", "crodo4c.voc",
		"xeenlaff.voc", "tiger2&.voc", "crodo5.voc", "crodo6.voc",
		"xeenlaff.voc", "tiger2&.voc"
	};
	SpriteResource groupo("groupo.vga"), group("group.vga"), crodo("crodo.vga");

	groupo.draw(screen, 0);
	groupo.draw(screen, 1, Common::Point(160, 0));
	crodo.draw(screen, 0, Common::Point(0, -5));
	screen._windows[0].writeString(CLOUDS_INTRO1);

	// TODO

	events.wait(5000);
	return true;
}

bool CloudsCutscenes::showCloudsEnding() {
	// TODO
	return true;
}

} // End of namespace Xeen
