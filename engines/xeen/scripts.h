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

#ifndef XEEN_SCRIPTS_H
#define XEEN_SCRIPTS_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/serializer.h"
#include "common/stack.h"
#include "common/str-array.h"
#include "xeen/files.h"
#include "xeen/party.h"

namespace Xeen {

enum Opcode {
	OP_None                  = 0x00,
	OP_Display0x01           = 0x01,
	OP_DoorTextSml           = 0x02,
	OP_DoorTextLrg           = 0x03,
	OP_SignText              = 0x04,
	OP_NPC                   = 0x05,
	OP_PlayFX                = 0x06,
	OP_TeleportAndExit       = 0x07,
	OP_If1                   = 0x08,
	OP_If2                   = 0x09,
	OP_If3                   = 0x0A,
	OP_MoveObj               = 0x0B,
	OP_TakeOrGive            = 0x0C,
	OP_NoAction              = 0x0D,
	OP_Remove                = 0x0E,
	OP_SetChar               = 0x0F,
	OP_Spawn                 = 0x10,
	OP_DoTownEvent           = 0x11,
	OP_Exit                  = 0x12,
	OP_AfterMap              = 0x13,
	OP_GiveMulti             = 0x14,
	OP_ConfirmWord           = 0x15,
	OP_Damage                = 0x16,
	OP_JumpRnd               = 0x17,
	OP_AfterEvent            = 0x18,
	OP_CallEvent             = 0x19,
	OP_Return                = 0x1A,
	OP_SetVar                = 0x1B,
	OP_TakeOrGive_2          = 0x1C,
	OP_TakeOrGive_3          = 0x1D,
	OP_CutsceneEndClouds     = 0x1E,
	OP_TeleportAndContinue   = 0x1F,
	OP_WhoWill               = 0x20,
	OP_RndDamage             = 0x21,
	OP_MoveWallObj           = 0x22,
	OP_AlterCellFlag         = 0x23,
	OP_AlterHed              = 0x24,
	OP_DisplayStat           = 0x25,
	OP_TakeOrGive_4          = 0x26,
	OP_SeatTextSml           = 0x27,
	OP_PlayEventVoc          = 0x28,
	OP_DisplayBottom         = 0x29,
	OP_IfMapFlag             = 0x2A,
	OP_SelectRandomChar      = 0x2B,
	OP_GiveEnchanted         = 0x2C,
	OP_ItemType              = 0x2D,
	OP_MakeNothingHere       = 0x2E,
	OP_NoAction_2            = 0x2F,
	OP_ChooseNumeric         = 0x30,
	OP_DisplayBottomTwoLines = 0x31,
	OP_DisplayLarge          = 0x32,
	OP_ExchObj               = 0x33,
	OP_FallToMap             = 0x34,
	OP_DisplayMain           = 0x35,
	OP_Goto                  = 0x36,
	OP_ConfirmWord_2         = 0x37,
	OP_GotoRandom            = 0x38,
	OP_CutsceneEndDarkside   = 0x39,
	OP_CutsceneEdWorld       = 0x3A,
	OP_FlipWorld             = 0x3B,
	OP_PlayCD                = 0x3C
};

enum {
	SCRIPT_ABORT = -1,
	SCRIPT_RESET = -2
};

class XeenEngine;

/**
 * Encapsulates the parameters passed to script opcodes
 */
class EventParameters : public Common::Array<byte> {
public:
	class Iterator {
	private:
		uint _index;
		const EventParameters &_data;
	public:
		Iterator(const EventParameters &owner) : _data(owner), _index(0) {}
		Iterator(const Iterator &it) : _data(it._data), _index(0) {}

		/**
		 * Return a byte
		 */
		byte readByte();

		/**
		 * Return a signed byte
		 */
		int8 readShort() { return (int8)readByte(); }

		/**
		 * Return a word
		 */
		uint16 readUint16LE();

		/**
		 * Return a 32-bit dword
		 */
		uint32 readUint32LE();
	};
public:
	/**
	 * Return an iterator for getting parameters
	 */
	Iterator getIterator() const {
		return Iterator(*this);
	}
};

/**
 * Represents a single event, an instruction of a script
 */
class MazeEvent {
public:
	Common::Point _position;
	int _direction;
	int _line;
	Opcode _opcode;
	EventParameters _parameters;
public:
	MazeEvent();

	/**
	 * Synchronizes the data for the item
	 */
	void synchronize(Common::Serializer &s);
};

/**
 * Represents an entire script that can be triggered within the maze
 */
class MazeEvents : public Common::Array<MazeEvent> {
public:
	Common::StringArray _text;
public:
	/**
	 * Synchronizes the data for the item
	 */
	void synchronize(XeenSerializer &s);
};

/**
 * Holds a single entry in the stack of subroutine call return points
 */
struct StackEntry : public Common::Point {
	int line;

	StackEntry(const Common::Point &pt, int l) : Common::Point(pt), line(l) {}
};

/**
 * Holds a single destination that the mirror can send the player to
 */
struct MirrorEntry {
	Common::String _name;
	int _mapId;
	Common::Point _position;
	int _direction;

	MirrorEntry() : _mapId(0), _direction(DIR_ALL) {}

	bool synchronize(Common::SeekableReadStream &s);
};

/**
 * Overall scripts manager
 */
class Scripts {
private:
	XeenEngine *_vm;
	int _treasureItems;
	int _lineNum;
	int _charIndex;
	int _mirrorId;
	int _refreshIcons;
	int _scriptResult;
	bool _scriptExecuted;
	bool _dirFlag;
	int _windowIndex;
	bool _redrawDone;
	MazeEvent *_event;
	Common::Point _currentPos;
	Common::Stack<StackEntry> _stack;
	Common::String _displayMessage;

	typedef EventParameters::Iterator ParamsIterator;

	/**
	 * Handles executing a given script command
	 */
	bool doOpcode(MazeEvent &event);

	/**
	 * Do nothing
	 */
	bool cmdDoNothing(ParamsIterator &params);

	/**
	 * Display a msesage on-screen
	 */
	bool cmdDisplay1(ParamsIterator &params);

	/**
	 * Displays a door text message using the small font
	 */
	bool cmdDoorTextSml(ParamsIterator &params);

	/**
	 * Displays a door text message using the large font
	 */
	bool cmdDoorTextLrg(ParamsIterator &params);

	/**
	 * Show a sign text on-screen
	 */
	bool cmdSignText(ParamsIterator &params);

	/**
	 * Show an NPC interaction message
	 */
	bool cmdNPC(ParamsIterator &params);

	/**
	 * Play a sound FX
	 */
	bool cmdPlayFX(ParamsIterator &params);

	/**
	 * Handles teleportation
	 */
	bool cmdTeleport(ParamsIterator &params);

	/**
	 * Do a conditional check
	 */
	bool cmdIf(ParamsIterator &params);

	/**
	 * Moves the position of an object
	 */
	bool cmdMoveObj(ParamsIterator &params);

	/**
	 * Take or give amounts from various character or party figures
	 */
	bool cmdTakeOrGive(ParamsIterator &params);

	/**
	 * Removes an object from the playfield
	 */
	bool cmdRemove(ParamsIterator &params);

	/**
	 * Set the currently active character for other script operations
	 */
	bool cmdSetChar(ParamsIterator &params);

	/**
	 * Spawn a monster
	 */
	bool cmdSpawn(ParamsIterator &params);

	/**
	 * Does various things that can be done within towns, like visiting
	 * banks, guilds, etc.
	 */
	bool cmdDoTownEvent(ParamsIterator &params);

	/**
	 * Stop executing the script
	 */
	bool cmdExit(ParamsIterator &params);

	/**
	 * Changes the value for the wall on a given cell
	 */
	bool cmdAlterMap(ParamsIterator &params);

	/**
	 * Gives up to three different item/amounts to various character and/or party properties
	 */
	bool cmdGiveMulti(ParamsIterator &params);

	/**
	 * Prompts the user to enter a word for passwords or mirror
	 * teleport destinations
	 */
	bool cmdConfirmWord(ParamsIterator &params);

	/**
	 * Deals damage to a character
	 */
	bool cmdDamage(ParamsIterator &params);

	/**
	 * Jump if a random number matches a given value
	 */
	bool cmdJumpRnd(ParamsIterator &params);

	/**
	 * Alter an existing event
	 */
	bool cmdAlterEvent(ParamsIterator &params);

	/**
	 * Stores the current location and line for later resuming, and set up to execute
	 * a script at a given location
	 */
	bool cmdCallEvent(ParamsIterator &params);

	/**
	 * Return from executing a script to the script location that previously
	 * called the script
	 */
	bool cmdReturn(ParamsIterator &params);

	/**
	 * Sets variables on characters like race, sex, and class
	 */
	bool cmdSetVar(ParamsIterator &params);

	/**
	 * Play the Clouds endgame
	 */
	bool cmdCutsceneEndClouds(ParamsIterator &params);

	/**
	 * Prompts the user for which character will do an action
	 */
	bool cmdWhoWill(ParamsIterator &params);

	/**
	 * Deals a random amount of damage to a character
	 */
	bool cmdRndDamage(ParamsIterator &params);

	/**
	 * Moves the wall object to the given coordinates. Doesn't change it's orientation.
	 * Wall objects are only visible when viewed straight on, and were never intended
	 * to be anywhere but on squares directly facing walls
	 */
	bool cmdMoveWallObj(ParamsIterator &params);

	/**
	 * Sets the cell flag at the specified X/Y coordinate on the current map
	 */
	bool cmdAlterCellFlag(ParamsIterator &params);

	/**
	 * Sets the word value at the current X/Y location in the HED file
	 * in memory to the given two bytes
	 */
	bool cmdAlterHed(ParamsIterator &params);

	/**
	 * Displays a text string which includes some stat of the currently selected character
	 */
	bool cmdDisplayStat(ParamsIterator &params);

	/**
	 * Displays text in the scene window for various objects
	 * the user interacts with
	 */
	bool cmdSignTextSml(ParamsIterator &params);

	/**
	 * An array of six VOC filenames are hard-coded into the game executable file.
	 * This function plays the VOC file at the specified index in this array
	 */
	bool cmdPlayEventVoc(ParamsIterator &params);

	/**
	 * Displays a large text message across the bottom of the screen
	 */
	bool cmdDisplayBottom(ParamsIterator &params);

	/**
	 * Checks if a given map flag/monster has been set, and if so
	 * jumps to a given line
	 */
	bool cmdIfMapFlag(ParamsIterator &params);

	/**
	 * Selects a random character for further other actions
	 */
	bool cmdSelectRandomChar(ParamsIterator &params);

	/**
	 * Gives an enchanted item to a character
	 */
	bool cmdGiveEnchanted(ParamsIterator &params);

	/**
	 * Sets the item category for used in character operations
	 */
	bool cmdItemType(ParamsIterator &params);

	/**
	 * Disable all the scripts at the party's current position
	 */
	bool cmdMakeNothingHere(ParamsIterator &params);

	/**
	 * Does a copy protection check
	 */
	bool cmdCheckProtection(ParamsIterator &params);

	/**
	 * Given a number of options, and a list of line numbers associated with
	 * those options, jumps to whichever line for the option the user selects
	 */
	bool cmdChooseNumeric(ParamsIterator &params);

	/**
	 * Displays a two line message at the bottom of the screen
	 */
	bool cmdDisplayBottomTwoLines(ParamsIterator &params);

	/**
	 * Displays a message
	 */
	bool cmdDisplayLarge(ParamsIterator &params);

	/**
	 * Exchange the positions of two objects in the maze
	 */
	bool cmdExchObj(ParamsIterator &params);

	/**
	 * Handles making the player fall down to the ground
	 */
	bool cmdFallToMap(ParamsIterator &params);

	/**
	 * Displays a message
	 */
	bool cmdDisplayMain(ParamsIterator &params);

	/**
	 * Jumps to a given line number if the surface at relative cell position 1 matches
	 * a specified surface.
	 * @remarks		This opcode is apparently never actually used
	 */
	bool cmdGoto(ParamsIterator &params);

	/**
	 * Pick a random value from the parameter list and jump to that line number
	 */
	bool cmdGotoRandom(ParamsIterator &params);

	/**
	 * Plays the Dark Side of Xeen ending
	 */
	bool cmdCutsceneEndDarkside(ParamsIterator &params);

	/**
	 * Plays the World of Xeen ending
	 */
	bool cmdCutsceneEndWorld(ParamsIterator &params);

	/**
	 * Switches the player between the Clouds and Dark Side
	 */
	bool cmdFlipWorld(ParamsIterator &params);

	/**
	 * Plays a CD track
	 */
	bool cmdPlayCD(ParamsIterator &params);

	int whoWill(int v1, int v2, int v3);

	/**
	 * Do the Clouds of Xeen ending
	 */
	void doCloudsEnding();

	/**
	 * Do the Dark Side of Xeen ending
	 */
	void doDarkSideEnding();

	/**
	 * Do the World of Xeen combined ending

	 */
	void doWorldEnding();

	/**
	 * Triggers an ending sequence
	 */
	void doEnding(const Common::String &endStr);

	/**
	 * This monstrosity handles doing the various types of If checks on various data
	 */
	bool ifProc(int action, uint32 val, int mode, int charIndex);

	/**
	 * Prompts the user for a copy protection check
	 */
	bool copyProtectionCheck();

	/**
	 * Displays a message
	 */
	void display(bool justifyFlag, int var46);

	/**
	 * Convert a CD time from the World of Xeen playCD opcodes to ScummVM CD frame number (which is at 75Hz)
	 */
	uint convertCDTime(uint srcTime);
public:
	int _animCounter;
	bool _eventSkipped;
	int _whoWill;
	DamageType _nEdamageType;
	Common::Array<MirrorEntry> _mirror;
	Common::String _message;
public:
	Scripts(XeenEngine *vm);

	/**
	 * Checks the event list, triggering any scripts as needed. Also does a
	 * series of checks for updating party status
	 */
	int checkEvents();

	/**
	 * Handles opening grates
	 * @returns		If true, no further event checking should be done
	 */
	bool openGrate(int wallVal, int action);
};

} // End of namespace Xeen

#endif /* XEEN_SCRIPTS_H */
