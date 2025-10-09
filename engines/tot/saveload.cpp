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
#include "common/hashmap.h"
#include "common/savefile.h"
#include "common/translation.h"
#include "graphics/thumbnail.h"
#include "gui/message.h"

#include "tot/forest.h"
#include "tot/tot.h"

namespace Tot {

#define SAVEGAME_CURRENT_VERSION 1

bool syncGeneralData(Common::Serializer &s, SavedGame *game) {
	// Uint16
	s.syncAsUint16LE(game->roomCode);
	s.syncAsUint16LE(game->trajectoryLength);
	s.syncAsUint16LE(game->currentTrajectoryIndex);
	s.syncAsUint16LE(game->backpackObjectCode);
	s.syncAsUint16LE(game->rightSfxVol);
	s.syncAsUint16LE(game->leftSfxVol);
	s.syncAsUint16LE(game->musicVolRight);
	s.syncAsUint16LE(game->musicVolLeft);
	s.syncAsUint16LE(game->oldGridX);
	s.syncAsUint16LE(game->oldGridY);
	s.syncAsUint16LE(game->secAnimDepth);
	s.syncAsUint16LE(game->secAnimDir);
	s.syncAsUint16LE(game->secAnimX);
	s.syncAsUint16LE(game->secAnimY);
	s.syncAsUint16LE(game->secAnimIFrame);
	// Bytes
	s.syncAsByte(game->currentZone);
	s.syncAsByte(game->targetZone);
	s.syncAsByte(game->oldTargetZone);
	s.syncAsByte(game->inventoryPosition);
	s.syncAsByte(game->actionCode);
	s.syncAsByte(game->oldActionCode);
	s.syncAsByte(game->steps);
	s.syncAsByte(game->doorIndex);
	s.syncAsByte(game->characterFacingDir);
	s.syncAsByte(game->iframe);
	s.syncAsByte(game->gamePart);

	// Booleans
	s.syncAsByte(game->isSealRemoved);
	s.syncAsByte(game->obtainedList1);
	s.syncAsByte(game->obtainedList2);
	s.syncAsByte(game->list1Complete);
	s.syncAsByte(game->list2Complete);
	s.syncAsByte(game->isVasePlaced);
	s.syncAsByte(game->isScytheTaken);
	s.syncAsByte(game->isTridentTaken);
	s.syncAsByte(game->isPottersWheelDelivered);
	s.syncAsByte(game->isMudDelivered);
	s.syncAsByte(game->isGreenDevilDelivered);
	s.syncAsByte(game->isRedDevilCaptured);
	s.syncAsByte(game->isPottersManualDelivered);
	s.syncAsByte(game->isCupboardOpen);
	s.syncAsByte(game->isChestOpen);
	s.syncAsByte(game->isTVOn);
	s.syncAsByte(game->isTrapSet);

	for (int i = 0; i < kInventoryIconCount; i++) {
		s.syncAsUint16LE(game->mobj[i].bitmapIndex);
		s.syncAsUint16LE(game->mobj[i].code);
		s.syncString(game->mobj[i].objectName);
	}

	// integers
	s.syncAsSint32LE(game->element1);
	s.syncAsSint32LE(game->element2);
	s.syncAsSint32LE(game->characterPosX);
	s.syncAsSint32LE(game->characterPosY);
	s.syncAsSint32LE(game->xframe2);
	s.syncAsSint32LE(game->yframe2);

	// Strings
	s.syncString(game->oldInventoryObjectName);
	s.syncString(game->objetomoinventoryObjectNamehila);
	s.syncString(game->characterName);

	for (int i = 0; i < kRoutePointCount; i++) {
		s.syncAsSint16LE(game->mainRoute[i].x);
		s.syncAsSint16LE(game->mainRoute[i].y);
	}

	for (int i = 0; i < 300; i++) {
		s.syncAsSint16LE(game->trajectory[i].x);
		s.syncAsSint16LE(game->trajectory[i].y);
	}

	for (int indiaux = 0; indiaux < kCharacterCount; indiaux++) {
		// interleave them just to avoid creating many loops
		s.syncAsByte(game->firstTimeTopicA[indiaux]);
		s.syncAsByte(game->firstTimeTopicB[indiaux]);
		s.syncAsByte(game->firstTimeTopicC[indiaux]);
		s.syncAsByte(game->bookTopic[indiaux]);
		s.syncAsByte(game->mintTopic[indiaux]);
	}

	for (int indiaux = 0; indiaux < 5; indiaux++) {
		s.syncAsByte(game->caves[indiaux]);
		s.syncAsUint16LE(game->firstList[indiaux]);
		s.syncAsUint16LE(game->secondList[indiaux]);
	}

	for (int indiaux = 0; indiaux < 4; indiaux++) {
		s.syncAsUint16LE(game->niche[0][indiaux]);
		s.syncAsUint16LE(game->niche[1][indiaux]);
	}
	return true;
}

bool syncRoomData(Common::Serializer &s, Common::MemorySeekableReadWriteStream *roomStream) {
	if (s.isSaving()) {

		// Restore trajectory
		g_engine->setRoomTrajectories(g_engine->_secondaryAnimHeight, g_engine->_secondaryAnimWidth, RESTORE);
		// Make sure to save any unsaved changes in the room
		g_engine->saveRoomData(g_engine->_currentRoomData, g_engine->_rooms);

		// Do not fix screen grids, they will be fixed differently below
		g_engine->setRoomTrajectories(g_engine->_secondaryAnimHeight, g_engine->_secondaryAnimWidth, SET_WITH_ANIM);

		int size = roomStream->size();
		byte *roomBuf = (byte *)malloc(size);
		roomStream->seek(0, 0);
		roomStream->read(roomBuf, size);
		s.syncBytes(roomBuf, size);
		free(roomBuf);
	}
	if (s.isLoading()) {
		int size = g_engine->_rooms->size();
		delete (g_engine->_rooms);
		byte *roomBuf = (byte *)malloc(size);
		s.syncBytes(roomBuf, size);

		g_engine->_rooms = new Common::MemorySeekableReadWriteStream(roomBuf, size, DisposeAfterUse::NO);
	}
	return true;
}

bool syncConversationData(Common::Serializer &s, Common::MemorySeekableReadWriteStream *conversations) {

	int size = conversations->size();
	if (s.isSaving()) {

		byte *convBuf = (byte *)malloc(size);
		conversations->seek(0, 0);
		conversations->read(convBuf, size);
		s.syncBytes(convBuf, size);
		free(convBuf);
	}
	if (s.isLoading()) {
		delete (g_engine->_conversationData);
		byte *convBuf = (byte *)malloc(size);
		s.syncBytes(convBuf, size);
		g_engine->_conversationData = new Common::MemorySeekableReadWriteStream(convBuf, size, DisposeAfterUse::NO);
	}
	return true;
}

bool syncItemData(Common::Serializer &s, Common::MemorySeekableReadWriteStream *sceneObjects) {
	int size = sceneObjects->size();
	if (s.isSaving()) {
		byte *objBuf = (byte *)malloc(size);
		sceneObjects->seek(0, 0);
		sceneObjects->read(objBuf, size);
		s.syncBytes(objBuf, size);
		free(objBuf);
	}
	if (s.isLoading()) {
		delete (g_engine->_sceneObjectsData);
		byte *objBuf = (byte *)malloc(size);
		s.syncBytes(objBuf, size);
		g_engine->_sceneObjectsData = new Common::MemorySeekableReadWriteStream(objBuf, size, DisposeAfterUse::NO);
	}
	return true;
}

Common::Error syncSaveData(Common::Serializer &ser, SavedGame *game) {
	if (!syncGeneralData(ser, game)) {
		warning("Error while synchronizing general data");
		return Common::kUnknownError;
	}
	if (!syncRoomData(ser, g_engine->_rooms)) {
		warning("Error while synchronizing room data");
		return Common::kUnknownError;
	}
	if (!syncItemData(ser, g_engine->_sceneObjectsData)) {
		warning("Error while syncrhonizing object data");
		return Common::kUnknownError;
	}
	if (!syncConversationData(ser, g_engine->_conversationData)) {
		warning("Error while syncrhonizing conversation data");
		return Common::kUnknownError;
	}
	if (ser.err()) {
		warning("Error while synchronizing");
		return Common::kUnknownError;
	}

	return Common::kNoError;
}

Common::Error TotEngine::syncGame(Common::Serializer &s) {
	Common::Error result;

	if (s.isLoading()) {
		SavedGame *loadedGame = new SavedGame();
		// Means we are loading from before the game has started
		// if(rooms == nullptr) {
		_graphics->clear();
		displayLoading();

		loadCharAnimation();
		loadInventory();

		_graphics->loadPaletteFromFile("DEFAULT");
		initScreenPointers();

		_graphics->totalFadeOut(0);
		_graphics->clear();
		initializeScreenFile();
		initializeObjectFile();
		readConversationFile();

		result = syncSaveData(s, loadedGame);
		loadGame(loadedGame);
	} else {
		SavedGame *saveGame = saveGameToRegister();
		result = syncSaveData(s, saveGame);
		free(saveGame);
	}
	return result;
}

Common::Error TotEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	const byte version = SAVEGAME_CURRENT_VERSION;
	Common::Serializer s(nullptr, stream);
	s.setVersion(version);
	stream->writeByte(version);

	return syncGame(s);
}
Common::Error TotEngine::loadGameStream(Common::SeekableReadStream *stream) {
	byte version = stream->readByte();
	if (version > SAVEGAME_CURRENT_VERSION) {
		GUI::MessageDialog dialog(_("Saved game was created with a newer version of ScummVM. Unable to load."));
		dialog.runModal();
		return Common::kUnknownError;
	}

	Common::Serializer s(stream, nullptr);
	s.setVersion(version);

	return syncGame(s);
}

bool TotEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return true;
}
bool TotEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	return _inGame && _saveAllowed;
}

SavedGame *TotEngine::saveGameToRegister() {
	SavedGame *_savedGame = new SavedGame();
	_savedGame->roomCode = _currentRoomData->code;
	_savedGame->trajectoryLength = _trajectoryLength;
	_savedGame->currentTrajectoryIndex = _currentTrajectoryIndex;
	_savedGame->backpackObjectCode = _backpackObjectCode;
	_savedGame->rightSfxVol = _sound->_rightSfxVol;
	_savedGame->leftSfxVol = _sound->_leftSfxVol;
	_savedGame->musicVolRight = _sound->_musicVolRight;
	_savedGame->musicVolLeft = _sound->_musicVolLeft;
	_savedGame->oldGridX = _oldGridX;
	_savedGame->oldGridY = _oldGridY;
	_savedGame->secAnimDepth = _secondaryAnimation.depth;
	_savedGame->secAnimDir = _secondaryAnimation.dir;
	_savedGame->secAnimX = _secondaryAnimation.posx;
	_savedGame->secAnimY = _secondaryAnimation.posy;
	_savedGame->secAnimIFrame = _iframe2;

	_savedGame->currentZone = _currentZone;
	_savedGame->targetZone = _targetZone;
	_savedGame->oldTargetZone = _oldTargetZone;
	_savedGame->inventoryPosition = _inventoryPosition;
	_savedGame->actionCode = _actionCode;
	_savedGame->oldActionCode = _oldActionCode;
	_savedGame->steps = _trajectorySteps;
	_savedGame->doorIndex = _doorIndex;
	_savedGame->characterFacingDir = _charFacingDirection;
	_savedGame->iframe = _iframe;
	_savedGame->gamePart = _gamePart;

	_savedGame->isSealRemoved = _isSealRemoved;
	_savedGame->obtainedList1 = _obtainedList1;
	_savedGame->obtainedList2 = _obtainedList2;
	_savedGame->list1Complete = _list1Complete;
	_savedGame->list2Complete = _list2Complete;
	_savedGame->isVasePlaced = _isVasePlaced;
	_savedGame->isScytheTaken = _isScytheTaken;
	_savedGame->isTridentTaken = _isTridentTaken;
	_savedGame->isPottersWheelDelivered = _isPottersWheelDelivered;
	_savedGame->isMudDelivered = _isMudDelivered;
	_savedGame->isGreenDevilDelivered = _isGreenDevilDelivered;
	_savedGame->isRedDevilCaptured = _isRedDevilCaptured;
	_savedGame->isPottersManualDelivered = _isPottersManualDelivered;
	_savedGame->isCupboardOpen = _isCupboardOpen;
	_savedGame->isChestOpen = _isChestOpen;
	_savedGame->isTVOn = _isTVOn;
	_savedGame->isTrapSet = _isTrapSet;

	for (int i = 0; i < kInventoryIconCount; i++) {
		_savedGame->mobj[i].bitmapIndex = _inventory[i].bitmapIndex;
		_savedGame->mobj[i].code = _inventory[i].code;
		_savedGame->mobj[i].objectName = _inventory[i].objectName;
	}

	_savedGame->element1 = _element1;
	_savedGame->element2 = _element2;
	_savedGame->characterPosX = _characterPosX;
	_savedGame->characterPosY = _characterPosY;
	_savedGame->xframe2 = _xframe2;
	_savedGame->yframe2 = _yframe2;

	_savedGame->oldInventoryObjectName = _oldInventoryObjectName;
	_savedGame->objetomoinventoryObjectNamehila = _inventoryObjectName;
	_savedGame->characterName = _characterName;

	for (int i = 0; i < kRoutePointCount; i++) {
		_savedGame->mainRoute[i].x = _mainRoute[i].x;
		_savedGame->mainRoute[i].y = _mainRoute[i].y;
	}

	for (int i = 0; i < 300; i++) {
		_savedGame->trajectory[i].x = _trajectory[i].x;
		_savedGame->trajectory[i].y = _trajectory[i].y;
	}

	for (int i = 0; i < kCharacterCount; i++) {
		_savedGame->firstTimeTopicA[i] = _firstTimeTopicA[i];
		_savedGame->firstTimeTopicB[i] = _firstTimeTopicB[i];
		_savedGame->firstTimeTopicC[i] = _firstTimeTopicC[i];
		_savedGame->bookTopic[i] = _bookTopic[i];
		_savedGame->mintTopic[i] = _mintTopic[i];
	}
	for (int i = 0; i < 5; i++) {
		_savedGame->caves[i] = _caves[i];
		_savedGame->firstList[i] = _firstList[i];
		_savedGame->secondList[i] = _secondList[i];
	}
	for (int i = 0; i < 4; i++) {
		_savedGame->niche[0][i] = _niche[0][i];
		_savedGame->niche[1][i] = _niche[1][i];
	}
	return _savedGame;
}

void TotEngine::loadGame(SavedGame *game) {
	clearAnimation();
	clearScreenLayers();

	_trajectoryLength = game->trajectoryLength;
	_currentTrajectoryIndex = game->currentTrajectoryIndex;
	_backpackObjectCode = game->backpackObjectCode;
	_sound->_rightSfxVol = game->rightSfxVol;
	_sound->_leftSfxVol = game->leftSfxVol;
	_sound->_musicVolRight = game->musicVolRight;
	_sound->_musicVolLeft = game->musicVolLeft;
	_oldGridX = game->oldGridX;
	_oldGridY = game->oldGridY;
	_secondaryAnimation.depth = game->secAnimDepth;
	_secondaryAnimation.dir = game->secAnimDir;
	_secondaryAnimation.posx = game->secAnimX;
	_secondaryAnimation.posy = game->secAnimY;
	_iframe2 = game->secAnimIFrame;
	_currentZone = game->currentZone;
	_targetZone = game->targetZone;
	_oldTargetZone = game->oldTargetZone;
	_inventoryPosition = game->inventoryPosition;
	_actionCode = game->actionCode;
	_oldActionCode = game->oldActionCode;
	_trajectorySteps = game->steps;
	_doorIndex = game->doorIndex;
	_charFacingDirection = game->characterFacingDir;
	_iframe = game->iframe;
	if (game->gamePart != _gamePart) {
		_gamePart = game->gamePart;
		for (int i = 0; i < kInventoryIconCount; i++) {
			free(_inventoryIconBitmaps[i]);
		}
		loadInventory();
	}
	_isSealRemoved = game->isSealRemoved;
	_obtainedList1 = game->obtainedList1;
	_obtainedList2 = game->obtainedList2;
	_list1Complete = game->list1Complete;
	_list2Complete = game->list2Complete;
	_isVasePlaced = game->isVasePlaced;
	_isScytheTaken = game->isScytheTaken;
	if (_cpCounter > 24)
		showError(274);
	_isTridentTaken = game->isTridentTaken;
	_isPottersWheelDelivered = game->isPottersWheelDelivered;
	_isMudDelivered = game->isMudDelivered;
	_isGreenDevilDelivered = game->isGreenDevilDelivered;
	_isRedDevilCaptured = game->isRedDevilCaptured;
	_isPottersManualDelivered = game->isPottersManualDelivered;
	_isCupboardOpen = game->isCupboardOpen;
	_isChestOpen = game->isChestOpen;
	_isTVOn = game->isTVOn;
	_isTrapSet = game->isTrapSet;
	for (int i = 0; i < kInventoryIconCount; i++) {
		_inventory[i].bitmapIndex = game->mobj[i].bitmapIndex;
		_inventory[i].code = game->mobj[i].code;
		_inventory[i].objectName = game->mobj[i].objectName;
	}
	_element1 = game->element1;
	_element2 = game->element2;
	_characterPosX = game->characterPosX;
	_characterPosY = game->characterPosY;
	_xframe2 = game->xframe2;
	_yframe2 = game->yframe2;
	_oldInventoryObjectName = game->oldInventoryObjectName;
	_inventoryObjectName = game->objetomoinventoryObjectNamehila;
	_characterName = game->characterName;
	for (int i = 0; i < kRoutePointCount; i++) {
		_mainRoute[i].x = game->mainRoute[i].x;
		_mainRoute[i].y = game->mainRoute[i].y;
	}
	for (int indiaux = 0; indiaux < 300; indiaux++) {
		_trajectory[indiaux].x = game->trajectory[indiaux].x;
		_trajectory[indiaux].y = game->trajectory[indiaux].y;
	}
	for (int i = 0; i < kCharacterCount; i++) {
		_firstTimeTopicA[i] = game->firstTimeTopicA[i];
		_firstTimeTopicB[i] = game->firstTimeTopicB[i];
		_firstTimeTopicC[i] = game->firstTimeTopicC[i];
		_bookTopic[i] = game->bookTopic[i];
		_mintTopic[i] = game->mintTopic[i];
	}
	for (int i = 0; i < 5; i++) {
		_caves[i] = game->caves[i];
		_firstList[i] = game->firstList[i];
		_secondList[i] = game->secondList[i];
	}
	for (int i = 0; i < 4; i++) {
		_niche[0][i] = game->niche[0][i];
		_niche[1][i] = game->niche[1][i];
	}

	_graphics->totalFadeOut(0);
	_screen->clear();
	_graphics->loadPaletteFromFile("DEFAULT");
	loadScreenData(game->roomCode);

	switch (_currentRoomData->code) {
	case 2: {
		if (_isTVOn)
			_sound->autoPlayVoc("PARASITO", 355778, 20129);
		else
			loadTV();
		_sound->setSfxVolume(_sound->_leftSfxVol, _sound->_rightSfxVol);
	} break;
	case 4: {
		_sound->loadVoc("GOTA", 140972, 1029);
		_sound->setSfxVolume(_sound->_leftSfxVol, _sound->_rightSfxVol);
	} break;
	case 5: {
		_sound->setSfxVolume(_sound->_leftSfxVol, 0);
		_sound->autoPlayVoc("CALDERA", 6433, 15386);
	} break;
	case 6: {
		_sound->setSfxVolume(_sound->_leftSfxVol, _sound->_rightSfxVol);
		_sound->autoPlayVoc("CALDERA", 6433, 15386);
	} break;
	case 17: {
		if (_bookTopic[0] == true && _currentRoomData->animationFlag)
			disableSecondAnimation();
	} break;
	case 20: {
		switch (_niche[0][_niche[0][3]]) {
		case 0:
			_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(4);
			break;
		case 561:
			_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(5);
			break;
		case 563:
			_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(6);
			break;
		case 615:
			_currentRoomData->screenObjectIndex[9]->objectName = getObjectName(7);
			break;
		}
	} break;
	case 23: {
		_sound->autoPlayVoc("Fuente", 0, 0);
		_sound->setSfxVolume(_sound->_leftSfxVol, _sound->_rightSfxVol);
	} break;
	case 24: {
		switch (_niche[1][_niche[1][3]]) {
		case 0:
			_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(4);
			break;
		case 561:
			_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(5);
			break;
		case 615:
			_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(7);
			break;
		case 622:
			_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(8);
			break;
		case 623:
			_currentRoomData->screenObjectIndex[8]->objectName = getObjectName(9);
			break;
		}
		if (_isTrapSet) {
			_currentRoomData->animationFlag = true;
			loadAnimation(_currentRoomData->animationName);
			_iframe2 = 0;
			_currentSecondaryTrajectoryIndex = 1;
			_currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].x = 214 - 15;
			_currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].y = 115 - 42;
			_secondaryAnimation.dir = _currentRoomData->secondaryAnimDirections[_currentSecondaryTrajectoryIndex - 1];
			_secondaryAnimation.posx = _currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].x;
			_secondaryAnimation.posy = _currentRoomData->secondaryAnimTrajectory[_currentSecondaryTrajectoryIndex - 1].y;
			_secondaryAnimation.depth = 14;

			for (int i = 0; i < _maxXGrid; i++)
				for (int j = 0; j < _maxYGrid; j++) {
					if (_maskGridSecondaryAnim[i][j] > 0)
						_currentRoomData->walkAreasGrid[_oldposx + i][_oldposy + j] = _maskGridSecondaryAnim[i][j];
					if (_maskMouseSecondaryAnim[i][j] > 0)
						_currentRoomData->mouseGrid[_oldposx + i][_oldposy + j] = _maskMouseSecondaryAnim[i][j];
				}
		}
		assembleScreen();
	} break;
	}

	drawInventoryMask();
	_inventoryPosition = 0;
	drawInventory();
	if (_isRedDevilCaptured == false && _currentRoomData->code == 24 && _isTrapSet == false)
		runaroundRed();
	_graphics->sceneTransition(false, _sceneBackground);

	_saveAllowed = true;
}

const int kMaxSaveSlots = 100;

Common::String drawAndSelectSaves(int pageNumber, Common::HashMap<int, SaveStateDescriptor> saveMap, uint selectedSlot) {
	g_engine->_mouse->hide();
	const char *availableText = getHardcodedTextsByCurrentLanguage()[11];
	ExtendedSavegameHeader header;
	int selectedGame = selectedSlot + pageNumber * 6;
	Common::String selectedGameDesc = "";
	for (uint i = 0; i < 6; i++) {
		int saveSlot = i + pageNumber * 6;
		int color = saveSlot == selectedGame ? 255 : 253;
		bar(60, 31 + (i * 15), 259, 39 + (i * 15), 251);
		if (saveSlot >= kMaxSaveSlots) {
			continue;
			;
		}
		Common::String desc = "";
		if (saveMap.contains(saveSlot)) {
			desc = saveMap[saveSlot].getDescription();
			if (saveSlot == selectedGame) {
				selectedGameDesc = saveMap[saveSlot].getDescription();
			}
		}
		if (desc.empty() == false) {
			euroText(65, 29 + (i * 15), desc, color);
		} else {
			euroText(65, 29 + (i * 15), Common::String::format(availableText, saveSlot), color);
		}
	}

	g_engine->_mouse->show();
	return selectedGameDesc;
}

void TotEngine::originalSaveLoadScreen() {
	uint oldMouseX, oldMouseY;
	int selectedSlot = 1;
	bool modified = false;
	Common::String saveName = "";

	bool exitSaveLoadMenu = false;
	oldMouseX = _mouse->mouseX;
	oldMouseY = _mouse->mouseY;
	_mouse->hide();

	uint menuBgSize = imagesize(20, 10, 300, 120);
	byte *menuBgPointer = (byte *)malloc(menuBgSize);
	_graphics->getImg(20, 10, 300, 120, menuBgPointer);

	for (int i = 0; i < 6; i++) {
		uint textY = i + 1;
		buttonBorder((120 - (textY * 10)), (80 - (textY * 10)), (200 + (textY * 10)), (60 + (textY * 10)), 251, 251, 251, 251, 0);
	}
	drawMenu(2); // 220 x 110	at 	xmenu = 50;ymenu = 10;
	// 50, 10, 270, 120
	// Arrows are 20, 18
	drawLeftArrow(20, 65);
	line(20, 65, 39, 65, 255);
	line(20, 65, 20, 80, 255);
	line(40, 66, 40, 82, 249);
	line(40, 82, 21, 82, 249);
	drawRightArrow(280, 65);
	line(280, 65, 299, 65, 255);
	line(280, 65, 280, 80, 255);
	line(300, 66, 300, 82, 249);
	line(300, 82, 292, 82, 249);

	if (!_saveAllowed) {
		bar(61, 15, 122, 23, 253);
		bar(201, 15, 259, 23, 253);
	}

	int pageNumber = 0;
	SaveStateList listSaves = getMetaEngine()->listSaves(_targetName.c_str());

	Common::HashMap<int, SaveStateDescriptor> saveMap;

	// Iterate and populate
	for (const SaveStateDescriptor &desc : listSaves) {
		int slot = desc.getSaveSlot();
		saveMap[slot] = desc;
	}

	saveName = drawAndSelectSaves(pageNumber, saveMap, selectedSlot);

	if (_cpCounter2 > 17)
		showError(274);
	_mouse->mouseX = 150;
	_mouse->mouseY = 60;
	_mouse->mouseMaskIndex = 1;
	_mouse->setMouseArea(Common::Rect(13, 13, 293, 105));
	_mouse->warpMouse(1, 150, 60);

	do {
		bool mouseClicked = false;
		bool keyPressed = false;
		Common::Event lastKeyboardEvent = Common::Event();
		do {
			_chrono->updateChrono();
			if (_chrono->_gameTick) {
				_mouse->animateMouseIfNeeded();
			}
			_events->pollEvent();
			if (_events->_leftMouseButton || _events->_rightMouseButton) {
				mouseClicked = true;
			} else if (_events->_keyPressed) {
				keyPressed = true;
				lastKeyboardEvent = _events->_lastKeyEvent;
			}

			_screen->update();
			g_system->delayMillis(10);
		} while (!keyPressed && !mouseClicked && !shouldQuit());

		if (mouseClicked) {
			if (_mouse->mouseX > 13 && _mouse->mouseX < 33 && _mouse->mouseY >= 58 && _mouse->mouseY <= 73) { // Left arrow

				if (pageNumber > 0)
					pageNumber--;
				else
					_sound->beep(100, 300);
				saveName = drawAndSelectSaves(pageNumber, saveMap, selectedSlot);
			} else if (_mouse->mouseX > 273 && _mouse->mouseX < 293 && _mouse->mouseY >= 58 && _mouse->mouseY <= 73) { // Right arrow

				if (pageNumber < kMaxSaveSlots / 6)
					pageNumber++;
				else
					_sound->beep(100, 300);
				saveName = drawAndSelectSaves(pageNumber, saveMap, selectedSlot);
			} else if (_mouse->mouseY >= 13 && _mouse->mouseY <= 16) {
				if (_mouse->mouseX >= 54 && _mouse->mouseX <= 124) { // Save
					if (selectedSlot > 0 && _saveAllowed && saveName != "") {
						saveGameState(selectedSlot + pageNumber * 6, saveName, false);
						_graphics->putImg(20, 10, menuBgPointer);
						exitSaveLoadMenu = true;
						selectedSlot = -1;
					} else {
						_sound->beep(100, 300);
					}
				} else if (_mouse->mouseX >= 130 && _mouse->mouseX <= 194) {
					if (selectedSlot >= 0 && !modified) { // Load
						if (selectedSlot < (int)listSaves.size()) {
							_mouse->hide();
							_graphics->putImg(20, 10, menuBgPointer);
							free(menuBgPointer);
							if (_saveAllowed) {
								clearAnimation();
								clearScreenLayers();
							}
							loadGameState(selectedSlot + pageNumber * 6);
							_mouse->mouseX = oldMouseX;
							_mouse->mouseY = oldMouseY;

							_mouse->show();
							_mouse->setMouseArea(Common::Rect(0, 0, 305, 185));
							exitSaveLoadMenu = true;
							selectedSlot = -1;

							return;
						} else {
							_sound->beep(100, 300);
						}
					} else {
						_sound->beep(100, 300);
						saveName = drawAndSelectSaves(pageNumber, saveMap, selectedSlot);
						_mouse->show();
					}
				} else if (_mouse->mouseClickX >= 200 && _mouse->mouseClickX <= 250) { // Exit
					if (_inGame && _saveAllowed) {
						_graphics->putImg(20, 10, menuBgPointer);
						exitSaveLoadMenu = true;
						selectedSlot = -1;
					} else {
						_sound->beep(100, 300);
					}
				}
			} else {
				if (_mouse->mouseClickY >= 24 && _mouse->mouseClickY <= 32) {
					if (0 + pageNumber * 6 >= kMaxSaveSlots) {
						_sound->beep(100, 300);
					} else {
						selectedSlot = 0;
						modified = false;
					}
				} else if (_mouse->mouseClickY >= 39 && _mouse->mouseClickY <= 47) {
					if (1 + pageNumber * 6 >= kMaxSaveSlots) {
						_sound->beep(100, 300);
					} else {
						selectedSlot = 1;
						modified = false;
					}
				} else if (_mouse->mouseClickY >= 54 && _mouse->mouseClickY <= 62) {
					if (2 + pageNumber * 6 >= kMaxSaveSlots) {
						_sound->beep(100, 300);
					} else {
						selectedSlot = 2;
						modified = false;
					}
				} else if (_mouse->mouseClickY >= 69 && _mouse->mouseClickY <= 77) {
					if (3 + pageNumber * 6 >= kMaxSaveSlots) {
						_sound->beep(100, 300);
					} else {
						selectedSlot = 3;
						modified = false;
					}
				} else if (_mouse->mouseClickY >= 84 && _mouse->mouseClickY <= 92) {
					if (4 + pageNumber * 6 >= kMaxSaveSlots) {
						_sound->beep(100, 300);
					} else {
						selectedSlot = 4;
						modified = false;
					}
				} else if (_mouse->mouseClickY >= 99 && _mouse->mouseClickY <= 107) {
					if (5 + pageNumber * 6 >= kMaxSaveSlots) {
						_sound->beep(100, 300);
					} else {
						selectedSlot = 5;
						modified = false;
					}
				}
				saveName = drawAndSelectSaves(pageNumber, saveMap, selectedSlot);
			}
		}

		if (keyPressed && _saveAllowed) {
			_mouse->hide();
			byte ytext = 29 + (selectedSlot * 15);
			readAlphaGraphSmall(saveName, 30, 65, ytext, 251, 254, lastKeyboardEvent);
			modified = true;
			_mouse->show();
			keyPressed = false;
		}
	} while (!exitSaveLoadMenu && !shouldQuit());
	_mouse->mouseX = oldMouseX;
	_mouse->mouseY = oldMouseY;
	_mouse->warpMouse(_mouse->mouseMaskIndex, _mouse->mouseX, _mouse->mouseY);
	free(menuBgPointer);
	_mouse->setMouseArea(Common::Rect(0, 0, 305, 185));
}

} // End of namespace Tot
