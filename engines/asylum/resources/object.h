/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ASYLUM_RESOURCES_OBJECT_H
#define ASYLUM_RESOURCES_OBJECT_H

#include "common/rect.h"
#include "common/serializer.h"
#include "common/stream.h"

#include "asylum/shared.h"

#include "asylum/system/sound.h"

namespace Asylum {

class Actor;
class AsylumEngine;

class Object : public Common::Serializable {
public:
	Object(AsylumEngine *engine);
	virtual ~Object() {};

	//////////////////////////////////////////////////////////////////////////
	// Public variables & accessors
	//////////////////////////////////////////////////////////////////////////
	int16 x;
	int16 y;
	uint32 flags;
	int32 actionType;

	void setFrameIndex(int32 index) { _frameIndex = index; }
	void setPriority(int32 priority) { _priority = priority; }
	void setSoundResourceId(ResourceId id) { _soundResourceId = id; }

	void setTransparency(int32 val) { _transparency = val; }
	void setField6A4(ActorDirection val) { _field_6A4 = val; }

	Common::Rect   *getBoundingRect()               { return &_boundingRect; }
	uint32          getFrameIndex()                 { return _frameIndex; }
	uint32          getFrameCount()                 { return _frameCount; }
	FrameSoundItem *getFrameSoundItem(uint32 index) { assert(index < ARRAYSIZE(_frameSoundItems)); return &_frameSoundItems[index]; }
	ObjectId        getId()                         { return _id; }
	uint32           getPolygonIndex()              { return _polygonIndex; }
	int32           getPriority()                   { return _priority; }
	ResourceId      getResourceId()                 { return _resourceId; }
	ResourceId      getSoundResourceId()            { return _soundResourceId; }

	Common::Rect   *getRect()         { return &_rect; }
	int32           getScriptIndex()  { return _scriptIndex; }
	int32           getSoundY()       { return _soundCoords.y; }
	int32           getTransparency() { return _transparency; }
	int32           getField688()     { return _field_688; }
	ActorDirection  getField6A4()     { return _field_6A4; }

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

	/**
	 * Adjust coordinates.
	 *
	 * @param point The point.
	 */
	void adjustCoordinates(Common::Point *point);

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
	void setNextFrame(uint32 flags);

	/////////////////////////////////////////////////////////////////////////
	// Misc
	////////////////////////////////////////////////////////////////////////

	/**
	 * Stop the object related sounds
	 */
	void stopSound();

	/**
	 * Stop all object sounds (called from scripts)
	 */
	void stopAllSounds();

	/**
	 * Checks the object flags
	 *
	 * @return true if it succeeds, false if it fails.
	 */
	bool checkFlags() const;

	char *getName() { return _name; }
	/**
	 * Convert this object into a string representation.
	 *
	 * @param shortString toggle for displaying the full dump or just a
	 *                    short summary string
	 * @return A string representation of this object.
	 */
	Common::String toString(bool shortString = true);

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &s);

private:
	AsylumEngine *_vm;

	//int32 _index;   ///< our index

	//////////////////////////////////////////////////////////////////////////
	// Data
	//////////////////////////////////////////////////////////////////////////
	ObjectId       _id;
	ResourceId     _resourceId;
	// x, y
	Common::Rect   _boundingRect;
	int32          _field_20;
	uint32         _frameIndex;
	uint32         _frameCount;
	int32          _field_2C;
	int32          _field_30;
	int32          _field_34;
	// flags
	int32          _field_3C;
	char           _name[52];
	Common::Rect   _rect;
	uint32         _polygonIndex;
	// actionType
	int32          _gameFlags[10];
	int32          _field_B4;
	uint32         _tickCount;
	uint32         _tickCount2;
	uint32         _field_C0;
	int32          _priority;
	int32          _scriptIndex;
	SoundItem      _soundItems[16];
	FrameSoundItem _frameSoundItems[50];
	int32          _transparency;
	Common::Point  _soundCoords;
	int32          _field_688;
	ResourceId     _randomResourceIds[5];
	ResourceId     _soundResourceId;
	ActorDirection _field_6A4;

	/**
	 * Query if this object is visible.
	 *
	 * @return true if visible, false if not.
	 */
	bool isVisible() const;

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
	 * Sets the volume.
	 */
	void setVolume();

	/**
	 * Gets a random resource identifier for this object (using the list in _randomResourceIds)
	 *
	 * @return The random resource identifier.
	 */
	ResourceId getRandomResourceId();

}; // end of class Object

} // end of namespace Asylum

#endif // ASYLUM_RESOURCES_OBJECT_H
