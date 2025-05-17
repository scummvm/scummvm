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
#include "tot/mouse.h"
#include "tot/playanim.h"
#include "tot/routines.h"
#include "tot/routines2.h"
#include "tot/sound.h"
#include "tot/tot.h"
#include "tot/util.h"

namespace Tot {
byte iaux, iaux2;

bool showMouseGrid = false;
bool showScreenGrid = false;
bool showGameGrid = false;
bool drawObjectAreas = false;

void loadTemporaryGame() {
	g_engine->loadGameState(g_engine->getMetaEngine()->getAutosaveSlot());
}

int startGame();

void newGame() {
	g_engine->_mouseManager->hide();
	obtainName(nombrepersonaje);

	if (!g_engine->shouldQuit()) {
		totalFadeOut(0);
		clear();
		processingActive();
		freeInventory();
		loadObjects();
		resetGameState();
		inGame = true;
		for (int i = 0; i < inventoryIconCount; i++) {
			mobj[i].bitmapIndex = 34;
			mobj[i].code = 0;
			mobj[i].objectName = "VACIO";
		}
		readConversationFile(Common::String("CONVERSA.TRE"));
		initializeScreenFile();
		initializeObjectFile();
		loadPalette("DEFAULT");
		loadScreenData(1);
		effect(13, false, background);
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
	clear();
	processingActive();

	loadCharAnimation();
	loadObjects();

	setMidiVolume(0, 0);
	playMidiFile("SILENT", false);

	setMouseArea(0, 0, 305, 185);
	playMidiFile("SILENT", true);

	totalFadeOut(0);
	clear();

	loadPalette("DEFAULT");
	loadScreenMemory();

	// initialLogo();
	playMidiFile("INTRODUC", true);
	setMidiVolume(3, 3);

	// firstIntroduction();
	setMousePos(1, xraton, yraton);
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

	boolean enforceSecondPart = false;
	if (enforceSecondPart) { // DEBUG
		completadalista1 = true;
		completadalista2 = true;
		gamePart = 1;
	}

	return startGame();
}

int startGame() {
	lowerMidiVolume(volumenmelodiaizquierdo, volumenmelodiaderecho);
	switch (gamePart) {
	case 1:
		playMidiFile("PRIMERA", true);
		break;
	case 2:
		playMidiFile("SEGUNDA", true);
		break;
	}
	contadorpc2 = contadorpc;
	restoreMidiVolume(volumenmelodiaizquierdo, volumenmelodiaderecho);
	inGame = true;

	Common::Event e;
	while (!salirdeljuego && !g_engine->shouldQuit()) {
		bool escapePressed = false;
		g_engine->_chrono->updateChrono();
		g_engine->_mouseManager->animateMouseIfNeeded();
		// debug
		while (g_system->getEventManager()->pollEvent(e)) {
			if (isMouseEvent(e)) {
				g_engine->_mouseManager->setMousePos(e.mouse);
				xraton = e.mouse.x;
				yraton = e.mouse.y;
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
					/* Debug */
					{
					case Common::KEYCODE_5:
						showMouseGrid = !showMouseGrid;
						break;
					case Common::KEYCODE_6:
						showScreenGrid = !showScreenGrid;
						break;
					case Common::KEYCODE_7:
						showGameGrid = !showGameGrid;
						break;
					case Common::KEYCODE_0:
						effect(13, false, background);
						break;
					case Common::KEYCODE_8:
						drawObjectAreas = !drawObjectAreas;
						break;
					case Common::KEYCODE_1:
						setRoomTrajectories(altoanimado, anchoanimado, RESTORE);
						break;
					case Common::KEYCODE_2:
						setRoomTrajectories(altoanimado, anchoanimado, SET_WITH_ANIM);
						break;
					}
				/* End debug */
				case Common::KEYCODE_a: // open
					numeroaccion = 5;
					action();
					oldxrejilla = 0;
					oldyrejilla = 0;
					break;
				case Common::KEYCODE_e: // close
					numeroaccion = 6;
					action();
					oldxrejilla = 0;
					oldyrejilla = 0;
					break;
				case Common::KEYCODE_c: // pickup
					numeroaccion = 2;
					action();
					oldxrejilla = 0;
					oldyrejilla = 0;
					break;
				case Common::KEYCODE_h: // talk
					numeroaccion = 1;
					action();
					oldxrejilla = 0;
					oldyrejilla = 0;
					break;
				case Common::KEYCODE_m: // look
					numeroaccion = 3;
					action();
					oldxrejilla = 0;
					oldyrejilla = 0;
					break;
				case Common::KEYCODE_u: // use
					numeroaccion = 4;
					action();
					oldxrejilla = 0;
					oldyrejilla = 0;
					break;
				default:
					numeroaccion = 0; // go to
				}
			} else if (e.type == Common::EVENT_LBUTTONUP) {
				pulsax = e.mouse.x;
				pulsay = e.mouse.y;
				if (pulsay > 0 && pulsay < 131) {
					switch (numeroaccion) {
					case 0: // go to
						contadorpc2 = contadorpc;
						// gets the area where the character is now standing. Area is calculated using xframe,yframe plus some adjustments to get the center of the feet
						zonaactual = currentRoomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory];
						if (zonaactual < 10) {
							xframe2 = pulsax + 7;
							yframe2 = pulsay + 7;
							// obtains the target area from the clicked coordinates
							zonadestino = currentRoomData->rejapantalla[xframe2 / factorx][yframe2 / factory];
							if (currentRoomData->codigo == 21 && currentRoomData->banderamovimiento) {
								if ((zonadestino >= 1 && zonadestino <= 5) ||
									(zonadestino >= 9 && zonadestino <= 13) ||
									(zonadestino >= 18 && zonadestino <= 21) ||
									zonadestino == 24 || zonadestino == 25) {

									zonadestino = 7;
									pulsax = 232;
									pulsay = 75;

									xframe2 = pulsax + 7;
									yframe2 = pulsay + 7;
								}
							}

							if (oldzonadestino != zonadestino || zonadestino < 10) {
								oldzonadestino = zonadestino;
								// Resets the entire route
								calculateRoute(zonaactual, zonadestino);

								indicepuertas = 0;
								cambiopantalla = false;

								for (indicepuertas = 0; indicepuertas < 5; indicepuertas++) {
									if (currentRoomData->doors[indicepuertas].codigopuerta == zonadestino) {

										if (currentRoomData->doors[indicepuertas].abiertacerrada == 1) {
											cambiopantalla = true;
											break;
										} else if ((currentRoomData->codigo == 5 && zonadestino == 27) || (currentRoomData->codigo == 6 && zonadestino == 21)) {
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
						destinox_paso = (pulsax + 7) / factorx;
						destinoy_paso = (pulsay + 7) / factory;
						if (currentRoomData->indexadoobjetos[currentRoomData->mouseGrid[destinox_paso][destinoy_paso]]->indicefichero > 0) {
							goToObject(
								currentRoomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory],
								currentRoomData->rejapantalla[destinox_paso][destinoy_paso]);
							if (currentRoomData->indexadoobjetos[currentRoomData->mouseGrid[destinox_paso][destinoy_paso]]->indicefichero == 562)

								switch (currentRoomData->codigo) {
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
								readItemRegister(currentRoomData->indexadoobjetos[currentRoomData->mouseGrid[destinox_paso][destinoy_paso]]->indicefichero);
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
				} else if (pulsay > 148 && pulsay < 158) {
					if (pulsax >= 3 && pulsax <= 53) {
						numeroaccion = 1;
						action();
						break;
					} else if (pulsax >= 58 && pulsax <= 103) {
						numeroaccion = 2;
						action();
						break;
					} else if (pulsax >= 108 && pulsax <= 153) {
						numeroaccion = 3;
						action();
						break;
					} else if (pulsax >= 158 && pulsax <= 198) {
						numeroaccion = 4;
						action();
						break;
					} else if (pulsax >= 203 && pulsax <= 248) {
						numeroaccion = 5;
						action();
						break;
					} else if (pulsax >= 253 && pulsax <= 311) {
						numeroaccion = 6;
						action();
						break;
					} else {
						numeroaccion = 0;
						action();
						contadorpc2 = contadorpc;
					}
				} else if (pulsay > 166 && pulsay < 199) {
					if (pulsax >= 3 && pulsax <= 19) {
						inventory(0, 33);
						break;
					} else if (pulsax >= 26 && pulsax <= 65) {
						handleAction(posicioninv);
						break;
					} else if (pulsax >= 70 && pulsax <= 108) {
						handleAction(posicioninv + 1);
						break;
					} else if (pulsax >= 113 && pulsax <= 151) {
						handleAction(posicioninv + 2);
						break;
					} else if (pulsax >= 156 && pulsax <= 194) {
						handleAction(posicioninv + 3);
						break;
					} else if (pulsax >= 199 && pulsax <= 237) {
						handleAction(posicioninv + 4);
						break;
					} else if (pulsax >= 242 && pulsax <= 280) {
						handleAction(posicioninv + 5);
						break;
					} else if (pulsax >= 290 && pulsax <= 311) {
						inventory(1, 33);
						break;
					} else {
						numeroaccion = 0;
						action();
					}
				}
			} else if (e.type == Common::EVENT_RBUTTONUP) {
				pulsax = e.mouse.x;
				pulsay = e.mouse.y;
				destinox_paso = (pulsax + 7) / factorx;
				destinoy_paso = (pulsay + 7) / factory;
				contadorpc2 = contadorpc;
				if (destinoy_paso < 28) {
					RoomObjectListEntry obj = *currentRoomData->indexadoobjetos[currentRoomData->mouseGrid[destinox_paso][destinoy_paso]];
					if (obj.indicefichero > 0) {

						drawLookAtItem(obj);
						goToObject(currentRoomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory], currentRoomData->rejapantalla[destinox_paso][destinoy_paso]);
						if (obj.indicefichero == 562)

							switch (currentRoomData->codigo) {
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
							readItemRegister(obj.indicefichero);
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
			lowerMidiVolume(volumenmelodiaizquierdo, volumenmelodiaderecho);
			clear();
			playMidiFile("INTRODUC", true);
			restoreMidiVolume(volumenmelodiaizquierdo, volumenmelodiaderecho);
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
			lowerMidiVolume(volumenmelodiaizquierdo, volumenmelodiaderecho);
			switch (gamePart) {
			case 1:
				playMidiFile("PRIMERA", true);
				break;
			case 2:
				playMidiFile("SEGUNDA", true);
				break;
			}
			restoreMidiVolume(volumenmelodiaizquierdo, volumenmelodiaderecho);
		}

		switch (gamePart) {
		case 1:
			if (completadalista1 && completadalista2) {
				completadalista1 = false;
				completadalista2 = false;
				contadorpc = contadorpc2;
				gamePart = 2;
				iframe = 0;
				freeInventory();
				freeAnimation();
				freeScreenObjects();
				g_engine->_mouseManager->hide();
				partialFadeOut(234);
				lowerMidiVolume(volumenmelodiaizquierdo, volumenmelodiaderecho);
				playMidiFile("CREDITOS", true);
				restoreMidiVolume(volumenmelodiaizquierdo, volumenmelodiaderecho);
				if (contadorpc2 > 43)
					showError(274);
				sacrificeScene();
				clear();
				loadObjects();
				loadPalette("SEGUNDA");
				indicetray = 0;
				characterPosX = 160;
				characterPosY = 60;
				trayec[indicetray].x = characterPosX;
				trayec[indicetray].y = characterPosY;
				loadScreenData(20);
				lowerMidiVolume(volumenmelodiaizquierdo, volumenmelodiaderecho);
				playMidiFile("SEGUNDA", true);
				restoreMidiVolume(volumenmelodiaizquierdo, volumenmelodiaderecho);
				effect(1, false, background);
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
			if (showMouseGrid) {
				drawMouseGrid(currentRoomData);
			}
			if (showScreenGrid) {
				drawScreenGrid(currentRoomData);
			}
			if (showGameGrid) {
				drawGrid();
			}

			if (drawObjectAreas) {
				for (int indice = 0; indice < nivelesdeprof; indice++) {
					if (screenObjects[indice] != NULL) {
						if (true) {
							// debug
							uint16 w = READ_LE_UINT16(screenObjects[indice]);
							uint16 h = READ_LE_UINT16(screenObjects[indice] + 2);
							Common::Rect r = Common::Rect(depthMap[indice].posx, depthMap[indice].posy, depthMap[indice].posx + w, depthMap[indice].posy + h);
							drawRect(180, depthMap[indice].posx, depthMap[indice].posy, depthMap[indice].posx + w, depthMap[indice].posy + h);

							outtextxy(r.left, r.top, Common::String().format("%d", indice), 0);
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
	// setSfxVolume(volumenfxizquierdo, volumenfxderecho);

	switch (currentRoomData->doors[indicepuertas].pantallaquecarga) {
	case 2: {
		tipoefectofundido = Random(15) + 1;
		iframe = 0;
		indicetray = 0;
		characterPosX = currentRoomData->doors[indicepuertas].posxsalida - rectificacionx;
		characterPosY = currentRoomData->doors[indicepuertas].posysalida - rectificaciony;
		trayec[indicetray].x = characterPosX;
		trayec[indicetray].y = characterPosY;
		freeAnimation();
		freeScreenObjects();
		g_engine->_mouseManager->hide();

		effect(tipoefectofundido, true, NULL);
		stopVoc();
		loadScreenData(currentRoomData->doors[indicepuertas].pantallaquecarga);
		if (contadorpc > 89)
			showError(274);
		setSfxVolume(volumenfxizquierdo, volumenfxderecho);
		if (teleencendida)
			autoPlayVoc("PARASITO", 355778, 20129);
		else
			cargatele();
		effect(tipoefectofundido, false, background);
		contadorpc = contadorpc2;
		g_engine->_mouseManager->show();
		oldxrejilla = 0;
		oldyrejilla = 0;
	} break;
	case 5: {
		if (currentRoomData->codigo != 6) {
			tipoefectofundido = Random(15) + 1;
			iframe = 0;
			indicetray = 0;
			characterPosX = currentRoomData->doors[indicepuertas].posxsalida - rectificacionx;
			characterPosY = currentRoomData->doors[indicepuertas].posysalida - rectificaciony + 15;
			trayec[indicetray].x = characterPosX;
			trayec[indicetray].y = characterPosY;
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			effect(tipoefectofundido, true, NULL);
			loadScreenData(currentRoomData->doors[indicepuertas].pantallaquecarga);
			stopVoc();
			autoPlayVoc("CALDERA", 6433, 15386);
			setSfxVolume(volumenfxizquierdo, 0);
			effect(tipoefectofundido, false, background);
			g_engine->_mouseManager->show();
			oldxrejilla = 0;
			oldyrejilla = 0;
			checkMouseGrid();
		} else {

			zonaactual = currentRoomData->rejapantalla[((characterPosX + rectificacionx) / factorx)][((characterPosY + rectificaciony) / factory)];
			zonadestino = 21;
			goToObject(zonaactual, zonadestino);
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			setSfxVolume(volumenfxizquierdo, 0);
			loadScrollData(currentRoomData->doors[indicepuertas].pantallaquecarga, true, 22, -2);
			g_engine->_mouseManager->show();
			oldxrejilla = 0;
			oldyrejilla = 0;
			checkMouseGrid();
		}
	} break;
	case 6: {
		zonaactual = currentRoomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory];
		zonadestino = 27;
		goToObject(zonaactual, zonadestino);
		freeAnimation();
		freeScreenObjects();
		g_engine->_mouseManager->hide();
		setSfxVolume(volumenfxizquierdo, volumenfxderecho);
		loadScrollData(currentRoomData->doors[indicepuertas].pantallaquecarga, false, 22, 2);
		g_engine->_mouseManager->show();
		oldxrejilla = 0;
		oldyrejilla = 0;
		checkMouseGrid();
	} break;
	case 9: {
		tipoefectofundido = Random(15) + 1;
		freeAnimation();
		freeScreenObjects();
		g_engine->_mouseManager->hide();
		effect(tipoefectofundido, true, NULL);
		iframe = 0;
		indicetray = 0;
		characterPosX = currentRoomData->doors[indicepuertas].posxsalida - rectificacionx;
		characterPosY = currentRoomData->doors[indicepuertas].posysalida - rectificaciony;
		trayec[indicetray].x = characterPosX;
		trayec[indicetray].y = characterPosY;
		loadScreenData(currentRoomData->doors[indicepuertas].pantallaquecarga);
		effect(tipoefectofundido, false, background);
		g_engine->_mouseManager->show();

		oldxrejilla = 0;
		oldyrejilla = 0;
		checkMouseGrid();
	} break;
	case 12: {
		if (currentRoomData->codigo != 13) {
			tipoefectofundido = Random(15) + 1;
			iframe = 0;
			indicetray = 0;
			characterPosX = currentRoomData->doors[indicepuertas].posxsalida - rectificacionx;
			characterPosY = currentRoomData->doors[indicepuertas].posysalida - rectificaciony;
			trayec[indicetray].x = characterPosX;
			trayec[indicetray].y = characterPosY;
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			effect(tipoefectofundido, true, NULL);
			loadScreenData(currentRoomData->doors[indicepuertas].pantallaquecarga);
			effect(tipoefectofundido, false, background);
			g_engine->_mouseManager->show();
			oldxrejilla = 0;
			oldyrejilla = 0;
			g_engine->_mouseManager->show();
		} else {

			zonaactual = currentRoomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory];
			goToObject(zonaactual, zonadestino);
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			loadScrollData(currentRoomData->doors[indicepuertas].pantallaquecarga, false, 64, 0);
			g_engine->_mouseManager->show();
			oldxrejilla = 0;
			oldyrejilla = 0;
			checkMouseGrid();
		}
	} break;
	case 13: {
		switch (currentRoomData->codigo) {
		case 12: {
			zonaactual = currentRoomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory];
			goToObject(zonaactual, zonadestino);
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			loadScrollData(currentRoomData->doors[indicepuertas].pantallaquecarga, true, 64, 0);
			g_engine->_mouseManager->show();
			oldxrejilla = 0;
			oldyrejilla = 0;
			checkMouseGrid();
		} break;
		case 14: {
			zonaactual = currentRoomData->rejapantalla[(characterPosX + rectificacionx) / factorx][(characterPosY + rectificaciony) / factory];
			goToObject(zonaactual, zonadestino);
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			loadScrollData(currentRoomData->doors[indicepuertas].pantallaquecarga, false, 56, 0);
			g_engine->_mouseManager->show();
			oldxrejilla = 0;
			oldyrejilla = 0;
			checkMouseGrid();
		} break;
		}
	} break;
	case 14: {
		if (currentRoomData->codigo != 13) {
			tipoefectofundido = Random(15) + 1;
			iframe = 0;
			indicetray = 0;
			characterPosX = currentRoomData->doors[indicepuertas].posxsalida - rectificacionx;
			characterPosY = currentRoomData->doors[indicepuertas].posysalida - rectificaciony;
			trayec[indicetray].x = characterPosX;
			trayec[indicetray].y = characterPosY;
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			effect(tipoefectofundido, true, NULL);
			loadScreenData(currentRoomData->doors[indicepuertas].pantallaquecarga);
			effect(tipoefectofundido, false, background);
			g_engine->_mouseManager->show();
			oldxrejilla = 0;
			oldyrejilla = 0;
			checkMouseGrid();
		} else {

			zonaactual = currentRoomData->rejapantalla[((characterPosX + rectificacionx) / factorx)][((characterPosY + rectificaciony) / factory)];
			goToObject(zonaactual, zonadestino);
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			loadScrollData(currentRoomData->doors[indicepuertas].pantallaquecarga, true, 56, 0);
			g_engine->_mouseManager->show();
			oldxrejilla = 0;
			oldyrejilla = 0;
			checkMouseGrid();
		}
	} break;
	case 17: {
		tipoefectofundido = Random(15) + 1;
		iframe = 0;
		indicetray = 0;
		characterPosX = currentRoomData->doors[indicepuertas].posxsalida - rectificacionx;
		characterPosY = currentRoomData->doors[indicepuertas].posysalida - rectificaciony;
		trayec[indicetray].x = characterPosX;
		trayec[indicetray].y = characterPosY;
		freeAnimation();
		freeScreenObjects();
		g_engine->_mouseManager->hide();
		effect(tipoefectofundido, true, NULL);
		stopVoc();
		loadScreenData(currentRoomData->doors[indicepuertas].pantallaquecarga);
		if (libro[0] == true && currentRoomData->banderamovimiento == true)
			disableSecondAnimation();
		if (contadorpc > 89)
			showError(274);
		setSfxVolume(volumenfxizquierdo, volumenfxderecho);
		effect(tipoefectofundido, false, background);
		contadorpc = contadorpc2;
		g_engine->_mouseManager->show();
		oldxrejilla = 0;
		oldyrejilla = 0;
		checkMouseGrid();
	} break;
	case 18: {
		if (currentRoomData->codigo != 19) {
			tipoefectofundido = Random(15) + 1;
			iframe = 0;
			indicetray = 0;
			characterPosX = currentRoomData->doors[indicepuertas].posxsalida - rectificacionx;
			characterPosY = currentRoomData->doors[indicepuertas].posysalida - rectificaciony;
			trayec[indicetray].x = characterPosX;
			trayec[indicetray].y = characterPosY;
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			effect(tipoefectofundido, true, NULL);
			loadScreenData(currentRoomData->doors[indicepuertas].pantallaquecarga);
			effect(tipoefectofundido, false, background);
			g_engine->_mouseManager->show();
			oldxrejilla = 0;
			oldyrejilla = 0;
			checkMouseGrid();
		} else {

			zonaactual = currentRoomData->rejapantalla[((characterPosX + rectificacionx) / factorx)][((characterPosY + rectificaciony) / factory)];
			goToObject(zonaactual, zonadestino);
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			loadScrollData(currentRoomData->doors[indicepuertas].pantallaquecarga, true, 131, -1);
			g_engine->_mouseManager->show();
			oldxrejilla = 0;
			oldyrejilla = 0;
			checkMouseGrid();
		}
	} break;
	case 19: {
		if (currentRoomData->codigo != 18) {
			tipoefectofundido = Random(15) + 1;
			iframe = 0;
			indicetray = 0;
			characterPosX = currentRoomData->doors[indicepuertas].posxsalida - rectificacionx;
			characterPosY = currentRoomData->doors[indicepuertas].posysalida - rectificaciony;
			trayec[indicetray].x = characterPosX;
			trayec[indicetray].y = characterPosY;
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			effect(tipoefectofundido, true, NULL);
			loadScreenData(currentRoomData->doors[indicepuertas].pantallaquecarga);
			effect(tipoefectofundido, false, background);
			g_engine->_mouseManager->show();
			oldxrejilla = 0;
			oldyrejilla = 0;
			checkMouseGrid();
		} else {

			zonaactual = currentRoomData->rejapantalla[((characterPosX + rectificacionx) / factorx)][((characterPosY + rectificaciony) / factory)];
			goToObject(zonaactual, zonadestino);
			freeAnimation();
			freeScreenObjects();
			g_engine->_mouseManager->hide();
			loadScrollData(currentRoomData->doors[indicepuertas].pantallaquecarga, false, 131, 1);
			g_engine->_mouseManager->show();
			oldxrejilla = 0;
			oldyrejilla = 0;
			checkMouseGrid();
		}
	} break;
	case 20: {
		tipoefectofundido = Random(15) + 1;
		iframe = 0;
		indicetray = 0;
		characterPosX = currentRoomData->doors[indicepuertas].posxsalida - rectificacionx;
		characterPosY = currentRoomData->doors[indicepuertas].posysalida - rectificaciony;
		trayec[indicetray].x = characterPosX;
		trayec[indicetray].y = characterPosY;
		freeAnimation();
		freeScreenObjects();
		g_engine->_mouseManager->hide();
		effect(tipoefectofundido, true, NULL);
		stopVoc();
		loadScreenData(currentRoomData->doors[indicepuertas].pantallaquecarga);
		switch (hornacina[0][hornacina[0][3]]) {
		case 0:
			currentRoomData->indexadoobjetos[9]->objectName = "HORNACINA";
			break;
		case 561:
			currentRoomData->indexadoobjetos[9]->objectName = "ESTATUA DIVINA";
			break;
		case 563:
			currentRoomData->indexadoobjetos[9]->objectName = "MANUAL DE ALFARERO";
			break;
		case 615:
			currentRoomData->indexadoobjetos[9]->objectName = "ESTATUA GROTESCA";
			break;
		}
		if (contadorpc > 89)
			showError(274);
		setSfxVolume(volumenfxizquierdo, volumenfxderecho);
		if (currentRoomData->codigo == 4)
			loadVoc("GOTA", 140972, 1029);
		effect(tipoefectofundido, false, background);
		contadorpc = contadorpc2;
		g_engine->_mouseManager->show();
		oldxrejilla = 0;
		oldyrejilla = 0;
		checkMouseGrid();
	} break;
	case 24: {
		tipoefectofundido = Random(15) + 1;
		iframe = 0;
		indicetray = 0;
		characterPosX = currentRoomData->doors[indicepuertas].posxsalida - rectificacionx;
		characterPosY = currentRoomData->doors[indicepuertas].posysalida - rectificaciony;
		trayec[indicetray].x = characterPosX;
		trayec[indicetray].y = characterPosY;
		freeAnimation();
		freeScreenObjects();
		g_engine->_mouseManager->hide();
		effect(tipoefectofundido, true, NULL);
		stopVoc();
		loadScreenData(currentRoomData->doors[indicepuertas].pantallaquecarga);
		switch (hornacina[1][hornacina[1][3]]) {
		case 0:
			currentRoomData->indexadoobjetos[8]->objectName = "HORNACINA";
			break;
		case 561:
			currentRoomData->indexadoobjetos[8]->objectName = "ESTATUA DIVINA";
			break;
		case 615:
			currentRoomData->indexadoobjetos[8]->objectName = "ESTATUA GROTESCA";
			break;
		case 622:
			currentRoomData->indexadoobjetos[8]->objectName = "PARED";
			break;
		case 623:
			currentRoomData->indexadoobjetos[8]->objectName = "TORNO";
			break;
		}
		if (contadorpc > 89)
			showError(274);
		setSfxVolume(volumenfxizquierdo, volumenfxderecho);
		if (trampa_puesta) {
			currentRoomData->banderamovimiento = true;
			loadAnimation(currentRoomData->nombremovto);
			iframe2 = 0;
			indicetray2 = 1;
			currentRoomData->tray2[indicetray2 - 1].x = 214 - 15;
			currentRoomData->tray2[indicetray2 - 1].y = 115 - 42;
			animado.dir = currentRoomData->dir2[indicetray2 - 1];
			animado.posx = currentRoomData->tray2[indicetray2 - 1].x;
			animado.posy = currentRoomData->tray2[indicetray2 - 1].y;
			animado.profundidad = 14;

			for (iaux = 0; iaux < maxrejax; iaux++)
				for (iaux2 = 0; iaux2 < maxrejay; iaux2++) {
					if (rejamascaramovto[iaux][iaux2] > 0) {
						currentRoomData->rejapantalla[oldposx + iaux][oldposy + iaux2] = rejamascaramovto[iaux][iaux2];
					}
					if (rejamascararaton[iaux][iaux2] > 0)
						currentRoomData->mouseGrid[oldposx + iaux][oldposy + iaux2] = rejamascararaton[iaux][iaux2];
				}
			assembleScreen();
		}
		effect(tipoefectofundido, false, background);
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
		tipoefectofundido = Random(15) + 1;
		iframe = 0;
		indicetray = 0;
		characterPosX = currentRoomData->doors[indicepuertas].posxsalida - rectificacionx;
		characterPosY = currentRoomData->doors[indicepuertas].posysalida - rectificaciony;
		trayec[indicetray].x = characterPosX;
		trayec[indicetray].y = characterPosY;
		freeAnimation();
		freeScreenObjects();
		g_engine->_mouseManager->hide();
		effect(tipoefectofundido, true, NULL);
		stopVoc();
		loadScreenData(currentRoomData->doors[indicepuertas].pantallaquecarga);
		if (contadorpc > 89)
			showError(274);
		setSfxVolume(volumenfxizquierdo, volumenfxderecho);
		switch (currentRoomData->codigo) {
		case 4:
			loadVoc("GOTA", 140972, 1029);
			break;
		case 23:
			autoPlayVoc("FUENTE", 0, 0);
			break;
		}
		effect(tipoefectofundido, false, background);
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
