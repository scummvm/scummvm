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

#ifndef ALCACHOFA_ROOMS_H
#define ALCACHOFA_ROOMS_H

#include "alcachofa/objects.h"

namespace Alcachofa {

class World;

class Room {
public:
	static constexpr const char *kClassName = "CHabitacion";
	Room(World *world, Common::SeekableReadStream &stream);
	virtual ~Room();

	inline World &world() { return *_world; }
	inline uint8 mapIndex() const { return _mapIndex; }
	inline const Common::String &name() const { return _name; }
	inline const PathFindingShape *activeFloor() const {
		return _activeFloorI < 0 ? nullptr : &_floors[_activeFloorI];
	}
	inline int8 orderAt(Common::Point query) const {
		return _activeFloorI < 0 ? 49 : activeFloor()->orderAt(query);
	}
	inline float depthAt(Common::Point query) const {
		return _activeFloorI < 0 ? 1 : activeFloor()->depthAt(query);
	}
	inline uint8 characterAlphaTint() const { return _characterAlphaTint; }
	inline uint8 characterAlphaPremultiplier() const { return _characterAlphaPremultiplier; }
	inline bool fixedCameraOnEntering() const { return _fixedCameraOnEntering; }
	inline int musicID() const { return _musicId; }

	using ObjectIterator = Common::Array<ObjectBase *>::const_iterator;
	inline ObjectIterator beginObjects() const { return _objects.begin(); }
	inline ObjectIterator endObjects() const { return _objects.end(); }

	void update();
	void draw();
	virtual bool updateInput();
	virtual void loadResources();
	virtual void freeResources();
	virtual void syncGame(Common::Serializer &serializer);
	ObjectBase *getObjectByName(const char *name) const;
	void toggleActiveFloor();
	void debugPrint(bool withObjects) const;

protected:
	Room(World *world);
	void readRoomV1(Common::SeekableReadStream &stream);
	void readRoomV2and3(Common::SeekableReadStream &stream, bool hasUselessByte);
	void readObjects(Common::SeekableReadStream &stream);
	void initBackground();
	void updateScripts();
	void updateRoomBounds();
	void updateInteraction();
	void updateObjects();
	void drawObjects();
	void drawDebug();
	ShapeObject *getSelectedObject(ShapeObject *best = nullptr) const;

	World *_world;
	ObjectBase *_backgroundObject = nullptr;
	Common::String _name, _backgroundName;
	PathFindingShape _floors[2];
	bool _fixedCameraOnEntering = false;
	int8 _activeFloorI = -1;
	int _musicId = -1;
	int16 _backgroundScale = kBaseScale;
	uint8
		_mapIndex,
		_characterAlphaTint = 0,
		_characterAlphaPremultiplier = 100; ///< for some reason in percent instead of 0-255

	Common::Array<ObjectBase *> _objects;
};

// only used for V1 where Rooms by default have no floor
class RoomWithFloor final : public Room {
public:
	static constexpr const char *kClassName = "CHabitacionConSuelo";
	RoomWithFloor(World *world, Common::SeekableReadStream &stream);
};

class OptionsMenu final : public Room {
public:
	static constexpr const char *kClassName = "CHabitacionMenuOpciones";
	OptionsMenu(World *world, Common::SeekableReadStream &stream);

	bool updateInput() override;
	void loadResources() override;

	void clearLastSelectedObject(); // to reset arm animation
	inline SlideButton *&currentSlideButton() { return _currentSlideButton; }

private:
	ShapeObject *_lastSelectedObject = nullptr;
	ObjectBase *_idleArm = nullptr;
	SlideButton *_currentSlideButton = nullptr;
};

class ConnectMenu final : public Room {
public:
	static constexpr const char *kClassName = "CHabitacionConectar";
	ConnectMenu(World *world, Common::SeekableReadStream &stream);
};

class ListenMenu final : public Room {
public:
	static constexpr const char *kClassName = "CHabitacionEsperar";
	ListenMenu(World *world, Common::SeekableReadStream &stream);
};

class Inventory final : public Room {
public:
	static constexpr const char *kClassName = "CInventario";
	Inventory(World *world, Common::SeekableReadStream &stream);
	~Inventory() override;

	bool updateInput() override;

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
	MortadeloDisabledIcon, // only used for multiplayer
	FilemonDisabledIcon,
	InventoryDisabledIcon,

	Count
};

constexpr char kNoXORKey = 0;
constexpr char kEmbeddedXORKey = -128;

class World final {
public:
	~World();
	void load(); ///< unfortunately has to be split from ctor, so g_engine->world() is already set during load

	// reference-returning queries will error if the object does not exist

	using RoomIterator = Common::Array<const Room *>::const_iterator;
	inline RoomIterator beginRooms() const { return _rooms.begin(); }
	inline RoomIterator endRooms() const { return _rooms.end(); }
	inline Room &globalRoom() const { assert(_globalRoom != nullptr); return *_globalRoom; }
	inline Inventory &inventory() const { assert(_inventory != nullptr); return *_inventory; }
	inline MainCharacter &filemon() const { assert(_filemon != nullptr); return *_filemon; }
	inline MainCharacter &mortadelo() const { assert(_mortadelo != nullptr);  return *_mortadelo; }
	inline GameFileReference scriptFileRef() const { return _scriptFileRef; }
	inline const Common::String &initScriptName() const { return _initScriptName; }
	inline uint8 loadedMapCount() const { return _loadedMapCount; }

	inline bool somebodyUsing(ObjectBase *object) const {
		return filemon().currentlyUsing() == object ||
			mortadelo().currentlyUsing() == object;
	}

	MainCharacter &getMainCharacterByKind(MainCharacterKind kind) const;
	MainCharacter &getOtherMainCharacterByKind(MainCharacterKind kind) const;
	Room *getRoomByName(const char *name) const;
	ObjectBase *getObjectByName(const char *name) const;
	ObjectBase *getObjectByName(MainCharacterKind character, const char *name) const;
	ObjectBase *getObjectByNameFromAnyRoom(const char *name) const;
	const GameFileReference &getGlobalAnimation(GlobalAnimationKind kind) const;
	const char *getLocalizedName(const Common::String &name) const;
	const char *getDialogLine(int32 dialogId) const;

	void toggleObject(MainCharacterKind character, const char *objName, bool isEnabled);
	void syncGame(Common::Serializer &s);

	GameFileReference readFileRef(Common::SeekableReadStream &stream) const;
	Common::ScopedPtr<Common::SeekableReadStream> openFileRef(const GameFileReference &ref) const;

private:
	bool loadWorldFileV3(const char *path);
	bool loadWorldFileV2(const char *path);
	bool loadWorldFileV1(const char *path);
	void readRooms(Common::File &file);
	void loadLocalizedNames();
	void loadDialogLines();

	// the default Hash<const char*> works on the characters, but the default EqualTo compares pointers...
	struct StringEqualTo {
		bool operator()(const char *a, const char *b) const { return strcmp(a, b) == 0; }
	};

	Common::Array<Common::SharedPtr<Common::File>> _files; ///< only used in V1 to read embedded files
	Common::Array<Room *> _rooms;
	GameFileReference _globalAnimations[(int)GlobalAnimationKind::Count];
	Common::String _initScriptName;
	GameFileReference _scriptFileRef;
	Room *_globalRoom;
	Inventory *_inventory;
	MainCharacter *_filemon, *_mortadelo;
	uint8 _loadedMapCount = 0;
	Common::HashMap<const char *, const char *,
		Common::Hash<const char *>,
		StringEqualTo> _localizedNames;
	Common::Array<const char *> _dialogLines;
	Common::Array<char> _namesChunk, _dialogChunk; ///< holds the memory for localizedNames / dialogLines
	bool _isLoading = true;
};

}

#endif // ALCACHOFA_ROOMS_H
