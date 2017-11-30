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
	Windows &windows = *_vm->_windows;

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
		_boxSprites->draw(0, 0, Common::Point(36, 189));

		// Write the subtitle line
		windows[windowIndex].writeString(_subtitleLine);
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

bool Cutscenes::waitForLineOrSound() {
	while (_vm->_sound->isPlaying() || _subtitleSize) {
		showSubtitles();
		_vm->_events->pollEventsAndWait();
		if (_vm->_events->isKeyMousePressed())
			return false;
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

bool Cutscenes::doScroll(bool rollUp, bool fadeIn) {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;
	const int SCROLL_L[8] = { 29, 23, 15, -5, -11, -23, -49, -71 };
	const int SCROLL_R[8] = { 165, 171, 198, 218, 228, 245, 264, 281 };

	if (_vm->_files->_isDarkCc) {
		if (fadeIn)
			screen.fadeIn(2);
		return _vm->shouldQuit();
	}

	screen.saveBackground();

	// Load hand sprites
	SpriteResource *hand[16];
	for (int i = 0; i < 16; ++i) {
		Common::String name = Common::String::format("hand%02d.vga", i);
		hand[i] = new SpriteResource(name);
	}

	// Load marb sprites
	SpriteResource *marb[5];
	for (int i = 0; i < 4; ++i) {
		Common::String name = Common::String::format("marb%02d.vga", i + 1);
		marb[i] = new SpriteResource(name);
	}

	if (rollUp) {
		for (int i = 22, ctr = 7; i > 0 && !events.isKeyMousePressed()
				&& !_vm->shouldQuit(); --i) {
			events.updateGameCounter();
			screen.restoreBackground();

			if (i > 14) {
				hand[14]->draw(0, 0, Common::Point(SCROLL_L[ctr], 0), SPRFLAG_800);
				hand[15]->draw(0, 0, Common::Point(SCROLL_R[ctr], 0), SPRFLAG_800);
				--ctr;
			} else if (i != 0) {
				hand[i - 1]->draw(0, 0);
			}

			if (i <= 20)
				marb[(i - 1) / 5]->draw(0, (i - 1) % 5);
			screen.update();

			while (!_vm->shouldQuit() && events.timeElapsed() == 0)
				events.pollEventsAndWait();

			if (i == 0 && fadeIn)
				screen.fadeIn(2);
		}
	} else {
		for (int i = 0, ctr = 0; i < 22 && !events.isKeyMousePressed()
				&& !_vm->shouldQuit(); ++i) {
			events.updateGameCounter();
			screen.restoreBackground();

			if (i < 14) {
				hand[i]->draw(0, 0);
			} else {
				hand[14]->draw(0, 0, Common::Point(SCROLL_L[ctr], 0), SPRFLAG_800);
				hand[15]->draw(0, 0, Common::Point(SCROLL_R[ctr], 0), SPRFLAG_800);
				++ctr;
			}

			if (i < 20) {
				marb[i / 5]->draw(0, i % 5);
			}
			screen.update();

			while (!_vm->shouldQuit() && events.timeElapsed() == 0)
				events.pollEventsAndWait();

			if (i == 0 && fadeIn)
				screen.fadeIn(2);
		}
	}

	if (rollUp) {
		hand[0]->draw(0, 0);
		marb[0]->draw(0, 0);
	} else {
		screen.restoreBackground();
	}
	screen.update();

	// Free resources
	for (int i = 0; i < 4; ++i)
		delete marb[i];
	for (int i = 0; i < 16; ++i)
		delete hand[i];

	return _vm->shouldQuit() || events.isKeyMousePressed();
}

} // End of namespace Xeen
