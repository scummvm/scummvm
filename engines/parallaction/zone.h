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




struct Question {
	char*		_text;
	char*		_answers[5];
	uint16		_mood;
	uint16		_answer_moods[5];
	union {
		Question*	_questions[5];
		char*		_names[5];
	} _following;
	Command*	_commands[5];
	uint32		_noFlags[5];
	uint32		_yesFlags[5];
};

struct GetData {	// size = 24
	uint32			_icon;
	StaticCnv		_cnv;
	uint16			field_14;		// unused
	uint16			field_16;		// unused
};
struct SpeakData {	// size = 36
	char		_name[32];
	Dialogue	*_dialogue;
};
struct ExamineData {	// size = 28
	StaticCnv	_cnv;
	uint16		_opBase;		   // unused
	uint16		field_12;			// unused
	char*		_description;
	char*		_filename;
};
struct DoorData {	// size = 28
	char*	_location;
	Cnv 	_cnv;
	byte*	_background;
	Point	_startPos;
	uint16	_startFrame;
};
struct HearData {	// size = 20
	char		_name[20];
};
struct MergeData {	// size = 12
	uint32	_obj1;
	uint32	_obj2;
	uint32	_obj3;
};

struct ZoneTypeData {
	GetData 	*get;
	SpeakData	*speak;
	ExamineData *examine;
	DoorData	*door;
	HearData	*hear;
	MergeData	*merge;
};

struct ZoneLabel {
	char*			_text;
	StaticCnv		_cnv;
};

struct Zone {
	Node			_node;
	union {
		Rect		_limits;		// for zones
		struct {	// for animations
			Point	_position;
			Point	_oldposition;
		} pos;
	};
	uint32			_type;
	uint32			_flags;
//	char*			_labeltext;
//	StaticCnv		_labelcnv;
	ZoneLabel       _label;
	uint16			field_2C;		// unused
	uint16			field_2E;		// unused
	ZoneTypeData	u;
	Command 		*_commands;
	Point			_moveTo;
};

struct LocalVariable {
	int16		_value;
	int16		_min;
	int16		_max;
};

union LValue {
	int16			_value;
	int16*			_pvalue;
	LocalVariable*	_local;
};

enum InstructionFlags {
	kInstUsesLiteral	= 1,
	kInstUsesLocal		= 2,
	kInstMod			= 4,
	kInstMaskedPut		= 8
};

struct Instruction {
	Node	_node;
	uint32	_index;
	uint32	_flags;
	union {
		Animation	*_a;
		Zone		*_z;
		uint32		_index;
		LValue		_loopCounter;
	} _opBase;
	LValue	_opA;
	LValue	_opB;
};


struct Program {
	Node			_node;
	LocalVariable	*_locals;
	uint16			_loopCounter;
	Instruction 	*_ip;
	Instruction 	*_loopStart;
};



struct Animation {
	Zone		_zone;
	Program 	*_program;
	Cnv 		_cnv;
	int16		_frame;
	uint16		field_50;		// unused
	int16		_z;
	uint16		field_54;		// unused
	uint16		field_56;		// unused
	uint16		field_58;		// unused
	uint16		field_5A;		// unused
	uint16		field_5C;		// unused
	uint16		field_5E;		// unused
};

extern Node _zones;
extern Node _animations;


Zone   *findZone(const char *name);
uint16	runZone(Zone *z);
void	dropItem(uint16 v);
int16	pickupItem(Zone *z);


Zone   *hitZone(uint32 type, uint16 x, uint16 y);
uint16	checkDoor();


Animation  *findAnimation(const char *name);
void		loadProgram(Animation *, char *filename);
int16		queryBackgroundLayer(int16 v);


} // namespace Parallaction

#endif
