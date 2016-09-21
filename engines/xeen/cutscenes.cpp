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

#include "xeen/cutscenes.h"
#include "xeen/xeen.h"

namespace Xeen {

static const char *SUBTITLE_LINE = "\xC" "35\x3" "c\xB" "190\x9" "000%s";

void Cutscenes::resetSubtitles(uint lineNum, uint defaultSize) {
	_subtitleLineNum = lineNum;
	_subtitleSize = defaultSize;
	recordTime();
}

void Cutscenes::showSubtitles(uint windowIndex) {
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;

	if (sound._soundOn || _vm->shouldQuit()) {
		// Sound is on, so subtitles aren't needed
		resetSubtitles(0, 0);
	} else {
		if (timeElapsed() > 1) {
			++_subtitleSize;
			const Common::String &line = _subtitles[_subtitleLineNum];
			Common::String lineStr(line.c_str(), line.c_str() + _subtitleSize);
			_subtitleLine = Common::String::format(SUBTITLE_LINE, lineStr.c_str());

			// If displayed a full line, then move to the next line
			if (_subtitleSize == line.size()) {
				_subtitleSize = 0;
				if (++_subtitleLineNum == _subtitles.size())
					_subtitleLineNum = 0;
			}
		}

		// Draw the box sprite
		if (!_boxSprites)
			// Not already loaded, so load it
			_boxSprites = new SpriteResource("box.vga");
		_boxSprites->draw(screen, 0, Common::Point(36, 189));

		// Write the subtitle line
		screen._windows[windowIndex].writeString(_subtitleLine);
	}

	screen.update();
}

void Cutscenes::freeSubtitles() {
	delete _boxSprites;
	_boxSprites = nullptr;
	_subtitles.clear();
}

bool Cutscenes::subtitlesWait(uint minTime) {
	EventsManager &events = *_vm->_events;

	events.updateGameCounter();
	recordTime();
	while (events.timeElapsed() < minTime || _subtitleSize != 0) {
		events.pollEventsAndWait();
		if (events.isKeyMousePressed())
			return false;

		showSubtitles();
	}

	return true;
}

void Cutscenes::recordTime() {
	_vm->_events->timeMark1();
}

uint Cutscenes::timeElapsed() {
	return _vm->_events->timeElapsed1();
}

uint Cutscenes::getSpeakingFrame(uint minFrame, uint maxFrame) {
	uint interval = g_system->getMillis() / 100;
	return minFrame + interval % (maxFrame + 1 - minFrame);
}

void Cutscenes::doScroll(bool drawFlag, bool doFade) {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;

	if (_vm->getGameID() != GType_Clouds) {
		if (doFade) {
			screen.fadeIn(2);
		}
		return;
	}

	const int SCROLL_L[8] = { 29, 23, 15, 251, 245, 233, 207, 185 };
	const int SCROLL_R[8] = { 165, 171, 198, 218, 228, 245, 264, 281 };

	screen.saveBackground();

	// Load hand vga files
	SpriteResource *hand[16];
	for (int i = 0; i < 16; ++i) {
		Common::String name = Common::String::format("hand%02d.vga", i);
		hand[i] = new SpriteResource(name);
	}

	// Load marb vga files
	SpriteResource *marb[5];
	for (int i = 1; i < 5; ++i) {
		Common::String name = Common::String::format("marb%02d.vga", i);
		marb[i] = new SpriteResource(name);
	}

	if (drawFlag) {
		for (int i = 22; i > 0; --i) {
			events.updateGameCounter();
			screen.restoreBackground();

			if (i > 0 && i <= 14) {
				hand[i - 1]->draw(screen, 0);
			} else {
				hand[14]->draw(screen, 0, Common::Point(SCROLL_L[i - 14], 0), SPRFLAG_800);
				marb[15]->draw(screen, 0, Common::Point(SCROLL_R[i - 14], 0), SPRFLAG_800);
			}

			if (i <= 20) {
				marb[i / 5]->draw(screen, i % 5);
			}

			while (!_vm->shouldQuit() && events.timeElapsed() == 0)
				events.pollEventsAndWait();

			screen._windows[0].update();
			if (i == 0 && doFade)
				screen.fadeIn(2);
		}
	} else {
		for (int i = 0; i < 22 && !events.isKeyMousePressed(); ++i) {
			events.updateGameCounter();
			screen.restoreBackground();

			if (i < 14) {
				hand[i]->draw(screen, 0);
			} else {
				hand[14]->draw(screen, 0, Common::Point(SCROLL_L[i - 7], 0), SPRFLAG_800);
				marb[15]->draw(screen, 0, Common::Point(SCROLL_R[i - 7], 0), SPRFLAG_800);
			}

			if (i < 20) {
				marb[i / 5]->draw(screen, i % 5);
			}

			while (!_vm->shouldQuit() && events.timeElapsed() == 0)
				events.pollEventsAndWait();

			screen._windows[0].update();
			if (i == 0 && doFade)
				screen.fadeIn(2);
		}
	}

	if (drawFlag) {
		hand[0]->draw(screen, 0);
		marb[0]->draw(screen, 0);
	} else {
		screen.restoreBackground();
	}

	screen._windows[0].update();

	// Free resources
	for (int i = 1; i < 5; ++i)
		delete marb[i];
	for (int i = 0; i < 16; ++i)
		delete hand[i];
}

} // End of namespace Xeen
