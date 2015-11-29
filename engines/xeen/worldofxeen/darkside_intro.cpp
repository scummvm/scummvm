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

#include "xeen/worldofxeen/worldofxeen_game.h"
#include "xeen/sound.h"

namespace Xeen {

void showDarkSideTitle(XeenEngine &vm) {
	EventsManager &events = *vm._events;
	Screen &screen = *vm._screen;
	SoundManager &sound = *vm._sound;
	
	// TODO: Starting method, and sound
	//sub_28F40
	screen.loadPalette("dark.pal");
	SpriteResource nwc[4] = {
		SpriteResource("nwc1.int"), SpriteResource("nwc2.int"),
		SpriteResource("nwc3.int"), SpriteResource("nwc4.int")
	};
	VOC voc[3];
	voc[0].open("dragon1.voc");
	voc[1].open("dragon2.voc");
	voc[2].open("dragon3.voc");
/*
	Common::File f;
	f.open("adsnd");
	Common::DumpFile df;
	byte *b = new byte[f.size()];
	f.read(b, f.size());
	df.open("d:\\temp\\adsnd.bin");
	df.write(b, f.size());
	df.close();
	f.close();
	delete[] b;
	*/
	// Load backgrounds
	screen.loadBackground("nwc1.raw");
	screen.loadPage(0);
	screen.loadBackground("nwc2.raw");
	screen.loadPage(1);

	// Draw the screen and fade it in
	screen.horizMerge(0);
	screen.draw();
	screen.fadeIn(4);

	// Initial loop for dragon roaring
	int nwcIndex = 0, nwcFrame = 0;
	for (int idx = 0; idx < 55 && !vm.shouldQuit(); ++idx) {
		// Render the next frame
		events.updateGameCounter();
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

		if (events.wait(2, true))
			return;
	}
	if (vm.shouldQuit())
		return;

	// Loop for dragon using flyspray
	for (int idx = 0; idx < 42 && !vm.shouldQuit(); ++idx) {
		events.updateGameCounter();
		screen.vertMerge(SCREEN_HEIGHT);
		nwc[3].draw(screen, idx);
		screen.draw();

		switch (idx) {
		case 3:
			sound.startMusic(40);
			break;
		case 11:
			sound.startMusic(0);
		case 27:
		case 30:
			sound.startMusic(3);
			break;
		case 31:
			sound.proc2(voc[2]);
			break;
		case 33:
			sound.startMusic(2);
			break;
		default:
			break;
		}

		if (events.wait(2, true))
			return;
	}

	// Pause for a bit
	if (events.wait(10, true))
		return;
	if (vm.shouldQuit())
		return;

	voc[0].stop();
	voc[1].stop();
	voc[2].stop();


	screen.fadeOut(8);
	//TODO: Stuff

	screen.loadBackground("jvc.raw");
	screen.draw();
	screen.fadeIn(4);

	events.updateGameCounter();
	events.wait(60, true);
}

void showDarkSideIntro(XeenEngine &vm) {
	EventsManager &events = *vm._events;
	Screen &screen = *vm._screen;
	SoundManager &sound = *vm._sound;

	screen.fadeOut(8);
	screen.loadBackground("pyramid2.raw");
	screen.loadPage(0);
	screen.loadPage(1);
	screen.loadBackground("pyramid3.raw");
	screen.saveBackground(1);

	SpriteResource sprites[3] = {
		SpriteResource("title.int"), SpriteResource("pyratop.int"), SpriteResource("pyramid.int")
	};
	Common::File voc[2];
	voc[0].open("pharoh1a.voc");
	voc[1].open("pharoh1b.voc");

	screen.vertMerge(SCREEN_HEIGHT);
	screen.loadPage(0);
	screen.loadPage(1);

	int yp = 0;
	int frame = 0;
	int idx1 = 0;
	bool skipElapsed = false;
	uint32 timeExpired = 0;
	//	bool fadeFlag = true;

	for (int idx = 200; idx > 0; ) {
		events.updateGameCounter();
		screen.vertMerge(yp);

		sprites[0].draw(screen, 0);
		if (frame)
			sprites[0].draw(screen, frame);

		idx1 = (idx1 + 1) % 4;
		if (!idx1)
			frame = (frame + 1) % 10;

		screen.draw();
		if (!skipElapsed) {
			timeExpired = MAX(events.timeElapsed(), (uint32)1);
			skipElapsed = true;
		}

		idx -= timeExpired;
		frame = MIN(frame + timeExpired, (uint)200);
		 
		while (events.timeElapsed() < 1) {
			events.pollEventsAndWait();
			if (events.isKeyMousePressed())
				return;
		}
	}

	screen.vertMerge(SCREEN_HEIGHT);
	screen.saveBackground(1);
	screen.draw();
	screen.freePages();

	events.updateGameCounter();
	events.wait(30, true);

	// TODO: More
	sound.playSong(voc[0]);
	sound.playSong(voc[1]);
}

} // End of namespace Xeen
