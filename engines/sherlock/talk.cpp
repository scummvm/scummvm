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

#include "sherlock/talk.h"
#include "sherlock/sherlock.h"
#include "sherlock/screen.h"

namespace Sherlock {

enum {
	SWITCH_SPEAKER				= 128,
	RUN_CANIMATION				= 129,
	ASSIGN_PORTRAIT_LOCATION	= 130,
	PAUSE						= 131,
	REMOVE_PORTRAIT				= 132,
	CLEAR_WINDOW				= 133,
	ADJUST_OBJ_SEQUENCE			= 134,
	WALK_TO_COORDS				= 135,
	PAUSE_WITHOUT_CONTROL		= 136,
	BANISH_WINDOW				= 137,
	SUMMON_WINDOW				= 138,
	SET_FLAG					= 139,
	SFX_COMMAND					= 140,
	TOGGLE_OBJECT				= 141,
	STEALTH_MODE_ACTIVE			= 142,
	IF_STATEMENT				= 143,
	ELSE_STATEMENT				= 144,
	END_IF_STATEMENT			= 145,
	STEALTH_MODE_DEACTIVATE		= 146,
	TURN_HOLMES_OFF				= 147,
	TURN_HOLMES_ON				= 148,
	GOTO_SCENE					= 149,
	PLAY_PROLOGUE				= 150,
	ADD_ITEM_TO_INVENTORY		= 151,
	SET_OBJECT					= 152,
	CALL_TALK_FILE				= 153,
	MOVE_MOUSE					= 154,
	DISPLAY_INFO_LINE			= 155,
	CLEAR_INFO_LINE				= 156,
	WALK_TO_CANIMATION			= 157,
	REMOVE_ITEM_FROM_INVENTORY	= 158,
	ENABLE_END_KEY				= 159,
	DISABLE_END_KEY				= 160,
	COMMAND_161					= 161
};

/*----------------------------------------------------------------*/

/**
 * Load the data for a single statement within a talk file
 */
void Statement::synchronize(Common::SeekableReadStream &s) {
	int length;

	length = s.readUint16LE();
	for (int idx = 0; idx < length - 1; ++idx)
		_statement += (char)s.readByte();
	s.readByte();	// Null ending

	length = s.readUint16LE();
	for (int idx = 0; idx < length - 1; ++idx)
		_reply += (char)s.readByte();
	s.readByte();	// Null ending

	length = s.readUint16LE();
	for (int idx = 0; idx < length - 1; ++idx)
		_linkFile += (char)s.readByte();
	s.readByte();	// Null ending

	length = s.readUint16LE();
	for (int idx = 0; idx < length - 1; ++idx)
		_voiceFile += (char)s.readByte();
	s.readByte();	// Null ending

	_required.resize(s.readByte());
	_modified.resize(s.readByte());

	// Read in flag required/modified data
	for (uint idx = 0; idx < _required.size(); ++idx)
		_required[idx] = s.readSint16LE();
	for (uint idx = 0; idx < _modified.size(); ++idx)
		_modified[idx] = s.readSint16LE();

	_portraitSide = s.readByte();
	_quotient = s.readUint16LE();
}

/*----------------------------------------------------------------*/

TalkHistoryEntry::TalkHistoryEntry() {
	Common::fill(&_data[0], &_data[16], false);
}

/*----------------------------------------------------------------*/

TalkSequences::TalkSequences(const byte *data) {
	Common::copy(data, data + MAX_TALK_SEQUENCES, _data);
}

void TalkSequences::clear() {
	Common::fill(&_data[0], &_data[MAX_TALK_SEQUENCES], 0); 
}

/*----------------------------------------------------------------*/

Talk::Talk(SherlockEngine *vm): _vm(vm) {
	_talkCounter = 0;
	_talkToAbort = false;
	_saveSeqNum = 0;
	_speaker = 0;
	_talkIndex = 0;
	_talkTo = 0;
	_scriptSelect = 0;
	_converseNum = -1;
	_talkStealth = 0;
	_talkToFlag = -1;
	_moreTalkDown = _moreTalkUp = false;
	_scriptMoreFlag = 0;
	_scriptSaveIndex = -1;
	_scriptCurrentIndex = -1;
}

void Talk::setSequences(const byte *talkSequences, const byte *stillSequences, int maxPeople) {
	for (int idx = 0; idx < maxPeople; ++idx) {
		STILL_SEQUENCES.push_back(TalkSequences(stillSequences));
		TALK_SEQUENCES.push_back(TalkSequences(talkSequences));
		stillSequences += MAX_TALK_SEQUENCES;
		talkSequences += MAX_TALK_SEQUENCES;
	}
}

/**
 * Called when either an NPC initiates a conversation or for inventory item
 * descriptions. It opens up a description window similar to how 'talk' does,
 * but shows a 'reply' directly instead of waiting for a statement option.
 */
void Talk::talkTo(const Common::String &filename) {
	Events &events = *_vm->_events;
	Inventory &inv = *_vm->_inventory;
	Journal &journal = *_vm->_journal;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	UserInterface &ui = *_vm->_ui;
	Common::Rect savedBounds = screen.getDisplayBounds();
	bool abortFlag = false;

	if (filename.empty())
		// No filename passed, so exit
		return;

	// If there any canimations currently running, or a portrait is being cleared,
	// save the filename for later executing when the canimation is done
	if (scene._ongoingCans || people._clearingThePortrait) {
		// Make sure we're not in the middle of a script
		if (!_scriptMoreFlag) {
			_scriptName = filename;
			_scriptSaveIndex = 0;

			// Flag the selection, since we don't yet know which statement yet
			_scriptSelect = 100;
			_scriptMoreFlag = 3;
		}

		return;
	}

	// Save the ui mode temporarily and switch to talk mode
	int savedMode = ui._menuMode;
	ui._menuMode = TALK_MODE;

	// Turn on the Exit option
	ui._endKeyActive = true;

	if (people[AL]._walkCount || people._walkTo.size() > 0) {
		// Only interrupt if an action if trying to do an action, and not just
		// if the player is walking around the scene
		if (people._allowWalkAbort)
			abortFlag = true;

		people.gotoStand(people._player);
	}

	if (_talkToAbort)
		return;

	freeTalkVars();

	// If any sequences have changed in the prior talk file, restore them
	if (_savedSequences.size() > 0) {
		for (uint idx = 0; idx < _savedSequences.size(); ++idx) {
			SequenceEntry &ss = _savedSequences[idx];
			for (uint idx2 = 0; idx2 < _savedSequences.size(); ++idx2)
				scene._bgShapes[ss._objNum]._sequences[idx2] = ss._sequences[idx2];
		
			// Reset the object's frame to the beginning of the sequence
			scene._bgShapes[ss._objNum]._frameNumber = 0;
		}
	}

	while (!_sequenceStack.empty())
		pullSequence();

	// Restore any pressed button
	if (!ui._windowOpen && savedMode != STD_MODE)
		ui.restoreButton(savedMode - 1);

	// Clear the ui counter so that anything displayed on the info line
	// before the window was opened isn't cleared
	ui._menuCounter = 0;

	// Close any previous window before starting the talk
	if (ui._windowOpen) {
		switch (savedMode) {
		case LOOK_MODE:
			events.setCursor(ARROW);

			if (ui._invLookFlag) {
				screen.resetDisplayBounds();
				ui.drawInterface(2);
			}

			ui.banishWindow();
			ui._windowBounds.top = CONTROLS_Y1;
			ui._temp = ui._oldTemp = ui._lookHelp = 0;
			ui._menuMode = STD_MODE;
			events._pressed = events._released = events._oldButtons = 0;
			ui._invLookFlag = false;
			break;

		case TALK_MODE:
			if (_speaker < 128)
				people.clearTalking();
			if (_talkCounter)
				return;

			// If we were in inventory mode looking at an object, restore the
			// back buffers before closing the window, so we get the ui restored
			// rather than the inventory again
			if (ui._invLookFlag) {
				screen.resetDisplayBounds();
				ui.drawInterface(2);
				ui._invLookFlag = ui._lookScriptFlag = false;
			}

			ui.banishWindow();
			ui._windowBounds.top = CONTROLS_Y1;
			abortFlag = true;
			break;

		case INV_MODE:
		case USE_MODE:
		case GIVE_MODE:
			inv.freeInv();
			if (ui._invLookFlag) {
				screen.resetDisplayBounds();
				ui.drawInterface(2);
				ui._invLookFlag = ui._lookScriptFlag = false;
			}

			ui._infoFlag = true;
			ui.clearInfo();
			ui.banishWindow(false);
			ui._key = -1;
			break;

		case FILES_MODE:
			ui.banishWindow(true);
			ui._windowBounds.top = CONTROLS_Y1;
			abortFlag = true;
			break;

		case SETUP_MODE:
			ui.banishWindow(true);
			ui._windowBounds.top = CONTROLS_Y1;
			ui._temp = ui._oldTemp = ui._lookHelp = ui._invLookFlag = false;
			ui._menuMode = STD_MODE;
			events._pressed = events._released = events._oldButtons = 0;
			abortFlag = true;
			break;
		}
	}

	screen.resetDisplayBounds();
	events._pressed = events._released = false;
	loadTalkFile(filename);
	ui._selector = ui._oldSelector = ui._key = ui._oldKey = -1;

	// Find the first statement that has the correct flags
	int select = -1;
	for (uint idx = 0; idx < _statements.size() && select == -1; ++idx) {
		if (_statements[idx]._talkMap == 0)
			select = _talkIndex = idx;
	}

	// If there's a pending automatic selection to be made, then use it
	if (_scriptMoreFlag && _scriptSelect != 100)
		select = _scriptSelect;

	if (select == -1)
		error("Couldn't find statement to display");

	// Add the statement into the journal and talk history
	if (_talkTo != -1 && !_talkHistory[_converseNum][select])
		journal.record(_converseNum, select, true);
	_talkHistory[_converseNum][select] = true;

	// Check if the talk file is meant to be a non-seen comment
	if (filename.size() < 8 || filename[7] != '*') {
		// Should we start in stealth mode?
		if (_statements[select]._statement.hasPrefix("^")) {
			_talkStealth = 2;
		} else {
			// Not in stealth mode, so bring up the ui window
			_talkStealth = 0;
			++_talkToFlag;
			events.setCursor(WAIT);

			ui._windowBounds.top = CONTROLS_Y;
			ui._infoFlag = true;
			ui.clearInfo();
		}

		// Handle replies until there's no further linked file, 
		// or the link file isn't a reply first cnversation
		while (!_vm->shouldQuit()) {
			clearSequences();
			_scriptSelect = select;
			_speaker = _talkTo;

			Statement &statement = _statements[select];
			doScript(_statements[select]._reply);

			if (_talkToAbort)
				return;

			if (!_talkStealth)
				ui.clearWindow();

			if (statement._modified.size() > 0) {
				for (uint idx = 0; idx < statement._modified.size(); ++idx)
					_vm->setFlags(statement._modified[idx]);

				setTalkMap();
			}
			
			// Check for a linked file
			if (!statement._linkFile.empty() && !_scriptMoreFlag) {
				Common::String linkFilename = statement._linkFile;
				freeTalkVars();
				loadTalkFile(linkFilename);

				// Scan for the first valid statement in the newly loaded file
				select = -1;
				for (uint idx = 0; idx < _statements.size(); ++idx) {
					if (_statements[idx]._talkMap == 0) {
						select = idx;
						break;
					}
				}

				if (_talkToFlag == 1)
					pullSequence();

				// Set the stealth mode for the new talk file
				Statement &newStatement = _statements[select];
				_talkStealth = newStatement._statement.hasPrefix("^") ? 2 : 0;

				// If the new conversion is a reply first, then we don't need
				// to display any choices, since the reply needs to be shown
				if (!newStatement._statement.hasPrefix("*") &&
						!newStatement._statement.hasPrefix("^")) {
					clearSequences();
					pushSequence(_talkTo);
					setStillSeq(_talkTo);
					_talkIndex = select;
					ui._selector = ui._oldSelector = -1;

					if (!ui._windowOpen) {
						// Draw the talk interface on the back buffer
						drawInterface();
						displayTalk(false);
					} else {
						displayTalk(true);
					}

					byte color = ui._endKeyActive ? COMMAND_FOREGROUND : COMMAND_NULL;

					// If the window is alraedy open, simply draw. Otherwise, do it
					// to the back buffer and then summon the window
					if (ui._windowOpen) {
						screen.buttonPrint(Common::Point(119, CONTROLS_Y), color, true, "Exit");
					} else {
						screen.buttonPrint(Common::Point(119, CONTROLS_Y), color, false, "Exit");
					
						if (!ui._windowStyle) {
							screen.slamRect(Common::Rect(0, CONTROLS_Y,
								SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
						} else {
							ui.summonWindow();
						}

						ui._windowOpen = true;
					}

					// Break out of loop now that we're waiting for player input
					events.setCursor(ARROW);
					break;
				} else {
					// Add the statement into the journal and talk history
					if (_talkTo != -1 && !_talkHistory[_converseNum][select])
						journal.record(_converseNum, select, true);
					_talkHistory[_converseNum][select] = true;

				}

				ui._key = ui._oldKey = COMMANDS[TALK_MODE - 1];
				ui._temp = ui._oldTemp = 0;
				ui._menuMode = TALK_MODE;
				_talkToFlag = 2;
			} else {
				freeTalkVars();

				if (!ui._lookScriptFlag) {
					ui.banishWindow();
					ui._windowBounds.top = CONTROLS_Y1;
					ui._menuMode = STD_MODE;
				}

				break;
			}
		}
	}

	_talkStealth = 0;
	events._pressed = events._released = events._oldButtons = 0;
	events.clearKeyboard();

	screen.setDisplayBounds(savedBounds);
	_talkToAbort = abortFlag;

	// If a script was added to the script stack, restore state so that the
	// previous script can continue
	popStack();

	events.setCursor(ARROW);
}

/**
 * Main method for handling conversations when a character to talk to has been
 * selected. It will make Holmes walk to the person to talk to, draws the 
 * interface window for the conversation and passes on control to give the
 * player a list of options to make a selection from
 */
void Talk::talk(int objNum) {
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	UserInterface &ui = *_vm->_ui;
	Object &obj = scene._bgShapes[objNum];

	ui._windowBounds.top = CONTROLS_Y;
	ui._infoFlag = true;
	_speaker = 128;
	loadTalkFile(scene._bgShapes[objNum]._name);

	// Find the first statement with the correct flags
	int select = -1;
	for (uint idx = 0; idx < _statements.size(); ++idx) {
		if (_statements[idx]._talkMap == 0) {
			select = idx;
			break;
		}
	}
	if (select == -1)
		error("No entry matched all required flags");

	// See if the statement is a stealth mode reply
	Statement &statement = _statements[select];
	if (statement._statement.hasPrefix("^")) {
		clearSequences();

		// Start talk in stealth mode
		_talkStealth = 2;

		talkTo(obj._name);
	} else if (statement._statement.hasPrefix("*")) {
		// Character being spoken to will speak first
		clearSequences();
		pushSequence(_talkTo);
		setStillSeq(_talkTo);

		events.setCursor(WAIT);
		if (obj._lookPosition.y != 0)
			// Need to walk to character first
			people.walkToCoords(Common::Point(obj._lookPosition.x, obj._lookPosition.y * 100), 
				obj._lookFacing);
		events.setCursor(ARROW);

		if (_talkToAbort)
			talkTo(obj._name);
	} else {
		// Holmes will be speaking first
		clearSequences();
		pushSequence(_talkTo);
		setStillSeq(_talkTo);

		_talkToFlag = false;
		events.setCursor(WAIT);
		if (obj._lookPosition.y != 0)
			// Walk over to person to talk to
			people.walkToCoords(Common::Point(obj._lookPosition.x, obj._lookPosition.y * 100),
			obj._lookFacing);
		events.setCursor(ARROW);

		if (!_talkToAbort) {
			// See if walking over triggered a conversation
			if (_talkToFlag) {
				if (_talkToFlag == 1) {
					events.setCursor(ARROW);
					// _sequenceStack._count = 1;
					pullSequence();
				}
			} else {
				drawInterface();

				events._pressed = events._released = false;
				_talkIndex = select;
				displayTalk(false);
				ui._selector = ui._oldSelector = -1;

				if (!ui._windowStyle) {
					screen.slamRect(Common::Rect(0, CONTROLS_Y, SHERLOCK_SCREEN_WIDTH,
						SHERLOCK_SCREEN_HEIGHT));
				} else {
					ui.summonWindow();
				}

				ui._windowOpen = true;
			}

			_talkToFlag = -1;
		}
	}
}

/**
 * Clear loaded talk data
 */
void Talk::freeTalkVars() {
	_statements.clear();
}

/**
 * Opens the talk file 'talk.tlk' and searches the index for the specified
 * conversation. If found, the data for that conversation is loaded
 */
void Talk::loadTalkFile(const Common::String &filename) {
	Resources &res = *_vm->_res;
	Sound &sound = *_vm->_sound;

	// Save a copy of the talk filename
	_scriptName = filename;

	// Check for an existing person being talked to
	_talkTo = -1;
	for (int idx = 0; idx < MAX_PEOPLE; ++idx) {
		if (!scumm_strnicmp(filename.c_str(), PORTRAITS[idx], 4)) {
			_talkTo = idx;
			break;
		}
	}

	const char *chP = strchr(filename.c_str(), '.');
	Common::String talkFile = chP ? Common::String(filename.c_str(), chP) + ".tlk" :
		Common::String(filename.c_str(), filename.c_str() + 7) + ".tlk";

	// Open the talk file for reading
	Common::SeekableReadStream *talkStream = res.load(talkFile);
	_converseNum = res.resourceIndex();
	talkStream->skip(2);	// Skip talk file version num

	_statements.resize(talkStream->readByte());
	for (uint idx = 0; idx < _statements.size(); ++idx)
		_statements[idx].synchronize(*talkStream);
	
	delete talkStream;

	if (!sound._voices)
		stripVoiceCommands();
	setTalkMap();
}

/**
 * Remove any voice commands from a loaded statement list
 */
void Talk::stripVoiceCommands() {
	for (uint sIdx = 0; sIdx < _statements.size(); ++sIdx) {
		Statement &statement = _statements[sIdx];

		// Scan for an sound effect byte, which indicates to play a sound
		for (uint idx = 0; idx < statement._reply.size(); ++idx) {
			if (statement._reply[idx] == SFX_COMMAND) {
				// Replace instruction character with a space, and delete the
				// rest of the name following it
				statement._reply = Common::String(statement._reply.c_str(), 
					statement._reply.c_str() + idx) + " " +
					Common::String(statement._reply.c_str() + 9);
			}
		}

		// Ensure the last character of the reply is not a space from the prior
		// conversion loop, to avoid any issues with the space ever causing a page
		// wrap, and ending up displaying another empty page
		while (statement._reply.lastChar() == ' ')
			statement._reply.deleteLastChar();
	}
}

/**
 * Form a table of the display indexes for statements
 */
void Talk::setTalkMap() {
	int statementNum = 0;

	for (uint sIdx = 0; sIdx < _statements.size(); ++sIdx) {
		Statement &statement = _statements[sIdx];

		// Set up talk map entry for the statement
		bool valid = true;
		for (uint idx = 0; idx < statement._required.size(); ++idx) {
			if (!_vm->readFlags(statement._required[idx]))
				valid = false;
		}

		statement._talkMap = valid ? statementNum++ : -1;
	}
}

/**
 * Draws the interface for conversation display
 */
void Talk::drawInterface() {
	Screen &screen = *_vm->_screen;
	Surface &bb = *screen._backBuffer;

	bb.fillRect(Common::Rect(0, CONTROLS_Y, SHERLOCK_SCREEN_WIDTH, CONTROLS_Y1 + 10), BORDER_COLOR);
	bb.fillRect(Common::Rect(0, CONTROLS_Y + 10, 2, SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
	bb.fillRect(Common::Rect(SHERLOCK_SCREEN_WIDTH - 2, CONTROLS_Y + 10,
		SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
	bb.fillRect(Common::Rect(0, SHERLOCK_SCREEN_HEIGHT - 1, SHERLOCK_SCREEN_WIDTH - 2,
		SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
	bb.fillRect(Common::Rect(2, CONTROLS_Y + 10, SHERLOCK_SCREEN_WIDTH - 2,
		SHERLOCK_SCREEN_HEIGHT - 2), INV_BACKGROUND);

	if (_talkTo != -1) {
		screen.makeButton(Common::Rect(99, CONTROLS_Y, 139, CONTROLS_Y + 10),
			119 - screen.stringWidth("Exit") / 2, "Exit");
		screen.makeButton(Common::Rect(140, CONTROLS_Y, 180, CONTROLS_Y + 10),
			159 - screen.stringWidth("Up") / 2, "Up");
		screen.makeButton(Common::Rect(181, CONTROLS_Y, 221, CONTROLS_Y + 10),
			200 - screen.stringWidth("Down") / 2, "Down");
	} else {
		int strWidth = screen.stringWidth(PRESS_KEY_TO_CONTINUE);
		screen.makeButton(Common::Rect(46, CONTROLS_Y, 273, CONTROLS_Y + 10),
			160 - strWidth / 2, PRESS_KEY_TO_CONTINUE);
		screen.gPrint(Common::Point(160 - strWidth / 2, CONTROLS_Y), COMMAND_FOREGROUND, "P");
	}
}

/**
 * Display a list of statements in a window at the bottom of the scren that the 
 * player can select from.
 */
bool Talk::displayTalk(bool slamIt) {
	Screen &screen = *_vm->_screen;
	int yp = CONTROLS_Y + 14;
	int lineY = -1;
	_moreTalkDown = _moreTalkUp = false;
	
	for (uint idx = 0; idx < _statements.size(); ++idx) {
		_statements[idx]._talkPos.top = _statements[idx]._talkPos.bottom = -1;
	}

	if (_talkIndex) {
		for (int idx = 0; idx < _talkIndex && !_moreTalkUp; ++idx) {
			if (_statements[idx]._talkMap != -1)
				_moreTalkUp = true;
		}
	}

	// Display the up arrow and enable Up button if the first option is scrolled off-screen
	if (_moreTalkUp) {
		if (slamIt) {
			screen.print(Common::Point(5, CONTROLS_Y + 13), INV_FOREGROUND, "~");
			screen.buttonPrint(Common::Point(159, CONTROLS_Y), COMMAND_FOREGROUND, true, "Up");
		} else {
			screen.gPrint(Common::Point(5, CONTROLS_Y + 12), INV_FOREGROUND, "~");
			screen.buttonPrint(Common::Point(159, CONTROLS_Y), COMMAND_FOREGROUND, false, "Up");
		}
	} else {
		if (slamIt) {
			screen.buttonPrint(Common::Point(159, CONTROLS_Y), COMMAND_NULL, true, "Up");
			screen.vgaBar(Common::Rect(5, CONTROLS_Y + 11, 15, CONTROLS_Y + 22), INV_BACKGROUND);
		} else {
			screen.buttonPrint(Common::Point(159, CONTROLS_Y), COMMAND_NULL, false, "Up");
			screen._backBuffer1.fillRect(Common::Rect(5, CONTROLS_Y + 11, 
				15, CONTROLS_Y + 22), INV_BACKGROUND);
		}
	}

	// Loop through the statements
	bool done = false;
	for (uint idx = _talkIndex; idx < _statements.size() && !done; ++idx) {
		Statement &statement = _statements[idx];

		if (statement._talkMap != -1) {
			bool flag = _talkHistory[_converseNum][idx];
			lineY = talkLine(idx, statement._talkMap, flag ? TALK_NULL : INV_FOREGROUND, 
				yp, slamIt);
		
			if (lineY != -1) {
				statement._talkPos.top = yp;
				yp = lineY;
				statement._talkPos.bottom = yp;

				if (yp == SHERLOCK_SCREEN_HEIGHT)
					done = true;
			} else {
				done = true;
			}
		}
	}

	// Display the down arrow and enable down button if there are more statements available down off-screen
	if (lineY == -1 || lineY == SHERLOCK_SCREEN_HEIGHT) {
		_moreTalkDown = true;

		if (slamIt) {
			screen.print(Common::Point(5, 190), INV_FOREGROUND, "|");
			screen.buttonPrint(Common::Point(200, CONTROLS_Y), COMMAND_FOREGROUND, true, "Down");
		} else {
			screen.gPrint(Common::Point(5, 189), INV_FOREGROUND, "|");
			screen.buttonPrint(Common::Point(200, CONTROLS_Y), COMMAND_FOREGROUND, false, "Down");
		}
	} else {
		if (slamIt) {
			screen.buttonPrint(Common::Point(200, CONTROLS_Y), COMMAND_NULL, true, "Down");
			screen.vgaBar(Common::Rect(5, 189, 16, 199), INV_BACKGROUND);
		} else {
			screen.buttonPrint(Common::Point(200, CONTROLS_Y), COMMAND_NULL, false, "Down");
			screen._backBuffer1.fillRect(Common::Rect(5, 189, 16, 199), INV_BACKGROUND);
		}
	}

	return done;
}

/**
 * Prints a single conversation option in the interface window
 */
int Talk::talkLine(int lineNum, int stateNum, byte color, int lineY, bool slamIt) {
	Screen &screen = *_vm->_screen;
	int idx = lineNum;
	Common::String msg, number;
	bool numberFlag = false;

	// Get the statement to display as well as optional number prefix
	if (idx < 128) {
		number = Common::String::format("%d.", stateNum + 1);
		numberFlag = true;
	} else {
		idx -= 128;
	}
	msg = _statements[idx]._statement;

	// Handle potentially multiple lines needed to display entire statement
	const char *lineStartP = msg.c_str();
	int maxWidth = 298 - (numberFlag ? 18 : 0);
	for (;;) {
		// Get as much of the statement as possible will fit on the
		Common::String sLine;
		const char *lineEndP = lineStartP;
		int width = 0;
		do {
			width += screen.charWidth(*lineEndP);
		} while (*++lineEndP && width < maxWidth);

		// Check if we need to wrap the line
		if (width >= maxWidth) {
			// Work backwards to the prior word's end
			while (*--lineEndP != ' ')
				;

			sLine = Common::String(lineStartP, lineEndP++);
		} else {
			// Can display remainder of the statement on the current line
			sLine = Common::String(lineStartP);
		}


		if (lineY <= (SHERLOCK_SCREEN_HEIGHT - 10)) {
			// Need to directly display on-screen?
			if (slamIt) {
				// See if a numer prefix is needed or not
				if (numberFlag) {
					// Are we drawing the first line?
					if (lineStartP == msg.c_str()) {
						// We are, so print the number and then the text
						screen.print(Common::Point(16, lineY), color, number.c_str());
					}

					// Draw the line with an indent
					screen.print(Common::Point(30, lineY), color, sLine.c_str());
				} else {
					screen.print(Common::Point(16, lineY), color, sLine.c_str());
				}
			} else {
				if (numberFlag) {
					if (lineStartP == msg.c_str()) {
						screen.gPrint(Common::Point(16, lineY - 1), color, number.c_str());
					}

					screen.gPrint(Common::Point(30, lineY - 1), color, sLine.c_str());
				} else {
					screen.gPrint(Common::Point(16, lineY - 1), color, sLine.c_str());
				}
			}

			// Move to next line, if any
			lineY += 9;
			lineStartP = lineEndP;
			
			if (!*lineEndP)
				break;
		} else {
			// We're close to the bottom of the screen, so stop display
			lineY = -1;
			break;
		}
	}

	if (lineY == -1 && lineStartP != msg.c_str())
		lineY = SHERLOCK_SCREEN_HEIGHT;

	// Return the Y position of the next line to follow this one
	return lineY;
}

/**
 * Clears the stack of pending object sequences associated with speakers in the scene
 */
void Talk::clearSequences() {
	_sequenceStack.clear();
}

/**
 * Pulls a background object sequence from the sequence stack and restore's the
 * object's sequence
 */
void Talk::pullSequence() {
	Scene &scene = *_vm->_scene;

	if (_sequenceStack.empty())
		return;

	SequenceEntry seq = _sequenceStack.pop();
	if (seq._objNum != -1) {
		Object &obj = scene._bgShapes[seq._objNum];

		if (obj._seqSize < MAX_TALK_SEQUENCES) {
			warning("Tried to restore too few frames");
		} else {
			for (int idx = 0; idx < MAX_TALK_SEQUENCES; ++idx)
				obj._sequences[idx] = seq._sequences[idx];

			obj._frameNumber = seq._frameNumber;
			obj._seqTo = seq._seqTo;
		}
	}
}

/**
 * Push the sequence of a background object that's an NPC that needs to be
 * saved onto the sequence stack.
 */
void Talk::pushSequence(int speaker) {
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;

	// Only proceed if a speaker is specified
	if (speaker == -1)
		return;

	SequenceEntry seqEntry;
	if (!speaker) {
		seqEntry._objNum = -1;
	} else {
		seqEntry._objNum = people.findSpeaker(speaker);

		if (seqEntry._objNum != -1) {
			Object &obj = scene._bgShapes[seqEntry._objNum];
			for (uint idx = 0; idx < MAX_TALK_SEQUENCES; ++idx)
				seqEntry._sequences.push_back(obj._sequences[idx]);

			seqEntry._frameNumber = obj._frameNumber;
			seqEntry._seqTo = obj._seqTo;
		}
	}
	
	_sequenceStack.push(seqEntry);
	if (_scriptStack.size() >= 5)
		error("script stack overflow");
}

/**
 * Change the sequence of the scene background object associated with the current speaker.
 */
void Talk::setSequence(int speaker) {
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	
	// If no speaker is specified, then nothing needs to be done
	if (speaker != -1)
		return;

	if (speaker) {
		int objNum = people.findSpeaker(speaker);
		if (objNum != -1) {
			Object &obj = scene._bgShapes[objNum];

			if (obj._seqSize < MAX_TALK_SEQUENCES) {
				warning("Tried to copy too many talk frames");
			} else {
				for (int idx = 0; idx < MAX_TALK_SEQUENCES; ++idx) {
					obj._sequences[idx] = TALK_SEQUENCES[speaker][idx];
					if (idx > 0 && !TALK_SEQUENCES[speaker][idx] && !TALK_SEQUENCES[speaker][idx - 1])
						return;

					obj._frameNumber = 0;
					obj._sequenceNumber = 0;
				}
			}
		}
	}
}

/**
 * Change the sequence of a background object corresponding to a given speaker.
 * The new sequence will display the character as "listening"
 */
void Talk::setStillSeq(int speaker) {
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;

	// Don't bother doing anything if no specific speaker is specified
	if (speaker == -1)
		return;

	if (speaker) {
		int objNum = people.findSpeaker(speaker);
		if (objNum != -1) {
			Object &obj = scene._bgShapes[objNum];
			
			if (obj._seqSize < MAX_TALK_SEQUENCES) {
				warning("Tried to copy too few still frames");
			} else {
				for (uint idx = 0; idx < MAX_TALK_SEQUENCES; ++idx) {
					obj._sequences[idx] = STILL_SEQUENCES[speaker][idx];
					if (idx > 0 && !TALK_SEQUENCES[speaker][idx] && !TALK_SEQUENCES[speaker][idx - 1])
						break;
				}

				obj._frameNumber = 0;
				obj._seqTo = 0;
			}
		}
	}
}

/**
 * Parses a reply for control codes and display text. The found text is printed within
 * the text window, handles delays, animations, and animating portraits.
 */
void Talk::doScript(const Common::String &script) {
	Animation &anim = *_vm->_animation;
	Events &events = *_vm->_events;
	Inventory &inv = *_vm->_inventory;
	Map &map = *_vm->_map;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	UserInterface &ui = *_vm->_ui;
	int wait = 0;
	bool pauseFlag = false;
	bool endStr = false;
	int yp = CONTROLS_Y + 12;
	int charCount = 0;
	int line = 0;
	bool noTextYet = true;
	bool openTalkWindow = false;
	int obj;
	int seqCount;

	_saveSeqNum = 0;

	const char *str = script.c_str();
	if (_scriptMoreFlag) {
		_scriptMoreFlag = 0;
		str = script.c_str() + _scriptSaveIndex;
	}

	// Check if the script begins with a Stealh Mode Active command
	if ((byte)str[0] == STEALTH_MODE_ACTIVE || _talkStealth) {
		_talkStealth = 2;
		_speaker |= 128;
	} else {
		pushSequence(_speaker);
		ui.clearWindow();

		// Need to switch speakers?
		if ((byte)str[0] == SWITCH_SPEAKER) {
			_speaker = str[1] - 1;
			str += 2;
			pullSequence();
			pushSequence(_speaker);
			setSequence(_speaker);
		} else {
			setSequence(_speaker);
		}

		// Assign portrait location?
		if ((byte)str[0] == ASSIGN_PORTRAIT_LOCATION) {
			switch (str[1] & 15) {
			case 1:
				people._portraitSide = 20;
				break;
			case 2:
				people._portraitSide = 220;
				break;
			case 3:
				people._portraitSide = 120;
				break;
			default:
				break;

			}

			if (str[1] > 15)
				people._speakerFlip = true;
			str += 2;
		}

		// Remove portrait?
		if ((byte)str[0] == REMOVE_PORTRAIT) {
			_speaker = 255;
		} else {
			// Nope, so set the first speaker
			people.setTalking(_speaker);
		}
	}

	do {
		Common::String tempString;
		wait = 0;

		byte c = (byte)str[0];
		if (!c) {
			endStr = true;
		} else if (c == '{') {
			// Start of comment, so skip over it
			while (*str++ != '}')
				;
		} else if (c >= 128) {
			// Handle control code
			switch (c) {
			case SWITCH_SPEAKER:
				// Save the current point in the script, since it might be intterupted by
				// doing bg anims in the next call, so we need to know where to return to
				_scriptCurrentIndex = str - script.c_str();

				if (!(_speaker & 128))
					people.clearTalking();
				if (_talkToAbort)
					return;

				ui.clearWindow();
				yp = CONTROLS_Y + 12;
				charCount = line = 0;

				_speaker = *++str - 1;
				people.setTalking(_speaker);
				pullSequence();
				pushSequence(_speaker);
				setSequence(_speaker);
				break;

			case RUN_CANIMATION:
				// Save the current point in the script, since it might be intterupted by
				// doing bg anims in the next call, so we need to know where to return to
				++str;
				_scriptCurrentIndex = (str + 1) - script.c_str();
				scene.startCAnim(((byte)str[0] - 1) & 127, ((byte)str[0] & 128) ? -1 : 1);
				if (_talkToAbort)
					return;

				// Check if next character is changing side or changing portrait
				if (charCount && ((byte)str[1] == SWITCH_SPEAKER || (byte)str[1] == ASSIGN_PORTRAIT_LOCATION))
					wait = 1;
				break;

			case ASSIGN_PORTRAIT_LOCATION:
				++str;
				switch (str[0] & 15) {
				case 1:
					people._portraitSide = 20;
					break;
				case 2:
					people._portraitSide = 220;
					break;
				case 3:
					people._portraitSide = 120;
					break;
				default:
					break;
				}

				if (str[0] > 15)
					people._speakerFlip = true;
				break;

			case PAUSE:
				// Pause
				charCount = *++str;
				wait = pauseFlag = true;
				break;

			case REMOVE_PORTRAIT:
				// Save the current point in the script, since it might be intterupted by
				// doing bg anims in the next call, so we need to know where to return to
				_scriptCurrentIndex = str - script.c_str();

				if (_speaker >= 0 && _speaker < 128)
					people.clearTalking();
				pullSequence();
				if (_talkToAbort)
					return;

				_speaker |= 128;
				break;

			case CLEAR_WINDOW:
				ui.clearWindow();
				yp = CONTROLS_Y + 12;
				charCount = line = 0;
				break;

			case ADJUST_OBJ_SEQUENCE:
				// Get the name of the object to adjust
				++str;
				for (int idx = 0; idx < (str[0] & 127); ++idx)
					tempString += str[idx + 2];

				// Scan for object
				obj = -1;
				for (uint idx = 0; idx < scene._bgShapes.size(); ++idx) {
					if (scumm_stricmp(tempString.c_str(), scene._bgShapes[idx]._name.c_str()) == 0)
						obj = idx;
				}
				if (obj == -1)
					error("Could not find object %s to change", tempString.c_str());

				// Should the script be overwritten?
				if (str[0] > 128) {
					// Save the current sequence
					_savedSequences.push(SequenceEntry());
					SequenceEntry &seqEntry = _savedSequences.top();
					seqEntry._objNum = obj;
					seqEntry._seqTo = scene._bgShapes[obj]._seqTo;
					for (uint idx = 0; idx < scene._bgShapes[obj]._seqSize; ++idx)
						seqEntry._sequences.push_back(scene._bgShapes[obj]._sequences[idx]);
				}

				// Get number of bytes to change
				seqCount = str[1];
				str += (str[0] & 127) + 2;

				// Copy in the new sequence
				for (int idx = 0; idx < seqCount; ++idx, ++str)
					scene._bgShapes[obj]._sequences[idx] = str[0] - 1;

				// Reset object back to beginning of new sequence
				scene._bgShapes[obj]._frameNumber = 0;
				continue;

			case WALK_TO_COORDS:
				// Save the current point in the script, since it might be intterupted by
				// doing bg anims in the next call, so we need to know where to return to
				++str;
				_scriptCurrentIndex = str - script.c_str();

				people.walkToCoords(Common::Point((((byte)str[0] - 1) * 256 + (byte)str[1] - 1) * 100, 
					(byte)str[2] * 100), str[3] - 1);
				if (_talkToAbort)
					return;

				str += 3;
				break;

			case PAUSE_WITHOUT_CONTROL:
				// Save the current point in the script, since it might be intterupted by
				// doing bg anims in the next call, so we need to know where to return to
				++str;
				_scriptCurrentIndex = str - script.c_str();

				for (int idx = 0; idx < (str[0] - 1); ++idx) {
					scene.doBgAnim();
					if (_talkToAbort)
						return;

					// Check for button press
					events.pollEvents();
					events.setButtonState();
				}
				break;

			case BANISH_WINDOW:
				// Save the current point in the script, since it might be intterupted by
				// doing bg anims in the next call, so we need to know where to return to
				_scriptCurrentIndex = str - script.c_str();

				if (!(_speaker & 128))
					people.clearTalking();
				pullSequence();

				if (_talkToAbort)
					return;

				_speaker |= 128;
				ui.banishWindow();
				ui._menuMode = TALK_MODE;
				noTextYet = true;
				break;

			case SUMMON_WINDOW:
				drawInterface();
				events._pressed = events._released = false;
				events.clearKeyboard();
				noTextYet = false;

				if (_speaker != -1) {
					screen.buttonPrint(Common::Point(119, CONTROLS_Y), COMMAND_NULL, false, "Exit");
					screen.buttonPrint(Common::Point(159, CONTROLS_Y), COMMAND_NULL, false, "Up");
					screen.buttonPrint(Common::Point(200, CONTROLS_Y), COMMAND_NULL, false, "Down");
				}
				break;

			case SET_FLAG: {
				++str;
				int flag1 = ((byte)str[0] - 1) * 256 + (byte)str[1] - 1 - (str[1] == 1 ? 1 : 0);
				int flag = (flag1 & 0x3fff) * (flag1 >= 0x4000 ? -1 : 1);
				_vm->setFlags(flag);
				++str;
				break;
			}
				
			case SFX_COMMAND:
				++str;
				if (sound._voices) {
					for (int idx = 0; idx < 8 && str[idx] != '~'; ++idx)
						tempString += str[idx];
					sound.playSound(tempString);

					// Set voices to wait for more
					sound._voices = 2;
					sound._speechOn = (*sound._soundIsOn);
				}

				wait = 1;
				str += 7;
				break;

			case TOGGLE_OBJECT:
				++str;
				for (int idx = 0; idx < str[0]; ++idx)
					tempString += str[idx + 1];

				scene.toggleObject(tempString);
				str += str[0];
				break;

			case STEALTH_MODE_ACTIVE:
				_talkStealth = 2;
				break;

			case IF_STATEMENT: {
				++str;
				int flag = ((byte)str[0] - 1) * 256 + (byte)str[1] - 1 - (str[1] == 1 ? 1 : 0);
				++str;
				wait = 0;
				
				bool result = flag < 0x8000;
				if (_vm->readFlags(flag & 0x7fff) != result) {
					do {
						++str;
					} while (str[0] && str[0] != ELSE_STATEMENT && str[0] != END_IF_STATEMENT);

					if (!str[0])
						endStr = true;
				}
				break;
			}

			case ELSE_STATEMENT:
				// If this is encountered here, it means that a preceeding IF statement was found,
				// and evaluated to true. Now all the statements for the true block are finished,
				// so skip over the block of code that would have executed if the result was false
				wait = 0;
				do {
					++str;
				} while (str[0] && str[0] != END_IF_STATEMENT);
				break;

			case STEALTH_MODE_DEACTIVATE:
				_talkStealth = 0;
				events.clearKeyboard();
				break;

			case TURN_HOLMES_OFF:
				people._holmesOn = false;
				break;

			case TURN_HOLMES_ON:
				people._holmesOn = true;
				break;

			case GOTO_SCENE:
				scene._goToScene = str[1] - 1;

				if (scene._goToScene != 100) {
					// Not going to the map overview
					map._oldCharPoint = scene._goToScene;
					map._overPos.x = map[scene._goToScene].x * 100 - 600;
					map._overPos.y = map[scene._goToScene].y * 100 + 900;

					// Run a canimation?
					if (str[2] > 100) {
						people._hSavedFacing = (byte)str[2];
						people._hSavedPos = Common::Point(160, 100);
					}
				}
				str += 6;

				_scriptMoreFlag = (scene._goToScene == 100) ? 2 : 1;
				_scriptSaveIndex = str - script.c_str();
				endStr = true;
				wait = 0;
				break;

			case PLAY_PROLOGUE:
				++str;
				for (int idx = 0; idx < 8 && str[idx] != '~'; ++idx)
					tempString += str[idx];

				anim.playPrologue(tempString, 1, 3, true, 4);
				break;

			case ADD_ITEM_TO_INVENTORY:
				++str;
				for (int idx = 0; idx < str[0]; ++idx)
					tempString += str[idx + 1];
				str += str[0];

				inv.putNameInInventory(tempString);
				break;

			case SET_OBJECT: {
				++str;
				for (int idx = 0; idx < (str[0] & 127); ++idx)
					tempString += str[idx + 1];
				str += str[0];

				// Set comparison state according to if we want to hide or unhide
				bool state = ((byte)str[0] >= 128);

				for (uint idx = 0; idx < scene._bgShapes.size(); ++idx) {
					Object &obj = scene._bgShapes[idx];
					if (scumm_stricmp(tempString.c_str(), obj._name.c_str()) == 0) {
						// Only toggle the object if it's not in the desired state already
						if ((obj._type == HIDDEN && state) || (obj._type != HIDDEN && !state))
							obj.toggleHidden();
					}
				}
				break;
			}

			case CALL_TALK_FILE:
				++str;
				for (int idx = 0; idx < 8 && str[idx] != '~'; ++idx)
					tempString += str[idx];
				str += 8;

				_scriptCurrentIndex = str - script.c_str();

				// Save the current script position and new talk file
				if (_scriptStack.size() < 9) {
					ScriptStackEntry rec1;
					rec1._name = _scriptName;
					rec1._currentIndex = _scriptCurrentIndex;
					rec1._select = _scriptSelect;
					_scriptStack.push(rec1);

					// Push the new talk file onto the stack
					ScriptStackEntry rec2;
					rec2._name = tempString;
					rec2._currentIndex = 0;
					rec2._select = 100;
					_scriptStack.push(rec2);
				} else {
					error("Script stack overflow");
				}

				_scriptMoreFlag = 1;
				endStr = true;
				wait = 0;
				break;

			case MOVE_MOUSE:
				// Save the current point in the script, since it might be intterupted by
				// doing bg anims in the next call, so we need to know where to return to
				++str;
				_scriptCurrentIndex = str - script.c_str();
				events.moveMouse(Common::Point(((byte)str[0] - 1) * 256 + (byte)str[1] - 1, str[2]));
				if (_talkToAbort)
					return;
				str += 3;
				break;

			case DISPLAY_INFO_LINE:
				++str;
				for (int idx = 0; idx < str[0]; ++idx)
					tempString += str[idx + 1];
				str += str[0];

				screen.print(Common::Point(0, INFO_LINE + 1), INFO_FOREGROUND, tempString.c_str());
				break;

			case CLEAR_INFO_LINE:
				ui._infoFlag = true;
				ui.clearInfo();
				break;

			case WALK_TO_CANIMATION: {
				++str;
				CAnim &anim = scene._cAnim[str[0] - 1];

				// Save the current point in the script, since it might be intterupted by
				// doing bg anims in the next call, so we need to know where to return to
				_scriptCurrentIndex = (str + 1) - script.c_str();

				people.walkToCoords(anim._goto, anim._gotoDir);
				if (_talkToAbort)
					return;
				break;
			}

			case REMOVE_ITEM_FROM_INVENTORY:
				++str;
				for (int idx = 0; idx < str[0]; ++idx)
					tempString += str[idx + 1];
				str += str[0];

				inv.deleteItemFromInventory(tempString);
				break;

			case ENABLE_END_KEY:
				ui._endKeyActive = true;
				break;

			case DISABLE_END_KEY:
				ui._endKeyActive = false;
				break;

			default:
				break;
			}

			++str;
		} else {
			// If the window isn't yet open, draw the window before printing starts
			if (!ui._windowOpen && noTextYet) {
				noTextYet = false;
				drawInterface();

				if (_talkTo != -1) {
					screen.buttonPrint(Common::Point(119, CONTROLS_Y), COMMAND_NULL, false, "Exit");
					screen.buttonPrint(Common::Point(159, CONTROLS_Y), COMMAND_NULL, false, "Up");
					screen.buttonPrint(Common::Point(200, CONTROLS_Y), COMMAND_NULL, false, "Down");
				}
			}

			// If it's the first line, display the speaker
			if (!line && _speaker >= 0 && _speaker < MAX_PEOPLE) {
				// If the window is open, display the name directly on-screen.
				// Otherwise, simply draw it on the back buffer
				if (ui._windowOpen) {
					screen.print(Common::Point(16, yp), TALK_FOREGROUND, NAMES[_speaker & 127]);
				} else {
					screen.gPrint(Common::Point(16, yp - 1), TALK_FOREGROUND, NAMES[_speaker & 127]);
					openTalkWindow = true;
				}

				yp += 9;
			}

			// Find amound of text that will fit on the line
			int width = 0, idx = 0;
			do {
				width += screen.charWidth(str[idx]);
				++idx;
				++charCount;
			} while (width < 298 && str[idx] && str[idx] != '{' && (byte)str[idx] < 128);

			if (str[idx] || width >= 298) {
				if ((byte)str[idx] < 128 && str[idx] != '{') {
					--idx;
					--charCount;
				}
			} else {
				endStr = true;
			}

			// If word wrap is needed, find the start of the current word
			if (width >= 298) {
				while (str[idx] != ' ') {
					--idx;
					--charCount;
				}
			}

			// Print the line
			Common::String lineStr(str, str + idx);

			// If the speaker indicates a description file, print it in yellow
			if (_speaker != -1) {
				if (ui._windowOpen) {
					screen.print(Common::Point(16, yp), INV_FOREGROUND, lineStr.c_str());
				} else {
					screen.gPrint(Common::Point(16, yp - 1), INV_FOREGROUND, lineStr.c_str());
					openTalkWindow = true;
				}
			} else {
				if (ui._windowOpen) {
					screen.print(Common::Point(16, yp), COMMAND_FOREGROUND, lineStr.c_str());
				} else {
					screen.gPrint(Common::Point(16, yp - 1), COMMAND_FOREGROUND, lineStr.c_str());
					openTalkWindow = true;
				}
			}

			// Move to end of displayed line
			str += idx;

			// If line wrap occurred, then move to after the separating space between the words
			if ((byte)str[0] < 128 && str[0] != '{')
				++str;

			yp += 9;
			++line;

			// Certain different conditions require a wait
			if ((line == 4 && str[0] != SFX_COMMAND && str[0] != PAUSE && _speaker != -1) ||
					(line == 5 && str[0] != PAUSE && _speaker != -1) ||
					endStr) {
				wait = 1;
			}

			switch ((byte)str[0]) {
			case SWITCH_SPEAKER:
			case ASSIGN_PORTRAIT_LOCATION:
			case BANISH_WINDOW:
			case IF_STATEMENT:
			case ELSE_STATEMENT:
			case END_IF_STATEMENT:
			case GOTO_SCENE:
			case CALL_TALK_FILE:
				wait = 1;
				break;
			default:
				break;
			}
		}

		// Open window if it wasn't already open, and text has already been printed
		if ((openTalkWindow && wait) || (openTalkWindow && (byte)str[0] >= 128 && (byte)str[0] != COMMAND_161)) {
			if (!ui._windowStyle) {
				screen.slamRect(Common::Rect(0, CONTROLS_Y, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
			} else {
				ui.summonWindow();
			}

			ui._windowOpen = true;
			openTalkWindow = false;
		}

		if (wait) {
			// Save the current point in the script, since it might be intterupted by
			// doing bg anims in the next call, so we need to know where to return to
			_scriptCurrentIndex = str - script.c_str();

			// Handling pausing
			if (!pauseFlag && charCount < 160)
				charCount = 160;

			wait = waitForMore(charCount);
			if (wait == -1)
				endStr = true;

			// If a key was pressed to finish the window, see if further voice files should be skipped
			if (wait >= 0 && wait < 254) {
				if ((byte)str[0] == SFX_COMMAND)
					str += 9;
			}

			// Clear the window unless the wait was due to a PAUSE command
			if (!pauseFlag && wait != -1 && (byte)str[0] != SFX_COMMAND) {
				if (!_talkStealth)
					ui.clearWindow();
				yp = CONTROLS_Y + 12;
				charCount = line = 0;
			}

			pauseFlag = false;
		}
	} while (!_vm->shouldQuit() && !endStr);

	if (wait != -1) {
		for (int ssIndex = 0; ssIndex < (int)_savedSequences.size(); ++ssIndex) {
			SequenceEntry &seq = _savedSequences[ssIndex];
			Object &obj = scene._bgShapes[seq._objNum];

			for (uint idx = 0; idx < seq._sequences.size(); ++idx)
				obj._sequences[idx] = seq._sequences[idx];
			obj._frameNumber = seq._frameNumber;
			obj._seqTo = seq._seqTo;
		}

		pullSequence();
		if (_speaker >= 0 && _speaker < 128)
			people.clearTalking();
	}
}

/**
 * When the talk window has been displayed, waits a period of time proportional to
 * the amount of text that's been displayed
 */
int Talk::waitForMore(int delay) {
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Sound &sound = *_vm->_sound;
	UserInterface &ui = *_vm->_ui;
	CursorId oldCursor = events.getCursor();
	int key2 = 254;

	// Unless we're in stealth mode, show the appropriate cursor
	if (!_talkStealth) {
		events.setCursor(ui._lookScriptFlag ? MAGNIFY : ARROW);
	}

	do {
		if (sound._speechOn && !*sound._soundIsOn)
			people._portrait._frameNumber = -1;

		scene.doBgAnim();

		// If talkTo call was done via doBgAnim, abort out of talk quietly
		if (_talkToAbort) {
			key2 = -1;
			events._released = true;
		} else {
			// See if there's been a button press
			events.setButtonState();

			if (events.kbHit()) {
				Common::KeyState keyState = events.getKey();
				if (keyState.keycode >= 32 && keyState.keycode < 128)
					key2 = keyState.keycode;
			}

			if (_talkStealth) {
				key2 = 254;
				events._released = false;
			}
		}

		// Count down the delay
		if ((delay > 0 && !ui._invLookFlag && !ui._lookScriptFlag) || _talkStealth)
			--delay;

		// If there are voices playing, reset delay so that they keep playing
		if (sound._voices == 2 && *sound._soundIsOn)
			delay = 0;
	} while (!_vm->shouldQuit() && key2 == 254 && (delay || (sound._voices == 2 && *sound._soundIsOn))
		&& !events._released && !events._rightReleased);

	// If voices was set 2 to indicate a voice file was place, then reset it back to 1
	if (sound._voices == 2)
		sound._voices = 1;

	if (delay > 0 && sound._diskSoundPlaying)
		sound.stopSndFuncPtr(0, 0);

	// Adjust _talkStealth mode:
	// mode 1 - It was by a pause without stealth being on before the pause, so reset back to 0
	// mode 3 - It was set by a pause with stealth being on before the pause, to set it to active
	// mode 0/2 (Inactive/active) No change
	switch (_talkStealth) {
	case 1:
		_talkStealth = 0;
		break;
	case 2:
		_talkStealth = 2;
		break;
	default:
		break;
	}

	sound._speechOn = false;
	events.setCursor(_talkToAbort ? ARROW : oldCursor);
	events._pressed = events._released = false;

	return key2;
}

void Talk::popStack() {
	if (!_scriptStack.empty()) {
		ScriptStackEntry scriptEntry = _scriptStack.pop();
		_scriptName = scriptEntry._name;
		_scriptSaveIndex = scriptEntry._currentIndex;
		_scriptSelect = scriptEntry._select;
		_scriptMoreFlag = 1;
	}
}

/**
 * Synchronize the data for a savegame
 */
void Talk::synchronize(Common::Serializer &s) {
	for (int idx = 0; idx < MAX_TALK_FILES; ++idx) {
		TalkHistoryEntry &he = _talkHistory[idx];
		
		for (int flag = 0; flag < 16; ++flag)
			s.syncAsByte(he._data[flag]);
	}
}

} // End of namespace Sherlock
