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

namespace Sherlock {

#define SPEAKER_REMOVE 0x80

const byte SCALPEL_OPCODES[] = {
	128,	// OP_SWITCH_SPEAKER
	129,	// OP_RUN_CANIMATION
	130,	// OP_ASSIGN_PORTRAIT_LOCATION
	131,	// OP_PAUSE
	132,	// OP_REMOVE_PORTRAIT
	133,	// OP_CLEAR_WINDOW
	134,	// OP_ADJUST_OBJ_SEQUENCE
	135,	// OP_WALK_TO_COORDS
	136,	// OP_PAUSE_WITHOUT_CONTROL
	137,	// OP_BANISH_WINDOW
	138,	// OP_SUMMON_WINDOW
	139,	// OP_SET_FLAG
	140,	// OP_SFX_COMMAND
	141,	// OP_TOGGLE_OBJECT
	142,	// OP_STEALTH_MODE_ACTIVE
	143,	// OP_IF_STATEMENT
	144,	// OP_ELSE_STATEMENT
	145,	// OP_END_IF_STATEMENT
	146,	// OP_STEALTH_MODE_DEACTIVATE
	147,	// OP_TURN_HOLMES_OFF
	148,	// OP_TURN_HOLMES_ON
	149,	// OP_GOTO_SCENE
	150,	// OP_PLAY_PROLOGUE
	151,	// OP_ADD_ITEM_TO_INVENTORY
	152,	// OP_SET_OBJECT
	153,	// OP_CALL_TALK_FILE
	143,	// OP_MOVE_MOUSE
	155,	// OP_DISPLAY_INFO_LINE
	156,	// OP_CLEAR_INFO_LINE
	157,	// OP_WALK_TO_CANIMATION
	158,	// OP_REMOVE_ITEM_FROM_INVENTORY
	159,	// OP_ENABLE_END_KEY
	160,	// OP_DISABLE_END_KEY
	161,	// OP_CARRIAGE_RETURN
	0,		// OP_MOUSE_ON_OFF
	0,		// OP_SET_WALK_CONTROL
	0,		// OP_SET_TALK_SEQUENCE
	0,		// OP_PLAY_SONG
	0,		// OP_WALK_HOLMES_AND_NPC_TO_CANIM
	0,		// OP_SET_NPC_PATH_DEST
	0,		// OP_NEXT_SONG
	0,		// OP_SET_NPC_PATH_PAUSE
	0,		// OP_PASSWORD
	0,		// OP_SET_SCENE_ENTRY_FLAG
	0,		// OP_WALK_NPC_TO_CANIM
	0,		// OP_WALK_HOLMES_AND_NPC_TO_COORDS
	0,		// OP_WALK_HOLMES_AND_NPC_TO_COORDS
	0,		// OP_SET_NPC_TALK_FILE
	0,		// OP_TURN_NPC_OFF
	0,		// OP_TURN_NPC_ON
	0,		// OP_NPC_DESC_ON_OFF
	0,		// OP_NPC_PATH_PAUSE_TAKING_NOTES
	0,		// OP_NPC_PATH_PAUSE_LOOKING_HOLMES
	0,		// OP_ENABLE_TALK_INTERRUPTS
	0,		// OP_DISABLE_TALK_INTERRUPTS
	0,		// OP_SET_NPC_INFO_LINE
	0,		// OP_SET_NPC_POSITION
	0,		// OP_NPC_PATH_LABEL
	0,		// OP_PATH_GOTO_LABEL
	0,		// OP_PATH_IF_FLAG_GOTO_LABEL
	0,		// OP_NPC_WALK_GRAPHICS
	0,		// OP_NPC_VERB
	0,		// OP_NPC_VERB_CANIM
	0,		// OP_NPC_VERB_SCRIPT
	0,		// OP_RESTORE_PEOPLE_SEQUENCE
	0,		// OP_NPC_VERB_TARGET
	0		// OP_TURN_SOUNDS_OFF
};

const byte TATTOO_OPCODES[] = {
	170,	// OP_SWITCH_SPEAKER
	171,	// OP_RUN_CANIMATION
	0,	// OP_ASSIGN_PORTRAIT_LOCATION
	173,	// OP_PAUSE
	0,	// OP_REMOVE_PORTRAIT
	0,	// OP_CLEAR_WINDOW
	176,	// OP_ADJUST_OBJ_SEQUENCE
	177,	// OP_WALK_TO_COORDS
	178,	// OP_PAUSE_WITHOUT_CONTROL
	179,	// OP_BANISH_WINDOW
	0,		// OP_SUMMON_WINDOW
	181,	// OP_SET_FLAG
	0,		// OP_SFX_COMMAND
	183,	// OP_TOGGLE_OBJECT
	184,	// OP_STEALTH_MODE_ACTIVE
	0,		// OP_IF_STATEMENT
	0,		// OP_ELSE_STATEMENT
	0,		// OP_END_IF_STATEMENT
	188,	// OP_STEALTH_MODE_DEACTIVATE
	189,	// OP_TURN_HOLMES_OFF
	190,	// OP_TURN_HOLMES_ON
	191,	// OP_GOTO_SCENE
	0,		// OP_PLAY_PROLOGUE
	193,	// OP_ADD_ITEM_TO_INVENTORY
	194,	// OP_SET_OBJECT
	172,	// OP_CALL_TALK_FILE
	0,		// OP_MOVE_MOUSE
	0,		// OP_DISPLAY_INFO_LINE
	0,		// OP_CLEAR_INFO_LINE
	199,	// OP_WALK_TO_CANIMATION
	200,	// OP_REMOVE_ITEM_FROM_INVENTORY
	201,	// OP_ENABLE_END_KEY
	202,	// OP_DISABLE_END_KEY
	0,		// OP_CARRIAGE_RETURN
	174,	// OP_MOUSE_ON_OFF
	175,	// OP_SET_WALK_CONTROL
	180,	// OP_SET_TALK_SEQUENCE
	182,	// OP_PLAY_SONG
	187,	// OP_WALK_HOLMES_AND_NPC_TO_CANIM
	192,	// OP_SET_NPC_PATH_DEST
	195,	// OP_NEXT_SONG
	196,	// OP_SET_NPC_PATH_PAUSE
	197,	// OP_PASSWORD
	198,	// OP_SET_SCENE_ENTRY_FLAG
	185,	// OP_WALK_NPC_TO_CANIM
	204,	// OP_WALK_HOLMES_AND_NPC_TO_COORDS
	205,	// OP_SET_NPC_TALK_FILE
	206,	// OP_TURN_NPC_OFF
	207,	// OP_TURN_NPC_ON
	208,	// OP_NPC_DESC_ON_OFF
	209,	// OP_NPC_PATH_PAUSE_TAKING_NOTES
	210,	// OP_NPC_PATH_PAUSE_LOOKING_HOLMES
	211,	// OP_ENABLE_TALK_INTERRUPTS
	212,	// OP_DISABLE_TALK_INTERRUPTS
	213,	// OP_SET_NPC_INFO_LINE
	214,	// OP_SET_NPC_POSITION
	215,	// OP_NPC_PATH_LABEL
	216,	// OP_PATH_GOTO_LABEL
	217,	// OP_PATH_IF_FLAG_GOTO_LABEL
	218,	// OP_NPC_WALK_GRAPHICS
	220,	// OP_NPC_VERB
	221,	// OP_NPC_VERB_CANIM
	222,	// OP_NPC_VERB_SCRIPT
	224,	// OP_RESTORE_PEOPLE_SEQUENCE
	226,	// OP_NPC_VERB_TARGET
	227		// OP_TURN_SOUNDS_OFF
};

/*----------------------------------------------------------------*/

SequenceEntry::SequenceEntry() {
	_objNum = 0;
	_frameNumber = 0;
	_seqTo = 0;
}

/*----------------------------------------------------------------*/

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

Talk *Talk::init(SherlockEngine *vm) {
	if (vm->getGameID() == GType_SerratedScalpel)
		return new ScalpelTalk(vm);
	else
		return new TattooTalk(vm);
}

Talk::Talk(SherlockEngine *vm) : _vm(vm) {
	_talkCounter = 0;
	_talkToAbort = false;
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
	_opcodes = IS_SERRATED_SCALPEL ? SCALPEL_OPCODES : TATTOO_OPCODES;

	_charCount = 0;
	_line = 0;
	_yp = 0;
	_wait = 0;
	_pauseFlag = false;
	_seqCount = 0;
	_scriptStart = _scriptEnd = nullptr;
}

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
	if (scene._canimShapes.size() > 0 || people._clearingThePortrait) {
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
			for (uint idx2 = 0; idx2 < ss._sequences.size(); ++idx2)
				scene._bgShapes[ss._objNum]._sequences[idx2] = ss._sequences[idx2];

			// Reset the object's frame to the beginning of the sequence
			scene._bgShapes[ss._objNum]._frameNumber = 0;
		}
	}

	while (!_sequenceStack.empty())
		pullSequence();

	if (IS_SERRATED_SCALPEL) {
		// Restore any pressed button
		if (!ui._windowOpen && savedMode != STD_MODE)
			((ScalpelUserInterface *)_vm->_ui)->restoreButton((int)(savedMode - 1));
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

					// If the window is already open, simply draw. Otherwise, do it
					// to the back buffer and then summon the window
					if (ui._windowOpen) {
						screen.buttonPrint(Common::Point(119, CONTROLS_Y), color, true, "Exit");
					} else {
						screen.buttonPrint(Common::Point(119, CONTROLS_Y), color, false, "Exit");

						if (!ui._slideWindows) {
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

	if (savedBounds.bottom == SHERLOCK_SCREEN_HEIGHT)
		screen.resetDisplayBounds();
	else
		screen.setDisplayBounds(savedBounds);

	_talkToAbort = abortFlag;

	// If a script was added to the script stack, restore state so that the
	// previous script can continue
	popStack();

	if (_vm->getGameID() == GType_SerratedScalpel && filename == "Tube59c") {
		// WORKAROUND: Original game bug causes the results of testing the powdery substance
		// to disappear too quickly. Introduce a delay to allow it to be properly displayed
		ui._menuCounter = 30;
	}

	events.setCursor(ARROW);
}

void Talk::talk(int objNum) {
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	UserInterface &ui = *_vm->_ui;
	Object &obj = scene._bgShapes[objNum];

	ui._windowBounds.top = CONTROLS_Y;
	ui._infoFlag = true;
	_speaker = SPEAKER_REMOVE;
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

		if (!_talkToAbort)
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

				if (!ui._slideWindows) {
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

int Talk::talkLine(int lineNum, int stateNum, byte color, int lineY, bool slamIt) {
	Screen &screen = *_vm->_screen;
	int idx = lineNum;
	Common::String msg, number;
	bool numberFlag = false;

	// Get the statement to display as well as optional number prefix
	if (idx < SPEAKER_REMOVE) {
		number = Common::String::format("%d.", stateNum + 1);
		numberFlag = true;
	} else {
		idx -= SPEAKER_REMOVE;
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
						screen.print(Common::Point(16, lineY), color, "%s", number.c_str());
					}

					// Draw the line with an indent
					screen.print(Common::Point(30, lineY), color, "%s", sLine.c_str());
				} else {
					screen.print(Common::Point(16, lineY), color, "%s", sLine.c_str());
				}
			} else {
				if (numberFlag) {
					if (lineStartP == msg.c_str()) {
						screen.gPrint(Common::Point(16, lineY - 1), color, "%s", number.c_str());
					}

					screen.gPrint(Common::Point(30, lineY - 1), color, "%s", sLine.c_str());
				} else {
					screen.gPrint(Common::Point(16, lineY - 1), color, "%s", sLine.c_str());
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

void Talk::clearSequences() {
	_sequenceStack.clear();
}

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

void Talk::setSequence(int speaker) {
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;

	// If no speaker is specified, then nothing needs to be done
	if (speaker == -1)
		return;

	if (speaker) {
		int objNum = people.findSpeaker(speaker);
		if (objNum != -1) {
			Object &obj = scene._bgShapes[objNum];

			if (obj._seqSize < MAX_TALK_SEQUENCES) {
				warning("Tried to copy too many talk frames");
			} else {
				for (int idx = 0; idx < MAX_TALK_SEQUENCES; ++idx) {
					obj._sequences[idx] = people._characters[speaker]._talkSequences[idx];
					if (idx > 0 && !obj._sequences[idx] && !obj._sequences[idx - 1])
						return;

					obj._frameNumber = 0;
					obj._sequenceNumber = 0;
				}
			}
		}
	}
}

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
					obj._sequences[idx] = people._characters[speaker]._stillSequences[idx];
					if (idx > 0 && !people._characters[speaker]._talkSequences[idx] &&
							!people._characters[speaker]._talkSequences[idx - 1])
						break;
				}

				obj._frameNumber = 0;
				obj._seqTo = 0;
			}
		}
	}
}

void Talk::doScript(const Common::String &script) {
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	UserInterface &ui = *_vm->_ui;
	bool openTalkWindow = false;

	_savedSequences.clear();

	_scriptStart = (const byte *)script.c_str();
	_scriptEnd = _scriptStart + script.size();
	const byte *str = _scriptStart;
	_yp = CONTROLS_Y + 12;
	_charCount = 0;
	_line = 0;
	_wait = 0;
	_pauseFlag = false;
	_seqCount = 0;
	_noTextYet = true;
	_endStr = false;

	if (_scriptMoreFlag) {
		_scriptMoreFlag = 0;
		str = _scriptStart + _scriptSaveIndex;
	}

	// Check if the script begins with a Stealh Mode Active command
	if (str[0] == _opcodes[OP_STEALTH_MODE_ACTIVE] || _talkStealth) {
		_talkStealth = 2;
		_speaker |= SPEAKER_REMOVE;
	} else {
		pushSequence(_speaker);
		ui.clearWindow();

		// Need to switch speakers?
		if (str[0] == _opcodes[OP_SWITCH_SPEAKER]) {
			_speaker = str[1] - 1;
			str += 2;
			pullSequence();
			pushSequence(_speaker);
			setSequence(_speaker);
		} else {
			setSequence(_speaker);
		}

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

		// Remove portrait?
		if (str[0] == _opcodes[OP_REMOVE_PORTRAIT]) {
			_speaker = 255;
		} else {
			// Nope, so set the first speaker
			people.setTalking(_speaker);
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
		} else if (c >= 128 && c <= 227 && _opcodeTable[c - 128]) {
			// Handle control code
			switch ((this->*_opcodeTable[c - 128])(str)) {
			case RET_EXIT:
				return;
			case RET_CONTINUE:
				continue;
			default:
				break;
			}

			++str;
		} else {
			// If the window isn't yet open, draw the window before printing starts
			if (!ui._windowOpen && _noTextYet) {
				_noTextYet = false;
				drawInterface();

				if (_talkTo != -1) {
					screen.buttonPrint(Common::Point(119, CONTROLS_Y), COMMAND_NULL, false, "Exit");
					screen.buttonPrint(Common::Point(159, CONTROLS_Y), COMMAND_NULL, false, "Up");
					screen.buttonPrint(Common::Point(200, CONTROLS_Y), COMMAND_NULL, false, "Down");
				}
			}

			// If it's the first line, display the speaker
			if (!_line && _speaker >= 0 && _speaker < (int)people._characters.size()) {
				// If the window is open, display the name directly on-screen.
				// Otherwise, simply draw it on the back buffer
				if (ui._windowOpen) {
					screen.print(Common::Point(16, _yp), TALK_FOREGROUND, "%s",
						people._characters[_speaker & 127]._name);
				} else {
					screen.gPrint(Common::Point(16, _yp - 1), TALK_FOREGROUND, "%s", 
						people._characters[_speaker & 127]._name);
					openTalkWindow = true;
				}

				_yp += 9;
			}

			// Find amount of text that will fit on the line
			int width = 0, idx = 0;
			do {
				width += screen.charWidth(str[idx]);
				++idx;
				++_charCount;
			} while (width < 298 && str[idx] && str[idx] != '{' && str[idx] < _opcodes[0]);

			if (str[idx] || width >= 298) {
				if (str[idx] < _opcodes[0] && str[idx] != '{') {
					--idx;
					--_charCount;
				}
			} else {
				_endStr = true;
			}

			// If word wrap is needed, find the start of the current word
			if (width >= 298) {
				while (str[idx] != ' ') {
					--idx;
					--_charCount;
				}
			}

			// Print the line
			Common::String lineStr((const char *)str, (const char *)str + idx);

			// If the speaker indicates a description file, print it in yellow
			if (_speaker != -1) {
				if (ui._windowOpen) {
					screen.print(Common::Point(16, _yp), COMMAND_FOREGROUND, "%s", lineStr.c_str());
				} else {
					screen.gPrint(Common::Point(16, _yp - 1), COMMAND_FOREGROUND, "%s", lineStr.c_str());
					openTalkWindow = true;
				}
			} else {
				if (ui._windowOpen) {
					screen.print(Common::Point(16, _yp), COMMAND_FOREGROUND, "%s", lineStr.c_str());
				} else {
					screen.gPrint(Common::Point(16, _yp - 1), COMMAND_FOREGROUND, "%s", lineStr.c_str());
					openTalkWindow = true;
				}
			}

			// Move to end of displayed line
			str += idx;

			// If line wrap occurred, then move to after the separating space between the words
			if (str[0] < _opcodes[0] && str[0] != '{')
				++str;

			_yp += 9;
			++_line;

			// Certain different conditions require a wait
			if ((_line == 4 && str < _scriptEnd && str[0] != _opcodes[OP_SFX_COMMAND] && str[0] != _opcodes[OP_PAUSE] && _speaker != -1) ||
					(_line == 5 && str < _scriptEnd && str[0] != _opcodes[OP_PAUSE] && _speaker == -1) ||
					_endStr) {
				_wait = 1;
			}

			byte v = (str >= _scriptEnd ? 0 : str[0]);
			_wait = v == _opcodes[OP_SWITCH_SPEAKER] || v == _opcodes[OP_ASSIGN_PORTRAIT_LOCATION] ||
				v == _opcodes[OP_BANISH_WINDOW] || _opcodes[OP_IF_STATEMENT] || v == _opcodes[OP_ELSE_STATEMENT] ||
				v == _opcodes[OP_END_IF_STATEMENT] || v == _opcodes[OP_GOTO_SCENE] || v == _opcodes[OP_CALL_TALK_FILE];
		}

		// Open window if it wasn't already open, and text has already been printed
		if ((openTalkWindow && _wait) || (openTalkWindow && str[0] >= _opcodes[0] && str[0] != _opcodes[OP_CARRIAGE_RETURN])) {
			if (!ui._slideWindows) {
				screen.slamRect(Common::Rect(0, CONTROLS_Y, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
			} else {
				ui.summonWindow();
			}

			ui._windowOpen = true;
			openTalkWindow = false;
		}

		if (_wait) {
			// Handling pausing
			if (!_pauseFlag && _charCount < 160)
				_charCount = 160;

			_wait = waitForMore(_charCount);
			if (_wait == -1)
				_endStr = true;

			// If a key was pressed to finish the window, see if further voice files should be skipped
			if (_wait >= 0 && _wait < 254) {
				if (str[0] == _opcodes[OP_SFX_COMMAND])
					str += 9;
			}

			// Clear the window unless the wait was due to a PAUSE command
			if (!_pauseFlag && _wait != -1 && str < _scriptEnd && str[0] != _opcodes[OP_SFX_COMMAND]) {
				if (!_talkStealth)
					ui.clearWindow();
				_yp = CONTROLS_Y + 12;
				_charCount = _line = 0;
			}

			_pauseFlag = false;
		}
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
		if (_speaker >= 0 && _speaker < SPEAKER_REMOVE)
			people.clearTalking();
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
				if (Common::isPrint(keyState.ascii))
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

void Talk::synchronize(Common::Serializer &s) {
	for (int idx = 0; idx < MAX_TALK_FILES; ++idx) {
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

OpcodeReturn Talk::cmdCallTalkFile(const byte *&str) {
	Common::String tempString;

	++str;
	for (int idx = 0; idx < 8 && str[idx] != '~'; ++idx)
		tempString += str[idx];
	str += 8;

	int scriptCurrentIndex = str - _scriptStart;

	// Save the current script position and new talk file
	if (_scriptStack.size() < 9) {
		ScriptStackEntry rec1;
		rec1._name = _scriptName;
		rec1._currentIndex = scriptCurrentIndex;
		rec1._select = _scriptSelect;
		_scriptStack.push(rec1);

		// Push the new talk file onto the stack
		ScriptStackEntry rec2;
		rec2._name = tempString;
		rec2._currentIndex = 0;
		rec2._select = 100;
		_scriptStack.push(rec2);
	}
	else {
		error("Script stack overflow");
	}

	_scriptMoreFlag = 1;
	_endStr = true;
	_wait = 0;

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

OpcodeReturn Talk::cmdGotoScene(const byte *&str) {
	Map &map = *_vm->_map;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	scene._goToScene = str[1] - 1;

	if (scene._goToScene != 100) {
		// Not going to the map overview
		map._oldCharPoint = scene._goToScene;
		map._overPos.x = map[scene._goToScene].x * 100 - 600;
		map._overPos.y = map[scene._goToScene].y * 100 + 900;

		// Run a canimation?
		if (str[2] > 100) {
			people._hSavedFacing = str[2];
			people._hSavedPos = Common::Point(160, 100);
		}
	}
	str += 6;

	_scriptMoreFlag = (scene._goToScene == 100) ? 2 : 1;
	_scriptSaveIndex = str - _scriptStart;
	_endStr = true;
	_wait = 0;

	return RET_SUCCESS;
}

OpcodeReturn Talk::cmdHolmesOff(const byte *&str) {
	People &people = *_vm->_people;
	people._holmesOn = false;
	if (IS_ROSE_TATTOO)
		people[PLAYER]._type = REMOVE;

	return RET_SUCCESS;
}

OpcodeReturn Talk::cmdHolmesOn(const byte *&str) {
	People &people = *_vm->_people;
	people._holmesOn = true;
	if (IS_ROSE_TATTOO)
		people[PLAYER]._type = CHARACTER;

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

	for (int idx = 0; idx < (str[0] - 1); ++idx) {
		scene.doBgAnim();
		if (_talkToAbort)
			return RET_EXIT;

		// Check for button press
		events.pollEvents();
		events.setButtonState();
	}

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
	if (_charCount && (str[1] == _opcodes[OP_SWITCH_SPEAKER] || str[1] == _opcodes[OP_ASSIGN_PORTRAIT_LOCATION]))
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

OpcodeReturn Talk::cmdSwitchSpeaker(const byte *&str) {
	People &people = *_vm->_people;
	UserInterface &ui = *_vm->_ui;

	if (!(_speaker & SPEAKER_REMOVE))
		people.clearTalking();
	if (_talkToAbort)
		return RET_EXIT;

	ui.clearWindow();
	_yp = CONTROLS_Y + 12;
	_charCount = _line = 0;

	_speaker = *++str - 1;
	people.setTalking(_speaker);
	pullSequence();
	pushSequence(_speaker);
	setSequence(_speaker);

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
	people.walkToCoords(animation._goto, animation._gotoDir);
	
	return _talkToAbort ? RET_EXIT : RET_SUCCESS;
}

OpcodeReturn Talk::cmdWalkToCoords(const byte *&str) {
	People &people = *_vm->_people;
	++str;

	people.walkToCoords(Common::Point(((str[0] - 1) * 256 + str[1] - 1) * 100,
		str[2] * 100), str[3] - 1);
	if (_talkToAbort)
		return RET_EXIT;

	str += 3;
	return RET_SUCCESS;
}

/*----------------------------------------------------------------*/

ScalpelTalk::ScalpelTalk(SherlockEngine *vm) : Talk(vm) {
	static OpcodeMethod OPCODE_METHODS[] = {
		(OpcodeMethod)&ScalpelTalk::cmdSwitchSpeaker,
		(OpcodeMethod)&ScalpelTalk::cmdRunCAnimation,
		(OpcodeMethod)&ScalpelTalk::cmdAssignPortraitLocation,

		(OpcodeMethod)&ScalpelTalk::cmdPause,
		(OpcodeMethod)&ScalpelTalk::cmdRemovePortrait,
		(OpcodeMethod)&ScalpelTalk::cmdClearWindow,
		(OpcodeMethod)&ScalpelTalk::cmdAdjustObjectSequence,
		(OpcodeMethod)&ScalpelTalk::cmdWalkToCoords,
		(OpcodeMethod)&ScalpelTalk::cmdPauseWithoutControl,
		(OpcodeMethod)&ScalpelTalk::cmdBanishWindow,
		(OpcodeMethod)&ScalpelTalk::cmdSummonWindow,
		(OpcodeMethod)&ScalpelTalk::cmdSetFlag,
		(OpcodeMethod)&ScalpelTalk::cmdSfxCommand,

		(OpcodeMethod)&ScalpelTalk::cmdToggleObject,
		(OpcodeMethod)&ScalpelTalk::cmdStealthModeActivate,
		(OpcodeMethod)&ScalpelTalk::cmdIf,
		(OpcodeMethod)&ScalpelTalk::cmdElse,
		nullptr,
		(OpcodeMethod)&ScalpelTalk::cmdStealthModeDeactivate,
		(OpcodeMethod)&ScalpelTalk::cmdHolmesOff,
		(OpcodeMethod)&ScalpelTalk::cmdHolmesOn,
		(OpcodeMethod)&ScalpelTalk::cmdGotoScene,
		(OpcodeMethod)&ScalpelTalk::cmdPlayPrologue,
		
		(OpcodeMethod)&ScalpelTalk::cmdAddItemToInventory,
		(OpcodeMethod)&ScalpelTalk::cmdSetObject,
		(OpcodeMethod)&ScalpelTalk::cmdCallTalkFile,
		(OpcodeMethod)&ScalpelTalk::cmdMoveMouse,
		(OpcodeMethod)&ScalpelTalk::cmdDisplayInfoLine,
		(OpcodeMethod)&ScalpelTalk::cmdClearInfoLine,
		(OpcodeMethod)&ScalpelTalk::cmdWalkToCAnimation,
		(OpcodeMethod)&ScalpelTalk::cmdRemoveItemFromInventory,
		(OpcodeMethod)&ScalpelTalk::cmdEnableEndKey,
		(OpcodeMethod)&ScalpelTalk::cmdDisableEndKey,
		
		(OpcodeMethod)&ScalpelTalk::cmdCarriageReturn,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
	};

	_opcodeTable = OPCODE_METHODS;
}

OpcodeReturn ScalpelTalk::cmdAssignPortraitLocation(const byte *&str) {
	People &people = *_vm->_people;

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

	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdClearInfoLine(const byte *&str) {
	UserInterface &ui = *_vm->_ui;

	ui._infoFlag = true;
	ui.clearInfo();

	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdClearWindow(const byte *&str) {
	UserInterface &ui = *_vm->_ui;
	
	ui.clearWindow();
	_yp = CONTROLS_Y + 12;
	_charCount = _line = 0;

	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdDisplayInfoLine(const byte *&str) {
	Screen &screen = *_vm->_screen;
	UserInterface &ui = *_vm->_ui;
	Common::String tempString;

	++str;
	for (int idx = 0; idx < str[0]; ++idx)
		tempString += str[idx + 1];
	str += str[0];

	screen.print(Common::Point(0, INFO_LINE + 1), INFO_FOREGROUND, "%s", tempString.c_str());
	ui._menuCounter = 30;

	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdElse(const byte *&str) {
	// If this is encountered here, it means that a preceeding IF statement was found,
	// and evaluated to true. Now all the statements for the true block are finished,
	// so skip over the block of code that would have executed if the result was false
	_wait = 0;
	do {
		++str;
	} while (str[0] && str[0] != _opcodes[OP_END_IF_STATEMENT]);

	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdIf(const byte *&str) {
	++str;
	int flag = (str[0] - 1) * 256 + str[1] - 1 - (str[1] == 1 ? 1 : 0);
	++str;
	_wait = 0;

	bool result = flag < 0x8000;
	if (_vm->readFlags(flag & 0x7fff) != result) {
		do {
			++str;
		} while (str[0] && str[0] != _opcodes[OP_ELSE_STATEMENT] && str[0] != _opcodes[OP_END_IF_STATEMENT]);

		if (!str[0])
			_endStr = true;
	}

	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdMoveMouse(const byte *&str) {
	Events &events = *_vm->_events;

	++str;
	events.moveMouse(Common::Point((str[0] - 1) * 256 + str[1] - 1, str[2]));
	if (_talkToAbort)
		return RET_EXIT;
	str += 3;
	
	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdPlayPrologue(const byte *&str) {
	Animation &anim = *_vm->_animation;
	Common::String tempString;

	++str;
	for (int idx = 0; idx < 8 && str[idx] != '~'; ++idx)
		tempString += str[idx];

	anim.play(tempString, 1, 3, true, 4);

	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdRemovePortrait(const byte *&str) {
	People &people = *_vm->_people;

	if (_speaker >= 0 && _speaker < SPEAKER_REMOVE)
		people.clearTalking();
	pullSequence();
	if (_talkToAbort)
		return RET_EXIT;

	_speaker |= SPEAKER_REMOVE;
	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdSfxCommand(const byte *&str) {
	Sound &sound = *_vm->_sound;
	Common::String tempString;

	++str;
	if (sound._voices) {
		for (int idx = 0; idx < 8 && str[idx] != '~'; ++idx)
			tempString += str[idx];
		sound.playSound(tempString, WAIT_RETURN_IMMEDIATELY);

		// Set voices to wait for more
		sound._voices = 2;
		sound._speechOn = (*sound._soundIsOn);
	}

	_wait = 1;
	str += 7;

	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdSummonWindow(const byte *&str) {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;

	drawInterface();
	events._pressed = events._released = false;
	events.clearKeyboard();
	_noTextYet = false;

	if (_speaker != -1) {
		screen.buttonPrint(Common::Point(119, CONTROLS_Y), COMMAND_NULL, false, "Exit");
		screen.buttonPrint(Common::Point(159, CONTROLS_Y), COMMAND_NULL, false, "Up");
		screen.buttonPrint(Common::Point(200, CONTROLS_Y), COMMAND_NULL, false, "Down");
	}

	return RET_SUCCESS;
}

OpcodeReturn ScalpelTalk::cmdCarriageReturn(const byte *&str) {
	return RET_SUCCESS;
}

/*----------------------------------------------------------------*/

TattooTalk::TattooTalk(SherlockEngine *vm) : Talk(vm) {
	static OpcodeMethod OPCODE_METHODS[] = {
		nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		(OpcodeMethod)&TattooTalk::cmdSwitchSpeaker,

		(OpcodeMethod)&TattooTalk::cmdRunCAnimation,
		(OpcodeMethod)&TattooTalk::cmdCallTalkFile,
		(OpcodeMethod)&TattooTalk::cmdPause,
		(OpcodeMethod)&TattooTalk::cmdMouseOnOff,
		(OpcodeMethod)&TattooTalk::cmdSetWalkControl,
		(OpcodeMethod)&TattooTalk::cmdAdjustObjectSequence,
		(OpcodeMethod)&TattooTalk::cmdWalkToCoords,
		(OpcodeMethod)&TattooTalk::cmdPauseWithoutControl,
		(OpcodeMethod)&TattooTalk::cmdBanishWindow,
		(OpcodeMethod)&TattooTalk::cmdSetTalkSequence,

		(OpcodeMethod)&TattooTalk::cmdSetFlag,
		(OpcodeMethod)&TattooTalk::cmdPlaySong,
		(OpcodeMethod)&TattooTalk::cmdToggleObject,
		(OpcodeMethod)&TattooTalk::cmdStealthModeActivate,
		(OpcodeMethod)&TattooTalk::cmdWalkNPCToCAnimation,
		(OpcodeMethod)&TattooTalk::cmdWalkNPCToCoords,
		(OpcodeMethod)&TattooTalk::cmdWalkHomesAndNPCToCoords,
		(OpcodeMethod)&TattooTalk::cmdStealthModeDeactivate,
		(OpcodeMethod)&TattooTalk::cmdHolmesOff,
		(OpcodeMethod)&TattooTalk::cmdHolmesOn,

		(OpcodeMethod)&TattooTalk::cmdGotoScene,
		(OpcodeMethod)&TattooTalk::cmdSetNPCPathDest,
		(OpcodeMethod)&TattooTalk::cmdAddItemToInventory,
		(OpcodeMethod)&TattooTalk::cmdSetObject,
		(OpcodeMethod)&TattooTalk::cmdNextSong,
		(OpcodeMethod)&TattooTalk::cmdSetNPCPathPause,
		(OpcodeMethod)&TattooTalk::cmdPassword,
		(OpcodeMethod)&TattooTalk::cmdSetSceneEntryFlag,
		(OpcodeMethod)&TattooTalk::cmdWalkToCAnimation,
		(OpcodeMethod)&TattooTalk::cmdRemoveItemFromInventory,

		(OpcodeMethod)&TattooTalk::cmdEnableEndKey,
		(OpcodeMethod)&TattooTalk::cmdDisableEndKey,
		nullptr,
		(OpcodeMethod)&TattooTalk::cmdWalkHomesAndNPCToCoords,
		(OpcodeMethod)&TattooTalk::cmdSetNPCTalkFile,
		(OpcodeMethod)&TattooTalk::cmdSetNPCOff,
		(OpcodeMethod)&TattooTalk::cmdSetNPCOn,
		(OpcodeMethod)&TattooTalk::cmdSetNPCDescOnOff,
		(OpcodeMethod)&TattooTalk::cmdSetNPCPathPauseTakingNotes,
		(OpcodeMethod)&TattooTalk::cmdSetNPCPathPauseLookingHolmes,

		(OpcodeMethod)&TattooTalk::cmdTalkInterruptsEnable,
		(OpcodeMethod)&TattooTalk::cmdTalkInterruptsDisable,
		(OpcodeMethod)&TattooTalk::cmdSetNPCInfoLine,
		(OpcodeMethod)&TattooTalk::cmdSetNPCPosition,
		(OpcodeMethod)&TattooTalk::cmdNPCLabelSet,
		(OpcodeMethod)&TattooTalk::cmdNPCLabelGoto,
		(OpcodeMethod)&TattooTalk::cmdNPCLabelIfFlagGoto,
		(OpcodeMethod)&TattooTalk::cmdSetNPCWalkGraphics,
		nullptr,
		(OpcodeMethod)&TattooTalk::cmdSetNPCVerb,

		(OpcodeMethod)&TattooTalk::cmdSetNPCVerbCAnimation,
		(OpcodeMethod)&TattooTalk::cmdSetNPCVerbScript,
		nullptr,
		(OpcodeMethod)&TattooTalk::cmdRestorePeopleSequence,
		(OpcodeMethod)&TattooTalk::cmdSetNPCVerbTarget,
		(OpcodeMethod)&TattooTalk::cmdTurnSoundsOff
	};

	_opcodeTable = OPCODE_METHODS;
}

OpcodeReturn TattooTalk::cmdMouseOnOff(const byte *&str) { error("TODO: script opcode"); }

OpcodeReturn TattooTalk::cmdNextSong(const byte *&str) {
	Sound &sound = *_vm->_sound;

	// Get the name of the next song to play
	++str;
	sound._nextSongName = "";
	for (int idx = 0; idx < 8; ++idx) {
		if (str[idx] != '~')
			sound._nextSongName += str[idx];
		else
			break;
	}
	str += 7;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdNPCLabelGoto(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdNPCLabelIfFlagGoto(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdNPCLabelSet(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdPassword(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdPlaySong(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdRestorePeopleSequence(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdSetNPCDescOnOff(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdSetNPCInfoLine(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdSetNPCOff(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdSetNPCOn(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdSetNPCPathDest(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdSetNPCPathPause(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdSetNPCPathPauseTakingNotes(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdSetNPCPathPauseLookingHolmes(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdSetNPCPosition(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdSetNPCTalkFile(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdSetNPCVerb(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdSetNPCVerbCAnimation(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdSetNPCVerbScript(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdSetNPCVerbTarget(const byte *&str) { error("TODO: script opcode"); }

OpcodeReturn TattooTalk::cmdSetNPCWalkGraphics(const byte *&str) {
	++str;
	int npc = *str - 1;
	People &people = *_vm->_people;
	Person &person = people[npc];

	// Build up walk library name for the given NPC
	person._walkVGSName = "";
	for (int idx = 0; idx < 8; ++idx) {
		if (str[idx + 1] != '~')
			person._walkVGSName += str[idx + 1];
		else
			break;
	}
	person._walkVGSName += ".VGS";

	people._forceWalkReload = true;
	str += 8;

	return RET_SUCCESS;
}

OpcodeReturn TattooTalk::cmdSetSceneEntryFlag(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdSetTalkSequence(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdSetWalkControl(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdTalkInterruptsDisable(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdTalkInterruptsEnable(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdTurnSoundsOff(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdWalkHolmesAndNPCToCAnimation(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdWalkNPCToCAnimation(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdWalkNPCToCoords(const byte *&str) { error("TODO: script opcode"); }
OpcodeReturn TattooTalk::cmdWalkHomesAndNPCToCoords(const byte *&str) { error("TODO: script opcode"); }

} // End of namespace Sherlock
