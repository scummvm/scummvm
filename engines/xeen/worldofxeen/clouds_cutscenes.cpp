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
#include "xeen/sound.h"

namespace Xeen {
namespace WorldOfXeen {

#define WAIT(TIME) if (_subtitles.wait(TIME)) return false
#define ROTATE_BG screen.horizMerge(_mergeX); \
	_mergeX = (_mergeX + 1) % SCREEN_WIDTH
#define LOAD_VORTEX loadScreen(Common::String::format("vort%02u.frm", cloudsCtr)); \
	if (++cloudsCtr > 20) \
		cloudsCtr = 1

bool CloudsCutscenes::showCloudsIntro() {
	EventsManager &events = *g_vm->_events;
	FileManager &files = *g_vm->_files;
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;

	bool darkCc = files._ccNum;
	files.setGameCc(0);
	sound._musicSide = 0;
	_subtitles.reset();

	bool seenIntro = showCloudsTitle() && showCloudsIntroInner();

	events.clearEvents();

	// Roll up the scroll again
	sound.songCommand(50);
	doScroll(true, false);

	sound.stopAllAudio();
	screen.freePages();
	files.setGameCc(darkCc ? 1 : 0);

	return seenIntro;
}

bool CloudsCutscenes::showCloudsTitle() {
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
		logo[idx / 65].draw(0, idx % 65);
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

bool CloudsCutscenes::showCloudsIntroInner() {
	EventsManager &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	Windows &windows = *_vm->_windows;

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
	stars.draw(0, 0);
	stars.draw(0, 1, Common::Point(160, 0));
	screen.loadPage(0);
	intro1.draw(0, 0);
	screen.loadPage(1);

	bool fadeFlag = true;
	for (int yCtr = SCREEN_HEIGHT, yScroll = 0, xeenCtr = -1; yCtr > 0; --yCtr, ++yScroll) {
		screen.vertMerge(yScroll);
		if (yCtr < 160) {
			xeen.draw(0, 0);
		}
		if (yCtr < 100) {
			xeen.draw(0, 0);
			if (++xeenCtr < 14)
				xeen1.draw(0, xeenCtr);
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
	intro1.draw(0, 0);
	screen.loadPage(0);
	lake.draw(0, 0);
	screen.loadPage(1);

	bool drawFlag = false;
	for (int yCtr = SCREEN_HEIGHT - 1, yScroll = 1, lakeCtr = 0; yCtr > 0; --yCtr, ++yScroll) {
		screen.vertMerge(yScroll);

		if (yCtr < 95) {
			if (++lakeCtr >= 44 || drawFlag) {
				lakeCtr = 0;
				drawFlag = true;
			} else {
				lake3.draw(0, lakeCtr, Common::Point(0, yCtr));
			}
		}

		xeen.draw(0, 0);
		screen.update();
		WAIT(1);
	}

	screen.freePages();

	// Flying creatures moving horizontally
	lake.draw(0, 0);
	screen.saveBackground();

	int frameNum = 0;
	events.updateGameCounter();
	for (int idx = 0; idx < 100; ++idx) {
		frameNum = (frameNum + 1) % 43;
		screen.restoreBackground();
		lake2.draw(0, frameNum, Common::Point(0, 0), SPRFLAG_800);
		WAIT(1);
	}

	// Zoom in on a closeup of the wizardry tower
	const int XLIST1[16] = { 0, 5, 10, 15, 20, 25, 30, 35, 40, 44, 48, 52, 56, 60, 64, 68 };
	const int XLIST2[16] = { 160, 155, 150, 145, 140, 135, 130, 125, 120, 114, 108, 102, 96, 90, 84, 78 };
	const int YLIST[23] = { 0, 6, 12, 17, 20, 23, 26, 29, 32, 35, 38, 41, 44, 47, 50, 51 };

	for (int idx = 15; idx >= 0; --idx) {
		events.updateGameCounter();

		screen.restoreBackground();
		lake2.draw(0, frameNum, Common::Point(0, 0), SPRFLAG_800);
		frameNum = (frameNum + 1) % 43;
		wizTower.draw(0, 0, Common::Point(XLIST1[idx], YLIST[idx]), 0, idx);
		wizTower.draw(0, 1, Common::Point(XLIST2[idx], YLIST[idx]), 0, idx);
		screen.update();
		WAIT(1);
	}

	// Cloaked figure walks horizontally
	wizTower.draw(0, 0);
	wizTower.draw(0, 1, Common::Point(160, 0));
	screen.saveBackground();

	for (int idx = 0; idx < 39; ++idx) {
		screen.restoreBackground();
		wizTower2.draw(0, idx);
		screen.update();

		WAIT(2);
	}

	screen.fadeOut();
	lake2.clear();
	lake3.clear();
	xeen1.clear();

	// All the lines whilst the scroll is open
	SpriteResource groupo("groupo.vga"), group("group.vga"),
		crodo("crodo.vga"), box("box.vga");

	groupo.draw(0, 0);
	groupo.draw(0, 1, Common::Point(160, 0));
	crodo.draw(0, 0, Common::Point(0, -5));
	windows[0].writeString(Res.CLOUDS_INTRO1);

	// Unroll a scroll
	if (doScroll(false, true))
		return false;

	sound.setMusicPercent(60);
	screen.restoreBackground();
	screen.update();
	_subtitles.setLine(0);

	// Loop through each spoken line
	int ctr1 = 0, ctr2 = 0, ctr3 = 0, ctr4 = 0, ctr5 = 0, totalCtr = 0;
	for (int lineCtr = 0; lineCtr < 14; ++lineCtr) {
		if (lineCtr != 6 && lineCtr != 7) {
			// Set subtitle to display (presuming subtitles are turned on)
			switch (lineCtr) {
			case 0:
				_subtitles.setLine(0);
				break;
			case 1:
				_subtitles.setLine(1);
				break;
			case 5:
				_subtitles.setLine(2);
				break;
			case 11:
				_subtitles.setLine(3);
				break;
			default:
				break;
			}

			// Play the next sample
			sound.playVoice(_INTRO_VOCS[lineCtr]);
		}

		events.timeMark1();
		for (int frameCtr = 0, lookup = 0; sound.isSoundPlaying() ||
				(_subtitles.active() && (lineCtr == 0 || lineCtr == 4 || lineCtr == 10 || lineCtr == 13)); ) {
			groupo.draw(0, 0);
			groupo.draw(0, 1, Common::Point(160, 0));

			switch (lineCtr) {
			case 2:
				ctr1 = (ctr1 + 1) % 5;
				group.draw(0, ctr1);
				ctr4 = (ctr4 + 1) % 9;
				break;

			case 4:
				ctr4 = (ctr4 + 1) % 9 + 9;
				break;

			case 8:
			case 12:
				ctr2 = (ctr2 + 1) % 3;
				ctr4 = (ctr4 + 1) % 9;
				ctr3 = (ctr3 + 1) % 6 + 3;
				break;

			case 9:
			case 13:
				ctr3 = (ctr3 + 1) % 3;
				group.draw(0, ctr3 + 43, Common::Point(178, 134));
				ctr4 = (ctr4 + 1) % 9;
				ctr2 = (ctr2 % 15) + 3;
				break;

			default:
				ctr4 = (ctr4 + 1) % 9;
				ctr2 = (ctr2 + 1) % 15 + 3;
				ctr3 = (ctr3 + 1) % 6 + 3;
				break;
			}

			group.draw(0, ctr4 + 5, Common::Point(0, 99));
			group.draw(0, ctr2 + 24, Common::Point(202, 12));
			if ((++totalCtr % 30) == 0)
				group.draw(0, 43, Common::Point(178, 134));

			switch (lineCtr) {
			case 2:
			case 4:
			case 8:
			case 9:
			case 12:
			case 13: {
				crodo.draw(0, 0, Common::Point(0, -5));
				windows[0].writeString(Res.CLOUDS_INTRO1);

				ctr5 = (ctr5 + 1) % 19;

				WAIT(1);
				continue;
			}

			default:
				crodo.draw(0, frameCtr, Common::Point(0, -5));
				if (lookup > 30)
					lookup = 30;
				frameCtr = _INTRO_FRAMES_VALS[_INTRO_FRAMES_LOOKUP[lineCtr]][lookup];
				windows[0].writeString(Res.CLOUDS_INTRO1);

				ctr5 = (ctr5 + 1) % 19;
				break;
			}

			uint expiry = _INTRO_FRAMES_WAIT[_INTRO_FRAMES_LOOKUP[lineCtr]][lookup];
			do {
				WAIT(1);
			} while (events.timeElapsed1() < expiry);

			++lookup;
			if (!sound._fxOn && lookup > 30)
				lookup = 0;
		}

		if (!sound._fxOn)
			lineCtr = 20;

		if (lineCtr == 5)
			sound.playVoice(_INTRO_VOCS[6]);
		else if (lineCtr == 6)
			sound.playVoice(_INTRO_VOCS[7]);
	}

	return true;
}

void CloudsCutscenes::showCloudsEnding(uint finalScore) {
	EventsManager &events = *g_vm->_events;
	FileManager &files = *g_vm->_files;
	Sound &sound = *g_vm->_sound;

	bool darkCc = files._ccNum;
	files.setGameCc(0);
	_subtitles.reset();

	_mirror.load("mirror.end");
	_mirrBack.load("mirrback.end");
	_mergeX = 0;
	doScroll(true, false);

	if (showCloudsEnding1())
		if (showCloudsEnding2())
			if (showCloudsEnding3())
				if (showCloudsEnding4(finalScore))
					showCloudsEnding5();

	events.clearEvents();
	sound.stopAllAudio();
	files.setGameCc(darkCc ? 1 : 0);

	if (!g_vm->shouldExit())
		doScroll(true, false);
}

bool CloudsCutscenes::showCloudsEnding1() {
	FileManager &files = *_vm->_files;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;

	files._ccNum = false;
	files.setGameCc(0);

	// Show the castle with swirling clouds and lightning
	SpriteResource prec;
	prec.load("prec.end");
	screen.loadBackground("blank.raw");
	screen.loadPalette("mm4e.pal");

	loadScreen(Common::String::format("prec00%02u.frm", 1));
	prec.draw(0, 0);
	prec.draw(0, 1, Common::Point(160, 0));
	screen.fadeIn();
	WAIT(15);

	sound.playFX(1);
	sound.playFX(34);

	// Initial animation of vortex & lightning in the sky
	for (int idx = 1; idx < 42; ++idx) {
		// Load up the background frame of swirling clouds
		loadScreen(Common::String::format("prec00%02u.frm", idx));

		// Render castle in front of it
		prec.draw(0, 0, Common::Point(0, 0));
		prec.draw(0, 1, Common::Point(160, 0));
		screen.update();

		switch (idx) {
		case 8:
		case 18:
		case 21:
			sound.playFX(33);
			break;

		case 19:
		case 25:
			sound.playFX(34);
			break;

		default:
			break;
		}

		WAIT(3);
	}

	prec.clear();

	SpriteResource cast1[7], cast2[7], darkLord[3];
	for (int idx = 1; idx < 7; ++idx)
		cast1[idx - 1].load(Common::String::format("cast%02d.end", idx));
	for (int idx = 1; idx < 7; ++idx)
		cast2[idx - 1].load(Common::String::format("casb%02d.end", idx));
	for (int idx = 1; idx < 4; ++idx)
		darkLord[idx - 1].load(Common::String::format("darklrd%d.end", idx));

	// Castle close-up
	int cloudsCtr = 1;
	for (int idx = 1; idx < 16; ++idx) {
		LOAD_VORTEX;
		cast1[0].draw(0, 0);
		cast2[0].draw(0, 0, Common::Point(0, 100));
		WAIT(3);
	}

	screen.loadPalette("mm4.pal");
	screen.fadeIn(0x81);

	// Castle gets destroyed / sucked into the vortex
	const byte COUNTS1[6] = { 9, 3, 2, 2, 3, 15 };
	bool flag = false;
	for (int idx1 = 1; idx1 < 7; ++idx1) {
		for (int idx2 = 0; idx2 < COUNTS1[idx1 - 1]; ++idx2) {
			if (flag && !sound.isSoundPlaying()) {
				flag = false;
				sound.playFX(34);
			} else if (!flag && idx1 == 1 && idx2 == 6) {
				flag = true;
				sound.playVoice("xeenlaff.voc");
			}

			switch (cloudsCtr) {
			case 0:
			case 1:
			case 5:
			case 9:
			case 15:
				sound.playFX(34);
				break;
			case 2:
			case 7:
			case 10:
			case 13:
				sound.playFX(33);
				break;
			default:
				break;
			}

			LOAD_VORTEX;
			cast1[idx1 - 1].draw(0, idx2, Common::Point(0, 0));
			cast2[idx1 - 1].draw(0, idx2, Common::Point(0, 100));
			WAIT(3);
		}
	}

	// Fade in of Alamar
	for (int idx = 0; idx < 16; ++idx) {
		LOAD_VORTEX;

		if (idx < 7)
			darkLord[0].draw(0, idx);
		else if (idx < 11)
			darkLord[1].draw(0, idx - 7);
		else
			darkLord[2].draw(0, idx - 11);

		switch (cloudsCtr - 1) {
		case 0:
		case 4:
		case 8:
		case 14:
			sound.playFX(34);
			break;
		case 1:
		case 6:
		case 9:
		case 12:
			sound.playFX(33);
			break;
		default:
			break;
		}

		WAIT(3);
	}
	sound.setMusicPercent(60);
	_subtitles.setLine(11);

	// Alamar's monologue
	for (int idx = 0; idx < (sound._subtitles ? 4 : 3); ++idx) {
		switch (idx) {
		case 0:
			// You have defeated my general, Lord Xeen
			sound.playVoice("dark1.voc");
			break;
		case 1:
			// And foiled my plans to conquer this world
			sound.playVoice("dark2.voc");
			break;
		case 2:
			// But the Dark Side will always be mine
			sound.playVoice("dark3.voc");
			break;
		default:
			// Laugh
			sound.playVoice("darklaff.voc");
			sound.setMusicPercent(75);
			break;
		}

		do {
			LOAD_VORTEX;
			darkLord[2].draw(0, getSpeakingFrame(2, 6));

			switch (cloudsCtr - 1) {
			case 0:
			case 4:
			case 8:
			case 14:
				sound.playFX(34);
				break;
			case 1:
			case 6:
			case 9:
			case 12:
				sound.playFX(33);
				break;
			default:
				break;
			}

			_subtitles.show();
			WAIT(3);
		} while (sound.isSoundPlaying() || (idx == 3 && _subtitles.active()));
	}

	if (!sound._subtitles) {
		// Laugh
		sound.playVoice("darklaff.voc");
		sound.setMusicPercent(75);
	}

	// Alamar fade out
	for (int idx = 12; idx >= 0; --idx) {
		LOAD_VORTEX;

		if (idx < 7)
			darkLord[0].draw(0, idx);
		else if (idx < 11)
			darkLord[1].draw(0, idx - 7);
		else
			darkLord[2].draw(0, idx - 11);

		switch (cloudsCtr - 1) {
		case 0:
		case 4:
		case 8:
		case 14:
			sound.playFX(34);
			break;
		case 1:
		case 6:
		case 9:
		case 12:
			sound.playFX(33);
			break;
		default:
			break;
		}

		WAIT(3);
	}

	sound.stopSound();
	sound.playSong("endgame.m");
	screen.fadeOut();
	return true;
}

bool CloudsCutscenes::showCloudsEnding2() {
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;

	SpriteResource king("king.end"), people("people.end"), crodo("crodo.end"),
		kingCord("kingcord.end");

	// Later at Castle Burlock
	screen.loadPalette("endgame.pal");
	screen.loadBackground("later.raw");
	screen.fadeIn();
	WAIT(100);
	screen.fadeOut();

	// Horizontal pan to the right within throne room
	screen.loadBackground("throne1.raw");
	screen.loadPage(0);
	screen.loadBackground("throne2.raw");
	screen.loadPage(1);

	int xp2 = SCREEN_WIDTH;
	bool fadeFlag = true;
	for (int ctr = SCREEN_WIDTH, xp1 = 117, xp3 = 0; ctr > 0; --ctr, xp1 -=   2, ++xp3) {
		screen.horizMerge(xp3);
		people.draw(0, 0, Common::Point(xp1, 68), SPRFLAG_800);
		if (xp3 > 250) {
			crodo.draw(0, 0, Common::Point(xp2, 68), SPRFLAG_800);
			xp2 -= 2;
			if (xp2 < 181)
				xp2 = 181;
		}

		if (ctr % 2) {
			WAIT(1);
		}
		if (fadeFlag) {
			screen.fadeIn();
			fadeFlag = false;
		}
	}

	screen.horizMerge(SCREEN_WIDTH);
	crodo.draw(0, 0, Common::Point(xp2, 68), SPRFLAG_800);
	screen.freePages();
	WAIT(5);

	Graphics::ManagedSurface savedBg;
	savedBg.blitFrom(screen);

	// Close up of King Roland
	const int XLIST1[13] = { 0, -5, -10, -15, -20, -25, -30, -33, -27, -22, -17 };
	const int XLIST2[13] = { 160, 145, 130, 115, 100, 85, 70, 57, 53, 48, 42, 39, 34 };
	const int YLIST[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 4 };

	for (int idx = 12; idx >= 0; --idx) {
		screen.blitFrom(savedBg);
		king.draw(0, 0, Common::Point(XLIST1[idx], YLIST[idx]), 0, idx);
		king.draw(0, 1, Common::Point(XLIST2[idx], YLIST[idx]), 0, idx);
		WAIT(1);
	}

	// Congratulations adventurers
	const char *const VOC_NAMES[3] = { "king1.voc", "king2.voc", "king3.voc" };
	_subtitles.setLine(12);
	for (int idx = 0; idx < 3; ++idx) {
		sound.playVoice(VOC_NAMES[idx]);

		do {
			king.draw(0, 0, Common::Point(0, 0));
			king.draw(0, 1, Common::Point(160, 0));

			int frame = getSpeakingFrame(1, 6);
			if (frame > 1)
				king.draw(0, frame);

			_subtitles.show();
			WAIT(3);
		} while (sound.isSoundPlaying() || (idx == 2 && _subtitles.active()));

		king.draw(0, 0, Common::Point(0, 0));
		king.draw(0, 1, Common::Point(160, 0));
		WAIT(1);
	}

	screen.fadeOut();
	return true;
}

const byte MONSTER_INDEXES[73] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 76,
	23, 16, 17, 80, 19, 20, 83, 22, 24, 25, 26, 27, 28, 29, 30,
	31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 42, 43, 44, 45, 84,
	47, 48, 49, 50, 51, 52, 53, 55, 56, 57, 58, 59, 60, 61, 62,
	63, 64, 65, 66, 67, 68, 70, 71, 72, 73, 75, 88, 89
};
const int8 XARRAY[8] = { -2, -1, 0, 1, 2, 1, 0, -1 };
const int8 YARRAY[8] = { -2, 0, 2, 0, -1, 0, 2, 0 };

bool CloudsCutscenes::showCloudsEnding3() {
	Map &map = *_vm->_map;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	SpriteResource monSprites, attackSprites;
	SpriteResource kingCord("kingcord.end"), room("room.end"), bigSky("bigsky.end");
	Graphics::ManagedSurface savedBg;
	int counter1 = 0;

	// Show the mirror room
	screen.loadPalette("mirror.pal");
	screen.loadBackground("miror-s.raw");
	screen.loadPage(0);
	screen.loadPage(1);

	room.draw(0, 0, Common::Point(0, 0));
	room.draw(0, 1, Common::Point(160, 0));
	screen.fadeIn();

	for (int idx = 0; idx < 83; ++idx) {
		screen.horizMerge(idx);
		room.draw(0, 0, Common::Point(0, 0));
		room.draw(0, 1, Common::Point(160, 0));
		WAIT(1);
	}

	// Zooming into the mirror
	screen.freePages();
	savedBg.blitFrom(screen);

	const int XLIST3[9] = { 0, -5, -10, -15, -24, -30, -39, -50, -59 };
	const int YLIST3[9] = { 0, 12, 25, 37, 46, 52, 59, 64, 68 };
	for (int idx = 8; idx >= 0; --idx) {
		screen.blitFrom(savedBg);
		bigSky.draw(0, 0, Common::Point(XLIST3[idx], YLIST3[idx]), 0, idx);
		_mirrBack.draw(0, 0, Common::Point(XLIST3[idx], YLIST3[idx]), 0, idx);
		WAIT(1);
	}

	// Roland and Crodo moving in to look at mirror
	const int DELTA = 2;
	for (int idx = 0, xc1 = -115, yp = SCREEN_HEIGHT, xc2 = 335;
	idx < 115; idx += DELTA, xc1 += DELTA, yp -= DELTA, xc2 -= DELTA) {
		ROTATE_BG;

		_mirrBack.draw(0, 0);
		_mirror.draw(0, 0);
		kingCord.draw(0, 0, Common::Point(xc1, yp), SPRFLAG_800);
		kingCord.draw(0, 1, Common::Point(xc2, yp), SPRFLAG_800);
		WAIT(1);
	}

	ROTATE_BG;
	_mirrBack.draw(0, 0);
	_mirror.draw(0, 0);
	kingCord.draw(0, 0, Common::Point(0, 85), SPRFLAG_800);
	kingCord.draw(0, 1, Common::Point(220, 85), SPRFLAG_800);

	// Loop through showing each monster
	for (int monsterCtr = 0; monsterCtr < 73; ++monsterCtr) {
		MonsterStruct &mon = map._monsterData[MONSTER_INDEXES[monsterCtr]];
		monSprites.load(Common::String::format("%03d.mon", mon._imageNumber));
		attackSprites.load(Common::String::format("%03d.att", mon._imageNumber));

		for (int frameCtr = 0; frameCtr < 8; ++frameCtr) {
			ROTATE_BG;
			counter1 = (counter1 + 1) % 8;
			Common::Point monPos(31, 10);
			if (mon._flying) {
				monPos.x += XARRAY[counter1];
				monPos.y += YARRAY[counter1];
			}

			_mirrBack.draw(0, 0);
			monSprites.draw(0, frameCtr, monPos);
			_mirror.draw(0, 0);
			kingCord.draw(0, 0, Common::Point(0, 85), SPRFLAG_800);
			kingCord.draw(0, 1, Common::Point(220, 85), SPRFLAG_800);
			WAIT(1);
		}

		for (int frameCtr = 0; frameCtr < 3; ++frameCtr) {
			if (frameCtr == 2)
				sound.playVoice(Common::String::format("%s.voc", mon._attackVoc.c_str()));

			ROTATE_BG;
			counter1 = (counter1 + 1) % 8;
			Common::Point monPos(31, 10);
			if (mon._flying) {
				monPos.x += XARRAY[counter1];
				monPos.y += YARRAY[counter1];
			}

			_mirrBack.draw(0, 0);
			attackSprites.draw(0, frameCtr, monPos);
			_mirror.draw(0, 0);
			kingCord.draw(0, 0, Common::Point(0, 85), SPRFLAG_800);
			kingCord.draw(0, 1, Common::Point(220, 85), SPRFLAG_800);
			WAIT(1);
		}

		for (int idx = 0; idx < 15; ++idx) {
			ROTATE_BG;
			counter1 = (counter1 + 1) % 8;
			Common::Point monPos(31, 10);
			if (mon._flying) {
				monPos.x += XARRAY[counter1];
				monPos.y += YARRAY[counter1];
			}

			_mirrBack.draw(0, 0);
			attackSprites.draw(0, 2, monPos);
			_mirror.draw(0, 0);
			kingCord.draw(0, 0, Common::Point(0, 85), SPRFLAG_800);
			kingCord.draw(0, 1, Common::Point(220, 85), SPRFLAG_800);
			WAIT(1);
		}

		int powNum = getSpeakingFrame(0, 5);
		sound.stopSound();
		sound.playSound(Common::String::format("pow%d.voc", powNum));

		for (int idx = 0; idx < 7; ++idx) {
			ROTATE_BG;
			counter1 = (counter1 + 1) % 8;
			Common::Point monPos(31, 10);
			if (mon._flying) {
				monPos.x += XARRAY[counter1];
				monPos.y += YARRAY[counter1];
			}

			_mirrBack.draw(0, 0);
			attackSprites.draw(0, 2, monPos);
			_mirror.draw(0, 0);
			kingCord.draw(0, 0, Common::Point(0, 85), SPRFLAG_800);
			kingCord.draw(0, 1, Common::Point(220, 85), SPRFLAG_800);
			WAIT(1);
		}
	}

	doScroll(true, false);
	return true;
}

bool CloudsCutscenes::showCloudsEnding4(uint finalScore) {
	EventsManager &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	Windows &windows = *_vm->_windows;
	SpriteResource endText("endtext.end");

	ROTATE_BG;
	_mirrBack.draw(0, 0);
	_mirror.draw(0, 0);
	doScroll(false, false);

	// Congratulations your final score
	for (int idx = 0; idx < 19; ++idx) {
		ROTATE_BG;
		_mirrBack.draw(0, 0);
		_mirror.draw(0, 0);
		endText.draw(0, idx);
		WAIT(1);
	}

	// Random animation of score numbers
	int frames[10];
	const int FRAMEX[10] = { 64, 83, 102, 121, 140, 159, 178, 197, 216, 235 };
	for (int idx1 = 0; idx1 < 30; ++idx1) {
		for (int idx2 = 0; idx2 < 10; ++idx2)
			frames[idx2] = getSpeakingFrame(20, 29);

		ROTATE_BG;
		_mirrBack.draw(0, 0);
		_mirror.draw(0, 0);
		endText.draw(0, 19);
		for (int idx2 = 0; idx2 < 10; ++idx2)
			endText.draw(0, frames[idx2], Common::Point(FRAMEX[idx2], 73));

		WAIT(2);
	}

	// Animate changing the score digits to the actual final score
	Common::String scoreStr = Common::String::format("%.10u", finalScore);
	for (int idx1 = 0; idx1 < 10; ++idx1) {
		for (int idx2 = 0; idx2 < 10; ++idx2)
			frames[idx2] = getSpeakingFrame(20, 29);

		for (int idx2 = 0; idx2 <= idx1; ++idx2)
			frames[9 - idx2] = (byte)scoreStr[9 - idx2] - 28;

		ROTATE_BG;
		_mirrBack.draw(0, 0);
		_mirror.draw(0, 0);
		endText.draw(0, 19);

		for (int idx2 = 0; idx2 < 10; ++idx2)
			endText.draw(0, frames[idx2], Common::Point(FRAMEX[idx2], 73));

		WAIT(2);
	}

	// Move the score vertically down
	for (int idx1 = 0; idx1 < 38; ++idx1) {
		ROTATE_BG;
		_mirrBack.draw(0, 0);
		_mirror.draw(0, 0);
		endText.draw(0, 19);

		for (int idx2 = 0; idx2 < 10; ++idx2)
			endText.draw(0, frames[idx2], Common::Point(FRAMEX[idx2], 73 + idx1));

		WAIT(1);
	}

	// Show two screens worth of text, with prompt to press a key
	windows[28].setBounds(Common::Rect(63, 60, 254, 160));

	for (int idx = 1; idx <= 2; ++idx) {
		events.clearEvents();
		do {
			ROTATE_BG;
			_mirrBack.draw(0, 0);
			_mirror.draw(0, 0);
			endText.draw(0, 19);

			for (int idx2 = 0; idx2 < 10; ++idx2)
				endText.draw(0, frames[idx2], Common::Point(FRAMEX[idx2], 110));
			windows[28].writeString(idx == 1 ? Res.CLOUDS_CONGRATULATIONS1 :
				Res.CLOUDS_CONGRATULATIONS2);

			events.updateGameCounter();
			events.wait(1, false);
		} while (!events.isKeyMousePressed());
	}

	doScroll(true, false);
	screen.fadeOut();

	return true;
}

bool CloudsCutscenes::showCloudsEnding5() {
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	SpriteResource king("king.end");

	king.draw(0, 0, Common::Point(0, 0));
	king.draw(0, 1, Common::Point(160, 0));
	screen.fadeIn();
	_subtitles.setLine(13);

	sound.playVoice("king4.voc");
	do {
		king.draw(0, 0, Common::Point(0, 0));
		king.draw(0, 1, Common::Point(160, 0));
		int frame = getSpeakingFrame(1, 6);
		if (frame > 1)
			king.draw(0, frame);

		WAIT(3);
	} while (sound.isSoundPlaying() || _subtitles.active());

	king.draw(0, 0, Common::Point(0, 0));
	king.draw(0, 1, Common::Point(160, 0));
	WAIT(1);
	return true;
}

void CloudsCutscenes::loadScreen(const Common::String &name) {
	Screen &screen = *_vm->_screen;
	File fSrc(name);
	byte *destP = (byte *)screen.getPixels();
	byte *destEndP = (byte *)destP + SCREEN_WIDTH * SCREEN_HEIGHT;

	// Setup reference arrays
	#define ARRAY_SIZE 314
	#define ARRAY_LAST1 ((ARRAY_SIZE - 1) * 2)
	#define ARRAY_LAST2 ((ARRAY_SIZE - 1) * 2 + 1)
	#define BUFFER_SIZE 0x1000
	uint array2[ARRAY_SIZE * 2], array3[ARRAY_SIZE * 2];
	uint array4[ARRAY_SIZE * 3];
	byte buffer[BUFFER_SIZE];

	for (int idx = 0; idx < ARRAY_SIZE; ++idx) {
		array3[idx] = 1;
		array4[idx + 627] = idx * 2;
		array2[idx] = idx * 2 + (ARRAY_SIZE * 4 - 2);
	}

	for (int ctr = 0, idx = 0, idx2 = ARRAY_SIZE; ctr < (ARRAY_SIZE - 1); ++ctr, idx += 2, ++idx2) {
		array3[idx2] = array3[idx] + array3[idx + 1];
		array2[idx2] = idx * 2;
		array4[idx] = array4[idx + 1] = idx2 * 2;
	}
	array4[ARRAY_LAST1] = 0;
	array3[ARRAY_LAST2] = (uint)-1;
	array2[ARRAY_LAST2] = 4036;
	uint16 bits = 0x8000;

	// Get the decompressed size and default buffer contents
	uint16 bytePair;
	fSrc.read((byte *)&bytePair, 2);
	Common::fill((uint16 *)buffer, (uint16 *)(buffer + BUFFER_SIZE),
		bytePair);

	int count = fSrc.readUint16BE();
	assert(count == (SCREEN_WIDTH * SCREEN_HEIGHT));

	for (int byteIdx = 0; byteIdx < count; ) {
		assert(fSrc.pos() < fSrc.size());

		int vMin = array2[(ARRAY_SIZE - 1) * 2];
		int vThreshold = ARRAY_SIZE * 4 - 2;
		while (vMin < vThreshold) {
			bool flag = (bits & 0x8000);
			bits <<= 1;

			if (!bits) {
				bits = fSrc.readUint16BE();
				flag = (bits & 0x8000);
				bits = (bits << 1) | 1;
			}

			vMin = array2[vMin / 2 + (flag ? 1 : 0)];
		}

		vMin -= vThreshold;

		if (array3[ARRAY_LAST1] == 0x8000) {
			for (int ctr = 0, ctr2 = 0; ctr < (ARRAY_SIZE * 2); ++ctr) {
				if (array2[ctr] >= (ARRAY_SIZE * 4 - 1)) {
					array3[ctr2] = (array3[ctr] + 1) / 2;
					array2[ctr2] = array2[ctr];
					++ctr2;
				}
			}

			for (int ctr = 0, ctr2 = ARRAY_SIZE; ctr < ARRAY_SIZE; ctr += 2, ++ctr2) {
				int currVal = array3[ctr] + array3[ctr + 1];
				array3[ctr2] = currVal;
				int ctr3 = ctr2;
				do {
					--ctr3;
				} while (array3[ctr3] >= array3[ctr2]);
				++ctr3;

				int diff = ctr2 - ctr3;
				uint *pDest = &array3[ctr2];
				Common::copy(pDest - 1, pDest - 1 + diff, pDest);
				array3[ctr3] = currVal;

				pDest = &array2[ctr2];
				Common::copy(pDest - 1, pDest - 1 + diff, pDest);
				array2[ctr3] = ctr * 2;
			}

			uint *arrEndP = &array4[ARRAY_SIZE * 2 - 1];
			for (int ctr = 0, val = 0; ctr < ARRAY_SIZE * 2; ++ctr, val += 2) {
				uint *arrP = &array4[array2[ctr] / 2];
				if (arrP < arrEndP)
					*arrP = val;
			}
		}

		int offset = array4[627 + vMin / 2] / 2;
		do {
			int offset2 = offset;
			uint val = ++array3[offset2];
			if (val > array3[offset2 + 1]) {
				while (val > array3[++offset2])
					;
				--offset2;

				array3[offset] = array3[offset2];
				array3[offset2] = val;

				int offset3 = array2[offset] / 2;
				array4[offset3] = offset2 * 2;
				if ((offset3 * 2) < (ARRAY_SIZE * 4 - 2))
					array4[offset3 + 1] = offset2 * 2;

				int offset4 = array2[offset2] / 2;
				array2[offset2] = offset3 * 2;
				array4[offset4] = offset * 2;
				if ((offset4 * 2) < (ARRAY_SIZE * 4 - 2))
					array4[offset4 + 1] = offset * 2;

				array2[offset] = offset4 * 2;
				offset = offset2;
			}
		} while ((offset = array4[offset] / 2) != 0);

		vMin /= 2;
		if (vMin < 256) {
			// Single byte write to destination
			*destP++ = (byte)vMin;
			int buffOffset = array2[ARRAY_LAST2];
			array2[ARRAY_LAST2] = (buffOffset + 1) & 0xfff;
			buffer[buffOffset] = (byte)vMin;
			++byteIdx;
			continue;
		}

		uint16 bitsLow = bits, bitsHigh = 0;
		for (int ctr = 8; ctr > 0; --ctr) {
			bool highBit = bitsLow & 0x8000;
			bitsLow <<= 1;

			if (bitsLow) {
				bitsHigh = (bitsHigh << 1) | (highBit ? 1 : 0);
			} else {
				bitsLow = fSrc.readUint16BE();

				byte loBit = 1;
				do {
					bitsHigh = (bitsHigh << 1) | ((bitsLow & 0x8000) ? 1 : 0);
					bitsLow = (bitsLow << 1) | (loBit ? 1 : 0);
					loBit = 0;
				} while (--ctr > 0);
				break;
			}
		}
		bits = bitsLow;

		int t2Val = _DECODE_TABLE2[bitsHigh] << 6;
		int tCount = _DECODE_TABLE1[bitsHigh] - 2;

		for (int ctr = 0; ctr < tCount; ++ctr) {
			bool highBit = bits & 0x8000;
			bits <<= 1;
			if (!bits) {
				bits = fSrc.readUint16BE();
				highBit = bits & 0x8000;
				bits = (bits << 1) | 1;
			}

			bitsHigh = (bitsHigh << 1) | (highBit ? 1 : 0);
		}

		t2Val |= (bitsHigh & 0x3F);
		uint &last2 = array2[ARRAY_LAST2];
		int buffOffset = last2 - t2Val - 1;

		for (int ctr = 0; ctr < vMin - 253; ++ctr, ++buffOffset) {
			buffOffset &= 0xfff;
			byte b = buffer[buffOffset];
			*destP++ = b;

			buffer[last2] = b;
			last2 = (last2 + 1) & 0xfff;
			++byteIdx;
		}
	}

	assert(destP == destEndP);
	screen.markAllDirty();
}

const char *const CloudsCutscenes::_INTRO_VOCS[14] = {
	"crodo1.voc", "crodo2.voc", "iamking.voc", "crodo3.voc",
	"ya1.voc", "crodo4a.voc", "crodo4b.voc", "crodo4c.voc",
	"xeenlaff.voc", "tiger2&.voc", "crodo5.voc", "crodo6.voc",
	"xeenlaff.voc", "tiger2&.voc"
};

const int CloudsCutscenes::_INTRO_FRAMES_LOOKUP[14] = {
	0, 1, 0, 2, 0, 3, 4, 5, 0, 0, 6, 7, 0, 0
};

const int CloudsCutscenes::_INTRO_FRAMES_VALS[8][32] = {
	{
		4, 2, 3, 0, 2, 3, 2, 0, 1, 1, 3, 4, 3, 2, 4, 2,
		3, 4, 3, 4, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	}, {
		3, 2, 3, 2, 4, 3, 0, 3, 2, 2, 3, 1, 2, 3, 3, 3,
		2, 3, 2, 3, 2, 0, 3, 2, 0, 0, 0, 0, 0, 0, 2, 4
	}, {
		3, 1, 2, 3, 0, 3, 4, 3, 2, 3, 0, 3, 2, 3, 2, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 4, 3
	}, {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2, 3
	}, {
		4, 2, 2, 3, 2, 3, 3, 4, 2, 4, 2, 0, 3, 2, 3, 2,
		3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 0, 2, 3
	}, {
		2, 0, 2, 3, 2, 4, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 3, 2, 3, 1
	}, {
		3, 2, 0, 2, 4, 2, 3, 2, 3, 2, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 4, 3, 4, 0, 2
	}, {
		3, 2, 4, 1, 2, 4, 3, 2, 3, 0, 2, 2, 0, 3, 2, 3,
		2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	}
};

const uint CloudsCutscenes::_INTRO_FRAMES_WAIT[8][32] = {
	{
		 2,  5,  6,  9, 10, 11, 12, 13, 14, 23, 25, 29, 31, 35, 38, 41,
		42, 45, 50, 52, 55, 56, 57,  0,  0,  0,  0,  0,  0,  0,  0,  0
	}, {
		 1,  4,  6,  8,  9, 11, 13, 15, 17, 18, 19, 22, 28, 29, 30, 31,
		 0, 39,  0, 44,  0, 50, 51,  0, 54,  0,  0,  0,  0,  0,  0,  4
	}, {
		 6,  9, 11, 13, 15, 19, 21, 23, 25, 27, 28, 31, 35, 39, 40,  0,
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  5,  7
	}, {
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  3,  4
	}, {
		 5,  9, 10, 11, 13, 15, 18, 23, 26, 31, 33, 36, 37, 41, 43, 45,
		48,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  9,  0, 12
	}, {
		14, 17, 20, 23, 27, 29,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  4,  8, 11, 13
	}, {
		15, 16, 17, 19, 21, 24, 24, 27, 34, 35,  0,  0,  0,  0,  0,  0,
		 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  5,  7, 10, 11, 13
	}, {
		17, 19, 22, 23, 26, 30, 32, 34, 40, 43, 47, 52, 53, 55, 57, 60,
		62,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
	}
};

const byte CloudsCutscenes::_DECODE_TABLE1[256] = {
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};

const byte CloudsCutscenes::_DECODE_TABLE2[256] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7,
	8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9,
	10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11,
	12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 14, 15, 15, 15, 15,
	16, 16, 16, 16, 17, 17, 17, 17, 18, 18, 18, 18, 19, 19, 19, 19,
	20, 20, 20, 20, 21, 21, 21, 21, 22, 22, 22, 22, 23, 23, 23, 23,
	24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 31,
	32, 32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 38, 38, 39, 39,
	40, 40, 41, 41, 42, 42, 43, 43, 44, 44, 45, 45, 46, 46, 47, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63
};

} // End of namespace WorldOfXeen
} // End of namespace Xeen
