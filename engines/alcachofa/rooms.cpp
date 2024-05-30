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

#include "alcachofa.h"
#include "rooms.h"
#include "script.h"
#include "stream-helper.h"

#include "common/file.h"

using namespace Common;

namespace Alcachofa {

Room::Room(World *world, ReadStream &stream) : Room(world, stream, false) {
}

static ObjectBase *readRoomObject(Room *room, ReadStream &stream) {
	const auto type = readVarString(stream);
	if (type == ObjectBase::kClassName)
		return new ObjectBase(room, stream);
	else if (type == PointObject::kClassName)
		return new PointObject(room, stream);
	else if (type == GraphicObject::kClassName)
		return new GraphicObject(room, stream);
	else if (type == SpecialEffectObject::kClassName)
		return new SpecialEffectObject(room, stream);
	else if (type == Item::kClassName)
		return new Item(room, stream);
	else if (type == PhysicalObject::kClassName)
		return new PhysicalObject(room, stream);
	else if (type == MainMenuButton::kClassName)
		return new MainMenuButton(room, stream);
	else if (type == InternetMenuButton::kClassName)
		return new InternetMenuButton(room, stream);
	else if (type == OptionsMenuButton::kClassName)
		return new OptionsMenuButton(room, stream);
	else if (type == EditBox::kClassName)
		return new EditBox(room, stream);
	else if (type == PushButton::kClassName)
		return new PushButton(room, stream);
	else if (type == CheckBox::kClassName)
		return new CheckBox(room, stream);
	else if (type == CheckBoxAutoAdjustNoise::kClassName)
		return new CheckBoxAutoAdjustNoise(room, stream);
	else if (type == SlideButton::kClassName)
		return new SlideButton(room, stream);
	else if (type == IRCWindow::kClassName)
		return new IRCWindow(room, stream);
	else if (type == MessageBox::kClassName)
		return new MessageBox(room, stream);
	else if (type == VoiceMeter::kClassName)
		return new VoiceMeter(room, stream);
	else if (type == InteractableObject::kClassName)
		return new InteractableObject(room, stream);
	else if (type == Door::kClassName)
		return new Door(room, stream);
	else if (type == Character::kClassName)
		return new Character(room, stream);
	else if (type == WalkingCharacter::kClassName)
		return new WalkingCharacter(room, stream);
	else if (type == MainCharacter::kClassName)
		return new MainCharacter(room, stream);
	else if (type == FloorColor::kClassName)
		return new FloorColor(room, stream);
	else
		error("Unknown type for room objects: %s", type.c_str());
}

Room::Room(World *world, ReadStream &stream, bool hasUselessByte)
	: _world(world) {
	_name = readVarString(stream);
	_musicId = stream.readSByte();
	_characterAlphaTint = stream.readByte();
	auto backgroundScale = stream.readSint16LE();
	_floors[0] = PathFindingShape(stream);
	_floors[1] = PathFindingShape(stream);
	_cameraFollowsUponLeaving = readBool(stream);
	PathFindingShape _(stream); // unused path finding area
	_characterAlphaPremultiplier = stream.readByte();
	if (hasUselessByte)
		stream.readByte();

	uint32 objectSize = stream.readUint32LE(); // TODO: Maybe switch to seekablereadstream and assert objectSize?
	while (objectSize > 0)
	{
		_objects.push_back(readRoomObject(this, stream));
		objectSize = stream.readUint32LE();
	}
	if (!_name.equalsIgnoreCase("Global"))
		_objects.push_back(new Background(this, _name, backgroundScale));

	if (!_floors[0].empty())
		_activeFloorI = 0;
}

Room::~Room() {
	for (auto *object : _objects)
		delete object;
}

ObjectBase *Room::getObjectByName(const Common::String &name) const {
	for (auto *object : _objects) {
		if (object->name().equalsIgnoreCase(name))
			return object;
	}
	return nullptr;
}

void Room::update() {
	updateScripts();

	if (g_engine->player().currentRoom() == this) {
		updateRoomBounds();
		if (!updateInput())
			return;
	}
	// TODO: Add condition for global room update
	world().globalRoom().updateObjects();
	if (g_engine->player().currentRoom() == this)
		updateObjects();
	if (g_engine->player().currentRoom() == this) {
		g_engine->camera().update();
		drawObjects();
		world().globalRoom().drawObjects();
		// TODO: Draw black borders
		g_engine->drawQueue().draw();
		drawDebug();
		world().globalRoom().drawDebug();
	}
}

void Room::updateScripts() {
	g_engine->script().updateCommonVariables();
	if (!g_engine->scheduler().hasProcessWithName("ACTUALIZAR_" + _name))
		g_engine->script().createProcess(MainCharacterKind::None, "ACTUALIZAR_" + _name, true);
	g_engine->scheduler().run();
}

bool Room::updateInput() {
	auto &player = g_engine->player();
	auto &input = g_engine->input();
	if (player.heldItem() != nullptr && !player.activeCharacter()->isBusy() && input.wasMouseRightPressed()) {
		player.heldItem() = nullptr;
		return false;
	}

	bool canInteract = !player.activeCharacter()->isBusy();
	// A complicated network condition can prevent interaction at this point
	if (player.isOptionsMenuOpen() || !player.isGameLoaded())
		canInteract = true;
	if (canInteract)
		updateInteraction();

	// TODO: Add main menu and opening inventory handling
	return player.currentRoom() == this;
}

void Room::updateInteraction() {
	auto &player = g_engine->player();
	auto &input = g_engine->input();
	// TODO: Add interaction with change character button / opening inventory

	if (player.activeCharacter()->room() != this) {
		player.activeCharacter()->room() = this;
	}

	player.selectedObject() = world().globalRoom().getSelectedObject(getSelectedObject());
	if (player.selectedObject() == nullptr) {
		if (input.wasMouseLeftPressed() && _activeFloorI >= 0 &&
			player.activeCharacter()->room() == this &&
			player.pressedObject() == nullptr) {
			player.activeCharacter()->walkToMouse();
			// TODO: Activate camera following character
		}
	}
	else {
		player.selectedObject()->markSelected();
		if (input.wasAnyMousePressed())
			player.pressedObject() = player.selectedObject();
	}
	player.updateCursor();
}

void Room::updateRoomBounds() {
	auto background = getObjectByName("Background");
	auto graphic = background == nullptr ? nullptr : background->graphic();
	if (graphic != nullptr)
		g_engine->camera().setRoomBounds(graphic->animation().imageSize(0), graphic->scale());
}

void Room::updateObjects() {
	const auto *previousRoom = g_engine->player().currentRoom();
	for (auto *object : _objects) {
		object->update();
		if (g_engine->player().currentRoom() != previousRoom)
			return;
	}
}

void Room::drawObjects() {
	for (auto *object : _objects)
		object->draw();
}

void Room::drawDebug() {
	auto renderer = dynamic_cast<IDebugRenderer *>(&g_engine->renderer());
	if (renderer == nullptr || !g_engine->console().isAnyDebugDrawingOn())
		return;
	for (auto *object : _objects)
		object->drawDebug();
	if (_activeFloorI < 0)
		return;
	if (_activeFloorI >= 0 && g_engine->console().showFloor())
		renderer->debugShape(_floors[_activeFloorI], kDebugBlue);
}

void Room::loadResources() {
	for (auto *object : _objects)
		object->loadResources();
}

void Room::freeResources() {
	for (auto *object : _objects)
		object->freeResources();
}

void Room::serializeSave(Serializer &serializer) {
	serializer.syncAsSByte(_musicId);
	serializer.syncAsSByte(_activeFloorI);
	for (auto *object : _objects)
		object->serializeSave(serializer);
}

void Room::toggleActiveFloor() {
	_activeFloorI ^= 1;
}

ShapeObject *Room::getSelectedObject(ShapeObject *best) const {
	for (auto object : _objects) {
		auto shape = object->shape();
		auto shapeObject = dynamic_cast<ShapeObject *>(object);
		if (!object->isEnabled() || shape == nullptr || shapeObject == nullptr ||
			object->room() != this || // e.g. a main character that is in another room
			!shape->contains(g_engine->input().mousePos3D()))
			continue;
		if (best == nullptr || shapeObject->order() < best->order())
			best = shapeObject;
	}
	return best;
}

OptionsMenu::OptionsMenu(World *world, ReadStream &stream)
	: Room(world, stream, true) {
}

ConnectMenu::ConnectMenu(World *world, ReadStream &stream)
	: Room(world, stream, true) {
}

ListenMenu::ListenMenu(World *world, ReadStream &stream)
	: Room(world, stream, true) {
}

Inventory::Inventory(World *world, ReadStream &stream)
	: Room(world, stream, true) {
}

Inventory::~Inventory() {
	for (auto *item : _items)
		delete item;
}

void Inventory::initItems() {
	auto &mortadelo = world().mortadelo();
	auto &filemon = world().filemon();
	for (auto object : _objects) {
		auto item = dynamic_cast<Item *>(object);
		if (item == nullptr)
			continue;
		_items.push_back(item);
		mortadelo._items.push_back(new Item(*item));
		filemon._items.push_back(new Item(*item));
	}
}

void Inventory::updateItemsByActiveCharacter() {
	auto *character = g_engine->player().activeCharacter();
	assert(character != nullptr);
	for (auto *item : _items)
		item->toggle(character->hasItem(item->name()));
}

static constexpr const char *kMapFiles[] = {
	"MAPAS/MAPA5.EMC",
	"MAPAS/MAPA4.EMC",
	"MAPAS/MAPA3.EMC",
	"MAPAS/MAPA2.EMC",
	"MAPAS/MAPA1.EMC",
	"MAPAS/GLOBAL.EMC",
	nullptr
};

World::World() {
	for (auto *itMapFile = kMapFiles; *itMapFile != nullptr; itMapFile++) {
		if (loadWorldFile(*itMapFile))
			_loadedMapCount++;
	}

	_globalRoom = getRoomByName("GLOBAL");
	if (_globalRoom == nullptr)
		error("Could not find GLOBAL room");
	_inventory = dynamic_cast<Inventory *>(getRoomByName("INVENTARIO"));
	if (_inventory == nullptr)
		error("Could not find INVENTARIO");
	_filemon = dynamic_cast<MainCharacter *>(_globalRoom->getObjectByName("FILEMON"));
	if (_filemon == nullptr)
		error("Could not find FILEMON");
	_mortadelo = dynamic_cast<MainCharacter *>(_globalRoom->getObjectByName("MORTADELO"));
	if (_mortadelo == nullptr)
		error("Could not find MORTADELO");

	_inventory->initItems();
}

World::~World() {
	for (auto *room : _rooms)
		delete room;
}

MainCharacter &World::getMainCharacterByKind(MainCharacterKind kind) const {
	switch (kind) {
	case MainCharacterKind::Mortadelo: return *_mortadelo;
	case MainCharacterKind::Filemon: return *_filemon;
	default:
		error("Invalid character kind given to getMainCharacterByKind");
	}
}

Room *World::getRoomByName(const Common::String &name) const {
	for (auto *room : _rooms) {
		if (room->name().equalsIgnoreCase(name))
			return room;
	}
	return nullptr;
}

ObjectBase *World::getObjectByName(const Common::String &name) const {
	ObjectBase *result = nullptr;
	if (result == nullptr && g_engine->player().currentRoom() != nullptr)
		result = g_engine->player().currentRoom()->getObjectByName(name);
	if (result == nullptr)
		result = globalRoom().getObjectByName(name);
	if (result == nullptr)
		result = inventory().getObjectByName(name);
	return result;
}

ObjectBase *World::getObjectByName(MainCharacterKind character, const Common::String &name) const {
	if (character == MainCharacterKind::None)
		return getObjectByName(name);
	const auto &player = g_engine->player();
	ObjectBase *result = nullptr;
	if (player.activeCharacterKind() == character && player.currentRoom() == player.activeCharacter()->room())
		result = player.currentRoom()->getObjectByName(name);
	if (result == nullptr)
		result = player.activeCharacter()->room()->getObjectByName(name);
	if (result == nullptr)
		result = globalRoom().getObjectByName(name);
	if (result == nullptr)
		result = inventory().getObjectByName(name);
	return result;
}

ObjectBase *World::getObjectByNameFromAnyRoom(const Common::String &name) const {
	for (auto *room : _rooms) {
		ObjectBase *result = room->getObjectByName(name);
		if (result != nullptr)
			return result;
	}
	return nullptr;
}

void World::toggleObject(MainCharacterKind character, const Common::String &objName, bool isEnabled) {
	ObjectBase *object = getObjectByName(character, objName);
	if (object == nullptr)
		object = getObjectByNameFromAnyRoom(objName);
	if (object == nullptr)
		error("Tried to toggle unknown object: %s", objName.c_str());
	else
		object->toggle(isEnabled);
}

const Common::String &World::getGlobalAnimationName(GlobalAnimationKind kind) const {
	int kindI = (int)kind;
	assert(kindI >= 0 && kindI < (int)GlobalAnimationKind::Count);
	return _globalAnimationNames[kindI];
}

static Room *readRoom(World *world, ReadStream &stream) {
	const auto type = readVarString(stream);
	if (type == Room::kClassName)
		return new Room(world, stream);
	else if (type == OptionsMenu::kClassName)
		return new OptionsMenu(world, stream);
	else if (type == ConnectMenu::kClassName)
		return new ConnectMenu(world, stream);
	else if (type == ListenMenu::kClassName)
		return new ListenMenu(world, stream);
	else if (type == Inventory::kClassName)
		return new Inventory(world, stream);
	else
		error("Unknown type for room %s", type.c_str());
}

bool World::loadWorldFile(const char *path) {
	Common::File file;
	if (!file.open(path)) {
		// this is not necessarily an error, apparently the demos just have less
		// chapter files. Being a demo is then also stored in some script vars
		warning("Could not open world file %s\n", path);
		return false;
	}

	// the first chunk seems to be debug symbols and/or info about the file structure
	// it is ignored in the published game.
	auto startOffset = file.readUint32LE();
	file.seek(startOffset, SEEK_SET);
	skipVarString(file); // some more unused strings related to development files?
	skipVarString(file);
	skipVarString(file);
	skipVarString(file);
	skipVarString(file);
	skipVarString(file);

	_initScriptName = readVarString(file);
	skipVarString(file); // would be _updateScriptName, but it is never called
	for (int i = 0; i < (int)GlobalAnimationKind::Count; i++)
		_globalAnimationNames[i] = readVarString(file);

	uint32 roomEnd = file.readUint32LE();
	while (roomEnd > 0) {
		_rooms.push_back(readRoom(this, file));
		assert(file.pos() == roomEnd);
		roomEnd = file.readUint32LE();
	}

	return true;
}

}
