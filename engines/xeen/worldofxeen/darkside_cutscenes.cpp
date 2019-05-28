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

#include "xeen/sound.h"
#include "xeen/xeen.h"
#include "xeen/worldofxeen/darkside_cutscenes.h"
#include "xeen/worldofxeen/worldofxeen.h"

#define WAIT(TIME) if (_subtitles.wait(TIME)) return false

namespace Xeen {
	namespace WorldOfXeen {

#define	WAIT_SUBTITLES(time) \
	_subtitles.show(); \
	WAIT(time)

static const int PHAROAH_FRAMES[32] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 3, 3, 2, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
const int PHAROAH_YLIST[32] = {
	-3, -3, -3, -3, -3, -3, -3, -3, -1, 0, 0, 0, 0, 0, 0, 0,
	-1, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3
};
const int LEFT_CLAW_RUB_X[32] = {
	223, 208, 187, 158, 129, 104, 85, 70, 61, 53, 49,
	46, 46, 49, 53, 56, 61, 67, 72, 82, 91, 103,
	117, 130, 142, 156, 170, 184, 199, 210, 221, 227
};
const int LEFT_CLAW_RUB_Y[32] = {
	116, 111, 105, 103, 101, 96, 93, 92, 92, 95, 98,
	100, 104, 107, 107, 105, 102, 100, 98, 99, 101, 104,
	107, 109, 110, 111, 114, 115, 115, 116, 116, 118
};
static const int RIGHT_CLAW_IDLE_X[32] = {
	146, 145, 143, 141, 141, 141, 141, 141, 141, 141, 141, 142, 143, 144, 145, 146,
	146, 146, 146, 146, 146, 146, 146, 146, 146, 146, 146, 146, 146, 146, 146, 146
};
static const int RIGHT_CLAW_IDLE_Y[32] = {
	143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143,
	143, 143, 144, 145, 145, 145, 145, 145, 145, 145, 145, 144, 143, 143, 143, 143
};
const int LEFT_CLAW_IDLE_X[32] = {
	223, 222, 220, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219,
	219, 219, 220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 221
};
const int LEFT_CLAW_IDLE_Y[32] = {
	116, 116, 116, 117, 117, 117, 117, 117, 118, 118, 118, 118, 118, 118, 118, 118,
	118, 118, 118, 118, 118, 118, 118, 118, 118, 118, 118, 118, 118, 118, 118, 118
};


bool DarkSideCutscenes::showDarkSideTitle(bool seenIntro) {
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;
	g_vm->_files->_ccNum = true;
	_subtitles.reset();

	screen.loadPalette("dark.pal");
	SpriteResource nwc[4] = {
		SpriteResource("nwc1.int"), SpriteResource("nwc2.int"),
		SpriteResource("nwc3.int"), SpriteResource("nwc4.int")
	};
	File voc[3];
	voc[0].open("dragon1.voc");
	voc[1].open("dragon2.voc");
	voc[2].open("dragon3.voc");

	// Load backgrounds
	screen.loadBackground("nwc1.raw");
	screen.loadPage(0);
	screen.loadBackground("nwc2.raw");
	screen.loadPage(1);

	// Draw the screen and fade it in
	screen.horizMerge(0);
	screen.fadeIn();

	sound.setMusicPercent(75);
	sound.playFX(1);

	// Initial loop for dragon roaring
	int nwcIndex = 0, nwcFrame = 0;
	for (int idx = 0; idx < 55 && !g_vm->shouldExit(); ++idx) {
		// Render the next frame
		screen.vertMerge(0);
		nwc[nwcIndex].draw(0, nwcFrame);

		switch (idx) {
		case 17:
			sound.playSound(voc[0]);
			break;
		case 34:
		case 44:
			++nwcIndex;
			nwcFrame = 0;
			break;
		case 35:
			sound.playSound(voc[1]);
			break;
		default:
			++nwcFrame;
		}

		WAIT(2);
	}

	// Loop for dragon using flyspray
	for (int idx = 0; idx < 42 && !g_vm->shouldExit(); ++idx) {
		screen.vertMerge(SCREEN_HEIGHT);
		nwc[3].draw(0, idx);

		switch (idx) {
		case 3:
			sound.playFX(40);
			break;
		case 11:
			sound.playFX(0);
			break;
		case 27:
		case 30:
			sound.playFX(3);
			break;
		case 31:
			sound.playSound(voc[2]);
			break;
		case 33:
			sound.playFX(2);
			break;
		default:
			break;
		}

		WAIT(2);
	}
	if (g_vm->shouldExit())
		return false;

	// Pause for a bit
	WAIT(10);
	screen.fadeOut(8);

	return true;
}

bool DarkSideCutscenes::showDarkSideIntro(bool seenIntro) {
	FileManager &files = *g_vm->_files;
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;

	files._ccNum = true;
	files.setGameCc(1);
	_subtitles.reset();

	if (showDarkSideTitle(seenIntro)) {
		if (seenIntro) {
			if (g_vm->getGameID() == GType_WorldOfXeen)
				seenIntro = showWorldOfXeenLogo();
		} else {
			_ball.load("ball.int");
			_dragon1.load("dragon1.int");
			_claw.load("claw.int");

			seenIntro = showDarkSideIntro1() && showDarkSideIntro2() && showDarkSideIntro3();

			_ball.clear();
			_dragon1.clear();
			_claw.clear();

			if (seenIntro && g_vm->getGameID() == GType_WorldOfXeen)
				seenIntro = showWorldOfXeenLogo();
		}
	}

	sound.stopAllAudio();
	screen.freePages();

	return seenIntro;
}

bool DarkSideCutscenes::rubCrystalBall(bool fadeIn) {
	Screen &screen = *g_vm->_screen;

	for (int frame = 0; frame < 32; ++frame) {
		screen.restoreBackground();

		if (frame)
			_ball.draw(0, frame - 1);
		_claw.draw(0, 5, Common::Point(RIGHT_CLAW_IDLE_X[frame], RIGHT_CLAW_IDLE_Y[frame]), SPRFLAG_800);
		_claw.draw(0, 6, Common::Point(149, 184));

		_dragon1.draw(0, PHAROAH_FRAMES[frame], Common::Point(139, PHAROAH_YLIST[frame]), SPRFLAG_800);
		_claw.draw(0, frame % 5, Common::Point(LEFT_CLAW_RUB_X[frame], LEFT_CLAW_RUB_Y[frame]), SPRFLAG_800);

		if (_subtitles.active())
			_subtitles.show();

		if (fadeIn && frame == 0)
			screen.fadeIn();
		WAIT(3);
	}

	return true;
}

void DarkSideCutscenes::animatePharoah(int frame, bool showBall) {
	if (showBall && frame)
		_ball.draw(0, frame - 1);
	_claw.draw(0, 5, Common::Point(RIGHT_CLAW_IDLE_X[frame], RIGHT_CLAW_IDLE_Y[frame]), SPRFLAG_800);
	_claw.draw(0, 6, Common::Point(149, 184));
	_dragon1.draw(0, PHAROAH_FRAMES[frame], Common::Point(139, PHAROAH_YLIST[frame]), SPRFLAG_800);
	_claw.draw(0, frame % 5, Common::Point(LEFT_CLAW_IDLE_X[frame], LEFT_CLAW_IDLE_Y[frame]), SPRFLAG_800);
}

bool DarkSideCutscenes::showDarkSideIntro1() {
	EventsManager &events = *g_vm->_events;
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;
	const int XLIST1[] = {
		0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 58, 60, 62
	};
	const int YLIST1[] = {
		0, 5, 10, 15, 20, 25, 30, 35, 40, 40, 39, 37, 35, 33, 31
	};
	const int XLIST2[] = {
		160, 155, 150, 145, 140, 135, 130, 125, 120, 115, 110, 105, 98, 90, 82
	};

	// Play the intro music
	sound.setMusicPercent(75);
	sound.playSong("bigtheme.m");

	screen.loadBackground("jvc.raw");
	screen.loadPalette("dark.pal");
	screen.fadeIn();

	WAIT(60);
	screen.fadeOut(8);

	screen.loadBackground("pyramid2.raw");
	screen.loadPage(0);
	screen.loadPage(1);
	screen.loadBackground("pyramid3.raw");
	screen.saveBackground();

	SpriteResource title("title.int");
	SpriteResource pyraTop("pyratop.int");
	SpriteResource pyramid("pyramid.int");

	screen.vertMerge(SCREEN_HEIGHT);
	screen.loadPage(0);
	screen.restoreBackground();
	screen.loadPage(1);

	// Show Might and Magic Darkside of Xeen title, and gradualy scroll
	// the background vertically down to show the Pharoah's base
	bool fadeFlag = true;
	for (int yp = 0, frameNum = 0, idx1 = 0; yp < SCREEN_HEIGHT && !g_vm->shouldExit(); ++yp) {
		screen.vertMerge(yp);

		title.draw(0, 0);
		if (frameNum)
			title.draw(0, frameNum);

		idx1 = (idx1 + 1) % 4;
		if (!idx1)
			frameNum = (frameNum + 1) % 10;

		WAIT(1);

		if (fadeFlag) {
			screen.fadeIn();
			fadeFlag = false;
		}
	}

	screen.vertMerge(SCREEN_HEIGHT);
	screen.saveBackground();
	screen.freePages();

	WAIT(20);

	// Zoom into the Pharoah's base closeup view
	sound.setMusicPercent(38);
	for (int idx = 14; idx >= 0 && !g_vm->shouldExit(); --idx) {
		pyraTop.draw(0, 0, Common::Point(XLIST1[idx], YLIST1[idx]), 0, idx);
		pyraTop.draw(0, 1, Common::Point(XLIST2[idx], YLIST1[idx]), 0, idx);

		WAIT(2);
	}

	// Inconceivable, the royal pyramid besieged
	screen.saveBackground();
	sound.playVoice("pharoh1a.voc");

	_subtitles.setLine(0);

	bool phar2 = false;
	for (int idx = 0; idx < 19 && !g_vm->shouldExit(); ++idx) {
		screen.restoreBackground();
		pyramid.draw(0, idx, Common::Point(132, 62));
		_subtitles.show();

		if (!sound.isSoundPlaying() && !phar2)
			sound.playVoice("pharoh1b.voc");

		WAIT_SUBTITLES(4);
	}

	if (!_subtitles.waitForLineOrSound())
		return false;
	screen.fadeOut();

	const int XLIST3[10] = { 102, 103, 104, 104, 104, 103, 102, 101, 101, 101 };
	const int YLIST3[10] = { 30, 29, 28, 27, 26, 25, 24, 25, 26, 28 };
	const int FRAMES3[70] = {
		9, 9, 9, 9, 9, 8, 8, 8, 8, 9, 9, 9, 9, 9, 0,
		1, 2, 3, 4, 5, 6, 7, 8, 9, 9, 9, 9, 9, 8, 8,
		8, 8, 9, 9, 9, 9, 165, 149, 126, 106, 92, 80, 66, 55, 42,
		29, 10,  -6, -26, -40, -56, -72, -83, 154, 141, 125, 105, 94, 83, 74,
		69, 68, 70, 73, 77, 83, 89, 94, 99, 109
	};
	const char *const PHAR_VOC[5] = {
		"pharoh2a.voc", "pharoh2b.voc", "pharoh2c.voc", "pharoh2d.voc", "pharoh2e.voc"
	};

	SpriteResource dragon("dragon.int");
	_subtitles.setLine(1);

	screen.loadBackground("2room.raw");
	screen.loadPage(1);
	screen.loadBackground("3room.raw");
	screen.loadPage(0);
	screen.loadBackground("1room.raw");
	screen.saveBackground();
	screen.horizMerge(SCREEN_WIDTH);
	dragon.draw(0, 9, Common::Point(XLIST3[0], YLIST3[0]), SPRFLAG_800);

	_subtitles.show();
	screen.fadeIn();

	int posNum = 0, phar2Index = 0, ctr = 0;
	for (int idx = SCREEN_WIDTH, frameNum = 0; idx >= 0; --idx) {
		events.updateGameCounter();
		screen.horizMerge(idx);
		dragon.draw(0, FRAMES3[frameNum], Common::Point(XLIST3[posNum], YLIST3[posNum]), SPRFLAG_800);
		_subtitles.show();

		if (idx == SCREEN_WIDTH)
			sound.playVoice(PHAR_VOC[0]);
		if (!sound.isSoundPlaying() && phar2Index < 4)
			sound.playVoice(PHAR_VOC[1 + phar2Index++]);

		if (phar2Index == 4) {
			if (!sound.isSoundPlaying() && !_subtitles.active())
				break;
		}

		if (++ctr > 2) {
			if (posNum == 5)
				sound.playFX(7);
			else if (posNum == 0)
				sound.playFX(8);

			posNum = (posNum + 1) % 10;
			frameNum = (frameNum + 1) % 36;
		}

		WAIT(1);
	}

	sound.stopSound();
	screen.fadeOut();
	screen.freePages();
	return true;
}

#define ANIMATE_PHAROAH \
	screen.restoreBackground(); \
	animatePharoah(clawCtr); \
	clawCtr = (clawCtr + 1) % 32

bool DarkSideCutscenes::showDarkSideIntro2() {
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;
	SpriteResource goon("goon.int"), darkLord("darklord.int"), queen("queen.int"),
		wizard("wizard.int"), fizzle("fizzle.int"), wizMth("wizmth.int");
	int idx, clawCtr;

	screen.loadPalette("dark.pal");
	screen.loadBackground("ball.raw");
	screen.saveBackground();

	if (!rubCrystalBall(true))
		return false;

	// Queen Kalindra?
	_subtitles.setLine(9);
	sound.playFX(42);

	for (idx = 0, clawCtr = 0; idx < 11; ++idx) {
		ANIMATE_PHAROAH;
		if (idx > 3)
			queen.draw(0, 0, Common::Point(9, 57));
		fizzle.draw(0, idx, Common::Point(9, 57));

		if (idx == 6) {
			sound.playVoice("pharoh3.voc");
			sound.playFX(0);
		}

		if (idx > 6)
			_subtitles.show();
		WAIT(3);
	}

	const char *const VOC_NAMES[4] = { "queen1.voc", "queen2.voc", "queenhis.voc", "pharoh4.voc" };
	int nwcIndex = 0, vocIndex = 0;
	do {
		ANIMATE_PHAROAH;

		if (!sound.isSoundPlaying() && vocIndex < 3) {
			if (!_subtitles.active()) {
				if (!vocIndex)
					_subtitles.setLine(10);

				sound.playVoice(VOC_NAMES[vocIndex++]);
				if (vocIndex == 3)
					nwcIndex = 7;
			}
		}

		if (nwcIndex) {
			queen.draw(0, (nwcIndex >= 17) ? 0 : nwcIndex,
				Common::Point(9, 57));
			if (nwcIndex < 17)
				++nwcIndex;
		} else {
			queen.draw(0, (vocIndex == 0) ? 0 : getSpeakingFrame(0, 5),
				Common::Point(9, 57));
		}

		_subtitles.show();
		WAIT(3);
	} while (vocIndex < 3 || sound.isSoundPlaying() || _subtitles.active());

	sound.playVoice(VOC_NAMES[3]);
	_subtitles.setLine(11);

	idx = 34;
	do {
		ANIMATE_PHAROAH;
		queen.draw(0, 0, Common::Point(9, 57));

		if (++idx <= 37)
			idx = 34;

		_subtitles.show();
		WAIT(3);
	} while (sound.isSoundPlaying() || _subtitles.active());

	_subtitles.setLine(12);
	sound.playVoice("dark3.voc");

	const char *const VOC_NAMES2[2] = { "pharoh5a.voc", "pharoh5b.voc" };
	vocIndex = 0;

	do {
		ANIMATE_PHAROAH;
		darkLord.draw(0, vocIndex == 0 ? getSpeakingFrame(0, 3) : 0, Common::Point(9, 57));

		_subtitles.show();
		WAIT(3);
		if (!sound.isSoundPlaying() && vocIndex < 2) {
			if (vocIndex)
				_subtitles.setLine(13);
			sound.playVoice(VOC_NAMES2[vocIndex++]);
		}
	} while (vocIndex < 2 || sound.isSoundPlaying() || _subtitles.active());

	sound.playFX(42);
	vocIndex = 0;

	for (idx = 10; idx >= 0; --idx) {
		ANIMATE_PHAROAH;

		if (!sound.isSoundPlaying() && !vocIndex) {
			sound.playVoice("laff1.voc");
			vocIndex = 1;
		}

		if (idx > 3)
			darkLord.draw(0, getSpeakingFrame(0, 3), Common::Point(9, 57));
		fizzle.draw(0, idx, Common::Point(9, 57));

		_subtitles.show();
		WAIT(3);
	}

	sound.playFX(0);

	do {
		ANIMATE_PHAROAH;

		_subtitles.show();
		WAIT(3);
	} while (sound.isSoundPlaying() || _subtitles.active());

	if (!rubCrystalBall())
		return false;

	for (idx = 0, clawCtr = 0; idx < 11; ++idx) {
		ANIMATE_PHAROAH;

		if (idx > 3)
			goon.draw(0, 0, Common::Point(9, 57));
		fizzle.draw(0, idx, Common::Point(9, 57));

		if (idx == 6)
			sound.playVoice("pharoh6w.voc");
		if (idx > 6)
			_subtitles.show();
		WAIT(3);
	}

	const char *const VOC_NAMES3[2] = { "alamar1.voc", "pharoh7t.voc" };
	vocIndex = nwcIndex = 0;
	_subtitles.setLine(14);

	do {
		ANIMATE_PHAROAH;
		goon.draw(0, (vocIndex == 0) ? 0 : nwcIndex, Common::Point(9, 57));

		if (!sound.isSoundPlaying() && !vocIndex && !_subtitles.active()) {
			_subtitles.setLine(15);
			sound.playVoice(VOC_NAMES3[vocIndex++]);
			sound.playFX(0);
		}

		nwcIndex = (nwcIndex + 1) % 17;
		_subtitles.show();
		WAIT(3);
	} while (!vocIndex || sound.isSoundPlaying() || _subtitles.active());

	_subtitles.setLine(16);
	sound.playFX(42);

	for (idx = 10, vocIndex = 0; idx >= 0; --idx) {
		ANIMATE_PHAROAH;
		if (idx > 4)
			goon.draw(0, 0, Common::Point(9, 57));
		fizzle.draw(0, idx, Common::Point(9, 57));

		if (!sound.isSoundPlaying() && !vocIndex) {
			sound.playVoice("pharoh7t.voc");
			vocIndex = 1;
		}

		_subtitles.show();
		WAIT(3);
	}

	sound.playFX(0);

	if (!rubCrystalBall())
		return false;

	_subtitles.setLine(17);

	for (idx = 0, clawCtr = 0; idx < 11; ++idx) {
		ANIMATE_PHAROAH;

		if (idx > 3)
			wizard.draw(0, 0, Common::Point(9, 57));
		fizzle.draw(0, idx, Common::Point(9, 57));

		if (idx == 5)
			sound.playVoice("pharoh8.voc");
		if (idx > 6)
			_subtitles.show();

		WAIT(3);
	}

	const char *const VOC_NAMES4[3] = { "ellinger.voc", "pharoh9a.voc", "pharoh9b.voc" };
	vocIndex = 0;

	do {
		ANIMATE_PHAROAH;
		wizard.draw(0, (vocIndex == 1) ? getSpeakingFrame(0, 3) : 0, Common::Point(9, 57));

		if (!sound.isSoundPlaying() && vocIndex < 2 && !_subtitles.active()) {
			_subtitles.setLine((vocIndex == 0) ? 18 : 19);
			sound.playVoice(VOC_NAMES4[vocIndex++]);
			sound.playFX(0);
		}

		_subtitles.show();
		WAIT(3);
	} while (vocIndex < 2 || sound.isSoundPlaying() || _subtitles.active());

	sound.playFX(42);

	for (idx = 10; idx >= 0; --idx) {
		ANIMATE_PHAROAH;
		if (idx > 4)
			wizard.draw(0, 0, Common::Point(9, 57));
		fizzle.draw(0, idx, Common::Point(9, 57));

		_subtitles.show();
		WAIT(3);
	}

	_subtitles.setLine(20);
	vocIndex = 0;

	do {
		ANIMATE_PHAROAH;

		if (!sound.isSoundPlaying() && !vocIndex) {
			sound.playVoice("pharoh9b.voc");
			vocIndex = 1;
		}

		_subtitles.show();
		WAIT(3);
	} while (sound.isSoundPlaying() || !vocIndex || _subtitles.active());

	sound.playFX(0);
	screen.fadeOut();

	return true;
}

bool DarkSideCutscenes::showDarkSideIntro3() {
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;
	SpriteResource fly("fly.int");
	SpriteResource bird[11];
	SpriteResource drop[4];
	for (int idx = 0; idx < 11; ++idx)
		bird[idx].load(Common::String::format("bird%02d.int", idx + 1));
	for (int idx = 0; idx < 4; ++idx)
		drop[idx].load(Common::String::format("drop%d.int", idx + 1));

	screen.loadPalette("fly.pal");
	screen.loadBackground("fly.raw");
	screen.saveBackground();

	_subtitles.setLine(6);
	_subtitles.show();

	screen.fadeIn();
	sound.playVoice("pharoh11.voc", 2);

	for (int idx = 0; idx < 35; ++idx) {
		screen.restoreBackground();
		fly.draw(0, idx);

		if (idx == 16 || idx == 28)
			sound.playFX(43);
		WAIT(2);
	}

	sound.playFX(0);
	screen.fadeOut();
	sound.setMusicPercent(75);
	screen.loadBackground("bird.raw");
	screen.saveBackground();
	screen.fadeIn();

	for (int idx = 0; idx < 81; ++idx) {
		screen.restoreBackground();
		bird[idx / 8].draw(0, idx % 8);

		switch (idx) {
		case 1:
			sound.playSound("comet.voc");
			sound.playFX(43);
			break;
		case 16:
		case 39:
			sound.playFX(43);
			break;
		case 21:
			sound.playSound("comet.voc");
			break;
		case 56:
			sound.playSound("screech.voc");
			break;
		case 61:
			sound.playSound("disint.voc");
			break;
		case 73:
			sound.playSound("laff1.voc");
			break;
		default:
			break;
		}

		WAIT(2);
	}

	sound.playFX(0);
	screen.loadBackground("drop.raw");
	screen.saveBackground();

	for (int idx = 0; idx < 39; ++idx) {
		screen.restoreBackground();
		drop[idx / 12].draw(0, idx % 12);

		if (idx == 9)
			sound.playSound("branch.voc");
		else if (idx == 22)
			sound.playSound("vine.voc");

		WAIT(2);
	}

	sound.playSound("thud.voc");
	screen.fadeOut(24);

	SpriteResource end[3] = {
		SpriteResource("end1.int"), SpriteResource("end2.int"), SpriteResource("end3.int")
	};
	screen.loadBackground("end.raw");
	screen.saveBackground();
	screen.fadeIn();
	//sound.playSong(16);

	for (int idx = 0; idx < 19; ++idx) {
		screen.restoreBackground();
		end[idx / 8].draw(0, idx % 8);
		WAIT(4);
	}

	SpriteResource title2("title2.int");
	const byte TITLE_XLIST1[] = { 0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55 };
	const byte TITLE_XLIST2[] = { 160, 155, 150, 145, 140, 135, 130, 125, 120, 115, 110, 105 };
	const byte TITLE_YLIST[] = { 0, 5, 10, 15, 20, 25, 30, 36, 44, 51, 57, 62 };

	screen.saveBackground();
	WAIT(20);

	for (int idx = 11; idx >= 0; --idx) {
		screen.restoreBackground();
		title2.draw(0, 0, Common::Point(TITLE_XLIST1[idx], TITLE_YLIST[idx]), 0, idx);
		title2.draw(0, 1, Common::Point(TITLE_XLIST2[idx], TITLE_YLIST[idx]), 0, idx);

		WAIT(2);
	}

	screen.fadeOut();
	return true;
}

bool DarkSideCutscenes::showWorldOfXeenLogo() {
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;
	SpriteResource fizzle("fizzle.int");
	SpriteResource wfire[7];
	for (uint idx = 0; idx < 7; ++idx)
		wfire[idx].load(Common::String::format("wfire%u.int", idx + 1));

	screen.loadBackground("firemain.raw");
	screen.loadPalette("firemain.pal");
	screen.saveBackground();
	screen.fadeIn();
	WAIT(10);

	for (int idx = 0; idx < 28; ++idx) {
		if (idx == 17)
			sound.playSound("explosio.voc");
		if (!sound.isSoundPlaying() && idx < 17)
			sound.playSound("rumble.voc");

		screen.restoreBackground();
		wfire[idx / 5].draw(0, idx % 5, Common::Point(0, 45));
		WAIT(2);
	}

	screen.saveBackground();

	for (int loopCtr = 0; loopCtr < 2; ++loopCtr) {
		for (int idx = 0; idx < 21; ++idx) {
			screen.restoreBackground();
			wfire[6].draw(0, idx, Common::Point(0, 45));

			switch (idx) {
			case 0:
			case 11:
				sound.playSound("thud.voc");
				break;
			case 3:
				sound.playFX(60);
				break;
			default:
				break;
			}

			WAIT(2);
		}
	}

	WAIT(10);
	screen.fadeOut();
	return true;
}

void DarkSideCutscenes::showDarkSideEnding(uint endingScore) {
	FileManager &files = *g_vm->_files;
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;

	files.setGameCc(1);
	_subtitles.reset();
	sound._musicSide = 1;
	screen.fadeOut();

	if (showDarkSideEnding1())
		if (showDarkSideEnding2())
			if (showDarkSideEnding3())
				showDarkSideEnding4();

	showDarkSideScore(endingScore);

	_claw.clear();
	_dragon1.clear();
	screen.fadeOut();
}

bool DarkSideCutscenes::showDarkSideEnding1() {
	EventsManager &events = *g_vm->_events;
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;

	sound.playSong("dngon3.m");
	screen.loadBackground("scene1.raw");
	screen.loadPalette("endgame.pal");
	screen.update();

	screen.fadeIn();
	WAIT(30);

	screen.fadeOut();
	screen.loadBackground("scene2-b.raw");
	screen.update();
	screen.saveBackground();
	SpriteResource::setClippedBottom(171);

	SpriteResource faceEnd("face.end");
	screen.restoreBackground();
	faceEnd.draw(0, 0, Common::Point(29, 76), SPRFLAG_BOTTOM_CLIPPED);
	screen.update();

	screen.fadeIn();
	WAIT(1);

	// Alamar stands up
	for (int yp = 74; yp > 20; yp -= 2) {
		if (yp == 60)
			sound.songCommand(207);
		else if (yp == 22)
			sound.stopSong();

		screen.restoreBackground();
		faceEnd.draw(0, 0, Common::Point(29, yp), SPRFLAG_BOTTOM_CLIPPED);
		screen.update();

		WAIT(2);
	}
	faceEnd.clear();

	// Alamar says "Come to me"
	sound.playVoice("come2.voc");
	_subtitles.setLine(21);
	WAIT(27);

	// Show the entire throne room
	screen.loadBackground("mainback.raw");
	SpriteResource sc03a("sc03a.end"), sc03b("sc03b.end"), sc03c("sc03c.end");
	sc03a.draw(0, 0, Common::Point(250, 0));

	screen.saveBackground();
	screen.update();
	WAIT(30);

	// Silhouette of door opening
	sound.playSound("door.voc");
	for (int idx = 0; idx < 6; ++idx) {
		screen.restoreBackground();
		sc03b.draw(0, idx, Common::Point(72, 125));
		screen.update();

		WAIT(4);
	}

	// Silhouette of playing entering
	for (int idx = 0; idx < 19; ++idx) {
		screen.restoreBackground();
		sc03c.draw(0, idx, Common::Point(72, 125));
		screen.update();

		if (idx == 3 || idx == 11)
			sound.playFX(7);
		if (idx == 7 || idx == 16)
			sound.playFX(8);

		WAIT(4);
	}

	sc03a.clear();
	sc03b.clear();
	sc03c.clear();

	// Box throwing
	screen.loadBackground("scene4.raw");
	screen.loadPage(0);
	screen.loadBackground("scene4-1.raw");
	screen.loadPage(1);
	SpriteResource disk("disk.end");
	File whoosh("whoosh.voc");

	screen.horizMerge();
	int yp = 101, tCtr = 0, frameNum = 0;
	for (int xp = 0; xp < 320; xp += 2) {
		screen.horizMerge(xp);

		disk.draw(0, frameNum, Common::Point(xp / 2, yp));
		if (xp < 44)
			disk.draw(0, 11, Common::Point(-xp, 22), SPRFLAG_800);
		disk.draw(0, 10, Common::Point(0, 134));

		if (!(xp % 22))
			sound.playSound(whoosh);
		WAIT(1);

		if (++tCtr % 2)
			frameNum = (frameNum + 1) % 10;

		if (xp < 100)
			--yp;
		else if (xp > 150)
			++yp;
	}

	whoosh.close();
	WAIT(10);

	// Play landing thud
	sound.playSound("thud.voc");
	while (!g_vm->shouldExit() && !events.isKeyMousePressed()
		&& sound.isSoundPlaying()) {
		events.pollEventsAndWait();
	}

	// Fade over to box opening background
	events.updateGameCounter();
	screen.freePages();
	screen.fadeOut();
	screen.loadBackground("sc050001.raw");
	screen.update();
	screen.saveBackground();
	screen.fadeIn();
	events.updateGameCounter();

	// Box opening
	SpriteResource diskOpen[5] = {
		SpriteResource("diska.end"), SpriteResource("diskb.end"),
		SpriteResource("diskc.end"), SpriteResource("diskd.end"),
		SpriteResource("diske.end")
	};
	File click("click.voc");

	for (int idx = 0; idx < 34; ++idx) {
		screen.restoreBackground();
		diskOpen[idx / 8].draw(0, idx % 8, Common::Point(28, 11));

		WAIT(3);
		if (idx == 0 || idx == 2 || idx == 4 || idx == 6)
			sound.playSound(click);
		else if (idx == 18)
			sound.playSound("explosio.voc");
		else if (idx == 9 || idx == 10 || idx == 11 || idx == 12)
			sound.playFX(30);
		else if (idx == 13)
			sound.playFX(14);
	}
	click.close();
	for (int idx = 0; idx < 5; ++idx)
		diskOpen[idx].clear();

	SpriteResource sc06[2] = {
		SpriteResource("sc06a.end"), SpriteResource("sc06b.end")
	};
	screen.loadBackground("mainback.raw");
	screen.saveBackground();
	sc06[0].draw(0, 0);
	screen.update();
	sound.playSound("alien2.voc");

	// Zoomed out throneroom view of beam coming out of box
	for (int idx = 0; idx < 20; ++idx) {
		if (idx == 6 || idx == 8 || idx == 9 || idx == 10
			|| idx == 13 || idx == 15 || idx == 16)
			sound.playFX(3);

		screen.restoreBackground();
		sc06[idx / 10].draw(0, idx % 10);
		screen.update();

		WAIT(3);
	}

	screen.fadeOut();
	sc06[0].clear();
	sc06[1].clear();

	// Corak appearing
	SpriteResource sc07[8] = {
		SpriteResource("sc07a.end"), SpriteResource("sc07b.end"),
		SpriteResource("sc07c.end"), SpriteResource("sc07d.end"),
		SpriteResource("sc07e.end"), SpriteResource("sc07f.end"),
		SpriteResource("sc07g.end"), SpriteResource("sc07h.end")
	};

	screen.loadBackground("sc070001.raw");
	screen.saveBackground();
	screen.update();
	screen.fadeIn();
	WAIT(2);

	for (int idx = 0; idx < 45; ++idx) {
		screen.restoreBackground();
		sc07[idx / 6].draw(0, idx % 6, Common::Point(61, 12));

		if (_subtitles.active() == 0)
			screen.update();
		else
			_subtitles.show();

		WAIT(2);
		if (idx == 40)
			_subtitles.setLine(22);
		else if (idx == 1 || idx == 19)
			// Wind storm
			sound.playSound("windstor.voc");
	}

	return true;
}

bool DarkSideCutscenes::showDarkSideEnding2() {
	EventsManager &events = *g_vm->_events;
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;

	// Corak?!
	sound.playVoice("corak2.voc");

	// Yep, that's my name, don't wear it out
	SpriteResource sc08("sc08.end");
	sound.playFX(0);

	for (int idx = 0; idx < 15; ++idx) {
		if (idx == 2) {
			if (!_subtitles.waitForLineOrSound())
				return false;
			sound.playVoice("yes1.voc");
			_subtitles.setLine(23);
		}

		// Animate Corak speaking
		sc08.draw(0, sound.isSoundPlaying() ? getSpeakingFrame(0, 2) : 0);
		_subtitles.show();
		WAIT(3);
	}

	if (!_subtitles.waitForLineOrSound())
		return false;
	sc08.clear();

	// Nowhere to run to
	SpriteResource sc09("sc09.end");

	screen.loadBackground("sc090001.raw");
	screen.saveBackground();
	screen.update();

	_subtitles.setLine(24);

	for (int idx = 0; idx < 15; ++idx) {
		if (idx == 2)
			sound.playVoice("nowre1.voc");

		screen.restoreBackground();
		sc09.draw(0, getSpeakingFrame(0, 8));
		_subtitles.show();
		WAIT(3);
	}

	if (!_subtitles.waitForLineOrSound())
		return false;
	sc09.clear();

	// Nor do you!
	SpriteResource sc10("sc10.end");
	_subtitles.setLine(25);

	for (int idx = 0; idx < 15; ++idx) {
		if (idx == 3)
			sound.playVoice("nordo2.voc");

		screen.restoreBackground();
		sc10.draw(0, getSpeakingFrame(0, 1));
		_subtitles.show();
		WAIT(3);
	}

	if (!_subtitles.waitForLineOrSound())
		return false;
	sc10.clear();

	// Closeup of side of Alamar's helmet
	SpriteResource sc11("sc11.end");
	sound.setMusicPercent(75);
	sound.playSong("dngon2.m");
	screen.fadeOut();
	screen.loadBackground("blank.raw");
	screen.saveBackground();

	for (int idx = 0; idx < 15; ++idx) {
		screen.restoreBackground();
		sc11.draw(0, idx, Common::Point(100, 44));

		if (idx == 0)
			screen.fadeIn();
		WAIT(3);

		if (idx == 3)
			sound.playSound("click.voc");
		else if (idx == 5)
			sound.playSound("airmon.voc");
	}

	screen.fadeOut();

	// Helmet raises, we see the face appear
	for (int idx = 80; idx > 20; idx -= 2) {
		screen.restoreBackground();
		sc11.draw(0, 15, Common::Point(60, 80));
		sc11.draw(0, 16, Common::Point(61, idx));
		sc11.draw(0, 17);
		screen.update();

		if (idx == 80)
			screen.fadeIn();

		WAIT(3);
	}

	// Sheltem is revealed
	sc11.clear();
	screen.fadeOut();
	screen.loadBackground("scene12.raw");
	screen.update();
	screen.fadeIn();
	WAIT(30);
	screen.fadeOut();

	// I'm ready for you this time
	SpriteResource sc13("sc13.end");
	_subtitles.setLine(26);
	sound.setMusicPercent(38);

	for (int idx = 0; idx < 16; ++idx) {
		if (idx == 1)
			sound.playVoice("ready2.voc");

		sc13.draw(0, getSpeakingFrame(0, 3));
		_subtitles.show();

		if (idx == 0)
			screen.fadeIn();
		WAIT(3);
	}

	do {
		sc13.draw(0, 0);
		_subtitles.show();
		events.pollEventsAndWait();
	} while (_subtitles.active() && !g_vm->shouldExit());

	sc13.clear();

	// This fight will be your last
	SpriteResource sc14("sc14.end");
	_subtitles.setLine(27);

	screen.fadeOut();
	screen.loadBackground("blank.raw");
	screen.saveBackground();

	sc14.draw(0, 0, Common::Point(37, 37));
	screen.update();
	screen.fadeIn();

	for (int waveNum = 0; waveNum < 2; ++waveNum) {
		for (int idx = 0; idx < 10; ++idx) {
			if (waveNum == 0 && idx == 3)
				sound.playSound("fight2.voc");

			screen.restoreBackground();
			sc14.draw(0, idx, Common::Point(37, 37));
			_subtitles.show();
			WAIT(3);
		}
	}

	if (!_subtitles.waitForLineOrSound())
		return false;

	return true;
}

bool DarkSideCutscenes::showDarkSideEnding3() {
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;

	// Fighting start
	SpriteResource sc15("sc15.end");
	screen.loadBackground("mainback.raw");
	screen.saveBackground();
	sc15.draw(0, 0, Common::Point(52, 84));
	screen.update();

	for (int idx = 0; idx < 21; ++idx) {
		screen.restoreBackground();
		sc15.draw(0, idx, Common::Point(52, 84));
		screen.update();
		WAIT(3);

		if (idx == 4)
			sound.playSound("disint.voc");
		else if (idx == 12)
			sound.playSound("explosio.voc");
	}

	sc15.clear();
	sound.stopSound();

	// I can not fail
	SpriteResource sc16("sc16.end");
	_subtitles.setLine(28);

	screen.loadBackground("scene4.raw");
	screen.loadPage(0);
	screen.loadBackground("scene4-1.raw");
	screen.loadPage(1);
	screen.loadBackground("sc170001.raw");
	screen.saveBackground();
	screen.horizMerge(0);
	sc16.draw(0, 0, Common::Point(7, 29));
	_subtitles.show();
	sound.playVoice("fail1.voc", 2);

	for (int idx = 0; idx < 5; ++idx) {
		screen.horizMerge(0);
		sc16.draw(0, idx, Common::Point(7, 29));
		_subtitles.show();
		WAIT(4);
	}

	// Force sphere goes horizontally
	int val = 5;
	for (int idx = 0; idx < 320; idx += 16) {
		screen.horizMerge(idx);
		sc16.draw(0, (val >= 12) ? 11 : val++,
			Common::Point(idx * -1 + 7, 29));

		if (val > 10)
			sc16.draw(0, 12, Common::Point(273, 70));

		_subtitles.show();
		WAIT(2);

		if (idx == 48)
			sound.playSound("alien2.voc");
	}

	screen.freePages();
	sc16.clear();

	// Sheltem protects himself with cloak
	SpriteResource sc17[2] = {
		SpriteResource("sc17xa.end"), SpriteResource("sc17xb.end")
	};
	screen.restoreBackground();
	screen.update();

	for (int idx = 0; idx < 16; ++idx) {
		screen.restoreBackground();
		sc17[idx / 8].draw(0, idx % 8, Common::Point(44, 19));
		screen.update();
		WAIT(3);

		if (idx == 9)
			sound.playSound("thud1.voc");
	}

	screen.fadeOut();
	sc17[0].clear();
	sc17[1].clear();

	// Sheltem fires on Corak
	SpriteResource sc18[4] = {
		SpriteResource("sc18a.end"), SpriteResource("sc18b.end"),
		SpriteResource("sc18c.end"), SpriteResource("sc18d.end")
	};
	screen.loadBackground("sc180001.raw");
	screen.saveBackground();
	screen.update();
	screen.fadeIn();

	for (int idx = 0; idx < 31; ++idx) {
		screen.restoreBackground();
		sc18[idx / 8].draw(0, idx % 8, Common::Point(29, 12));
		screen.update();
		WAIT(3);

		if (idx == 2)
			sound.playSound("gascompr.voc");
	}

	for (int idx = 0; idx < 4; ++idx)
		sc18[idx].clear();

	// Closeup of Corak bleeding
	SpriteResource sc19("sc19.end");
	screen.loadBackground("sc190001.raw");
	screen.saveBackground();
	screen.update();

	for (int idx = 0; idx < 28; ++idx) {
		if (idx == 20)
			sound.songCommand(207);

		screen.restoreBackground();
		sc19.draw(0, idx, Common::Point(33, 10));
		WAIT(3);
	}

	sc19.clear();
	sound.stopSong();
	screen.fadeOut();

	// Corak waving his hands
	SpriteResource sc20("sc20.end");
	sound.setMusicPercent(75);
	sound.playSong("sf17.m");
	screen.loadBackground("blank.raw");
	screen.saveBackground();
	screen.update();
	screen.fadeIn();

	for (int waveNum = 0; waveNum < 2; ++waveNum) {
		sound.playSound("cast.voc");

		for (int idx = 0; idx < 9; ++idx) {
			screen.restoreBackground();
			sc20.draw(0, idx, Common::Point(80, 18));
			screen.update();
			WAIT(3);
		}
	}

	sc20.clear();

	// Sheltem bounces back bolts that Corak fires
	SpriteResource sc21("sc21.end");
	File alien2("alien2.voc");
	screen.loadBackground("mainback.raw");
	screen.saveBackground();
	sc21.draw(0, 0, Common::Point(133, 81));
	screen.update();

	for (int boltNum = 0; boltNum < 4; ++boltNum) {
		sound.playSound(alien2);

		for (int idx = (boltNum == 0) ? 0 : 3; idx < 13; ++idx) {
			screen.restoreBackground();
			sc21.draw(0, idx, Common::Point(133, 81));
			WAIT(2);
		}
	}

	screen.restoreBackground();
	sc21.draw(0, 0, Common::Point(133, 81));
	screen.update();

	sc21.clear();
	alien2.close();

	// Sheltem fires force sphere at Corak
	SpriteResource sc22("sc22.end");
	screen.loadBackground("scene4.raw");
	screen.loadPage(0);
	screen.loadBackground("scene4-1.raw");
	screen.loadPage(1);
	screen.loadBackground("sc230001.raw");
	screen.saveBackground();
	screen.horizMerge(SCREEN_WIDTH);

	while (sound.isSoundPlaying()) {
		WAIT(1);
	}

	sc22.draw(0, 0, Common::Point(156, 28));
	screen.update();
	sound.playSound("cast.voc");

	for (int idx = 0; idx < 5; ++idx) {
		screen.horizMerge(SCREEN_WIDTH);
		sc22.draw(0, idx, Common::Point(156, 28));
		screen.update();
		WAIT(3);
	}

	// The sphere travels horizontally
	sound.playSound("gascompr.voc");

	int ctr = 5;
	for (int idx = SCREEN_WIDTH, xOffset = 0; idx >= 0; idx -= 16, xOffset += 16) {
		screen.horizMerge(idx);
		sc22.draw(0, (ctr >= 8) ? 7 : ctr++,
			Common::Point(xOffset + 156, 28), SPRFLAG_800);
		if (ctr > 7)
			sc22.draw(0, 8, Common::Point(136, 64));
		screen.update();

		WAIT(2);
	}

	screen.freePages();
	sc22.clear();

	// Corak raises shield and blocks sphere
	SpriteResource sc23[2] = {
		SpriteResource("sc23a.end"), SpriteResource("sc23b.end")
	};

	screen.restoreBackground();
	screen.update();
	sound.playSound("cast.voc");

	for (int idx = 0; idx < 19; ++idx) {
		screen.restoreBackground();
		sc23[idx / 12].draw(0, idx % 12, Common::Point(72, 4));
		screen.update();
		WAIT(3);

		if (idx == 5)
			sound.playSound("padspell.voc");
		else if (idx == 11)
			sound.playFX(9);
	}

	screen.restoreBackground();
	sc23[0].draw(0, 0, Common::Point(72, 4));
	screen.update();
	screen.fadeOut();

	return true;
}

bool DarkSideCutscenes::showDarkSideEnding4() {
	EventsManager &events = *g_vm->_events;
	FileManager &files = *g_vm->_files;
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;

	// Corak does a ricochet shot on Sheltem
	SpriteResource sc24[2] = {
		SpriteResource("sc24a.end"), SpriteResource("sc24b.end")
	};

	screen.loadBackground("mainback.raw");
	screen.saveBackground();
	sc24[0].draw(0, 0, Common::Point(148, 0));
	screen.update();
	screen.fadeIn(4);
	sound.playSound("cast.voc");

	for (int idx = 0; idx < 30; ++idx) {
		screen.restoreBackground();
		sc24[idx / 15].draw(0, idx % 15, Common::Point(148, 0));
		screen.update();
		WAIT(3);

		if (idx == 6)
			sound.playSound("padspell.voc");
		else if (idx == 11 || idx == 14)
			sound.playSound("click.voc");
		else if (idx == 17)
			sound.playSound("thud1.voc");
	}

	sc24[0].clear();
	sc24[1].clear();

	// Admit your defeat Corak
	// Interestingly, the original also has a sc25b.end here, which was of
	// Corak speaking here. It seems this was ditched in favor of the
	// following closeup of him speaking "I do"
	SpriteResource sc25("sc25a.end");
	_subtitles.setLine(29);

	screen.loadBackground("sc250001.raw");
	screen.saveBackground();
	sc25.draw(0, 0, Common::Point(27, 30));
	screen.update();

	for (int struggleNum = 0; struggleNum < 2; ++struggleNum) {
		for (int idx = 0; idx < 9; ++idx) {
			if (struggleNum == 0 && idx == 2)
				sound.playVoice("admit2.voc");

			sc25.draw(0, idx, Common::Point(27, 30));
			_subtitles.show();
			WAIT(3);
		}
	}

	sc25.clear();

	// I do. Kamakazi time
	SpriteResource sc26[4] = {
		SpriteResource("sc26a.end"), SpriteResource("sc26b.end"),
		SpriteResource("sc26c.end"), SpriteResource("sc26d.end")
	};
	_subtitles.setLine(30);

	screen.loadBackground("sc260001.raw");
	screen.saveBackground();
	_subtitles.show();
	sound.playVoice("ido2.voc");

	for (int idx = 0; sound.isSoundPlaying() || _subtitles.active(); ) {
		screen.restoreBackground();
		sc26[idx / 8].draw(0, idx % 8, Common::Point(58, 25));
		WAIT(2);

		idx = (idx == 31) ? 22 : idx + 1;
	}

	screen.loadBackground("sc270001.raw");
	screen.saveBackground();

	while (sound.isSoundPlaying() && !g_vm->shouldExit()) {
		events.pollEventsAndWait();
		if (events.isKeyMousePressed())
			return false;
	}

	for (int idx = 0; idx < 4; ++idx)
		sc26[idx].clear();

	// What? No!
	SpriteResource sc27("sc27.end");
	_subtitles.setLine(31);
	_subtitles.show();

	for (int idx = 0; idx < 12; ++idx) {
		if (idx == 1)
			sound.playVoice("what3.voc");

		screen.restoreBackground();
		sc27.draw(0, idx, Common::Point(65, 22));
		_subtitles.show();
		WAIT(3);
	}

	sc27.clear();

	// Vortex is opened and the two are sucked in, obliterating them
	files.setGameCc(2);
	SpriteResource sc28[11] = {
		SpriteResource("sca28.end"), SpriteResource("scb28.end"),
		SpriteResource("scc28.end"), SpriteResource("scd28.end"),
		SpriteResource("sce28.end"), SpriteResource("scf28.end"),
		SpriteResource("scg28.end"), SpriteResource("sch28.end"),
		SpriteResource("sci28.end"), SpriteResource("sck28.end"),
		SpriteResource("sck28.end")
	};

	sound.playSong("dungeon2.m");
	screen.fadeOut();
	screen.loadBackground("mainback.raw");
	screen.saveBackground();

	sc28[0].draw(0, 0, Common::Point(74, 0));
	screen.update();
	screen.fadeIn();
	files.setGameCc(1);

	for (int idx = 0; idx < 44; ++idx) {
		screen.restoreBackground();
		sc28[idx / 4].draw(0, idx % 4, Common::Point(74, 0));
		screen.update();
		WAIT(3);

		if (idx == 0 || idx == 10 || idx == 18)
			sound.playSound("rumble.voc");
		else if (idx == 27)
			sound.playSound("airmon.voc");
		else if (idx == 38)
			sound.playSound("explosio.voc");
	}

	screen.fadeOut();
	for (int idx = 0; idx < 11; ++idx)
		sc28[idx].clear();

	// Longshot of castle blowing up
	SpriteResource sc29[6] = {
		SpriteResource("sc29a.end"), SpriteResource("sc29b.end"),
		SpriteResource("sc29c.end"), SpriteResource("sc29d.end"),
		SpriteResource("sc29e.end"), SpriteResource("sc29f.end")
	};

	screen.loadBackground("sc290001.raw");
	screen.saveBackground();
	screen.fadeIn();

	for (int idx = 0; idx < 54; ++idx) {
		screen.restoreBackground();
		if (idx == 40)
			sound.songCommand(207);

		sc29[idx / 10].draw(0, idx % 10, Common::Point(92, 10));
		screen.update();
		WAIT(3);

		if (idx == 1 || idx == 5 || idx == 11 || idx == 19 || idx == 34 || idx == 35)
			sound.playSound("explosio.voc");
	}

	// Final bit of smoke from destroyed castle
	for (int loopNum = 0; loopNum < 4; ++loopNum) {
		for (int idx = 49; idx < 54; ++idx) {
			screen.restoreBackground();
			sc29[idx / 10].draw(0, idx % 10, Common::Point(92, 10));
			screen.update();
			WAIT(3);
		}
	}

	screen.fadeOut();
	sound.stopSong();
	return true;
}

void DarkSideCutscenes::showDarkSideScore(uint endingScore) {
	SavesManager &saves = *g_vm->_saves;
	Sound &sound = *g_vm->_sound;

	sound.stopAllAudio();

	if (!g_vm->shouldExit()) {
		sound.playSong("outday3.m");

		Common::String str = Common::String::format(Res.DARKSIDE_ENDING1, endingScore);
		showPharaohEndText(str.c_str(), Res.DARKSIDE_ENDING2);

		g_vm->_mode = MODE_INTERACTIVE;
		if (!g_vm->shouldExit())
			saves.saveGame();
	}
}

bool DarkSideCutscenes::showPharaohEndText(const char *msg1, const char *msg2, const char *msg3) {
	Windows &windows = *g_vm->_windows;
	_ball.load("ball.int");
	_claw.load("claw.int");
	_dragon1.load("dragon1.int");

	windows[39].setBounds(Common::Rect(12, 8, 162, 198));
	bool result = showPharaohEndTextInner(msg1, msg2, msg3);

	_ball.clear();
	_claw.clear();
	_dragon1.clear();
	return result;
}

bool DarkSideCutscenes::showPharaohEndTextInner(const char *msg1, const char *msg2, const char *msg3) {
	Screen &screen = *g_vm->_screen;
	EventsManager &events = *g_vm->_events;
	Windows &windows = *g_vm->_windows;
	int numPages = 0 + (msg1 ? 1 : 0) + (msg2 ? 1 : 0) + (msg3 ? 1 : 0);
	const char *const text[3] = { msg1, msg2, msg3 };

	screen.loadBackground("3room.raw");
	screen.saveBackground();
	screen.loadPalette("dark.pal");

	int clawCtr = 0;
	ANIMATE_PHAROAH;
	screen.fadeIn();
	events.clearEvents();

	// Iterate through showing the pages
	for (int pageNum = 0; !g_vm->shouldExit() && pageNum < numPages; ++pageNum) {
		// Show each page until a key is pressed
		do {
			events.updateGameCounter();
			screen.restoreBackground();
			animatePharoah(clawCtr, false);
			clawCtr = (clawCtr + 1) % 32;

			// Form the text string to display the text
			Common::String str1 = Common::String::format(Res.PHAROAH_ENDING_TEXT1,
				text[pageNum]);
			windows[39].writeString(str1);

			Common::String str2 = Common::String::format(Res.PHAROAH_ENDING_TEXT2,
				text[pageNum]);
			windows[39].writeString(str2);

			windows[0].update();
			events.wait(1);
		} while (!g_vm->shouldExit() && !events.isKeyMousePressed());

		events.clearEvents();
	}

	return true;
}

} // End of namespace WorldOfXeen
} // End of namespace Xeen
