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

#include "sherlock/talk.h"
#include "sherlock/sherlock.h"
#include "sherlock/screen.h"
#include "sherlock/scalpel/scalpel.h"
#include "sherlock/scalpel/scalpel_people.h"
#include "sherlock/scalpel/scalpel_talk.h"
#include "sherlock/scalpel/scalpel_user_interface.h"
#include "sherlock/tattoo/tattoo.h"
#include "sherlock/tattoo/tattoo_fixed_text.h"
#include "sherlock/tattoo/tattoo_people.h"
#include "sherlock/tattoo/tattoo_scene.h"
#include "sherlock/tattoo/tattoo_talk.h"
#include "sherlock/tattoo/tattoo_user_interface.h"
#include "common/config-manager.h"

#ifdef USE_TTS
#include "common/text-to-speech.h"
#endif

namespace Sherlock {

SequenceEntry::SequenceEntry() {
	_objNum = 0;
	_obj = nullptr;
	_seqStack = 0;
	_seqTo = 0;
	_sequenceNumber = _frameNumber = 0;
	_seqCounter = _seqCounter2 = 0;
}

/*----------------------------------------------------------------*/

void Statement::load(Common::SeekableReadStream &s, bool isRoseTattoo) {
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
	_journal = isRoseTattoo ? s.readByte() : 0;
}

/*----------------------------------------------------------------*/

TalkHistoryEntry::TalkHistoryEntry() {
	Common::fill(&_data[0], &_data[16], false);
}

/*----------------------------------------------------------------*/

Talk *Talk::init(SherlockEngine *vm) {
	if (vm->getGameID() == GType_SerratedScalpel)
		return new Scalpel::ScalpelTalk(vm);
	else
		return new Tattoo::TattooTalk(vm);
}

Talk::Talk(SherlockEngine *vm) : _vm(vm) {
	_talkCounter = 0;
	_talkToAbort = false;
	_openTalkWindow = false;
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
	_opcodes = nullptr;
	_opcodeTable = nullptr;
	_3doSpeechIndex = -1;

	_charCount = 0;
	_line = 0;
	_yp = 0;
	_wait = 0;
	_pauseFlag = false;
	_seqCount = 0;
	_scriptStart = _scriptEnd = nullptr;
	_endStr = _noTextYet = false;

	_talkHistory.resize(IS_ROSE_TATTOO ? 1500 : 500);
}

void Talk::talkTo(const Common::String filename) {
	Events &events = *_vm->_events;
	Inventory &inv = *_vm->_inventory;
	Journal &journal = *_vm->_journal;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	UserInterface &ui = *_vm->_ui;
	Common::Rect savedBounds = screen.getDisplayBounds();
	bool abortFlag = false;

	if (filename.empty())
		// No filename passed, so exit
		return;

	// If there any canimations currently running, or a portrait is being cleared,
	// save the filename for later executing when the canimation is done
	bool ongoingAnim = scene._canimShapes.size() > 0;
	if (IS_ROSE_TATTOO) {
		ongoingAnim = static_cast<Tattoo::TattooScene *>(_vm->_scene)->_activeCAnim.active();
	}
	if (ongoingAnim || people._clearingThePortrait) {
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
	MenuMode savedMode = ui._menuMode;
	ui._menuMode = TALK_MODE;

	// Turn on the Exit option
	ui._endKeyActive = true;

	if (people[HOLMES]._walkCount || (!people[HOLMES]._walkTo.empty() &&
			(IS_SERRATED_SCALPEL || people._allowWalkAbort))) {
		// Only interrupt if trying to do an action, and not just if player is walking around the scene
		if (people._allowWalkAbort)
			abortFlag = true;

		people[HOLMES].gotoStand();
	}

	if (_talkToAbort)
		return;

	freeTalkVars();

	// If any sequences have changed in the prior talk file, restore them
	if (_savedSequences.size() > 0) {
		for (uint idx = 0; idx < _savedSequences.size(); ++idx) {
			SequenceEntry &ss = _savedSequences[idx];
			for (uint idx2 = 0; idx2 < ss._sequences.size(); ++idx2)
				scene._bgShapes[ss._objNum]._sequences[idx2] = ss._sequences[idx2];

			// Reset the object's frame to the beginning of the sequence
			scene._bgShapes[ss._objNum]._frameNumber = 0;
		}
	}

	if (IS_ROSE_TATTOO) {
		pullSequence();
	} else {
		while (!isSequencesEmpty())
			pullSequence();
	}

	if (IS_SERRATED_SCALPEL) {
		// Restore any pressed button
		if (!ui._windowOpen && savedMode != STD_MODE)
			static_cast<Scalpel::ScalpelUserInterface *>(_vm->_ui)->restoreButton((int)(savedMode - 1));
	} else {
		static_cast<Tattoo::TattooPeople *>(_vm->_people)->pullNPCPaths();
	}

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
			if (_speaker < SPEAKER_REMOVE)
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

		default:
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

	if (select == -1) {
		if (IS_ROSE_TATTOO) {
			static_cast<Tattoo::TattooUserInterface *>(&ui)->putMessage(
				"%s", _vm->_fixedText->getText(Tattoo::kFixedText_NoEffect));
			return;
		}
		error("Couldn't find statement to display");
	}

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

			// Set up the talk file extension
			if (IS_ROSE_TATTOO && sound._speechOn && _scriptMoreFlag != 1)
				sound._talkSoundFile += Common::String::format("%02dB", select + 1);

			// Make a copy of the statement (in case the script frees the statement list), and then execute it
			Statement statement = _statements[select];

#ifdef USE_TTS
			if (_talkTo == -1 && ConfMan.getBool("tts_narrator")) {
				Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
				if (ttsMan != nullptr) {
					ttsMan->stop();
					ttsMan->say(_statements[select]._reply.c_str());
				}
			}
#endif

			doScript(_statements[select]._reply);

			if (IS_ROSE_TATTOO) {
				for (int idx = 0; idx < MAX_CHARACTERS; ++idx)
					people[idx]._misc = 0;
			}

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
				if (!newStatement._statement.hasPrefix("*") && !newStatement._statement.hasPrefix("^")) {
					_talkIndex = select;
					showTalk();

					// Break out of loop now that we're waiting for player input
					events.setCursor(ARROW);
					break;
				} else {
					// Add the statement into the journal and talk history
					if (_talkTo != -1 && !_talkHistory[_converseNum][select])
						journal.record(_converseNum, select, true);
					_talkHistory[_converseNum][select] = true;
				}

				ui._key = ui._oldKey = 'T'; // FIXME: I'm not sure what to do here, I need ScalpelUI->_hotkeyTalk
				ui._temp = ui._oldTemp = 0;
				ui._menuMode = TALK_MODE;
				_talkToFlag = 2;
			} else {
				freeTalkVars();

				if (IS_SERRATED_SCALPEL) {
					if (!ui._lookScriptFlag) {
						ui.drawInterface(2);
						ui._menuMode = STD_MODE;
						ui._windowBounds.top = CONTROLS_Y1;
					}
				} else {
					ui._menuMode = static_cast<Tattoo::TattooScene *>(_vm->_scene)->_labTableScene ? LAB_MODE : STD_MODE;
				}

				ui.banishWindow();
				break;
			}
		}
	}

	_talkStealth = 0;
	events._pressed = events._released = events._oldButtons = 0;
	events.clearKeyboard();

	if (savedBounds.bottom == SHERLOCK_SCREEN_HEIGHT)
		screen.resetDisplayBounds();
	else
		screen.setDisplayBounds(savedBounds);

	_talkToAbort = abortFlag;

	// If a script was added to the script stack, restore state so that the
	// previous script can continue
	popStack();

	events.setCursor(ARROW);
}

void Talk::initTalk(int objNum) {
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	UserInterface &ui = *_vm->_ui;

	ui._windowBounds.top = CONTROLS_Y;
	ui._infoFlag = true;
	_speaker = SPEAKER_REMOVE;

	Common::String talkFilename = (objNum >= 1000) ? people[objNum - 1000]._npcName : scene._bgShapes[objNum]._name;
	loadTalkFile(talkFilename);

	// Find the first statement with the correct flags
	int select = -1;
	for (uint idx = 0; idx < _statements.size(); ++idx) {
		if (_statements[idx]._talkMap == 0) {
			select = idx;
			break;
		}
	}

	if (select == -1) {
		freeTalkVars();
		if (!scumm_strnicmp(talkFilename.c_str(), "PATH", 4))
			error("No entries found to execute in path file");

		nothingToSay();
		return;
	}

	// See if the statement is a stealth mode reply
	Statement &statement = _statements[select];
	if (statement._statement.hasPrefix("^")) {
		clearSequences();

		// Start talk in stealth mode
		_talkStealth = 2;

		talkTo(talkFilename);
	} else if (statement._statement.hasPrefix("*")) {
		// Character being spoken to will speak first
		if (objNum > 1000) {
			(*static_cast<Tattoo::TattooPeople *>(_vm->_people))[objNum - 1000].walkHolmesToNPC();
		} else {
			Object &obj = scene._bgShapes[objNum];
			clearSequences();
			pushSequence(_talkTo);
			people.setListenSequence(_talkTo, 129);

			events.setCursor(WAIT);
			if (obj._lookPosition.y != 0)
				// Need to walk to character first
				people[HOLMES].walkToCoords(obj._lookPosition, obj._lookPosition._facing);
			events.setCursor(ARROW);
		}

		if (!_talkToAbort)
			talkTo(talkFilename);
	} else {
		// Holmes will be speaking first
		_talkToFlag = false;

		if (objNum > 1000) {
			(*static_cast<Tattoo::TattooPeople *>(_vm->_people))[objNum - 1000].walkHolmesToNPC();
		} else {
			Object &obj = scene._bgShapes[objNum];
			clearSequences();
			pushSequence(_talkTo);
			people.setListenSequence(_talkTo, 129);

			events.setCursor(WAIT);
			if (obj._lookPosition.y != 0)
				// Walk over to person to talk to
				people[HOLMES].walkToCoords(obj._lookPosition, obj._lookPosition._facing);
			events.setCursor(ARROW);
		}

		if (!_talkToAbort) {
			// See if walking over triggered a conversation
			if (_talkToFlag) {
				if (_talkToFlag == 1) {
					events.setCursor(ARROW);
					// _sequenceStack._count = 1;
					pullSequence();
				}
			} else {
				_talkIndex = select;
				showTalk();

				// Break out of loop now that we're waiting for player input
				events.setCursor(ARROW);
			}

			_talkToFlag = -1;
		}
	}
}

void Talk::freeTalkVars() {
	_statements.clear();
}

void Talk::loadTalkFile(const Common::String &filename) {
	People &people = *_vm->_people;
	Resources &res = *_vm->_res;
	Sound &sound = *_vm->_sound;

	// Save a copy of the talk filename
	_scriptName = filename;

	// Check for an existing person being talked to
	_talkTo = -1;
	for (int idx = 0; idx < (int)people._characters.size(); ++idx) {
		if (!scumm_strnicmp(filename.c_str(), people._characters[idx]._portrait, 4)) {
			_talkTo = idx;
			break;
		}
	}

	const char *chP = strchr(filename.c_str(), '.');
	Common::String talkFile = chP ? Common::String(filename.c_str(), chP) + ".tlk" :
		Common::String(filename.c_str(), filename.c_str() + 7) + ".tlk";

	// Create the base of the sound filename used for talking in Rose Tattoo
	if (IS_ROSE_TATTOO && _scriptMoreFlag != 1)
		sound._talkSoundFile = Common::String(filename.c_str(), filename.c_str() + 7) + ".";

	// Open the talk file for reading
	Common::SeekableReadStream *talkStream = res.load(talkFile);
	_converseNum = res.resourceIndex();
	talkStream->skip(2);	// Skip talk file version num

	_statements.clear();
	_statements.resize(talkStream->readByte());
	for (uint idx = 0; idx < _statements.size(); ++idx)
		_statements[idx].load(*talkStream, IS_ROSE_TATTOO);

	delete talkStream;

	if (!sound._voices)
		stripVoiceCommands();
	setTalkMap();
}

void Talk::stripVoiceCommands() {
	for (uint sIdx = 0; sIdx < _statements.size(); ++sIdx) {
		Statement &statement = _statements[sIdx];

		// Scan for an sound effect byte, which indicates to play a sound
		for (uint idx = 0; idx < statement._reply.size(); ++idx) {
			if (statement._reply[idx] == (char)_opcodes[OP_SFX_COMMAND]) {
				// Replace instruction character with a space, and delete the
				// rest of the name following it
				statement._reply = Common::String(statement._reply.c_str(),
					statement._reply.c_str() + idx) + " " +
					Common::String(statement._reply.c_str() + idx + 9);
			}
		}

		// Ensure the last character of the reply is not a space from the prior
		// conversion loop, to avoid any issues with the space ever causing a page
		// wrap, and ending up displaying another empty page
		while (statement._reply.lastChar() == ' ')
			statement._reply.deleteLastChar();
	}
}

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

void Talk::pushSequence(int speaker) {
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;

	// Only proceed if a speaker is specified
	if (speaker != -1) {
		int objNum = people.findSpeaker(speaker);
		if (objNum != -1)
			pushSequenceEntry(&scene._bgShapes[objNum]);
	}
}

void Talk::doScript(const Common::String &script) {
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	UserInterface &ui = *_vm->_ui;

	_savedSequences.clear();

	_scriptStart = (const byte *)script.c_str();
	_scriptEnd = _scriptStart + script.size();
	const byte *str = _scriptStart;
	_charCount = 0;
	_line = 0;
	_wait = 0;
	_pauseFlag = false;
	_seqCount = 0;
	_noTextYet = true;
	_endStr = false;
	_openTalkWindow = false;

	if (IS_SERRATED_SCALPEL)
		_yp = CONTROLS_Y + 12;
	else
		_yp = (_talkTo == -1) ? 5 : screen.fontHeight() + 11;

	if (IS_ROSE_TATTOO) {
		for (int idx = 0; idx < MAX_CHARACTERS; ++idx) {
			Tattoo::TattooPerson &p = (*(Tattoo::TattooPeople *)_vm->_people)[idx];
			p._savedNpcSequence = p._sequenceNumber;
			p._savedNpcFrame = p._frameNumber;
			p._resetNPCPath = true;
		}
	}

	if (_scriptMoreFlag) {
		_scriptMoreFlag = 0;
		str = _scriptStart + _scriptSaveIndex;
	}

	// Check if the script begins with a Stealh Mode Active command
	if (str[0] == _opcodes[OP_STEALTH_MODE_ACTIVE] || _talkStealth) {
		_talkStealth = 2;
		_speaker |= SPEAKER_REMOVE;
	} else {
		if (IS_SERRATED_SCALPEL)
			pushSequence(_speaker);
		if (IS_SERRATED_SCALPEL || ui._windowOpen)
			ui.clearWindow();

		// Need to switch speakers?
		if (str[0] == _opcodes[OP_SWITCH_SPEAKER]) {
			_speaker = str[1] - 1;

			if (IS_SERRATED_SCALPEL) {
				str += 2;
				pullSequence();
				pushSequence(_speaker);
			} else {
				str += 3;
			}

			people.setTalkSequence(_speaker);
		} else {
			people.setTalkSequence(_speaker);
		}

		if (IS_SERRATED_SCALPEL) {
			// Assign portrait location?
			if (str[0] == _opcodes[OP_ASSIGN_PORTRAIT_LOCATION]) {
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

			if (IS_SERRATED_SCALPEL) {
				// Remove portrait?
				if ( str[0] == _opcodes[OP_REMOVE_PORTRAIT]) {
					_speaker = -1;
				} else {
					// Nope, so set the first speaker
					((Scalpel::ScalpelPeople *)_vm->_people)->setTalking(_speaker);
				}
			}
		}
	}

	do {
		Common::String tempString;
		_wait = 0;

		byte c = str[0];
		if (!c) {
			_endStr = true;
		} else if (c == '{') {
			// Start of comment, so skip over it
			while (*str++ != '}')
				;
		} else if (isOpcode(c)) {
			// the original interpreter checked for c being >= 0x80
			// and if that is the case, it tried to process it as opcode, BUT ALSO ALWAYS skipped over it
			// This was done inside the Spanish + German interpreters of Serrated Scalpel, not the original
			// English interpreter (reverse engineered from the binaries).
			//
			// This resulted in special characters not getting shown in case they occurred at the start
			// of sentences like for example the inverted exclamation mark and the inverted question mark.
			// For further study see fonts.cpp
			//
			// We create an inverted exclamation mark for the Spanish version and we show it.
			//
			// Us not skipping over those characters may result in an assert() happening inside fonts.cpp
			// in case more invalid characters exist.
			// More information see bug #6931
			//

			// Handle control code
			switch ((this->*_opcodeTable[c - _opcodes[0]])(str)) {
			case RET_EXIT:
				return;
			case RET_CONTINUE:
				continue;
			default:
				break;
			}

			++str;
		} else {
			// Handle drawing the talk interface with the text
			talkInterface(str);
		}

		// Open window if it wasn't already open, and text has already been printed
		if ((_openTalkWindow && _wait) || (_openTalkWindow && str[0] >= _opcodes[0] && str[0] != _opcodes[OP_END_TEXT_WINDOW])) {
			if (!ui._slideWindows) {
				screen.slamRect(Common::Rect(0, CONTROLS_Y, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
			} else {
				ui.summonWindow();
			}

			ui._windowOpen = true;
			_openTalkWindow = false;
		}

		if (_wait)
			// Handling pausing
			talkWait(str);
	} while (!_vm->shouldQuit() && !_endStr);

	if (_wait != -1) {
		for (int ssIndex = 0; ssIndex < (int)_savedSequences.size(); ++ssIndex) {
			SequenceEntry &seq = _savedSequences[ssIndex];
			Object &object = scene._bgShapes[seq._objNum];

			for (uint idx = 0; idx < seq._sequences.size(); ++idx)
				object._sequences[idx] = seq._sequences[idx];
			object._frameNumber = seq._frameNumber;
			object._seqTo = seq._seqTo;
		}

		pullSequence();

		if (IS_SERRATED_SCALPEL) {
			if (_speaker >= 0 && _speaker < SPEAKER_REMOVE)
				people.clearTalking();
		} else {
			static_cast<Tattoo::TattooPeople *>(_vm->_people)->pullNPCPaths();
		}
	}
}

int Talk::waitForMore(int delay) {
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Sound &sound = *_vm->_sound;
	UserInterface &ui = *_vm->_ui;
	CursorId oldCursor = events.getCursor();
	int key2 = 254;
	bool playingSpeech = false;

	// Unless we're in stealth mode, show the appropriate cursor
	if (!_talkStealth) {
		events.setCursor(ui._lookScriptFlag ? MAGNIFY : ARROW);
	}

	// Handle playing any speech associated with the text being displayed
	switchSpeaker();
	if (sound._speechOn && IS_ROSE_TATTOO) {
		sound.playSpeech(sound._talkSoundFile);
		sound._talkSoundFile.setChar(sound._talkSoundFile.lastChar() + 1, sound._talkSoundFile.size() - 1);
	}
	playingSpeech = sound.isSpeechPlaying();

	do {
		if (IS_SERRATED_SCALPEL && playingSpeech && !sound.isSpeechPlaying())
			people._portrait._frameNumber = -1;

		scene.doBgAnim();

		// If talkTo call was done via doBgAnim, abort out of talk quietly
		if (_talkToAbort) {
			key2 = -1;
			events._released = true;
		} else {
			// See if there's been a button press
			events.pollEventsAndWait();
			events.setButtonState();

			if (events.kbHit()) {
				Common::KeyState keyState = events.getKey();
				if (keyState.keycode == Common::KEYCODE_ESCAPE) {
					if (IS_ROSE_TATTOO && static_cast<Tattoo::TattooEngine *>(_vm)->_runningProlog) {
						// Skip out of the introduction
						_vm->setFlags(-76);
						_vm->setFlags(396);
						scene._goToScene = 1;
					}
					break;

				} else if (Common::isPrint(keyState.ascii))
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

		if (playingSpeech && !sound.isSpeechPlaying())
			delay = 0;
	} while (!_vm->shouldQuit() && key2 == 254 && (delay || (playingSpeech && sound.isSpeechPlaying()))
		&& !events._released && !events._rightReleased);

	// If voices was set 2 to indicate a Scalpel voice file was playing, then reset it back to 1
	if (sound._voices == 2)
		sound._voices = 1;

	if (delay > 0 && sound.isSpeechPlaying())
		sound.stopSpeech();

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


	sound.stopSpeech();
	events.setCursor(_talkToAbort ? ARROW : oldCursor);
	events._pressed = events._released = false;

	return key2;
}

bool Talk::isOpcode(byte checkCharacter) {
	if ((checkCharacter < _opcodes[0]) || (checkCharacter >= (_opcodes[0] + 99)))
		return false; // outside of range
	if (_opcodeTable[checkCharacter - _opcodes[0]])
		return true;
	return false;
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

void Talk::synchronize(Serializer &s) {
	for (uint idx = 0; idx < _talkHistory.size(); ++idx) {
		TalkHistoryEntry &he = _talkHistory[idx];

		for (int flag = 0; flag < 16; ++flag)
			s.syncAsByte(he._data[flag]);
	}
}

OpcodeReturn Talk::cmdAddItemToInventory(const byte *&str) {
	Inventory &inv = *_vm->_inventory;
	Common::String tempString;

	++str;
	for (int idx = 0; idx < str[0]; ++idx)
		tempString += str[idx + 1];
	str += str[0];

	inv.putNameInInventory(tempString);
	return RET_SUCCESS;
}

OpcodeReturn Talk::cmdAdjustObjectSequence(const byte *&str) {
	Scene &scene = *_vm->_scene;
	Common::String tempString;

	// Get the name of the object to adjust
	++str;
	for (int idx = 0; idx < (str[0] & 127); ++idx)
		tempString += str[idx + 2];

	// Scan for object
	int objId = -1;
	for (uint idx = 0; idx < scene._bgShapes.size(); ++idx) {
		if (tempString.equalsIgnoreCase(scene._bgShapes[idx]._name))
			objId = idx;
	}
	if (objId == -1)
		error("Could not find object %s to change", tempString.c_str());

	// Should the script be overwritten?
	if (str[0] > 0x80) {
		// Save the current sequence
		_savedSequences.push(SequenceEntry());
		SequenceEntry &seqEntry = _savedSequences.top();
		seqEntry._objNum = objId;
		seqEntry._seqTo = scene._bgShapes[objId]._seqTo;
		for (uint idx = 0; idx < scene._bgShapes[objId]._seqSize; ++idx)
			seqEntry._sequences.push_back(scene._bgShapes[objId]._sequences[idx]);
	}

	// Get number of bytes to change
	_seqCount = str[1];
	str += (str[0] & 127) + 2;

	// WORKAROUND: Original German Scalpel crash when moving box at Tobacconists
	if (_vm->getLanguage() == Common::DE_DEU && _scriptName == "Alfr30Z")
		_seqCount = 16;

	// Copy in the new sequence
	for (int idx = 0; idx < _seqCount; ++idx, ++str)
		scene._bgShapes[objId]._sequences[idx] = str[0] - 1;

	// Reset object back to beginning of new sequence
	scene._bgShapes[objId]._frameNumber = 0;

	return RET_CONTINUE;
}

OpcodeReturn Talk::cmdBanishWindow(const byte *&str) {
	People &people = *_vm->_people;
	UserInterface &ui = *_vm->_ui;

	if (!(_speaker & SPEAKER_REMOVE))
		people.clearTalking();
	pullSequence();

	if (_talkToAbort)
		return RET_EXIT;

	_speaker |= SPEAKER_REMOVE;
	ui.banishWindow();
	ui._menuMode = TALK_MODE;
	_noTextYet = true;

	return RET_SUCCESS;
}

OpcodeReturn Talk::cmdDisableEndKey(const byte *&str) {
	_vm->_ui->_endKeyActive = false;
	return RET_SUCCESS;
}

OpcodeReturn Talk::cmdEnableEndKey(const byte *&str) {
	_vm->_ui->_endKeyActive = true;
	return RET_SUCCESS;
}

OpcodeReturn Talk::cmdEndTextWindow(const byte *&str) {
	return RET_SUCCESS;
}

OpcodeReturn Talk::cmdHolmesOff(const byte *&str) {
	People &people = *_vm->_people;
	people[HOLMES]._type = REMOVE;
	people._holmesOn = false;

	return RET_SUCCESS;
}

OpcodeReturn Talk::cmdHolmesOn(const byte *&str) {
	People &people = *_vm->_people;
	people[HOLMES]._type = CHARACTER;
	people._holmesOn = true;

	return RET_SUCCESS;
}

OpcodeReturn Talk::cmdPause(const byte *&str) {
	_charCount = *++str;
	_wait = _pauseFlag = true;

	return RET_SUCCESS;
}

OpcodeReturn Talk::cmdPauseWithoutControl(const byte *&str) {
	Events &events = *_vm->_events;
	Scene &scene = *_vm->_scene;
	++str;

	events.incWaitCounter();

	for (int idx = 0; idx < (str[0] - 1); ++idx) {
		scene.doBgAnim();
		if (_talkToAbort)
			return RET_EXIT;

		// Check for button press
		events.pollEvents();
		events.setButtonState();
	}

	events.decWaitCounter();

	_endStr = false;
	return RET_SUCCESS;
}

OpcodeReturn Talk::cmdRemoveItemFromInventory(const byte *&str) {
	Inventory &inv = *_vm->_inventory;
	Common::String tempString;

	++str;
	for (int idx = 0; idx < str[0]; ++idx)
		tempString += str[idx + 1];
	str += str[0];

	inv.deleteItemFromInventory(tempString);

	return RET_SUCCESS;
}

OpcodeReturn Talk::cmdRunCAnimation(const byte *&str) {
	Scene &scene = *_vm->_scene;

	++str;
	scene.startCAnim((str[0] - 1) & 127, (str[0] & 0x80) ? -1 : 1);
	if (_talkToAbort)
		return RET_EXIT;

	// Check if next character is changing side or changing portrait
	_wait = 0;
	if (_charCount && (str[1] == _opcodes[OP_SWITCH_SPEAKER] ||
			(IS_SERRATED_SCALPEL && str[1] == _opcodes[OP_ASSIGN_PORTRAIT_LOCATION])))
		_wait = 1;

	return RET_SUCCESS;
}

OpcodeReturn Talk::cmdSetFlag(const byte *&str) {
	++str;
	int flag1 = (str[0] - 1) * 256 + str[1] - 1 - (str[1] == 1 ? 1 : 0);
	int flag = (flag1 & 0x3fff) * (flag1 >= 0x4000 ? -1 : 1);
	_vm->setFlags(flag);
	++str;

	return RET_SUCCESS;
}

OpcodeReturn Talk::cmdSetObject(const byte *&str) {
	Scene &scene = *_vm->_scene;
	Common::String tempString;

	++str;
	for (int idx = 0; idx < (str[0] & 127); ++idx)
		tempString += str[idx + 1];

	// Set comparison state according to if we want to hide or unhide
	bool state = (str[0] >= SPEAKER_REMOVE);
	str += str[0] & 127;

	for (uint idx = 0; idx < scene._bgShapes.size(); ++idx) {
		Object &object = scene._bgShapes[idx];
		if (tempString.equalsIgnoreCase(object._name)) {
			// Only toggle the object if it's not in the desired state already
			if ((object._type == HIDDEN && state) || (object._type != HIDDEN && !state))
				object.toggleHidden();
		}
	}

	return RET_SUCCESS;
}

OpcodeReturn Talk::cmdStealthModeActivate(const byte *&str) {
	_talkStealth = 2;
	return RET_SUCCESS;
}

OpcodeReturn Talk::cmdStealthModeDeactivate(const byte *&str) {
	Events &events = *_vm->_events;

	_talkStealth = 0;
	events.clearKeyboard();

	return RET_SUCCESS;
}

OpcodeReturn Talk::cmdToggleObject(const byte *&str) {
	Scene &scene = *_vm->_scene;
	Common::String tempString;

	++str;
	for (int idx = 0; idx < str[0]; ++idx)
		tempString += str[idx + 1];

	scene.toggleObject(tempString);
	str += str[0];

	return RET_SUCCESS;
}

OpcodeReturn Talk::cmdWalkToCAnimation(const byte *&str) {
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;

	++str;
	CAnim &animation = scene._cAnim[str[0] - 1];
	people[HOLMES].walkToCoords(animation._goto[0], animation._goto[0]._facing);

	return _talkToAbort ? RET_EXIT : RET_SUCCESS;
}

void Talk::talkWait(const byte *&str) {
	if (!_pauseFlag && _charCount < 160)
		_charCount = 160;

	_wait = waitForMore(_charCount);
	if (_wait == -1)
		_endStr = true;

	// If a key was pressed to finish the window, see if further voice files should be skipped
	if (IS_SERRATED_SCALPEL && _wait >= 0 && _wait < 254) {
		if (str[0] == _opcodes[OP_SFX_COMMAND])
			str += 9;
	}

	_pauseFlag = false;
}

} // End of namespace Sherlock
