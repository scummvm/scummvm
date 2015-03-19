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

#ifndef SHERLOCK_SPRITES_H
#define SHERLOCK_SPRITES_H

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

#define MAX_HOLMES_SEQUENCE 16
#define MAX_FRAME 30

struct Sprite {
	Common::String _name;				// Name
	Common::String _description;		// Description
	Common::StringArray _examine;		// Examine in-depth description
	Common::String _pickUp;				// Message for if you can't pick up object

	const uint8 (*_sequences)[MAX_HOLMES_SEQUENCE][MAX_FRAME];  // Holds animation sequences
	Sprite *_sprites;					// Sprite shapes
	ImageFrame *_spriteFrame;			// Pointer to shape in the sprite
	int _walkCount;						// Character walk counter
	int _allow;							// Allowed menu commands - ObjectAllow
	int _frameNumber;					// Frame number in rame sequence to draw
	int _sequenceNumber;				// Sequence being used
	Common::Point _position;			// Current position
	Common::Point _movement;			// Momvement amount
	Common::Point _oldPosition;			// Old position
	Common::Point _oldSize;				// Image's old size
	Common::Point _goto;				// Walk destination
	SpriteType _type;					// Type of object
	int _pickup;
	Common::Point _noShapeSize;			// Size of a NO_SHAPE
	int _status;						// Status: open/closed, moved/not moved
	byte _misc;							// Miscellaneous use
	int _numFrames;						// How many frames the object has
};

} // End of namespace Sherlock

#endif
