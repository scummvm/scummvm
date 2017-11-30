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
#include "xeen/worldofxeen/worldofxeen_resources.h"

namespace Xeen {
namespace WorldOfXeen {

static const int CUTSCENES_XLIST[32] = {
	146, 145, 143, 141, 141, 141, 141, 141, 141, 141, 141, 142, 143, 144, 145, 146,
	146, 146, 146, 146, 146, 146, 146, 146, 146, 146, 146, 146, 146, 146, 146, 146
};
static const int CUTSCENES_YLIST[32] = {
	143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143,
	143, 143, 144, 145, 145, 145, 145, 145, 145, 145, 145, 144, 143, 143, 143, 143
};

bool DarkSideCutscenes::showDarkSideTitle() {
	EventsManager &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	_vm->_files->_isDarkCc = true;

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
	screen.draw();
	screen.fadeIn();

	sound.setMusicVolume(0x5f);
	sound.playFX(1);

	// Initial loop for dragon roaring
	int nwcIndex = 0, nwcFrame = 0;
	for (int idx = 0; idx < 55 && !_vm->shouldQuit(); ++idx) {
		// Render the next frame
		screen.vertMerge(0);
		nwc[nwcIndex].draw(screen, nwcFrame);
		screen.draw();

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
	for (int idx = 0; idx < 42 && !_vm->shouldQuit(); ++idx) {
		screen.vertMerge(SCREEN_HEIGHT);
		nwc[3].draw(screen, idx);
		screen.draw();

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
	if (_vm->shouldQuit())
		return false;

	// Pause for a bit
	WAIT(10);

	sound.setMusicVolume(95);

	screen.fadeOut(8);
	screen.loadBackground("jvc.raw");
	screen.draw();
	screen.fadeIn();

	WAIT(60);
	return true;
}

bool DarkSideCutscenes::showDarkSideIntro() {
	EventsManager &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	const int XLIST1[] = {
		0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 58, 60, 62
	};
	const int YLIST1[] = {
		0, 5, 10, 15, 20, 25, 30, 35, 40, 40, 39, 37, 35, 33, 31
	};
	const int XLIST2[] = {
		160, 155, 150, 145, 140, 135, 130, 125, 120, 115, 110, 105, 98, 90, 82
	};

	_vm->_files->_isDarkCc = true;
	_subtitles.load("special.bin");
	screen.fadeOut(8);
	screen.loadPalette("dark.pal");
	screen.loadBackground("pyramid2.raw");
	screen.loadPage(0);
	screen.loadPage(1);
	screen.loadBackground("pyramid3.raw");
	screen.saveBackground(1);

	SpriteResource title("title.int");
	SpriteResource pyraTop("pyratop.int");
	SpriteResource pyramid("pyramid.int");

	screen.vertMerge(SCREEN_HEIGHT);
	screen.loadPage(0);
	screen.restoreBackground();
	screen.loadPage(1);

	// Play the intro music
	sound.playSong("bigtheme.m");

	// Show Might and Magic Darkside of Xeen title, and gradualy scroll
	// the background vertically down to show the Pharoah's base
	int yp = 0;
	int idx1 = 0;
	bool skipElapsed = false;
	uint32 timeExpired = 0;
	bool fadeFlag = true;

	for (int yCtr = SCREEN_HEIGHT, frameNum = 0; yCtr > 0; ) {
		screen.vertMerge(yp);

		title.draw(screen, 0);
		if (frameNum)
			title.draw(screen, frameNum);

		idx1 = (idx1 + 1) % 4;
		if (!idx1)
			frameNum = (frameNum + 1) % 10;

		screen.draw();
		if (!skipElapsed) {
			timeExpired = MAX((int)events.timeElapsed() - 1, 1);
			skipElapsed = true;
		}

		yCtr -= timeExpired;
		yp = MIN((uint)(yp + timeExpired), (uint)200);

		WAIT(1);

		if (fadeFlag) {
			screen.fadeIn();
			fadeFlag = false;
		}
	}

	screen.vertMerge(SCREEN_HEIGHT);
	screen.saveBackground(1);
	screen.draw();
	screen.freePages();

	WAIT(30);

	// Zoom into the Pharoah's base closeup view
	for (int idx = 14; idx >= 0; --idx) {
		pyraTop.draw(screen, 0, Common::Point(XLIST1[idx], YLIST1[idx]));
		pyraTop.draw(screen, 1, Common::Point(XLIST2[idx], YLIST1[idx]));
		screen.draw();

		if (idx == 2)
			sound.setMusicVolume(48);
		WAIT(2);
	}

	// Inconceivable, the royal pyramid besieged
	screen.saveBackground();
	sound.playSound("pharoh1a.voc");

	recordTime();
	resetSubtitles(0);
	_subtitleLineNum = 0;

	bool phar2 = false;
	for (int idx = 0; idx < 19; ++idx) {
		screen.restoreBackground();
		pyramid.draw(screen, idx, Common::Point(132, 62));
		showSubtitles();

		if (!sound.isPlaying() && !phar2)
			sound.playSound("pharoh1b.voc");

		events.updateGameCounter();
		while (timeElapsed() < 4) {
			showSubtitles();
			events.pollEventsAndWait();
			if (events.isKeyMousePressed())
				return false;
		}
	}

	waitForLineOrSound();
	screen.fadeOut();
	screen.freePages();
	title.clear();
	pyraTop.clear();
	pyramid.clear();

	//
	SpriteResource dragon("dragon.int");
	const int XLIST3[10] = { 102, 103, 104, 104, 104, 103, 102, 101, 101, 101 };
	const int YLIST3[10] = { 30, 29, 28, 27, 26, 25, 24, 25, 26, 28 };
	const int FRAMES3[70] = {
		9, 9, 9, 9, 9, 8, 8, 8, 8, 9, 9, 9, 9, 9, 0,
		1, 2, 3, 4, 5, 6, 7, 8, 9, 9, 9, 9, 9, 8, 8,
		8, 8, 9, 9, 9, 9, 165, 149, 126, 106, 92, 80, 66, 55, 42,
		29, 10,  -6, -26, -40, -56, -72, -83, 154, 141, 125, 105, 94, 83, 74,
		69, 68, 70, 73, 77, 83, 89, 94, 99, 109
	};
	const char *const PHAR2_VOC[5] = {
		"pharoh2a.voc", "pharoh2b.voc", "pharoh2c.voc", "pharoh2d.voc", "pharoh2e.voc"
	};
	recordTime();
	resetSubtitles(0);
	_subtitleLineNum = 0;
	_subtitleSize = 25;

	screen.loadBackground("2room.raw");
	screen.loadPage(1);
	screen.loadBackground("3room.raw");
	screen.loadPage(0);
	screen.loadBackground("1room.raw");
	screen.horizMerge(SCREEN_WIDTH);
	dragon.draw(screen, 0, Common::Point(XLIST3[0], YLIST3[0]), SPRFLAG_800);

	int posNum = 0, phar2Index = 0, ctr = 0;
	for (int idx = SCREEN_WIDTH, frameNum = 0; idx >= 0; --idx) {
		events.updateGameCounter();
		screen.horizMerge(idx);
		dragon.draw(screen, FRAMES3[frameNum], Common::Point(XLIST3[posNum], YLIST3[posNum]), SPRFLAG_800);
		showSubtitles();
		events.pollEventsAndWait();
		if (events.isKeyMousePressed())
			return false;

		if (idx == SCREEN_WIDTH)
			sound.playSound(PHAR2_VOC[0]);
		if (!sound.isPlaying() && phar2Index < 4)
			sound.playSound(PHAR2_VOC[1 + phar2Index++]);

		if (phar2Index == 4) {
			if (!sound.isPlaying() && !_subtitleSize)
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

	return true;
}

bool DarkSideCutscenes::showDarkSideEnding() {
	EventsManager &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	_vm->_files->_isDarkCc = true;

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

	SpriteResource faceEnd("face.end");
	screen.restoreBackground();
	faceEnd.draw(screen, 0, Common::Point(29, 76), SPRFLAG_4000);
	screen.update();

	screen.fadeIn();
	WAIT(1);

	_subtitles.load("special.bin");
	resetSubtitles(21, 0);

	// Alamar stands up
	for (int idx = 74; idx > 20; idx -= 2) {
		if (idx == 60)
			sound.songCommand(207);
		else if (idx == 22)
			sound.stopSong();

		screen.restoreBackground();
		faceEnd.draw(screen, 0, Common::Point(29, idx), SPRFLAG_4000);
		screen.update();

		WAIT(2);
	}
	faceEnd.clear();

	// Alamar says "Come to me"
	sound.playSound("come2.voc");
	WAIT(27);

	// Show the entire throne room
	screen.loadBackground("mainback.raw");
	SpriteResource sc03a("sc03a.end"), sc03b("sc03b.end"), sc03c("sc03c.end");
	sc03a.draw(screen, 0, Common::Point(250, 0));

	screen.saveBackground();
	screen.update();
	WAIT(30);

	// Silhouette of door opening
	sound.playSound("door.voc");
	for (int idx = 0; idx < 6; ++idx) {
		screen.restoreBackground();
		sc03b.draw(screen, idx, Common::Point(72, 125));
		screen.update();

		WAIT(4);
	}

	// Silhouette of playing entering
	for (int idx = 0; idx < 19; ++idx) {
		screen.restoreBackground();
		sc03c.draw(screen, idx, Common::Point(72, 125));
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

		disk.draw(screen, frameNum, Common::Point(xp / 2, yp));
		if (xp < 44)
			disk.draw(screen, 11, Common::Point(-xp, 22), SPRFLAG_800);
		disk.draw(screen, 10, Common::Point(0, 134));

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
	while (!_vm->shouldQuit() && !events.isKeyMousePressed()
			&& sound.isPlaying()) {
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
		diskOpen[idx / 8].draw(screen, idx % 8, Common::Point(28, 11));

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
	sc06[0].draw(screen, 0);
	screen.update();
	sound.playSound("alien2.voc");

	// Zoomed out throneroom view of beam coming out of box
	for (int idx = 0; idx < 20; ++idx) {
		if (idx == 6 || idx == 8 || idx == 9 || idx == 10
				|| idx == 13 || idx == 15 || idx == 16)
			sound.playFX(3);

		screen.restoreBackground();
		sc06[idx / 10].draw(screen, idx % 10);
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

	resetSubtitles(22, 0);

	for (int idx = 0; idx < 45; ++idx) {
		screen.restoreBackground();
		sc07[idx / 6].draw(screen, idx % 6, Common::Point(61, 12));

		if (_subtitleSize == 0)
			screen.update();
		else
			showSubtitles();

		WAIT(2);
		if (idx == 40)
			_subtitleSize = 1;
		else if (idx == 1 || idx == 19)
			// Wind storm
			sound.playSound("windstor.voc");
	}

	for (int idx = 0; idx < 8; ++idx)
		sc07[idx].clear();

	// Corak?!
	sound.playSound("corak2.voc");
	subtitlesWait(30);

	// Yep, that's my name, don't wear it out
	SpriteResource sc08("sc08.end");
	resetSubtitles(23);
	sound.playFX(0);

	for (int idx = 0; idx < 15; ++idx) {
		if (idx == 2)
			sound.playSound("yes1.voc");

		// Animate Corak speaking
		sc08.draw(screen, sound.isPlaying() ? getSpeakingFrame(0, 2) : 0);
		showSubtitles();
		WAIT(3);
	}

	do {
		sc08.draw(screen, 0);
		showSubtitles();
	} while (_subtitleSize);

	sc08.clear();

	// Nowhere to run to
	SpriteResource sc09("sc09.end");

	screen.loadBackground("sc090001.raw");
	screen.saveBackground();
	screen.update();

	resetSubtitles(24);

	for (int idx = 0; idx < 15; ++idx) {
		if (idx == 2)
			sound.playSound("nowre1.voc");

		screen.restoreBackground();
		sc09.draw(screen, getSpeakingFrame(0, 8));
		showSubtitles();
		WAIT(3);
	}

	do {
		screen.restoreBackground();
		showSubtitles();
	} while (_subtitleSize);

	sc09.clear();

	// Nor do you!
	SpriteResource sc10("sc10.end");
	resetSubtitles(25);

	for (int idx = 0; idx < 15; ++idx) {
		if (idx == 3)
			sound.playSound("nordo2.voc");

		screen.restoreBackground();
		sc10.draw(screen, getSpeakingFrame(0, 1));
		showSubtitles();
		WAIT(3);
	}

	do {
		screen.restoreBackground();
		showSubtitles();
	} while (_subtitleSize);

	sc10.clear();

	// Closeup of side of Alamar's helmet
	SpriteResource sc11("sc11.end");
	sound.setMusicVolume(95);
	sound.playSong("dngon2.m");
	screen.fadeOut();
	screen.loadBackground("blank.raw");
	screen.saveBackground();

	for (int idx = 0; idx < 15; ++idx) {
		screen.restoreBackground();
		sc11.draw(screen, idx, Common::Point(100, 44));

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
		sc11.draw(screen, 15, Common::Point(60, 80));
		sc11.draw(screen, 16, Common::Point(61, idx));
		sc11.draw(screen, 17);
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
	resetSubtitles(26);
	sound.setMusicVolume(48);

	for (int idx = 0; idx < 16; ++idx) {
		if (idx == 1)
			sound.playSound("ready2.voc");

		sc13.draw(screen, getSpeakingFrame(0, 3));
		showSubtitles();

		if (idx == 0)
			screen.fadeIn();
		WAIT(3);
	}

	do {
		sc13.draw(screen, 0);
		showSubtitles();
		events.pollEventsAndWait();
	} while (_subtitleSize);

	sc13.clear();

	// This fight will be your last
	SpriteResource sc14("sc14.end");
	resetSubtitles(27);

	screen.fadeOut();
	screen.loadBackground("blank.raw");
	screen.saveBackground();

	sc14.draw(screen, 0, Common::Point(37, 37));
	screen.update();
	screen.fadeIn();

	for (int waveNum = 0; waveNum < 2; ++waveNum) {
		for (int idx = 0; idx < 10; ++idx) {
			if (waveNum == 0 && idx == 3)
				sound.playSound("fight2.voc");

			screen.restoreBackground();
			sc14.draw(screen, idx, Common::Point(37, 37));
			showSubtitles();
			WAIT(3);
		}
	}

	while (_subtitleSize) {
		showSubtitles();
		events.pollEventsAndWait();
		showSubtitles();
	}
	sc14.clear();

	// Fighting start
	SpriteResource sc15("sc15.end");
	screen.loadBackground("mainback.raw");
	screen.saveBackground();
	sc15.draw(screen, 0, Common::Point(52, 84));
	screen.update();

	for (int idx = 0; idx < 21; ++idx) {
		screen.restoreBackground();
		sc15.draw(screen, idx, Common::Point(52, 84));
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
	resetSubtitles(28);

	screen.loadBackground("scene4.raw");
	screen.loadPage(0);
	screen.loadBackground("scene4-1.raw");
	screen.loadPage(1);
	screen.loadBackground("sc170001.raw");
	screen.saveBackground();
	screen.horizMerge(0);
	sc16.draw(screen, 0, Common::Point(7, 29));
	showSubtitles();
	sound.playSound("fail1.voc");

	for (int idx = 0; idx < 5; ++idx) {
		screen.horizMerge(0);
		sc16.draw(screen, idx, Common::Point(7, 29));
		showSubtitles();
		WAIT(4);
	}

	// Force sphere goes horizontally
	int val = 5;
	for (int idx = 0; idx < 320; idx += 16) {
		screen.horizMerge(idx);
		sc16.draw(screen, (val >= 12) ? 11 : val++,
			Common::Point(idx * -1 + 7, 29));

		if (val > 10)
			sc16.draw(screen, 12, Common::Point(273, 70));

		showSubtitles();
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
		sc17[idx / 8].draw(screen, idx % 8, Common::Point(44, 19));
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
		sc18[idx / 8].draw(screen, idx % 8, Common::Point(29, 12));
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
		sc19.draw(screen, idx, Common::Point(33, 10));
		WAIT(3);
	}

	sc19.clear();
	sound.stopSong();
	screen.fadeOut();

	// Corak waving his hands
	SpriteResource sc20("sc20.end");
	sound.setMusicVolume(95);
	sound.playSong("sf17.m");
	screen.loadBackground("blank.raw");
	screen.saveBackground();
	screen.update();
	screen.fadeIn();

	for (int waveNum = 0; waveNum < 2; ++waveNum) {
		sound.playSound("cast.voc");

		for (int idx = 0; idx < 9; ++idx) {
			screen.restoreBackground();
			sc20.draw(screen, idx, Common::Point(80, 18));
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
	sc21.draw(screen, 0, Common::Point(133, 81));
	screen.update();

	for (int boltNum = 0; boltNum < 4; ++boltNum) {
		sound.playSound(alien2);

		for (int idx = (boltNum == 0) ? 0 : 3; idx < 13; ++idx) {
			screen.restoreBackground();
			sc21.draw(screen, idx, Common::Point(133, 81));
			WAIT(2);
		}
	}

	screen.restoreBackground();
	sc21.draw(screen, 0, Common::Point(133, 81));
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

	while (sound.isPlaying()) {
		WAIT(1);
	}

	sc22.draw(screen, 0, Common::Point(156, 28));
	screen.update();
	sound.playSound("cast.voc");

	for (int idx = 0; idx < 5; ++idx) {
		screen.horizMerge(SCREEN_WIDTH);
		sc22.draw(screen, idx, Common::Point(156, 28));
		screen.update();
		WAIT(3);
	}

	// The sphere travels horizontally
	sound.playSound("gascompr.voc");

	int ctr = 5;
	for (int idx = SCREEN_WIDTH, xOffset = 0; idx >= 0; idx -= 16, xOffset += 16) {
		screen.horizMerge(idx);
		sc22.draw(screen, (ctr >= 8) ? 7 : ctr++,
			Common::Point(xOffset + 156, 28), SPRFLAG_800);
		if (ctr > 7)
			sc22.draw(screen, 8, Common::Point(136, 64));
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
		sc23[idx / 12].draw(screen, idx % 12, Common::Point(72, 4));
		screen.update();
		WAIT(3);

		if (idx == 5)
			sound.playSound("padspell.voc");
		else if (idx == 11)
			sound.playFX(9);
	}

	screen.restoreBackground();
	sc23[0].draw(screen, 0, Common::Point(72, 4));
	screen.update();
	screen.fadeOut();

	sc23[0].clear();
	sc23[1].clear();

	// Corak does a ricochet shot on Sheltem
	SpriteResource sc24[2] = {
		SpriteResource("sc24a.end"), SpriteResource("sc24b.end")
	};

	screen.loadBackground("mainback.raw");
	screen.saveBackground();
	sc24[0].draw(screen, 0, Common::Point(148, 0));
	screen.update();
	screen.fadeIn(4);
	sound.playSound("cast.voc");

	for (int idx = 0; idx < 30; ++idx) {
		screen.restoreBackground();
		sc24[idx / 15].draw(screen, idx % 15, Common::Point(148, 0));
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
	resetSubtitles(29);

	screen.loadBackground("sc250001.raw");
	screen.saveBackground();
	sc25.draw(screen, 0, Common::Point(27, 30));
	screen.update();

	for (int struggleNum = 0; struggleNum < 2; ++struggleNum) {
		for (int idx = 0; idx < 9; ++idx) {
			if (struggleNum == 0 && idx == 2)
				sound.playSound("admit2.voc");

			sc25.draw(screen, idx, Common::Point(27, 30));
			showSubtitles();
			WAIT(3);
		}
	}
	subtitlesWait();
	sc25.clear();

	// I do. Kamakazi time
	SpriteResource sc26[4] = {
		SpriteResource("sc26a.end"), SpriteResource("sc26b.end"),
		SpriteResource("sc26c.end"), SpriteResource("sc26d.end")
	};
	resetSubtitles(30);

	screen.loadBackground("sc260001.raw");
	screen.saveBackground();
	showSubtitles();
	sound.playSound("ido2.voc");

	for (int idx = 0; sound.isPlaying() || _subtitleSize; ) {
		screen.restoreBackground();
		sc26[idx / 8].draw(screen, idx % 8, Common::Point(58, 25));
		WAIT(2);

		idx = (idx == 31) ? 22 : idx + 1;
	}

	screen.loadBackground("sc270001.raw");
	screen.saveBackground();

	while (sound.isPlaying() && !_vm->shouldQuit()) {
		events.pollEventsAndWait();
		if (events.isKeyMousePressed())
			return false;
	}

	for (int idx = 0; idx < 4; ++idx)
		sc26[idx].clear();

	// What? No!
	SpriteResource sc27("sc27.end");
	resetSubtitles(31);
	showSubtitles();

	for (int idx = 0; idx < 12; ++idx) {
		if (idx == 1)
			sound.playSound("what3.voc");

		screen.restoreBackground();
		sc27.draw(screen, idx, Common::Point(65, 22));
		showSubtitles();
		WAIT(3);
	}

	subtitlesWait();
	sc27.clear();

	// Vortex is opened and the two are sucked in, obliterating them
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

	sc28[0].draw(screen, 0, Common::Point(74, 0));
	screen.update();
	screen.fadeIn();

	for (int idx = 0; idx < 44; ++idx) {
		screen.restoreBackground();
		sc28[idx / 4].draw(screen, idx % 4, Common::Point(74, 0));
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

		sc29[idx / 10].draw(screen, idx % 10, Common::Point(92, 10));
		screen.update();
		WAIT(3);

		if (idx == 1 || idx == 5 || idx == 11 || idx == 19 || idx == 34 || idx == 35)
			sound.playSound("explosio.voc");
	}

	// Final bit of smoke from destroyed castle
	for (int loopNum = 0; loopNum < 4; ++loopNum) {
		for (int idx = 49; idx < 54; ++idx) {
			screen.restoreBackground();
			sc29[idx / 10].draw(screen, idx % 10, Common::Point(92, 10));
			screen.update();
			WAIT(3);
		}
	}

	screen.fadeOut();
	sound.stopSong();
	for (int idx = 0; idx < 6; ++idx)
		sc29[idx].clear();

	freeSubtitles();
	return true;
}

void DarkSideCutscenes::showDarkSideScore() {
	Common::String str = Common::String::format(Res.DARKSIDE_ENDING1, _vm->_endingScore);
	showPharaohEndText(str.c_str(), Res.DARKSIDE_ENDING2);
}

void DarkSideCutscenes::showPharaohEndText(const char *msg1, const char *msg2, const char *msg3) {
	const int YLIST[32] = {
		-3, -3, -3, -3, -3, -3, -3, -3, -1, 0, 0, 0, 0, 0, 0, 0,
		-1, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3
	};
	const int FRAMES[32] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 3, 3, 2, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	const int XLIST2[32] = {
		223, 222, 220, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219,
		219, 219, 220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 221
	};
	const int YLIST2[32] = {
		116, 116, 116, 117, 117, 117, 117, 117, 118, 118, 118, 118, 118, 118, 118, 118,
		118, 118, 118, 118, 118, 118, 118, 118, 118, 118, 118, 118, 118, 118, 118, 118
	};
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;
	Windows &windows = *_vm->_windows;
	SpriteResource claw("claw.int");
	SpriteResource dragon1("dragon1.int");
	int numPages = 0 + (msg1 ? 1 : 0) + (msg2 ? 1 : 0) + (msg3 ? 1 : 0);
	const char *const text[3] = { msg1, msg2, msg3 };

	screen.loadBackground("3room.raw");
	screen.saveBackground();
	screen.loadPalette("dark.pal");
	claw.draw(screen, 5, Common::Point(CUTSCENES_XLIST[0], CUTSCENES_YLIST[0]), SPRFLAG_800);
	claw.draw(screen, 6, Common::Point(149, 184));
	dragon1.draw(screen, FRAMES[0], Common::Point(139, YLIST[0]), SPRFLAG_800);
	claw.draw(screen, 0, Common::Point(XLIST2[0], YLIST2[0]), SPRFLAG_800);
	screen.update();
	screen.fadeIn();
	events.clearEvents();

	// Iterate through showing the pages
	int idx = 1;
	for (int pageNum = 0; !_vm->shouldQuit() && pageNum < numPages; ++pageNum) {
		// Show each page until a key is pressed
		do {
			// Draw the dragon pharoah
			screen.restoreBackground();
			claw.draw(screen, 5, Common::Point(CUTSCENES_XLIST[idx], CUTSCENES_YLIST[idx]), SPRFLAG_800);
			claw.draw(screen, 6, Common::Point(145, 185));
			dragon1.draw(screen, FRAMES[idx], Common::Point(139, YLIST[idx]), SPRFLAG_800);
			claw.draw(screen, idx % 5, Common::Point(XLIST2[idx], YLIST2[idx]), SPRFLAG_800);

			// Form the text string to display the text
			Common::String str1 = Common::String::format(Res.PHAROAH_ENDING_TEXT1,
				text[pageNum]);
			windows[39].writeString(str1);

			Common::String str2 = Common::String::format(Res.PHAROAH_ENDING_TEXT2,
				text[pageNum]);
			windows[39].writeString(str2);

			idx = (idx + 1) % 32;
			screen.update();

			events.pollEventsAndWait();
		} while (!_vm->shouldQuit() && !events.isKeyMousePressed());

		events.clearEvents();
	}
}

} // End of namespace WorldOfXeen
} // End of namespace Xeen
