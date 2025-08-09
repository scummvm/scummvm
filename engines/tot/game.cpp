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
#include "common/textconsole.h"

#include "tot/dialog.h"
#include "tot/graphics.h"
#include "tot/playanim.h"
#include "tot/routines.h"
#include "tot/routines2.h"
#include "tot/texts.h"
#include "tot/tot.h"
#include "tot/util.h"

namespace Tot {

void loadTemporaryGame() {
	g_engine->loadGameState(g_engine->getMetaEngine()->getAutosaveSlot());
}

int startGame();

void newGame() {
	saveAllowed = true;
	g_engine->_mouseManager->hide();
	obtainName(characterName);

	if (!g_engine->shouldQuit()) {
		totalFadeOut(0);
		g_engine->_graphics->clear();
		processingActive();
		freeInventory();
		loadObjects();
		resetGameState();
		inGame = true;
		for (int i = 0; i < inventoryIconCount; i++) {
			mobj[i].bitmapIndex = 34;
			mobj[i].code = 0;
			mobj[i].objectName = getObjectName(10);
		}
		readConversationFile(Common::String("CONVERSA.TRE"));
		initializeScreenFile();
		initializeObjectFile();
		loadPalette("DEFAULT");
		loadScreenData(1);
		screenTransition(13, false, sceneBackground);
		mask();
		inventoryPosition = 0;
		drawBackpack();
		iframe = 0;
		g_engine->_mouseManager->show();
	}
}

void sceneChange();

int engine_start() {
	if (ConfMan.hasKey("save_slot")) {
		return startGame();
	}
	g_engine->_graphics->clear();
	processingActive();

	loadCharAnimation();
	loadObjects();

	g_engine->_sound->setMidiVolume(0, 0);
	g_engine->_sound->playMidi("SILENT", false);

	g_engine->_mouseManager->setMouseArea(Common::Rect(0, 0, 305, 185));
	g_engine->_sound->playMidi("SILENT", true);

	totalFadeOut(0);
	g_engine->_graphics->clear();

	loadPalette("DEFAULT");
	loadScreenMemory();
	initialLogo();
	g_engine->_sound->playMidi("INTRODUC", true);
	g_engine->_sound->setMidiVolume(3, 3);
	firstIntroduction();
	g_engine->_mouseManager->setMousePos(1, mouseX, mouseY);
	initialMenu(firstTimeDone);
	if (startNewGame && !g_engine->shouldQuit()) {
		newGame();
	} else if (continueGame && !g_engine->shouldQuit()) {
		loadTemporaryGame();
	} else {
		isSavingDisabled = true;
		g_engine->openMainMenuDialog();
		isSavingDisabled = false;
	}

	return startGame();
}

int startGame() {
	g_engine->_sound->fadeOutMusic(musicVolLeft, musicVolRight);
	switch (gamePart) {
	case 1:
		g_engine->_sound->playMidi("PRIMERA", true);
		break;
	case 2:
		g_engine->_sound->playMidi("SEGUNDA", true);
		break;
	}
	contadorpc2 = contadorpc;
	g_engine->_sound->fadeInMusic(musicVolLeft, musicVolRight);
	inGame = true;

	Common::Event e;
	const char hotKeyOpen = hotKeyFor(OPEN);
	const char hotKeyClose = hotKeyFor(CLOSE);
	const char hotKeyPickup = hotKeyFor(PICKUP);
	const char hotKeyTalk = hotKeyFor(TALK);
	const char hotKeyLook = hotKeyFor(LOOKAT);
	const char hotKeyUse = hotKeyFor(USE);

	while (!shouldQuitGame && !g_engine->shouldQuit()) {
		bool escapePressed = false;
		g_engine->_chrono->updateChrono();
		g_engine->_mouseManager->animateMouseIfNeeded();
		// debug
		while (g_system->getEventManager()->pollEvent(e)) {
			if (isMouseEvent(e)) {
				g_engine->_mouseManager->setMousePos(e.mouse);
				mouseX = e.mouse.x;
				mouseY = e.mouse.y;
			}
			if (e.type == Common::EVENT_KEYUP) {
				changeGameSpeed(e);

				switch (e.kbd.keycode) {

				case Common::KEYCODE_ESCAPE:
					escapePressed = true;
					break;
				case Common::KEYCODE_F1:
					soundControls();
					break;
				case Common::KEYCODE_F2:
					g_engine->openMainMenuDialog();
					// saveLoad();
					break;
				default:
					if (e.kbd.keycode == hotKeyOpen) {
						actionCode = 5;
						action();
						oldGridX = 0;
						oldGridY = 0;
					} else if (e.kbd.keycode == hotKeyClose) {
						actionCode = 6;
						action();
						oldGridX = 0;
						oldGridY = 0;
					} else if (e.kbd.keycode == hotKeyPickup) {
						actionCode = 2;
						action();
						oldGridX = 0;
						oldGridY = 0;
					} else if (e.kbd.keycode == hotKeyTalk) {
						actionCode = 1;
						action();
						oldGridX = 0;
						oldGridY = 0;
					} else if (e.kbd.keycode == hotKeyLook) {
						actionCode = 3;
						action();
						oldGridX = 0;
						oldGridY = 0;
					} else if (e.kbd.keycode == hotKeyUse) {
						actionCode = 4;
						action();
						oldGridX = 0;
						oldGridY = 0;
					} else {
						actionCode = 0; // go to
					}
				}
			} else if (e.type == Common::EVENT_LBUTTONUP) {
				mouseClickX = e.mouse.x;
				mouseClickY = e.mouse.y;
				if (mouseClickY > 0 && mouseClickY < 131) {
					switch (actionCode) {
					case 0: // go to
						contadorpc2 = contadorpc;
						// gets the zone where the character is now standing. Zone is calculated using xframe,yframe plus some adjustments to get the center of the feet
						currentZone = currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount];
						if (currentZone < 10) {
							xframe2 = mouseClickX + 7;
							yframe2 = mouseClickY + 7;
							// obtains the target zone from the clicked coordinates
							targetZone = currentRoomData->walkAreasGrid[xframe2 / xGridCount][yframe2 / yGridCount];
							if (currentRoomData->code == 21 && currentRoomData->animationFlag) {
								if ((targetZone >= 1 && targetZone <= 5) ||
									(targetZone >= 9 && targetZone <= 13) ||
									(targetZone >= 18 && targetZone <= 21) ||
									targetZone == 24 || targetZone == 25) {

									targetZone = 7;
									mouseClickX = 232;
									mouseClickY = 75;

									xframe2 = mouseClickX + 7;
									yframe2 = mouseClickY + 7;
								}
							}

							if (oldTargetZone != targetZone || targetZone < 10) {
								oldTargetZone = targetZone;
								// Resets the entire route
								calculateRoute(currentZone, targetZone);

								doorIndex = 0;
								roomChange = false;

								for (doorIndex = 0; doorIndex < 5; doorIndex++) {
									if (currentRoomData->doors[doorIndex].doorcode == targetZone) {

										if (currentRoomData->doors[doorIndex].openclosed == 1) {
											roomChange = true;
											break;
										} else if ((currentRoomData->code == 5 && targetZone == 27) || (currentRoomData->code == 6 && targetZone == 21)) {
											;
										} else {
											steps -= 1;
										}
									}
								}
								// Sets xframe2 again due to the substraction when closed doors
								xframe2 = steps;
							} else
								xframe2 = 0;
						}
						break;
					case 1: // talk
						roomChange = false;
						actionCode = 0;
						talkScreenObject();
						contadorpc2 = contadorpc;
						break;
					case 2: // pick up
						roomChange = false;
						actionCode = 0;
						pickupScreenObject();
						contadorpc = contadorpc2;
						break;
					case 3: // look at
						roomChange = false;
						destinationStepX = (mouseClickX + 7) / xGridCount;
						destinationStepY = (mouseClickY + 7) / yGridCount;
						if (currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[destinationStepX][destinationStepY]]->fileIndex > 0) {
							goToObject(
								currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount],
								currentRoomData->walkAreasGrid[destinationStepX][destinationStepY]);
							if (currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[destinationStepX][destinationStepY]]->fileIndex == 562)

								switch (currentRoomData->code) {
								case 20:
									if (niche[0][niche[0][3]] > 0)
										readItemRegister(niche[0][niche[0][3]]);
									else
										readItemRegister(562);
									break;
								case 24:
									if (niche[1][niche[1][3]] > 0)
										readItemRegister(niche[1][niche[1][3]]);
									else
										readItemRegister(562);
									break;
								}
							else
								readItemRegister(currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[destinationStepX][destinationStepY]]->fileIndex);
							if (regobj.lookAtTextRef > 0)
								drawText(regobj.lookAtTextRef);
							actionCode = 0;
						}
						break;
					case 4: // use
						roomChange = false;
						actionCode = 0;
						useScreenObject();
						contadorpc = contadorpc2;
						break;
					case 5: // open
						roomChange = false;
						actionCode = 0;
						openScreenObject();
						break;
					case 6: { // close
						roomChange = false;
						actionCode = 0;
						closeScreenObject();
						contadorpc = contadorpc2;
					} break;
					}
				} else if (mouseClickY > 148 && mouseClickY < 158) {
					if (mouseClickX >= 3 && mouseClickX <= 53) {
						actionCode = 1;
						action();
						break;
					} else if (mouseClickX >= 58 && mouseClickX <= 103) {
						actionCode = 2;
						action();
						break;
					} else if (mouseClickX >= 108 && mouseClickX <= 153) {
						actionCode = 3;
						action();
						break;
					} else if (mouseClickX >= 158 && mouseClickX <= 198) {
						actionCode = 4;
						action();
						break;
					} else if (mouseClickX >= 203 && mouseClickX <= 248) {
						actionCode = 5;
						action();
						break;
					} else if (mouseClickX >= 253 && mouseClickX <= 311) {
						actionCode = 6;
						action();
						break;
					} else {
						actionCode = 0;
						action();
						contadorpc2 = contadorpc;
					}
				} else if (mouseClickY > 166 && mouseClickY < 199) {
					if (mouseClickX >= 3 && mouseClickX <= 19) {
						inventory(0, 33);
						break;
					} else if (mouseClickX >= 26 && mouseClickX <= 65) {
						handleAction(inventoryPosition);
						break;
					} else if (mouseClickX >= 70 && mouseClickX <= 108) {
						handleAction(inventoryPosition + 1);
						break;
					} else if (mouseClickX >= 113 && mouseClickX <= 151) {
						handleAction(inventoryPosition + 2);
						break;
					} else if (mouseClickX >= 156 && mouseClickX <= 194) {
						handleAction(inventoryPosition + 3);
						break;
					} else if (mouseClickX >= 199 && mouseClickX <= 237) {
						handleAction(inventoryPosition + 4);
						break;
					} else if (mouseClickX >= 242 && mouseClickX <= 280) {
						handleAction(inventoryPosition + 5);
						break;
					} else if (mouseClickX >= 290 && mouseClickX <= 311) {
						inventory(1, 33);
						break;
					} else {
						actionCode = 0;
						action();
					}
				}
			} else if (e.type == Common::EVENT_RBUTTONUP) {
				mouseClickX = e.mouse.x;
				mouseClickY = e.mouse.y;
				destinationStepX = (mouseClickX + 7) / xGridCount;
				destinationStepY = (mouseClickY + 7) / yGridCount;
				contadorpc2 = contadorpc;
				if (destinationStepY < 28) {
					RoomObjectListEntry obj = *currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[destinationStepX][destinationStepY]];
					if (obj.fileIndex > 0) {

						drawLookAtItem(obj);
						goToObject(currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount], currentRoomData->walkAreasGrid[destinationStepX][destinationStepY]);
						if (obj.fileIndex == 562)

							switch (currentRoomData->code) {
							case 20:
								if (niche[0][niche[0][3]] > 0)
									readItemRegister(niche[0][niche[0][3]]);
								else
									readItemRegister(562);
								break;
							case 24:
								if (niche[1][niche[1][3]] > 0)
									readItemRegister(niche[1][niche[1][3]]);
								else
									readItemRegister(562);
								break;
							}
						else
							readItemRegister(obj.fileIndex);
						if (regobj.lookAtTextRef > 0)
							drawText(regobj.lookAtTextRef);
						actionCode = 0;
					}
				}
			}
		}

		checkMouseGrid();
		advanceAnimations(false, true);

		// Scene changes
		if (xframe2 == 0 && roomChange) {
			sceneChange();
		}

		if (escapePressed && xframe2 == 0) {
			freeAnimation();
			freeScreenObjects();
			contadorpc2 = contadorpc;
			startNewGame = false;
			continueGame = false;
			g_engine->saveAutosaveIfEnabled();
			totalFadeOut(0);
			g_engine->_sound->fadeOutMusic(musicVolLeft, musicVolRight);
			g_engine->_graphics->clear();
			g_engine->_sound->playMidi("INTRODUC", true);
			g_engine->_sound->fadeInMusic(musicVolLeft, musicVolRight);
			initialMenu(true);
			verifyCopyProtection2();

			if (startNewGame && !g_engine->shouldQuit()) {
				newGame();
			} else if (continueGame && !g_engine->shouldQuit())
				loadTemporaryGame();
			else {
				isSavingDisabled = true;
				g_engine->openMainMenuDialog();
				contadorpc = contadorpc2;
				isSavingDisabled = false;
			}
			g_engine->_sound->fadeOutMusic(musicVolLeft, musicVolRight);
			switch (gamePart) {
			case 1:
				g_engine->_sound->playMidi("PRIMERA", true);
				break;
			case 2:
				g_engine->_sound->playMidi("SEGUNDA", true);
				break;
			}
			g_engine->_sound->fadeInMusic(musicVolLeft, musicVolRight);
		}

		switch (gamePart) {
		case 1:
			if (list1Complete && list2Complete) {
				list1Complete = false;
				list2Complete = false;
				contadorpc = contadorpc2;
				gamePart = 2;
				iframe = 0;
				freeInventory();
				freeAnimation();
				freeScreenObjects();
				g_engine->_mouseManager->hide();
				partialFadeOut(234);
				g_engine->_sound->fadeOutMusic(musicVolLeft, musicVolRight);
				g_engine->_sound->playMidi("CREDITOS", true);
				g_engine->_sound->fadeInMusic(musicVolLeft, musicVolRight);
				if (contadorpc2 > 43)
					showError(274);
				sacrificeScene();
				g_engine->_graphics->clear();
				loadObjects();
				loadPalette("SEGUNDA");
				currentTrajectoryIndex = 0;
				characterPosX = 160;
				characterPosY = 60;
				trajectory[currentTrajectoryIndex].x = characterPosX;
				trajectory[currentTrajectoryIndex].y = characterPosY;
				loadScreenData(20);
				g_engine->_sound->fadeOutMusic(musicVolLeft, musicVolRight);
				g_engine->_sound->playMidi("SEGUNDA", true);
				g_engine->_sound->fadeInMusic(musicVolLeft, musicVolRight);
				screenTransition(1, false, sceneBackground);
				mask();
				inventoryPosition = 0;
				drawBackpack();
				g_engine->_mouseManager->show();

				firstTimeTopicA[8] = true;
				oldGridX = 0;
				oldGridY = 0;
				checkMouseGrid();
			}
			break;
		}

		// Debug graphics
		{
			// g_engine->_graphics->euroText(Common::String::format("Room: %d", currentRoomNumber), 0, 0, 220, Graphics::kTextAlignLeft);
			// g_engine->_mouseManager->printPos(xraton, yraton, 220, 0);
			// printPos(characterPosX, characterPosY, 220, 10, "CharPos");
			if (g_engine->_showMouseGrid) {
				drawMouseGrid(currentRoomData);
			}
			if (g_engine->_showScreenGrid) {
				drawScreenGrid(currentRoomData);
			}
			if (g_engine->_showGameGrid) {
				drawGrid();
			}

			if (g_engine->_drawObjectAreas) {
				for (int i = 0; i < depthLevelCount; i++) {
					if (screenObjects[i] != NULL) {
						if (true) {
							// debug
							uint16 w = READ_LE_UINT16(screenObjects[i]);
							uint16 h = READ_LE_UINT16(screenObjects[i] + 2);
							Common::Rect r = Common::Rect(depthMap[i].posx, depthMap[i].posy, depthMap[i].posx + w, depthMap[i].posy + h);
							drawRect(180, depthMap[i].posx, depthMap[i].posy, depthMap[i].posx + w, depthMap[i].posy + h);

							littText(r.left, r.top, Common::String().format("%d", i), 0);
						}
					}
				}
			}
			// g_engine->_screen->markAllDirty();
			// g_engine->_screen->update();
		}

		g_engine->_screen->update();
		g_system->delayMillis(10);
	}
	g_engine->_mouseManager->hide();
	if (!g_engine->shouldQuit()) {
		ending();
	}
	if (!g_engine->shouldQuit()) {
		obtainName(photoFileName);
	}
	if (!g_engine->shouldQuit()) {
		generateDiploma(photoFileName);
	}
	if (!g_engine->shouldQuit()) {
		credits();
	}
	return EXIT_SUCCESS;
}

void sceneChange() {
	roomChange = false;
	contadorpc = contadorpc2;
	setRoomTrajectories(secondaryAnimHeight, secondaryAnimWidth, RESTORE);
	saveRoomData(currentRoomData, rooms);
	// verifyCopyProtection();
	g_engine->_sound->setSfxVolume(leftSfxVol, rightSfxVol);

	switch (currentRoomData->doors[doorIndex].nextScene) {
	case 2: {
		transitionEffect = Random(15) + 1;
		iframe = 0;
		currentTrajectoryIndex = 0;
		characterPosX = currentRoomData->doors[doorIndex].exitPosX - characterCorrectionX;
		characterPosY = currentRoomData->doors[doorIndex].exitPosY - characerCorrectionY;
		trajectory[currentTrajectoryIndex].x = characterPosX;
		trajectory[currentTrajectoryIndex].y = characterPosY;
		freeAnimation();
		freeScreenObjects();
		g_engine->_mouseManager->hide();

		screenTransition(transitionEffect, true, NULL);
		g_engine->_sound->stopVoc();
		loadScreenData(currentRoomData->doors[doorIndex].nextScene);
		if (contadorpc > 89)
			showError(274);
		g_engine->_sound->setSfxVolume(leftSfxVol, rightSfxVol);
		if (isTVOn)
			g_engine->_sound->autoPlayVoc("PARASITO", 355778, 20129);
		else
			cargatele();
		screenTransition(transitionEffect, false, sceneBackground);
		contadorpc = contadorpc2;
		g_engine->_mouseManager->show();
		oldGridX = 0;
		oldGridY = 0;
	} break;
	case 5: {
		if (currentRoomData->code != 6) {
			transitionEffect = Random(15) + 1;
			iframe = 0;
			currentTrajectoryIndex = 0;
			characterPosX = currentRoomData->doors[doorIndex].exitPosX - characterCorrectionX;
			characterPosY = currentRoomData->doors[doorIndex].exitPosY - characerCorrectionY + 15;
			trajectory[currentTrajectoryIndex].x = characterPosX;
			trajectory[currentTrajectoryIndex].y = characterPosY;
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			screenTransition(transitionEffect, true, NULL);
			loadScreenData(currentRoomData->doors[doorIndex].nextScene);
			g_engine->_sound->stopVoc();
			g_engine->_sound->autoPlayVoc("CALDERA", 6433, 15386);
			g_engine->_sound->setSfxVolume(leftSfxVol, 0);
			screenTransition(transitionEffect, false, sceneBackground);
			g_engine->_mouseManager->show();
			oldGridX = 0;
			oldGridY = 0;
			checkMouseGrid();
		} else {

			currentZone = currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount];
			targetZone = 21;
			goToObject(currentZone, targetZone);
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			g_engine->_sound->setSfxVolume(leftSfxVol, 0);
			loadScrollData(currentRoomData->doors[doorIndex].nextScene, true, 22, -2);
			g_engine->_mouseManager->show();
			oldGridX = 0;
			oldGridY = 0;
			checkMouseGrid();
		}
	} break;
	case 6: {
		currentZone = currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount];
		targetZone = 27;
		goToObject(currentZone, targetZone);
		freeAnimation();
		freeScreenObjects();
		g_engine->_mouseManager->hide();
		g_engine->_sound->setSfxVolume(leftSfxVol, rightSfxVol);
		loadScrollData(currentRoomData->doors[doorIndex].nextScene, false, 22, 2);
		g_engine->_mouseManager->show();
		oldGridX = 0;
		oldGridY = 0;
		checkMouseGrid();
	} break;
	case 9: {
		transitionEffect = Random(15) + 1;
		freeAnimation();
		freeScreenObjects();
		g_engine->_mouseManager->hide();
		screenTransition(transitionEffect, true, NULL);
		iframe = 0;
		currentTrajectoryIndex = 0;
		characterPosX = currentRoomData->doors[doorIndex].exitPosX - characterCorrectionX;
		characterPosY = currentRoomData->doors[doorIndex].exitPosY - characerCorrectionY;
		trajectory[currentTrajectoryIndex].x = characterPosX;
		trajectory[currentTrajectoryIndex].y = characterPosY;
		loadScreenData(currentRoomData->doors[doorIndex].nextScene);
		screenTransition(transitionEffect, false, sceneBackground);
		g_engine->_mouseManager->show();

		oldGridX = 0;
		oldGridY = 0;
		checkMouseGrid();
	} break;
	case 12: {
		if (currentRoomData->code != 13) {
			transitionEffect = Random(15) + 1;
			iframe = 0;
			currentTrajectoryIndex = 0;
			characterPosX = currentRoomData->doors[doorIndex].exitPosX - characterCorrectionX;
			characterPosY = currentRoomData->doors[doorIndex].exitPosY - characerCorrectionY;
			trajectory[currentTrajectoryIndex].x = characterPosX;
			trajectory[currentTrajectoryIndex].y = characterPosY;
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			screenTransition(transitionEffect, true, NULL);
			loadScreenData(currentRoomData->doors[doorIndex].nextScene);
			screenTransition(transitionEffect, false, sceneBackground);
			g_engine->_mouseManager->show();
			oldGridX = 0;
			oldGridY = 0;
			g_engine->_mouseManager->show();
		} else {

			currentZone = currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount];
			goToObject(currentZone, targetZone);
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			loadScrollData(currentRoomData->doors[doorIndex].nextScene, false, 64, 0);
			g_engine->_mouseManager->show();
			oldGridX = 0;
			oldGridY = 0;
			checkMouseGrid();
		}
	} break;
	case 13: {
		switch (currentRoomData->code) {
		case 12: {
			currentZone = currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount];
			goToObject(currentZone, targetZone);
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			loadScrollData(currentRoomData->doors[doorIndex].nextScene, true, 64, 0);
			g_engine->_mouseManager->show();
			oldGridX = 0;
			oldGridY = 0;
			checkMouseGrid();
		} break;
		case 14: {
			currentZone = currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount];
			goToObject(currentZone, targetZone);
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			loadScrollData(currentRoomData->doors[doorIndex].nextScene, false, 56, 0);
			g_engine->_mouseManager->show();
			oldGridX = 0;
			oldGridY = 0;
			checkMouseGrid();
		} break;
		}
	} break;
	case 14: {
		if (currentRoomData->code != 13) {
			transitionEffect = Random(15) + 1;
			iframe = 0;
			currentTrajectoryIndex = 0;
			characterPosX = currentRoomData->doors[doorIndex].exitPosX - characterCorrectionX;
			characterPosY = currentRoomData->doors[doorIndex].exitPosY - characerCorrectionY;
			trajectory[currentTrajectoryIndex].x = characterPosX;
			trajectory[currentTrajectoryIndex].y = characterPosY;
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			screenTransition(transitionEffect, true, NULL);
			loadScreenData(currentRoomData->doors[doorIndex].nextScene);
			screenTransition(transitionEffect, false, sceneBackground);
			g_engine->_mouseManager->show();
			oldGridX = 0;
			oldGridY = 0;
			checkMouseGrid();
		} else {

			currentZone = currentRoomData->walkAreasGrid[((characterPosX + characterCorrectionX) / xGridCount)][((characterPosY + characerCorrectionY) / yGridCount)];
			goToObject(currentZone, targetZone);
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			loadScrollData(currentRoomData->doors[doorIndex].nextScene, true, 56, 0);
			g_engine->_mouseManager->show();
			oldGridX = 0;
			oldGridY = 0;
			checkMouseGrid();
		}
	} break;
	case 17: {
		transitionEffect = Random(15) + 1;
		iframe = 0;
		currentTrajectoryIndex = 0;
		characterPosX = currentRoomData->doors[doorIndex].exitPosX - characterCorrectionX;
		characterPosY = currentRoomData->doors[doorIndex].exitPosY - characerCorrectionY;
		trajectory[currentTrajectoryIndex].x = characterPosX;
		trajectory[currentTrajectoryIndex].y = characterPosY;
		freeAnimation();
		freeScreenObjects();
		g_engine->_mouseManager->hide();
		screenTransition(transitionEffect, true, NULL);
		g_engine->_sound->stopVoc();
		loadScreenData(currentRoomData->doors[doorIndex].nextScene);
		if (bookTopic[0] == true && currentRoomData->animationFlag == true)
			disableSecondAnimation();
		if (contadorpc > 89)
			showError(274);
		g_engine->_sound->setSfxVolume(leftSfxVol, rightSfxVol);
		screenTransition(transitionEffect, false, sceneBackground);
		contadorpc = contadorpc2;
		g_engine->_mouseManager->show();
		oldGridX = 0;
		oldGridY = 0;
		checkMouseGrid();
	} break;
	case 18: {
		if (currentRoomData->code != 19) {
			transitionEffect = Random(15) + 1;
			iframe = 0;
			currentTrajectoryIndex = 0;
			characterPosX = currentRoomData->doors[doorIndex].exitPosX - characterCorrectionX;
			characterPosY = currentRoomData->doors[doorIndex].exitPosY - characerCorrectionY;
			trajectory[currentTrajectoryIndex].x = characterPosX;
			trajectory[currentTrajectoryIndex].y = characterPosY;
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			screenTransition(transitionEffect, true, NULL);
			loadScreenData(currentRoomData->doors[doorIndex].nextScene);
			screenTransition(transitionEffect, false, sceneBackground);
			g_engine->_mouseManager->show();
			oldGridX = 0;
			oldGridY = 0;
			checkMouseGrid();
		} else {

			currentZone = currentRoomData->walkAreasGrid[((characterPosX + characterCorrectionX) / xGridCount)][((characterPosY + characerCorrectionY) / yGridCount)];
			goToObject(currentZone, targetZone);
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			loadScrollData(currentRoomData->doors[doorIndex].nextScene, true, 131, -1);
			g_engine->_mouseManager->show();
			oldGridX = 0;
			oldGridY = 0;
			checkMouseGrid();
		}
	} break;
	case 19: {
		if (currentRoomData->code != 18) {
			transitionEffect = Random(15) + 1;
			iframe = 0;
			currentTrajectoryIndex = 0;
			characterPosX = currentRoomData->doors[doorIndex].exitPosX - characterCorrectionX;
			characterPosY = currentRoomData->doors[doorIndex].exitPosY - characerCorrectionY;
			trajectory[currentTrajectoryIndex].x = characterPosX;
			trajectory[currentTrajectoryIndex].y = characterPosY;
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			screenTransition(transitionEffect, true, NULL);
			loadScreenData(currentRoomData->doors[doorIndex].nextScene);
			screenTransition(transitionEffect, false, sceneBackground);
			g_engine->_mouseManager->show();
			oldGridX = 0;
			oldGridY = 0;
			checkMouseGrid();
		} else {

			currentZone = currentRoomData->walkAreasGrid[((characterPosX + characterCorrectionX) / xGridCount)][((characterPosY + characerCorrectionY) / yGridCount)];
			goToObject(currentZone, targetZone);
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			loadScrollData(currentRoomData->doors[doorIndex].nextScene, false, 131, 1);
			g_engine->_mouseManager->show();
			oldGridX = 0;
			oldGridY = 0;
			checkMouseGrid();
		}
	} break;
	case 20: {
		transitionEffect = Random(15) + 1;
		iframe = 0;
		currentTrajectoryIndex = 0;
		characterPosX = currentRoomData->doors[doorIndex].exitPosX - characterCorrectionX;
		characterPosY = currentRoomData->doors[doorIndex].exitPosY - characerCorrectionY;
		trajectory[currentTrajectoryIndex].x = characterPosX;
		trajectory[currentTrajectoryIndex].y = characterPosY;
		freeAnimation();
		freeScreenObjects();
		g_engine->_mouseManager->hide();
		screenTransition(transitionEffect, true, NULL);
		g_engine->_sound->stopVoc();
		loadScreenData(currentRoomData->doors[doorIndex].nextScene);
		switch (niche[0][niche[0][3]]) {
		case 0:
			currentRoomData->screenObjectIndex[9]->objectName = getObjectName(4);
			break;
		case 561:
			currentRoomData->screenObjectIndex[9]->objectName = getObjectName(5);
			break;
		case 563:
			currentRoomData->screenObjectIndex[9]->objectName = getObjectName(6);
			break;
		case 615:
			currentRoomData->screenObjectIndex[9]->objectName = getObjectName(7);
			break;
		}
		if (contadorpc > 89)
			showError(274);
		g_engine->_sound->setSfxVolume(leftSfxVol, rightSfxVol);
		if (currentRoomData->code == 4)
			g_engine->_sound->loadVoc("GOTA", 140972, 1029);
		screenTransition(transitionEffect, false, sceneBackground);
		contadorpc = contadorpc2;
		g_engine->_mouseManager->show();
		oldGridX = 0;
		oldGridY = 0;
		checkMouseGrid();
	} break;
	case 24: {
		transitionEffect = Random(15) + 1;
		iframe = 0;
		currentTrajectoryIndex = 0;
		characterPosX = currentRoomData->doors[doorIndex].exitPosX - characterCorrectionX;
		characterPosY = currentRoomData->doors[doorIndex].exitPosY - characerCorrectionY;
		trajectory[currentTrajectoryIndex].x = characterPosX;
		trajectory[currentTrajectoryIndex].y = characterPosY;
		freeAnimation();
		freeScreenObjects();
		g_engine->_mouseManager->hide();
		screenTransition(transitionEffect, true, NULL);
		g_engine->_sound->stopVoc();
		loadScreenData(currentRoomData->doors[doorIndex].nextScene);
		switch (niche[1][niche[1][3]]) {
		case 0:
			currentRoomData->screenObjectIndex[8]->objectName = getObjectName(4);
			break;
		case 561:
			currentRoomData->screenObjectIndex[8]->objectName = getObjectName(5);
			break;
		case 615:
			currentRoomData->screenObjectIndex[8]->objectName = getObjectName(7);
			break;
		case 622:
			currentRoomData->screenObjectIndex[8]->objectName = getObjectName(8);
			break;
		case 623:
			currentRoomData->screenObjectIndex[8]->objectName = getObjectName(9);
			break;
		}
		if (contadorpc > 89)
			showError(274);
		g_engine->_sound->setSfxVolume(leftSfxVol, rightSfxVol);
		if (isTrapSet) {
			currentRoomData->animationFlag = true;
			loadAnimation(currentRoomData->animationName);
			iframe2 = 0;
			currentSecondaryTrajectoryIndex = 1;
			currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].x = 214 - 15;
			currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].y = 115 - 42;
			secondaryAnimation.dir = currentRoomData->secondaryAnimDirections[currentSecondaryTrajectoryIndex - 1];
			secondaryAnimation.posx = currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].x;
			secondaryAnimation.posy = currentRoomData->secondaryAnimTrajectory[currentSecondaryTrajectoryIndex - 1].y;
			secondaryAnimation.depth = 14;

			for (int i = 0; i < maxXGrid; i++)
				for (int j = 0; j < maxYGrid; j++) {
					if (maskGridSecondaryAnim[i][j] > 0) {
						currentRoomData->walkAreasGrid[oldposx + i][oldposy + j] = maskGridSecondaryAnim[i][j];
					}
					if (maskMouseSecondaryAnim[i][j] > 0)
						currentRoomData->mouseGrid[oldposx + i][oldposy + j] = maskMouseSecondaryAnim[i][j];
				}
			assembleScreen();
		}
		screenTransition(transitionEffect, false, sceneBackground);
		if ((isRedDevilCaptured == false) && (isTrapSet == false))
			runaroundRed();
		contadorpc = contadorpc2;
		g_engine->_mouseManager->show();
		oldGridX = 0;
		oldGridY = 0;
		checkMouseGrid();
	} break;
	case 255:
		wcScene();
		break;
	default: {
		transitionEffect = Random(15) + 1;
		iframe = 0;
		currentTrajectoryIndex = 0;
		characterPosX = currentRoomData->doors[doorIndex].exitPosX - characterCorrectionX;
		characterPosY = currentRoomData->doors[doorIndex].exitPosY - characerCorrectionY;
		trajectory[currentTrajectoryIndex].x = characterPosX;
		trajectory[currentTrajectoryIndex].y = characterPosY;
		freeAnimation();
		freeScreenObjects();
		g_engine->_mouseManager->hide();
		screenTransition(transitionEffect, true, NULL);
		g_engine->_sound->stopVoc();
		loadScreenData(currentRoomData->doors[doorIndex].nextScene);
		if (contadorpc > 89)
			showError(274);
		g_engine->_sound->setSfxVolume(leftSfxVol, rightSfxVol);
		switch (currentRoomData->code) {
		case 4:
			g_engine->_sound->loadVoc("GOTA", 140972, 1029);
			break;
		case 23:
			g_engine->_sound->autoPlayVoc("FUENTE", 0, 0);
			break;
		}
		screenTransition(transitionEffect, false, sceneBackground);
		contadorpc = contadorpc2;
		g_engine->_mouseManager->show();
		oldGridX = 0;
		oldGridY = 0;
		checkMouseGrid();
	}
	}
	oldTargetZone = 0;
}
} // end of namespace Tot
