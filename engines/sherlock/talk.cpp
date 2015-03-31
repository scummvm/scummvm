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

/**
 * Load the data for a single statement within a talk file
 */
void Statement::synchronize(Common::SeekableReadStream &s, bool voices) {
	int length;

	length = s.readUint16LE();
	for (int idx = 0; idx < length; ++idx)
		_statement += (char)s.readByte();

	length = s.readUint16LE();
	for (int idx = 0; idx < length; ++idx)
		_reply += (char)s.readByte();

	// If we don't have digital sound, we'll need to strip out voice commands from reply
	if (!voices) {
		// Scan for a 140 byte, which indicates playing a sound
		for (uint idx = 0; idx < _reply.size(); ++idx) {
			if (_reply[idx] == 140) {
				// Replace instruction character with a space, and delete the
				// rest of the name following it
				_reply = Common::String(_reply.c_str(), _reply.c_str() + idx) + " " +
					Common::String(_reply.c_str() + 9);
			}
		}

		// Ensure the last character of the reply is not a space from the prior
		// conversion loop, to avoid any issues with the space ever causing a page
		// wrap, and ending up displaying another empty page
		while (_reply.lastChar() == ' ')
			_reply.deleteLastChar();
	}

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

Talk::Talk(SherlockEngine *vm): _vm(vm) {
	_talkCounter = 0;
	_talkToAbort = false;
	_saveSeqNum = 0;
	_speaker = 0;
	_talkIndex = 0;
	_talkTo = 0;
}

/**
 * Called when either an NPC initiates a conversation or for inventory item
 * descriptions. It opens up a description window similar to how 'talk' does,
 * but shows a 'reply' directly instead of waiting for a statement option.
 */
void Talk::talkTo(const Common::String &filename) {
	Events &events = *_vm->_events;
	Inventory &inv = *_vm->_inventory;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	Scripts &scripts = *_vm->_scripts;
	Talk &talk = *_vm->_talk;
	UserInterface &ui = *_vm->_ui;
	Common::Rect savedBounds = screen.getDisplayBounds();

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
			scripts._abortFlag = true;

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
			scripts._abortFlag = true;
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
			scripts._abortFlag = true;
			break;

		case SETUP_MODE:
			ui.banishWindow(true);
			ui._windowBounds.top = CONTROLS_Y1;
			ui._temp = ui._oldTemp = ui._lookHelp = ui._invLookFlag = false;
			ui._menuMode = STD_MODE;
			events._pressed = events._released = events._oldButtons = 0;
			scripts._abortFlag = true;
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
		/*
		if (!_talkMap[idx])
			select = _talkIndex = idx;
		*/
	}

	// TODOa
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
		_statements[idx].synchronize(*talkStream, sound._voicesOn);
	
	delete talkStream;
	setTalkMap();
}

void Talk::clearTalking() {
	// TODO
}

/**
 * Form a translate table from the loaded statements from a talk file
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


} // End of namespace Sherlock
