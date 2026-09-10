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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/util.h"
#include "ultima/ultima1/views/interactions/win.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

constexpr int TEXT_COLOR = 15;   // white
constexpr int BORDER_COLOR = 1;  // dark blue

// The small "Mondain is dead!" dialog reuses the same generic drawDialog
// box geometry as Pillar's inscription dialog - both come from the
// original's own shared drawDialog function
constexpr int DIALOG_X1 = 31, DIALOG_Y1 = 23, DIALOG_X2 = 287, DIALOG_Y2 = 127;
constexpr int BORDER_X1 = 34, BORDER_Y1 = 26, BORDER_X2 = 284, BORDER_Y2 = 124;

// The original's ending image decodes to exactly 320x168 pixels (21 rows
// of an 8px-tall font) - matching the 21 lines of text below exactly
constexpr int ENDING_WIDTH = 320, ENDING_HEIGHT = 168;

// FRAME_RATE ticks/second (see shared/engine/events.h) - about 10 seconds
// to fully reveal, matching the original's scanline-by-scanline draw
constexpr int REVEAL_TICKS = FRAME_RATE * 10;

// The full text of the original's "nif.bin" ending picture, transcribed
// from the decoded bitmap - 21 lines, matching its exact line breaks
static const char *const ENDING_TEXT[] = {
	"A rain of silver lightning heralds the",
	"death of Mondain.  Fleeting glimpses of",
	"fates avoided rush through thy mind as",
	"the arcane power of the mage's dying",
	"scream echoes in thy ears.  A thousand",
	"years pass in but a moment's time as a",
	"strange sleep overcomes thee.",
	"",
	"Upon awakening thou dost find thyself",
	"in new surroundings.  A stately youth in",
	"violet robes helps thee to thy feet",
	"whereupon thou dost see the thousands",
	"who gaze upon thee in adoration.",
	"",
	"\"Thy selfless heroism hath saved our",
	"people, my worthy one.  Should our",
	"gratitude alone not be enough to sustain",
	"thee, know that I, Lord British, hereby",
	"ordain that the entire realm of Sosaria",
	"be at thy service for all time hence-",
	"forth.  So let it be done.\""
};

bool Win::msgFocus(const FocusMessage &msg) {
	setBounds(TextRect(0, 0, 39, 19));
	_phase = PHASE_DIALOG;
	_revealedRows = 0;
	_elapsedTicks = 0;

	renderEndingText();

	for (int i = 0; i < 3; ++i)
		g_engine->playFX(5);

	return true;
}

void Win::renderEndingText() {
	_textSurface.create(ENDING_WIDTH, ENDING_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	_textSurface.setColor(TEXT_COLOR);

	for (int i = 0; i < ARRAYSIZE(ENDING_TEXT); ++i)
		_textSurface.writeString(Common::Point(0, i), ENDING_TEXT[i]);
}

void Win::draw() {
	if (_phase == PHASE_DIALOG) {
		drawDialog();
		return;
	}

	auto s = getSurface();
	s.fillRect(Rect(0, 0, 320, 200), 0);
	s.blitFrom(_textSurface, Common::Rect(0, 0, ENDING_WIDTH, _revealedRows), Common::Point(0, 0));
}

void Win::drawDialog() {
	auto s = getSurface();
	s.fillRect(Rect(DIALOG_X1, DIALOG_Y1, DIALOG_X2, DIALOG_Y2), 0);
	s.frameRect(Rect(BORDER_X1, BORDER_Y1, BORDER_X2, BORDER_Y2), BORDER_COLOR);

	s.setColor(TEXT_COLOR);
	s.writeString(Point(11, 8), "THOU ART VICTORIOUS!");
}

bool Win::tick() {
	if (_phase == PHASE_GLYPH && _revealedRows < ENDING_HEIGHT) {
		++_elapsedTicks;

		int rows = MIN(ENDING_HEIGHT, ENDING_HEIGHT * _elapsedTicks / REVEAL_TICKS);
		if (rows != _revealedRows) {
			_revealedRows = rows;
			redraw();
		}
	}

	return Interaction::tick();
}

bool Win::msgAction(const ActionMessage &msg) {
	advancePhase();
	return true;
}

bool Win::msgKeypress(const KeypressMessage &msg) {
	advancePhase();
	return true;
}

void Win::advancePhase() {
	if (_phase == PHASE_DIALOG) {
		// First key wipes the announcement dialog and starts the ending
		// text revealing itself from the top down
		_phase = PHASE_GLYPH;
		setBounds(Common::Rect(0, 0, 320, 200));
		redraw();
		return;
	}

	// A key during or after the reveal takes the player on to the main menu
	g_engine->replaceView("MainMenu", true);
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima
