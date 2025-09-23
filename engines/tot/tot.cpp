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

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/scummsys.h"
#include "common/system.h"

#include "engines/util.h"

#include "tot/anims.h"
#include "tot/chrono.h"
#include "tot/console.h"
#include "tot/detection.h"
#include "tot/debug.h"
#include "tot/dialog.h"
#include "tot/events.h"
#include "tot/sound.h"
#include "tot/tot.h"
#include "tot/util.h"

namespace Tot {

TotEngine *g_engine;

TotEngine::TotEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																 _gameDescription(gameDesc), _randomSource("Tot") {
	g_engine = this;
	_lang = _gameDescription->language;
	_rooms = nullptr;
	_conversationData = nullptr;
	_sceneObjectsData = nullptr;
}

TotEngine::~TotEngine() {
	clearGame();
	delete _screen;
	delete _graphics;
	delete _sound;
	delete _chrono;
	delete _mouse;
	delete _events;
}

uint32 TotEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String TotEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error TotEngine::run() {
	// Initialize 320x200 paletted graphics mode
	initGraphics(320, 200);

	ConfMan.registerDefault("introSeen", false);
	//Static initializations

	_screen = new Graphics::Screen();
	_graphics = new GraphicsManager();
	_sound = new SoundManager(_mixer);
	_chrono = new ChronoManager();
	_mouse = new MouseManager();
	_events = new TotEventManager();

	_sound->init();
	syncSoundSettings();
	_graphics->init();
	initVars();

	_isIntroSeen = ConfMan.getBool("introSeen");
	// Set the engine's debugger console
	setDebugger(new TotConsole(this));

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);
	engineStart();

	return Common::kNoError;
}

void TotEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	_sound->syncSoundSettings();
}

int TotEngine::engineStart() {
	if (ConfMan.hasKey("save_slot")) {
		return startGame();
	}
	_graphics->clear();
	displayLoading();

	loadCharAnimation();
	loadInventory();
	// The track "SILENT" (a short fanfare) plays at 0 volume in the original code.
	// Not sure if this is intended or a bug... It is called "SILENT", but why play
	// it at all? It can be played at normal volume by uncommenting the playMidi line.
	//_sound->setMidiVolume(0, 0);
	//_sound->playMidi("SILENT", false);

	_mouse->setMouseArea(Common::Rect(0, 0, 305, 185));
	//_sound->playMidi("SILENT", true);

	_graphics->totalFadeOut(0);
	_graphics->clear();

	_graphics->loadPaletteFromFile("DEFAULT");
	initScreenPointers();
	initialLogo();
	_sound->playMidi("INTRODUC", true);
	//_sound->setMidiVolume(3, 3);
	firstIntroduction();
	_mouse->warpMouse(1, _mouse->mouseX, _mouse->mouseY);
	mainMenu(_firstTimeDone);
	if (_startNewGame && !shouldQuit()) {
		newGame();
	} else if (_continueGame && !shouldQuit()) {
		resumeGame();
	}

	return startGame();
}

void TotEngine::resumeGame() {
	loadGameState(getMetaEngine()->getAutosaveSlot());
}

void TotEngine::processEvents(bool &escapePressed) {
	_events->pollEvent();
	if (_events->_escKeyFl) {
		escapePressed = true;
	} else if (_events->_gameKey == KEY_VOLUME) {
		soundControls();
		g_engine->_events->zeroEvents();
	} else if (_events->_gameKey == KEY_SAVELOAD) {
		if (ConfMan.getBool("original_save_load_screen"))
			originalSaveLoadScreen();
		else
			openMainMenuDialog();

		g_engine->_events->zeroEvents();
	} else if (_events->_gameKey == KEY_OPEN) {
		_actionCode = 5;
		action();
		_oldGridX = 0;
		_oldGridY = 0;
	} else if (_events->_gameKey == KEY_CLOSE) {
		_actionCode = 6;
		action();
		_oldGridX = 0;
		_oldGridY = 0;
	} else if (_events->_gameKey == KEY_PICKUP) {
		_actionCode = 2;
		action();
		_oldGridX = 0;
		_oldGridY = 0;
	} else if (_events->_gameKey == KEY_TALK) {
		_actionCode = 1;
		action();
		_oldGridX = 0;
		_oldGridY = 0;
	} else if (_events->_gameKey == KEY_LOOKAT) {
		_actionCode = 3;
		action();
		_oldGridX = 0;
		_oldGridY = 0;
	} else if (_events->_gameKey == KEY_USE) {
		_actionCode = 4;
		action();
		_oldGridX = 0;
		_oldGridY = 0;
	} else if (_events->_gameKey == KEY_NONE && _events->_keyPressed) {
		_actionCode = 0; // go to
		action();
	}

	if (_events->_leftMouseButton == 1) {
		_mouse->mouseClickX = _events->_mouseX;
		_mouse->mouseClickY = _events->_mouseY;
		if (_mouse->mouseClickY > 0 && _mouse->mouseClickY < 131) {
			switch (_actionCode) {
			case 0: // go to
				_cpCounter2 = _cpCounter;
				// gets the zone where the character is now standing. Zone is calculated using xframe,yframe plus some adjustments to get the center of the feet
				_currentZone = _currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount];
				if (_currentZone < 10) {
					_xframe2 = _mouse->mouseClickX + 7;
					_yframe2 = _mouse->mouseClickY + 7;
					// obtains the target zone from the clicked coordinates
					_targetZone = _currentRoomData->walkAreasGrid[_xframe2 / kXGridCount][_yframe2 / kYGridCount];
					if (_currentRoomData->code == 21 && _currentRoomData->animationFlag) {
						if ((_targetZone >= 1 && _targetZone <= 5) ||
							(_targetZone >= 9 && _targetZone <= 13) ||
							(_targetZone >= 18 && _targetZone <= 21) ||
							_targetZone == 24 || _targetZone == 25) {

							_targetZone = 7;
							_mouse->mouseClickX = 232;
							_mouse->mouseClickY = 75;

							_xframe2 = _mouse->mouseClickX + 7;
							_yframe2 = _mouse->mouseClickY + 7;
						}
					}

					if (_oldTargetZone != _targetZone || _targetZone < 10) {
						_oldTargetZone = _targetZone;
						// Resets the entire route
						calculateRoute(_currentZone, _targetZone);

						_doorIndex = 0;
						_roomChange = false;

						for (_doorIndex = 0; _doorIndex < 5; _doorIndex++) {
							if (_currentRoomData->doors[_doorIndex].doorcode == _targetZone) {

								if (_currentRoomData->doors[_doorIndex].openclosed == 1) {
									_roomChange = true;
									break;
								} else if ((_currentRoomData->code == 5 && _targetZone == 27) || (_currentRoomData->code == 6 && _targetZone == 21)) {
									;
								} else {
									_trajectorySteps -= 1;
								}
							}
						}
						// Sets xframe2 again due to the substraction when closed doors
						_xframe2 = _trajectorySteps;
					} else
						_xframe2 = 0;
				}
				break;
			case 1: // talk
				_roomChange = false;
				_actionCode = 0;
				talkToSceneObject();
				_cpCounter2 = _cpCounter;
				break;
			case 2: // pick up
				_roomChange = false;
				_actionCode = 0;
				pickupScreenObject();
				_cpCounter = _cpCounter2;
				break;
			case 3: // look at
				_roomChange = false;
				_destinationX = _mouse->getClickCoordsWithinGrid().x;
				_destinationY = _mouse->getClickCoordsWithinGrid().y;
				if (_currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[_destinationX][_destinationY]]->fileIndex > 0) {
					goToObject(
						_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount],
						_currentRoomData->walkAreasGrid[_destinationX][_destinationY]);
					if (_currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[_destinationX][_destinationY]]->fileIndex == 562)

						switch (_currentRoomData->code) {
						case 20:
							if (_niche[0][_niche[0][3]] > 0)
								readObject(_niche[0][_niche[0][3]]);
							else
								readObject(562);
							break;
						case 24:
							if (_niche[1][_niche[1][3]] > 0)
								readObject(_niche[1][_niche[1][3]]);
							else
								readObject(562);
							break;
						}
					else
						readObject(_currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[_destinationX][_destinationY]]->fileIndex);
					if (_curObject.lookAtTextRef > 0)
						drawText(_curObject.lookAtTextRef);
					_actionCode = 0;
				}
				break;
			case 4: // use
				_roomChange = false;
				_actionCode = 0;
				useScreenObject();
				_cpCounter = _cpCounter2;
				break;
			case 5: // open
				_roomChange = false;
				_actionCode = 0;
				openScreenObject();
				break;
			case 6: { // close
				_roomChange = false;
				_actionCode = 0;
				closeScreenObject();
				_cpCounter = _cpCounter2;
			} break;
			}
		} else if (_mouse->mouseClickY > 148 && _mouse->mouseClickY < 158) {
			if (_mouse->mouseClickX >= 3 && _mouse->mouseClickX <= 53) {
				_actionCode = 1;
				action();
			} else if (_mouse->mouseClickX >= 58 && _mouse->mouseClickX <= 103) {
				_actionCode = 2;
				action();
			} else if (_mouse->mouseClickX >= 108 && _mouse->mouseClickX <= 153) {
				_actionCode = 3;
				action();
			} else if (_mouse->mouseClickX >= 158 && _mouse->mouseClickX <= 198) {
				_actionCode = 4;
				action();
			} else if (_mouse->mouseClickX >= 203 && _mouse->mouseClickX <= 248) {
				_actionCode = 5;
				action();
			} else if (_mouse->mouseClickX >= 253 && _mouse->mouseClickX <= 311) {
				_actionCode = 6;
				action();
			} else {
				_actionCode = 0;
				action();
				_cpCounter2 = _cpCounter;
			}
		} else if (_mouse->mouseClickY > 166 && _mouse->mouseClickY < 199) {
			if (_mouse->mouseClickX >= 3 && _mouse->mouseClickX <= 19) {
				drawInventory(0, 33);
			} else if (_mouse->mouseClickX >= 26 && _mouse->mouseClickX <= 65) {
				handleAction(_inventoryPosition);
			} else if (_mouse->mouseClickX >= 70 && _mouse->mouseClickX <= 108) {
				handleAction(_inventoryPosition + 1);
			} else if (_mouse->mouseClickX >= 113 && _mouse->mouseClickX <= 151) {
				handleAction(_inventoryPosition + 2);
			} else if (_mouse->mouseClickX >= 156 && _mouse->mouseClickX <= 194) {
				handleAction(_inventoryPosition + 3);
			} else if (_mouse->mouseClickX >= 199 && _mouse->mouseClickX <= 237) {
				handleAction(_inventoryPosition + 4);
			} else if (_mouse->mouseClickX >= 242 && _mouse->mouseClickX <= 280) {
				handleAction(_inventoryPosition + 5);
			} else if (_mouse->mouseClickX >= 290 && _mouse->mouseClickX <= 311) {
				drawInventory(1, 33);
			} else {
				_actionCode = 0;
				action();
			}
		}
	} else if (_events->_rightMouseButton) {
		_mouse->mouseClickX = _events->_mouseX;
		_mouse->mouseClickY = _events->_mouseY;
		Common::Point p = _mouse->getClickCoordsWithinGrid();
		_destinationX = p.x;
		_destinationY = p.y;
		_cpCounter2 = _cpCounter;
		if (_destinationY < 28) {
			RoomObjectListEntry obj = *_currentRoomData->screenObjectIndex[_currentRoomData->mouseGrid[_destinationX][_destinationY]];
			if (obj.fileIndex > 0) {

				drawLookAtItem(obj);
				goToObject(_currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount], _currentRoomData->walkAreasGrid[_destinationX][_destinationY]);
				if (obj.fileIndex == 562)

					switch (_currentRoomData->code) {
					case 20:
						if (_niche[0][_niche[0][3]] > 0)
							readObject(_niche[0][_niche[0][3]]);
						else
							readObject(562);
						break;
					case 24:
						if (_niche[1][_niche[1][3]] > 0)
							readObject(_niche[1][_niche[1][3]]);
						else
							readObject(562);
						break;
					}
				else
					readObject(obj.fileIndex);
				if (_curObject.lookAtTextRef > 0)
					drawText(_curObject.lookAtTextRef);
				_actionCode = 0;
			}
		}
	}
}

int TotEngine::startGame() {
	_sound->fadeOutMusic();
	switch (_gamePart) {
	case 1:
		_sound->playMidi("PRIMERA", true);
		break;
	case 2:
		_sound->playMidi("SEGUNDA", true);
		break;
	}
	_cpCounter2 = _cpCounter;
	_sound->fadeInMusic();
	_inGame = true;

	while (!_shouldQuitGame && !shouldQuit()) {
		bool escapePressed = false;
		_chrono->updateChrono();
		_mouse->animateMouseIfNeeded();

		processEvents(escapePressed);
		checkMouseGrid();
		advanceAnimations(false, true);

		// Scene changes
		if (_xframe2 == 0 && _roomChange) {
			changeRoom();
		}

		if (escapePressed && _xframe2 == 0) {
			clearAnimation();
			clearScreenLayers();
			_cpCounter2 = _cpCounter;
			_startNewGame = false;
			_continueGame = false;
			saveAutosaveIfEnabled();
			_graphics->totalFadeOut(0);
			_sound->fadeOutMusic();
			_graphics->clear();
			_sound->playMidi("INTRODUC", true);
			_sound->fadeInMusic();
			mainMenu(true);

			if (_startNewGame && !shouldQuit()) {
				newGame();
			} else if (_continueGame && !shouldQuit())
				resumeGame();
			else {
				_isSavingDisabled = true;
				openMainMenuDialog();
				_cpCounter = _cpCounter2;
				_isSavingDisabled = false;
			}
			_sound->fadeOutMusic();
			switch (_gamePart) {
			case 1:
				_sound->playMidi("PRIMERA", true);
				break;
			case 2:
				_sound->playMidi("SEGUNDA", true);
				break;
			}
			_sound->fadeInMusic();
		}

		switch (_gamePart) {
		case 1:
			if (_list1Complete && _list2Complete) {
				_list1Complete = false;
				_list2Complete = false;
				_cpCounter = _cpCounter2;
				_gamePart = 2;
				_iframe = 0;
				freeInventory();
				clearAnimation();
				clearScreenLayers();
				_mouse->hide();
				_graphics->partialFadeOut(234);
				_sound->fadeOutMusic();
				_sound->playMidi("CREDITOS", true);
				_sound->fadeInMusic();
				if (_cpCounter2 > 43)
					showError(274);
				sacrificeScene();
				_graphics->clear();
				loadInventory();
				_graphics->loadPaletteFromFile("SEGUNDA");
				_currentTrajectoryIndex = 0;
				_characterPosX = 160;
				_characterPosY = 60;
				_trajectory[_currentTrajectoryIndex].x = _characterPosX;
				_trajectory[_currentTrajectoryIndex].y = _characterPosY;
				loadScreenData(20);
				_sound->fadeOutMusic();
				_sound->playMidi("SEGUNDA", true);
				_sound->fadeInMusic();
				_graphics->sceneTransition(false, _sceneBackground, 1);
				drawInventoryMask();
				_inventoryPosition = 0;
				drawInventory();
				_mouse->show();
   				copyProtection();
				_firstTimeTopicA[8] = true;
				_oldGridX = 0;
				_oldGridY = 0;
				checkMouseGrid();
			}
			break;
		}

		// Debug graphics
		{
			// _graphics->euroText(Common::String::format("Room: %d", currentRoomNumber), 0, 0, 220, Graphics::kTextAlignLeft);
			// _mouseManager->printPos(xraton, yraton, 220, 0);
			// printPos(characterPosX, characterPosY, 220, 10, "CharPos");
			if (_showMouseGrid) {
				drawMouseGrid(_currentRoomData);
			}
			if (_showScreenGrid) {
				drawScreenGrid(_currentRoomData);
			}
			if (_showGameGrid) {
				drawGrid();
			}

			if (_drawObjectAreas) {
				for (int i = 0; i < kDepthLevelCount; i++) {
					if (_screenLayers[i] != nullptr) {
						if (true) {
							// debug
							uint16 w = READ_LE_UINT16(_screenLayers[i]);
							uint16 h = READ_LE_UINT16(_screenLayers[i] + 2);
							Common::Rect r = Common::Rect(_depthMap[i].posx, _depthMap[i].posy, _depthMap[i].posx + w, _depthMap[i].posy + h);
							drawRect(180, _depthMap[i].posx, _depthMap[i].posy, _depthMap[i].posx + w, _depthMap[i].posy + h);

							littText(r.left, r.top, Common::String().format("%d", i), 0);
						}
					}
				}
			}
		}

		_screen->update();
		g_system->delayMillis(10);
	}
	_mouse->hide();
	if (!shouldQuit()) {
		ending();
	}
	Common::String photoFileName;
	if (!shouldQuit()) {
		obtainName(photoFileName);
	}
	if (!shouldQuit()) {
		generateDiploma(photoFileName);
	}
	if (!shouldQuit()) {
		credits();
	}
	return EXIT_SUCCESS;
}

void TotEngine::newGame() {
	_saveAllowed = true;
	_mouse->hide();
	obtainName(_characterName);
	if (!shouldQuit()) {
		_graphics->totalFadeOut(0);
		_graphics->clear();
		displayLoading();
		freeInventory();
		resetGameState();
		loadInventory();
		_inGame = true;
		for (int i = 0; i < kInventoryIconCount; i++) {
			_inventory[i].bitmapIndex = 34;
			_inventory[i].code = 0;
			_inventory[i].objectName = getObjectName(10);
		}
		readConversationFile();
		initializeScreenFile();
		initializeObjectFile();
		_graphics->loadPaletteFromFile("DEFAULT");
		loadScreenData(1);
		_graphics->sceneTransition(false, _sceneBackground, 13);
		drawInventoryMask();
		_inventoryPosition = 0;
		drawInventory();
		_iframe = 0;
		_mouse->show();
	}
}

void TotEngine::changeRoom() {
	_roomChange = false;
	_cpCounter = _cpCounter2;
	setRoomTrajectories(_secondaryAnimHeight, _secondaryAnimWidth, RESTORE);
	saveRoomData(_currentRoomData, _rooms);
	_sound->setSfxVolume(_sound->_leftSfxVol, _sound->_rightSfxVol);

	if (_currentRoomData->doors[_doorIndex].nextScene != 255) {
		clearAnimation();
		clearScreenLayers();
	}
	switch (_currentRoomData->doors[_doorIndex].nextScene) {
	case 2: {
		_iframe = 0;
		_currentTrajectoryIndex = 0;
		_characterPosX = _currentRoomData->doors[_doorIndex].exitPosX - kCharacterCorrectionX;
		_characterPosY = _currentRoomData->doors[_doorIndex].exitPosY - kCharacerCorrectionY;
		_trajectory[_currentTrajectoryIndex].x = _characterPosX;
		_trajectory[_currentTrajectoryIndex].y = _characterPosY;
		_mouse->hide();

		_graphics->sceneTransition(true, nullptr);
		_sound->stopVoc();
		loadScreenData(_currentRoomData->doors[_doorIndex].nextScene);
		if (_cpCounter > 89)
			showError(274);
		_sound->setSfxVolume(_sound->_leftSfxVol, _sound->_rightSfxVol);
		if (_isTVOn)
			_sound->autoPlayVoc("PARASITO", 355778, 20129);
		else
			loadTV();
		_graphics->sceneTransition(false, _sceneBackground);
		_cpCounter = _cpCounter2;
		_mouse->show();
	} break;
	case 5: {
		if (_currentRoomData->code != 6) {
			_iframe = 0;
			_currentTrajectoryIndex = 0;
			_characterPosX = _currentRoomData->doors[_doorIndex].exitPosX - kCharacterCorrectionX;
			_characterPosY = _currentRoomData->doors[_doorIndex].exitPosY - kCharacerCorrectionY + 15;
			_trajectory[_currentTrajectoryIndex].x = _characterPosX;
			_trajectory[_currentTrajectoryIndex].y = _characterPosY;
			_mouse->hide();
			_graphics->sceneTransition(true, nullptr);
			loadScreenData(_currentRoomData->doors[_doorIndex].nextScene);
			_sound->stopVoc();
			_sound->autoPlayVoc("CALDERA", 6433, 15386);
			_sound->setSfxVolume(_sound->_leftSfxVol, 0);
			_graphics->sceneTransition(false, _sceneBackground);
			_mouse->show();
		} else {
			_currentZone = _currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount];
			_targetZone = 21;
			goToObject(_currentZone, _targetZone);
			_mouse->hide();
			_sound->setSfxVolume(_sound->_leftSfxVol, 0);
			loadScrollData(_currentRoomData->doors[_doorIndex].nextScene, true, 22, -2);
			_mouse->show();
		}
	} break;
	case 6: {
		_currentZone = _currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount];
		_targetZone = 27;
		goToObject(_currentZone, _targetZone);
		_mouse->hide();
		_sound->setSfxVolume(_sound->_leftSfxVol, _sound->_rightSfxVol);
		loadScrollData(_currentRoomData->doors[_doorIndex].nextScene, false, 22, 2);
		_mouse->show();
	} break;
	case 9: {
		_mouse->hide();
		_graphics->sceneTransition(true, nullptr);
		_iframe = 0;
		_currentTrajectoryIndex = 0;
		_characterPosX = _currentRoomData->doors[_doorIndex].exitPosX - kCharacterCorrectionX;
		_characterPosY = _currentRoomData->doors[_doorIndex].exitPosY - kCharacerCorrectionY;
		_trajectory[_currentTrajectoryIndex].x = _characterPosX;
		_trajectory[_currentTrajectoryIndex].y = _characterPosY;
		loadScreenData(_currentRoomData->doors[_doorIndex].nextScene);
		_graphics->sceneTransition(false, _sceneBackground);
		_mouse->show();
		if (getRandom(2) == 0) copyProtection();
	} break;
	case 12: {
		if (_currentRoomData->code != 13) {
			_iframe = 0;
			_currentTrajectoryIndex = 0;
			_characterPosX = _currentRoomData->doors[_doorIndex].exitPosX - kCharacterCorrectionX;
			_characterPosY = _currentRoomData->doors[_doorIndex].exitPosY - kCharacerCorrectionY;
			_trajectory[_currentTrajectoryIndex].x = _characterPosX;
			_trajectory[_currentTrajectoryIndex].y = _characterPosY;
			_mouse->hide();
			_graphics->sceneTransition(true, nullptr);
			loadScreenData(_currentRoomData->doors[_doorIndex].nextScene);
			_graphics->sceneTransition(false, _sceneBackground);
			_mouse->show();
		} else {
			_currentZone = _currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount];
			goToObject(_currentZone, _targetZone);
			_mouse->hide();
			loadScrollData(_currentRoomData->doors[_doorIndex].nextScene, false, 64, 0);
			_mouse->show();
		}
	} break;
	case 13: {
		switch (_currentRoomData->code) {
		case 12: {
			_currentZone = _currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount];
			goToObject(_currentZone, _targetZone);
			_mouse->hide();
			loadScrollData(_currentRoomData->doors[_doorIndex].nextScene, true, 64, 0);
			_mouse->show();
		} break;
		case 14: {
			_currentZone = _currentRoomData->walkAreasGrid[(_characterPosX + kCharacterCorrectionX) / kXGridCount][(_characterPosY + kCharacerCorrectionY) / kYGridCount];
			goToObject(_currentZone, _targetZone);
			_mouse->hide();
			loadScrollData(_currentRoomData->doors[_doorIndex].nextScene, false, 56, 0);
			_mouse->show();
		} break;
		}
	} break;
	case 14: {
		if (_currentRoomData->code != 13) {
			_iframe = 0;
			_currentTrajectoryIndex = 0;
			_characterPosX = _currentRoomData->doors[_doorIndex].exitPosX - kCharacterCorrectionX;
			_characterPosY = _currentRoomData->doors[_doorIndex].exitPosY - kCharacerCorrectionY;
			_trajectory[_currentTrajectoryIndex].x = _characterPosX;
			_trajectory[_currentTrajectoryIndex].y = _characterPosY;
			_mouse->hide();
			_graphics->sceneTransition(true, nullptr);
			loadScreenData(_currentRoomData->doors[_doorIndex].nextScene);
			_graphics->sceneTransition(false, _sceneBackground);
			_mouse->show();
		} else {
			_currentZone = _currentRoomData->walkAreasGrid[((_characterPosX + kCharacterCorrectionX) / kXGridCount)][((_characterPosY + kCharacerCorrectionY) / kYGridCount)];
			goToObject(_currentZone, _targetZone);
			_mouse->hide();
			loadScrollData(_currentRoomData->doors[_doorIndex].nextScene, true, 56, 0);
			_mouse->show();
		}
	} break;
	case 17: {
		_iframe = 0;
		_currentTrajectoryIndex = 0;
		_characterPosX = _currentRoomData->doors[_doorIndex].exitPosX - kCharacterCorrectionX;
		_characterPosY = _currentRoomData->doors[_doorIndex].exitPosY - kCharacerCorrectionY;
		_trajectory[_currentTrajectoryIndex].x = _characterPosX;
		_trajectory[_currentTrajectoryIndex].y = _characterPosY;
		_mouse->hide();
		_graphics->sceneTransition(true, nullptr);
		_sound->stopVoc();
		loadScreenData(_currentRoomData->doors[_doorIndex].nextScene);
		if (_bookTopic[0] == true && _currentRoomData->animationFlag == true)
			disableSecondAnimation();
		if (_cpCounter > 89)
			showError(274);
		_sound->setSfxVolume(_sound->_leftSfxVol, _sound->_rightSfxVol);
		_graphics->sceneTransition(false, _sceneBackground);
		_cpCounter = _cpCounter2;
		_mouse->show();
	} break;
	case 18: {
		if (_currentRoomData->code != 19) {
			_iframe = 0;
			_currentTrajectoryIndex = 0;
			_characterPosX = _currentRoomData->doors[_doorIndex].exitPosX - kCharacterCorrectionX;
			_characterPosY = _currentRoomData->doors[_doorIndex].exitPosY - kCharacerCorrectionY;
			_trajectory[_currentTrajectoryIndex].x = _characterPosX;
			_trajectory[_currentTrajectoryIndex].y = _characterPosY;
			_mouse->hide();
			_graphics->sceneTransition(true, nullptr);
			loadScreenData(_currentRoomData->doors[_doorIndex].nextScene);
			_graphics->sceneTransition(false, _sceneBackground);
			_mouse->show();
		} else {

			_currentZone = _currentRoomData->walkAreasGrid[((_characterPosX + kCharacterCorrectionX) / kXGridCount)][((_characterPosY + kCharacerCorrectionY) / kYGridCount)];
			goToObject(_currentZone, _targetZone);
			_mouse->hide();
			loadScrollData(_currentRoomData->doors[_doorIndex].nextScene, true, 131, -1);
			_mouse->show();
		}
	} break;
	case 19: {
		if (_currentRoomData->code != 18) {
			_iframe = 0;
			_currentTrajectoryIndex = 0;
			_characterPosX = _currentRoomData->doors[_doorIndex].exitPosX - kCharacterCorrectionX;
			_characterPosY = _currentRoomData->doors[_doorIndex].exitPosY - kCharacerCorrectionY;
			_trajectory[_currentTrajectoryIndex].x = _characterPosX;
			_trajectory[_currentTrajectoryIndex].y = _characterPosY;
			_mouse->hide();
			_graphics->sceneTransition(true, nullptr);
			loadScreenData(_currentRoomData->doors[_doorIndex].nextScene);
			_graphics->sceneTransition(false, _sceneBackground);
			_mouse->show();
		} else {
			_currentZone = _currentRoomData->walkAreasGrid[((_characterPosX + kCharacterCorrectionX) / kXGridCount)][((_characterPosY + kCharacerCorrectionY) / kYGridCount)];
			goToObject(_currentZone, _targetZone);
			_mouse->hide();
			loadScrollData(_currentRoomData->doors[_doorIndex].nextScene, false, 131, 1);
			_mouse->show();
		}
	} break;
	case 20: {
		_iframe = 0;
		_currentTrajectoryIndex = 0;
		_characterPosX = _currentRoomData->doors[_doorIndex].exitPosX - kCharacterCorrectionX;
		_characterPosY = _currentRoomData->doors[_doorIndex].exitPosY - kCharacerCorrectionY;
		_trajectory[_currentTrajectoryIndex].x = _characterPosX;
		_trajectory[_currentTrajectoryIndex].y = _characterPosY;
		_mouse->hide();
		_graphics->sceneTransition(true, nullptr);
		_sound->stopVoc();
		loadScreenData(_currentRoomData->doors[_doorIndex].nextScene);
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
		if (_cpCounter > 89)
			showError(274);
		_sound->setSfxVolume(_sound->_leftSfxVol, _sound->_rightSfxVol);
		if (_currentRoomData->code == 4)
			_sound->loadVoc("GOTA", 140972, 1029);
		_graphics->sceneTransition(false, _sceneBackground);
		_cpCounter = _cpCounter2;
		_mouse->show();
	} break;
	case 24: {
		_iframe = 0;
		_currentTrajectoryIndex = 0;
		_characterPosX = _currentRoomData->doors[_doorIndex].exitPosX - kCharacterCorrectionX;
		_characterPosY = _currentRoomData->doors[_doorIndex].exitPosY - kCharacerCorrectionY;
		_trajectory[_currentTrajectoryIndex].x = _characterPosX;
		_trajectory[_currentTrajectoryIndex].y = _characterPosY;
		_mouse->hide();
		_graphics->sceneTransition(true, nullptr);
		_sound->stopVoc();
		loadScreenData(_currentRoomData->doors[_doorIndex].nextScene);
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
		if (_cpCounter > 89)
			showError(274);
		_sound->setSfxVolume(_sound->_leftSfxVol, _sound->_rightSfxVol);
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
					if (_maskGridSecondaryAnim[i][j] > 0) {
						_currentRoomData->walkAreasGrid[_oldposx + i][_oldposy + j] = _maskGridSecondaryAnim[i][j];
					}
					if (_maskMouseSecondaryAnim[i][j] > 0)
						_currentRoomData->mouseGrid[_oldposx + i][_oldposy + j] = _maskMouseSecondaryAnim[i][j];
				}
			assembleScreen();
		}
		_graphics->sceneTransition(false, _sceneBackground);
		if ((_isRedDevilCaptured == false) && (_isTrapSet == false))
			runaroundRed();
		_cpCounter = _cpCounter2;
		_mouse->show();
	} break;
	case 255:
		wcScene();
		break;
	default: {
		_iframe = 0;
		_currentTrajectoryIndex = 0;
		_characterPosX = _currentRoomData->doors[_doorIndex].exitPosX - kCharacterCorrectionX;
		_characterPosY = _currentRoomData->doors[_doorIndex].exitPosY - kCharacerCorrectionY;
		_trajectory[_currentTrajectoryIndex].x = _characterPosX;
		_trajectory[_currentTrajectoryIndex].y = _characterPosY;

		_mouse->hide();
		_graphics->sceneTransition(true, nullptr);
		_sound->stopVoc();
		loadScreenData(_currentRoomData->doors[_doorIndex].nextScene);
		if (_cpCounter > 89)
			showError(274);
		_sound->setSfxVolume(_sound->_leftSfxVol, _sound->_rightSfxVol);
		switch (_currentRoomData->code) {
		case 4:
			_sound->loadVoc("GOTA", 140972, 1029);
			break;
		case 23:
			_sound->autoPlayVoc("FUENTE", 0, 0);
			break;
		}
		_graphics->sceneTransition(false, _sceneBackground);
		_cpCounter = _cpCounter2;
		_mouse->show();
	}
	}

	if (_currentRoomData->doors[_doorIndex].nextScene != 255) {
		_oldGridX = 0;
		_oldGridY = 0;
		checkMouseGrid();
	}
	_oldTargetZone = 0;
}

void TotEngine::clearCurrentInventoryObject() {

	_curObject.code = 0;
	_curObject.height = 0;
	_curObject.name = "";
	_curObject.lookAtTextRef = 0;
	_curObject.beforeUseTextRef = 0;
	_curObject.afterUseTextRef = 0;
	_curObject.pickTextRef = 0;
	_curObject.useTextRef = 0;
	_curObject.speaking = 0;
	_curObject.openable = false;
	_curObject.closeable = false;
	for (int i = 0; i <= 7; i++)
		_curObject.used[i] = 0;
	_curObject.pickupable = false;
	_curObject.useWith = 0;
	_curObject.replaceWith = 0;
	_curObject.depth = 0;
	_curObject.bitmapPointer = 0;
	_curObject.bitmapSize = 0;
	_curObject.rotatingObjectAnimation = 0;
	_curObject.rotatingObjectPalette = 0;
	_curObject.dropOverlayX = 0;
	_curObject.dropOverlayY = 0;
	_curObject.dropOverlay = 0;
	_curObject.dropOverlaySize = 0;
	_curObject.objectIconBitmap = 0;
	_curObject.xgrid1 = 0;
	_curObject.ygrid1 = 0;
	_curObject.xgrid2 = 0;
	_curObject.ygrid2 = 0;
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			_curObject.walkAreasPatch[i][j] = 0;
			_curObject.mouseGridPatch[i][j] = 0;
		}
	}
	_cpCounter2 = _cpCounter;
}

/**
 * Originally the Room file contains 8 copies of each room, one for every save plus the baseline (which is 0).
 * To put this into memory we need to get the baseline of each room and then put them continuously in a byte stream.addr
 * Whenever the game access a room instead of accessing the room for the autosave or the current save,
 * we assume only one room register is there.
 *
 * To save a game we merely copy the entire stream into the save.
 */
void TotEngine::initializeScreenFile() {

	Common::File roomFile;
	if (!roomFile.open(Common::Path("PANTALLA.DAT"))) {
		showError(320);
	}

	int64 fileSize = roomFile.size();
	delete (_rooms);

	byte *roomData = (byte *)malloc(kRoomRegSize * 32);
	int roomCount = 0;

	while (!roomFile.eos()) {
		if (fileSize - roomFile.pos() >= kRoomRegSize) {
			roomFile.read(roomData + kRoomRegSize * roomCount, kRoomRegSize);
			// This one doesnt work for some reason:
			// rooms->writeStream(roomFile.readStream(roomRegSize), roomRegSize);
			roomFile.skip(kRoomRegSize * 7);
			roomCount++;
		} else {
			break;
		}
	}
	_rooms = new Common::MemorySeekableReadWriteStream(roomData, kRoomRegSize * roomCount, DisposeAfterUse::NO);
	roomFile.close();
}

void TotEngine::resetGameState() {
	_characterPosX = 160;
	_characterPosY = 80;
	_iframe = 0;
	_trajectory[0].x = _characterPosX;
	_trajectory[0].y = _characterPosY;
	_xframe2 = 0;
	_yframe2 = 1;
	_currentZone = 1;
	_targetZone = 1;
	_oldTargetZone = 0;
	_charFacingDirection = 1;
	for (int i = 0; i < 9; i++) {
		_firstTimeTopicA[i] = true;
		_firstTimeTopicB[i] = false;
		_firstTimeTopicC[i] = false;
		_bookTopic[i] = false;
		_mintTopic[i] = false;
	}

	for (int i = 0; i < 5; i++) {
		_caves[i] = false;
	}

	_isSecondaryAnimationEnabled = false;
	_mainCharAnimation.depth = 0;

	_isDrawingEnabled = true;
	_isSavingDisabled = false;
	_startNewGame = false;
	_shouldQuitGame = false;
	_obtainedList1 = false;
	_obtainedList2 = false;

	_list1Complete = false;
	_list2Complete = false;

	_graphics->_paletteAnimFrame = 0;
	_gamePart = 1;

	_isVasePlaced = false;
	_isScytheTaken = false;
	_isTridentTaken = false;
	_isPottersWheelDelivered = false;
	_isMudDelivered = false;
	_isSealRemoved = false;

	_isGreenDevilDelivered = false;
	_isRedDevilCaptured = false;
	_isCupboardOpen = false;
	_isChestOpen = false;

	_isTVOn = false;
	_isTrapSet = false;
	_graphics->_palAnimStep = 0;

	_niche[0][0] = 563;
	_niche[0][1] = 561;
	_niche[0][2] = 0;
	_niche[0][3] = 2;

	_niche[1][0] = 615;
	_niche[1][1] = 622;
	_niche[1][2] = 623;
	_niche[1][3] = 0;

	_currentTrajectoryIndex = 0;
	_inventoryPosition = 0;
}

void TotEngine::initVars() {
	_isLoadingFromLauncher = false;
	_decryptionKey = "23313212133122121312132132312312122132322131221322222112121"
					"32121121212112111212112333131232323213222132123211213221231"
					"32132213232333333213132132132322113212132121322123121232332"
					"23123221322213233221112312231221233232122332211112233122321"
					"222312211322312223";


	for(int i = 0; i < kNumScreenOverlays; i++) {
		_screenLayers[i] = nullptr;
	}

	for(int i = 0; i < kInventoryIconCount; i++) {
		_inventoryIconBitmaps[i] = nullptr;
	}

	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < kWalkFrameCount + 30; j++) {
			_mainCharAnimation.bitmap[i][j] = nullptr;
		}
	}

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < kSecAnimationFrameCount; j++) {
			_secondaryAnimation.bitmap[i][j] = nullptr;
		}
	}

	_curSecondaryAnimationFrame = nullptr;
	_characterDirtyRect = nullptr;

	resetGameState();
	_chrono->_gameTick = false;
	for (int i = 0; i < kNumScreenOverlays; i++) {
		_screenLayers[i] = nullptr;
	}
	_firstList[0] = 222;
	_firstList[1] = 295;
	_firstList[2] = 402;
	_firstList[3] = 223;
	_firstList[4] = 521;

	_secondList[0] = 221;
	_secondList[1] = 423;
	_secondList[2] = 308;
	_secondList[3] = 362;
	_secondList[4] = 537;
	_cpCounter = 0;
	_cpCounter2 = 0;
	_continueGame = true;
	_firstTimeDone = false;
	_isIntroSeen = false;
	_inGame = false;

	_sceneBackground = nullptr;
	_backgroundCopy = nullptr;
	_conversationData = nullptr;
	_rooms = nullptr;
	_sceneObjectsData = nullptr;
}

void TotEngine::clearVars() {
	if (_sceneBackground != nullptr) {
		free(_sceneBackground);
	}

	if (_backgroundCopy != nullptr) {
		free(_backgroundCopy);
	}
	if (_conversationData != nullptr) {
		delete _conversationData;
	}
	if (_rooms != nullptr) {
		delete _rooms;
	}
	if (_sceneObjectsData != nullptr) {
		delete _sceneObjectsData;
	}

	if (_currentRoomData) {
		delete _currentRoomData;
	}

	clearScreenLayers();

	for (int i = 0; i < kInventoryIconCount; i++) {
		if (_inventoryIconBitmaps[i] != nullptr) {
			free(_inventoryIconBitmaps[i]);
		}
	}
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < kWalkFrameCount + 30; j++) {
			if (_mainCharAnimation.bitmap[i][j] != nullptr) {
				free(_mainCharAnimation.bitmap[i][j]);
			}
		}
	}

	if(_curSecondaryAnimationFrame != nullptr) {
		free(_curSecondaryAnimationFrame);
	}

	for (int i = 0; i < _secondaryAnimDirCount; i++) {
		for (int j = 0; j < _secondaryAnimationFrameCount; j++) {
			if (_secondaryAnimation.bitmap[i][j] != nullptr) {
				free(_secondaryAnimation.bitmap[i][j]);
			}
		}
	}
}

void TotEngine::mainMenu(bool fade) {
	bool bar = false;
	bool validOption = false;
	_sound->stopVoc();

	int32 offset = getOffsetsByCurrentLanguage()[1];
	_mouse->hide();
	if (fade)
		drawFlc(0, 0, offset, 0, 9, 0, true, false, false, bar);
	else
		drawFlc(0, 0, offset, 0, 9, 0, false, false, false, bar);

	if (_cpCounter2 > 10)
		showError(274);
	_mouse->mouseX = 160;
	_mouse->mouseY = 95;
	_mouse->mouseMaskIndex = 1;
	_mouse->warpMouse(_mouse->mouseMaskIndex, _mouse->mouseX, _mouse->mouseY);
	_mouse->show();
	do {
		_chrono->updateChrono();
		_mouse->animateMouseIfNeeded();
		_events->pollEvent();

		if (_events->_escKeyFl) {
			exitToDOS();
		}
		if (_events->_leftMouseButton) {
			uint x = _events->_mouseX + 7;
			uint y = _events->_mouseY + 7;
			if (y > 105 && y < 120) {
				if (x > 46 && x < 145) {
					_startNewGame = true;
					_continueGame = false;
					validOption = true;
				} else if (x > 173 && x < 267) {
					credits();
					if (!g_engine->shouldQuit()) {
						drawFlc(0, 0, offset, 0, 9, 0, true, false, false, bar);
					}
				}
			} else if (y > 140 && y < 155) {
				if (x > 173 && x < 292) {
					_graphics->totalFadeOut(0);
					_screen->clear();
					introduction();
					if (!g_engine->shouldQuit()) {
						drawFlc(0, 0, offset, 0, 9, 0, true, false, false, bar);
					}
				} else if (x >= 18 && x <= 145) {
					_isSavingDisabled = true;
					if (ConfMan.getBool("original_save_load_screen")) {
						originalSaveLoadScreen();
						validOption = true;
					} else {
						bool result = loadGameDialog();
						if (result) {
							validOption = true;
						}
					}
					_startNewGame = false;
					_continueGame = false;
					_isSavingDisabled = false;
				}
			} else if (y > 174 && y < 190) {
				if (x > 20 && x < 145) {
					_startNewGame = false;
					validOption = true;
					_continueGame = true;
				} else if (x > 173 && y < 288) {
					exitToDOS();
				}
			}
		}
		_screen->update();
		g_system->delayMillis(10);
	} while (!validOption && !shouldQuit());
}

void exitGame() {
	g_engine->_graphics->clear();
	g_system->quit();
}

void TotEngine::clearGame() {
	resetGameState();
	clearAnims();
	clearVars();
}

void TotEngine::exitToDOS() {
	uint oldMousePosX, oldMousePosY, dialogSize;
	byte oldMouseMask;
	char exitChar;

	oldMousePosX = _mouse->mouseX;
	oldMousePosY = _mouse->mouseY;
	oldMouseMask = _mouse->mouseMaskIndex;
	_mouse->hide();
	dialogSize = imagesize(58, 48, 262, 120);
	byte *dialogBackground = (byte *)malloc(dialogSize);
	_graphics->getImg(58, 48, 262, 120, dialogBackground);

	drawMenu(7);
	_mouse->mouseX = 160;
	_mouse->mouseY = 90;
	_mouse->mouseMaskIndex = 1;

	_mouse->setMouseArea(Common::Rect(115, 80, 190, 100));
	_mouse->warpMouse(_mouse->mouseMaskIndex, _mouse->mouseX, _mouse->mouseY);
	exitChar = '@';
	do {
		_chrono->updateChrono();
		_mouse->animateMouseIfNeeded();
		_events->pollEvent();
		if (_events->_escKeyFl) {
			exitChar = '\33';
		} else if (_events->_gameKey == KEY_YES) {
			debug("would exit game now");
			free(dialogBackground);
			exitGame();
		} else if (_events->_gameKey == KEY_NO) {
			exitChar = '\33';
		}

		if (_events->_leftMouseButton) {
			uint x = g_engine->_mouse->mouseClickX;
			if (x < 145) {
				free(dialogBackground);
				g_system->quit();
			} else if (x > 160) {
				exitChar = '\33';
			}
		}
		_screen->update();
	} while (exitChar != '\33' && !shouldQuit());
	debug("finished exitToDos");
	_graphics->putImg(58, 48, dialogBackground);
	_mouse->mouseX = oldMousePosX;
	_mouse->mouseY = oldMousePosY;
	_mouse->mouseMaskIndex = oldMouseMask;
	_mouse->show();
	free(dialogBackground);
	_mouse->setMouseArea(Common::Rect(0, 0, 305, 185));
}

} // End of namespace Tot
