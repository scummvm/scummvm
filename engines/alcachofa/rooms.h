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

#ifndef ROOMS_H
#define ROOMS_H

#include "objects.h"

namespace Alcachofa {

class World;

class Room {
public:
	static constexpr const char *kClassName = "CHabitacion";
	Room(World *world, Common::ReadStream &stream);
	virtual ~Room();

	inline World &world() { return *_world; }
	inline const Common::String &name() const { return _name; }
	inline const PathFindingShape *activeFloor() const {
		return _activeFloorI < 0 ? nullptr : &_floors[_activeFloorI];
	}
	inline int8 orderAt(const Common::Point &query) const {
		return _activeFloorI < 0 ? 49 : activeFloor()->orderAt(query);
	}
	inline float depthAt(const Common::Point &query) const {
		return _activeFloorI < 0 ? 1 : activeFloor()->depthAt(query);
	}
	inline uint8 characterAlphaTint() const { return _characterAlphaTint; }
	inline uint8 characterAlphaPremultiplier() const { return _characterAlphaPremultiplier; }
	inline bool fixedCameraOnEntering() const { return _fixedCameraOnEntering; }

	using ObjectIterator = Common::Array<const ObjectBase *>::const_iterator;
	inline ObjectIterator beginObjects() const { return _objects.begin(); }
	inline ObjectIterator endObjects() const { return _objects.end(); }

	void update();
	virtual bool updateInput();
	virtual void loadResources();
	virtual void freeResources();
	virtual void serializeSave(Common::Serializer &serializer);
	ObjectBase *getObjectByName(const Common::String &name) const;
	void toggleActiveFloor();
	void startClosingInventory();
	void debugPrint(bool withObjects) const;

protected:
	Room(World *world, Common::ReadStream &stream, bool hasUselessByte);
	void updateScripts();
	void updateRoomBounds();
	bool updateOpeningInventory();
	void updateClosingInventory();
	void updateInteraction();
	void updateObjects();
	void drawObjects();
	void drawDebug();
	ShapeObject *getSelectedObject(ShapeObject *best = nullptr) const;

	World *_world;
	Common::String _name;
	PathFindingShape _floors[2];
	bool
		_fixedCameraOnEntering,
		_isOpeningInventory = false,
		_isClosingInventory = false;
	int8
		_musicId,
		_activeFloorI = -1;
	uint8
		_characterAlphaTint,
		_characterAlphaPremultiplier; ///< for some reason in percent instead of 0-255
	uint32 _timeForInventory = 0;

	Common::Array<ObjectBase *> _objects;
};

class OptionsMenu final : public Room {
public:
	static constexpr const char *kClassName = "CHabitacionMenuOpciones";
	OptionsMenu(World *world, Common::ReadStream &stream);
};

class ConnectMenu final : public Room {
public:
	static constexpr const char *kClassName = "CHabitacionConectar";
	ConnectMenu(World *world, Common::ReadStream &stream);
};

class ListenMenu final : public Room {
public:
	static constexpr const char *kClassName = "CHabitacionEsperar";
	ListenMenu(World *world, Common::ReadStream &stream);
};

class Inventory final : public Room {
public:
	static constexpr const char *kClassName = "CInventario";
	Inventory(World *world, Common::ReadStream &stream);
	virtual ~Inventory() override;

	virtual bool updateInput() override;

	void initItems();
	void updateItemsByActiveCharacter();
	void drawAsOverlay(int32 scrollY);
	void open();
	void close();

private:
	Item *getHoveredItem();

	Common::Array<Item *> _items;
};

enum class GlobalAnimationKind {
	GeneralFont = 0,
	DialogFont,
	Cursor,
	MortadeloIcon,
	FilemonIcon,
	InventoryIcon,
	MortadeloDisabledIcon,
	FilemonDisabledIcon,
	InventoryDisabledIcon,

	Count
};

class World final {
public:
	World();
	~World();

	// reference-returning queries will error if the object does not exist

	using RoomIterator = Common::Array<const Room *>::const_iterator;
	inline RoomIterator beginRooms() const { return _rooms.begin(); }
	inline RoomIterator endRooms() const { return _rooms.end(); }
	inline Room &globalRoom() const { assert(_globalRoom != nullptr); return *_globalRoom; }
	inline Inventory &inventory() const { assert(_inventory != nullptr); return *_inventory; }
	inline MainCharacter &filemon() const { assert(_filemon != nullptr); return *_filemon; }
	inline MainCharacter &mortadelo() const { assert(_mortadelo != nullptr);  return *_mortadelo; }
	inline Font &generalFont() const { assert(_generalFont != nullptr); return *_generalFont; }
	inline Font &dialogFont() const { assert(_dialogFont != nullptr); return *_dialogFont; }
	inline const Common::String &initScriptName() const { return _initScriptName; }
	inline uint8 loadedMapCount() const { return _loadedMapCount; }

	inline bool somebodyUsing(ObjectBase *object) const {
		return filemon().currentlyUsing() == object ||
			mortadelo().currentlyUsing() == object;
	}

	MainCharacter &getMainCharacterByKind(MainCharacterKind kind) const;
	MainCharacter &getOtherMainCharacterByKind(MainCharacterKind kind) const;
	Room *getRoomByName(const Common::String &name) const;
	ObjectBase *getObjectByName(const Common::String &name) const;
	ObjectBase *getObjectByName(MainCharacterKind character, const Common::String &name) const;
	ObjectBase *getObjectByNameFromAnyRoom(const Common::String &name) const;
	const Common::String &getGlobalAnimationName(GlobalAnimationKind kind) const;
	const char *getLocalizedName(const Common::String &name) const;
	const char *getDialogLine(int32 dialogId) const;

	void toggleObject(MainCharacterKind character, const Common::String &objName, bool isEnabled);

private:
	bool loadWorldFile(const char *path);
	void loadLocalizedNames();
	void loadDialogLines();

	// the default Hash<const char*> works on the characters, but the default EqualTo compares pointers...
	struct StringEqualTo {
		bool operator()(const char *a, const char *b) const { return strcmp(a, b) == 0; }
	};

	Common::Array<Room *> _rooms;
	Common::String _globalAnimationNames[(int)GlobalAnimationKind::Count];
	Common::String _initScriptName;
	Room *_globalRoom;
	Inventory *_inventory;
	MainCharacter *_filemon, *_mortadelo;
	Common::ScopedPtr<Font> _generalFont, _dialogFont;
	uint8 _loadedMapCount = 0;
	Common::HashMap<const char *, const char *,
		Common::Hash<const char*>,
		StringEqualTo> _localizedNames;
	Common::Array<const char *> _dialogLines;
	Common::Array<char> _namesChunk, _dialogChunk; ///< holds the memory for localizedNames / dialogLines
};

}

#endif // ROOMS_H
