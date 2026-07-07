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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/config-manager.h"
#include "common/scummsys.h"
#include "mm/xeen/subtitles.h"
#include "mm/xeen/events.h"
#include "mm/xeen/files.h"
#include "mm/xeen/screen.h"
#include "mm/xeen/xeen.h"

namespace MM {
namespace Xeen {

static const char *SUBTITLE_LINE = "\f35\x3""c\v190\t000%s";

// Bounds of the subtitle box (box.vga frame 0, an opaque 255x11 panel)
static constexpr Common::Rect SUBTITLE_BOX(Common::Point(36, 189), 255, 11);

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
			const char *lineSep;

			if (Common::RU_RUS == g_vm->getLanguage()) {
				lineSep = strstr(line.c_str(), ".");
				if (!lineSep) break;
				_lines.push_back(Common::String(line.c_str(), lineSep + 1) + "   ");
				line = Common::String(lineSep + 1);
			} else {
				lineSep = strstr(line.c_str(), "   ");
				if (!lineSep) break;
				_lines.push_back(Common::String(line.c_str(), lineSep));
				line = Common::String(lineSep + 3);
			}
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

#ifdef USE_TTS
	// Only voice subtitles if there's no voice
	if ((ConfMan.hasKey("subtitles") && ConfMan.getBool("subtitles")) || ConfMan.getInt("speech_volume") == 0) {
		g_vm->sayText(_lines[_lineNum], Common::TextToSpeechManager::QUEUE_NO_REPEAT);
	}
#endif
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
		Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
		bool speaking = sound.isSoundPlaying() || (ttsMan && ttsMan->isSpeaking());

		if (timeElapsed() && (_lineEnd != _lineSize || !speaking)) {
			// Advance the reveal. The original reveals one character per
			// interval, which was tuned for the English lines; localized
			// lines can be much longer, so scale the step with the length
			// to keep long lines from lagging behind the voice
			int step = 1 + _lineSize / 100;
			_lineEnd = (_lineEnd == _lineSize) ? 0 : MIN(_lineEnd + step, _lineSize);
			int count = MAX(_lineEnd - 40, 0);

			// Get the portion of the line to display
			char buffer[1000];
			strncpy(buffer, _lines[_lineNum].c_str() + count, _lineEnd - count);
			buffer[_lineEnd - count] = '\0';

			// The character-based cap above doesn't bound the pixel width of
			// the variable-width font, so drop further leading characters
			// until the text fits within the subtitle box, whose opaque
			// interior then erases the previous text on each redraw. The
			// text is centered on the screen, so it stays inside the box as
			// long as it doesn't reach the box's left edge
			const char *text = windows[0].fitToWidth(buffer,
				2 * (SCREEN_WIDTH / 2 - SUBTITLE_BOX.left));

			// Form the display line
			_displayLine = Common::String::format(SUBTITLE_LINE, text);
			markTime();
		}

		// Draw the box sprite
		if (!_boxSprites)
			// Not already loaded, so load it
			_boxSprites = new SpriteResource("box.vga");
		_boxSprites->draw(0, 0, Common::Point(SUBTITLE_BOX.left, SUBTITLE_BOX.top));

		// Write the subtitle line
		windows[0].writeString(_displayLine, false);

		// The subtitles are redrawn more often than the scenes showing them
		// redraw the screen, so always push the whole box out, so that a
		// partially drawn line never lingers over a more recent one
		windows[0].addDirtyRect(SUBTITLE_BOX);

		if (_lineEnd == 0 && !speaking)
			reset();
	}
}

} // End of namespace Xeen
} // End of namespace MM
