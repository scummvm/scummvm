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

#include "alcachofa/alcachofa.h"
#include "alcachofa/rooms.h"
#include "alcachofa/script.h"
#include "alcachofa/global-ui.h"
#include "alcachofa/menu.h"

#include "common/file.h"

using namespace Common;

namespace Alcachofa {

Room::Room(World *world, SeekableReadStream &stream) : Room(world, stream, false) {}

static ObjectBase *readRoomObject(Room *room, const String &type, ReadStream &stream) {
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
		return nullptr; // handled in Room::Room
}

Room::Room(World *world, SeekableReadStream &stream, bool hasUselessByte)
	: _world(world) {
	_name = readVarString(stream);
	_musicId = (int)stream.readByte();
	_characterAlphaTint = stream.readByte();
	auto backgroundScale = stream.readSint16LE();
	_floors[0] = PathFindingShape(stream);
	_floors[1] = PathFindingShape(stream);
	_fixedCameraOnEntering = readBool(stream);
	PathFindingShape _(stream); // unused path finding area
	_characterAlphaPremultiplier = stream.readByte();
	if (hasUselessByte)
		stream.readByte();

	uint32 objectEnd = stream.readUint32LE();
	while (objectEnd > 0) {
		const auto type = readVarString(stream);
		auto object = readRoomObject(this, type, stream);
		if (object == nullptr) {
			g_engine->game().unknownRoomObject(type);
			stream.seek(objectEnd, SEEK_SET);
		} else if (stream.pos() < objectEnd) {
			g_engine->game().notEnoughObjectDataRead(_name.c_str(), stream.pos(), objectEnd);
			stream.seek(objectEnd, SEEK_SET);
		} else if (stream.pos() > objectEnd) // this is probably not recoverable
			error("Read past the object data (%u > %lld) in room %s", objectEnd, (long long int)stream.pos(), _name.c_str());

		if (object != nullptr)
			_objects.push_back(object);
		objectEnd = stream.readUint32LE();
	}
	if (g_engine->game().doesRoomHaveBackground(this))
		_objects.push_back(new Background(this, _name, backgroundScale));

	if (!_floors[0].empty())
		_activeFloorI = 0;
}

Room::~Room() {
	for (auto *object : _objects)
		delete object;
}

ObjectBase *Room::getObjectByName(const char *name) const {
	for (auto *object : _objects) {
		if (object->name().equalsIgnoreCase(name))
			return object;
	}
	return nullptr;
}

void Room::update() {
	if (g_engine->isDebugModeActive())
		return;
	updateScripts();

	if (g_engine->player().currentRoom() == this) {
		updateRoomBounds();
		g_engine->globalUI().updateClosingInventory();
		if (!updateInput())
			return;
	}
	if (!g_engine->menu().isOpen() &&
		g_engine->player().currentRoom() != &g_engine->world().inventory())
		world().globalRoom().updateObjects();
	if (g_engine->player().currentRoom() == this)
		updateObjects();
}

void Room::draw() {
	g_engine->camera().update();
	drawObjects();
	world().globalRoom().drawObjects();
	g_engine->globalUI().drawScreenStates();
	g_engine->drawQueue().draw();
	drawDebug();
	world().globalRoom().drawDebug();
}

void Room::updateScripts() {
	g_engine->game().updateScriptVariables();
	if (!g_engine->scheduler().hasProcessWithName("ACTUALIZAR_" + _name))
		g_engine->script().createProcess(MainCharacterKind::None, "ACTUALIZAR_" + _name, ScriptFlags::AllowMissing | ScriptFlags::IsBackground);
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
	if (g_engine->menu().isOpen() || !player.isGameLoaded())
		canInteract = true;
	if (canInteract) {
		player.resetCursor();
		if (!g_engine->globalUI().updateChangingCharacter() &&
			!g_engine->globalUI().updateOpeningInventory()) {
			updateInteraction();
			player.updateCursor();
		}
		player.drawCursor();
	}

	if (player.currentRoom() == this) {
		g_engine->globalUI().drawChangingButton();
		g_engine->menu().updateOpeningMenu();
	}

	return player.currentRoom() == this;
}

void Room::updateInteraction() {
	auto &player = g_engine->player();
	auto &input = g_engine->input();

	player.selectedObject() = world().globalRoom().getSelectedObject(getSelectedObject());
	if (player.selectedObject() == nullptr) {
		if (input.wasMouseLeftPressed() && _activeFloorI >= 0 &&
			player.activeCharacter()->room() == this &&
			player.pressedObject() == nullptr) {
			player.activeCharacter()->walkToMouse();
			g_engine->camera().setFollow(player.activeCharacter());
		}
	} else {
		player.selectedObject()->markSelected();
		if (input.wasAnyMousePressed())
			player.pressedObject() = player.selectedObject();
	}
}

void Room::updateRoomBounds() {
	auto background = getObjectByName("Background");
	auto graphic = background == nullptr ? nullptr : background->graphic();
	if (graphic != nullptr) {
		auto bgSize = graphic->animation().imageSize(0);
		/* This fixes a bug where if the background image is invalid the original engine
		 * would not update the background size. This would be around 1024,768 due to
		 * previous rooms in the bug instances I found.
		 */
		if (bgSize == Point(0, 0))
			bgSize = Point(1024, 768);
		g_engine->camera().setRoomBounds(bgSize, graphic->scale());
	}
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
	for (auto *object : _objects) {
		object->draw();
	}
}

void Room::drawDebug() {
	auto renderer = dynamic_cast<IDebugRenderer *>(&g_engine->renderer());
	if (renderer == nullptr || !g_engine->console().isAnyDebugDrawingOn())
		return;
	for (auto *object : _objects) {
		if (object->room() == g_engine->player().currentRoom())
			object->drawDebug();
	}
	if (_activeFloorI < 0)
		return;
	auto &floor = _floors[_activeFloorI];
	if (g_engine->console().showFloor())
		renderer->debugShape(floor, kDebugBlue);

	if (g_engine->console().showFloorEdges()) {
		auto &camera = g_engine->camera();
		for (uint polygonI = 0; polygonI < floor.polygonCount(); polygonI++) {
			auto polygon = floor.at(polygonI);
			for (uint pointI = 0; pointI < polygon._points.size(); pointI++) {
				int32 targetI = floor.edgeTarget(polygonI, pointI);
				if (targetI < 0)
					continue;
				Point a = camera.transform3Dto2D(polygon._points[pointI]);
				Point b = camera.transform3Dto2D(polygon._points[(pointI + 1) % polygon._points.size()]);
				Point source = (a + b) / 2;
				Point target = camera.transform3Dto2D(floor.at((uint)targetI).midPoint());
				renderer->debugPolyline(source, target, kDebugLightBlue);
			}
		}
	}
}

void Room::loadResources() {
	for (auto *object : _objects)
		object->loadResources();

	// this fixes some camera backups not working when closing the inventory
	if (g_engine->player().currentRoom() == this)
		updateRoomBounds();
}

void Room::freeResources() {
	for (auto *object : _objects)
		object->freeResources();
}

void Room::syncGame(Serializer &serializer) {
	serializer.syncAsSByte(_activeFloorI);
	for (auto *object : _objects)
		object->syncGame(serializer);
}

void Room::toggleActiveFloor() {
	_activeFloorI ^= 1;
}

ShapeObject *Room::getSelectedObject(ShapeObject *best) const {
	for (auto object : _objects) {
		auto shape = object->shape();
		auto shapeObject = dynamic_cast<ShapeObject *>(object);
		if (!object->isEnabled() ||
			shape == nullptr || shapeObject == nullptr ||
			object->room() != g_engine->player().currentRoom() || // e.g. a main character that is in another room
			!shape->contains(g_engine->input().mousePos3D()))
			continue;
		if (best == nullptr || shapeObject->order() < best->order())
			best = shapeObject;
	}
	return best;
}

OptionsMenu::OptionsMenu(World *world, SeekableReadStream &stream)
	: Room(world, stream, true) {}

bool OptionsMenu::updateInput() {
	if (!Room::updateInput())
		return false;

	auto currentSelectedObject = g_engine->player().selectedObject();
	if (currentSelectedObject == nullptr) {
		if (_lastSelectedObject == nullptr) {
			if (_idleArm != nullptr)
				_idleArm->toggle(true);
			return true;
		}

		_lastSelectedObject->markSelected();
	} else
		_lastSelectedObject = currentSelectedObject;
	if (_idleArm != nullptr)
		_idleArm->toggle(false);
	return true;
}

void OptionsMenu::loadResources() {
	Room::loadResources();
	_lastSelectedObject = nullptr;
	_currentSlideButton = nullptr;
	_idleArm = getObjectByName("Brazo");
}

void OptionsMenu::clearLastSelectedObject() {
	_lastSelectedObject = nullptr;
}

ConnectMenu::ConnectMenu(World *world, SeekableReadStream &stream)
	: Room(world, stream, true) {}

ListenMenu::ListenMenu(World *world, SeekableReadStream &stream)
	: Room(world, stream, true) {}

Inventory::Inventory(World *world, SeekableReadStream &stream)
	: Room(world, stream, true) {}

Inventory::~Inventory() {
	// No need to delete items, they are room objects and thus deleted in Room::~Room
}

bool Inventory::updateInput() {
	auto &player = g_engine->player();
	auto &input = g_engine->input();
	auto *hoveredItem = getHoveredItem();

	if (!player.activeCharacter()->isBusy())
		player.drawCursor(0);

	if (hoveredItem != nullptr && !player.activeCharacter()->isBusy()) {
		if ((input.wasMouseLeftPressed() && player.heldItem() == nullptr) ||
			(input.wasMouseLeftReleased() && player.heldItem() != nullptr) ||
			input.wasMouseRightReleased()) {
			hoveredItem->trigger();
			player.pressedObject() = nullptr;
		}

		g_engine->drawQueue().add<TextDrawRequest>(
			g_engine->globalUI().generalFont(),
			g_engine->world().getLocalizedName(hoveredItem->name()),
			input.mousePos2D() + Point(0, -50),
			-1, true, kWhite, -kForegroundOrderCount + 1);
	}

	const bool userWantsToCloseInventory =
		closeInventoryTriggerBounds().contains(input.mousePos2D()) ||
		input.wasMenuKeyPressed() ||
		input.wasInventoryKeyPressed();
	if (!player.activeCharacter()->isBusy() &&
		userWantsToCloseInventory)
		close();

	if (!player.activeCharacter()->isBusy() &&
		hoveredItem == nullptr &&
		input.wasMouseRightReleased()) {
		player.heldItem() = nullptr;
		return false;
	}

	return player.currentRoom() == this;
}

Item *Inventory::getHoveredItem() {
	auto mousePos = g_engine->input().mousePos2D();
	for (auto item : _items) {
		if (!item->isEnabled())
			continue;
		if (g_engine->player().heldItem() != nullptr &&
			g_engine->player().heldItem()->name().equalsIgnoreCase(item->name()))
			continue;

		auto graphic = item->graphic();
		assert(graphic != nullptr);
		auto bounds = graphic->animation().frameBounds(0);
		auto totalOffset = graphic->animation().totalFrameOffset(0);
		auto delta = mousePos - graphic->topLeft() - totalOffset;
		if (delta.x >= 0 && delta.y >= 0 && delta.x <= bounds.width() && delta.y <= bounds.height())
			return item;
	}
	return nullptr;
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

void Inventory::drawAsOverlay(int32 scrollY) {
	for (auto object : _objects) {
		auto graphic = object->graphic();
		if (graphic == nullptr)
			continue;

		int16 oldY = graphic->topLeft().y;
		int8 oldOrder = graphic->order();
		graphic->topLeft().y += scrollY;
		graphic->order() = -kForegroundOrderCount;
		if (object->name().equalsIgnoreCase("Background"))
			graphic->order()++;
		object->draw();
		graphic->topLeft().y = oldY;
		graphic->order() = oldOrder;
	}
}

void Inventory::open() {
	g_engine->camera().backup(1);
	g_engine->player().changeRoom(name(), true);
	updateItemsByActiveCharacter();
}

void Inventory::close() {
	g_engine->player().changeRoomToBeforeInventory();
	g_engine->camera().restore(1);
	g_engine->globalUI().startClosingInventory();
}

void Room::debugPrint(bool withObjects) const {
	auto &console = g_engine->console();
	console.debugPrintf("  %s\n", _name.c_str());
	if (!withObjects)
		return;

	for (auto *object : _objects) {
		console.debugPrintf("\t%20s %-32s %s\n",
			object->typeName(),
			object->name().c_str(),
			object->isEnabled() ? "" : "disabled");
	}
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
	loadLocalizedNames();
	loadDialogLines();

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
	case MainCharacterKind::Mortadelo:
		return *_mortadelo;
	case MainCharacterKind::Filemon:
		return *_filemon;
	default:
		error("Invalid character kind given to getMainCharacterByKind");
	}
}

MainCharacter &World::getOtherMainCharacterByKind(MainCharacterKind kind) const {
	switch (kind) {
	case MainCharacterKind::Mortadelo:
		return *_filemon;
	case MainCharacterKind::Filemon:
		return *_mortadelo;
	default:
		error("Invalid character kind given to getOtherMainCharacterByKind");
	}
}

Room *World::getRoomByName(const char *name) const {
	assert(name != nullptr);
	if (*name == '\0')
		return nullptr;
	for (auto *room : _rooms) {
		if (room->name().equalsIgnoreCase(name))
			return room;
	}
	return nullptr;
}

ObjectBase *World::getObjectByName(const char *name) const {
	ObjectBase *result = nullptr;
	if (g_engine->player().currentRoom() != nullptr)
		result = g_engine->player().currentRoom()->getObjectByName(name);
	if (result == nullptr)
		result = globalRoom().getObjectByName(name);
	if (result == nullptr)
		result = inventory().getObjectByName(name);
	return result;
}

ObjectBase *World::getObjectByName(MainCharacterKind character, const char *name) const {
	if (character == MainCharacterKind::None)
		return getObjectByName(name);
	const auto &player = g_engine->player();
	ObjectBase *result = nullptr;
	if (player.activeCharacterKind() == character && player.currentRoom() != player.activeCharacter()->room())
		result = player.currentRoom()->getObjectByName(name);
	if (result == nullptr)
		result = player.activeCharacter()->room()->getObjectByName(name);
	if (result == nullptr)
		result = globalRoom().getObjectByName(name);
	if (result == nullptr)
		result = inventory().getObjectByName(name);
	return result;
}

ObjectBase *World::getObjectByNameFromAnyRoom(const char *name) const {
	assert(name != nullptr);
	if (*name == '\0')
		return nullptr;
	for (auto *room : _rooms) {
		ObjectBase *result = room->getObjectByName(name);
		if (result != nullptr)
			return result;
	}
	return nullptr;
}

void World::toggleObject(MainCharacterKind character, const char *objName, bool isEnabled) {
	ObjectBase *object = getObjectByName(character, objName);
	if (object == nullptr)
		object = getObjectByNameFromAnyRoom(objName);
	if (object == nullptr) // I would have liked an error for this, but original inconsistencies...
		warning("Tried to toggle unknown object: %s", objName);
	else
		object->toggle(isEnabled);
}

const Common::String &World::getGlobalAnimationName(GlobalAnimationKind kind) const {
	int kindI = (int)kind;
	assert(kindI >= 0 && kindI < (int)GlobalAnimationKind::Count);
	return _globalAnimationNames[kindI];
}

const char *World::getLocalizedName(const String &name) const {
	const char *localizedName;
	return _localizedNames.tryGetVal(name.c_str(), localizedName)
		? localizedName
		: name.c_str();
}

const char *World::getDialogLine(int32 dialogId) const {
	if (dialogId < 0 || (uint)dialogId >= _dialogLines.size())
		error("Invalid dialog line index %d", dialogId);
	return _dialogLines[dialogId];
}

static Room *readRoom(World *world, SeekableReadStream &stream) {
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
	else {
		g_engine->game().unknownRoomType(type);
		return nullptr;
	}
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
		Room *room = readRoom(this, file);
		if (room != nullptr)
			_rooms.push_back(room);
		if (file.pos() < roomEnd) {
			g_engine->game().notEnoughRoomDataRead(path, file.pos(), roomEnd);
			file.seek(roomEnd, SEEK_SET);
		} else if (file.pos() > roomEnd) // this surely is not recoverable
			error("Read past the room data for world %s", path);
		roomEnd = file.readUint32LE();
	}

	return true;
}

/**
 * @brief Behold the incredible encryption of text files:
 *   - first 32 bytes are cipher
 *   - next byte is the XOR key
 *   - next 4 bytes are garbage
 *   - rest of the file is cipher
 */
static void loadEncryptedFile(const char *path, Array<char> &output) {
	constexpr uint kHeaderSize = 32;
	File file;
	if (!file.open(path))
		error("Could not open text file %s", path);
	output.resize(file.size() - 4 - 1 + 1); // garbage bytes, key and we add a zero terminator for safety
	if (file.read(output.data(), kHeaderSize) != kHeaderSize)
		error("Could not read text file header");
	char key = file.readSByte();
	uint remainingSize = output.size() - kHeaderSize - 1;
	if (!file.skip(4) || file.read(output.data() + kHeaderSize, remainingSize) != remainingSize)
		error("Could not read text file body");
	for (auto &ch : output)
		ch ^= key;
	output.back() = '\0'; // one for good measure and a zero-terminator
}

static char *trimLeading(char *start, char *end) {
	while (start < end && isSpace(*start))
		start++;
	return start;
}

static char *skipWord(char *start, char *end) {
	while (start < end && !isSpace(*start))
		start++;
	return start;
}

static char *trimTrailing(char *start, char *end) {
	while (start < end && isSpace(end[-1]))
		end--;
	return end;
}

void World::loadLocalizedNames() {
	loadEncryptedFile("Textos/OBJETOS.nkr", _namesChunk);
	char *lineStart = _namesChunk.begin(), *fileEnd = _namesChunk.end() - 1;
	while (lineStart < fileEnd) {
		char *lineEnd = find(lineStart, fileEnd, '\n');
		char *keyEnd = find(lineStart, lineEnd, '#');
		if (keyEnd == lineStart || keyEnd == lineEnd || keyEnd + 1 == lineEnd)
			error("Invalid localized name line separator");
		char *valueEnd = trimTrailing(keyEnd + 1, lineEnd);

		*keyEnd = 0;
		*valueEnd = 0;
		if (valueEnd == keyEnd + 1) {
			// happens in the english version of Movie Adventure
			warning("Empty localized name for %s", lineStart);
		}

		_localizedNames[lineStart] = keyEnd + 1;
		lineStart = lineEnd + 1;
	}
}

void World::loadDialogLines() {
	/* This "encrypted" file contains lines in any of the following formats:
	 * Name 123, "This is the dialog line"\r\n
	 * Name 123, "This is the dialog line\r\n
	 *     Name     123   This is the dialog line    \r\n
	 *
	 * - The ID does not have to be correct, it is ignored by the original engine.
	 * - We only need the dialog line and insert null-terminators where appropriate.
	 */
	loadEncryptedFile("Textos/DIALOGOS.nkr", _dialogChunk);
	char *lineStart = _dialogChunk.begin(), *fileEnd = _dialogChunk.end() - 1;
	while (lineStart < fileEnd) {
		char *lineEnd = find(lineStart, fileEnd, '\n');

		char *cursor = trimLeading(lineStart, lineEnd); // space before the name
		cursor = skipWord(cursor, lineEnd); // the name
		cursor = trimLeading(cursor, lineEnd); // space between dialog id
		cursor = skipWord(cursor, lineEnd); // the dialog id
		cursor = trimLeading(cursor, lineEnd); // space between id and line
		char *dialogLineEnd = trimTrailing(cursor, lineEnd);
		if (*cursor == '\"')
			cursor++;
		if (dialogLineEnd > cursor && dialogLineEnd[-1] == '\"')
			dialogLineEnd--;

		if (cursor >= dialogLineEnd) {
			if (cursor > dialogLineEnd)
				g_engine->game().invalidDialogLine(_dialogLines.size());
			cursor = lineStart; // store an empty string
			dialogLineEnd = lineStart;
		}

		*dialogLineEnd = 0;
		_dialogLines.push_back(cursor);
		lineStart = lineEnd + 1;
	}
}

void World::syncGame(Serializer &s) {
	for (Room *room : _rooms)
		room->syncGame(s);
}

}
