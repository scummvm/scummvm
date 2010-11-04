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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_OBJECT_H
#define ASYLUM_OBJECT_H

#include "asylum/shared.h"

#include "asylum/system/sound.h"

#include "common/rect.h"
#include "common/stream.h"

namespace Asylum {

class Actor;
class AsylumEngine;

class Object {
public:
	Object(AsylumEngine *engine);
	virtual ~Object();

	//////////////////////////////////////////////////////////////////////////
	// Public variables & accessors
	//////////////////////////////////////////////////////////////////////////
	int32 x;
	int32 y;
	int32 flags;
	int32 actionType;

	void setFrameIndex(int32 index) { _frameIndex = index; }
	void setPriority(int32 priority) { _priority = priority; }

	void setField67C(int32 val) { _field_67C = val; }

	Common::Rect   *getBoundingRect() { return &_boundingRect; }
	uint32          getFrameIndex() { return _frameIndex; }
	uint32          getFrameCount() { return _frameCount; }
	FrameSoundItem *getFrameSoundItem(uint32 index) { assert(index < ARRAYSIZE(_frameSoundItems)); return &_frameSoundItems[index]; }
	int32           getId() { return _id; }
	int32           getPolygonIndex() { return _polygonIndex; }
	int32           getPriority() { return _priority; }
	ResourceId      getResourceId() { return _resourceId; }
	ResourceId      getSoundResourceId() { return _soundResourceId; }

	Common::Rect   *getRect()     { return &_rect; }
	int32           getField67C() { return _field_67C; }
	int32           getField688() { return _field_688; }

	/////////////////////////////////////////////////////////////////////////
	// Loading & disabling
	/////////////////////////////////////////////////////////////////////////

	/**
	 * Loads the object data
	 *
	 * @param stream If non-null, the Common::SeekableReadStream to load from
	 */
	void load(Common::SeekableReadStream *stream);

	/**
	 * Sets the object disabled flag
	 */
	void disable();

	/**
	 * Sets the object destroyed flag and remove this object from the graphics queue
	 */
	void disableAndRemoveFromQueue();

	/////////////////////////////////////////////////////////////////////////
	// Visibility
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Query if this object is on screen.
	 *
	 * @return true if on screen, false if not.
	 */
	bool isOnScreen();

	/////////////////////////////////////////////////////////////////////////
	// Drawing & update
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Draws the object
	 */
	void draw();

	/**
	 * Updates the object.
	 */
	void update();

	/**
	 * Sets the next frame.
	 *
	 * @param flags The flags.
	 */
	void setNextFrame(int flags);

	/////////////////////////////////////////////////////////////////////////
	// Misc
	////////////////////////////////////////////////////////////////////////

	/**
	 * Stop all object sounds (called from scripts)
	 */
	void stopAllSounds();

	/**
	 * Checks the object flags
	 *
	 * @return true if it succeeds, false if it fails.
	 */
	bool checkFlags();

	/**
	 * Convert this object into a string representation.
	 *
	 * @return A string representation of this object.
	 */
	Common::String toString();

private:
	AsylumEngine *_vm;

	int32 _index;   ///< our index

	//////////////////////////////////////////////////////////////////////////
	// Data
	//////////////////////////////////////////////////////////////////////////
	int32		   _id;
	ResourceId     _resourceId;
	// x, y
	Common::Rect   _boundingRect;
	int32		   _field_20;
	uint32		   _frameIndex;
	uint32		   _frameCount;
	int32		   _field_2C;
	int32		   _field_30;
	int32		   _field_34;
	// flags
	int32		   _field_3C;
	uint8		   _name[52];
	Common::Rect   _rect;
	int32		   _polygonIndex;
	// actionType
	GameFlag	   _gameFlags[10];
	int32		   _field_B4;
	int32		   _tickCount;
	int32		   _tickCount2;
	int32		   _field_C0;
	int32		   _priority;
	int32		   _actionListIdx;
	SoundItem	   _soundItems[16];
	FrameSoundItem _frameSoundItems[50];
	int32		   _field_67C;
	int32		   _soundX;
	int32		   _soundY;
	int32		   _field_688;
	int32		   _randomResourceIds[5];
	ResourceId     _soundResourceId;
	int32		   _field_6A4;

	/**
	 * Query if this object is visible.
	 *
	 * @return true if visible, false if not.
	 */
	bool isVisible();

	/**
	 * Play object sounds
	 */
	void playSounds();

	/**
	 * Check if any items in the object sound array are playing,
	 * and based on their flag values, stop them accordingly
	 */
	void updateSoundItems();

	/**
	 * Stop the object related sounds
	 */
	void stopSound();

	/**
	 * Gets a random resource identifier for this object (using the list in _randomResourceIds)
	 *
	 * @return The random resource identifier.
	 */
	int32 getRandomResourceId();

}; // end of class Object

} // end of namespace Asylum

#endif // ASYLUM_OBJECT_H
