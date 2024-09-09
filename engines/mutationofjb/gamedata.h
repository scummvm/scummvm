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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MUTATIONOFJB_GAMEDATA_H
#define MUTATIONOFJB_GAMEDATA_H

#include "mutationofjb/inventory.h"

#include "common/serializer.h"
#include "common/scummsys.h"

namespace Common {
class ReadStream;
}

namespace MutationOfJB {

enum {
	MAX_ENTITY_NAME_LENGTH = 0x14
};

/** @file
 * There are 4 types of entities present in the game data:
 * - Door
 * - Object
 * - Static
 * - Bitmap
 */

/**
 * An interactable scene changer with no visual representation.
 */
struct Door : public Common::Serializable {
	/**
	 * Door name (NM register).
	 *
	 * Can be empty - deactivates door completely (you can't mouse over or interact with it at all).
	 *
	 * If it ends with '+', using the "go" verb on the door will not implicitly change the scene,
	 * but the player will still walk towards the door.
	 */
	char _name[MAX_ENTITY_NAME_LENGTH + 1];
	/**
	 * Scene ID where the door leads (LT register).
	 * Can be 0 - you can hover your mouse over it, but clicking it doesn't do anything (unless scripted).
	 */
	uint8  _destSceneId;
	/** X coordinate for player's position after going through the door (SX register). */
	uint16 _destX;
	/** Y coordinate for player's position after going through the door (SY register). */
	uint16 _destY;
	/** X coordinate of the door rectangle (XX register). */
	uint16 _x;
	/** Y coordinate of the door rectangle (YY register). */
	uint8  _y;
	/** Width of the door rectangle (XL register). */
	uint16 _width;
	/** Height of the door rectangle (YL register). */
	uint8  _height;
	/** X coordinate for position player will walk towards after clicking the door (WX register). */
	uint16 _walkToX;
	/** Y coordinate for position player will walk towards after clicking the door (WY register). */
	uint8  _walkToY;
	/**
	 * Encoded player frames.
	 *   4 bits - destFrame
	 *   4 bits - walkToFrame
	 */
	uint8  _SP;

	/**
	 * Check if this door can be interacted with.
	 * @return True if this door can be interacted with, false otherwise.
	 */
	bool isActive();

	/**
	 * Load initial state from game data file.
	 *
	 * @param stream Stream for reading.
	 * @return True if success, false otherwise.
	 */
	bool loadInitialState(Common::ReadStream &stream);

	/**
	 * (De)serialization for save/load.
	 *
	 * @param sz Serializer.
	 */
	void saveLoadWithSerializer(Common::Serializer &sz) override;

	/**
	 * Check whether walk action used on this door causes implicit scene change.
	 *
	 * @return True if door implicitly changes current scene, false otherwise.
	 */
	bool allowsImplicitSceneChange() const;
};

/**
 * An animated image in the scene.
 *
 * Object frames consist of surfaces carved out of room frames (starting from _roomFrame
 * up until _roomFrame + _numFrames - 1) based on the object's rectangle. They are stored
 * in the shared object frame space that each object occupies a continuous part of from
 * the beginning.
 *
 * By using the term "frame" alone we will be referring to an object frame, not a room
 * frame.
 *
 * For details regarding animation playback, see objectanimationtask.cpp.
 */
struct Object : public Common::Serializable {
	/** Controls whether the animation is playing. */
	uint8  _active;
	/**
	 * Number of the first frame this object has in the shared object frame space (FA register).
	 *
	 * For the first object, it is equal to 1.
	 * For any subsequent object, it is equal to (_firstFrame + _numFrames) of the previous object.
	 *
	 * @note The numbering starts from 1.
	 * @note Technically this field is useless because it can be calculated.
	 */
	uint8  _firstFrame;
	/**
	 * The frame that is jumped to randomly based on _jumpChance (FR register).
	 *
	 * @note Numbered from 1 and relative to _firstFrame.
	 * @note A value of 0 disables randomness completely.
	 * @see objectanimationtask.cpp
	 * @see _jumpChance
	 */
	uint8  _randomFrame;
	/** Number of animation frames (NA register). */
	uint8  _numFrames;
	/**
	 * Low 8 bits of the 16-bit starting room frame (FS register).
	 * This is in the room frame space.
	 *
	 * @see _roomFrameMSB
	 */
	uint8  _roomFrameLSB;
	/**
	 * Chance (1 in x) of the animation jumping to _randomFrame.
	 *
	 * @see objectanimationtask.cpp
	*/
	uint8  _jumpChance;
	/**
	 * Current animation frame (CA register).
	 *
	 * @note Index in the shared object frame space. Numbered from 1.
	 */
	uint8  _currentFrame;
	/** X coordinate of the object rectangle (XX register). */
	uint16 _x;
	/** Y coordinate of the object rectangle (YY register). */
	uint8  _y;
	/** Width of the object rectangle (XL register). */
	uint16 _width;
	/** Height of the object rectangle (YL register). */
	uint8  _height;
	/** A general-purpose register for use in scripts. Nothing to do with animation. */
	uint16 _WX;
	/**
	 * High 8 bits of the 16-bit starting room frame (WY register).
	 * This is in the room frame space.
	 *
	 * @see _roomFrameLSB
	 */
	uint8  _roomFrameMSB;
	/** Unknown. TODO: Figure out what this does. */
	uint8  _SP;

	/**
	 * Load initial state from game data file.
	 *
	 * @param stream Stream for reading.
	 * @return True if success, false otherwise.
	 */
	bool loadInitialState(Common::ReadStream &stream);

	/**
	 * (De)serialization for save/load.
	 *
	 * @param sz Serializer.
	 */
	void saveLoadWithSerializer(Common::Serializer &sz) override;
};

/**
 * An interactable area, usually without a visual representation.
 */
struct Static : public Common::Serializable {
	/** Whether you can mouse over and interact with the static (AC register). */
	uint8  _active;
	/**
	 * Static name (NM register).
	 *
	 * If it starts with '~', the static has an implicit "pickup" action that adds
	 * an item with the same name (except '`' replaces '~') to your inventory and
	 * disables the static. If there is a matching scripted "pickup" action, it
	 * overrides the implicit action. This kind of static also has graphics in the
	 * form of its rectangle extracted from room frame 2 (and 3 after pickup).
	 *
	 * If it ends with '[', the "use" action allows combining the static with another
	 * entity.
	 *
	 * TODO: Support '~' statics.
	 */
	char _name[MAX_ENTITY_NAME_LENGTH + 1];
	/** X coordinate of the static rectangle (XX register). */
	uint16 _x;
	/** Y coordinate of the static rectangle (YY register). */
	uint8  _y;
	/** Width of the static rectangle (XL register). */
	uint16 _width;
	/** Height of the static rectangle (YL register). */
	uint8  _height;
	/** X coordinate of the position the player will walk towards after clicking the static (WX register). */
	uint16 _walkToX;
	/** Y coordinate of the position the player will walk towards after clicking the static (WY register). */
	uint8  _walkToY;
	/** Player frame (rotation) set after the player finishes walking towards the walk to position (SP register). */
	uint8  _walkToFrame;

	/**
	 * Load initial state from game data file.
	 *
	 * @param stream Stream for reading.
	 * @return True if success, false otherwise.
	 */
	bool loadInitialState(Common::ReadStream &stream);

	/**
	 * (De)serialization for save/load.
	 *
	 * @param sz Serializer.
	 */
	void saveLoadWithSerializer(Common::Serializer &sz) override;

	/**
	 * Check whether this static is combinable.
	 * Statics with names ending with '[' are allowed to be combined with other items.
	 *
	 * @return True if combinable, false otherwise.
	 */
	bool isCombinable() const;

	/**
	 * Check whether this static is implicitly picked up.
	 * Statics with names starting with '~' are implicitly picked up.
	 *
	 * @return Returns true if this static is implicitly picked up by pick up action, false otherwise.
	 */
	bool allowsImplicitPickup() const;
};

/**
 * A static image that is carved out of a room frame based on its rectangle.
 * The bitmap rectangle also specifies where to blit it on the screen.
 */
struct Bitmap : public Common::Serializable {
	/** Room frame that this bitmap carves out of. */
	uint8  _roomFrame;
	/** Whether to draw the bitmap. */
	uint8  _isVisible;
	/** X coordinate of the top left corner of the bitmap rectangle. */
	uint16 _x1;
	/** Y coordinate of the top left corner of the bitmap rectangle. */
	uint8  _y1;
	/** X coordinate of the bottom right corner of the bitmap rectangle. */
	uint16 _x2;
	/** Y coordinate of the bottom right corner of the bitmap rectangle. */
	uint8  _y2;

	/**
	 * Load initial state from game data file.
	 *
	 * @param stream Stream for reading.
	 * @return True if success, false otherwise.
	 */
	bool loadInitialState(Common::ReadStream &stream);

	/**
	 * (De)serialization for save/load.
	 *
	 * @param sz Serializer.
	 */
	void saveLoadWithSerializer(Common::Serializer &sz) override;
};

/**
 * Encoded exhausted convesation item.
 */
struct ExhaustedConvItem {
	/**
	 * 1 bit - context.
	 * 3 bits - conversation item index.
	 * 4 bits - conversation group index.
	 */
	uint8 _encodedData;

	uint8 getContext() const {
		return (_encodedData >> 7) & 0x1;
	}
	uint8 getConvItemIndex() const {
		return (_encodedData >> 4) & 0x7;
	}
	uint8 getConvGroupIndex() const {
		return _encodedData & 0xF;
	}

	ExhaustedConvItem() : _encodedData(0) {}
	ExhaustedConvItem(uint8 context, uint8 convItemIndex, uint8 convGroupIndex) :
		_encodedData(((context & 0x1) << 7) | ((convItemIndex & 0x7) << 4) | (convGroupIndex & 0xF)) {}
};

struct Scene : Common::Serializable {
	Door *getDoor(uint8 objectId);
	Object *getObject(uint8 objectId, bool ignoreNo = false);
	Static *getStatic(uint8 staticId, bool ignoreNo = false);
	Bitmap *getBitmap(uint8 bitmapId);

	uint8 getNoDoors(bool ignoreNo = false) const;
	uint8 getNoObjects(bool ignoreNo = false) const;
	uint8 getNoStatics(bool ignoreNo = false) const;
	uint8 getNoBitmaps() const;

	/**
	 * Finds the door at the given position. By default, only active doors are considered.
	 *
	 * @param x X coordinate.
	 * @param y Y coordinate.
	 * @param activeOnly If true, consider only active doors; otherwise consider any.
	 * @param index Output parameter for the found door's ID.
	 * @return A door if found, nullptr otherwise.
	 */
	Door *findDoor(int16 x, int16 y, bool activeOnly = true, int *index = nullptr);
	/**
	 * Finds the static at the given position. By default, only active statics are considered.
	 *
	 * @param x X coordinate.
	 * @param y Y coordinate.
	 * @param activeOnly If true, consider only active statics; otherwise consider any.
	 * @param index Output parameter for the found static's ID.
	 * @return A static if found, nullptr otherwise.
	 */
	Static *findStatic(int16 x, int16 y, bool activeOnly = true, int *index = nullptr);
	Bitmap *findBitmap(int16 x, int16 y, int *index = nullptr);

	void addExhaustedConvItem(uint8 context, uint8 convItemIndex, uint8 convGroupIndex);
	bool isConvItemExhausted(uint8 context, uint8 convItemIndex, uint8 convGroupIndex) const;

	/** Refers to the script block that will be executed when you enter this scene (DS register). */
	uint8 _startup;
	/**
	 * These three variables control downscaling of the player character depending on his Y.
	 * TODO: Find out more.
	*/
	uint8 _unknown001;
	uint8 _unknown002;
	uint8 _unknown003;
	uint8 _delay; /**< Delay between object animation advancements (DL register). */

	uint8 _noDoors; /**< Number of doors in the scene (ND register). */
	Door _doors[5]; /**< Door definitions. */

	uint8 _noObjects; /**< Number of animated objects in the scene (NO register). */
	Object _objects[9]; /**< Object definitions. */

	uint8 _noStatics; /**< Number of statics in the scene (NS register). */
	Static _statics[15]; /**< Static definitions. */

	Bitmap _bitmaps[10]; /**< Bitmap definitions. There is no corresponding _noBitmaps field. */

	uint16 _obstacleY1; /**< Fixed Y coordinate for all static obstacles in the scene. Always 0 in data files. */

	/** First index (inclusive and 0-indexed) of the rotating portion of the palette (PF register). */
	uint8 _palRotFirst;
	/** Last index (inclusive and 0-indexed) of the rotating portion of the palette (PL register). */
	uint8 _palRotLast;
	/** Delay between each right rotation of the palette portion (PD register). */
	uint8 _palRotDelay;

	/**
	 * Points to the first free item in exhausted conversation item array.
	 * @note Indexed from 1.
	 */
	uint8 _exhaustedConvItemNext;
	ExhaustedConvItem _exhaustedConvItems[79];

	/**
	 * Load initial state from game data file.
	 *
	 * @param stream Stream for reading.
	 * @return True if success, false otherwise.
	 */
	bool loadInitialState(Common::ReadStream &stream);

	/**
	 * (De)serialization for save/load.
	 *
	 * @param sz Serializer.
	 */
	void saveLoadWithSerializer(Common::Serializer &sz) override;
};

struct ConversationInfo {
	struct Item {
		uint8 _question;
		uint8 _response;
		uint8 _nextGroupIndex;
	};

	typedef Common::Array<Item> ItemGroup;

	Common::Array<ItemGroup> _itemGroups;
	uint8 _context;
	uint8 _objectId;
	uint8 _color;
};

struct GameData : public Common::Serializable {
public:
	GameData();
	Scene *getScene(uint8 sceneId);
	Scene *getCurrentScene();
	Inventory &getInventory();

	/**
	 * Load initial state from game data file.
	 *
	 * @param stream Stream for reading.
	 * @return True if success, false otherwise.
	 */
	bool loadInitialState(Common::ReadStream &stream);

	/**
	 * (De)serialization for save/load.
	 *
	 * @param sz Serializer.
	 */
	void saveLoadWithSerializer(Common::Serializer &sz) override;

	uint8 _currentScene; // Persistent.
	uint8 _lastScene;
	bool _partB; // Persistent.
	Inventory _inventory; // Persistent.
	Common::String _currentAPK; // Persistent.
	ConversationInfo _conversationInfo;
	/** Current SayCommand color. */
	uint8 _color;
private:
	Scene _scenes[45]; // Persistent.
};

enum Colors {
	WHITE = 0xC6,
	DARKGRAY = 0xC2,
	LIGHTGRAY = 0xC4,
	GREEN = 0xC8,
	ORANGE = 0xCA,
	DARKBLUE = 0xD6,
	LIGHTBLUE = 0xDA,
	BROWN = 0xDC
};

}

#endif
