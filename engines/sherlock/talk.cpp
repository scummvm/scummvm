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

namespace Sherlock {

#define SFX_COMMAND 140

/*----------------------------------------------------------------*/

/**
 * Load the data for a single statement within a talk file
 */
void Statement::synchronize(Common::SeekableReadStream &s) {
	int length;

	length = s.readUint16LE();
	for (int idx = 0; idx < length; ++idx)
		_statement += (char)s.readByte();

	length = s.readUint16LE();
	for (int idx = 0; idx < length; ++idx)
		_reply += (char)s.readByte();

	length = s.readUint16LE();
	for (int idx = 0; idx < length; ++idx)
		_linkFile += (char)s.readByte();

	length = s.readUint16LE();
	for (int idx = 0; idx < length; ++idx)
		_voiceFile += (char)s.readByte();

	_required.resize(s.readByte());
	_modified.resize(s.readByte());

	// Read in flag required/modified data
	for (uint idx = 0; idx < _required.size(); ++idx)
		_required[idx] = s.readUint16LE();
	for (uint idx = 0; idx < _modified.size(); ++idx)
		_modified[idx] = s.readUint16LE();

	_portraitSide = s.readByte();
	_quotient = s.readUint16LE();
}

/*----------------------------------------------------------------*/

TalkHistoryEntry::TalkHistoryEntry() {
	Common::fill(&_data[0], &_data[16], false);
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
	Scripts &scripts = *_vm->_scripts;
	Talk &talk = *_vm->_talk;
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
		if (!scripts._scriptMoreFlag) {
			scripts._scriptName = filename;
			scripts._scriptSaveIndex = 0;

			// Flag the selection, since we don't yet know which statement yet
			scripts._scriptSelect = 100;
			scripts._scriptMoreFlag = 3;
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

	if (talk._talkToAbort)
		return;

	talk.freeTalkVars();

	// If any sequences have changed in the prior talk file, restore them
	if (_savedSequences.size() > 0) {
		for (uint idx = 0; idx < _savedSequences.size(); ++idx) {
			SavedSequence &ss = _savedSequences[idx];
			for (uint idx2 = 0; idx2 < _savedSequences.size(); ++idx2)
				scene._bgShapes[ss._objNum]._sequences[idx2] = ss._sequences[idx2];
		
			// Reset the object's frame to the beginning of the sequence
			scene._bgShapes[ss._objNum]._frameNumber = 0;
		}
	}

	while (_sequenceStack.empty())
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
				clearTalking();
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

	if (scripts._scriptMoreFlag && _scriptSelect != 0)
		select = _scriptSelect;

	if (select == -1)
		error("Couldn't find statement to display");

	// Add the statement into the journal and talk history
	if (_talkTo != -1 && !_talkHistory[_converseNum][select])
		journal.record(_converseNum | 2048, select);
	_talkHistory[_converseNum][select] = true;

	// Check if the talk file is meant to be a non-seen comment
	if (filename[7] != '*') {
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
		for (;;) {
			_sequenceStack.clear();
			_scriptSelect = select;
			_speaker = _talkTo;

			Statement &statement = _statements[select];
			scripts.doScript(_statements[select]._reply);

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
			if (!statement._linkFile.empty() && !scripts._scriptMoreFlag) {
				freeTalkVars();
				loadTalkFile(statement._linkFile);

				// Scan for the first valid statement in the newly loaded file
				select = -1;
				for (uint idx = 0; idx < _statements.size(); ++idx) {
					if (_statements[idx]._talkMap == 0) {
						select = idx;
						break;
					}
				}

				if (_talkToFlag == 1)
					scripts.pullSeq();

				// Set the stealth mode for the new talk file
				Statement &newStatement = _statements[select];
				_talkStealth = newStatement._statement.hasPrefix("^") ? 2 : 0;

				// If the new conversion is a reply first, then we don't need
				// to display any choices, since the reply needs to be shown
				if (!newStatement._statement.hasPrefix("*") &&
						!newStatement._statement.hasPrefix("^")) {
					_sequenceStack.clear();
					scripts.pushSeq(_talkTo);
					scripts.setStillSeq(_talkTo);
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
						journal.record(_converseNum | 2048, select);
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
	if (!scripts._scriptStack.empty()) {
		scripts.popStack();
	}

	events.setCursor(ARROW);
}

void Talk::talk(int objNum) {
	// TODO
}

/**
 * Clear loaded talk data
 */
void Talk::freeTalkVars() {
	_statements.clear();
}

void Talk::pullSequence() {
	// TODO
}

/**
 * Opens the talk file 'talk.tlk' and searches the index for the specified
 * conversation. If found, the data for that conversation is loaded
 */
void Talk::loadTalkFile(const Common::String &filename) {
	People &people = *_vm->_people;
	Resources &res = *_vm->_res;
	Sound &sound = *_vm->_sound;

	// Check for an existing person being talked to
	_talkTo = -1;
	for (int idx = 0; idx < MAX_PEOPLE; ++idx) {
		if (scumm_strnicmp(filename.c_str(), people[(PeopleId)idx]._portrait.c_str(), 4)) {
			_talkTo = idx;
			break;
		}
	}

	const char *chP = strchr(filename.c_str(), '.');
	Common::String talkFile = !chP ? filename + ".tlk" :
		Common::String(filename.c_str(), chP) + ".tlk";

	// Open the talk file for reading
	Common::SeekableReadStream *talkStream = res.load(talkFile);
	talkStream->skip(2);	// Skip talk file version num

	_statements.resize(talkStream->readByte());
	for (uint idx = 0; idx < _statements.size(); ++idx)
		_statements[idx].synchronize(*talkStream);
	
	delete talkStream;

	if (!sound._voicesOn)
		stripVoiceCommands();
	setTalkMap();
}

void Talk::clearTalking() {
	// TODO
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

void Talk::drawInterface() {
	// TODO
}

void Talk::displayTalk(bool slamIt) {
	// TODO
}

} // End of namespace Sherlock
