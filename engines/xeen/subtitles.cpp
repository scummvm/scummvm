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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "xeen/subtitles.h"
#include "xeen/events.h"
#include "xeen/files.h"
#include "xeen/xeen.h"

namespace Xeen {

static const char *SUBTITLE_LINE = "\f35\x3""c\v190\t000%s";

Subtitles::Subtitles() : _lineNum(-1), _boxSprites(nullptr), _lineEnd(0), _lineSize(0) {
}

Subtitles::~Subtitles() {
	delete _boxSprites;
}

void Subtitles::loadSubtitles() {
	File f("special.bin");

	if (!g_vm->_files->_ccNum) {
		// The first subtitle line contains all the text for the Clouds intro. Since ScummVM allows
		// both voice and subtitles at the same time, unlike the original, we need to split up the
		// first subtitle into separate lines to allow them to better interleave with the voice
		Common::String line = f.readString();
		for (;;) {
			const char *lineSep = strstr(line.c_str(), "   ");
			if (!lineSep)
				break;

			_lines.push_back(Common::String(line.c_str(), lineSep));
			line = Common::String(lineSep + 3);
			while (line.hasPrefix(" "))
				line.deleteChar(0);
		}
	}

	while (f.pos() < f.size())
		_lines.push_back(f.readString());
	f.close();
}

void Subtitles::reset() {
	_lineNum = -1;
}

void Subtitles::markTime() {
	g_vm->_events->timeMark3();
}

bool Subtitles::timeElapsed() const {
	return g_vm->_events->timeElapsed3() >= 2;
}

void Subtitles::setLine(int line) {
	if (_lines.empty())
		loadSubtitles();

	markTime();
	_lineNum = line;
	_lineSize = _lines[_lineNum].size();
	_lineEnd = 1;
	_displayLine.clear();
}

bool Subtitles::active() const {
	return !g_vm->shouldExit() && _lineNum != -1;
}

bool Subtitles::lineActive() const {
	return !g_vm->shouldExit() && (active() || g_vm->_sound->isSoundPlaying());
}

bool Subtitles::wait(uint numFrames, bool interruptable) {
	EventsManager &events = *g_vm->_events;
	bool result = g_vm->shouldExit();

	events.updateGameCounter();
	while (!g_vm->shouldExit() && events.timeElapsed() < numFrames && !result) {
		show();
		events.pollEventsAndWait();
		result = events.isKeyMousePressed();
	}

	events.clearEvents();
	return result;
}

bool Subtitles::waitForLineOrSound() {
	while (g_vm->_sound->isSoundPlaying() || active()) {
		show();
		g_vm->_events->pollEventsAndWait();
		if (g_vm->_events->isKeyMousePressed() || g_vm->shouldExit())
			return false;
	}

	return true;
}

void Subtitles::show() {
	Sound &sound = *g_vm->_sound;
	Windows &windows = *g_vm->_windows;

	if (!sound._subtitles || !active() || g_vm->shouldExit()) {
		// Subtitles aren't needed
		reset();
	} else {
		if (timeElapsed()) {
			_lineEnd = (_lineEnd + 1) % _lineSize;
			int count = MAX(_lineEnd - 40, 0);

			// Get the portion of the line to display
			char buffer[1000];
			strncpy(buffer, _lines[_lineNum].c_str() + count, _lineEnd - count);
			buffer[_lineEnd - count] = '\0';

			// Form the display line
			_displayLine = Common::String::format(SUBTITLE_LINE, buffer);
			markTime();
		}

		// Draw the box sprite
		if (!_boxSprites)
			// Not already loaded, so load it
			_boxSprites = new SpriteResource("box.vga");
		_boxSprites->draw(0, 0, Common::Point(36, 189));

		// Write the subtitle line
		windows[0].writeString(_displayLine);

		if (_lineEnd == 0)
			reset();
	}
}

} // End of namespace Xeen
