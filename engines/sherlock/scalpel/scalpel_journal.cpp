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

#include "sherlock/journal.h"
#include "sherlock/sherlock.h"
#include "sherlock/scalpel/scalpel_fixed_text.h"
#include "sherlock/scalpel/scalpel_journal.h"
#include "sherlock/scalpel/scalpel_screen.h"
#include "sherlock/scalpel/scalpel.h"
#include "sherlock/tattoo/tattoo_journal.h"

namespace Sherlock {

namespace Scalpel {

#define JOURNAL_BUTTONS_Y 178
#define JOURNAL_SEARCH_LEFT 15
#define JOURNAL_SEARCH_TOP 186
#define JOURNAL_SEARCH_RIGHT 296
#define JOURNAL_SEACRH_MAX_CHARS 50

// Positioning of buttons in the journal view
static const int JOURNAL_POINTS[9][3] = {
	{ 6, 68, 37 },
	{ 69, 131, 100 },
	{ 132, 192, 162 },
	{ 193, 250, 221 },
	{ 251, 313, 281 },
	{ 6, 82, 44 },
	{ 83, 159, 121 },
	{ 160, 236, 198 },
	{ 237, 313, 275 }
};

static const int SEARCH_POINTS[3][3] = {
	{ 51, 123, 86 },
	{ 124, 196, 159 },
	{ 197, 269, 232 }
};

/*----------------------------------------------------------------*/

ScalpelJournal::ScalpelJournal(SherlockEngine *vm) : Journal(vm) {
	if (_vm->_interactiveFl) {
		// Load the journal directory and location names
		loadLocations();
	}
}

void ScalpelJournal::loadLocations() {
	Resources &res = *_vm->_res;

	_directory.clear();
	_locations.clear();


	Common::SeekableReadStream *dir = res.load("talk.lib");
	dir->skip(4);		// Skip header

	// Get the numer of entries
	_directory.resize(dir->readUint16LE());

	// Read in each entry
	char buffer[17];
	for (uint idx = 0; idx < _directory.size(); ++idx) {
		dir->read(buffer, 17);
		buffer[16] = '\0';

		_directory[idx] = Common::String(buffer);
	}

	delete dir;

	if (IS_3DO) {
		// 3DO: storage of locations is currently unknown TODO
		return;
	}

	// Load in the locations stored in journal.txt
	Common::SeekableReadStream *loc = res.load("journal.txt");

	while (loc->pos() < loc->size()) {
		Common::String line;
		char c;
		while ((c = loc->readByte()) != 0)
			line += c;

		_locations.push_back(line);
	}

	delete loc;
}

void ScalpelJournal::drawFrame() {
	FixedText &fixedText = *_vm->_fixedText;
	Resources &res = *_vm->_res;
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;
	byte palette[PALETTE_SIZE];

	// Load in the journal background
	Common::SeekableReadStream *bg = res.load("journal.lbv");
	bg->read(screen._backBuffer1.getPixels(), SHERLOCK_SCREEN_WIDTH * SHERLOCK_SCREEN_HEIGHT);
	bg->read(palette, PALETTE_SIZE);
	delete bg;

	// Translate the palette for display
	for (int idx = 0; idx < PALETTE_SIZE; ++idx)
		palette[idx] = VGA_COLOR_TRANS(palette[idx]);

	Common::String fixedText_WatsonsJournal = fixedText.getText(kFixedText_Journal_WatsonsJournal);
	Common::String fixedText_Exit           = fixedText.getText(kFixedText_Journal_Exit);
	Common::String fixedText_Back10         = fixedText.getText(kFixedText_Journal_Back10);
	Common::String fixedText_Up             = fixedText.getText(kFixedText_Journal_Up);
	Common::String fixedText_Down           = fixedText.getText(kFixedText_Journal_Down);
	Common::String fixedText_Ahead10        = fixedText.getText(kFixedText_Journal_Ahead10);
	Common::String fixedText_Search         = fixedText.getText(kFixedText_Journal_Search);
	Common::String fixedText_FirstPage      = fixedText.getText(kFixedText_Journal_FirstPage);
	Common::String fixedText_LastPage       = fixedText.getText(kFixedText_Journal_LastPage);
	Common::String fixedText_PrintText      = fixedText.getText(kFixedText_Journal_PrintText);

	// Set the palette and print the title
	screen.setPalette(palette);
	screen.gPrint(Common::Point(111, 18), BUTTON_BOTTOM, "%s", fixedText_WatsonsJournal.c_str());
	screen.gPrint(Common::Point(110, 17), INV_FOREGROUND, "%s", fixedText_WatsonsJournal.c_str());

	// Draw the buttons
	screen.makeButton(Common::Rect(JOURNAL_POINTS[0][0], JOURNAL_BUTTONS_Y,
		JOURNAL_POINTS[0][1], JOURNAL_BUTTONS_Y + 10),
		JOURNAL_POINTS[0][2] - screen.stringWidth(fixedText_Exit) / 2, fixedText_Exit);
	screen.makeButton(Common::Rect(JOURNAL_POINTS[1][0], JOURNAL_BUTTONS_Y,
		JOURNAL_POINTS[1][1], JOURNAL_BUTTONS_Y + 10),
		JOURNAL_POINTS[1][2] - screen.stringWidth(fixedText_Back10) / 2, fixedText_Back10);
	screen.makeButton(Common::Rect(JOURNAL_POINTS[2][0], JOURNAL_BUTTONS_Y,
		JOURNAL_POINTS[2][1], JOURNAL_BUTTONS_Y + 10),
		JOURNAL_POINTS[2][2] - screen.stringWidth(fixedText_Up) / 2, fixedText_Up);
	screen.makeButton(Common::Rect(JOURNAL_POINTS[3][0], JOURNAL_BUTTONS_Y,
		JOURNAL_POINTS[3][1], JOURNAL_BUTTONS_Y + 10),
		JOURNAL_POINTS[3][2] - screen.stringWidth(fixedText_Down) / 2, fixedText_Down);
	screen.makeButton(Common::Rect(JOURNAL_POINTS[4][0], JOURNAL_BUTTONS_Y,
		JOURNAL_POINTS[4][1], JOURNAL_BUTTONS_Y + 10),
		JOURNAL_POINTS[4][2] - screen.stringWidth(fixedText_Ahead10) / 2, fixedText_Ahead10);
	screen.makeButton(Common::Rect(JOURNAL_POINTS[5][0], JOURNAL_BUTTONS_Y + 11,
		JOURNAL_POINTS[5][1], JOURNAL_BUTTONS_Y + 21),
		JOURNAL_POINTS[5][2] - screen.stringWidth(fixedText_Search) / 2, fixedText_Search);
	screen.makeButton(Common::Rect(JOURNAL_POINTS[6][0], JOURNAL_BUTTONS_Y + 11,
		JOURNAL_POINTS[6][1], JOURNAL_BUTTONS_Y + 21),
		JOURNAL_POINTS[6][2] - screen.stringWidth(fixedText_FirstPage) / 2, fixedText_FirstPage);
	screen.makeButton(Common::Rect(JOURNAL_POINTS[7][0], JOURNAL_BUTTONS_Y + 11,
		JOURNAL_POINTS[7][1], JOURNAL_BUTTONS_Y + 21),
		JOURNAL_POINTS[7][2] - screen.stringWidth(fixedText_LastPage) / 2, fixedText_LastPage);

	// WORKAROUND: Draw Print Text button as disabled, since we don't support it in ScummVM
	screen.makeButton(Common::Rect(JOURNAL_POINTS[8][0], JOURNAL_BUTTONS_Y + 11,
		JOURNAL_POINTS[8][1], JOURNAL_BUTTONS_Y + 21),
		JOURNAL_POINTS[8][2] - screen.stringWidth(fixedText_PrintText) / 2, fixedText_PrintText);
	screen.buttonPrint(Common::Point(JOURNAL_POINTS[8][2], JOURNAL_BUTTONS_Y + 11),
		COMMAND_NULL, false, fixedText_PrintText);
}

void ScalpelJournal::drawInterface() {
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;

	drawFrame();

	if (_journal.empty()) {
		_up = _down = 0;
	} else {
		drawJournal(0, 0);
	}

	doArrows();

	// Show the entire screen
	screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
}

void ScalpelJournal::doArrows() {
	FixedText &fixedText = *_vm->_fixedText;
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;
	byte color;

	Common::String fixedText_Back10         = fixedText.getText(kFixedText_Journal_Back10);
	Common::String fixedText_Up             = fixedText.getText(kFixedText_Journal_Up);
	Common::String fixedText_Down           = fixedText.getText(kFixedText_Journal_Down);
	Common::String fixedText_Ahead10        = fixedText.getText(kFixedText_Journal_Ahead10);
	Common::String fixedText_Search         = fixedText.getText(kFixedText_Journal_Search);
	Common::String fixedText_FirstPage      = fixedText.getText(kFixedText_Journal_FirstPage);
	Common::String fixedText_LastPage       = fixedText.getText(kFixedText_Journal_LastPage);
	Common::String fixedText_PrintText      = fixedText.getText(kFixedText_Journal_PrintText);

	color = (_page > 1) ? COMMAND_FOREGROUND : COMMAND_NULL;
	screen.buttonPrint(Common::Point(JOURNAL_POINTS[1][2], JOURNAL_BUTTONS_Y), color, false, fixedText_Back10);
	screen.buttonPrint(Common::Point(JOURNAL_POINTS[2][2], JOURNAL_BUTTONS_Y), color, false, fixedText_Up);

	color = _down ? COMMAND_FOREGROUND : COMMAND_NULL;
	screen.buttonPrint(Common::Point(JOURNAL_POINTS[3][2], JOURNAL_BUTTONS_Y), color, false, fixedText_Down);
	screen.buttonPrint(Common::Point(JOURNAL_POINTS[4][2], JOURNAL_BUTTONS_Y), color, false, fixedText_Ahead10);
	screen.buttonPrint(Common::Point(JOURNAL_POINTS[7][2], JOURNAL_BUTTONS_Y + 11), color, false, fixedText_LastPage);

	color = _journal.size() > 0 ? COMMAND_FOREGROUND : COMMAND_NULL;
	screen.buttonPrint(Common::Point(JOURNAL_POINTS[5][2], JOURNAL_BUTTONS_Y + 11), color, false, fixedText_Search);
	screen.buttonPrint(Common::Point(JOURNAL_POINTS[8][2], JOURNAL_BUTTONS_Y + 11), COMMAND_NULL, false, fixedText_PrintText);

	color = _page > 1 ? COMMAND_FOREGROUND : COMMAND_NULL;
	screen.buttonPrint(Common::Point(JOURNAL_POINTS[6][2], JOURNAL_BUTTONS_Y + 11), color, false, fixedText_FirstPage);
}

JournalButton ScalpelJournal::getHighlightedButton(const Common::Point &pt) {
	if (pt.x > JOURNAL_POINTS[0][0] && pt.x < JOURNAL_POINTS[0][1] && pt.y >= JOURNAL_BUTTONS_Y &&
			pt.y < (JOURNAL_BUTTONS_Y + 10))
		return BTN_EXIT;

	if (pt.x > JOURNAL_POINTS[1][0] && pt.x < JOURNAL_POINTS[1][1] && pt.y >= JOURNAL_BUTTONS_Y &&
			pt.y < (JOURNAL_BUTTONS_Y + 10) && _page > 1)
		return BTN_BACK10;

	if (pt.x > JOURNAL_POINTS[2][0] && pt.x < JOURNAL_POINTS[2][1] && pt.y >= JOURNAL_BUTTONS_Y &&
			pt.y < (JOURNAL_BUTTONS_Y + 10) && _up)
		return BTN_UP;

	if (pt.x > JOURNAL_POINTS[3][0] && pt.x < JOURNAL_POINTS[3][1] && pt.y >= JOURNAL_BUTTONS_Y &&
			pt.y < (JOURNAL_BUTTONS_Y + 10) && _down)
		return BTN_DOWN;

	if (pt.x > JOURNAL_POINTS[4][0] && pt.x < JOURNAL_POINTS[4][1] && pt.y >= JOURNAL_BUTTONS_Y &&
			pt.y < (JOURNAL_BUTTONS_Y + 10) && _down)
		return BTN_AHEAD110;

	if (pt.x > JOURNAL_POINTS[5][0] && pt.x < JOURNAL_POINTS[5][1] && pt.y >= (JOURNAL_BUTTONS_Y + 11) &&
			pt.y < (JOURNAL_BUTTONS_Y + 20) && !_journal.empty())
		return BTN_SEARCH;

	if (pt.x > JOURNAL_POINTS[6][0] && pt.x < JOURNAL_POINTS[6][1] && pt.y >= (JOURNAL_BUTTONS_Y + 11) &&
			pt.y < (JOURNAL_BUTTONS_Y + 20) && _up)
		return BTN_FIRST_PAGE;

	if (pt.x > JOURNAL_POINTS[7][0] && pt.x < JOURNAL_POINTS[7][1] && pt.y >= (JOURNAL_BUTTONS_Y + 11) &&
			pt.y < (JOURNAL_BUTTONS_Y + 20) && _down)
		return BTN_LAST_PAGE;

	if (pt.x > JOURNAL_POINTS[8][0] && pt.x < JOURNAL_POINTS[8][1] && pt.y >= (JOURNAL_BUTTONS_Y + 11) &&
			pt.y < (JOURNAL_BUTTONS_Y + 20) && !_journal.empty())
		return BTN_PRINT_TEXT;

	return BTN_NONE;
}

bool ScalpelJournal::handleEvents(int key) {
	Events    &events    = *_vm->_events;
	FixedText &fixedText = *_vm->_fixedText;
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;
	bool doneFlag = false;

	Common::Point pt = events.mousePos();
	JournalButton btn = getHighlightedButton(pt);
	byte color;

	if (events._pressed || events._released) {
		Common::String fixedText_Exit           = fixedText.getText(kFixedText_Journal_Exit);
		Common::String fixedText_Back10         = fixedText.getText(kFixedText_Journal_Back10);
		Common::String fixedText_Up             = fixedText.getText(kFixedText_Journal_Up);
		Common::String fixedText_Down           = fixedText.getText(kFixedText_Journal_Down);
		Common::String fixedText_Ahead10        = fixedText.getText(kFixedText_Journal_Ahead10);
		Common::String fixedText_Search         = fixedText.getText(kFixedText_Journal_Search);
		Common::String fixedText_FirstPage      = fixedText.getText(kFixedText_Journal_FirstPage);
		Common::String fixedText_LastPage       = fixedText.getText(kFixedText_Journal_LastPage);
		Common::String fixedText_PrintText      = fixedText.getText(kFixedText_Journal_PrintText);

		// Exit button
		color = (btn == BTN_EXIT) ? COMMAND_HIGHLIGHTED : COMMAND_FOREGROUND;
		screen.buttonPrint(Common::Point(JOURNAL_POINTS[0][2], JOURNAL_BUTTONS_Y), color, true, fixedText_Exit);

		// Back 10 button
		if (btn == BTN_BACK10) {
			screen.buttonPrint(Common::Point(JOURNAL_POINTS[1][2], JOURNAL_BUTTONS_Y), COMMAND_HIGHLIGHTED, true, fixedText_Back10);
		} else if (_page > 1) {
			screen.buttonPrint(Common::Point(JOURNAL_POINTS[1][2], JOURNAL_BUTTONS_Y), COMMAND_FOREGROUND, true, fixedText_Back10);
		}

		// Up button
		if (btn == BTN_UP) {
			screen.buttonPrint(Common::Point(JOURNAL_POINTS[2][2], JOURNAL_BUTTONS_Y), COMMAND_HIGHLIGHTED, true, fixedText_Up);
		} else if (_up) {
			screen.buttonPrint(Common::Point(JOURNAL_POINTS[2][2], JOURNAL_BUTTONS_Y), COMMAND_FOREGROUND, true, fixedText_Up);
		}

		// Down button
		if (btn == BTN_DOWN) {
			screen.buttonPrint(Common::Point(JOURNAL_POINTS[3][2], JOURNAL_BUTTONS_Y), COMMAND_HIGHLIGHTED, true, fixedText_Down);
		} else if (_down) {
			screen.buttonPrint(Common::Point(JOURNAL_POINTS[3][2], JOURNAL_BUTTONS_Y), COMMAND_FOREGROUND, true, fixedText_Down);
		}

		// Ahead 10 button
		if (btn == BTN_AHEAD110) {
			screen.buttonPrint(Common::Point(JOURNAL_POINTS[4][2], JOURNAL_BUTTONS_Y), COMMAND_HIGHLIGHTED, true, fixedText_Ahead10);
		} else if (_down) {
			screen.buttonPrint(Common::Point(JOURNAL_POINTS[4][2], JOURNAL_BUTTONS_Y), COMMAND_FOREGROUND, true, fixedText_Ahead10);
		}

		// Search button
		if (btn == BTN_SEARCH) {
			color = COMMAND_HIGHLIGHTED;
		} else if (_journal.empty()) {
			color = COMMAND_NULL;
		} else {
			color = COMMAND_FOREGROUND;
		}
		screen.buttonPrint(Common::Point(JOURNAL_POINTS[5][2], JOURNAL_BUTTONS_Y + 11), color, true, fixedText_Search);

		// First Page button
		if (btn == BTN_FIRST_PAGE) {
			color = COMMAND_HIGHLIGHTED;
		} else if (_up) {
			color = COMMAND_FOREGROUND;
		} else {
			color = COMMAND_NULL;
		}
		screen.buttonPrint(Common::Point(JOURNAL_POINTS[6][2], JOURNAL_BUTTONS_Y + 11), color, true, fixedText_FirstPage);

		// Last Page button
		if (btn == BTN_LAST_PAGE) {
			color = COMMAND_HIGHLIGHTED;
		} else if (_down) {
			color = COMMAND_FOREGROUND;
		} else {
			color = COMMAND_NULL;
		}
		screen.buttonPrint(Common::Point(JOURNAL_POINTS[7][2], JOURNAL_BUTTONS_Y + 11), color, true, fixedText_LastPage);

		// Print Text button
		screen.buttonPrint(Common::Point(JOURNAL_POINTS[8][2], JOURNAL_BUTTONS_Y + 11), COMMAND_NULL, true, fixedText_PrintText);
	}

	if (btn == BTN_EXIT && events._released) {
		// Exit button pressed
		doneFlag = true;

	} else if (((btn == BTN_BACK10 && events._released) || key == 'B') && (_page > 1)) {
		// Scrolll up 10 pages
		if (_page < 11)
			drawJournal(1, (_page - 1) * LINES_PER_PAGE);
		else
			drawJournal(1, 10 * LINES_PER_PAGE);

		doArrows();
		screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);

	} else if (((btn == BTN_UP && events._released) || key == 'U') && _up) {
		// Scroll up
		drawJournal(1, LINES_PER_PAGE);
		doArrows();
		screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);

	} else if (((btn == BTN_DOWN && events._released) || key == 'D') && _down) {
		// Scroll down
		drawJournal(2, LINES_PER_PAGE);
		doArrows();
		screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);

	} else if (((btn == BTN_AHEAD110 && events._released) || key == 'A') && _down) {
		// Scroll down 10 pages
		if ((_page + 10) > _maxPage)
			drawJournal(2, (_maxPage - _page) * LINES_PER_PAGE);
		else
			drawJournal(2, 10 * LINES_PER_PAGE);

		doArrows();
		screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);

	} else if (((btn == BTN_SEARCH && events._released) || key == 'S') && !_journal.empty()) {
		screen.buttonPrint(Common::Point(JOURNAL_POINTS[5][2], JOURNAL_BUTTONS_Y + 11), COMMAND_FOREGROUND, true, "Search");
		bool notFound = false;

		do {
			int dir;
			if ((dir = getSearchString(notFound)) != 0) {
				int savedIndex = _index;
				int savedSub = _sub;
				int savedPage = _page;

				if (drawJournal(dir + 2, 1000 * LINES_PER_PAGE) == 0) {
					_index = savedIndex;
					_sub = savedSub;
					_page = savedPage;

					drawFrame();
					drawJournal(0, 0);
					notFound = true;
				} else {
					doneFlag = true;
				}

				doArrows();
				screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
			} else {
				doneFlag = true;
			}
		} while (!doneFlag);
		doneFlag = false;

	} else if (((btn == BTN_FIRST_PAGE && events._released) || key == 'F') && _up) {
		// First page
		_index = _sub = 0;
		_up = _down = false;
		_page = 1;

		drawFrame();
		drawJournal(0, 0);
		doArrows();
		screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);

	} else if (((btn == BTN_LAST_PAGE && events._released) || key == 'L') && _down) {
		// Last page
		if ((_page + 10) > _maxPage)
			drawJournal(2, (_maxPage - _page) * LINES_PER_PAGE);
		else
			drawJournal(2, 1000 * LINES_PER_PAGE);

		doArrows();
		screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
	}

	events.wait(2);

	return doneFlag;
}

int ScalpelJournal::getSearchString(bool printError) {
	enum Button { BTN_NONE, BTN_EXIT, BTN_BACKWARD, BTN_FORWARD };

	Events    &events    = *_vm->_events;
	FixedText &fixedText = *_vm->_fixedText;
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;
	Talk &talk = *_vm->_talk;
	int xp;
	int yp = 174;
	bool flag = false;
	Common::String name;
	int done = 0;
	byte color;

	Common::String fixedText_Exit           = fixedText.getText(kFixedText_JournalSearch_Exit);
	Common::String fixedText_Backward       = fixedText.getText(kFixedText_JournalSearch_Backward);
	Common::String fixedText_Forward        = fixedText.getText(kFixedText_JournalSearch_Forward);
	Common::String fixedText_NotFound       = fixedText.getText(kFixedText_JournalSearch_NotFound);

	// Draw search panel
	screen.makePanel(Common::Rect(6, 171, 313, 199));
	screen.makeButton(Common::Rect(SEARCH_POINTS[0][0], yp, SEARCH_POINTS[0][1], yp + 10),
		SEARCH_POINTS[0][2] - screen.stringWidth(fixedText_Exit) / 2, fixedText_Exit);
	screen.makeButton(Common::Rect(SEARCH_POINTS[1][0], yp, SEARCH_POINTS[1][1], yp + 10),
		SEARCH_POINTS[1][2] - screen.stringWidth(fixedText_Backward) / 2, fixedText_Backward);
	screen.makeButton(Common::Rect(SEARCH_POINTS[2][0], yp, SEARCH_POINTS[2][1], yp + 10),
		SEARCH_POINTS[2][2] - screen.stringWidth(fixedText_Forward) / 2, fixedText_Forward);

	screen.gPrint(Common::Point(SEARCH_POINTS[0][2] - screen.stringWidth(fixedText_Exit) / 2, yp),
		COMMAND_HIGHLIGHTED, "%c", fixedText_Exit[0]);
	screen.gPrint(Common::Point(SEARCH_POINTS[1][2] - screen.stringWidth(fixedText_Backward) / 2, yp),
		COMMAND_HIGHLIGHTED, "%c", fixedText_Backward[0]);
	screen.gPrint(Common::Point(SEARCH_POINTS[2][2] - screen.stringWidth(fixedText_Forward) / 2, yp),
		COMMAND_HIGHLIGHTED, "%c", fixedText_Forward[0]);

	screen.makeField(Common::Rect(12, 185, 307, 196));

	screen.fillRect(Common::Rect(12, 185, 307, 186), BUTTON_BOTTOM);
	screen.vLine(12, 185, 195, BUTTON_BOTTOM);
	screen.hLine(13, 195, 306, BUTTON_TOP);
	screen.hLine(306, 186, 195, BUTTON_TOP);

	if (printError) {
		screen.gPrint(Common::Point((SHERLOCK_SCREEN_WIDTH - screen.stringWidth(fixedText_NotFound)) / 2, 185),
			INV_FOREGROUND, "%s", fixedText_NotFound.c_str());
	} else if (!_find.empty()) {
		// There's already a search term, display it already
		screen.gPrint(Common::Point(15, 185), TALK_FOREGROUND, "%s", _find.c_str());
		name = _find;
	}

	screen.slamArea(6, 171, 307, 28);

	if (printError) {
		// Give time for user to see the message
		events.setButtonState();
		for (int idx = 0; idx < 40 && !_vm->shouldQuit() && !events.kbHit() && !events._released; ++idx) {
			events.pollEvents();
			events.setButtonState();
			events.wait(2);
		}

		events.clearKeyboard();
		screen._backBuffer1.fillRect(Common::Rect(13, 186, 306, 195), BUTTON_MIDDLE);

		if (!_find.empty()) {
			screen.gPrint(Common::Point(15, 185), TALK_FOREGROUND, "%s", _find.c_str());
			name = _find;
		}

		screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
	}

	xp = JOURNAL_SEARCH_LEFT + screen.stringWidth(name);
	yp = JOURNAL_SEARCH_TOP;

	do {
		events._released = false;
		Button found = BTN_NONE;

		while (!_vm->shouldQuit() && !events.kbHit() && !events._released) {
			found = BTN_NONE;
			if (talk._talkToAbort)
				return 0;

			// Check if key or mouse button press has occurred
			events.setButtonState();
			Common::Point pt = events.mousePos();

			flag = !flag;
			screen.vgaBar(Common::Rect(xp, yp, xp + 8, yp + 9), flag ? INV_FOREGROUND : BUTTON_MIDDLE);

			if (events._pressed || events._released) {
				if (pt.x > SEARCH_POINTS[0][0] && pt.x < SEARCH_POINTS[0][1] && pt.y > 174 && pt.y < 183) {
					found = BTN_EXIT;
					color = COMMAND_HIGHLIGHTED;
				} else {
					color = COMMAND_FOREGROUND;
				}
				screen.print(Common::Point(SEARCH_POINTS[0][2] - screen.stringWidth(fixedText_Exit) / 2, 175), color, "%s", fixedText_Exit.c_str());

				if (pt.x > SEARCH_POINTS[1][0] && pt.x < SEARCH_POINTS[1][1] && pt.y > 174 && pt.y < 183) {
					found = BTN_BACKWARD;
					color = COMMAND_HIGHLIGHTED;
				} else {
					color = COMMAND_FOREGROUND;
				}
				screen.print(Common::Point(SEARCH_POINTS[1][2] - screen.stringWidth(fixedText_Backward) / 2, 175), color, "%s", fixedText_Backward.c_str());

				if (pt.x > SEARCH_POINTS[2][0] && pt.x < SEARCH_POINTS[2][1] && pt.y > 174 && pt.y < 183) {
					found = BTN_FORWARD;
					color = COMMAND_HIGHLIGHTED;
				} else {
					color = COMMAND_FOREGROUND;
				}
				screen.print(Common::Point(SEARCH_POINTS[2][2] - screen.stringWidth(fixedText_Forward) / 2, 175), color, "%s", fixedText_Forward.c_str());
			}

			events.wait(2);
		}

		if (events.kbHit()) {
			Common::KeyState keyState = events.getKey();

			if ((keyState.keycode == Common::KEYCODE_BACKSPACE) && (name.size() > 0)) {
				screen.vgaBar(Common::Rect(xp - screen.charWidth(name.lastChar()), yp, xp + 8, yp + 9), BUTTON_MIDDLE);
				xp -= screen.charWidth(name.lastChar());
				screen.vgaBar(Common::Rect(xp, yp, xp + 8, yp + 9), INV_FOREGROUND);
				name.deleteLastChar();

			} else  if (keyState.keycode == Common::KEYCODE_RETURN) {
				done = 1;

			}  else if (keyState.keycode == Common::KEYCODE_ESCAPE) {
				screen.vgaBar(Common::Rect(xp, yp, xp + 8, yp + 9), BUTTON_MIDDLE);
				done = -1;

			} else if (keyState.ascii >= ' ' && keyState.ascii <= 'z' && keyState.keycode != Common::KEYCODE_AT &&
				name.size() < JOURNAL_SEACRH_MAX_CHARS && (xp + screen.charWidth(keyState.ascii)) < JOURNAL_SEARCH_RIGHT) {
				char ch = toupper(keyState.ascii);
				screen.vgaBar(Common::Rect(xp, yp, xp + 8, yp + 9), BUTTON_MIDDLE);
				screen.print(Common::Point(xp, yp), TALK_FOREGROUND, "%c", ch);
				xp += screen.charWidth(ch);
				name += ch;
			}
		}

		if (events._released) {
			switch (found) {
			case BTN_EXIT:
				done = -1; break;
			case BTN_BACKWARD:
				done = 2; break;
			case BTN_FORWARD:
				done = 1; break;
			default:
				break;
			}
		}
	} while (!done && !_vm->shouldQuit());

	if (done != -1) {
		_find = name;
	} else {
		done = 0;
	}

	// Redisplay the journal screen
	drawFrame();
	drawJournal(0, 0);
	screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);

	return done;
}

void ScalpelJournal::resetPosition() {
	_index = _sub = _up = _down = 0;
	_page = 1;
}

} // End of namespace Scalpel

} // End of namespace Sherlock
