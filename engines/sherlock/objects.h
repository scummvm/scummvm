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

#ifndef SHERLOCK_OBJECTS_H
#define SHERLOCK_OBJECTS_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/str-array.h"
#include "common/str.h"
#include "sherlock/resources.h"

namespace Sherlock {

class SherlockEngine;

enum ObjectAllow {
	ALLOW_MOVEMENT = 1, ALLOW_OPEN = 2, ALLOW_CLOSE = 4
};

enum SpriteType {
	INVALID = 0,
	CHARACTER = 1,
	CURSOR = 2,
	STATIC_BG_SHAPE = 3,		// Background shape that doesn't animate
	ACTIVE_BG_SHAPE = 4,		// Background shape that animates
	REMOVE = 5,					// Object should be removed next frame
	NO_SHAPE = 6,				// Background object with no shape
	HIDDEN = 7,					// Hidden backgruond object
	HIDE_SHAPE = 8				// Object needs to be hidden
};

enum AType {
	OBJECT		= 0,
	PERSON		= 1,
	SOLID		= 2,
	TALK		= 3,  // Standard talk zone
	FLAG_SET	= 4,
	DELTA		= 5,
	WALK_AROUND	= 6,
	TALK_EVERY	= 7,  // Talk zone that turns on every room visit
	TALK_MOVE	= 8,  // Talk zone that only activates when Holmes moves
	PAL_CHANGE	= 9,  // Changes the palette down so that it gets darker
	PAL_CHANGE2	= 10, // Same as PAL_CHANGE, except that it goes up
	SCRIPT_ZONE	= 11, // If this is clicked in, it is activated
	BLANK_ZONE	= 12, // This masks out other objects when entered
	NOWALK_ZONE = 13  // Player cannot walk here
};

#define MAX_HOLMES_SEQUENCE 16
#define MAX_FRAME 30

// code put into sequences to defines 1-10 type seqs
#define SEQ_TO_CODE 67			
#define FLIP_CODE (64 + 128)
#define SOUND_CODE (34 + 128)

class Sprite {
private:
	static SherlockEngine *_vm;
public:
	Common::String _name;				// Name
	Common::String _description;		// Description
	Common::StringArray _examine;		// Examine in-depth description
	Common::String _pickUp;				// Message for if you can't pick up object

	const uint8 (*_sequences)[MAX_HOLMES_SEQUENCE][MAX_FRAME];  // Holds animation sequences
	ImageFile *_images;					// Sprite images
	ImageFrame *_imageFrame;			// Pointer to shape in the images
	int _walkCount;						// Character walk counter
	int _allow;							// Allowed menu commands - ObjectAllow
	int _frameNumber;					// Frame number in rame sequence to draw
	int _sequenceNumber;				// Sequence being used
	Common::Point _position;			// Current position
	Common::Point _delta;				// Momvement delta
	Common::Point _oldPosition;			// Old position
	Common::Point _oldSize;				// Image's old size
	Common::Point _goto;				// Walk destination
	SpriteType _type;					// Type of object
	int _pickup;
	Common::Point _noShapeSize;			// Size of a NO_SHAPE
	int _status;						// Status: open/closed, moved/not moved
	int8 _misc;							// Miscellaneous use
	int _numFrames;						// How many frames the object has
public:
	Sprite() { clear(); }
	static void setVm(SherlockEngine *vm) { _vm = vm; }

	void clear();

	void setImageFrame();

	void adjustSprite();
};

struct ActionType {
	char _cAnimNum;
	char _cAnimSpeed;				// if high bit set, play in reverse
	Common::String _names[4];

	void synchronize(Common::SeekableReadStream &s);
};

struct UseType {
	int _cAnimNum;
	int _cAnimSpeed;				// if high bit set, play in reverse
	Common::String _names[4];
	int _useFlag;					// Which flag USE will set (if any)
	int _dFlag[1];
	int _lFlag[2];
	Common::String _target;

	void synchronize(Common::SeekableReadStream &s);
};

struct Object {
	Common::String _name;			// Name
	Common::String _description;	// Description
	Common::String _examine;		// Examine in-depth description
	int _sequenceOffset;
	uint8 *_sequences;				// Holds animation sequences
	ImageFile *_images;				// Sprite images
	ImageFrame *_imageFrame;		// Pointer to shape in the images
	int _walkCount;					// Character walk counter
	int _allow;						// Allowed menu commands - ObjectAllow
	int _frameNumber;				// Frame number in rame sequence to draw
	int _sequenceNumber;			// Sequence being used
	SpriteType _type;				// Object type
	Common::Point _position;		// Current position
	Common::Point _movement;		// Momvement amount
	Common::Point _oldPosition;		// Old position
	Common::Point _oldSize;			// Image's old size
	Common::Point _goto;			// Walk destination

	int _pickup;
	int _defaultCommand;			// Default right-click command
	int _lookFlag;					// Which flag LOOK   will set (if any)
	int _pickupFlag;				// Which flag PICKUP will set (if any)
	int _requiredFlag;				// Object will be hidden if not set
	Common::Point _noShapeSize;		// Size of a NO_SHAPE
	int _status;					// Status (open/closed, moved/not)
	int8 _misc;						// Misc field -- use varies with type
	int _maxFrames;					// Number of frames
	int _flags;						// Tells if object can be walked behind
	ActionType _aOpen;				// Holds data for moving object
	AType _aType;					// Tells if this is an object, person, talk, etc.
	int _lookFrames;				// How many frames to play of the look anim before pausing
	int _seqCounter;				// How many times this sequence has been executed
	Common::Point _lookPosition;	// Where to walk when examining object
	int _lookFacing;				// Direction to face when examining object
	int _lookcAnim;
	ActionType _aClose;
	int _seqStack;					// Allows gosubs to return to calling frame
	int _seqTo;						// Allows 1-5, 8-3 type sequences encoded in 2 bytes
	uint _descOffset;					// Tells where description starts in DescText
	int _seqcounter2;				// Counter of calling frame sequence
	uint _seqSize;					// Tells where description starts 
	ActionType _aMove;
	UseType _use[4];

	void synchronize(Common::SeekableReadStream &s);

	void toggleHidden();
};

struct CAnim {
	Common::String _name;			// Name
	int _sequences[MAX_FRAME];		// Animation sequences
	Common::Point _position;		// Position
	int _size;						// Size of uncompressed animation
	SpriteType _type;
	int _flags;						// Tells if can be walked behind
	Common::Point _goto;			// coords holmes should walk to before starting canim
	int _sequenceNumber;
	Common::Point _teleportPos;		// Location Holmes shoul teleport to after
	int _teleportS;					// playing canim

	void synchronize(Common::SeekableReadStream &s);
};

struct InvGraphicType {
	ImageFile *_images;				// Object images
	int _maxFrames;					// How many frames in object
	int _filesize;					// File size

	InvGraphicType();
} ;

} // End of namespace Sherlock

#endif
