/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#ifndef PARALLACTION_ZONE_H
#define PARALLACTION_ZONE_H

#include "parallaction/defs.h"

namespace Parallaction {

enum ZoneTypes {
	kZoneExamine	   = 1, 				// zone displays comment if activated
	kZoneDoor		   = 2, 				// zone activated on click (after some walk if needed)
	kZoneGet		   = 4, 				// for pickable items
	kZoneMerge		   = 8, 				// tags items which can be merged in inventory
	kZoneTaste		   = 0x10,				// NEVER USED
	kZoneHear		   = 0x20,				// NEVER USED: they ran out of time before integrating sfx
	kZoneFeel		   = 0x40,				// NEVER USED
	kZoneSpeak		   = 0x80,				// tags NPCs the character can talk with
	kZoneNone		   = 0x100, 			// used to prevent parsing on peculiar Animations
	kZoneTrap		   = 0x200, 			// zone activated when character enters
	kZoneYou		   = 0x400, 			// marks the character
	kZoneCommand	   = 0x800
};


enum ZoneFlags {
	kFlagsClosed		= 1,				// Zone: door is closed / switch is off
	kFlagsActive		= 2,				// Zone/Animation: object is visible
	kFlagsRemove		= 4,				// Zone/Animation: object is soon to be removed
	kFlagsActing		= 8,				// Animation: script execution is active
	kFlagsLocked		= 0x10, 			// Zone: door or switch cannot be toggled
	kFlagsFixed 		= 0x20, 			// Zone: Zone item cannot be picked up
	kFlagsNoName		= 0x40, 			// Zone with no name (used to prevent some kEvEnterZone events)
	kFlagsNoMasked		= 0x80, 			// Animation is to be drawn ignoring z buffer
	kFlagsLooping		= 0x100,			// Animation: script is to be executed repeatedly
	kFlagsAdded 		= 0x200,			// NEVER USED in Nippon Safes
	kFlagsCharacter 	= 0x400,			//
	kFlagsNoWalk		= 0x800 			// Zone: character doesn't need to walk towards object to interact
};


#define NUM_ANSWERS		 5

struct Question {
	char*		_text;
	char*		_answers[NUM_ANSWERS];
	uint16		_mood;
	uint16		_answer_moods[NUM_ANSWERS];
	union {
		Question*	_questions[NUM_ANSWERS];
		char*		_names[NUM_ANSWERS];
	} _following;
	Command*	_commands[NUM_ANSWERS];
	uint32		_noFlags[NUM_ANSWERS];
	uint32		_yesFlags[NUM_ANSWERS];

	Question() {
		_text = NULL;
		_mood = 0;

		for (uint32 i = 0; i < NUM_ANSWERS; i++) {
			_answers[i] = NULL;
			_answer_moods[i] = 0;
			_following._questions[i] =  NULL;
			_commands[i] = NULL;
			_noFlags[i] = 0;
			_yesFlags[i] = 0;
		}
	}
};

struct GetData {	// size = 24
	uint32			_icon;
	StaticCnv		_cnv;
	byte		   *_backup;
	uint16			field_14;		// unused
	uint16			field_16;		// unused

	GetData() {
		_icon = 0;
		_backup = NULL;
	}
};
struct SpeakData {	// size = 36
	char		_name[32];
	Dialogue	*_dialogue;

	SpeakData() {
		_name[0] = '\0';
		_dialogue = NULL;
	}
};
struct ExamineData {	// size = 28
	StaticCnv	_cnv;
	uint16		_opBase;		   // unused
	uint16		field_12;			// unused
	char*		_description;
	char*		_filename;

	ExamineData() {
		_opBase = 0;
		_description = NULL;
		_filename = NULL;
	}
};
struct DoorData {	// size = 28
	char*	_location;
	Cnv 	_cnv;
	byte*	_background;
	Common::Point	_startPos;
	uint16	_startFrame;

	DoorData() {
		_location = NULL;
		_background = NULL;
		_startFrame = 0;
	}
};
struct HearData {	// size = 20
	char		_name[20];

	HearData() {
		_name[0] = '\0';
	}
};
struct MergeData {	// size = 12
	uint32	_obj1;
	uint32	_obj2;
	uint32	_obj3;

	MergeData() {
		_obj1 = _obj2 = _obj3 = 0;
	}
};

struct TypeData {
	GetData 	*get;
	SpeakData	*speak;
	ExamineData *examine;
	DoorData	*door;
	HearData	*hear;
	MergeData	*merge;

	TypeData() {
		get = NULL;
		speak = NULL;
		examine = NULL;
		door = NULL;
		hear = NULL;
		merge = NULL;
	}
};

struct Label {
	char*			_text;
	StaticCnv		_cnv;

	Label() {
		_text = NULL;
	}
};

struct Zone : public Node {

	int16 		_left;
	int16		_top;
	int16		_right;
	int16		_bottom;
	int16		_oldLeft;
	int16		_oldTop;

	uint32			_type;
	uint32			_flags;
	Label       _label;
	uint16			field_2C;		// unused
	uint16			field_2E;		// unused
	TypeData	u;
	Command 		*_commands;
	Common::Point	_moveTo;

	Zone() {
		_left = _top = _right = _bottom = 0;
		_oldLeft = _oldTop = 0;

		_type = 0;
		_flags = 0;
		_commands = NULL;
	}

	void getRect(Common::Rect& r) const {
		r.left = _left;
		r.right = _right;
		r.top = _top;
		r.bottom = _bottom;
	}

	void translate(int16 x, int16 y) {
		_left += x;
		_right += x;
		_top += y;
		_bottom += y;
	}

	uint16 width() const {
		return _right - _left;
	}

	uint16 height() const {
		return _bottom - _top;
	}
};

struct LocalVariable {
	int16		_value;
	int16		_min;
	int16		_max;

	LocalVariable() {
		_value = 0;
		_min = -10000;
		_max = 10000;
	}
};

union LValue {
	int16			_value;
	int16*			_pvalue;
	LocalVariable*	_local;

	LValue() {
		_local = NULL;
	}
};

enum InstructionFlags {
	kInstUsesLiteral	= 1,
	kInstUsesLocal		= 2,
	kInstMod			= 4,
	kInstMaskedPut		= 8
};

struct Instruction : public Node {
	uint32	_index;
	uint32	_flags;
	struct {
		Animation	*_a;
		Zone		*_z;
		uint32		_index;
		LValue		_loopCounter;
	} _opBase;
	LValue	_opA;
	LValue	_opB;

	Instruction() {
		_index = 0;
		_flags = 0;
		_opBase._a = NULL;
	}
};


struct Program : public Node {
	LocalVariable	*_locals;
	uint16			_loopCounter;
	Instruction 	*_ip;
	Instruction 	*_loopStart;

	Program() {
		_locals = NULL;
		_loopCounter = 0;
		_ip = NULL;
		_loopStart = NULL;
	}
};



struct Animation {
	Zone		_zone;
	Program 	*_program;
	Cnv 		*_cnv;
	int16		_frame;
	uint16		field_50;		// unused
	int16		_z;
	uint16		field_54;		// unused
	uint16		field_56;		// unused
	uint16		field_58;		// unused
	uint16		field_5A;		// unused
	uint16		field_5C;		// unused
	uint16		field_5E;		// unused

	Animation() {

		// FIXME: temporary hack until Animation become a subclass of Zone
		_zone._left = _zone._top = _zone._right = _zone._bottom = 0;
		_zone._oldLeft = _zone._oldTop = 0;

		_zone._type = 0;
		_zone._flags = 0;
		_zone._commands = NULL;




		_program = NULL;
		_frame = 0;
		_z = 0;
	}

	uint16 width() const {
		if (!_cnv) return 0;
		return _cnv->_width;
	}

	uint16 height() const {
		if (!_cnv) return 0;
		return _cnv->_height;
	}

	uint16 getFrameNum() const {
		if (!_cnv) return 0;
		return _cnv->_count;
	}

	byte* getFrameData(uint32 index) const {
		if (!_cnv) return NULL;
		return _cnv->getFramePtr(index);
	}
};


uint16	runZone(Zone *z);
void	dropItem(uint16 v);
int16	pickupItem(Zone *z);


uint16	checkDoor();


void		loadProgram(Animation *, char *filename);



} // namespace Parallaction

#endif
