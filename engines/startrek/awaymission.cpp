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

#include "startrek/iwfile.h"
#include "startrek/startrek.h"

namespace StarTrek {

void StarTrekEngine::initAwayMission() {
	_awayMission = AwayMission(); // Initialize members to 0

	// memset(bitmapBuffer->pixels, 0, 0xfa00);
	// sub_15ab1("ground");
	// sub_23a60();
	_sound->loadMusicFile("ground");

	loadRoom(_missionToLoad, _roomIndexToLoad);

	// Load crew positions for beaming in
	initAwayCrewPositions(4);
}

void StarTrekEngine::runAwayMission() {
	while (true) {
		handleAwayMissionEvents();

		Common::Point mousePos = _gfx->getMousePos();
		_awayMission.mouseX = mousePos.x;
		_awayMission.mouseY = mousePos.y;

		assert(_commandQueue.size() <= 16);
		if (!_commandQueue.empty()) {
			// sub_200e7();
			// sub_20118();
			handleAwayMissionCommand();
		}
	}
}

void StarTrekEngine::cleanupAwayMission() {
	// TODO
}

void StarTrekEngine::loadRoom(const Common::String &missionName, int roomIndex) {
	_keyboardControlsMouse = true;

	_missionName = _missionToLoad;
	_roomIndex = roomIndex;

	_roomFrameCounter = 0;
	_awayMission.transitioningIntoRoom = 0;

	_gfx->fadeoutScreen();
	_sound->stopAllVocSounds();

	_screenName = _missionName + (char)(_roomIndex + '0');

	_gfx->setBackgroundImage(_gfx->loadBitmap(_screenName));
	_gfx->loadPri(_screenName + ".pri");
	_gfx->loadPalette("palette");
	_gfx->copyBackgroundScreen();

	_room = SharedPtr<Room>(new Room(this, _screenName));

	// Original sets up bytes 0-3 of rdf file as "remote function caller"

	// Load map file
	_awayMission.mapFileLoaded = 1;
	_mapFilename = _screenName;
	_mapFile = loadFile(_mapFilename + ".map");
	_iwFile = SharedPtr<IWFile>(new IWFile(this, _mapFilename + ".iw"));

	objectFunc1();
	initObjects();

	double num = _room->getVar0c() - _room->getVar0a();
	double den = _room->getVar06() - _room->getVar08() + 1;
	_playerObjectScale = (int32)(num * 256 / den);

	// TODO: RDF vars 1e/1f and 20/21; relates to BAN files?

	_commandQueue.clear();
}

void StarTrekEngine::initAwayCrewPositions(int warpEntryIndex) {
	_sound->stopAllVocSounds();

	memset(_awayMission.field25, 0xff, 4);

	switch (warpEntryIndex) {
	case 0: // 0-3: Read warp positions from RDF file
	case 1:
	case 2:
	case 3:
		for (int i = 0; i < (_awayMission.redshirtDead ? 3 : 4); i++) {
			Common::String anim = getCrewmanAnimFilename(i, "walk");

			int16 rdfOffset = RDF_ROOM_ENTRY_POSITIONS + warpEntryIndex * 32 + i * 8;

			int16 srcX = _room->readRdfWord(rdfOffset + 0);  // Position to spawn at
			int16 srcY = _room->readRdfWord(rdfOffset + 2);
			int16 destX = _room->readRdfWord(rdfOffset + 4); // Position to walk to
			int16 destY = _room->readRdfWord(rdfOffset + 6);

			objectWalkToPosition(i, anim, srcX, srcY, destX, destY);
		}

		_kirkObject->walkingIntoRoom = 1;
		_kirkObject->field66 = 0xff;
		_awayMission.transitioningIntoRoom = 1;
		_warpHotspotsActive = false;
		break;
	case 4: // Crew is beaming in.
		warpEntryIndex -= 4;
		for (int i = 0; i < (_awayMission.redshirtDead ? 3 : 4); i++) {
			Common::String animFilename = getCrewmanAnimFilename(i, "tele");
			Common::Point warpPos = _room->getBeamInPosition(i);
			loadObjectAnimWithRoomScaling(i, animFilename, warpPos.x, warpPos.y);
		}
		_kirkObject->walkingIntoRoom = 1;
		_kirkObject->field66 = 0xff;
		_awayMission.transitioningIntoRoom = 1;
		playSoundEffectIndex(0x09);
		_warpHotspotsActive = false;
		break;
	case 5:
		break;
	case 6:
		break;
	}
}

void StarTrekEngine::handleAwayMissionEvents() {
	TrekEvent event;

	if (popNextEvent(&event)) {
		switch (event.type) {
		case TREKEVENT_TICK:
			updateObjectAnimations();
			// sub_236bb();
			// sub_2325d();
			// doSomethingWithBanData1();
			_gfx->drawAllSprites();
			// doSomethingWithBanData2();
			// updateMusic(); // Is this needed?
			// sub_22de0();
			_frameIndex++;
			_roomFrameCounter++;
			addCommand(Command(COMMAND_TICK, _roomFrameCounter & 0xff, (_roomFrameCounter >> 8) & 0xff, 0));
			if (_roomFrameCounter >= 2)
				_gfx->incPaletteFadeLevel();
			break;
		case TREKEVENT_LBUTTONDOWN:
			if (_awayMission.transitioningIntoRoom != 0)
				break;
			switch (_awayMission.mapFileLoaded) {
			case 1:
				if (_awayMission.field1c == 0) {
					_kirkObject->sprite.drawMode = 1; // Hide these objects for function call below?
					_spockObject->sprite.drawMode = 1;
					_mccoyObject->sprite.drawMode = 1;
					_redshirtObject->sprite.drawMode = 1;

					// findObjectClickedOn();
					// ...

					_kirkObject->sprite.drawMode = 0;
					_spockObject->sprite.drawMode = 0;
					_mccoyObject->sprite.drawMode = 0;
					_redshirtObject->sprite.drawMode = 0;

					Common::String animFilename = getCrewmanAnimFilename(0, "walk");
					Common::Point mousePos = _gfx->getMousePos();
					objectWalkToPosition(0, animFilename, _kirkObject->pos.x, _kirkObject->pos.y, mousePos.x, mousePos.y);
				}
				break;
			case 2:
				break;
			case 3:
				break;
			case 4:
				break;
			case 5:
				break;
			}
			break;
		case TREKEVENT_MOUSEMOVE:
			break;
		case TREKEVENT_RBUTTONDOWN:
			break;
		case TREKEVENT_KEYDOWN:
			break;
		default:
			break;
		}
	}
}

void StarTrekEngine::unloadRoom() {
	_gfx->fadeoutScreen();
	// sub_2394b(); // TODO
	objectFunc1();
	_room.reset();
	_mapFile.reset();
}

/**
 * Similar to loadObjectAnim, but scale is determined by the y-position in the room. The
 * further up (away) the object is, the smaller it is.
 */
int StarTrekEngine::loadObjectAnimWithRoomScaling(int objectIndex, const Common::String &animName, int16 x, int16 y) {
	uint16 scale = getObjectScaleAtPosition(y);
	return loadObjectAnim(objectIndex, animName, x, y, scale);
}

uint16 StarTrekEngine::getObjectScaleAtPosition(int16 y) {
	int16 var06 = _room->getVar06();
	int16 var08 = _room->getVar08();
	int16 var0a = _room->getVar0a();

	if (var06 < y)
		y = var06;
	if (var08 > y)
		y = var08;

	return ((_playerObjectScale * (y - var08)) >> 8) + var0a;
}

SharedPtr<Room> StarTrekEngine::getRoom() {
	return _room;
}

void StarTrekEngine::addCommand(const Command &command) {
	if (command.type != COMMAND_TICK)
		debug("Command %d: %x, %x, %x", command.type, command.b1, command.b2, command.b3);
	_commandQueue.push(command);
}

void StarTrekEngine::handleAwayMissionCommand() {
	Command command = _commandQueue.pop();

	if ((command.type == COMMAND_FINISHED_BEAMING_IN || command.type == FINISHED_ENTERING_ROOM) && command.b1 == 0xff) {
		_awayMission.transitioningIntoRoom = 0;
		_warpHotspotsActive = true;
		return;
	}
	else if (command.type == FINISHED_ENTERING_ROOM && command.b1 >= 0xe0) { // TODO
		return;
	}

	switch (command.type) { // TODO: everything
	case COMMAND_TOUCHED_WARP:
		// if (!sub_203e1(command.type)) // Probably calls RDF code
		{
			byte warpIndex = command.b1;
			int16 roomIndex = _room->readRdfWord(RDF_WARP_ROOM_INDICES + warpIndex * 2);
			unloadRoom();
			_sound->loadMusicFile("ground");
			loadRoom(_missionName, roomIndex);
			initAwayCrewPositions(warpIndex ^ 1);
		}
		break;
	}
}

/**
 * Returns true if the given position is contained in a polygon?
 *
 * The data passed contains the following words in this order:
 *   * Index of polygon (unused here)
 *   * Number of vertices in polygon
 *   * For each vertex: x and y coordinates.
 */
bool StarTrekEngine::isPointInPolygon(int16 *data, int16 x, int16 y) {
	int16 numVertices = data[1];
	int16 *vertData = &data[2];

	for (int i = 0; i < numVertices; i++) {
		Common::Point p1(vertData[0], vertData[1]);
		Common::Point p2;
		if (i == numVertices - 1) // Loop to 1st vertex
			p2 = Common::Point(data[2], data[3]);
		else
			p2 = Common::Point(vertData[2], vertData[3]);

		if ((p2.x - p1.x) * (y - p1.y) - (p2.y - p1.y) * (x - p1.x) < 0)
			return false;

		vertData += 2;
	}

	return true;
}

void StarTrekEngine::checkTouchedLoadingZone(int16 x, int16 y) {
	int16 offset = _room->getFirstDoorPolygonOffset();

	while (offset != _room->getDoorPolygonEndOffset()) {
		if (isPointInPolygon((int16*)(_room->_rdfData + offset), x, y)) {
			uint16 var = _room->readRdfWord(offset);
			if (_activeDoorWarpHotspot != var) {
				_activeDoorWarpHotspot = var;
				addCommand(Command(COMMAND_7, var & 0xff, 0, 0));
			}
			return;
		}

		int16 numVertices = _room->readRdfWord(offset + 2);
		offset += numVertices * 4 + 4;
	}
	_activeDoorWarpHotspot = -1;

	if (_awayMission.field24 == 0 && _warpHotspotsActive) {
		offset = _room->getFirstWarpPolygonOffset();

		while (offset != _room->getWarpPolygonEndOffset()) {
			if (isPointInPolygon((int16*)(_room->_rdfData + offset), x, y)) {
				uint16 var = _room->readRdfWord(offset);
				if (_activeWarpHotspot != var) {
					_activeWarpHotspot = var;
					addCommand(Command(COMMAND_TOUCHED_WARP, var & 0xff, 0, 0));
				}
				return;
			}

			int16 numVertices = _room->readRdfWord(offset + 2);
			offset += numVertices * 4 + 4;
		}
	}
	_activeWarpHotspot = -1;
}

/**
 * Returns true if the given position in the room is solid (not walkable).
 * Reads from a ".map" file which has a bit for each position in the room, which is true
 * when that position is solid.
 */
bool StarTrekEngine::isPositionSolid(int16 x, int16 y) {
	assert(x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT);

	_mapFile->seek((y * SCREEN_WIDTH + x) / 8, SEEK_SET);
	return _mapFile->readByte() & (0x80 >> (x % 8));
}

}
