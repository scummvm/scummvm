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

#include "startrek/iwfile.h"
#include "startrek/resource.h"
#include "startrek/room.h"
#include "startrek/startrek.h"

namespace StarTrek {

void StarTrekEngine::initAwayMission() {
	_awayMission = AwayMission(); // Initialize members to 0

	// memset(bitmapBuffer->pixels, 0, 0xfa00);

	_resource->setTxtFileName("ground");

	// sub_23a60(); // TODO
	_sound->loadMusicFile("ground");

	loadRoom(_missionToLoad, _roomIndexToLoad);
	_roomIndexToLoad = -1;

	// Load crew positions for beaming in
	initAwayCrewPositions(4);
}

void StarTrekEngine::runAwayMission() {
	while (_gameMode == GAMEMODE_AWAYMISSION && !_resetGameMode) {
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
		while (!_actionQueue.empty()) {
			// sub_200e7(); // TODO
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

	_gfx->setBackgroundImage(getScreenName());
	_gfx->loadPri(getScreenName());
	_gfx->loadPalette("palette");
	_gfx->copyBackgroundScreen();

	_room = new Room(this, getScreenName());

	// Original sets up bytes 0-3 of rdf file as "remote function caller"

	bool isDemo = getFeatures() & GF_DEMO;
	if (!isDemo)
		_room->loadMapFile(getScreenName());

	_awayMission.activeAction = ACTION_WALK;

	removeDrawnActorsFromScreen();
	initActors();

	Fixed8 num = _room->getMaxScale() - _room->getMinScale();
	int16 den = _room->getMaxY() - _room->getMinY() + 1;
	_playerActorScale = Fixed16(num) / den;

	_actionQueue.clear();

	if (!isDemo) {
		int16 addr = _room->getBanDataStart();
		while (addr != _room->getBanDataEnd()) {
			Common::String name((char *)&_room->_rdfData[addr]);
			loadBanFile(name);
			addr += strlen((char *)&_room->_rdfData[addr]) + 1;
		}
	}
}

void StarTrekEngine::initAwayCrewPositions(int warpEntryIndex) {
	_sound->stopAllVocSounds();

	memset(_awayMission.crewDirectionsAfterWalk, 0xff, 4);

	switch (warpEntryIndex) {
	case 0: // 0-3: Crew spawns in a spot and walks to a spot.
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
		_sound->playSoundEffectIndex(kSfxTransporterMaterialize);
		_warpHotspotsActive = false;
		break;
	case 5: // Crew spawns in directly at a position.
		for (int i = 0; i < (_awayMission.redshirtDead ? 3 : 4); i++) {
			Common::String animFilename = getCrewmanAnimFilename(i, "stnds");
			Common::Point warpPos = _room->getSpawnPosition(i);
			loadActorAnimWithRoomScaling(i, animFilename, warpPos.x, warpPos.y);
		}
		_warpHotspotsActive = true;
		break;
	case 6:
		loadBridgeActors();
		break;
	default:
		warning("Invalid parameter (%d) to initAwayCrewPositions", warpEntryIndex);
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
			renderBanBelowSprites();
			_gfx->drawAllSprites(false);
			renderBanAboveSprites();
			_gfx->updateScreen();

			_sound->checkLoopMusic();
			updateAwayMissionTimers();
			_frameIndex++;
			_roomFrameCounter++;
			addAction(ACTION_TICK, _roomFrameCounter & 0xff, (_roomFrameCounter >> 8) & 0xff, 0);
			if (_roomFrameCounter >= 2)
				_gfx->incPaletteFadeLevel();
			break;

		case TREKEVENT_LBUTTONDOWN:
			awayMissionLeftClick();
			break; // End of TREKEVENT_LBUTTONDOWN

		case TREKEVENT_MOUSEMOVE:
			break;

		case TREKEVENT_RBUTTONDOWN:
			awayMissionSelectAction(true);
			break;

		case TREKEVENT_KEYDOWN:
			if (_awayMission.disableInput)
				break;

			switch (event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
			case Common::KEYCODE_SPACE:
			case Common::KEYCODE_F2:
				awayMissionSelectAction(true);
				break;

			case Common::KEYCODE_t:
				hideInventoryIcons();
				_awayMission.activeAction = ACTION_TALK;
				awayMissionSelectAction(false);
				break;

			case Common::KEYCODE_l:
				hideInventoryIcons();
				_awayMission.activeAction = ACTION_LOOK;
				awayMissionSelectAction(false);
				break;

			case Common::KEYCODE_g:
				hideInventoryIcons();
				_awayMission.activeAction = ACTION_GET;
				awayMissionSelectAction(false);
				break;

			case Common::KEYCODE_u:
				hideInventoryIcons();
				_awayMission.activeAction = ACTION_USE;
				awayMissionSelectAction(false);
				break;

			case Common::KEYCODE_w:
				hideInventoryIcons();
				_awayMission.activeAction = ACTION_WALK;
				break;

			case Common::KEYCODE_i:
				if (_awayMission.activeAction == ACTION_USE) {
					hideInventoryIcons();
					int clickedObject = showInventoryMenu(50, 50, true);
					if (clickedObject == -1)
						clickedObject = -2;
					awayMissionUseObject(clickedObject);
				} else if (_awayMission.activeAction == ACTION_LOOK) {
					hideInventoryIcons();
					int clickedObject = showInventoryMenu(50, 50, true);
					if (clickedObject == -1)
						clickedObject = -2;
					awayMissionGetLookOrTalk(clickedObject);
				}
				break;

			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
			case Common::KEYCODE_F1:
				awayMissionLeftClick();
				break;

			case Common::KEYCODE_c:
				// Bridge computer, where the player can ask about various topics.
				// ENHANCEMENT: Normally, this is only available when in the bridge.
				// We also show it in missions.
				if (!(getFeatures() & GF_DEMO))
					handleBridgeComputer();
				break;

			case Common::KEYCODE_p:
				// Pause game
				// TODO
				break;

			case Common::KEYCODE_e:
				if (event.kbd.flags & Common::KBD_CTRL) {
					_sound->toggleSfx();
				}
				break;

			case Common::KEYCODE_m:
				if (event.kbd.flags & Common::KBD_CTRL) {
					_sound->toggleMusic();
				}
				break;

			case Common::KEYCODE_q:
				if (event.kbd.flags & Common::KBD_CTRL) {
					showQuitGamePrompt(20, 20);
				}
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
	}
}

void StarTrekEngine::awayMissionLeftClick() {
	if (_awayMission.disableInput)
		return;

	switch (_awayMission.activeAction) {
	case ACTION_WALK: {
		if (_awayMission.disableWalking)
			break;
		_kirkActor->sprite.drawMode = 1; // Hide these objects for function call below?
		_spockActor->sprite.drawMode = 1;
		_mccoyActor->sprite.drawMode = 1;
		_redshirtActor->sprite.drawMode = 1;

		int16 clickedObject = findObjectAt(_gfx->getMousePos());

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

		int16 clickedObject = findObjectAt(_gfx->getMousePos());
		hideInventoryIcons();

		if (clickedObject == OBJECT_INVENTORY_ICON) {
			clickedObject = showInventoryMenu(50, 50, false);

			// -1 means "clicked on something unknown"; -2 means "clicked on
			// nothing". In the case of the inventory, either one clicks on an
			// inventory item, or no action is performed.
			if (clickedObject == -1)
				clickedObject = -2;
		}

		awayMissionUseObject(clickedObject);
		break;
	}

	case ACTION_GET:
	case ACTION_LOOK:
	case ACTION_TALK: {
		int16 clickedObject = findObjectAt(_gfx->getMousePos());
		if (!isObjectUnusable(clickedObject, _awayMission.activeAction)) {
			hideInventoryIcons();

			if (clickedObject == OBJECT_INVENTORY_ICON) {
				clickedObject = showInventoryMenu(50, 50, false);
				if (clickedObject == -1)
					clickedObject = -2;
			}

			awayMissionGetLookOrTalk(clickedObject);
		}
		break;
	}

	default:
		break;
	}
}

void StarTrekEngine::awayMissionSelectAction(bool openActionMenu) {
	if (openActionMenu) {
		if (_awayMission.disableInput)
			return;
		hideInventoryIcons();
		_sound->playSoundEffectIndex(kSfxButton);
		_awayMission.activeAction = showActionMenu();
	}

	if (_awayMission.activeAction == ACTION_USE) {
		int16 clickedObject = selectObjectForUseAction();
		if (clickedObject == -1)
			return;
		else
			_awayMission.activeObject = clickedObject;
	}
	if (_awayMission.activeAction == ACTION_USE
			&& _awayMission.activeObject == OBJECT_ICOMM && (_awayMission.crewDownBitset & (1 << OBJECT_KIRK)) == 0) {
		if (!walkActiveObjectToHotspot()) {
			addAction(_awayMission.activeAction, _awayMission.activeObject, 0, 0);
			_sound->playVoc("communic");
			_awayMission.activeAction = ACTION_WALK;
		}
	} else if (_awayMission.activeAction == ACTION_LOOK)
		showInventoryIcons(false);
	else if (_awayMission.activeAction == ACTION_USE && (_awayMission.crewDownBitset & (1 << OBJECT_KIRK)) == 0)
		showInventoryIcons(true);
}

void StarTrekEngine::awayMissionUseObject(int16 clickedObject) {
	_awayMission.passiveObject = clickedObject;

	bool activeIsCrewman = _awayMission.activeObject <= OBJECT_REDSHIRT;
	bool activeIsItem = _awayMission.activeObject >= ITEMS_START && _awayMission.activeObject < ITEMS_END;
	bool passiveIsCrewman = _awayMission.passiveObject <= OBJECT_REDSHIRT;
	bool passiveIsItem = _awayMission.passiveObject >= ITEMS_START && _awayMission.passiveObject <= ITEMS_END; // FIXME: "<= ITEMS_END" doesn't make sense?

	bool tryWalkToHotspot = false;
	bool showInventory = false;

	if (clickedObject == -2)
		tryWalkToHotspot = true;
	else if (_room->actionHasCode(ACTION_USE, _awayMission.activeObject, _awayMission.passiveObject, 0))
		tryWalkToHotspot = true;
	else if (_awayMission.activeObject == OBJECT_MCCOY && _room->actionHasCode(ACTION_USE, OBJECT_IMEDKIT, _awayMission.passiveObject, 0))
		tryWalkToHotspot = true;
	// CHECKME: Identical to the previous check, thus never used
	//else if (_awayMission.activeObject == OBJECT_MCCOY && _room->actionHasCode(ACTION_USE, OBJECT_IMEDKIT, _awayMission.passiveObject, 0))
	//	tryWalkToHotspot = true;
	else if (_awayMission.activeObject == OBJECT_SPOCK && _room->actionHasCode(ACTION_USE, OBJECT_ISTRICOR, _awayMission.passiveObject, 0))
		tryWalkToHotspot = true;

	if (!tryWalkToHotspot) {
		if ((activeIsCrewman && passiveIsCrewman)
				|| (activeIsCrewman && passiveIsItem)
				|| (activeIsItem && passiveIsItem)) {
			if (_awayMission.passiveObject == OBJECT_ICOMM) {
				if (walkActiveObjectToHotspot())
					return;
				addAction(ACTION_USE, OBJECT_ICOMM, 0, 0);
				_sound->playVoc("commun30");
				if (_awayMission.activeObject <= OBJECT_REDSHIRT) {
					showInventory = true;
				} else {
					_awayMission.activeAction = ACTION_WALK;
					return;
				}
			}

			_awayMission.activeObject = _awayMission.passiveObject;
			showInventory = true;
		} else
			tryWalkToHotspot = true;
	}

	if (tryWalkToHotspot) {
		if (!walkActiveObjectToHotspot()) {
			if (clickedObject != -2)
				addAction(_awayMission.activeAction, _awayMission.activeObject, _awayMission.passiveObject, 0);
			showInventory = true;
		}
	}

	if (showInventory && !(_awayMission.crewDownBitset & (1 << OBJECT_KIRK)))
		showInventoryIcons(true);
}

void StarTrekEngine::awayMissionGetLookOrTalk(int16 clickedObject) {
	_awayMission.activeObject = clickedObject;

	if (walkActiveObjectToHotspot())
		return;

	if (clickedObject != -2)
		addAction(_awayMission.activeAction, _awayMission.activeObject, 0, 0);

	if (_awayMission.activeAction == ACTION_LOOK && !(_awayMission.crewDownBitset & (1 << OBJECT_KIRK)))
		showInventoryIcons(false);
}

void StarTrekEngine::unloadRoom() {
	_gfx->fadeoutScreen();
	// sub_2394b(); // TODO
	removeDrawnActorsFromScreen();
	delete _room;
	_room = nullptr;
	delete _mapFile;
	_mapFile = nullptr;
	delete _iwFile;
	_iwFile = nullptr;
}

int StarTrekEngine::loadActorAnimWithRoomScaling(int actorIndex, const Common::String &animName, int16 x, int16 y) {
	Fixed8 scale = getActorScaleAtPosition(y);
	return loadActorAnim(actorIndex, animName, x, y, scale);
}

Fixed8 StarTrekEngine::getActorScaleAtPosition(int16 y) {
	int16 maxY = _room->getMaxY();
	int16 minY = _room->getMinY();
	Fixed8 minScale = _room->getMinScale();

	if (y > maxY)
		y = maxY;
	if (y < minY)
		y = minY;

	return Fixed8(_playerActorScale * (y - minY)) + minScale;
}

Room *StarTrekEngine::getRoom() {
	return _room;
}

void StarTrekEngine::addAction(const Action &action) {
	if (action.type != ACTION_TICK)
		debugC(kDebugGeneral, 4, "Action %d: %x, %x, %x", action.type, action.b1, action.b2, action.b3);
	_actionQueue.push(action);
}

void StarTrekEngine::addAction(int8 type, byte b1, byte b2, byte b3) {
	const Action a = {type, b1, b2, b3, 0};
	addAction(a);
}

void StarTrekEngine::handleAwayMissionAction() {
	Action action = _actionQueue.pop();

	if ((action.type == ACTION_FINISHED_ANIMATION || action.type == ACTION_FINISHED_WALKING) && action.b1 == 0xff) {
		// Just finished walking or beaming into a room
		if (_awayMission.disableInput == 1)
			_awayMission.disableInput = false;
		_warpHotspotsActive = true;
		return;
	} else if (action.type == ACTION_FINISHED_WALKING && action.b1 >= 0xe0) {
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

	case ACTION_USE:
		if (action.activeObject() != action.passiveObject()) {
			switch (action.activeObject()) {
			case OBJECT_KIRK:
				// BUGFIX: Don't allow the "use" action to bypass the "disableWalking" variable
				if (!(!_awayMission.disableWalking && _room->handleAction(ACTION_WALK, action.passiveObject(), 0, 0))
						&& !_room->handleAction(ACTION_GET, action.passiveObject(), 0, 0)) {
					showTextbox("Capt. Kirk", _resource->getLoadedText(GROUNDTX_KIRK_USE), 20, 20, TEXTCOLOR_YELLOW, 0);
				}
				break;

			case OBJECT_SPOCK:
				if (!_room->handleAction(ACTION_USE, OBJECT_ISTRICOR, action.passiveObject(), 0)) {
					// BUGFIX: Original game has just "Spock" instead of "Mr. Spock" as the
					// speaker. That's inconsistent.
					// Same applies to other parts of this function.
					showTextbox("Mr. Spock", _resource->getLoadedText(GROUNDTX_SPOCK_USE), 20, 20, TEXTCOLOR_BLUE, 0);
				}
				break;

			case OBJECT_MCCOY:
				if (!_room->handleAction(ACTION_USE, OBJECT_IMEDKIT, action.passiveObject(), 0)
						&& !_room->handleAction(ACTION_USE, OBJECT_IMTRICOR, action.passiveObject(), 0)) {
					// BUGFIX: Original game has just "McCoy" instead of "Dr. McCoy".
					showTextbox("Dr. McCoy", _resource->getLoadedText(GROUNDTX_MCCOY_USE), 20, 20, TEXTCOLOR_BLUE, 0);
				}
				break;

			case OBJECT_REDSHIRT:
				showTextbox(NULL, _resource->getLoadedText(GROUNDTX_REDSHIRT_USE), 20, 20, TEXTCOLOR_YELLOW, 0);
				break;

			case OBJECT_IPHASERS:
			case OBJECT_IPHASERK:
				if (action.passiveObject() == OBJECT_SPOCK) {
					int text = GROUNDTX_PHASER_ON_SPOCK + getRandomWord() % 8;
					showTextbox("Mr. Spock", _resource->getLoadedText(text), 20, 20, TEXTCOLOR_BLUE, 0);
				} else if (action.passiveObject() == OBJECT_MCCOY) {
					int text = GROUNDTX_PHASER_ON_MCCOY + getRandomWord() % 8;
					showTextbox("Dr. McCoy", _resource->getLoadedText(text), 20, 20, TEXTCOLOR_BLUE, 0);
				} else if (action.passiveObject() == OBJECT_REDSHIRT) {
					Common::String text = _resource->getLoadedText(GROUNDTX_PHASER_ON_REDSHIRT + getRandomWord() % 8);
					// Replace audio filename with start of mission name (to load the
					// audio for the crewman specific to the mission))
					text.setChar(_missionName[0], 6);
					text.setChar(_missionName[1], 7);
					text.setChar(_missionName[2], 8);
					showTextbox("Security Officer", text, 20, 20, TEXTCOLOR_RED, 0);
					// TODO: replace "Security Officer" string with their actual name as
					// an enhancement?
				} else if (!_room->handleActionWithBitmask(action)) {
					int index = getRandomWord() % 7;
					if (index & 1)
						showTextbox("Dr. McCoy", _resource->getLoadedText(GROUNDTX_PHASER_ANYWHERE + index), 20, 20, TEXTCOLOR_BLUE, 0);
					else
						showTextbox("Mr. Spock", _resource->getLoadedText(GROUNDTX_PHASER_ANYWHERE + index), 20, 20, TEXTCOLOR_BLUE, 0);
				}
				break;

			case OBJECT_ISTRICOR:
				showTextbox("Mr. Spock", _resource->getLoadedText(GROUNDTX_SPOCK_SCAN), 20, 20, TEXTCOLOR_BLUE, 0);
				break;

			case OBJECT_IMTRICOR:
				showTextbox("Dr. McCoy", _resource->getLoadedText(GROUNDTX_MCCOY_SCAN), 20, 20, TEXTCOLOR_BLUE, 0);
				break;

			case OBJECT_ICOMM:
				if (!_room->handleAction(ACTION_USE, OBJECT_ICOMM, 0xff, 0))
					showTextbox("Lt. Uhura", _resource->getLoadedText(GROUNDTX_USE_COMMUNICATOR), 20, 20, TEXTCOLOR_RED, 0);
				break;

			case OBJECT_IMEDKIT:
				showTextbox("Dr. McCoy", _resource->getLoadedText(GROUNDTX_USE_MEDKIT), 20, 20, TEXTCOLOR_BLUE, 0);
				break;

			default:
				if (!_room->handleActionWithBitmask(action.type, action.b1, action.b2, action.b3))
					showTextbox("", _resource->getLoadedText(GROUNDTX_NOTHING_HAPPENS), 20, 20, TEXTCOLOR_YELLOW, 0);
			}
		}
		break;

	case ACTION_GET:
		if (!_room->handleActionWithBitmask(action.type, action.b1, action.b2, action.b3))
			showTextbox("", _resource->getLoadedText(GROUNDTX_FAIL_TO_OBTAIN_ANYTHING), 20, 20, TEXTCOLOR_YELLOW, 0);
		break;

	case ACTION_LOOK:
		if (action.activeObject() >= ITEMS_START && action.activeObject() < ITEMS_END) {
			int i = action.activeObject() - ITEMS_START;
			Common::String text = _resource->getLoadedText(_itemList[i].textIndex);
			showTextbox("", text, 20, 20, TEXTCOLOR_YELLOW, 0);
		} else if (action.activeObject() == OBJECT_KIRK)
			showTextbox("", _resource->getLoadedText(GROUNDTX_LOOK_KIRK), 20, 20, TEXTCOLOR_YELLOW, 0);
		else if (action.activeObject() == OBJECT_SPOCK)
			showTextbox("", _resource->getLoadedText(GROUNDTX_LOOK_SPOCK), 20, 20, TEXTCOLOR_YELLOW, 0);
		else if (action.activeObject() == OBJECT_MCCOY)
			showTextbox("", _resource->getLoadedText(GROUNDTX_LOOK_MCCOY), 20, 20, TEXTCOLOR_YELLOW, 0);
		else if (action.activeObject() == OBJECT_REDSHIRT)
			showTextbox("", _resource->getLoadedText(GROUNDTX_LOOK_REDSHIRT), 20, 20, TEXTCOLOR_YELLOW, 0);
		else
			// Show generic "nothing of note" text.
			// BUGFIX: originally this was shown after the redshirt's text as well.
			// Though, the original game may not have used this default implementation
			// anywhere...
			showTextbox("", _resource->getLoadedText(GROUNDTX_LOOK_ANYWHERE), 20, 20, TEXTCOLOR_YELLOW, 0);
		break;

	case ACTION_TALK:
		switch (action.activeObject()) {
		case OBJECT_KIRK:
		case OBJECT_SPOCK:
		case OBJECT_MCCOY:
		case OBJECT_REDSHIRT:
			showTextbox("", _resource->getLoadedText(GROUNDTX_TALK_TO_CREWMAN), 20, 20, TEXTCOLOR_YELLOW, 0);
			break;

		default:
			showTextbox("", _resource->getLoadedText(GROUNDTX_NO_RESPONSE), 20, 20, TEXTCOLOR_YELLOW, 0);
			break;
		}
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

	default:
		_room->handleActionWithBitmask(action);
		break;
	}
}

void StarTrekEngine::checkTouchedLoadingZone(int16 x, int16 y) {
	int16 offset = _room->getFirstDoorPolygonOffset();

	while (offset != _room->getDoorPolygonEndOffset()) {
		if (_room->isPointInPolygon(offset, x, y)) {
			uint16 var = _room->readRdfWord(offset);
			if (_activeDoorWarpHotspot != var) {
				_activeDoorWarpHotspot = var;
				addAction(ACTION_TOUCHED_HOTSPOT, var & 0xff, 0, 0);
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
			if (_room->isPointInPolygon(offset, x, y)) {
				uint16 var = _room->readRdfWord(offset);
				if (_activeWarpHotspot != var) {
					_activeWarpHotspot = var;
					addAction(ACTION_TOUCHED_WARP, var & 0xff, 0, 0);
				}
				return;
			}

			int16 numVertices = _room->readRdfWord(offset + 2);
			offset += numVertices * 4 + 4;
		}
	}
	_activeWarpHotspot = -1;
}

void StarTrekEngine::updateAwayMissionTimers() {
	for (int i = 0; i < 8; i++) {
		if (_awayMission.timers[i] == 0)
			continue;
		_awayMission.timers[i]--;
		if (_awayMission.timers[i] == 0)
			addAction(ACTION_TIMER_EXPIRED, i, 0, 0);
	}
}

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

} // End of namespace StarTrek
