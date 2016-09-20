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
	// TODO
	EventsManager &events = *_vm->_events;
	Sound &sound = *_vm->_sound;
	sound.playSong("mm4theme.m");

	events.wait(5000);
	return true;
}

bool CloudsCutscenes::showCloudsEnding() {
	// TODO
	return true;
}

} // End of namespace Xeen
