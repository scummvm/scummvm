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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/config-manager.h"
#include "common/rect.h"
#include "tsage/graphics.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"
#include "tsage/ringworld2/ringworld2_logic.h"
#include "tsage/ringworld2/ringworld2_dialogs.h"
#include "tsage/ringworld2/ringworld2_scenes0.h"
#include "tsage/ringworld2/ringworld2_scenes1.h"
#include "tsage/ringworld2/ringworld2_scenes2.h"
#include "tsage/ringworld2/ringworld2_scenes3.h"
#include "tsage/ringworld2/ringworld2_airduct.h"
#include "tsage/ringworld2/ringworld2_outpost.h"
#include "tsage/ringworld2/ringworld2_vampire.h"

namespace TsAGE {

namespace Ringworld2 {

Scene *Ringworld2Game::createScene(int sceneNumber) {
	switch (sceneNumber) {
	/* Scene group #0 */
	case 50:
		// Waking up cutscene
		return new Scene50();
	case 100:
		// Quinn's room
		return new Scene100();
	case 125:
		// Computer console
		return new Scene125();
	case 150:
		// Empty Bedroom #1
		return new Scene150();
	case 160:
		// Credits
		return new Scene160();
	case 175:
		// Empty Bedroom #2
		return new Scene175();
	case 180:
		// Title Screen
		return new Scene180();
	case 200:
		// Deck #2 - By Lift
		return new Scene200();
	case 205:
		if (g_vm->getFeatures() & GF_DEMO)
			// End of Demo
			return new Scene205Demo();
		else
			// Star-field Credits
			return new Scene205();
	case 250:
		// Lift
		return new Scene250();
	case 300:
		// Bridge
		return new Scene300();
	case 325:
		// Bridge Console
		return new Scene325();
	case 400:
		// Science Lab
		return new Scene400();
	case 500:
		// Lander Bay 2 Storage
		return new Scene500();
	case 525:
		// Cutscene - Walking in hall
		return new Scene525();
	case 600:
		// Drive Room
		return new Scene600();
	case 700:
		// Lander Bay 2
		return new Scene700();
	case 800:
		// Sick bay
		return new Scene800();
	case 825:
		// Autodoc
		return new Scene825();
	case 850:
		// Deck #5 - By Lift
		return new Scene850();
	case 900:
		// Lander Bay 2 - Crane Controls
		return new Scene900();
	/* Scene group #1 */
	//
	case 1000:
		// Cutscene scene
		return new Scene1000();
	case 1010:
		// Cutscene - trip in space
		return new Scene1010();
	case 1020:
		// Cutscene - trip in space 2
		return new Scene1020();
	case 1100:
		// Canyon
		return new Scene1100();
	case 1200:
		// ARM Base - Air Ducts Maze
		return new Scene1200();
	case 1337:
	case 1330:
		// Card Game
		return new Scene1337();
	case 1500:
		// Cutscene: Ship landing
		return new Scene1500();
	case 1525:
		// Cutscene - Ship
		return new Scene1525();
	case 1530:
		// Cutscene - Crashing on Rimwall
		return new Scene1530();
	case 1550:
		// Spaceport
		return new Scene1550();
	case 1575:
		// Spaceport - unused ship scene
		return new Scene1575();
	case 1580:
		// Inside wreck
		return new Scene1580();
	case 1625:
		// Miranda being questioned
		return new Scene1625();
	case 1700:
		// Rim
		return new Scene1700();
	case 1750:
		// Rim Transport Vechile
		return new Scene1750();
	case 1800:
		// Rim Lift Exterior
		return new Scene1800();
	case 1850:
		// Rim Lift Interior
		return new Scene1850();
	case 1875:
		// Rim Lift Computer
		return new Scene1875();
	case 1900:
		// Spill Mountains Elevator Exit
		return new Scene1900();
	case 1925:
		// Spill Mountains Elevator Shaft
		return new Scene1925();
	case 1945:
		// Spill Mountains Shaft Bottom
		return new Scene1945();
	case 1950:
		// Flup Tube Corridor Maze
		return new Scene1950();
	/* Scene group #2 */
	case 2000:
		// Spill Mountains
		return new Scene2000();
	case 2350:
		// Spill Mountains: Balloon Launch Platform
		return new Scene2350();
	case 2400:
		// Spill Mountains: Unused large empty room
		return new Scene2400();
	case 2425:
		// Spill Mountains: The Hall of Records
		return new Scene2425();
	case 2430:
		// Spill Mountains: Bedroom
		return new Scene2430();
	case 2435:
		// Spill Mountains: Throne room
		return new Scene2435();
	case 2440:
		// Spill Mountains: Another bedroom
		return new Scene2440();
	case 2445:
		// Spill Mountains:
		return new Scene2445();
	case 2450:
		// Spill Mountains: Another bedroom
		return new Scene2450();
	case 2455:
		// Spill Mountains: Inside crevasse
		return new Scene2455();
	case 2500:
		// Spill Mountains: Large Ledge
		return new Scene2500();
	case 2525:
		// Spill Mountains: Furnace room
		return new Scene2525();
	case 2530:
		// Spill Mountains: Well
		return new Scene2530();
	case 2535:
		// Spill Mountains: Tannery
		return new Scene2535();
	case 2600:
		// Spill Mountains: Exit
		return new Scene2600();
	case 2700:
		// Outer Forest
		return new Scene2700();
	case 2750:
		// Inner Forest
		return new Scene2750();
	case 2800:
		// Guard post
		return new Scene2800();
	case 2900:
		// Balloon Cutscene
		return new Scene2900();

	/* Scene group #3 */
	// ARM Base Hanager
	case 3100:
		return new Scene3100();
	case 3125:
		// Ghouls dormitory
		return new Scene3125();
	case 3150:
		// Jail
		return new Scene3150();
	case 3175:
		// Autopsy room
		return new Scene3175();
	case 3200:
		// Cutscene : Guards - Discussion
		return new Scene3200();
	case 3210:
		// Cutscene : Captain and Private - Discussion
		return new Scene3210();
	case 3220:
		// Cutscene : Guards in cargo zone
		return new Scene3220();
	case 3230:
		// Cutscene : Guards on duty
		return new Scene3230();
	case 3240:
		// Cutscene : Teal monolog
		return new Scene3240();
	case 3245:
		// Cutscene : Discussions with Dr. Tomko
		return new Scene3245();
	case 3250:
		// Room with large stasis field negator
		return new Scene3250();
	case 3255:
		// Guard Post
		return new Scene3255();
	case 3260:
		// ARM Base - Computer room
		return new Scene3260();
	case 3275:
		// ARM Base - Hall
		return new Scene3275();
	case 3350:
		// Cutscene - Ship landing
		return new Scene3350();
	case 3375:
		// Circular Walkway
		return new Scene3375();
	case 3385:
		// Corridor
		return new Scene3385();
	case 3395:
		// Walkway
		return new Scene3395();
	case 3400:
		// Confrontation
		return new Scene3400();
	case 3500:
		// Flub tube maze
		return new Scene3500();
	case 3600:
		// Cutscene - walking at gunpoint
		return new Scene3600();
	case 3700:
		// Cutscene - Teleport outside
		return new Scene3700();
	case 3800:
		// Desert
		return new Scene3800();
	case 3900:
		// Forest Entrance
		return new Scene3900();
	default:
		error("Unknown scene number - %d", sceneNumber);
		break;
	}
}

/**
 * Returns true if it is currently okay to restore a game
 */
bool Ringworld2Game::canLoadGameStateCurrently() {
	// Don't allow a game to be loaded if a dialog is active
	return g_globals->_gfxManagers.size() == 1;
}

/**
 * Returns true if it is currently okay to save the game
 */
bool Ringworld2Game::canSaveGameStateCurrently() {
	// Don't allow a game to be saved if a dialog is active, or if an animation
	// is playing, or if an active scene prevents it
	return g_globals->_gfxManagers.size() == 1 && R2_GLOBALS._animationCtr == 0 &&
		(!R2_GLOBALS._sceneManager._scene ||
		!((SceneExt *)R2_GLOBALS._sceneManager._scene)->_preventSaving);
}

/*--------------------------------------------------------------------------*/

SceneExt::SceneExt(): Scene() {
	_stripManager._onBegin = SceneExt::startStrip;
	_stripManager._onEnd = SceneExt::endStrip;

	for (int i = 0; i < 256; i++)
		_shadowPaletteMap[i] = 0;

	_savedPlayerEnabled = false;
	_savedUiEnabled = false;
	_savedCanWalk = false;
	_preventSaving = false;

	// Reset screen clipping area
	R2_GLOBALS._screen._clipRect = Rect();

	// WORKAROUND: In the original, playing animations don't reset the global _animationCtr
	// counter as scene changes unless the playing animation explicitly finishes. For now,
	// to make inter-scene debugging easier, I'm explicitly resetting the _animationCtr
	// on scene start, since scene objects aren't drawn while it's non-zero
	R2_GLOBALS._animationCtr = 0;

	// WORKAROUND: We had a case where at some point the number of modal dialogs
	// open became incorrect. So reset it on scene changes to fix the problem if
	// it ever happens
	R2_GLOBALS._insetUp = 0;
}

void SceneExt::synchronize(Serializer &s) {
	Scene::synchronize(s);

	s.syncBytes(&_shadowPaletteMap[0], 256);
	_sceneAreas.synchronize(s);
}

void SceneExt::postInit(SceneObjectList *OwnerList) {
	Scene::postInit(OwnerList);

	// Exclude the bottom area of the screen to allow room for the UI
	T2_GLOBALS._interfaceY = UI_INTERFACE_Y;

	// Initialize fields
	_action = NULL;
	_sceneMode = 0;

	static_cast<SceneHandlerExt *>(R2_GLOBALS._sceneHandler)->setupPaletteMaps();

	int prevScene = R2_GLOBALS._sceneManager._previousScene;
	int sceneNumber = R2_GLOBALS._sceneManager._sceneNumber;
	if (g_vm->getFeatures() & GF_DEMO) {
		if (prevScene == 0 && sceneNumber == 180) {
			// Very start of the demo, title & intro about to be shown
			R2_GLOBALS._uiElements._active = false;
			R2_GLOBALS._uiElements.hide();
		} else if (((prevScene == -1) && (sceneNumber != 180) && (sceneNumber != 205) && (sceneNumber != 50))
			|| (prevScene == 0) || (sceneNumber == 600)
			|| ((prevScene == 205 || prevScene == 180 || prevScene == 50) && (sceneNumber == 100))) {
				R2_GLOBALS._uiElements._active = true;
				R2_GLOBALS._uiElements.show();
		} else {
			R2_GLOBALS._uiElements.updateInventory();
		}
	} else if (((prevScene == -1) && (sceneNumber != 180) && (sceneNumber != 205) && (sceneNumber != 50))
			|| (sceneNumber == 50)
			|| ((sceneNumber == 100) && (prevScene == 0 || prevScene == 180 || prevScene == 205))) {
		R2_GLOBALS._uiElements._active = true;
		R2_GLOBALS._uiElements.show();
	} else {
		R2_GLOBALS._uiElements.updateInventory();
	}
}

void SceneExt::remove() {
	_sceneAreas.clear();
	Scene::remove();
	R2_GLOBALS._uiElements._active = true;

	if (R2_GLOBALS._events.getCursor() >= EXITCURSOR_N &&
			R2_GLOBALS._events.getCursor() <= SHADECURSOR_DOWN)
		R2_GLOBALS._events.setCursor(CURSOR_WALK);
}

void SceneExt::process(Event &event) {
	if (!event.handled)
		Scene::process(event);
}

void SceneExt::dispatch() {
	Scene::dispatch();
}

bool SceneExt::display(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_CROSSHAIRS:
	case CURSOR_WALK:
		return false;
	case CURSOR_LOOK:
		SceneItem::display2(1, R2_GLOBALS._randomSource.getRandomNumber(4));
		break;
	case CURSOR_USE:
		SceneItem::display2(1, R2_GLOBALS._randomSource.getRandomNumber(4) + 5);
		break;
	case CURSOR_TALK:
		SceneItem::display2(1, R2_GLOBALS._randomSource.getRandomNumber(4) + 10);
		break;
	case R2_NEGATOR_GUN:
		if (R2_GLOBALS.getFlag(1))
			SceneItem::display2(2, action);
		else
			SceneItem::display2(5, 0);
		break;
	case R2_SONIC_STUNNER:
		if ((R2_GLOBALS._scannerFrequencies[R2_QUINN] == 2)
			|| ((R2_GLOBALS._scannerFrequencies[R2_QUINN] == 1) &&
				(R2_GLOBALS._scannerFrequencies[R2_SEEKER] == 2) &&
				(R2_GLOBALS._sceneManager._previousScene == 300))) {
			R2_GLOBALS._sound4.stop();
			R2_GLOBALS._sound3.play(46);
			SceneItem::display2(5, 15);

			R2_GLOBALS._sound4.play(45);
		} else {
			R2_GLOBALS._sound3.play(43, 0);
			SceneItem::display2(2, R2_SONIC_STUNNER);
		}
		break;
	case R2_COM_SCANNER:
	case R2_COM_SCANNER_2:
		R2_GLOBALS._sound3.play(44);
		SceneItem::display2(2, action);
		R2_GLOBALS._sound3.stop();
		break;
	case R2_PHOTON_STUNNER:
		R2_GLOBALS._sound3.play(99);
		SceneItem::display2(2, action);
		break;
	default:
		SceneItem::display2(2, action);
		break;
	}

	event.handled = true;
	return true;
}

void SceneExt::fadeOut() {
	uint32 black = 0;
	R2_GLOBALS._scenePalette.fade((const byte *)&black, false, 100);
}

void SceneExt::startStrip() {
	SceneExt *scene = (SceneExt *)R2_GLOBALS._sceneManager._scene;
	scene->_savedPlayerEnabled = R2_GLOBALS._player._enabled;

	if (scene->_savedPlayerEnabled) {
		scene->_savedUiEnabled = R2_GLOBALS._player._uiEnabled;
		scene->_savedCanWalk = R2_GLOBALS._player._canWalk;
		R2_GLOBALS._player.disableControl();
/*
		if (!R2_GLOBALS._v50696 && R2_GLOBALS._uiElements._active)
			R2_GLOBALS._uiElements.hide();
*/
	}
}

void SceneExt::endStrip() {
	SceneExt *scene = (SceneExt *)R2_GLOBALS._sceneManager._scene;

	if (scene->_savedPlayerEnabled) {
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._uiEnabled = scene->_savedUiEnabled;
		R2_GLOBALS._player._canWalk = scene->_savedCanWalk;
/*
		if (!R2_GLOBALS._v50696 && R2_GLOBALS._uiElements._active)
			R2_GLOBALS._uiElements.show();
*/
	}
}

void SceneExt::clearScreen() {
	R2_GLOBALS._screen.fillRect(R2_GLOBALS._screen.getBounds(), 0);
}

void SceneExt::refreshBackground(int xAmount, int yAmount) {
	switch (_activeScreenNumber) {
	case 700:
	case 1020:
	case 1100:
	case 1700:
	case 2600:
	case 2950:
	case 3100:
	case 3101:
	case 3275:
	case 3600:
		// Use traditional style sectioned screen loading
		Scene::refreshBackground(xAmount, yAmount);
		return;
	default:
		// Break out to new style screen loading
		break;
	}

	/* New style background loading */

	// Get the screen data
	byte *dataP = g_resourceManager->getResource(RT18, _activeScreenNumber, 0);
	int screenSize = g_vm->_memoryManager.getSize(dataP);

	// Lock the background for update
	assert(screenSize == (_backSurface.w * _backSurface.h));
	Graphics::Surface s = _backSurface.getSubArea(Common::Rect(0, 0, _backSurface.w, _backSurface.h));

	// Copy the data into the surface
	byte *destP = (byte *)s.getPixels();
	Common::copy(dataP, dataP + (s.w * s.h), destP);

	// Free the resource data
	DEALLOCATE(dataP);
}

/**
 * Saves the current player position and view in the details for the specified character index
 */
void SceneExt::saveCharacter(int characterIndex) {
	R2_GLOBALS._player._characterPos[characterIndex] = R2_GLOBALS._player._position;
	R2_GLOBALS._player._characterStrip[characterIndex] = R2_GLOBALS._player._strip;
	R2_GLOBALS._player._characterFrame[characterIndex] = R2_GLOBALS._player._frame;
}

void SceneExt::scalePalette(int RFactor, int GFactor, int BFactor) {
	byte *tmpPal = R2_GLOBALS._scenePalette._palette;
	byte newR, newG, newB;
	int tmp, varD = 0;

	for (int i = 0; i < 256; i++) {
		newR = (RFactor * tmpPal[(3 * i)]) / 100;
		newG = (GFactor * tmpPal[(3 * i) + 1]) / 100;
		newB = (BFactor * tmpPal[(3 * i) + 2]) / 100;

		int varC = 769;
		for (int j = 255; j >= 0; j--) {
			tmp = abs(tmpPal[(3 * j)] - newR);
			if (tmp >= varC)
				continue;

			tmp += abs(tmpPal[(3 * j) + 1] - newG);
			if (tmp >= varC)
				continue;

			tmp += abs(tmpPal[(3 * j) + 2] - newB);
			if (tmp >= varC)
				continue;

			varC = tmp;
			varD = j;
		}
		this->_shadowPaletteMap[i] = varD;
	}
}

void SceneExt::loadBlankScene() {
	_backSurface.create(SCREEN_WIDTH, SCREEN_HEIGHT * 3 / 2);
	_backSurface.fillRect(_backSurface.getBounds(), 0);

	R2_GLOBALS._screen.fillRect(R2_GLOBALS._screen.getBounds(), 0);
}

/*--------------------------------------------------------------------------*/

void SceneHandlerExt::postInit(SceneObjectList *OwnerList) {
	SceneHandler::postInit(OwnerList);

	if (!R2_GLOBALS._playStream.setFile("SND4K.RES"))
		warning("Could not find SND4K.RES voice file");
}

void SceneHandlerExt::process(Event &event) {
	if (T2_GLOBALS._uiElements._active && R2_GLOBALS._player._uiEnabled) {
		T2_GLOBALS._uiElements.process(event);
		if (event.handled)
			return;
	}

	SceneExt *scene = static_cast<SceneExt *>(R2_GLOBALS._sceneManager._scene);
	if (scene && R2_GLOBALS._player._uiEnabled) {
		// Handle any scene areas that have been registered
		SynchronizedList<EventHandler *>::iterator saIter;
		for (saIter = scene->_sceneAreas.begin(); saIter != scene->_sceneAreas.end() && !event.handled; ++saIter) {
			(*saIter)->process(event);
		}
	}

	if (!event.handled)
		SceneHandler::process(event);
}

void SceneHandlerExt::dispatch() {
	R2_GLOBALS._playStream.dispatch();
	SceneHandler::dispatch();
}

void SceneHandlerExt::postLoad(int priorSceneBeforeLoad, int currentSceneBeforeLoad) {
	// Set up the shading maps used for showing the player in shadows
	setupPaletteMaps();

	if (currentSceneBeforeLoad == 2900) {
		R2_GLOBALS._gfxFontNumber = 50;
		R2_GLOBALS._gfxColors.background = 0;
		R2_GLOBALS._gfxColors.foreground = 59;
		R2_GLOBALS._fontColors.background = 4;
		R2_GLOBALS._fontColors.foreground = 15;
		R2_GLOBALS._frameEdgeColor = 2;

		R2_GLOBALS._scenePalette.loadPalette(0);
		R2_GLOBALS._scenePalette.setEntry(255, 0xff, 0xff, 0xff);
		R2_GLOBALS._fadePaletteFlag = false;
		setupPaletteMaps();
	}
}

void SceneHandlerExt::setupPaletteMaps() {
	byte *palP = &R2_GLOBALS._scenePalette._palette[0];

	// Set up the mapping table for giving faded versions of pixels at different fade percentages
	if (!R2_GLOBALS._fadePaletteFlag) {
		R2_GLOBALS._fadePaletteFlag = true;

		for (int idx = 0; idx < 10; ++idx) {
			for (int palIndex = 0; palIndex < 224; ++palIndex) {
				int r, g, b;

				// Get adjusted RGB values
				switch (idx) {
				case 7:
					r = palP[palIndex * 3] * 85 / 100;
					g = palP[palIndex * 3 + 1] * 7 / 10;
					b = palP[palIndex * 3 + 2] * 7 / 10;
					break;
				case 8:
					r = palP[palIndex * 3] * 7 / 10;
					g = palP[palIndex * 3 + 1] * 85 / 100;
					b = palP[palIndex * 3 + 2] * 7 / 10;
					break;
				case 9:
					r = palP[palIndex * 3] * 8 / 10;
					g = palP[palIndex * 3 + 1] * 5 / 10;
					b = palP[palIndex * 3 + 2] * 9 / 10;
					break;
				default:
					r = palP[palIndex * 3] * (10 - idx) / 10;
					g = palP[palIndex * 3 + 1] * (10 - idx) / 12;
					b = palP[palIndex * 3 + 2] * (10 - idx) / 10;
					break;
				}

				// Scan for the palette index with the closest matching color
				int threshold = 769;
				int foundIndex = -1;
				for (int pIndex2 = 223; pIndex2 >= 0; --pIndex2) {
					int diffSum = ABS(palP[pIndex2 * 3] - r);
					if (diffSum >= threshold)
						continue;

					diffSum += ABS(palP[pIndex2 * 3 + 1] - g);
					if (diffSum >= threshold)
						continue;

					diffSum += ABS(palP[pIndex2 * 3 + 2] - b);
					if (diffSum >= threshold)
						continue;

					threshold = diffSum;
					foundIndex = pIndex2;
				}

				R2_GLOBALS._fadePaletteMap[idx][palIndex] = foundIndex;
			}
		}
	}

	for (int palIndex = 0; palIndex < 224; ++palIndex) {
		int r = palP[palIndex * 3] >> 4;
		int g = palP[palIndex * 3 + 1] >> 4;
		int b = palP[palIndex * 3 + 2] >> 4;

		int v = (r << 8) | (g << 4) | b;
		assert(v < 0x1000);
		R2_GLOBALS._paletteMap[v] = palIndex;
	}

	int vdx = 0;
	int idx = 0;
	int palIndex = 224;

	for (int vIndex = 0; vIndex < 4096; ++vIndex) {
		int v = R2_GLOBALS._paletteMap[vIndex];
		if (!v) {
			R2_GLOBALS._paletteMap[vIndex] = idx;
		} else {
			idx = v;
		}

		if (!palIndex) {
			vdx = palIndex;
		} else {
			int idxTemp = palIndex;
			palIndex = (palIndex + vdx) / 2;
			vdx = idxTemp;
		}
	}
}

/*--------------------------------------------------------------------------*/

DisplayHotspot::DisplayHotspot(int regionId, ...) {
	_sceneRegionId = regionId;

	// Load up the actions
	va_list va;
	va_start(va, regionId);

	int param = va_arg(va, int);
	while (param != LIST_END) {
		_actions.push_back(param);
		param = va_arg(va, int);
	}

	va_end(va);
}

bool DisplayHotspot::performAction(int action) {
	for (uint i = 0; i < _actions.size(); i += 3) {
		if (_actions[i] == action) {
			display(_actions[i + 1], _actions[i + 2], SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
			return true;
		}
	}

	return false;
}

/*--------------------------------------------------------------------------*/

DisplayObject::DisplayObject(int firstAction, ...) {
	// Load up the actions
	va_list va;
	va_start(va, firstAction);

	int param = firstAction;
	while (param != LIST_END) {
		_actions.push_back(param);
		param = va_arg(va, int);
	}

	va_end(va);
}

bool DisplayObject::performAction(int action) {
	for (uint i = 0; i < _actions.size(); i += 3) {
		if (_actions[i] == action) {
			display(_actions[i + 1], _actions[i + 2], SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
			return true;
		}
	}

	return false;
}

/*--------------------------------------------------------------------------*/

Ringworld2InvObjectList::Ringworld2InvObjectList():
		_none(1, 1),
		_optoDisk(1, 2),
		_reader(1, 3),
		_negatorGun(1, 4),
		_steppingDisks(1, 5),
		_attractorUnit(1, 6),
		_sensorProbe(1, 7),
		_sonicStunner(1, 8),
		_cableHarness(1, 9),
		_comScanner(1, 10),
		_spentPowerCapsule(1, 11),		// 10
		_chargedPowerCapsule(1, 12),
		_aerosol(1, 13),
		_remoteControl(1, 14),
		_opticalFiber(1, 15),
		_clamp(1, 16),
		_attractorHarness(1, 17),
		_fuelCell(2, 2),
		_gyroscope(2, 3),
		_airbag(2, 4),
		_rebreatherTank(2, 5),			// 20
		_reserveTank(2, 5),
		_guidanceModule(2, 6),
		_thrusterValve(2, 7),
		_balloonBackpack(2, 8),
		_radarMechanism(2, 9),
		_joystick(2, 10),
		_ignitor(2, 11),
		_diagnosticsDisplay(2, 12),
		_glassDome(2, 13),
		_wickLamp(2, 14),				// 30
		_scrithKey(2, 15),
		_tannerMask(2, 16),
		_pureGrainAlcohol(3, 2),
		_blueSapphire(3, 3),
		_ancientScrolls(3, 4),
		_flute(3, 5),
		_gunpowder(3, 6),
		_unused(3, 7),
		_comScanner2(1, 10),
		_superconductorWire(3, 8),		// 40
		_pillow(3, 9),
		_foodTray(3, 10),
		_laserHacksaw(3, 11),
		_photonStunner(3, 12),
		_battery(3, 13),
		_soakedFaceMask(2, 17),
		_lightBulb(3, 14),
		_alcoholLamp1(2, 14),
		_alcoholLamp2(3, 15),
		_alocholLamp3(3, 15),			// 50
		_brokenDisplay(3, 17),
		_toolbox(4, 2) {

	// Add the items to the list
	_itemList.push_back(&_none);
	_itemList.push_back(&_optoDisk);
	_itemList.push_back(&_reader);
	_itemList.push_back(&_negatorGun);
	_itemList.push_back(&_steppingDisks);
	_itemList.push_back(&_attractorUnit);
	_itemList.push_back(&_sensorProbe);
	_itemList.push_back(&_sonicStunner);
	_itemList.push_back(&_cableHarness);
	_itemList.push_back(&_comScanner);
	_itemList.push_back(&_spentPowerCapsule);	// 10
	_itemList.push_back(&_chargedPowerCapsule);
	_itemList.push_back(&_aerosol);
	_itemList.push_back(&_remoteControl);
	_itemList.push_back(&_opticalFiber);
	_itemList.push_back(&_clamp);
	_itemList.push_back(&_attractorHarness);
	_itemList.push_back(&_fuelCell);
	_itemList.push_back(&_gyroscope);
	_itemList.push_back(&_airbag);
	_itemList.push_back(&_rebreatherTank);		// 20
	_itemList.push_back(&_reserveTank);
	_itemList.push_back(&_guidanceModule);
	_itemList.push_back(&_thrusterValve);
	_itemList.push_back(&_balloonBackpack);
	_itemList.push_back(&_radarMechanism);
	_itemList.push_back(&_joystick);
	_itemList.push_back(&_ignitor);
	_itemList.push_back(&_diagnosticsDisplay);
	_itemList.push_back(&_glassDome);
	_itemList.push_back(&_wickLamp);			// 30
	_itemList.push_back(&_scrithKey);
	_itemList.push_back(&_tannerMask);
	_itemList.push_back(&_pureGrainAlcohol);
	_itemList.push_back(&_blueSapphire);
	_itemList.push_back(&_ancientScrolls);
	_itemList.push_back(&_flute);
	_itemList.push_back(&_gunpowder);
	_itemList.push_back(&_unused);
	_itemList.push_back(&_comScanner2);
	_itemList.push_back(&_superconductorWire);	// 40
	_itemList.push_back(&_pillow);
	_itemList.push_back(&_foodTray);
	_itemList.push_back(&_laserHacksaw);
	_itemList.push_back(&_photonStunner);
	_itemList.push_back(&_battery);
	_itemList.push_back(&_soakedFaceMask);
	_itemList.push_back(&_lightBulb);
	_itemList.push_back(&_alcoholLamp1);
	_itemList.push_back(&_alcoholLamp2);
	_itemList.push_back(&_alocholLamp3);		// 50
	_itemList.push_back(&_brokenDisplay);
	_itemList.push_back(&_toolbox);

	_selectedItem = NULL;
}

void Ringworld2InvObjectList::reset() {
	// Reset all object scene numbers
	SynchronizedList<InvObject *>::iterator i;
	for (i = _itemList.begin(); i != _itemList.end(); ++i) {
		(*i)->_sceneNumber = 0;
	}

	// Set up default inventory
	setObjectScene(R2_OPTO_DISK, 800);
	setObjectScene(R2_READER, 400);
	setObjectScene(R2_NEGATOR_GUN, 100);
	setObjectScene(R2_STEPPING_DISKS, 100);
	setObjectScene(R2_ATTRACTOR_UNIT, 400);
	setObjectScene(R2_SENSOR_PROBE, 400);
	setObjectScene(R2_SONIC_STUNNER, 500);
	setObjectScene(R2_CABLE_HARNESS, 700);
	setObjectScene(R2_COM_SCANNER, 800);
	setObjectScene(R2_SPENT_POWER_CAPSULE, 100);
	setObjectScene(R2_CHARGED_POWER_CAPSULE, 400);
	setObjectScene(R2_AEROSOL, 500);
	setObjectScene(R2_REMOTE_CONTROL, 1550);
	setObjectScene(R2_OPTICAL_FIBER, 850);
	setObjectScene(R2_CLAMP, 850);
	setObjectScene(R2_ATTRACTOR_CABLE_HARNESS, 0);
	setObjectScene(R2_FUEL_CELL, 1550);
	setObjectScene(R2_GYROSCOPE, 1550);
	setObjectScene(R2_AIRBAG, 1550);
	setObjectScene(R2_REBREATHER_TANK, 500);
	setObjectScene(R2_RESERVE_REBREATHER_TANK, 500);
	setObjectScene(R2_GUIDANCE_MODULE, 1550);
	setObjectScene(R2_THRUSTER_VALVE, 1580);
	setObjectScene(R2_BALLOON_BACKPACK, 9999);
	setObjectScene(R2_RADAR_MECHANISM, 1550);
	setObjectScene(R2_JOYSTICK, 1550);
	setObjectScene(R2_IGNITOR, 1580);
	setObjectScene(R2_DIAGNOSTICS_DISPLAY, 1550);
	setObjectScene(R2_GLASS_DOME, 2525);
	setObjectScene(R2_WICK_LAMP, 2440);
	setObjectScene(R2_SCRITH_KEY, 2455);
	setObjectScene(R2_TANNER_MASK, 2535);
	setObjectScene(R2_PURE_GRAIN_ALCOHOL, 2530);
	setObjectScene(R2_SAPPHIRE_BLUE, 1950);
	setObjectScene(R2_ANCIENT_SCROLLS, 1950);
	setObjectScene(R2_FLUTE, 9999);
	setObjectScene(R2_GUNPOWDER, 2430);
	setObjectScene(R2_NONAME, 9999);
	setObjectScene(R2_COM_SCANNER_2, 2);
	setObjectScene(R2_SUPERCONDUCTOR_WIRE, 9999);
	setObjectScene(R2_PILLOW, 3150);
	setObjectScene(R2_FOOD_TRAY, 0);
	setObjectScene(R2_LASER_HACKSAW, 3260);
	setObjectScene(R2_PHOTON_STUNNER, 2);
	setObjectScene(R2_BATTERY, 1550);
	setObjectScene(R2_SOAKED_FACEMASK, 0);
	setObjectScene(R2_LIGHT_BULB, 3150);
	setObjectScene(R2_ALCOHOL_LAMP, 2435);
	setObjectScene(R2_ALCOHOL_LAMP_2, 2440);
	setObjectScene(R2_ALCOHOL_LAMP_3, 2435);
	setObjectScene(R2_BROKEN_DISPLAY, 1580);
	setObjectScene(R2_TOOLBOX, 3260);

	// Set up the select item handler method
	T2_GLOBALS._onSelectItem = SelectItem;
}

void Ringworld2InvObjectList::setObjectScene(int objectNum, int sceneNumber) {
	// Find the appropriate object
	int num = objectNum;
	SynchronizedList<InvObject *>::iterator i = _itemList.begin();
	while (num-- > 0) ++i;
	(*i)->_sceneNumber = sceneNumber;

	// If the item is the currently active one, default back to the use cursor
	if (R2_GLOBALS._events.getCursor() == objectNum)
		R2_GLOBALS._events.setCursor(CURSOR_USE);

	// Update the user interface if necessary
	T2_GLOBALS._uiElements.updateInventory(
		(sceneNumber == R2_GLOBALS._player._characterIndex) ? objectNum : 0);
}

/**
 * When an inventory item is selected, in Return to Ringworld two objects can be combined
 */
bool Ringworld2InvObjectList::SelectItem(int objectNumber) {
	// If no existing item selected, don't go any further
	int currentItem = R2_GLOBALS._events.getCursor();
	if (currentItem >= 256)
		return false;

	switch (objectNumber) {
	case R2_NEGATOR_GUN:
		switch (currentItem) {
		case R2_SENSOR_PROBE:
			if (R2_GLOBALS.getFlag(1))
				SceneItem::display2(5, 1);
			else if (R2_INVENTORY.getObjectScene(R2_SPENT_POWER_CAPSULE) != 100)
				SceneItem::display2(5, 3);
			else {
				R2_GLOBALS._sound3.play(48);
				SceneItem::display2(5, 2);
				R2_INVENTORY.setObjectScene(R2_SPENT_POWER_CAPSULE, 1);
			}
			break;
		case R2_COM_SCANNER:
			R2_GLOBALS._sound3.play(44);
			if (R2_GLOBALS.getFlag(1))
				SceneItem::display2(5, 9);
			else if (R2_INVENTORY.getObjectScene(R2_SPENT_POWER_CAPSULE) == 100)
				SceneItem::display2(5, 8);
			else
				SceneItem::display2(5, 10);

			R2_GLOBALS._sound3.stop();
			break;
		case R2_CHARGED_POWER_CAPSULE:
			if (R2_INVENTORY.getObjectScene(R2_SPENT_POWER_CAPSULE) == 1) {
				R2_GLOBALS._sound3.play(49);
				R2_INVENTORY.setObjectScene(R2_CHARGED_POWER_CAPSULE, 100);
				R2_GLOBALS.setFlag(1);
				SceneItem::display2(5, 4);
			} else {
				SceneItem::display2(5, 5);
			}
			break;
		default:
			selectDefault(objectNumber);
			break;
		}
		break;
	case R2_STEPPING_DISKS:
		switch (currentItem) {
		case R2_SENSOR_PROBE:
			if (R2_INVENTORY.getObjectScene(R2_CHARGED_POWER_CAPSULE) == 400) {
				R2_GLOBALS._sound3.play(48);
				SceneItem::display2(5, 6);
				R2_INVENTORY.setObjectScene(R2_CHARGED_POWER_CAPSULE, 1);
			} else {
				SceneItem::display2(5, 7);
			}
			break;
		case R2_COM_SCANNER:
			R2_GLOBALS._sound3.play(44);
			if (R2_INVENTORY.getObjectScene(R2_CHARGED_POWER_CAPSULE) == 400)
				SceneItem::display2(5, 16);
			else
				SceneItem::display2(5, 17);
			R2_GLOBALS._sound3.stop();
			break;
		default:
			selectDefault(objectNumber);
			break;
		}
		break;
	case R2_ATTRACTOR_UNIT:
	case R2_CABLE_HARNESS:
		if (currentItem == R2_CABLE_HARNESS ||
				currentItem == R2_ATTRACTOR_UNIT) {
			R2_INVENTORY.setObjectScene(R2_CABLE_HARNESS, 0);
			R2_INVENTORY.setObjectScene(R2_ATTRACTOR_UNIT, 0);
			R2_INVENTORY.setObjectScene(R2_ATTRACTOR_CABLE_HARNESS, 1);
		} else {
			selectDefault(objectNumber);
		}
		break;
	case R2_TANNER_MASK:
	case R2_PURE_GRAIN_ALCOHOL:
		if (currentItem == R2_TANNER_MASK ||
				currentItem == R2_PURE_GRAIN_ALCOHOL) {
			R2_INVENTORY.setObjectScene(R2_TANNER_MASK, 0);
			R2_INVENTORY.setObjectScene(R2_PURE_GRAIN_ALCOHOL, 0);
			R2_INVENTORY.setObjectScene(R2_SOAKED_FACEMASK, R2_SEEKER);
		} else {
			selectDefault(objectNumber);
		}
		break;
	default:
		// Standard item selection
		return false;
	}

	return true;
}

void Ringworld2InvObjectList::selectDefault(int objectNumber) {
	Common::String msg1 = g_resourceManager->getMessage(4, 53);
	Common::String msg2 = g_resourceManager->getMessage(4, R2_GLOBALS._events.getCursor());
	Common::String msg3 = g_resourceManager->getMessage(4, 54);
	Common::String msg4 = g_resourceManager->getMessage(4, objectNumber);
	Common::String line = Common::String::format("%.5s%.5s%.5s%.5s%s %s %s %s.",
		msg1.c_str(), msg2.c_str(), msg3.c_str(), msg4.c_str(),
		msg1.c_str() + 5, msg2.c_str() + 5, msg3.c_str() + 5, msg4.c_str() + 5);

	SceneItem::display(-1, -1, line.c_str(),
		SET_WIDTH, 280,
		SET_X, 160,
		SET_Y, 20,
		SET_POS_MODE, 1,
		SET_EXT_BGCOLOR, 7,
		LIST_END);
}

/*--------------------------------------------------------------------------*/

void Ringworld2Game::start() {
	int slot = -1;

	if (ConfMan.hasKey("save_slot")) {
		slot = ConfMan.getInt("save_slot");
		Common::String file = g_vm->getSaveStateName(slot);
		Common::InSaveFile *in = g_vm->_system->getSavefileManager()->openForLoading(file);
		if (in)
			delete in;
		else
			slot = -1;
	}

	if (slot >= 0)
		R2_GLOBALS._sceneHandler->_loadGameSlot = slot;
	else {
		// Switch to the first title screen
		R2_GLOBALS._uiElements._active = true;
		R2_GLOBALS._sceneManager.setNewScene(180);
	}

	g_globals->_events.showCursor();
}

void Ringworld2Game::restartGame() {
	if (MessageDialog::show(Ringworld2::R2_RESTART_MSG, CANCEL_BTN_STRING, YES_MSG) == 1)
		restart();
}

void Ringworld2Game::restart() {
	g_globals->_scenePalette.clearListeners();
	g_globals->_soundHandler.stop();

	// Reset the globals
	g_globals->reset();

	// Clear save/load slots
	g_globals->_sceneHandler->_saveGameSlot = -1;
	g_globals->_sceneHandler->_loadGameSlot = -1;

	// Change to the first game scene
	g_globals->_sceneManager.changeScene(100);
}

void Ringworld2Game::endGame(int resNum, int lineNum) {
	g_globals->_events.setCursor(CURSOR_WALK);
	Common::String msg = g_resourceManager->getMessage(resNum, lineNum);
	bool savesExist = g_saver->savegamesExist();

	if (!savesExist) {
		// No savegames exist, so prompt the user to restart or quit
		if (MessageDialog::show(msg, QUIT_BTN_STRING, RESTART_BTN_STRING) == 0)
			g_vm->quitGame();
		else
			restart();
	} else {
		// Savegames exist, so prompt for Restore/Restart
		bool breakFlag;
		do {
			if (g_vm->shouldQuit()) {
				breakFlag = true;
			} else if (MessageDialog::show(msg, RESTART_BTN_STRING, RESTORE_BTN_STRING) == 0) {
				restart();
				breakFlag = true;
			} else {
				handleSaveLoad(false, g_globals->_sceneHandler->_loadGameSlot, g_globals->_sceneHandler->_saveName);
				breakFlag = g_globals->_sceneHandler->_loadGameSlot >= 0;
			}
		} while (!breakFlag);
	}

	g_globals->_events.setCursorFromFlag();
}

void Ringworld2Game::processEvent(Event &event) {
	if (event.eventType == EVENT_KEYPRESS) {
		switch (event.kbd.keycode) {
		case Common::KEYCODE_F1:
			// F1 - Help
			HelpDialog::show();
			break;

		case Common::KEYCODE_F2:
			// F2 - Sound Options
			SoundDialog::execute();
			break;

		case Common::KEYCODE_F3:
			// F3 - Quit
			quitGame();
			event.handled = false;
			break;

		case Common::KEYCODE_F4:
			// F4 - Restart
			restartGame();
			R2_GLOBALS._events.setCursorFromFlag();
			break;

		case Common::KEYCODE_F5:
			// F5 - Save
			saveGame();
			R2_GLOBALS._events.setCursorFromFlag();
			event.handled = true;
			break;

		case Common::KEYCODE_F7:
			// F7 - Restore
			restoreGame();
			R2_GLOBALS._events.setCursorFromFlag();
			break;

		case Common::KEYCODE_F8:
			// F8 - Credits
			if (R2_GLOBALS._sceneManager._sceneNumber != 205)
				R2_GLOBALS._sceneManager.changeScene(205);
			break;

		case Common::KEYCODE_F10:
			// F10 - Pause
			GfxDialog::setPalette();
			MessageDialog::show(GAME_PAUSED_MSG, OK_BTN_STRING);
			R2_GLOBALS._events.setCursorFromFlag();
			break;

		default:
			break;
		}
	}
}

void Ringworld2Game::rightClick() {
	RightClickDialog *dlg = new RightClickDialog();
	int option = dlg->execute();
	delete dlg;

	if (option == 0)
		CharacterDialog::show();
	else if (option == 1)
		HelpDialog::show();
}

/*--------------------------------------------------------------------------*/

NamedHotspot::NamedHotspot() : SceneHotspot() {
	_resNum = 0;
	_lookLineNum = _useLineNum = _talkLineNum = -1;
}

bool NamedHotspot::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_WALK:
		// Nothing
		return false;
	case CURSOR_LOOK:
		if (_lookLineNum == -1)
			return SceneHotspot::startAction(action, event);

		SceneItem::display2(_resNum, _lookLineNum);
		return true;
	case CURSOR_USE:
		if (_useLineNum == -1)
			return SceneHotspot::startAction(action, event);

		SceneItem::display2(_resNum, _useLineNum);
		return true;
	case CURSOR_TALK:
		if (_talkLineNum == -1)
			return SceneHotspot::startAction(action, event);

		SceneItem::display2(_resNum, _talkLineNum);
		return true;
	default:
		return SceneHotspot::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

void SceneActor::postInit(SceneObjectList *OwnerList) {
	_lookLineNum = _talkLineNum = _useLineNum = -1;
	SceneObject::postInit();
}

void SceneActor::remove() {
	R2_GLOBALS._sceneItems.remove(this);
	_shadowMap = NULL;
	_linkedActor = NULL;

	SceneObject::remove();
}

bool SceneActor::startAction(CursorType action, Event &event) {
	bool handled = true;

	switch (action) {
	case CURSOR_LOOK:
		if (_lookLineNum == -1)
			handled = false;
		else
			SceneItem::display2(_resNum, _lookLineNum);
		break;
	case CURSOR_USE:
		if (_useLineNum == -1)
			handled = false;
		else
			SceneItem::display2(_resNum, _useLineNum);
		break;
	case CURSOR_TALK:
		if (_talkLineNum == -1)
			handled = false;
		else
			SceneItem::display2(_resNum, _talkLineNum);
		break;
	default:
		handled = false;
		break;
	}

	if (!handled)
		handled = ((SceneExt *)R2_GLOBALS._sceneManager._scene)->display(action, event);
	return handled;
}

GfxSurface SceneActor::getFrame() {
	GfxSurface frame = SceneObject::getFrame();

	return frame;
}

/*--------------------------------------------------------------------------*/

SceneArea::SceneArea(): SceneItem() {
	_enabled = true;
	_insideArea = false;
	_savedCursorNum = CURSOR_NONE;
	_cursorState = 0;
	_cursorNum = CURSOR_NONE;
}

void SceneArea::synchronize(Serializer &s) {
	EventHandler::synchronize(s);

	_bounds.synchronize(s);
	s.syncAsSint16LE(_enabled);
	s.syncAsSint16LE(_insideArea);
	s.syncAsSint32LE(_cursorNum);
	s.syncAsSint32LE(_savedCursorNum);
	s.syncAsSint16LE(_cursorState);
}

void SceneArea::remove() {
	static_cast<SceneExt *>(R2_GLOBALS._sceneManager._scene)->_sceneAreas.remove(this);
}

void SceneArea::process(Event &event) {
	Common::Point mousePos = event.mousePos;
	mousePos.x += R2_GLOBALS._sceneManager._scene->_sceneBounds.left;

	if (!R2_GLOBALS._insetUp && _enabled && R2_GLOBALS._events.isCursorVisible()) {
		CursorType cursor = R2_GLOBALS._events.getCursor();

		if (_bounds.contains(mousePos)) {
			// Cursor moving in bounded area
			if (cursor != _cursorNum) {
				_savedCursorNum = cursor;
				_cursorState = 0;
				R2_GLOBALS._events.setCursor(_cursorNum);
			}
			_insideArea = true;
		} else if ((mousePos.y < 171) && _insideArea && (_cursorNum == cursor) &&
				(_savedCursorNum != CURSOR_NONE)) {
			// Cursor moved outside bounded area
			R2_GLOBALS._events.setCursor(_savedCursorNum);
		}
	}
}

void SceneArea::setDetails(const Rect &bounds, CursorType cursor) {
	_bounds = bounds;
	_cursorNum = cursor;

	static_cast<SceneExt *>(R2_GLOBALS._sceneManager._scene)->_sceneAreas.push_front(this);
}

/*--------------------------------------------------------------------------*/

SceneExit::SceneExit(): SceneArea() {
	_moving = false;
	_destPos = Common::Point(-1, -1);

	_sceneNumber = 0;
}

void SceneExit::synchronize(Serializer &s) {
	SceneArea::synchronize(s);

	s.syncAsSint16LE(_moving);
	s.syncAsSint16LE(_destPos.x);
	s.syncAsSint16LE(_destPos.y);
}

void SceneExit::setDetails(const Rect &bounds, CursorType cursor, int sceneNumber) {
	_sceneNumber = sceneNumber;
	SceneArea::setDetails(bounds, cursor);
}

void SceneExit::changeScene() {
	R2_GLOBALS._sceneManager.setNewScene(_sceneNumber);
}

void SceneExit::process(Event &event) {
	Common::Point mousePos = event.mousePos;
	mousePos.x += R2_GLOBALS._sceneManager._scene->_sceneBounds.left;

	if (!R2_GLOBALS._insetUp) {
		SceneArea::process(event);

		if (_enabled && R2_GLOBALS._player._enabled) {
			if (event.eventType == EVENT_BUTTON_DOWN) {
				if (!_bounds.contains(mousePos))
					_moving = false;
				else if (!R2_GLOBALS._player._canWalk) {
					_moving = false;
					changeScene();
					event.handled = true;
				} else {
					Common::Point dest((_destPos.x == -1) ? mousePos.x : _destPos.x,
						(_destPos.y == -1) ? mousePos.y : _destPos.y);
					ADD_PLAYER_MOVER(dest.x, dest.y);

					_moving = true;
					event.handled = true;
				}
			}

			if (_moving && (_bounds.contains(R2_GLOBALS._player._position) || (R2_GLOBALS._player._position == _destPos)))
				changeScene();
		}
	}
}

/*--------------------------------------------------------------------------*/

void SceneAreaObject::remove() {
	R2_GLOBALS._sceneItems.remove(this);
	_object1.remove();
	SceneArea::remove();
	--R2_GLOBALS._insetUp;
}

void SceneAreaObject::process(Event &event) {
	if (_insetCount == R2_GLOBALS._insetUp) {
		CursorType cursor = R2_GLOBALS._events.getCursor();

		if (_object1._bounds.contains(event.mousePos)) {
			// Cursor moving in bounded area
			if (cursor == _cursorNum) {
				R2_GLOBALS._events.setCursor(_savedCursorNum);
			}
		} else if (event.mousePos.y < 168) {
			if (_cursorNum != cursor) {
				// Cursor moved outside bounded area
				_savedCursorNum = R2_GLOBALS._events.getCursor();
				R2_GLOBALS._events.setCursor(CURSOR_INVALID);
			}

			if (event.eventType == EVENT_BUTTON_DOWN) {
				event.handled = true;
				R2_GLOBALS._events.setCursor(_savedCursorNum);
				remove();
			}
		}
	}
}

void SceneAreaObject::setDetails(int visage, int strip, int frameNumber, const Common::Point &pt) {
	_object1.postInit();
	_object1.setup(visage, strip, frameNumber);
	_object1.setPosition(pt);
	_object1.fixPriority(250);

	_cursorNum = CURSOR_INVALID;
	Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;
	scene->_sceneAreas.push_front(this);

	_insetCount = ++R2_GLOBALS._insetUp;
}

void SceneAreaObject::setDetails(int resNum, int lookLineNum, int talkLineNum, int useLineNum) {
	_object1.setDetails(resNum, lookLineNum, talkLineNum, useLineNum,
		2, (SceneItem *)NULL);
}

/*****************************************************************************/

MazeUI::MazeUI() {
	_mapData = NULL;
	_cellsVisible.x = _cellsVisible.y = 0;
	_mapCells.x = _mapCells.y = 0;
	_cellSize.x = _cellSize.y = 0;
	_mapOffset.x = _mapOffset.y = 0;
	_resNum = _cellsResNum = 0;
	_frameCount = _resCount = _mapImagePitch = 0;
}

MazeUI::~MazeUI() {
	DEALLOCATE(_mapData);
}

void MazeUI::synchronize(Serializer &s) {
	SceneObject::synchronize(s);

	s.syncAsSint16LE(_resNum);
	if (s.isLoading())
		load(_resNum);

	s.syncAsSint16LE(_mapOffset.x);
	s.syncAsSint16LE(_mapOffset.y);

	int dummy = 0;
	s.syncAsSint16LE(dummy);
}

void MazeUI::load(int resNum) {
	clear();
	_resNum = resNum;

	const byte *header = g_resourceManager->getResource(RT17, resNum, 0);

	_cellsResNum = resNum + 1000;
	_mapCells.x = READ_LE_UINT16(header + 2);
	_mapCells.y = READ_LE_UINT16(header + 4);
	_frameCount = 10;
	_resCount = _frameCount << 3;

	Visage visage;
	visage.setVisage(_cellsResNum, 1);

	GfxSurface frame = visage.getFrame(2);
	_cellSize.x = frame.getBounds().width();
	_cellSize.y = frame.getBounds().height();

	_mapData = g_resourceManager->getResource(RT17, resNum, 1);

	_mapOffset.y = _mapOffset.x = 0;
	_cellsVisible.x = (_bounds.width() + _cellSize.x - 1) / _cellSize.x;
	_cellsVisible.y = (_bounds.height() + _cellSize.y - 1) / _cellSize.y;

	_mapImagePitch = (_cellsVisible.x + 1) * _cellSize.x;
	_mapImage.create(_mapImagePitch, _cellSize.y);

	_mapBounds = Rect(0, 0, _cellSize.x * _mapCells.x, _cellSize.y * _mapCells.y);
}

void MazeUI::clear() {
	if (!_resNum)
		_resNum = 1;

	if (_mapData)
		DEALLOCATE(_mapData);
	_mapData = NULL;

	_mapImage.clear();
}

bool MazeUI::setMazePosition(const Common::Point &pt) {
	bool retval = false;

	_mapOffset = pt;

	if (_mapOffset.x < _mapBounds.top) {
		_mapOffset.x = _mapBounds.top;
		retval = true;
	}

	if (_mapOffset.y < _mapBounds.left) {
		_mapOffset.y = _mapBounds.left;
		retval = true;
	}

	if (_mapOffset.x + _bounds.width() > _mapBounds.right) {
		_mapOffset.x = _mapBounds.right - _bounds.width();
		retval = true;
	}

	if (_mapOffset.y + _bounds.height() > _mapBounds.bottom) {
		_mapOffset.y = _mapBounds.bottom - _bounds.height();
		retval = true;
	}

	return retval;
}

void MazeUI::reposition() {
}

void MazeUI::draw() {
	int yPos = 0;
	int ySize;
	Visage visage;

	_cellsVisible.y = ((_mapOffset.y % _cellSize.y) + _bounds.height() +
		(_cellSize.y - 1)) / _cellSize.y;

	// Loop to handle the cell rows of the visible display area one at a time
	for (int yCtr = 0; yCtr <= _cellsVisible.y; ++yCtr, yPos += ySize) {
		int cellY = _mapOffset.y / _cellSize.y + yCtr;

		// Loop to iterate through the horizontal visible cells to build up
		// an entire cell high horizontal slice of the map, plus one extra cell
		// to allow for partial cell scrolling on-screen on the left/right sides
		for (int xCtr = 0; xCtr <= _cellsVisible.x; ++xCtr) {
			int cellX = _mapOffset.x / _cellSize.x + xCtr;

			// Get the type of content to display in the cell
			int cell = getCellFromCellXY(Common::Point(cellX, cellY)) - 1;
			if (cell >= 0) {
				int frameNum = (cell % _frameCount) + 1;
				int rlbNum = (cell % _resCount) / _frameCount + 1;
				int resNum = _cellsResNum + (cell / _resCount);

				visage.setVisage(resNum, rlbNum);
				GfxSurface frame = visage.getFrame(frameNum);

				_mapImage.copyFrom(frame, xCtr * _cellSize.x, 0);
			} else {
				GfxSurface emptyRect;
				emptyRect.create(_cellSize.x, _cellSize.y);

				_mapImage.copyFrom(emptyRect, xCtr * _cellSize.x, 0);
			}
		}

		if (yPos == 0) {
			// First line of the map to be displayed - only the bottom portion of that
			// first cell row may be visible
			yPos = _bounds.top;
			ySize = _cellSize.y - (_mapOffset.y % _cellSize.y);

			Rect srcBounds(_mapOffset.x % _cellSize.x, _mapOffset.y % _cellSize.y,
				(_mapOffset.x % _cellSize.x) + _bounds.width(), _cellSize.y);
			Rect destBounds(_bounds.left, yPos, _bounds.right, yPos + ySize);

			R2_GLOBALS.gfxManager().copyFrom(_mapImage, srcBounds, destBounds);
		} else {
			if ((yPos + _cellSize.y) < _bounds.bottom) {
				ySize = _cellSize.y;
			} else {
				ySize = _bounds.bottom - yPos;
			}

			Rect srcBounds(_mapOffset.x % _cellSize.x, 0,
				(_mapOffset.x % _cellSize.x) + _bounds.width(), ySize);
			Rect destBounds(_bounds.left, yPos, _bounds.right, yPos + ySize);

			R2_GLOBALS.gfxManager().copyFrom(_mapImage, srcBounds, destBounds);
		}
	}
}

int MazeUI::getCellFromPixelXY(const Common::Point &pt) {
	if (!_bounds.contains(pt))
		return -1;

	int cellX = (pt.x - _bounds.left + _mapOffset.x) / _cellSize.x;
	int cellY = (pt.y - _bounds.top + _mapOffset.y) / _cellSize.y;

	if ((cellX >= 0) && (cellY >= 0) && (cellX < _mapCells.x) && (cellY < _mapCells.y))
		return (int16)READ_LE_UINT16(_mapData + (_mapCells.x * cellY + cellX) * 2);

	return -1;
}

int MazeUI::getCellFromCellXY(const Common::Point &p) {
	if (p.x < 0 || p.y < 0 || p.x >= _mapCells.x || p.y >= _mapCells.y) {
		return -1;
	} else {
		return (int16)READ_LE_UINT16(_mapData + (_mapCells.x * p.y + p.x) * 2);
	}
}

int MazeUI::pixelToCellXY(Common::Point &pt) {
	pt.x /= _cellSize.x;
	pt.y /= _cellSize.y;

	if ((pt.x >= 0) && (pt.y >= 0) && (pt.x < _mapCells.x) && (pt.y < _mapCells.y)) {
		return (int16)READ_LE_UINT16(_mapData + (_mapCells.x * pt.y + pt.x) * 2);
	}

	return -1;
}

void MazeUI::setDisplayBounds(const Rect &r) {
	_bounds = r;
	_bounds.clip(g_globals->gfxManager()._bounds);
}

/*--------------------------------------------------------------------------*/

void AnimationSlice::load(Common::File &f) {
	f.skip(2);
	_sliceOffset = f.readUint16LE();
	f.skip(6);
	_drawMode = f.readByte();
	_secondaryIndex = f.readByte();
}

/*--------------------------------------------------------------------------*/

AnimationSlices::AnimationSlices() {
	_pixelData = NULL;

	_dataSize = 0;
	_dataSize2 = 0;
	_slices->_sliceOffset = 0;
	_slices->_drawMode = 0;
	_slices->_secondaryIndex = 0;
}

AnimationSlices::~AnimationSlices() {
	delete[] _pixelData;
}

void AnimationSlices::load(Common::File &f) {
	f.skip(4);
	_dataSize = f.readUint32LE();
	f.skip(8);
	_dataSize2 = f.readUint32LE();
	f.skip(28);

	// Load the four slice indexes
	for (int idx = 0; idx < 4; ++idx)
		_slices[idx].load(f);
}

int AnimationSlices::loadPixels(Common::File &f, int slicesSize) {
	delete[] _pixelData;
	_pixelData = new byte[slicesSize];
	return f.read(_pixelData, slicesSize);
}

/*--------------------------------------------------------------------------*/

void AnimationPlayerSubData::load(Common::File &f) {
	uint32 posStart = f.pos();

	f.skip(6);
	_duration = f.readUint32LE();
	_frameRate = f.readUint16LE();
	_framesPerSlices = f.readUint16LE();
	_drawType = f.readUint16LE();
	f.skip(2);
	_sliceSize = f.readUint16LE();
	_ySlices = f.readUint16LE();
	_field16 = f.readUint32LE();
	f.skip(2);
	_palStart = f.readUint16LE();
	_palSize = f.readUint16LE();
	f.read(_palData, 768);
	_totalSize = f.readSint32LE();
	f.skip(12);
	_slices.load(f);

	uint32 posEnd = f.pos();
	assert((posEnd - posStart) == 0x390);
}

/*--------------------------------------------------------------------------*/

AnimationPlayer::AnimationPlayer(): EventHandler() {
	_endAction = NULL;

	_animData1 = NULL;
	_animData2 = NULL;

	_screenBounds = R2_GLOBALS._gfxManagerInstance._bounds;
	_rect1 = R2_GLOBALS._gfxManagerInstance._bounds;
	_paletteMode = ANIMPALMODE_REPLACE_PALETTE;
	_canSkip = true;
	_sliceHeight = 1;
	_endAction = NULL;

	_sliceCurrent = nullptr;
	_sliceNext = nullptr;
	_animLoaded = false;
	_objectMode = ANIMOBJMODE_1;
	_dataNeeded = 0;
	_playbackTick = 0;
	_playbackTickPrior = 0;
	_position = 0;
	_nextSlicesPosition = 0;
	_frameDelay = 0;
	_gameFrame = 0;
}

AnimationPlayer::~AnimationPlayer() {
	if (!isCompleted())
		close();
}

void AnimationPlayer::synchronize(Serializer &s) {
	EventHandler::synchronize(s);

	// TODO: Implement saving for animation player state. Currently, I disable saving
	// when an animation is active, so saving it's state would a "nice to have".
}

void AnimationPlayer::remove() {
	if (_endAction)
		_endAction->signal();

	_endAction = NULL;
}

void AnimationPlayer::process(Event &event) {
	if ((event.eventType == EVENT_KEYPRESS) && (event.kbd.keycode == Common::KEYCODE_ESCAPE) && _canSkip) {
		// Move the current position to the end
		_position = _subData._duration;
	}
}

void AnimationPlayer::dispatch() {
	uint32 gameFrame = R2_GLOBALS._events.getFrameNumber();
	uint32 gameDiff = gameFrame - _gameFrame;

	if (gameDiff >= _frameDelay) {
		drawFrame(_playbackTick % _subData._framesPerSlices);
		++_playbackTick;
		_position = _playbackTick / _subData._framesPerSlices;

		if (_position == _nextSlicesPosition)
			nextSlices();

		_playbackTickPrior = _playbackTick;
		_gameFrame = gameFrame;
	}
}

bool AnimationPlayer::load(int animId, Action *endAction) {
	// Open up the main resource file for access
	TLib &libFile = g_resourceManager->first();
	if (!_resourceFile.open(libFile.getFilename()))
		error("Could not open resource");

	// Get the offset of the given resource and seek to it in the player's file reference
	ResourceEntry entry;
	uint32 fileOffset = libFile.getResourceStart(RES_IMAGE, animId, 0, entry);
	_resourceFile.seek(fileOffset);

	// At this point, the file is pointing to the start of the resource data

	// Set the end action
	_endAction = endAction;

	// Load the sub data block
	_subData.load(_resourceFile);

	// Set other properties
	_playbackTickPrior = -1;
	_playbackTick = 0;

	_frameDelay = (60 / _subData._frameRate);
	_gameFrame = R2_GLOBALS._events.getFrameNumber();

	// WORKAROUND: Slow down the title sequences to better match the original
	if (animId <= 4 || animId == 15)
		_frameDelay *= 8;

	if (_subData._totalSize) {
		_dataNeeded = _subData._totalSize;
	} else {
		int v = (_subData._sliceSize + 2) * _subData._ySlices * _subData._framesPerSlices;
		_dataNeeded = (_subData._field16 / _subData._framesPerSlices) + v + 96;
	}

	debugC(1, ktSageDebugGraphics, "Data needed %d", _dataNeeded);

	// Set up animation data objects
	_animData1 = new AnimationData();
	_sliceCurrent = _animData1;

	if (_subData._framesPerSlices <= 1) {
		_animData2 = NULL;
		_sliceNext = _sliceCurrent;
	} else {
		_animData2 = new AnimationData();
		_sliceNext = _animData2;
	}

	_position = 0;
	_nextSlicesPosition = 1;

	// Load up the first slices set
	_sliceCurrent->_dataSize = _subData._slices._dataSize;
	_sliceCurrent->_slices = _subData._slices;
	int slicesSize = _sliceCurrent->_dataSize - 96;
	int readSize = _sliceCurrent->_slices.loadPixels(_resourceFile, slicesSize);
	_sliceCurrent->_animSlicesSize = readSize + 96;

	if (_sliceNext != _sliceCurrent) {
		getSlices();
	}

	// Handle starting palette
	switch (_paletteMode) {
	case ANIMPALMODE_REPLACE_PALETTE:
		// Use the palette provided with the animation directly
		_palette.getPalette();
		for (int idx = _subData._palStart; idx < (_subData._palStart + _subData._palSize); ++idx) {
			byte r = _subData._palData[idx * 3];
			byte g = _subData._palData[idx * 3 + 1];
			byte b = _subData._palData[idx * 3 + 2];

			R2_GLOBALS._scenePalette.setEntry(idx, r, g, b);
		}

		R2_GLOBALS._sceneManager._hasPalette = true;
		break;
	case ANIMPALMODE_NONE:
		break;

	default:
		// ANIMPALMODE_CURR_PALETTE
		// Use the closest matching colors in the currently active palette to those specified in the animation
		for (int idx = _subData._palStart; idx < (_subData._palStart + _subData._palSize); ++idx) {
			byte r = _subData._palData[idx * 3];
			byte g = _subData._palData[idx * 3 + 1];
			byte b = _subData._palData[idx * 3 + 2];

			int palIndex = R2_GLOBALS._scenePalette.indexOf(r, g, b);
			_palIndexes[idx] = palIndex;
		}
		break;
	}

	++R2_GLOBALS._animationCtr;
	_animLoaded = true;
	return true;
}

void AnimationPlayer::drawFrame(int sliceIndex) {
	assert(sliceIndex < 4);
	AnimationSlices &slices = _sliceCurrent->_slices;
	AnimationSlice &slice = _sliceCurrent->_slices._slices[sliceIndex];

	byte *sliceDataStart = &slices._pixelData[slice._sliceOffset - 96];
	byte *sliceData1 = sliceDataStart;

	Rect playerBounds = _screenBounds;

	Graphics::Surface dest = R2_GLOBALS._screen.getSubArea(playerBounds);
	int y = 0;

	// Handle different drawing modes
	switch (slice._drawMode) {
	case 0:
		// Draw from uncompressed source
		for (int sliceNum = 0; sliceNum < _subData._ySlices; ++sliceNum) {
			for (int yIndex = 0; yIndex < _sliceHeight; ++yIndex, ++y) {
				// TODO: Check of _subData._drawType was done for two different kinds of
				// line slice drawing in original
				const byte *pSrc = (const byte *)sliceDataStart + READ_LE_UINT16(sliceData1 + sliceNum * 2);
				byte *pDest = (byte *)dest.getBasePtr(0, y);

				Common::copy(pSrc, pSrc + _subData._sliceSize, pDest);
			}
		}
		break;

	case 1:
		switch (slice._secondaryIndex) {
		case 0xfe:
			// Draw from uncompressed source with optional skipped rows
			for (int sliceNum = 0; sliceNum < _subData._ySlices; ++sliceNum) {
				for (int yIndex = 0; yIndex < _sliceHeight; ++yIndex, ++y) {
					int offset = READ_LE_UINT16(sliceData1 + sliceNum * 2);

					if (offset) {
						const byte *pSrc = (const byte *)sliceDataStart + offset;
						byte *pDest = (byte *)dest.getBasePtr(0, y);

						//Common::copy(pSrc, pSrc + playerBounds.width(), pDest);
						rleDecode(pSrc, pDest, playerBounds.width());
					}
				}
			}
			break;
		case 0xff:
			// Draw from RLE compressed source
			for (int sliceNum = 0; sliceNum < _subData._ySlices; ++sliceNum) {
				for (int yIndex = 0; yIndex < _sliceHeight; ++yIndex, ++y) {
					// TODO: Check of _subData._drawType was done for two different kinds of
					// line slice drawing in original
					const byte *pSrc = (const byte *)sliceDataStart + READ_LE_UINT16(sliceData1 + sliceNum * 2);
					byte *pDest = (byte *)dest.getBasePtr(0, y);

					rleDecode(pSrc, pDest, _subData._sliceSize);
				}
			}
			break;
		default: {
			// Draw from two slice sets simultaneously
			AnimationSlice &slice2 = _sliceCurrent->_slices._slices[slice._secondaryIndex];
			byte *sliceData2 = &slices._pixelData[slice2._sliceOffset - 96];

			for (int sliceNum = 0; sliceNum < _subData._ySlices; ++sliceNum) {
				for (int yIndex = 0; yIndex < _sliceHeight; ++yIndex, ++y) {
					const byte *pSrc1 = (const byte *)sliceDataStart + READ_LE_UINT16(sliceData2 + sliceNum * 2);
					const byte *pSrc2 = (const byte *)sliceDataStart + READ_LE_UINT16(sliceData1 + sliceNum * 2);
					byte *pDest = (byte *)dest.getBasePtr(0, y);

					if (slice2._drawMode == 0) {
						// Uncompressed background, foreground compressed
						Common::copy(pSrc1, pSrc1 + _subData._sliceSize, pDest);
						rleDecode(pSrc2, pDest, _subData._sliceSize);
					} else {
						// Both background and foreground is compressed
						rleDecode(pSrc1, pDest, _subData._sliceSize);
						rleDecode(pSrc2, pDest, _subData._sliceSize);
					}
				}
			}
			break;
		}
		}
	default:
		break;
	}

	if (_objectMode == ANIMOBJMODE_42) {
		_screenBounds.expandPanes();

		// Copy the drawn frame to the back surface
		Rect srcRect = R2_GLOBALS._screen.getBounds();
		Rect destRect = srcRect;
		destRect.translate(-g_globals->_sceneOffset.x, -g_globals->_sceneOffset.y);
		R2_GLOBALS._sceneManager._scene->_backSurface.copyFrom(R2_GLOBALS._screen,
			srcRect, destRect);

		// Draw any objects into the scene
		R2_GLOBALS._sceneObjects->draw();
	} else {
		if (R2_GLOBALS._sceneManager._hasPalette) {
			R2_GLOBALS._sceneManager._hasPalette = false;
			R2_GLOBALS._scenePalette.refresh();
		}
	}
}

/**
 * Read the next frame's slice set
 */
void AnimationPlayer::nextSlices() {
	_position = _nextSlicesPosition++;
	_playbackTick = _position * _subData._framesPerSlices;
	_playbackTickPrior = _playbackTick - 1;

	if (_sliceNext == _sliceCurrent) {
		int dataSize = _sliceCurrent->_slices._dataSize2;
		_sliceCurrent->_dataSize = dataSize;
		debugC(1, ktSageDebugGraphics, "Next frame size = %xh", dataSize);
		if (dataSize == 0)
			return;

		dataSize -= 96;
		assert(dataSize >= 0);
		_sliceCurrent->_slices.load(_resourceFile);
		_sliceCurrent->_animSlicesSize = _sliceCurrent->_slices.loadPixels(_resourceFile, dataSize);
	} else {
		SWAP(_sliceCurrent, _sliceNext);
		getSlices();
	}
}

bool AnimationPlayer::isCompleted() {
	return (_position >= _subData._duration);
}

void AnimationPlayer::close() {
	if (_animLoaded) {
		switch (_paletteMode) {
		case 0:
			R2_GLOBALS._scenePalette.replace(&_palette);
			changePane();
			R2_GLOBALS._sceneManager._hasPalette = true;
			break;
		case 2:
			closing();
			break;
		default:
			changePane();
			break;
		}
	}

	// Close the resource file
	_resourceFile.close();

	if (_objectMode != ANIMOBJMODE_42) {
		// flip screen in original
	}

	// Free animation objects
	delete _animData1;
	delete _animData2;
	_animData1 = NULL;
	_animData2 = NULL;

	_animLoaded = false;
	if (g_globals != NULL)
		R2_GLOBALS._animationCtr = MAX(R2_GLOBALS._animationCtr - 1, 0);
}

void AnimationPlayer::rleDecode(const byte *pSrc, byte *pDest, int size) {
	while (size > 0) {
		byte v = *pSrc++;
		if (!(v & 0x80)) {
			// Following uncompressed set of bytes
			Common::copy(pSrc, pSrc + v, pDest);
			pSrc += v;
			pDest += v;
			size -= v;
		} else {
			int count = v & 0x3F;
			size -= count;

			if (!(v & 0x40)) {
				// Skip over a number of bytes
				pDest += count;
			} else {
				// Replicate a number of bytes
				Common::fill(pDest, pDest + count, *pSrc++);
				pDest += count;
			}
		}
	}
}

void AnimationPlayer::getSlices() {
	assert((_sliceNext == _animData1) || (_sliceNext == _animData2));
	assert((_sliceCurrent == _animData1) || (_sliceCurrent == _animData2));

	_sliceNext->_dataSize = _sliceCurrent->_slices._dataSize2;
	if (_sliceNext->_dataSize) {
		if (_sliceNext->_dataSize >= _dataNeeded)
			error("Bogus dataNeeded == %d / %d", _sliceNext->_dataSize, _dataNeeded);
	}

	int dataSize = _sliceNext->_dataSize - 96;
	_sliceNext->_slices.load(_resourceFile);
	_sliceNext->_animSlicesSize = _sliceNext->_slices.loadPixels(_resourceFile, dataSize);
}

/*--------------------------------------------------------------------------*/

AnimationPlayerExt::AnimationPlayerExt(): AnimationPlayer() {
	_isActive = false;
	_canSkip = false;
}

void AnimationPlayerExt::synchronize(Serializer &s) {
	AnimationPlayer::synchronize(s);
	s.syncAsSint16LE(_isActive);
}

/*--------------------------------------------------------------------------*/

ModalWindow::ModalWindow() {
	_insetCount = 0;
}

void ModalWindow::remove() {
	R2_GLOBALS._sceneItems.remove(&_object1);
	_object1.remove();

	SceneArea::remove();

	--R2_GLOBALS._insetUp;
}

void ModalWindow::synchronize(Serializer &s) {
	SceneArea::synchronize(s);

	s.syncAsByte(_insetCount);
}

void ModalWindow::process(Event &event) {
	if (_insetCount != R2_GLOBALS._insetUp)
		return;

	CursorType cursor = R2_GLOBALS._events.getCursor();

	if (_object1._bounds.contains(event.mousePos.x + g_globals->gfxManager()._bounds.left , event.mousePos.y)) {
		if (cursor == _cursorNum) {
			R2_GLOBALS._events.setCursor(_savedCursorNum);
		}
	} else if (event.mousePos.y < 168) {
		if (cursor != _cursorNum) {
			_savedCursorNum = cursor;
			R2_GLOBALS._events.setCursor(CURSOR_INVALID);
		}
		if (event.eventType == EVENT_BUTTON_DOWN) {
			event.handled = true;
			R2_GLOBALS._events.setCursor(_savedCursorNum);
			remove();
		}
	}
}

void ModalWindow::setup2(int visage, int stripFrameNum, int frameNum, int posX, int posY) {
	Scene1200 *scene = (Scene1200 *)R2_GLOBALS._sceneManager._scene;

	_object1.postInit();
	_object1.setup(visage, stripFrameNum, frameNum);
	_object1.setPosition(Common::Point(posX, posY));
	_object1.fixPriority(250);
	_cursorNum = CURSOR_INVALID;
	scene->_sceneAreas.push_front(this);
	++R2_GLOBALS._insetUp;
	_insetCount = R2_GLOBALS._insetUp;
}

void ModalWindow::setup3(int resNum, int lookLineNum, int talkLineNum, int useLineNum) {
	_object1.setDetails(resNum, lookLineNum, talkLineNum, useLineNum, 2, (SceneItem *) NULL);
}

/*--------------------------------------------------------------------------*/

ScannerDialog::Button::Button() {
	_buttonId = 0;
	_buttonDown = false;
}

void ScannerDialog::Button::setup(int buttonId) {
	_buttonId = buttonId;
	_buttonDown = false;
	SceneActor::postInit();

	SceneObject::setup(4, 2, 2);
	fixPriority(255);

	if (_buttonId == 1)
		setPosition(Common::Point(141, 99));
	else if (_buttonId == 2)
		setPosition(Common::Point(141, 108));

	static_cast<SceneExt *>(R2_GLOBALS._sceneManager._scene)->_sceneAreas.push_front(this);
}

void ScannerDialog::Button::synchronize(Serializer &s) {
	SceneActor::synchronize(s);
	s.syncAsSint16LE(_buttonId);
}

void ScannerDialog::Button::process(Event &event) {
	if (event.eventType == EVENT_BUTTON_DOWN && R2_GLOBALS._events.getCursor() == CURSOR_USE
			&& _bounds.contains(event.mousePos) && !_buttonDown) {
		setFrame(3);
		_buttonDown = true;
		event.handled = true;
	}

	if (event.eventType == EVENT_BUTTON_UP && _buttonDown) {
		setFrame(2);
		_buttonDown = false;
		event.handled = true;

		reset();
	}
}

bool ScannerDialog::Button::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE)
		return false;

	return startAction(action, event);
}

void ScannerDialog::Button::reset() {
	Scene *scene = R2_GLOBALS._sceneManager._scene;
	ScannerDialog &scanner = *R2_GLOBALS._scannerDialog;

	switch (_buttonId) {
	case 1:
		// Talk button
		switch (R2_GLOBALS._sceneManager._sceneNumber) {
		case 1550:
			scene->_sceneMode = 80;
			scene->signal();
			break;
		case 1700:
			scene->_sceneMode = 30;
			scene->signal();
			remove();
			break;
		default:
			break;
		}
		break;
	case 2:
		// Scan button
		switch (R2_GLOBALS._sceneManager._sceneNumber) {
		case 1550:
			scanner._obj4.setup(4, 3, 1);

			scanner._obj5.postInit();
			scanner._obj5.setup(4, 4, 1);
			scanner._obj5.setPosition(Common::Point(R2_GLOBALS._s1550PlayerArea[R2_QUINN].x + 145,
				R2_GLOBALS._s1550PlayerArea[R2_QUINN].y + 59));
			scanner._obj5.fixPriority(257);

			scanner._obj6.postInit();
			scanner._obj6.setup(4, 4, 2);
			scanner._obj6.setPosition(Common::Point(R2_GLOBALS._s1550PlayerArea[R2_SEEKER].x + 145,
				R2_GLOBALS._s1550PlayerArea[R2_SEEKER].y + 59));
			scanner._obj6.fixPriority(257);
			break;
		case 1700:
		case 1800:
			if (R2_GLOBALS._rimLocation < 1201)
				scanner._obj4.setup(4, 3, 3);
			else if (R2_GLOBALS._rimLocation > 1201)
				scanner._obj4.setup(4, 3, 4);
			else
				scanner._obj4.setup(4, 3, 5);
			break;
		case 3800:
		case 3900:
			if ((R2_GLOBALS._desertWrongDirCtr + 1) == 0 && R2_GLOBALS._desertCorrectDirection == 0) {
				do {
					R2_GLOBALS._desertCorrectDirection = R2_GLOBALS._randomSource.getRandomNumber(3) + 1;
				} while (R2_GLOBALS._desertCorrectDirection == R2_GLOBALS._desertPreviousDirection);
			}

			scanner._obj4.setup(4, 7, R2_GLOBALS._desertCorrectDirection);
			if (!R2_GLOBALS.getFlag(46))
				R2_GLOBALS.setFlag(46);
			break;
		default:
			scanner._obj4.setup(4, 3, 2);
			break;
		}
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

ScannerDialog::Slider::Slider() {
	_initial = _xStart = _yp = 0;
	_width = _xInc = 0;
	_sliderDown = false;
}

void ScannerDialog::Slider::synchronize(Serializer &s) {
	SceneActor::synchronize(s);

	s.syncAsSint16LE(_initial);
	s.syncAsSint16LE(_xStart);
	s.syncAsSint16LE(_yp);
	s.syncAsSint16LE(_width);
	s.syncAsSint16LE(_xInc);
}

void ScannerDialog::Slider::remove() {
	static_cast<SceneExt *>(R2_GLOBALS._sceneManager._scene)->_sceneAreas.remove(this);
	SceneActor::remove();
}

void ScannerDialog::Slider::process(Event &event) {
	if (event.eventType == EVENT_BUTTON_DOWN && R2_GLOBALS._events.getCursor() == CURSOR_USE
			&& _bounds.contains(event.mousePos)) {
		_sliderDown = true;
	}

	if (event.eventType == EVENT_BUTTON_UP && _sliderDown) {
		_sliderDown = false;
		event.handled = true;
		update();
	}

	if (_sliderDown) {
		event.handled = true;
		if (event.mousePos.x < _xStart) {
			setPosition(Common::Point(_xStart, _yp));
		} else if (event.mousePos.x >= (_xStart + _width)) {
			setPosition(Common::Point(_xStart + _width, _yp));
		} else {
			setPosition(Common::Point(event.mousePos.x, _yp));
		}
	}
}

bool ScannerDialog::Slider::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE)
		return false;

	return startAction(action, event);
}

void ScannerDialog::Slider::update() {
	int incHalf = (_width / (_xInc - 1)) / 2;
	int newFrequency = ((_position.x - _xStart + incHalf) * _xInc) / (_width + incHalf * 2);
	setPosition(Common::Point(_xStart + ((_width * newFrequency) / (_xInc - 1)), _yp));

	R2_GLOBALS._scannerFrequencies[R2_GLOBALS._player._characterIndex] = newFrequency + 1;

	switch (newFrequency) {
	case 0:
		R2_GLOBALS._sound4.stop();
		break;
	case 1:
		R2_GLOBALS._sound4.play(45);
		break;
	case 2:
		R2_GLOBALS._sound4.play(4);
		break;
	case 3:
		R2_GLOBALS._sound4.play(5);
		break;
	case 4:
		R2_GLOBALS._sound4.play(6);
		break;
	default:
		break;
	}
}

void ScannerDialog::Slider::setup(int initial, int xStart, int yp, int width, int xInc) {
	_initial = initial;
	_xStart = xStart;
	_yp = yp;
	_width = width;
	_xInc = xInc;
	_sliderDown = false;
	SceneActor::postInit();
	SceneObject::setup(4, 2, 1);
	fixPriority(255);
	setPosition(Common::Point(_width * (_initial - 1) / (_xInc - 1) + _xStart, yp));

	static_cast<SceneExt *>(R2_GLOBALS._sceneManager._scene)->_sceneAreas.push_front(this);
}

/*--------------------------------------------------------------------------*/

ScannerDialog::ScannerDialog() {
}

void ScannerDialog::remove() {
	switch (R2_GLOBALS._sceneManager._sceneNumber) {
	case 1550:
	case 1700:
		R2_GLOBALS._events.setCursor(R2_GLOBALS._player._canWalk ? CURSOR_WALK : CURSOR_USE);
		break;
	case 3800:
	case 3900: {
		Scene *scene = R2_GLOBALS._sceneManager._scene;
		scene->_sceneMode = 3806;
		scene->signal();
		break;
		}
	default:
		break;
	}

	SceneExt *scene = static_cast<SceneExt *>(R2_GLOBALS._sceneManager._scene);
	scene->_sceneAreas.remove(&_talkButton);
	scene->_sceneAreas.remove(&_scanButton);
	_talkButton.remove();
	_scanButton.remove();
	_slider.remove();
	_obj4.remove();
	_obj5.remove();
	_obj6.remove();
	_obj7.remove();

	ModalWindow::remove();
}

void ScannerDialog::setup2(int visage, int stripFrameNum, int frameNum, int posX, int posY) {
	// Stop player moving if currently doing so
	if (R2_GLOBALS._player._mover)
		R2_GLOBALS._player.addMover(NULL);

	R2_GLOBALS._events.setCursor(CURSOR_USE);
	ModalWindow::setup2(visage, stripFrameNum, frameNum, posX, posY);

	setup3(100, -1, -1, -1);
	_talkButton.setup(1);
	_scanButton.setup(2);
	_slider.setup(R2_GLOBALS._scannerFrequencies[R2_GLOBALS._player._characterIndex], 142, 124, 35, 5);

	_obj4.postInit();
	_obj4.setup(4, 3, 2);
	_obj4.setPosition(Common::Point(160, 83));
	_obj4.fixPriority(256);

	if (R2_GLOBALS._sceneManager._sceneNumber == 3800 || R2_GLOBALS._sceneManager._sceneNumber == 3900) {
		Scene *scene = R2_GLOBALS._sceneManager._scene;
		scene->_sceneMode = 3805;
		scene->signal();
	}
}

/*--------------------------------------------------------------------------*/

} // End of namespace Ringworld2

} // End of namespace TsAGE
