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
		posicioninv = 0;
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
	initialMenu(hechaprimeravez);
	if (partidanueva && !g_engine->shouldQuit()) {
		newGame();
	} else if (continuarpartida && !g_engine->shouldQuit()) {
		loadTemporaryGame();
	} else {
		desactivagrabar = true;
		g_engine->openMainMenuDialog();
		desactivagrabar = false;
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

	while (!salirdeljuego && !g_engine->shouldQuit()) {
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
						numeroaccion = 5;
						action();
						oldxrejilla = 0;
						oldyrejilla = 0;
					} else if (e.kbd.keycode == hotKeyClose) {
						numeroaccion = 6;
						action();
						oldxrejilla = 0;
						oldyrejilla = 0;
					} else if (e.kbd.keycode == hotKeyPickup) {
						numeroaccion = 2;
						action();
						oldxrejilla = 0;
						oldyrejilla = 0;
					} else if (e.kbd.keycode == hotKeyTalk) {
						numeroaccion = 1;
						action();
						oldxrejilla = 0;
						oldyrejilla = 0;
					} else if (e.kbd.keycode == hotKeyLook) {
						numeroaccion = 3;
						action();
						oldxrejilla = 0;
						oldyrejilla = 0;
					} else if (e.kbd.keycode == hotKeyUse) {
						numeroaccion = 4;
						action();
						oldxrejilla = 0;
						oldyrejilla = 0;
					} else {
						numeroaccion = 0; // go to
					}
				}
			} else if (e.type == Common::EVENT_LBUTTONUP) {
				mouseClickX = e.mouse.x;
				mouseClickY = e.mouse.y;
				if (mouseClickY > 0 && mouseClickY < 131) {
					switch (numeroaccion) {
					case 0: // go to
						contadorpc2 = contadorpc;
						// gets the area where the character is now standing. Area is calculated using xframe,yframe plus some adjustments to get the center of the feet
						zonaactual = currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount];
						if (zonaactual < 10) {
							xframe2 = mouseClickX + 7;
							yframe2 = mouseClickY + 7;
							// obtains the target area from the clicked coordinates
							zonadestino = currentRoomData->walkAreasGrid[xframe2 / xGridCount][yframe2 / yGridCount];
							if (currentRoomData->code == 21 && currentRoomData->animationFlag) {
								if ((zonadestino >= 1 && zonadestino <= 5) ||
									(zonadestino >= 9 && zonadestino <= 13) ||
									(zonadestino >= 18 && zonadestino <= 21) ||
									zonadestino == 24 || zonadestino == 25) {

									zonadestino = 7;
									mouseClickX = 232;
									mouseClickY = 75;

									xframe2 = mouseClickX + 7;
									yframe2 = mouseClickY + 7;
								}
							}

							if (oldzonadestino != zonadestino || zonadestino < 10) {
								oldzonadestino = zonadestino;
								// Resets the entire route
								calculateRoute(zonaactual, zonadestino);

								indicepuertas = 0;
								cambiopantalla = false;

								for (indicepuertas = 0; indicepuertas < 5; indicepuertas++) {
									if (currentRoomData->doors[indicepuertas].doorcode == zonadestino) {

										if (currentRoomData->doors[indicepuertas].openclosed == 1) {
											cambiopantalla = true;
											break;
										} else if ((currentRoomData->code == 5 && zonadestino == 27) || (currentRoomData->code == 6 && zonadestino == 21)) {
											;
										} else {
											pasos -= 1;
										}
									}
								}
								// Sets xframe2 again due to the substraction when closed doors
								xframe2 = pasos;
							} else
								xframe2 = 0;
						}
						break;
					case 1: // talk
						cambiopantalla = false;
						numeroaccion = 0;
						talkScreenObject();
						contadorpc2 = contadorpc;
						break;
					case 2: // pick up
						cambiopantalla = false;
						numeroaccion = 0;
						pickupScreenObject();
						contadorpc = contadorpc2;
						break;
					case 3: // look at
						cambiopantalla = false;
						destinox_paso = (mouseClickX + 7) / xGridCount;
						destinoy_paso = (mouseClickY + 7) / yGridCount;
						if (currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[destinox_paso][destinoy_paso]]->fileIndex > 0) {
							goToObject(
								currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount],
								currentRoomData->walkAreasGrid[destinox_paso][destinoy_paso]);
							if (currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[destinox_paso][destinoy_paso]]->fileIndex == 562)

								switch (currentRoomData->code) {
								case 20:
									if (hornacina[0][hornacina[0][3]] > 0)
										readItemRegister(hornacina[0][hornacina[0][3]]);
									else
										readItemRegister(562);
									break;
								case 24:
									if (hornacina[1][hornacina[1][3]] > 0)
										readItemRegister(hornacina[1][hornacina[1][3]]);
									else
										readItemRegister(562);
									break;
								}
							else
								readItemRegister(currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[destinox_paso][destinoy_paso]]->fileIndex);
							if (regobj.lookAtTextRef > 0)
								drawText(regobj.lookAtTextRef);
							numeroaccion = 0;
						}
						break;
					case 4: // use
						cambiopantalla = false;
						numeroaccion = 0;
						useScreenObject();
						contadorpc = contadorpc2;
						break;
					case 5: // open
						cambiopantalla = false;
						numeroaccion = 0;
						openScreenObject();
						break;
					case 6: { // close
						cambiopantalla = false;
						numeroaccion = 0;
						closeScreenObject();
						contadorpc = contadorpc2;
					} break;
					}
				} else if (mouseClickY > 148 && mouseClickY < 158) {
					if (mouseClickX >= 3 && mouseClickX <= 53) {
						numeroaccion = 1;
						action();
						break;
					} else if (mouseClickX >= 58 && mouseClickX <= 103) {
						numeroaccion = 2;
						action();
						break;
					} else if (mouseClickX >= 108 && mouseClickX <= 153) {
						numeroaccion = 3;
						action();
						break;
					} else if (mouseClickX >= 158 && mouseClickX <= 198) {
						numeroaccion = 4;
						action();
						break;
					} else if (mouseClickX >= 203 && mouseClickX <= 248) {
						numeroaccion = 5;
						action();
						break;
					} else if (mouseClickX >= 253 && mouseClickX <= 311) {
						numeroaccion = 6;
						action();
						break;
					} else {
						numeroaccion = 0;
						action();
						contadorpc2 = contadorpc;
					}
				} else if (mouseClickY > 166 && mouseClickY < 199) {
					if (mouseClickX >= 3 && mouseClickX <= 19) {
						inventory(0, 33);
						break;
					} else if (mouseClickX >= 26 && mouseClickX <= 65) {
						handleAction(posicioninv);
						break;
					} else if (mouseClickX >= 70 && mouseClickX <= 108) {
						handleAction(posicioninv + 1);
						break;
					} else if (mouseClickX >= 113 && mouseClickX <= 151) {
						handleAction(posicioninv + 2);
						break;
					} else if (mouseClickX >= 156 && mouseClickX <= 194) {
						handleAction(posicioninv + 3);
						break;
					} else if (mouseClickX >= 199 && mouseClickX <= 237) {
						handleAction(posicioninv + 4);
						break;
					} else if (mouseClickX >= 242 && mouseClickX <= 280) {
						handleAction(posicioninv + 5);
						break;
					} else if (mouseClickX >= 290 && mouseClickX <= 311) {
						inventory(1, 33);
						break;
					} else {
						numeroaccion = 0;
						action();
					}
				}
			} else if (e.type == Common::EVENT_RBUTTONUP) {
				mouseClickX = e.mouse.x;
				mouseClickY = e.mouse.y;
				destinox_paso = (mouseClickX + 7) / xGridCount;
				destinoy_paso = (mouseClickY + 7) / yGridCount;
				contadorpc2 = contadorpc;
				if (destinoy_paso < 28) {
					RoomObjectListEntry obj = *currentRoomData->screenObjectIndex[currentRoomData->mouseGrid[destinox_paso][destinoy_paso]];
					if (obj.fileIndex > 0) {

						drawLookAtItem(obj);
						goToObject(currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount], currentRoomData->walkAreasGrid[destinox_paso][destinoy_paso]);
						if (obj.fileIndex == 562)

							switch (currentRoomData->code) {
							case 20:
								if (hornacina[0][hornacina[0][3]] > 0)
									readItemRegister(hornacina[0][hornacina[0][3]]);
								else
									readItemRegister(562);
								break;
							case 24:
								if (hornacina[1][hornacina[1][3]] > 0)
									readItemRegister(hornacina[1][hornacina[1][3]]);
								else
									readItemRegister(562);
								break;
							}
						else
							readItemRegister(obj.fileIndex);
						if (regobj.lookAtTextRef > 0)
							drawText(regobj.lookAtTextRef);
						numeroaccion = 0;
					}
				}
			}
		}

		checkMouseGrid();
		advanceAnimations(false, true);

		// Scene changes
		if (xframe2 == 0 && cambiopantalla) {
			sceneChange();
		}

		if (escapePressed && xframe2 == 0) {
			freeAnimation();
			freeScreenObjects();
			contadorpc2 = contadorpc;
			partidanueva = false;
			continuarpartida = false;
			g_engine->saveAutosaveIfEnabled();
			totalFadeOut(0);
			g_engine->_sound->fadeOutMusic(musicVolLeft, musicVolRight);
			g_engine->_graphics->clear();
			g_engine->_sound->playMidi("INTRODUC", true);
			g_engine->_sound->fadeInMusic(musicVolLeft, musicVolRight);
			initialMenu(true);
			verifyCopyProtection2();

			if (partidanueva && !g_engine->shouldQuit()) {
				newGame();
			} else if (continuarpartida && !g_engine->shouldQuit())
				loadTemporaryGame();
			else {
				desactivagrabar = true;
				g_engine->openMainMenuDialog();
				contadorpc = contadorpc2;
				desactivagrabar = false;
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
				indicetray = 0;
				characterPosX = 160;
				characterPosY = 60;
				trayec[indicetray].x = characterPosX;
				trayec[indicetray].y = characterPosY;
				loadScreenData(20);
				g_engine->_sound->fadeOutMusic(musicVolLeft, musicVolRight);
				g_engine->_sound->playMidi("SEGUNDA", true);
				g_engine->_sound->fadeInMusic(musicVolLeft, musicVolRight);
				screenTransition(1, false, sceneBackground);
				mask();
				posicioninv = 0;
				drawBackpack();
				g_engine->_mouseManager->show();

				primera[8] = true;
				oldxrejilla = 0;
				oldyrejilla = 0;
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
				for (int indice = 0; indice < depthLevelCount; indice++) {
					if (screenObjects[indice] != NULL) {
						if (true) {
							// debug
							uint16 w = READ_LE_UINT16(screenObjects[indice]);
							uint16 h = READ_LE_UINT16(screenObjects[indice] + 2);
							Common::Rect r = Common::Rect(depthMap[indice].posx, depthMap[indice].posy, depthMap[indice].posx + w, depthMap[indice].posy + h);
							drawRect(180, depthMap[indice].posx, depthMap[indice].posy, depthMap[indice].posx + w, depthMap[indice].posy + h);

							littText(r.left, r.top, Common::String().format("%d", indice), 0);
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
		obtainName(nombreficherofoto);
	}
	if (!g_engine->shouldQuit()) {
		generateDiploma(nombreficherofoto);
	}
	if (!g_engine->shouldQuit()) {
		credits();
	}
	return EXIT_SUCCESS;
}

void sceneChange() {
	cambiopantalla = false;
	contadorpc = contadorpc2;
	setRoomTrajectories(altoanimado, anchoanimado, RESTORE);
	saveRoomData(currentRoomData, rooms);
	// verifyCopyProtection();
	g_engine->_sound->setSfxVolume(leftSfxVol, rightSfxVol);

	switch (currentRoomData->doors[indicepuertas].nextScene) {
	case 2: {
		transitionEffect = Random(15) + 1;
		iframe = 0;
		indicetray = 0;
		characterPosX = currentRoomData->doors[indicepuertas].exitPosX - characterCorrectionX;
		characterPosY = currentRoomData->doors[indicepuertas].exitPosY - characerCorrectionY;
		trayec[indicetray].x = characterPosX;
		trayec[indicetray].y = characterPosY;
		freeAnimation();
		freeScreenObjects();
		g_engine->_mouseManager->hide();

		screenTransition(transitionEffect, true, NULL);
		g_engine->_sound->stopVoc();
		loadScreenData(currentRoomData->doors[indicepuertas].nextScene);
		if (contadorpc > 89)
			showError(274);
		g_engine->_sound->setSfxVolume(leftSfxVol, rightSfxVol);
		if (teleencendida)
			g_engine->_sound->autoPlayVoc("PARASITO", 355778, 20129);
		else
			cargatele();
		screenTransition(transitionEffect, false, sceneBackground);
		contadorpc = contadorpc2;
		g_engine->_mouseManager->show();
		oldxrejilla = 0;
		oldyrejilla = 0;
	} break;
	case 5: {
		if (currentRoomData->code != 6) {
			transitionEffect = Random(15) + 1;
			iframe = 0;
			indicetray = 0;
			characterPosX = currentRoomData->doors[indicepuertas].exitPosX - characterCorrectionX;
			characterPosY = currentRoomData->doors[indicepuertas].exitPosY - characerCorrectionY + 15;
			trayec[indicetray].x = characterPosX;
			trayec[indicetray].y = characterPosY;
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			screenTransition(transitionEffect, true, NULL);
			loadScreenData(currentRoomData->doors[indicepuertas].nextScene);
			g_engine->_sound->stopVoc();
			g_engine->_sound->autoPlayVoc("CALDERA", 6433, 15386);
			g_engine->_sound->setSfxVolume(leftSfxVol, 0);
			screenTransition(transitionEffect, false, sceneBackground);
			g_engine->_mouseManager->show();
			oldxrejilla = 0;
			oldyrejilla = 0;
			checkMouseGrid();
		} else {

			zonaactual = currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount];
			zonadestino = 21;
			goToObject(zonaactual, zonadestino);
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			g_engine->_sound->setSfxVolume(leftSfxVol, 0);
			loadScrollData(currentRoomData->doors[indicepuertas].nextScene, true, 22, -2);
			g_engine->_mouseManager->show();
			oldxrejilla = 0;
			oldyrejilla = 0;
			checkMouseGrid();
		}
	} break;
	case 6: {
		zonaactual = currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount];
		zonadestino = 27;
		goToObject(zonaactual, zonadestino);
		freeAnimation();
		freeScreenObjects();
		g_engine->_mouseManager->hide();
		g_engine->_sound->setSfxVolume(leftSfxVol, rightSfxVol);
		loadScrollData(currentRoomData->doors[indicepuertas].nextScene, false, 22, 2);
		g_engine->_mouseManager->show();
		oldxrejilla = 0;
		oldyrejilla = 0;
		checkMouseGrid();
	} break;
	case 9: {
		transitionEffect = Random(15) + 1;
		freeAnimation();
		freeScreenObjects();
		g_engine->_mouseManager->hide();
		screenTransition(transitionEffect, true, NULL);
		iframe = 0;
		indicetray = 0;
		characterPosX = currentRoomData->doors[indicepuertas].exitPosX - characterCorrectionX;
		characterPosY = currentRoomData->doors[indicepuertas].exitPosY - characerCorrectionY;
		trayec[indicetray].x = characterPosX;
		trayec[indicetray].y = characterPosY;
		loadScreenData(currentRoomData->doors[indicepuertas].nextScene);
		screenTransition(transitionEffect, false, sceneBackground);
		g_engine->_mouseManager->show();

		oldxrejilla = 0;
		oldyrejilla = 0;
		checkMouseGrid();
	} break;
	case 12: {
		if (currentRoomData->code != 13) {
			transitionEffect = Random(15) + 1;
			iframe = 0;
			indicetray = 0;
			characterPosX = currentRoomData->doors[indicepuertas].exitPosX - characterCorrectionX;
			characterPosY = currentRoomData->doors[indicepuertas].exitPosY - characerCorrectionY;
			trayec[indicetray].x = characterPosX;
			trayec[indicetray].y = characterPosY;
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			screenTransition(transitionEffect, true, NULL);
			loadScreenData(currentRoomData->doors[indicepuertas].nextScene);
			screenTransition(transitionEffect, false, sceneBackground);
			g_engine->_mouseManager->show();
			oldxrejilla = 0;
			oldyrejilla = 0;
			g_engine->_mouseManager->show();
		} else {

			zonaactual = currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount];
			goToObject(zonaactual, zonadestino);
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			loadScrollData(currentRoomData->doors[indicepuertas].nextScene, false, 64, 0);
			g_engine->_mouseManager->show();
			oldxrejilla = 0;
			oldyrejilla = 0;
			checkMouseGrid();
		}
	} break;
	case 13: {
		switch (currentRoomData->code) {
		case 12: {
			zonaactual = currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount];
			goToObject(zonaactual, zonadestino);
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			loadScrollData(currentRoomData->doors[indicepuertas].nextScene, true, 64, 0);
			g_engine->_mouseManager->show();
			oldxrejilla = 0;
			oldyrejilla = 0;
			checkMouseGrid();
		} break;
		case 14: {
			zonaactual = currentRoomData->walkAreasGrid[(characterPosX + characterCorrectionX) / xGridCount][(characterPosY + characerCorrectionY) / yGridCount];
			goToObject(zonaactual, zonadestino);
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			loadScrollData(currentRoomData->doors[indicepuertas].nextScene, false, 56, 0);
			g_engine->_mouseManager->show();
			oldxrejilla = 0;
			oldyrejilla = 0;
			checkMouseGrid();
		} break;
		}
	} break;
	case 14: {
		if (currentRoomData->code != 13) {
			transitionEffect = Random(15) + 1;
			iframe = 0;
			indicetray = 0;
			characterPosX = currentRoomData->doors[indicepuertas].exitPosX - characterCorrectionX;
			characterPosY = currentRoomData->doors[indicepuertas].exitPosY - characerCorrectionY;
			trayec[indicetray].x = characterPosX;
			trayec[indicetray].y = characterPosY;
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			screenTransition(transitionEffect, true, NULL);
			loadScreenData(currentRoomData->doors[indicepuertas].nextScene);
			screenTransition(transitionEffect, false, sceneBackground);
			g_engine->_mouseManager->show();
			oldxrejilla = 0;
			oldyrejilla = 0;
			checkMouseGrid();
		} else {

			zonaactual = currentRoomData->walkAreasGrid[((characterPosX + characterCorrectionX) / xGridCount)][((characterPosY + characerCorrectionY) / yGridCount)];
			goToObject(zonaactual, zonadestino);
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			loadScrollData(currentRoomData->doors[indicepuertas].nextScene, true, 56, 0);
			g_engine->_mouseManager->show();
			oldxrejilla = 0;
			oldyrejilla = 0;
			checkMouseGrid();
		}
	} break;
	case 17: {
		transitionEffect = Random(15) + 1;
		iframe = 0;
		indicetray = 0;
		characterPosX = currentRoomData->doors[indicepuertas].exitPosX - characterCorrectionX;
		characterPosY = currentRoomData->doors[indicepuertas].exitPosY - characerCorrectionY;
		trayec[indicetray].x = characterPosX;
		trayec[indicetray].y = characterPosY;
		freeAnimation();
		freeScreenObjects();
		g_engine->_mouseManager->hide();
		screenTransition(transitionEffect, true, NULL);
		g_engine->_sound->stopVoc();
		loadScreenData(currentRoomData->doors[indicepuertas].nextScene);
		if (libro[0] == true && currentRoomData->animationFlag == true)
			disableSecondAnimation();
		if (contadorpc > 89)
			showError(274);
		g_engine->_sound->setSfxVolume(leftSfxVol, rightSfxVol);
		screenTransition(transitionEffect, false, sceneBackground);
		contadorpc = contadorpc2;
		g_engine->_mouseManager->show();
		oldxrejilla = 0;
		oldyrejilla = 0;
		checkMouseGrid();
	} break;
	case 18: {
		if (currentRoomData->code != 19) {
			transitionEffect = Random(15) + 1;
			iframe = 0;
			indicetray = 0;
			characterPosX = currentRoomData->doors[indicepuertas].exitPosX - characterCorrectionX;
			characterPosY = currentRoomData->doors[indicepuertas].exitPosY - characerCorrectionY;
			trayec[indicetray].x = characterPosX;
			trayec[indicetray].y = characterPosY;
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			screenTransition(transitionEffect, true, NULL);
			loadScreenData(currentRoomData->doors[indicepuertas].nextScene);
			screenTransition(transitionEffect, false, sceneBackground);
			g_engine->_mouseManager->show();
			oldxrejilla = 0;
			oldyrejilla = 0;
			checkMouseGrid();
		} else {

			zonaactual = currentRoomData->walkAreasGrid[((characterPosX + characterCorrectionX) / xGridCount)][((characterPosY + characerCorrectionY) / yGridCount)];
			goToObject(zonaactual, zonadestino);
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			loadScrollData(currentRoomData->doors[indicepuertas].nextScene, true, 131, -1);
			g_engine->_mouseManager->show();
			oldxrejilla = 0;
			oldyrejilla = 0;
			checkMouseGrid();
		}
	} break;
	case 19: {
		if (currentRoomData->code != 18) {
			transitionEffect = Random(15) + 1;
			iframe = 0;
			indicetray = 0;
			characterPosX = currentRoomData->doors[indicepuertas].exitPosX - characterCorrectionX;
			characterPosY = currentRoomData->doors[indicepuertas].exitPosY - characerCorrectionY;
			trayec[indicetray].x = characterPosX;
			trayec[indicetray].y = characterPosY;
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			screenTransition(transitionEffect, true, NULL);
			loadScreenData(currentRoomData->doors[indicepuertas].nextScene);
			screenTransition(transitionEffect, false, sceneBackground);
			g_engine->_mouseManager->show();
			oldxrejilla = 0;
			oldyrejilla = 0;
			checkMouseGrid();
		} else {

			zonaactual = currentRoomData->walkAreasGrid[((characterPosX + characterCorrectionX) / xGridCount)][((characterPosY + characerCorrectionY) / yGridCount)];
			goToObject(zonaactual, zonadestino);
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			loadScrollData(currentRoomData->doors[indicepuertas].nextScene, false, 131, 1);
			g_engine->_mouseManager->show();
			oldxrejilla = 0;
			oldyrejilla = 0;
			checkMouseGrid();
		}
	} break;
	case 20: {
		transitionEffect = Random(15) + 1;
		iframe = 0;
		indicetray = 0;
		characterPosX = currentRoomData->doors[indicepuertas].exitPosX - characterCorrectionX;
		characterPosY = currentRoomData->doors[indicepuertas].exitPosY - characerCorrectionY;
		trayec[indicetray].x = characterPosX;
		trayec[indicetray].y = characterPosY;
		freeAnimation();
		freeScreenObjects();
		g_engine->_mouseManager->hide();
		screenTransition(transitionEffect, true, NULL);
		g_engine->_sound->stopVoc();
		loadScreenData(currentRoomData->doors[indicepuertas].nextScene);
		switch (hornacina[0][hornacina[0][3]]) {
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
		oldxrejilla = 0;
		oldyrejilla = 0;
		checkMouseGrid();
	} break;
	case 24: {
		transitionEffect = Random(15) + 1;
		iframe = 0;
		indicetray = 0;
		characterPosX = currentRoomData->doors[indicepuertas].exitPosX - characterCorrectionX;
		characterPosY = currentRoomData->doors[indicepuertas].exitPosY - characerCorrectionY;
		trayec[indicetray].x = characterPosX;
		trayec[indicetray].y = characterPosY;
		freeAnimation();
		freeScreenObjects();
		g_engine->_mouseManager->hide();
		screenTransition(transitionEffect, true, NULL);
		g_engine->_sound->stopVoc();
		loadScreenData(currentRoomData->doors[indicepuertas].nextScene);
		switch (hornacina[1][hornacina[1][3]]) {
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
		if (trampa_puesta) {
			currentRoomData->animationFlag = true;
			loadAnimation(currentRoomData->animationName);
			iframe2 = 0;
			indicetray2 = 1;
			currentRoomData->secondaryAnimTrajectory[indicetray2 - 1].x = 214 - 15;
			currentRoomData->secondaryAnimTrajectory[indicetray2 - 1].y = 115 - 42;
			animado.dir = currentRoomData->secondaryAnimDirections[indicetray2 - 1];
			animado.posx = currentRoomData->secondaryAnimTrajectory[indicetray2 - 1].x;
			animado.posy = currentRoomData->secondaryAnimTrajectory[indicetray2 - 1].y;
			animado.depth = 14;

			for (int i = 0; i < maxrejax; i++)
				for (int j = 0; j < maxrejay; j++) {
					if (rejamascaramovto[i][j] > 0) {
						currentRoomData->walkAreasGrid[oldposx + i][oldposy + j] = rejamascaramovto[i][j];
					}
					if (rejamascararaton[i][j] > 0)
						currentRoomData->mouseGrid[oldposx + i][oldposy + j] = rejamascararaton[i][j];
				}
			assembleScreen();
		}
		screenTransition(transitionEffect, false, sceneBackground);
		if ((rojo_capturado == false) && (trampa_puesta == false))
			runaroundRed();
		contadorpc = contadorpc2;
		g_engine->_mouseManager->show();
		oldxrejilla = 0;
		oldyrejilla = 0;
		checkMouseGrid();
	} break;
	case 255:
		wcScene();
		break;
	default: {
		transitionEffect = Random(15) + 1;
		iframe = 0;
		indicetray = 0;
		characterPosX = currentRoomData->doors[indicepuertas].exitPosX - characterCorrectionX;
		characterPosY = currentRoomData->doors[indicepuertas].exitPosY - characerCorrectionY;
		trayec[indicetray].x = characterPosX;
		trayec[indicetray].y = characterPosY;
		freeAnimation();
		freeScreenObjects();
		g_engine->_mouseManager->hide();
		screenTransition(transitionEffect, true, NULL);
		g_engine->_sound->stopVoc();
		loadScreenData(currentRoomData->doors[indicepuertas].nextScene);
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
		oldxrejilla = 0;
		oldyrejilla = 0;
		checkMouseGrid();
	}
	}
	oldzonadestino = 0;
}
} // end of namespace Tot
