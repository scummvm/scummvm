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
#include "sherlock/scalpel/scalpel.h"
#include "sherlock/scalpel/scalpel_fixed_text.h"
#include "sherlock/scalpel/scalpel_journal.h"
#include "sherlock/tattoo/tattoo.h"
#include "sherlock/tattoo/tattoo_journal.h"

namespace Sherlock {

Journal *Journal::init(SherlockEngine *vm) {
	if (vm->getGameID() == GType_SerratedScalpel)
		return new Scalpel::ScalpelJournal(vm);
	else
		return new Tattoo::TattooJournal(vm);
}

Journal::Journal(SherlockEngine *vm) : _vm(vm) {
	_up = _down = false;
	_index = 0;
	_page = 0;
	_maxPage = 0;
	_sub = 0;
}

bool Journal::drawJournal(int direction, int howFar) {
	Events &events = *_vm->_events;
	FixedText &fixedText = *_vm->_fixedText;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	int yp = 37;
	int startPage = _page;
	bool endJournal = false;
	bool firstOccurance = true;
	bool searchSuccessful = false;
	bool endFlag = false;
	int lineNum = 0;
	int savedIndex;
	int temp;
	const char *matchP;
	int width;

	talk._converseNum = -1;
	_down = true;

	do {
		// Get the number of lines for the current journal entry
		loadJournalFile(false);
		if (_lines.empty()) {
			// Entry has no text, so it must be a stealth eny. Move onto further journal entries
			// until an entry with text is found
			if (++_index == (int)_journal.size()) {
				endJournal = true;
			} else {
				_sub = 0;
				loadJournalFile(false);
			}
		}
	} while (!endJournal && _lines.empty());

	// Check if there no further pages with text until the end of the journal
	if (endJournal) {
		// If moving forward or backwards, clear the page before printing
		if (direction)
			drawFrame();

		screen.gPrint(Common::Point(235, 21), COL_PEN_COLOR, "Page %d", _page);
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
			// Move backwards through the journal file a line at a time
			if (--_sub < 0) {
				do {
					if (--_index < 0) {
						_index = 0;
						_sub = 0;
						endJournal = true;
					}
					else {
						loadJournalFile(false);
						_sub = _lines.size() - 1;
					}
				} while (!endJournal && _lines.empty());
			}

			// If it's search mode, check each line for the given keyword
			if (direction >= 3 && !_lines.empty() && !endJournal && !searchSuccessful) {
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
			int savedSub = _sub;

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
					if (++_sub == (int)_lines.size()) {
						// Reached end of page
						do {
							if (++_index == (int)_journal.size()) {
								_index = savedIndex;
								_sub = savedSub;
								loadJournalFile(false);
								endJournal = true;
							} else {
								_sub = 0;
								loadJournalFile(false);
							}
						} while (!endJournal && _lines.empty());
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
				loadJournalFile(false);
			}
		}
		break;

	default:
		break;
	}

	if (direction) {
		events.setCursor(ARROW);
		drawFrame();
	}

	Common::String fixedText_Page = IS_SERRATED_SCALPEL ? fixedText.getText(Scalpel::kFixedText_Journal_Page) : "TODO";

	screen.gPrint(Common::Point(235, 21), COL_PEN_COLOR, fixedText_Page.c_str(), _page);

	temp = _sub;
	savedIndex = _index;
	lineNum = 0;

	do {
		bool inc = true;

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
					screen.gPrint(Common::Point(53, yp), COL_PEN_COLOR, "%s", lineStart.c_str());
				 }

				// Print out the found keyword
				Common::String lineMatch(matchP, matchP + _find.size());
				byte fgColor = IS_SERRATED_SCALPEL ? (byte)Scalpel::INV_FOREGROUND : (byte)Tattoo::INV_FOREGROUND;
				screen.gPrint(Common::Point(53 + width, yp), fgColor, "%s", lineMatch.c_str());
				width += screen.stringWidth(lineMatch.c_str());

				// Print remainder of line
				screen.gPrint(Common::Point(53 + width, yp), COL_PEN_COLOR, "%s", matchP + _find.size());
			} else if (_lines[temp].hasPrefix("@")) {
				screen.gPrint(Common::Point(53, yp), 15, "%s", _lines[temp].c_str() + 1);
			} else {
				screen.gPrint(Common::Point(53, yp), COL_PEN_COLOR, "%s", _lines[temp].c_str());
			}
		} else {
			if (_lines[temp].hasPrefix("@")) {
				screen.gPrint(Common::Point(53, yp), 15, "%s", _lines[temp].c_str() + 1);
			} else {
				screen.gPrint(Common::Point(53, yp), COL_PEN_COLOR, "%s", _lines[temp].c_str());
			}
		}

		if (++temp == (int)_lines.size()) {
			// Move to next page
			do {
				if (_index < ((int)_journal.size() - 1) && lineNum < (LINES_PER_PAGE - 1)) {
					++_index;
					loadJournalFile(false);
					temp = 0;
				} else {
					if (_index == ((int)_journal.size() - 1))
						_down = false;
					endFlag = true;
				}
			} while (!endFlag && _lines.empty());
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

void Journal::loadJournalFile(bool alreadyLoaded) {
	People &people = *_vm->_people;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	JournalEntry &journalEntry = _journal[_index];
	const byte *opcodes = talk._opcodes;

	Common::String dirFilename = _directory[journalEntry._converseNum];
	bool replyOnly = journalEntry._replyOnly;

	// Get the location number from within the filename
	Common::String locStr(dirFilename.c_str() + 4, dirFilename.c_str() + 6);
	int newLocation = atoi(locStr.c_str());

	// If not flagged as already loaded, load the conversation into script variables
	if (!alreadyLoaded) {
		// See if the file to be used is already loaded
		if (journalEntry._converseNum != talk._converseNum) {
			// Nope. Free any previously loaded talk
			talk.freeTalkVars();

			// Find the person being referred to
			talk._talkTo = -1;
			for (int idx = 0; idx < (int)people._characters.size(); ++idx) {
				Common::String portrait = people._characters[idx]._portrait;
				Common::String numStr(portrait.c_str(), portrait.c_str() + 4);

				if (locStr == numStr) {
					talk._talkTo = idx;
					break;
				}
			}

			// Load their talk file
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
	Statement &statement = talk[journalEntry._statementNum];
	Common::String journalString;

	if (newLocation != oldLocation) {
		// Add in scene title
		journalString = "@";
		if (IS_SERRATED_SCALPEL || newLocation - 1 < 100)
			journalString += _locations[newLocation - 1];
		journalString += ":";

		// See if title can fit into a single line, or requires splitting on 2 lines
		int width = screen.stringWidth(journalString.c_str() + 1);
		if (width > JOURNAL_MAX_WIDTH) {
			// Scan backwards from end of title to find a space between a word
			// where the width is less than the maximum allowed for the line
			const char *lineP = journalString.c_str() + journalString.size() - 1;
			while (width > JOURNAL_MAX_WIDTH || *lineP != ' ')
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

		if (talk._talkTo == 1) {
			journalString += "me";
		} else if ((talk._talkTo == 2 && IS_SERRATED_SCALPEL) || (talk._talkTo == 18 && IS_ROSE_TATTOO)) {
			journalString += "the Inspector";
		} else {
			journalString += people._characters[talk._talkTo]._name;
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
	const byte *replyP = (const byte *)statement._reply.c_str();
	const int inspectorId = (IS_SERRATED_SCALPEL) ? 2 : 18;

	while (*replyP) {
		byte c = *replyP++;

		if (IS_ROSE_TATTOO) {
			// Ignore commented out data
			if (c == '/' && *(replyP + 1) == '*') {
				replyP++;	// skip *
				while (*replyP++ != '*') {}	// empty loop on purpose
				replyP++;	// skip /
				c = *replyP;
			}
		}

		// Is it a control character?
		if (c < opcodes[0]) {
			// Nope. Set flag for allowing control codes to insert spaces
			ctrlSpace = true;
			assert(c >= ' ');

			// Check for embedded comments
			if (c == '{' || c == '}') {

				// TODO: Rose Tattoo checks if no text was added for the last
				// comment here. In such a case, the last "XXX said" string is
				// removed here.

				// Comment characters. If we're starting a comment and there's
				// already text displayed, add a closing quote
				if (c == '{' && !startOfReply && !commentJustPrinted)
					journalString += '"';

				// If a reply isn't just being started, and we didn't just end
				// a comment (which would have added a line), add a carriage return
				if (!startOfReply && ((!commentJustPrinted && c == '{') || c == '}'))
					journalString += '\n';
				startOfReply = false;

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
						else if (talk._talkTo == inspectorId)
							journalString += "The Inspector";
						else
							journalString += people._characters[talk._talkTo]._name;

						const byte *strP = replyP + 1;
						byte v;
						do {
							v = *strP++;
						} while (v && (v < opcodes[0]) && (v != '.') && (v != '!') && (v != '?'));

						if (v == '?')
							journalString += " asked, \"";
						else
							journalString += " said, \"";
					}

					startOfReply = false;
				}

				// Copy text from the place until either the reply ends, a comment
				// {} block is started, or a control character is encountered
				journalString += c;
				do {
					journalString += *replyP++;
				} while (*replyP && *replyP < opcodes[0] && *replyP != '{' && *replyP != '}');

				commentJustPrinted = false;
			}
		} else if (c == opcodes[OP_SWITCH_SPEAKER]) {
			if (!startOfReply) {
				if (!commentFlag && !commentJustPrinted)
					journalString += "\"\n";

				journalString += "Then ";
				commentFlag = false;
			} else if (!replyOnly) {
				journalString += "\"\n";
			}

			startOfReply = false;
			c = *replyP++ - 1;
			if (IS_ROSE_TATTOO)
				replyP++;

			if (c == 0)
				journalString += "Holmes";
			else if (c == 1)
				journalString += "I";
			else if (c == inspectorId)
				journalString += "the Inspector";
			else
				journalString += people._characters[c]._name;

			const byte *strP = replyP;
			byte v;
			do {
				v = *strP++;
			} while (v && v < opcodes[0] && v != '.' && v != '!' && v != '?');

			if (v == '?')
				journalString += " asked, \"";
			else
				journalString += " said, \"";
		} else {
			if (IS_SERRATED_SCALPEL) {
				// Control code, so move past it and any parameters
				if (c == opcodes[OP_RUN_CANIMATION] ||
					c == opcodes[OP_ASSIGN_PORTRAIT_LOCATION] ||
					c == opcodes[OP_PAUSE] ||
					c == opcodes[OP_PAUSE_WITHOUT_CONTROL] ||
					c == opcodes[OP_WALK_TO_CANIMATION]) {
					// These commands have a single parameter
					++replyP;
				} else if (c == opcodes[OP_ADJUST_OBJ_SEQUENCE]) {
					replyP += (replyP[0] & 127) + replyP[1] + 2;
				} else if (c == opcodes[OP_WALK_TO_COORDS] || c == opcodes[OP_MOVE_MOUSE]) {
					replyP += 4;
				} else if (c == opcodes[OP_SET_FLAG] || c == opcodes[OP_IF_STATEMENT]) {
					replyP += 2;
				} else if (c == opcodes[OP_SFX_COMMAND] || c == opcodes[OP_PLAY_PROLOGUE] ||
					c == opcodes[OP_CALL_TALK_FILE]) {
					replyP += 8;
					break;
				} else if (
					c == opcodes[OP_TOGGLE_OBJECT] ||
					c == opcodes[OP_ADD_ITEM_TO_INVENTORY] ||
					c == opcodes[OP_SET_OBJECT] ||
					c == opcodes[OP_DISPLAY_INFO_LINE] ||
					c == opcodes[OP_REMOVE_ITEM_FROM_INVENTORY]) {
					replyP += (*replyP & 127) + 1;
				} else if (c == opcodes[OP_GOTO_SCENE]) {
					replyP += 5;
				} else if (c == opcodes[OP_CARRIAGE_RETURN]) {
					journalString += "\n";
				}
			} else {
				if (c == opcodes[OP_RUN_CANIMATION] ||
					c == opcodes[OP_PAUSE] ||
					c == opcodes[OP_MOUSE_OFF_ON] ||
					c == opcodes[OP_SET_WALK_CONTROL] ||
					c == opcodes[OP_PAUSE_WITHOUT_CONTROL] ||
					c == opcodes[OP_WALK_TO_CANIMATION] ||
					c == opcodes[OP_TURN_NPC_OFF] ||
					c == opcodes[OP_TURN_NPC_ON] ||
					c == opcodes[OP_RESTORE_PEOPLE_SEQUENCE])
					++replyP;
				else if (
					c == opcodes[OP_SET_TALK_SEQUENCE] ||
					c == opcodes[OP_SET_FLAG] ||
					c == opcodes[OP_WALK_NPC_TO_CANIM] ||
					c == opcodes[OP_WALK_HOLMES_AND_NPC_TO_CANIM] ||
					c == opcodes[OP_NPC_PATH_LABEL] ||
					c == opcodes[OP_PATH_GOTO_LABEL])
					replyP += 2;
				else if (
					c == opcodes[OP_SET_NPC_PATH_PAUSE] ||
					c == opcodes[OP_NPC_PATH_PAUSE_TAKING_NOTES] ||
					c == opcodes[OP_NPC_PATH_PAUSE_LOOKING_HOLMES] ||
					c == opcodes[OP_NPC_VERB_CANIM])
					replyP += 3;
				else if (
					c == opcodes[OP_SET_SCENE_ENTRY_FLAG] ||
					c == opcodes[OP_PATH_IF_FLAG_GOTO_LABEL])
					replyP += 4;
				else if (
					c == opcodes[OP_WALK_TO_COORDS])
					replyP += 5;
				else if (
					c == opcodes[OP_WALK_NPC_TO_COORDS] ||
					c == opcodes[OP_GOTO_SCENE] ||
					c == opcodes[OP_SET_NPC_PATH_DEST] ||
					c == opcodes[OP_SET_NPC_POSITION])
					replyP += 6;
				else if (
					c == opcodes[OP_PLAY_SONG] ||
					c == opcodes[OP_NEXT_SONG])
					replyP += 8;
				else if (
					c == opcodes[OP_CALL_TALK_FILE] ||
					c == opcodes[OP_SET_NPC_TALK_FILE] ||
					c == opcodes[OP_NPC_WALK_GRAPHICS])
					replyP += 9;
				else if (
					c == opcodes[OP_NPC_VERB_SCRIPT])
					replyP += 10;
				else if (
					c == opcodes[OP_WALK_HOLMES_AND_NPC_TO_COORDS])
					replyP += 11;
				else if (
					c == opcodes[OP_NPC_VERB] ||
					c == opcodes[OP_NPC_VERB_TARGET])
					replyP += 14;
				else if (
					c == opcodes[OP_ADJUST_OBJ_SEQUENCE])
					replyP += (replyP[0] & 127) + replyP[1] + 2;
				else if (
					c == opcodes[OP_TOGGLE_OBJECT] ||
					c == opcodes[OP_ADD_ITEM_TO_INVENTORY] ||
					c == opcodes[OP_SET_OBJECT] ||
					c == opcodes[OP_REMOVE_ITEM_FROM_INVENTORY])
					replyP += (*replyP & 127) + 1;
				else if (
					c == opcodes[OP_END_TEXT_WINDOW]) {
					journalString += '\n';
				} else if (
					c == opcodes[OP_NPC_DESC_ON_OFF]) {
					replyP++;
					while (replyP[0] && replyP[0] != opcodes[OP_NPC_DESC_ON_OFF])
						replyP++;
					replyP++;
				} else if (
					c == opcodes[OP_SET_NPC_INFO_LINE])
					replyP += replyP[1] + 2;
			}

			// Put a space in the output for a control character, unless it's
			// immediately coming after another control character
			if (ctrlSpace && c != opcodes[OP_ASSIGN_PORTRAIT_LOCATION] && c != opcodes[OP_CARRIAGE_RETURN] && 
					!commentJustPrinted) {
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
		while (width < JOURNAL_MAX_WIDTH && *endP && *endP != '\n' && (endP - startP) < (JOURNAL_MAX_CHARS - 1))
			width += screen.charWidth(*endP++);

		// If word wrapping, move back to end of prior word
		if (width >= JOURNAL_MAX_WIDTH || (endP - startP) >= (JOURNAL_MAX_CHARS - 1)) {
			while (*--endP != ' ')
				;
		}

		// Add in the line
		_lines.push_back(Common::String(journalString.c_str(), endP));

		// Strip line off from string being processed
		journalString = *endP ? Common::String(endP + 1) : "";
	}

	// Add a blank line at the end of the text as long as text was present
	if (!startOfReply) {
		_lines.push_back("");
	} else {
		_lines.clear();
	}
}

} // End of namespace Sherlock
