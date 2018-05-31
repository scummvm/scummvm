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
#include "startrek/room.h"
#include "startrek/startrek.h"

namespace StarTrek {

void StarTrekEngine::initAwayMission() {
	_awayMission = AwayMission(); // Initialize members to 0

	// memset(bitmapBuffer->pixels, 0, 0xfa00);

	_txtFilename = "ground";
	_loadedText = "";

	// sub_23a60(); // TODO
	_sound->loadMusicFile("ground");

	loadRoom(_missionToLoad, _roomIndexToLoad);
	_roomIndexToLoad = -1;

	// Load crew positions for beaming in
	initAwayCrewPositions(4);
}

void StarTrekEngine::runAwayMission() {
	while (true) {
		// Original game manipulates the stack when the room changes to return execution
		// to this point. Instead of doing that, just check if a variable is set.
		if (_roomIndexToLoad != -1 && _spawnIndexToLoad != -1) {
			loadRoomIndex(_roomIndexToLoad, _spawnIndexToLoad);
			_roomIndexToLoad = -1;
			_spawnIndexToLoad = -1;
		}

		handleAwayMissionEvents();

		Common::Point mousePos = _gfx->getMousePos();
		_awayMission.mouseX = mousePos.x;
		_awayMission.mouseY = mousePos.y;

		assert(_actionQueue.size() <= 16);
		if (!_actionQueue.empty()) {
			// sub_200e7();
			// sub_20118();
			handleAwayMissionAction();
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
	_awayMission.disableInput = false;

	_gfx->fadeoutScreen();
	_sound->stopAllVocSounds();

	_screenName = _missionName + (char)(_roomIndex + '0');

	_gfx->setBackgroundImage(_gfx->loadBitmap(_screenName));
	_gfx->loadPri(_screenName);
	_gfx->loadPalette("palette");
	_gfx->copyBackgroundScreen();

	_room = SharedPtr<Room>(new Room(this, _screenName));

	// Original sets up bytes 0-3 of rdf file as "remote function caller"

	// Load map file
	_awayMission.activeAction = ACTION_WALK;
	_mapFilename = _screenName;
	_mapFile = loadFile(_mapFilename + ".map");
	_iwFile = SharedPtr<IWFile>(new IWFile(this, _mapFilename + ".iw"));

	actorFunc1();
	initActors();

	double num = _room->getVar0c() - _room->getVar0a();
	double den = _room->getVar06() - _room->getVar08() + 1;
	_playerActorScale = (int32)(num * 256 / den);

	// TODO: RDF vars 1e/1f and 20/21; relates to BAN files?

	_actionQueue.clear();
}

void StarTrekEngine::initAwayCrewPositions(int warpEntryIndex) {
	_sound->stopAllVocSounds();

	memset(_awayMission.crewDirectionsAfterWalk, 0xff, 4);

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

			actorWalkToPosition(i, anim, srcX, srcY, destX, destY);
		}

		_kirkActor->triggerActionWhenAnimFinished = true;
		_kirkActor->finishedAnimActionParam = 0xff;
		_awayMission.disableInput = true;
		_warpHotspotsActive = false;
		break;
	case 4: // Crew is beaming in.
		warpEntryIndex -= 4;
		for (int i = 0; i < (_awayMission.redshirtDead ? 3 : 4); i++) {
			Common::String animFilename = getCrewmanAnimFilename(i, "tele");
			Common::Point warpPos = _room->getBeamInPosition(i);
			loadActorAnimWithRoomScaling(i, animFilename, warpPos.x, warpPos.y);
		}
		_kirkActor->triggerActionWhenAnimFinished = true;
		_kirkActor->finishedAnimActionParam = 0xff;
		_awayMission.disableInput = true;
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
			updateActorAnimations();
			updateCrewmanGetupTimers();
			updateMouseBitmap();
			// doSomethingWithBanData1();
			_gfx->drawAllSprites();
			// doSomethingWithBanData2();
			_sound->checkLoopMusic();
			updateAwayMissionTimers();
			_frameIndex++;
			_roomFrameCounter++;
			addAction(Action(ACTION_TICK, _roomFrameCounter & 0xff, (_roomFrameCounter >> 8) & 0xff, 0));
			if (_roomFrameCounter >= 2)
				_gfx->incPaletteFadeLevel();
			break;

		case TREKEVENT_LBUTTONDOWN:
			if (_awayMission.disableInput)
				break;

			switch (_awayMission.activeAction) {
			case ACTION_WALK: {
				if (_awayMission.disableWalking)
					break;
				_kirkActor->sprite.drawMode = 1; // Hide these objects for function call below?
				_spockActor->sprite.drawMode = 1;
				_mccoyActor->sprite.drawMode = 1;
				_redshirtActor->sprite.drawMode = 1;

				int clickedObject = findObjectAt(_gfx->getMousePos());

				_kirkActor->sprite.drawMode = 0;
				_spockActor->sprite.drawMode = 0;
				_mccoyActor->sprite.drawMode = 0;
				_redshirtActor->sprite.drawMode = 0;

				if (walkActiveObjectToHotspot())
					break;

				if (clickedObject > OBJECT_KIRK && clickedObject < ITEMS_START)
					addAction(ACTION_WALK, clickedObject, 0, 0);
				else {
					Common::String animFilename = getCrewmanAnimFilename(OBJECT_KIRK, "walk");
					Common::Point mousePos = _gfx->getMousePos();
					actorWalkToPosition(OBJECT_KIRK, animFilename, _kirkActor->pos.x, _kirkActor->pos.y, mousePos.x, mousePos.y);
				}
				break;
			}

			case ACTION_USE: {
				if (_awayMission.activeObject == OBJECT_REDSHIRT && (_awayMission.redshirtDead || (_awayMission.crewDownBitset & (1 << OBJECT_REDSHIRT)))) {
					hideInventoryIcons();
					_awayMission.activeAction = ACTION_WALK;
					break;
				}

				int clickedObject = findObjectAt(_gfx->getMousePos());
				hideInventoryIcons();

				if (clickedObject == OBJECT_INVENTORY_ICON) {
					clickedObject = showInventoryMenu(50, 50, false);

					// -1 means "clicked on something unknown"; -2 means "clicked on
					// nothing". In the case of the inventory, either one clicks on an
					// inventory item, or no action is performed.
					if (clickedObject == -1)
						clickedObject = -2;
				}

				_awayMission.passiveObject = clickedObject;

				bool activeIsCrewman = _awayMission.activeObject <= OBJECT_REDSHIRT;
				bool activeIsItem = _awayMission.activeObject >= ITEMS_START && _awayMission.activeObject < ITEMS_END;
				bool passiveIsCrewman = _awayMission.passiveObject <= OBJECT_REDSHIRT;
				bool passiveIsItem = _awayMission.passiveObject >= ITEMS_START && _awayMission.passiveObject <= ITEMS_END; // FIXME: "<= ITEMS_END" doesn't make sense?

				if (clickedObject == -2)
					goto checkAddAction;
				if (_room->actionHasCode(Action(ACTION_USE, _awayMission.activeObject, _awayMission.passiveObject, 0)))
					goto checkAddAction;
				if (_awayMission.activeObject == OBJECT_MCCOY) {
					if (_room->actionHasCode(Action(ACTION_USE, OBJECT_IMEDKIT, _awayMission.passiveObject, 0)))
						goto checkAddAction;
					if (_room->actionHasCode(Action(ACTION_USE, OBJECT_IMEDKIT, _awayMission.passiveObject, 0)))
						goto checkAddAction;
				}
				else if (_awayMission.activeObject == OBJECT_SPOCK) {
					if (_room->actionHasCode(Action(ACTION_USE, OBJECT_ISTRICOR, _awayMission.passiveObject, 0)))
						goto checkAddAction;
				}

				if ((activeIsCrewman && passiveIsCrewman)
						|| (activeIsCrewman && passiveIsItem)
						|| (activeIsItem && passiveIsItem)) {
					if (_awayMission.passiveObject == OBJECT_ICOMM) {
						if (walkActiveObjectToHotspot())
							break;
						addAction(Action(ACTION_USE, OBJECT_ICOMM, 0, 0));
						_sound->playVoc("commun30");
						if (_awayMission.activeObject <= OBJECT_REDSHIRT) {
							goto checkShowInventory;
						}
						else {
							_awayMission.activeAction = ACTION_WALK;
							break;
						}
					}

					_awayMission.activeObject = _awayMission.passiveObject;
					goto checkShowInventory;
				}

checkAddAction:
				if (!walkActiveObjectToHotspot())
				{
					if (clickedObject != -2)
						addAction(Action(_awayMission.activeAction, _awayMission.activeObject, _awayMission.passiveObject, 0));

checkShowInventory:
					if (!(_awayMission.crewDownBitset & (1 << OBJECT_KIRK)))
						showInventoryIcons(true);
				}
				break;
			}

			case ACTION_GET:
			case ACTION_LOOK:
			case ACTION_TALK: {
				int clickedObject = findObjectAt(_gfx->getMousePos());
				// if (!sub_23611(clickedObject, _awayMission.activeAction)) // TODO
				{
					hideInventoryIcons();
					if (clickedObject == OBJECT_INVENTORY_ICON) {
						clickedObject = showInventoryMenu(50, 50, false);
						if (clickedObject == -1)
							clickedObject = -2;
					}

					_awayMission.activeObject = clickedObject;

					if (walkActiveObjectToHotspot())
						break;

					if (clickedObject != -2)
						addAction(Action(_awayMission.activeAction, _awayMission.activeObject, 0, 0));

					if (_awayMission.activeAction == ACTION_LOOK && !(_awayMission.crewDownBitset & (1 << OBJECT_KIRK)))
						showInventoryIcons(false);
				}
				break;
			}

			}
			break; // End of TREKEVENT_LBUTTONDOWN

		case TREKEVENT_MOUSEMOVE:
			break;

		case TREKEVENT_RBUTTONDOWN: // TODO: also triggered by key press?
			if (_awayMission.disableInput)
				break;
			hideInventoryIcons();
			playSoundEffectIndex(0x07);
			_awayMission.activeAction = showActionMenu();
			if (_awayMission.activeAction == ACTION_USE) {
				int16 clickedObject = selectObjectForUseAction();
				if (clickedObject == -1)
					break;
				else
					_awayMission.activeObject = clickedObject;
			}
			if (_awayMission.activeAction == ACTION_USE
					&& _awayMission.activeObject == OBJECT_ICOMM && (_awayMission.crewDownBitset & (1 << OBJECT_KIRK)) == 0) {
				if (!walkActiveObjectToHotspot()) {
					addAction(Action(_awayMission.activeAction, _awayMission.activeObject, 0, 0));
					_sound->playVoc("communic");
					_awayMission.activeAction = ACTION_WALK;
				}
			}
			else if (_awayMission.activeAction == ACTION_LOOK)
				showInventoryIcons(false);
			else if (_awayMission.activeAction == ACTION_USE && (_awayMission.crewDownBitset & 1) == 0)
				showInventoryIcons(true);
			break;

		case TREKEVENT_KEYDOWN: // TODO
			break;

		default:
			break;
		}
	}
}

void StarTrekEngine::unloadRoom() {
	_gfx->fadeoutScreen();
	// sub_2394b(); // TODO
	actorFunc1();
	_room.reset();
	_mapFile.reset();
}

/**
 * Similar to loadActorAnim, but scale is determined by the y-position in the room. The
 * further up (away) the object is, the smaller it is.
 */
int StarTrekEngine::loadActorAnimWithRoomScaling(int actorIndex, const Common::String &animName, int16 x, int16 y) {
	uint16 scale = getActorScaleAtPosition(y);
	return loadActorAnim(actorIndex, animName, x, y, scale);
}

uint16 StarTrekEngine::getActorScaleAtPosition(int16 y) {
	int16 var06 = _room->getVar06();
	int16 var08 = _room->getVar08();
	int16 var0a = _room->getVar0a();

	if (var06 < y)
		y = var06;
	if (var08 > y)
		y = var08;

	return ((_playerActorScale * (y - var08)) >> 8) + var0a;
}

SharedPtr<Room> StarTrekEngine::getRoom() {
	return _room;
}

void StarTrekEngine::addAction(const Action &action) {
	if (action.type != ACTION_TICK)
		debug("Action %d: %x, %x, %x", action.type, action.b1, action.b2, action.b3);
	_actionQueue.push(action);
}

void StarTrekEngine::handleAwayMissionAction() {
	Action action = _actionQueue.pop();

	if ((action.type == ACTION_FINISHED_ANIMATION || action.type == ACTION_FINISHED_WALKING) && action.b1 == 0xff) {
		_awayMission.disableInput = false;
		_warpHotspotsActive = true;
		return;
	}
	else if (action.type == ACTION_FINISHED_WALKING && action.b1 >= 0xe0) {
		// Finished walking to a position; perform the action that was input back when
		// they started walking over there.
		int index = action.b1 - 0xe0;
		addAction(_actionOnWalkCompletion[index]);
		_actionOnWalkCompletionInUse[index] = false;
	}

	if (_room->handleAction(action))
		return;

	// Action not defined for the room, check for default behaviour

	switch (action.type) {

	case ACTION_WALK:
		if (!_room->handleActionWithBitmask(action)) {
			Common::String animFilename = getCrewmanAnimFilename(OBJECT_KIRK, "walk");
			Common::Point mousePos = _gfx->getMousePos();
			actorWalkToPosition(OBJECT_KIRK, animFilename, _kirkActor->pos.x, _kirkActor->pos.y, mousePos.x, mousePos.y);
		}
		break;

	case ACTION_USE: // TODO
		warning("Unhandled use action: %d %d %d", action.b1, action.b2, action.b3);
		break;

	case ACTION_GET: // TODO
		warning("Unhandled get action: %d %d %d", action.b1, action.b2, action.b3);
		break;

	case ACTION_LOOK:
		if (action.activeObject() >= ITEMS_START && action.activeObject() < ITEMS_END) {
			int i = action.activeObject() - ITEMS_START;
			Common::String text = getItemDescription(_itemList[i].textIndex);
			showTextbox("", text, 20, 20, TEXTCOLOR_YELLOW, 0);
		}
		else if (action.activeObject() == OBJECT_KIRK)
			showTextbox("", getItemDescription(0x49), 20, 20, TEXTCOLOR_YELLOW, 0);
		else if (action.activeObject() == OBJECT_SPOCK)
			showTextbox("", getItemDescription(0x4a), 20, 20, TEXTCOLOR_YELLOW, 0);
		else if (action.activeObject() == OBJECT_MCCOY)
			showTextbox("", getItemDescription(0x4b), 20, 20, TEXTCOLOR_YELLOW, 0);
		else {
			if (action.activeObject() == OBJECT_REDSHIRT)
				showTextbox("", getItemDescription(0x4c), 20, 20, TEXTCOLOR_YELLOW, 0);

			// Show generic "nothing of note" text.
			// BUG? This text is also shown after looking at the redshirt. However, his
			// text is normally overridden on a per-mission basis, so perhaps this bug
			// never manifests itself?
			showTextbox("", getItemDescription(0x4d), 20, 20, TEXTCOLOR_YELLOW, 0);
		}
		break;

	case ACTION_TALK: // TODO
		warning("Unhandled talk action: %d %d %d", action.b1, action.b2, action.b3);
		break;

	case ACTION_TOUCHED_WARP:
		if (!_room->handleActionWithBitmask(action)) {
			byte warpIndex = action.b1;
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
				addAction(Action(ACTION_TOUCHED_HOTSPOT, var & 0xff, 0, 0));
			}
			return;
		}

		int16 numVertices = _room->readRdfWord(offset + 2);
		offset += numVertices * 4 + 4;
	}
	_activeDoorWarpHotspot = -1;

	if (_awayMission.crewDownBitset == 0 && _warpHotspotsActive) {
		offset = _room->getFirstWarpPolygonOffset();

		while (offset != _room->getWarpPolygonEndOffset()) {
			if (isPointInPolygon((int16*)(_room->_rdfData + offset), x, y)) {
				uint16 var = _room->readRdfWord(offset);
				if (_activeWarpHotspot != var) {
					_activeWarpHotspot = var;
					addAction(Action(ACTION_TOUCHED_WARP, var & 0xff, 0, 0));
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
 * Updates any nonzero away mission timers, and invokes ACTION_TIMER_EXPIRED when any one
 * reached 0.
 */
void StarTrekEngine::updateAwayMissionTimers() {
	for (int i = 0; i < 8; i++) {
		if (_awayMission.timers[i] == 0)
			continue;
		_awayMission.timers[i]--;
		if (_awayMission.timers[i] == 0)
			addAction(ACTION_TIMER_EXPIRED, i, 0, 0);
	}
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

void StarTrekEngine::loadRoomIndex(int roomIndex, int spawnIndex) {
	unloadRoom();
	_sound->loadMusicFile("ground");

	loadRoom(_missionName, roomIndex);
	initAwayCrewPositions(spawnIndex % 6);

	// WORKAROUND: original game calls "retrieveStackVars" to return execution directly to
	// the top of "runAwayMission". That can't really be done here. But does it matter?
}

}
