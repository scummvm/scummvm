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

#include "Objects.h"

namespace Alcachofa {

class World;

class Room {
public:
	static constexpr const char *kClassName = "CHabitacion";
	Room(World *world, Common::ReadStream &stream);
	virtual ~Room();

	inline World &world() { return *_world; }
	inline const Common::String &name() const { return _name; }

	void update();
	virtual void updateInput();
	virtual void loadResources();
	virtual void freeResources();
	virtual void serializeSave(Common::Serializer &serializer);
	ObjectBase *getObjectByName(const Common::String &name) const;

protected:
	Room(World *world, Common::ReadStream &stream, bool hasUselessByte);
	void updateRoomBounds();
	void updateObjects();
	void drawObjects();
	void drawDebug();

	World *_world;
	Common::String _name;
	PathFindingShape _floors[2];
	bool _cameraFollowsUponLeaving;
	int8
		_musicId,
		_activeFloorI = -1;
	uint8
		_characterAlpha,
		_characterAlphaPercent;

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

private:
	Common::Array<Item *> _items;
};

enum class GlobalAnimationKind {
	GeneralFont = 0,
	TextFont,
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

	inline Room &globalRoom() const { return *_globalRoom; }
	inline Inventory &inventory() const { return *_inventory; }
	inline MainCharacter &filemon() const { return *_filemon; }
	inline MainCharacter &mortadelo() const { return *_mortadelo; }
	inline const Common::String &initScriptName() const { return _initScriptName; }
	inline uint8 loadedMapCount() const { return _loadedMapCount; }

	inline Room *&currentRoom() { return _currentRoom; }
	inline Room *currentRoom() const { return _currentRoom; }

	MainCharacter &getMainCharacterByKind(MainCharacterKind kind) const;
	Room *getRoomByName(const Common::String &name) const;
	ObjectBase *getObjectByName(const Common::String &name) const;
	const Common::String &getGlobalAnimationName(GlobalAnimationKind kind) const;

private:
	bool loadWorldFile(const char *path);

	Common::Array<Room *> _rooms;
	Common::String _globalAnimationNames[(int)GlobalAnimationKind::Count];
	Common::String _initScriptName;
	Room *_globalRoom, *_currentRoom = nullptr;
	Inventory *_inventory;
	MainCharacter *_filemon, *_mortadelo;
	uint8 _loadedMapCount = 0;
};

}

#endif // ROOMS_H
