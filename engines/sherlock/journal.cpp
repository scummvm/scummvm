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

#include "sherlock/journal.h"
#include "sherlock/sherlock.h"

namespace Sherlock {

#define JOURNAL_BUTTONS_Y 178
#define LINES_PER_PAGE 11

// Positioning of buttons in the journal view
const int JOURNAL_POINTS[9][3] = { 
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

/*----------------------------------------------------------------*/

Journal::Journal(SherlockEngine *vm): _vm(vm) {
	// Allow up to 1000 statements
	_journal.resize(1000);

	// Initialize fields
	_count = 0;
	_maxPage = 0;
	_index = 0;
	_sub = 0;
	_up = _down = false;
	_page = 0;
	_converseNum = -1;

	// Load the journal directory and location names
	loadJournalLocations();
}

/**
 * Records statements that are said, in the order which they are said. The player
 * can then read the journal to review them
 */
void Journal::record(int converseNum, int statementNum) {
	int saveIndex = _index;
	int saveSub = _sub;

	// Record the entry into the list
	_journal.push_back(JournalEntry(converseNum, statementNum));
	_index = _journal.size() - 1;

	// Load the text for the new entry to get the number of lines it will have
	int newLines = loadJournalFile(true);

	// Restore old state
	_index = saveIndex;
	_sub = saveSub;

	// If new lines were added to the ournal, update the total number of lines
	// the journal continues
	if (newLines) {
		_maxPage += newLines;
	} else {
		// No lines in entry, so remove the new entry from the journal
		_journal.remove_at(_journal.size() - 1);
	}
}

void Journal::loadJournalLocations() {
	Resources &res = *_vm->_res;
	char c;

	_directory.clear();

	Common::SeekableReadStream *dir = res.load("talk.lib");
	dir->skip(4);		// Skip header
	
	// Get the numer of entries
	_directory.resize(dir->readUint16LE());

	// Read in each entry
	for (uint idx = 0; idx < _directory.size(); ++idx) {
		Common::String line;
		while ((c = dir->readByte()) != 0)
			line += c;

		_directory.push_back(line);
	}

	delete dir;

	// Load in the locations stored in journal.txt
	Common::SeekableReadStream *loc = res.load("journal.txt");

	_locations.clear();
	while (loc->pos() < loc->size()) {
		Common::String line;
		while ((c = loc->readByte()) != '\0')
			line += c;

		_locations.push_back(line);
	}

	delete loc;
}

/**
 * Loads the description for the current display index in the journal, and then
 * word wraps the result to prepare it for being displayed
 */
bool Journal::loadJournalFile(bool alreadyLoaded) {
	Inventory &inv = *_vm->_inventory;
	People &people = *_vm->_people;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	JournalEntry &journalEntry = _journal[_index];
	Statement &statement = talk[journalEntry._statementNum];

	Common::String dirFilename = _directory[journalEntry._converseNum];
	bool replyOnly = journalEntry._replyOnly;
	Common::String locStr(dirFilename.c_str(), dirFilename.c_str() + 4);
	int newLocation = atoi(locStr.c_str());

	// If not flagged as alrady loaded, load the conversation into script variables
	if (!alreadyLoaded) {
		// See if the file to be used is already loaded
		if (journalEntry._converseNum != _converseNum) {
			// Nope. Free any previously loaded talk
			talk.freeTalkVars();

			// Find the person being talked to
			talk._talkTo = -1;
			for (int idx = 0; idx < MAX_PEOPLE; ++idx) {
				Common::String portrait = people[idx]._portrait;
				Common::String numStr(portrait.c_str(), portrait.c_str() + 4);

				if (locStr == numStr) {
					talk._talkTo = idx;
					break;
				}
			}

			// Load the talk file
			talk.loadTalkFile(dirFilename);
		}
	}

	if (talk[0]._statement.hasPrefix("*") || talk[0]._statement.hasPrefix("^"))
		replyOnly = true;

	// If this isn't the first journal entry, see if the previous journal entry
	// was in the same scene to see if we need to include the scene header
	int oldLocation = -1;
	if (_index != 0) {
		// Get the scene number of the prior journal entry
		Common::String priorEntry = _directory[_journal[_index - 1]._converseNum];
		oldLocation = atoi(Common::String(priorEntry.c_str() + 4, priorEntry.c_str() + 6).c_str());
	}

	// Start building journal string
	Common::String journalString;

	if (newLocation != oldLocation) {
		// Add in scene title
		journalString = "@" + _locations[newLocation - 1] + ":";

		// See if title can fit into a single line, or requires splitting on 2 lines
		int width = screen.stringWidth(journalString.c_str() + 1);
		if (width > 230) {
			// Scan backwards from end of title to find a space between a word
			// where the width is less than the maximum allowed for the line
			const char *lineP = journalString.c_str() + journalString.size() - 1;
			while (width > 230 || *lineP != ' ')
				width -= screen.charWidth(*lineP--);
			
			// Split the header into two lines, and add a '@' prefix 
			// to the second line as well
			journalString = Common::String(journalString.c_str(), lineP) + "\n@" +
				Common::String(lineP + 1);
		}

		// Add a newline at the end of the title
		journalString += '\n';
	}

	// If Holmes has something to say first, then take care of it
	if (!replyOnly) {
		// Handle the grammar
		journalString += "Holmes ";
		if (talk[journalEntry._statementNum]._statement.hasSuffix("?"))
			journalString += "asked ";
		else
			journalString += "said to ";
	
		switch (talk._talkTo) {
		case 1:
			journalString += "me";
			break;
		case 2:
			journalString += "the Inspector";
			break;
		default:
			journalString += inv._names[talk._talkTo];
			break;
		}
		journalString += ", \"";

		// Add the statement
		journalString += statement._statement;
	}

	// Handle including the reply
	bool startOfReply = true;
	bool ctrlSpace = false;
	bool commentFlag = false;
	bool commentJustPrinted = false;
	const char *replyP = statement._reply.c_str();

	while (*replyP) {
		char c = *replyP;

		// Is it a control character?
		if (c < 128) {
			// Nope. Set flag for allowing control coes to insert spaces
			ctrlSpace = true;

			// Check for embedded comments
			if (c == '{' || c == '}') {
				// Comment characters. If we're starting a comment and there's
				// already text displayed, add a closing quote
				if (c == '{' && !startOfReply && !commentJustPrinted)
					journalString += '"';

				// If a reply isn't just being started, and we didn't just end
				// a comment (which would have added a line), add a carriage return
				if (!startOfReply && ((!commentJustPrinted && c == '{') || c == '}'))
					journalString += '"';
			
				// Handle setting or clearing comment state
				if (c == '{') {
					commentFlag = true;
					commentJustPrinted = false;
				} else {
					commentFlag = false;
					commentJustPrinted = true;
				}
			} else {
				if (startOfReply) {
					if (!replyOnly) {
						journalString += "\"\n";

						if (talk._talkTo == 1)
							journalString += "I replied, \"";
						else
							journalString += "The reply was, \"";
					} else {
						if (talk._talkTo == 1)
							journalString += "I";
						else if (talk._talkTo == 2)
							journalString += "The Inspector";
						else
							journalString += inv._names[talk._talkTo];

						const char *strP = replyP + 1;
						char v;
						do {
							v = *strP++;						
						} while (v && v < 128 && v != '.' && v != '!' && v != '?');

						if (v == '?')
							journalString += " asked, \"";
						else
							journalString += " said, \"";
					}

					startOfReply = false;
				}

				// Copy text from the place until either the reply ends, a comment
				// {} block is started, or a control character is encountered
				do {
					journalString += *replyP++;
				} while (*replyP && *replyP < 128 && *replyP != '{' && *replyP != '}');

				// Move pointer back, since the outer for loop will increment it again
				--replyP;
				commentJustPrinted = false;
			}
		} else if (c == 128) {
			if (!startOfReply) {
				if (!commentFlag && !commentJustPrinted)
					journalString += "\"\n";

				journalString += "Then ";
				commentFlag = false;
			} else if (!replyOnly) {
				journalString += "\"\n";
			}

			startOfReply = false;
			c = *++replyP;

			if ((c - 1) == 0)
				journalString += "Holmes";
			else if ((c - 1) == 1)
				journalString += "I";
			else if ((c - 1) == 2)
				journalString += "the Inspector";
			else
				journalString += inv._names[c - 1];

			const char *strP = replyP + 1;
			char v;
			do {
				v = *strP++;
			} while (v && v < 128 && v != '.' && v != '!' && v != '?');

			if (v == '?')
				journalString += " asked, \"";
			else
				journalString += " said, \"";
		} else {
			// Control code, so move past it and any parameters
			++replyP;
			switch (c) {
			case 129:		// Run canim
			case 130:		// Assign side
			case 131:		// Pause with control
			case 136:		// Pause without control
			case 157:		// Walk to canimation
				// These commands don't have any param
				break;

			case 134:		// Change sequence
				replyP += (replyP[0] & 127) + replyP[2] + 1;
				break;

			case 135:       // Walk to co-ords
			case 154:		// Move mouse
				replyP += 3; 
				break;

			case 139:		// Set flag
			case 143:		// If statement 
				++replyP;
				break;

			case 140:		// Play voice file
			case 150:		// Play prologue
			case 153:		// Call talk file 
				replyP += 7;
				break;

			case 141:		// Toggle object
			case 151:		// Put item in inventory
			case 152:		// Set object
			case 155:		// Info line
			case 158:		// Delete item from inventory
				replyP += *replyP & 127;
				break;

			case 149:		// Goto scene
				replyP += 4;
				break;

			case 161:		// End of line
				journalString += "\n";
				break;

			default:
				break;
			}

			// Put a space in the output for a control character, unless it's
			// immediately coming after another control character
			if (ctrlSpace && c != 130 && c != 161 && !commentJustPrinted) {
				journalString += " ";
				ctrlSpace = false;
			}
		}
	}

	if (!startOfReply && !commentJustPrinted)
		journalString += '"';

	// Finally finished building the journal text. Need to process the text to
	// word wrap it to fit on-screen. The resulting lines are stored in the
	// _lines array
	_lines.clear();

	while (!journalString.empty()) {
		const char *startP = journalString.c_str();

		// If the first character is a '@' flagging a title line, then move
		// past it, so the @ won't be included in the line width calculation
		if (*startP == '@')
			++startP;
	
		// Build up chacters until a full line is found
		int width = 0;
		const char *endP = startP;
		while (width < 230 && *endP && *endP != '\n' && (endP - startP) < 79)
			width += screen.charWidth(*endP++);
		
		// If word wrapping, move back to end of prior word
		if (width >= 230 || (endP - startP) >= 79) {
			while (*--endP != ' ')
				;
		}

		// Add in the line
		_lines.push_back(Common::String(startP, endP));

		// Strip line off from string being processed
		journalString = *endP ? Common::String(endP + 1) : "";
	}

	// Add a blank line at the end of the text as long as text was present
	if (!startOfReply) {
		_lines.push_back("");
	} else {
		_lines.clear();
	}

	return _lines.size();
}

void Journal::drawInterface() {
	Resources &res = *_vm->_res;
	Screen &screen = *_vm->_screen;
	byte palette[PALETTE_SIZE];

	// Load in the journal background
	Common::SeekableReadStream *bg = res.load("journal.lbv");
	bg->read(screen._backBuffer1.getPixels(), SHERLOCK_SCREEN_WIDTH * SHERLOCK_SCREEN_HEIGHT);
	bg->read(palette, PALETTE_SIZE);
	delete bg;

	// Set the palette and print the title
	screen.setPalette(palette);
	screen.gPrint(Common::Point(111, 18), BUTTON_BOTTOM, "Watson's Journal");
	screen.gPrint(Common::Point(110, 17), INV_FOREGROUND, "Watson's Journal");

	// Draw the buttons
	screen.makeButton(Common::Rect(JOURNAL_POINTS[0][0], JOURNAL_BUTTONS_Y, 
		JOURNAL_POINTS[0][1], JOURNAL_BUTTONS_Y + 10), 
		JOURNAL_POINTS[0][2] - screen.stringWidth("Exit") / 2, "Exit");
	screen.makeButton(Common::Rect(JOURNAL_POINTS[1][0], JOURNAL_BUTTONS_Y, 
		JOURNAL_POINTS[1][1], JOURNAL_BUTTONS_Y + 10), 
		JOURNAL_POINTS[1][2] - screen.stringWidth("Back 10") / 2, "Back 10");
	screen.makeButton(Common::Rect(JOURNAL_POINTS[2][0], JOURNAL_BUTTONS_Y, 
		JOURNAL_POINTS[2][1], JOURNAL_BUTTONS_Y + 10), 
		JOURNAL_POINTS[2][2] - screen.stringWidth("Up") / 2, "Up");
	screen.makeButton(Common::Rect(JOURNAL_POINTS[3][0], JOURNAL_BUTTONS_Y, 
		JOURNAL_POINTS[3][1], JOURNAL_BUTTONS_Y + 10), 
		JOURNAL_POINTS[3][2] - screen.stringWidth("Down") / 2, "Down");
	screen.makeButton(Common::Rect(JOURNAL_POINTS[4][0], JOURNAL_BUTTONS_Y, 
		JOURNAL_POINTS[4][1], JOURNAL_BUTTONS_Y + 10),
		JOURNAL_POINTS[4][2] - screen.stringWidth("Ahead 10") / 2, "Ahead 10");
	screen.makeButton(Common::Rect(JOURNAL_POINTS[5][0], JOURNAL_BUTTONS_Y + 11, 
		JOURNAL_POINTS[5][1], JOURNAL_BUTTONS_Y + 21),
		JOURNAL_POINTS[5][2] - screen.stringWidth("Search") / 2, "Search");
	screen.makeButton(Common::Rect(JOURNAL_POINTS[6][0], JOURNAL_BUTTONS_Y + 11, 
		JOURNAL_POINTS[6][1], JOURNAL_BUTTONS_Y + 21),
		JOURNAL_POINTS[6][2] - screen.stringWidth("First Page") / 2, "First Page");
	screen.makeButton(Common::Rect(JOURNAL_POINTS[7][0], JOURNAL_BUTTONS_Y + 11, 
		JOURNAL_POINTS[7][1], JOURNAL_BUTTONS_Y + 21),
		JOURNAL_POINTS[7][2] - screen.stringWidth("Last Page") / 2, "Last Page");
	screen.makeButton(Common::Rect(JOURNAL_POINTS[8][0], JOURNAL_BUTTONS_Y + 11, 
		JOURNAL_POINTS[8][1], JOURNAL_BUTTONS_Y + 21),
		JOURNAL_POINTS[8][2] - screen.stringWidth("Print Text") / 2, "Print Text");

	if (_journal.size() == 0) {
		_up = _down = 0;
	} else {
		doJournal(0, 0);
	}

	doArrows();
		
	// Show the entire screen
	screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
}

/**
 * Display the arrows that can be used to scroll up and down pages
 */
void Journal::doArrows() {
	Screen &screen = *_vm->_screen;
	byte color;

	color = (_page > 1) ? COMMAND_FOREGROUND : COMMAND_NULL;
	screen.buttonPrint(Common::Point(JOURNAL_POINTS[1][2], JOURNAL_BUTTONS_Y), color, false, "Back 10");
	screen.buttonPrint(Common::Point(JOURNAL_POINTS[2][2], JOURNAL_BUTTONS_Y), color, false, "Up");

	color = _down ? COMMAND_FOREGROUND : COMMAND_NULL;
	screen.buttonPrint(Common::Point(JOURNAL_POINTS[3][2], JOURNAL_BUTTONS_Y), color, false, "Down");
	screen.buttonPrint(Common::Point(JOURNAL_POINTS[4][2], JOURNAL_BUTTONS_Y), color, false, "Ahead 10");
	screen.buttonPrint(Common::Point(JOURNAL_POINTS[7][2], JOURNAL_BUTTONS_Y + 11), color, false, "Last Page");

	color = _journal.size() > 0 ? COMMAND_FOREGROUND : COMMAND_NULL;
	screen.buttonPrint(Common::Point(JOURNAL_POINTS[5][2], JOURNAL_BUTTONS_Y + 11), color, false, "Search");
	screen.buttonPrint(Common::Point(JOURNAL_POINTS[8][2], JOURNAL_BUTTONS_Y + 11), color, false, "Print Text");

	color = _page > 1 ? COMMAND_FOREGROUND : COMMAND_NULL;
	screen.buttonPrint(Common::Point(JOURNAL_POINTS[6][2], JOURNAL_BUTTONS_Y + 11), color, false, "First Page");
}

/**
 * Displays a page of the journal at the current index
 */
bool Journal::doJournal(int direction, int howFar) {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	int yp = 37;
	int startPage = _page;
	bool endJournal = false;
	bool firstOccurance = false;
	bool searchSuccessful = false;
	bool endFlag = false;
	int lineNum = 0;
	int maxLines;
	int savedIndex;
	int savedSub;
	int temp;
	bool inc;
	const char *matchP;
	int width;

	_converseNum = -1;
	_down = true;

	do {
		// Get the number of lines for the current journal entry
		maxLines = loadJournalFile(false);
		if (!maxLines) {
			// Entry has no text, so it must be a stealth eny. Move onto further journal entries
			// until an entry with text is found
			if (++_index == (int)_journal.size()) {
				endJournal = true;
			} else {
				_sub = 0;
				maxLines = loadJournalFile(false);
			}
		}
	} while (!endJournal && !maxLines);

	// Check if there no further pages with text until the end of the journal
	if (endJournal) {
		// If moving forward or backwards, clear the page before printing
		if (direction)
			clearPage();

		screen.gPrint(Common::Point(235, 21), PEN_COLOR, "Page %d", _page);
		return false;
	}

	// If the journal page is being changed, set the wait cursor
	if (direction)
		events.setCursor(WAIT);

	switch (direction) {
	case 1:
	case 4:
		// Move backwards howFar number of lines unless either the start of the journal is reached,
		// or a searched for keyword is found
		do {
			// Animate the glass mouse cursor
			int cursorNum = (int)events.getCursor() + 1;
			if (cursorNum > (WAIT + 2))
				cursorNum = WAIT;
			events.setCursor((CursorId)cursorNum);

			// Move backwards through the journal file a line at a time
			if (--_sub < 0) {
				do {
					if (--_index < 0) {
						_index = 0;
						_sub = 0;
						endJournal = true;
					}
					else {
						maxLines = loadJournalFile(false);
						_sub = maxLines - 1;
					}
				} while (!endJournal && !maxLines);
			}

			// If it's search mode, check each line for the given keyword
			if (direction >= 3 && maxLines && !endJournal && !searchSuccessful) {
				Common::String line = _lines[_sub];
				line.toUppercase();
				if (strstr(line.c_str(), _find.c_str()) != nullptr) {
					// Found a match. Reset howFar so that the start of page that the match
					// was found on will be displayed
					searchSuccessful = true;
					howFar = ((lineNum / LINES_PER_PAGE) + 1) * LINES_PER_PAGE;
				}
			}

			++lineNum;
		} while (lineNum < howFar && !endJournal);

		if (!_index && !_sub)
			_page = 1;
		else
			_page -= howFar / LINES_PER_PAGE;
		break;

	case 2:
	case 3:
		// Move howFar lines ahead unless the end of the journal is reached, 
		// or a searched for keyword is found
		for (temp = 0; (temp < (howFar / LINES_PER_PAGE)) && !endJournal && !searchSuccessful; ++temp) {
			// Handle animating mouse cursor
			int cursorNum = (int)events.getCursor() + 1;
			if (cursorNum >(WAIT + 2))
				cursorNum = WAIT;
			events.setCursor((CursorId)cursorNum);

			lineNum = 0;
			savedIndex = _index;
			savedSub = _sub;

			// Move a single page ahead
			do {
				// If in search mode, check for keyword
				if (direction >= 3 && _page != startPage) {
					Common::String line = _lines[_sub];
					line.toUppercase();
					if (strstr(line.c_str(), _find.c_str()) != nullptr)
						searchSuccessful = true;
				}

				// Move forwards a line at a time, unless search word was found
				if (!searchSuccessful) {
					if (++_sub == maxLines) {
						// Reached end of page
						do {
							if (++_index == (int)_lines.size()) {
								_index = savedIndex;
								_sub = savedSub;
								maxLines = loadJournalFile(false);
								endJournal = true;
							} else {
								_sub = 0;
								maxLines = loadJournalFile(false);
							}
						} while (!endJournal && !maxLines);						
					}

					++lineNum;
				}
			} while ((lineNum < LINES_PER_PAGE) && !endJournal && !searchSuccessful);

			if (!endJournal && !searchSuccessful)
				// Move to next page
				++_page;

			if (searchSuccessful) {
				// Search found, so show top of the page it was found on
				_index = savedIndex;
				_sub = savedSub;
				maxLines = loadJournalFile(false);
			}
		}
		break;

	default:
		break;
	}

	if (direction) {
		events.setCursor(ARROW);
		clearPage();
	}

	screen.gPrint(Common::Point(235, 21), PEN_COLOR, "Page %d", _page);

	temp = _sub;
	savedIndex = _index;
	lineNum = 0;

	do {
		inc = true;

		// If there wasn't any line to print at the top of the page, we won't need to
		// increment the y position
		if (_lines[temp].empty() && yp == 37)
			inc = false;

		// If there's a searched for keyword in the line, it will need to be highlighted
		if (searchSuccessful && firstOccurance) {
			// Check if line has the keyword
			Common::String line = _lines[temp];
			line.toUppercase();
			if ((matchP = strstr(line.c_str(), _find.c_str())) != nullptr) {
				matchP = _lines[temp].c_str() + (matchP - line.c_str());
				firstOccurance = false;

				// Print out the start of the line before the matching keyword
				Common::String lineStart(_lines[temp].c_str(), matchP);
				if (lineStart.hasPrefix("@")) {
					width = screen.stringWidth(lineStart.c_str() + 1);
					screen.gPrint(Common::Point(53, yp), 15, "%s", lineStart.c_str() + 1);
				} else {
					width = screen.stringWidth(lineStart.c_str());
					screen.gPrint(Common::Point(53, yp), PEN_COLOR, lineStart.c_str());
				}

				// Print out the found keyword
				Common::String lineMatch(matchP, matchP + _find.size());
				screen.gPrint(Common::Point(53 + width, yp), INV_FOREGROUND, lineMatch.c_str());
				width += screen.stringWidth(lineMatch.c_str());

				// Print remainder of line
				screen.gPrint(Common::Point(53 + width, yp), PEN_COLOR, matchP + _find.size());
			} else if (_lines[temp].hasPrefix("@")) {
				screen.gPrint(Common::Point(53, yp), 15, _lines[temp].c_str() + 1);
			} else {
				screen.gPrint(Common::Point(53, yp), PEN_COLOR, _lines[temp].c_str());
			}
		} else {
			if (_lines[temp].hasPrefix("@")) {
				screen.gPrint(Common::Point(53, yp), 15, _lines[temp].c_str() + 1);
			} else {
				screen.gPrint(Common::Point(53, yp), PEN_COLOR, _lines[temp].c_str());
			}
		}

		if (++temp == maxLines) {
			// Move to next page
			do {
				if (_index < (_journal.size() - 1) && lineNum < (LINES_PER_PAGE - 1)) {
					++_index;
					maxLines = loadJournalFile(false);
					temp = 0;
				} else {
					if (_index == (_journal.size() - 1))
						_down = false;
					endFlag = true;
				}
			} while (!endFlag && !maxLines);
		}

		if (inc) {
			// Move to next line
			++lineNum;
			yp += 13;
		}
	} while (lineNum < LINES_PER_PAGE && !endFlag);

	_index = savedIndex;
	_up = _index || _sub;

	return direction >= 3 && searchSuccessful;
}

/**
 * Clears the journal page
 */
void Journal::clearPage() {
	// Clear the journal page by redrawing it from scratch
	drawInterface();
}

} // End of namespace Sherlock
