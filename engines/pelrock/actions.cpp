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

#include "graphics/paletteman.h"

#include "pelrock.h"
#include "pelrock/actions.h"
#include "pelrock/offsets.h"
#include "pelrock/pelrock.h"
#include "pelrock/util.h"

namespace Pelrock {

#define MASCULINE true
#define FEMININE false

const ActionEntry actionTable[] = {
	// Room 0
	{261, OPEN, &PelrockEngine::openRoomDrawer},
	{261, CLOSE, &PelrockEngine::closeRoomDrawer},
	{268, OPEN, &PelrockEngine::openRoomDoor},
	{268, CLOSE, &PelrockEngine::closeRoomDoor},
	{3, PICKUP, &PelrockEngine::pickUpPhoto},
	{0, PICKUP, &PelrockEngine::pickYellowBook}, // Generic pickup for other items
	// Room 1
	{4, PICKUP, &PelrockEngine::pickUpBrick}, // Brick
	{277, OPEN, &PelrockEngine::openIceCreamShopDoor},
	// Room 2
	{282, OPEN, &PelrockEngine::openMcDoor},
	{282, CLOSE, &PelrockEngine::closeMcDoor},

	// Room 12
	{60, PICKUP, &PelrockEngine::grabKetchup},
	{61, PICKUP, &PelrockEngine::grabMustard},
	{62, PICKUP, &PelrockEngine::grabSpicey},
	{370, OPEN, &PelrockEngine::openKitchenDoor},
	{370, CLOSE, &PelrockEngine::closeKitchenDoor},

	// Room 13
	{375, OPEN, &PelrockEngine::openKitchenDrawer},
	{374, OPEN, &PelrockEngine::openKitchenDoorFromInside},

	// Room 3
	{290, OPEN, &PelrockEngine::openShopDoor},
	{290, CLOSE, &PelrockEngine::closeShopDoor},
	{32, OPEN, &PelrockEngine::openLamppost},
	{308, PICKUP, &PelrockEngine::moveCable}, // Lamppost cable

	// Room 15
	{65, PICKUP, &PelrockEngine::pickGuitar},
	{66, PICKUP, &PelrockEngine::pickFish},
	{67, PICKUP, &PelrockEngine::pickTeddyBear},
	{68, PICKUP, &PelrockEngine::pickDiscs},
	{69, PICKUP, &PelrockEngine::pickMonkeyBrain},
	{70, PICKUP, &PelrockEngine::pickBooks},
	{71, PICKUP, &PelrockEngine::pickPalette},
	{72, PICKUP, &PelrockEngine::pickCandy},
	{73, PICKUP, &PelrockEngine::pickConch},
	{74, PICKUP, &PelrockEngine::pickHat},
	{6, PICKUP, &PelrockEngine::pickCord},
	{7, PICKUP, &PelrockEngine::pickAmulet},

	// Room 4
	{315, OPEN, &PelrockEngine::openPlug},
	{316, PICKUP, &PelrockEngine::pickCables},
	{312, OPEN, &PelrockEngine::openMuseumDoor},

	// // Room 5
	// {},

	// // Room 7
	// {},

	// Room 8
	{355, OPEN, &PelrockEngine::openLibraryOutdoorsDoor},
	{355, CLOSE, &PelrockEngine::closeLibraryOutdoorsDoor},
	{357, PICKUP, &PelrockEngine::pickUpLetter},

	// Room 9
	{363, OPEN, &PelrockEngine::openLibraryIndoorsDoor},
	{363, CLOSE, &PelrockEngine::closeLibraryIndoorsDoor},
	{360, PICKUP, &PelrockEngine::pickBooksFromShelf1},
	{361, PICKUP, &PelrockEngine::pickBooksFromShelf2},
	{362, PICKUP, &PelrockEngine::pickBooksFromShelf3},

	// Room 16
	{388, OPEN, &PelrockEngine::openNewspaperDoor},
	{388, CLOSE, &PelrockEngine::closeNewspaperDoor},

	// Room 17
	{393, OPEN, &PelrockEngine::openNewspaperBossDor},
	{393, CLOSE, &PelrockEngine::closeNewspaperBossDoor},

	// Room 19
	{400, OPEN, &PelrockEngine::openTravelAgencyDoor},
	{400, CLOSE, &PelrockEngine::closeTravelAgencyDoor},

	// Room 25
	{609, PICKUP, &PelrockEngine::pickupSunflower},

	// Room 28
	{472, PICKUP, &PelrockEngine::pickUpMatches},
	{87, PICKUP, &PelrockEngine::pickUpChainsaw},
	{88, PICKUP, &PelrockEngine::pickUpSpellbook},
	{89, PICKUP, &PelrockEngine::pickUpBoot},
	{112, PICKUP, &PelrockEngine::pickupCondoms},

	// Generic handlers
	{WILDCARD, PICKUP, &PelrockEngine::noOpAction}, // Generic pickup action
	{WILDCARD, TALK, &PelrockEngine::noOpAction},   // Generic talk action
	{WILDCARD, WALK, &PelrockEngine::noOpAction},   // Generic walk action
	{WILDCARD, LOOK, &PelrockEngine::noOpAction},   // Generic look action
	{WILDCARD, PUSH, &PelrockEngine::noOpAction},   // Generic push action
	{WILDCARD, PULL, &PelrockEngine::noOpAction},   // Generic pull action
	{WILDCARD, OPEN, &PelrockEngine::noOpAction},   // Generic open action
	{WILDCARD, CLOSE, &PelrockEngine::noOpAction},  // Generic close action

	// End marker
	{WILDCARD, NO_ACTION, nullptr}};

const CombinationEntry combinationTable[] = {
	{2, 281, &PelrockEngine::useCardWithATM},
	{62, 373, &PelrockEngine::useSpicySauceWithBurger},
	{4, 294, &PelrockEngine::useBrickWithWindow},
	{4, 295, &PelrockEngine::useBrickWithShopWindow},
	{6, 315, &PelrockEngine::useCordWithPlug},
	{1, 309, &PelrockEngine::giveIdToGuard},
	{5, 309, &PelrockEngine::giveMoneyToGuard},
	{7, 353, &PelrockEngine::useAmuletWithStatue},
	{8, 353, &PelrockEngine::useSecretCodeWithStatue},
	{8, 358, &PelrockEngine::giveSecretCodeToLibrarian},
	{4, 358, &PelrockEngine::useBrickWithLibrarian}, // Any hotspot in the lamppost will work
	{76, 469, &PelrockEngine::usePumpkinWithRiver},
	// End marker
	{WILDCARD, WILDCARD, nullptr}};

void PelrockEngine::openDoor(HotSpot *hotspot, int exitIndex, int sticker, bool masculine, bool stayClosed) {
	if (_room->hasSticker(sticker)) {
		int text = masculine == MASCULINE ? YA_ABIERTO_M : YA_ABIERTA_F;
		_dialog->say(_res->_ingameTexts[text]);
		return;
	}
	_room->enableExit(exitIndex, stayClosed ? PERSIST_TEMP : PERSIST_BOTH);
	_room->addSticker(sticker, stayClosed ? PERSIST_TEMP : PERSIST_BOTH);
	_sound->playSound(_room->_roomSfx[0]);
}

void PelrockEngine::closeDoor(HotSpot *hotspot, int exitIndex, int sticker, bool masculine, bool stayOpen) {
	if (!_room->hasSticker(sticker)) {
		int text = masculine == MASCULINE ? YA_CERRADO_M : YA_CERRADA_F;
		_dialog->say(_res->_ingameTexts[text]);
		return;
	}
	_room->disableExit(exitIndex, stayOpen ? PERSIST_TEMP : PERSIST_BOTH);
	_room->removeSticker(sticker);
	_sound->playSound(_room->_roomSfx[1]);
}

void PelrockEngine::addInventoryItem(int item) {
	if (_state->inventoryItems.empty()) {
		_state->selectedInventoryItem = item;
	}
	_newItem = item;
	int frameCounter = 0;
	while (frameCounter < kIconFlashDuration) {
		_events->pollEvent();

		bool didRender = renderScene(OVERLAY_PICKUP_ICON);
		_screen->update();
		if (didRender) {
			frameCounter++;
		}
		g_system->delayMillis(10);
	}
	_state->addInventoryItem(item);

	checkObjectsForPart2();
}

void PelrockEngine::buyFromStore(HotSpot *hotspot, int stickerId) {
	if (_state->hasInventoryItem(5) == false) {
		_dialog->say(_res->_ingameTexts[NOTENGODINERO]);
		return;
	} else {
		_room->addSticker(stickerId);
		_room->disableHotspot(hotspot);
		if (hotspot->extra == 69) {
			_room->disableSprite(15, 3); // Disable monkey brain sprite
		}
		addInventoryItem(hotspot->extra);
		_currentHotspot = nullptr;
		walkLoop(224, 283, ALFRED_LEFT);
		_dialog->say(_res->_ingameTexts[CUESTA1000]);
		_dialog->say(_res->_ingameTexts[AQUITIENE]);
		_dialog->say(_res->_ingameTexts[MUYBIEN]);
		_state->removeInventoryItem(5); // Remove 1000 pesetas bill
	}
}

void PelrockEngine::dialogActionTrigger(uint16 actionTrigger, byte room, byte rootIndex) {
	switch (actionTrigger) {
	case 328:
		debug("Setting current root to %d in room %d", rootIndex + 1, room);
		_state->setCurrentRoot(room, rootIndex + 1);
		break;
	case 329:
		_state->setFlag(FLAG_PUTA_250_VECES, true);
		break;
	case 258:
		_state->setFlag(FLAG_GUARDIA_PIDECOSAS, true);
		_state->setCurrentRoot(4, 2);
		break;
	case 259:
		_dialog->say(_res->_ingameTexts[NO_EMPECEMOS]);
		break;
	case 260:
		_dialog->say(_res->_ingameTexts[CUERPO_DANONE], 1);
		_dialog->say(_res->_ingameTexts[CABEZA_HUECA]);
		break;
	case 261:
		_dialog->say(_res->_ingameTexts[ESO_LO_SERAS_TU], 1);
		break;
	case 262:
		_dialog->say(_res->_ingameTexts[DEMASIADO_NO_PUEDO_PENSAR], 1);
		break;
	case 263:
		_dialog->say(_res->_ingameTexts[UN_POCO_RESPETO]);
		break;
	case 264:
		// skip to root after the next one
		_state->setCurrentRoot(room, rootIndex + 2);
		break;
	case 267:
		_state->setCurrentRoot(7, 2);
		break;
	case 272:
		_state->setCurrentRoot(room, rootIndex + 1);
		break;
	case 273:
		WalkBox w1;
		w1.x = 436;
		w1.y = 356;
		w1.w = 4;
		w1.h = 14;
		w1.flags = 0;
		WalkBox w2;
		w2.x = 440;
		w2.y = 368;
		w2.w = 148;
		w2.h = 2;
		w2.flags = 0;

		_room->addWalkbox(w1);
		_room->addWalkbox(w2);
	case 274:
	case 275:
	case 276:
		_state->setCurrentRoot(room, rootIndex + 1);
		break;
	case 277:
		_state->setCurrentRoot(room, rootIndex + 1);
		_state->setFlag(FLAG_JEFE_INGRESA_PASTA, true);
		break;
	case 278:
		_state->setCurrentRoot(room, rootIndex + 1);
		break;
	case 279:
		travelToEgypt();
		break;
	// moros
	case 317:
		addInventoryItem(95);
		break;

	case 330:
		// Two oranges
		addInventoryItem(103);
		break;
	case 331:
		_dialog->say(_res->_ingameTexts[HECHOELPRIMO]);
		break;
	case 332:
		// psychologist card
		if (!_state->hasInventoryItem(104)) {
			addInventoryItem(104);
		}
		break;
	case 333:
		_dialog->say(_res->_ingameTexts[MEHANTOMADO_EL_PELO]);
		break;
	case 334:
		addInventoryItem(86);
		_state->setCurrentRoot(room, rootIndex + 1);

		break;
	case 335:
		// many oranges
		addInventoryItem(104);
		break;
	case 336:
		_dialog->say(_res->_ingameTexts[PESADO_UNRATO]);
		break;
	case 337:
	case 338:
	case 339:
	case 340:
	case 341:
	case 342:
	case 343:
	case 344:
	case 345:
	case 346:
		_state->setCurrentRoot(room, rootIndex + 1);
		break;
	case 348: {
		// Anti-piracy punishment: corrupt screen + noise + crash
		antiPiracyEffect();
		break;
	}
	case 349:
		_state->setFlag(FLAG_CONSIGNAS_VENDEDOR, _state->getFlag(FLAG_CONSIGNAS_VENDEDOR) + 1);
		if (_state->getFlag(FLAG_CONSIGNAS_VENDEDOR) == 2) {
			_state->setCurrentRoot(room, rootIndex + 1);
		}
		break;
	case 350:
		_state->setFlag(FLAG_CONSIGNAS_VENDEDOR, _state->getFlag(FLAG_CONSIGNAS_VENDEDOR) + 1);
		if (_state->getFlag(FLAG_CONSIGNAS_VENDEDOR) == 2) {
			_state->setCurrentRoot(room, rootIndex + 1);
		}
		break;
	case 351:
		_dialog->say(_res->_ingameTexts[TRAIDOR], 0);
		_dialog->say(_res->_ingameTexts[TUTIA], 1);
		_dialog->say(_res->_ingameTexts[LATUYA], 0);
		_dialog->say(_res->_ingameTexts[GORDO], 1);
		_dialog->say(_res->_ingameTexts[FIDEO], 0);
		_dialog->say(_res->_ingameTexts[LIMPIACULO], 1);
		_dialog->say(_res->_ingameTexts[CONTUTURBANTE], 0);
		_dialog->say(_res->_ingameTexts[OSO], 1);
		_dialog->say(_res->_ingameTexts[COMADREJA], 0);
		_dialog->say(_res->_ingameTexts[CABEZON], 1);
		_dialog->say(_res->_ingameTexts[TUABUELO], 0);
		_dialog->say(_res->_ingameTexts[TUMUJER], 1);
		_dialog->say(_res->_ingameTexts[PERDEDOR], 0);
		_dialog->say(_res->_ingameTexts[SOYMEJORQUETU], 1);
		_dialog->say(_res->_ingameTexts[TRAMPOSO], 0);
		_dialog->say(_res->_ingameTexts[MALPERDEDOR], 1);
		_dialog->say(_res->_ingameTexts[PARAUNAVEZ], 0);
		_dialog->say(_res->_ingameTexts[MEJORMELARGO], 1);
		break;
		// end moros
	case 353:
		_state->setCurrentRoot(room, rootIndex + 2);
		break;
	case 354:
		if (_state->hasInventoryItem(105)) {
			addInventoryItem(105);
		}
		break;
	case 352:
	case 355:
		// a la carcel
		toJail();
		break;
	case 356:
		_state->setCurrentRoot(room, 3);
		break;
	// end puta
	//  sabio
	case 366:
		_state->setCurrentRoot(room, rootIndex + 1);
		break;
	case 363:
		toJail();
		break;
	case 367: // accept riddle
		_state->setCurrentRoot(room, 27);
		walkAndAction(_room->findHotspotByExtra(467), TALK);
		break;
		// hasta aqui

	case 357: // wrong answer: counter-- (min 0)
	{
		if (_state->getFlag(FLAG_RESPUESTAS_ACERTADAS) > 0) {
			_state->setFlag(FLAG_RESPUESTAS_ACERTADAS, _state->getFlag(FLAG_RESPUESTAS_ACERTADAS) - 1);
		}
		advanceQuotesConversation(rootIndex, room);
		break;
	}
	case 358: // very wrong answer: counter-=2 (min 0)
	{
		if (_state->getFlag(FLAG_RESPUESTAS_ACERTADAS) > 1) {
			_state->setFlag(FLAG_RESPUESTAS_ACERTADAS, _state->getFlag(FLAG_RESPUESTAS_ACERTADAS) - 2);
		}
		advanceQuotesConversation(rootIndex, room);
		break;
	}
	case 359: // correct answer: counter++, award pin at 15
	{
		_state->setFlag(FLAG_RESPUESTAS_ACERTADAS, _state->getFlag(FLAG_RESPUESTAS_ACERTADAS) + 1);
		if (_state->getFlag(FLAG_RESPUESTAS_ACERTADAS) == 15) {
			addInventoryItem(106); // pin
			_state->setFlag(FLAG_RESPUESTAS_ACERTADAS, 0);
		}
		advanceQuotesConversation(rootIndex, room);
		break;
	}
	case 360: // neutral reset: counter = 0
	{
		_state->setFlag(FLAG_RESPUESTAS_ACERTADAS, 0);
		_state->setCurrentRoot(room, rootIndex + 1);
		advanceQuotesConversation(rootIndex, room);
		break;
	}
	case 361: // "no sé" (I don't know): no counter change, just advance
		_state->setCurrentRoot(room, rootIndex + 1);
		break;
	case 362: // special trigger: enables HIJODELAGRANPUTA cheat code
		// Sets cheat_code_checking_enabled flag in original (0x495F3)
		// TODO: Implement cheat code sequence checker in game loop
		_state->setFlag(FLAG_CHEAT_CODE_ENABLED, 1);
		advanceQuotesConversation(rootIndex, room);
		break;
	case 364: // riddle wrong answer: advance to next riddle
	{
		int targetIndex = rootIndex + 1;
		if (rootIndex == 43) {
			targetIndex = 27; // skip riddle explanation
		}
		_state->setCurrentRoot(room, targetIndex);
		break;
	}
	case 365: // riddle correct: set riddle-solved flag
		_state->setFlag(FLAG_PARADOJA_RESUELTA, 1);
		_state->setCurrentRoot(room, 1);
		break;

	default:
		debug("Got actionTrigger %d in dialogActionTrigger, but no handler defined", actionTrigger);
		break;
	}
}

void PelrockEngine::advanceQuotesConversation(byte rootIndex, byte room) {
	int targetRoot = rootIndex + 1;
	if (targetRoot == 26) {
		targetRoot = 2;
	}
	_state->setCurrentRoot(room, targetRoot);
}

void PelrockEngine::toJail() {
	_graphics->fadeToBlack(10);
	_alfredState.x = 342;
	_alfredState.y = 277;
	setScreen(31, ALFRED_DOWN);
}

void PelrockEngine::noOpAction(HotSpot *hotspot) {
}

void PelrockEngine::noOpItem(int item, HotSpot *hotspot) {
	// 154 to 169
	debug("No-op item %d with hotspot %d", item, hotspot->extra);
	_alfredState.direction = ALFRED_DOWN;
	byte response = (byte)getRandomNumber(12);
	_dialog->say(_res->_ingameTexts[154 + response]);
}

void PelrockEngine::openRoomDoor(HotSpot *hotspot) {
	openDoor(hotspot, 0, 93, FEMININE, true);
}

void PelrockEngine::openRoomDrawer(HotSpot *hotspot) {
	if (_room->hasSticker(91)) {
		_dialog->say(_res->_ingameTexts[YA_ABIERTO_M]);
		return;
	}
	_room->addSticker(91);
	// TODO: Check if we need to disable the hotspot
	if (_room->findHotspotByExtra(1)->isEnabled &&
		_room->findHotspotByExtra(2)->isEnabled &&
		_room->findHotspotByExtra(3)->isEnabled) {
		_room->disableHotspot(hotspot);
	}
}

void PelrockEngine::closeRoomDoor(HotSpot *hotspot) {
	closeDoor(hotspot, 0, 93, FEMININE, true);
}

void PelrockEngine::pickUpPhoto(HotSpot *hotspot) {
	_room->enableHotspot(_room->findHotspotByExtra(261));
}

void PelrockEngine::pickYellowBook(HotSpot *hotspot) {
	_room->addSticker(95);
}

void PelrockEngine::pickUpBrick(HotSpot *hotspot) {
	_room->addSticker(133);
}

void PelrockEngine::openIceCreamShopDoor(HotSpot *hotspot) {
	_dialog->say(_res->_ingameTexts[HELADERIA_CERRADA]);
}

void PelrockEngine::closeRoomDrawer(HotSpot *hotspot) {
	if (!_room->hasSticker(91)) {
		_dialog->say(_res->_ingameTexts[YA_CERRADO_M]);
		return;
	}
	_room->removeSticker(91);
	_room->enableHotspot(hotspot);
}

void PelrockEngine::useCardWithATM(int inventoryObject, HotSpot *hotspot) {
	debug("Withdrawing money from ATM using card (inv obj %d)", inventoryObject);
	if (_state->getFlag(FLAG_JEFE_INGRESA_PASTA)) {
		_state->setFlag(FLAG_JEFE_INGRESA_PASTA, false);
		addInventoryItem(75);
		_state->setCurrentRoot(20, 2);
	} else {
		int billCount = 0;
		for (uint i = 0; i < _state->inventoryItems.size(); i++) {
			if (_state->inventoryItems[i] == 5) {
				billCount++;
			}
		}
		if (billCount < 13) {
			addInventoryItem(5); // 1000 pesetas bill
			_dialog->say(_res->_ingameTexts[TEAPETECE_BUENRATO]);
		} else {
			_dialog->say(_res->_ingameTexts[NOTENGOMASDINERO]);
		}
	}
}

void PelrockEngine::openMcDoor(HotSpot *hotspot) {
	openDoor(hotspot, 2, 7, FEMININE, false);
}

void PelrockEngine::closeMcDoor(HotSpot *hotspot) {
	// FIXME: Impossible to close right now
	closeDoor(hotspot, 2, 7, FEMININE, false);
}

void PelrockEngine::pickUpAndDisable(HotSpot *hotspot) {
	addInventoryItem(hotspot->extra);
	_room->disableHotspot(hotspot);
}

void PelrockEngine::grabKetchup(HotSpot *hotspot) {
	_room->addSticker(70);
}

void PelrockEngine::grabMustard(HotSpot *hotspot) {
	_room->addSticker(72);
}

void PelrockEngine::grabSpicey(HotSpot *hotspot) {
	_room->addSticker(71);
}

void PelrockEngine::openKitchenDoor(HotSpot *hotspot) {
	openDoor(hotspot, 1, 32, MASCULINE, true);
}

void PelrockEngine::closeKitchenDoor(HotSpot *HotSpot) {
	closeDoor(HotSpot, 1, 32, MASCULINE, true);
}

void PelrockEngine::openKitchenDrawer(HotSpot *hotspot) {
	if (!_state->getFlag(FLAG_JEFE_ENCARCELADO)) {
		_dialog->say(_res->_ingameTexts[QUITA_ESAS_MANOS]);
	} else {
		_room->addSticker(36);
		addInventoryItem(63); // Add recipe
		_dialog->say(_res->_ingameTexts[QUESESTO_RECETA]);
	}
}

void PelrockEngine::openKitchenDoorFromInside(HotSpot *hotspot) {
	openDoor(hotspot, 0, 34, MASCULINE, true);
}

void PelrockEngine::useSpicySauceWithBurger(int inventoryObject, HotSpot *hotspot) {
	_state->setFlag(FLAG_PUESTA_SALSA_PICANTE, true);
	_sound->playSound(_room->_roomSfx[2]);
	_dialog->say(_res->_ingameTexts[VAESTAR_POCOFUERTE]);
}

void PelrockEngine::openShopDoor(HotSpot *hotspot) {
	if (!_state->getFlag(FLAG_TIENDA_ABIERTA)) {
		_dialog->say(_res->_ingameTexts[TIENDA_CERRADA]);
		return;
	} else {
		openDoor(hotspot, 0, 13, MASCULINE, false);
	}
}

void PelrockEngine::closeShopDoor(HotSpot *hotspot) {
	closeDoor(hotspot, 0, 13, MASCULINE, false);
}

void PelrockEngine::openLamppost(HotSpot *hotspot) {
	debug("Opening lamppost");
	_room->addSticker(14);
	_room->moveHotspot(_room->findHotspotByExtra(308), 519, 363);
}

void PelrockEngine::useBrickWithWindow(int inventoryObject, HotSpot *hotspot) {

	// TODO: Play Alfred's throwing animation
	// This would require adding a new special animation entry
	_res->loadAlfredSpecialAnim(4);
	_alfredState.animState = ALFRED_SPECIAL_ANIM;
	waitForSpecialAnimation();

	// TODO: Animate sprite 8 (brick projectile) moving to window
	Sprite *brickSprite = _room->findSpriteByIndex(7);
	HotSpot *windowHotspot = _room->findHotspotByExtra(294);
	brickSprite->x = 420;
	brickSprite->y = 241;
	brickSprite->zOrder = 10; // Make it visible
	int target = windowHotspot->y + windowHotspot->h / 2;
	while (!shouldQuit()) {
		_events->pollEvent();
		renderScene(OVERLAY_NONE);
		// if (_chrono->_gameTick) {
		_room->findSpriteByIndex(7)->y -= 10;
		if (_room->findSpriteByIndex(7)->y <= 70) {
			_room->findSpriteByIndex(7)->zOrder = -1;
			break;
		}
		// }
		_screen->update();
		g_system->delayMillis(10);
	}
	// This would involve loading and animating the room sprite

	// Add the broken window sticker
	_room->addSticker(11);
	_sound->playSound(_room->_roomSfx[2]); // Play glass breaking sound

	// Remove brick from inventory
	_state->removeInventoryItem(4);

	int16 x = 639; // put at the very edge of the screen so it gets adjusted
	int16 y = windowHotspot->y;
	// Play the NPC dialog sequence
	int16 dialog1y = y + 22;
	int16 dialog2y = dialog1y + 10 + _largeFont->getFontHeight();
	_dialog->say(_res->_ingameTexts[QUEHASIDOESO], x, dialog1y);
	_dialog->say(_res->_ingameTexts[QUIENANDAAHI], x, dialog2y);
	_dialog->say(_res->_ingameTexts[YOMEVOY]);

	_state->setFlag(FLAG_TIENDA_ABIERTA, true);
	_room->addStickerToRoom(_room->_currentRoomNumber, 9, PERSIST_PERM);
	_room->addStickerToRoom(_room->_currentRoomNumber, 10, PERSIST_PERM);
	_room->disableHotspot(_room->findHotspotByExtra(295)); // Disable storefront hotspot
	_room->disableHotspot(_room->findHotspotByExtra(294)); // Disable window hotspot
	walkTo(630, _alfredState.y);
}

void PelrockEngine::moveCable(HotSpot *hotspot) {
	_room->addSticker(15);
	_room->addSticker(16);
	_room->addSticker(17);
	_room->addStickerToRoom(4, 20); // Room 4, sticker 20
	_state->setFlag(FLAG_CABLES_PUESTOS, true);
}

void PelrockEngine::useBrickWithShopWindow(int inventoryObject, HotSpot *hotspot) {
	_dialog->say(_res->_ingameTexts[NOSE_ENTERARIA]);
}

void PelrockEngine::pickGuitar(HotSpot *hotspot) {
	buyFromStore(hotspot, 38);
}

void PelrockEngine::pickFish(HotSpot *hotspot) {
	buyFromStore(hotspot, 39);
}

void PelrockEngine::pickTeddyBear(HotSpot *hotspot) {
	buyFromStore(hotspot, 40);
}

void PelrockEngine::pickDiscs(HotSpot *hotspot) {
	buyFromStore(hotspot, 41);
}

void PelrockEngine::pickMonkeyBrain(HotSpot *hotspot) {
	buyFromStore(hotspot, 42);
}

void PelrockEngine::pickBooks(HotSpot *hotspot) {
	buyFromStore(hotspot, 43);
}

void PelrockEngine::pickPalette(HotSpot *hotspot) {
	buyFromStore(hotspot, 44);
}

void PelrockEngine::pickCandy(HotSpot *hotspot) {
	buyFromStore(hotspot, 45);
}

void PelrockEngine::pickConch(HotSpot *hotspot) {
	buyFromStore(hotspot, 46);
}

void PelrockEngine::pickHat(HotSpot *hotspot) {
	buyFromStore(hotspot, 47);
}

void PelrockEngine::pickCord(HotSpot *hotspot) {
	buyFromStore(hotspot, 48);
}

void PelrockEngine::pickAmulet(HotSpot *hotspot) {
	buyFromStore(hotspot, 49);
}

void PelrockEngine::openPlug(HotSpot *hotspot) {
	_room->addSticker(18);
}

void PelrockEngine::useCordWithPlug(int inventoryObject, HotSpot *hotspot) {
	if (!_room->hasSticker(18)) {
		_dialog->say(_res->_ingameTexts[PRIMERO_ABRIRLO]);
	} else {
		debug("Flag is %d", _state->getFlag(FLAG_CABLES_PUESTOS));
		if (_state->getFlag(FLAG_CABLES_PUESTOS)) {
			_room->addSticker(19);
			_room->moveHotspot(_room->findHotspotByIndex(6), 391, 381);
		}
	}
}

void PelrockEngine::pickCables(HotSpot *hotspot) {
	if (_room->hasSticker(21)) {
		_dialog->say(_res->_ingameTexts[QUELOSCOJA_SUPADRE]);
		return;
	}
	// Duck to pick cables
	_res->loadAlfredSpecialAnim(2);
	_alfredState.animState = ALFRED_SPECIAL_ANIM;
	waitForSpecialAnimation();

	// electric shock
	int prevX = _alfredState.x;
	_alfredState.x -= 20;
	_res->loadAlfredSpecialAnim(3);
	_alfredState.animState = ALFRED_SPECIAL_ANIM;
	waitForSpecialAnimation();
	_alfredState.x = prevX;

	// Stand up (reverse of duck)
	_res->loadAlfredSpecialAnim(2, true);
	_alfredState.animState = ALFRED_SPECIAL_ANIM;
	waitForSpecialAnimation();
	_room->addSticker(21);

	_dialog->say(_res->_ingameTexts[RELOJ_HA_CAMBIADO]);
	_state->setCurrentRoot(4, 1);
}

void PelrockEngine::giveIdToGuard(int inventoryObject, HotSpot *hotspot) {
	if (!_state->getFlag(FLAG_GUARDIA_PIDECOSAS)) {
		_dialog->say(_res->_ingameTexts[CUANDOMELOPIDA]);
		return;
	}

	if (!_state->getFlag(FLAG_GUARDIA_DNI_ENTREGADO)) {
		_state->setFlag(FLAG_GUARDIA_DNI_ENTREGADO, true);
		_dialog->say(_res->_ingameTexts[DEACUERDO]);
	}
	if (_state->getFlag(FLAG_SOBORNO_PORTERO) && _state->getFlag(FLAG_GUARDIA_DNI_ENTREGADO)) {
		unlockMuseum();
		return;
	}
}

void PelrockEngine::unlockMuseum() {
	_state->setCurrentRoot(4, 3);
	_room->enableSprite(2, 100, PERSIST_PERM);
	_room->enableSprite(3, 100, PERSIST_PERM);
	_room->addStickerToRoom(4, 87, PERSIST_PERM);
	_room->addStickerToRoom(4, 88, PERSIST_PERM);
	_room->addStickerToRoom(4, 89, PERSIST_PERM);
	_room->addStickerToRoom(4, 90, PERSIST_PERM);
}

void PelrockEngine::giveMoneyToGuard(int inventoryObject, HotSpot *hotspot) {
	if (!_state->getFlag(FLAG_GUARDIA_PIDECOSAS)) {
		_dialog->say(_res->_ingameTexts[PRETENDEUSTED_SOBORNARME]);
		return;
	} else if (!_state->getFlag(FLAG_SOBORNO_PORTERO)) {
		_state->setFlag(FLAG_SOBORNO_PORTERO, true);
		_dialog->say(_res->_ingameTexts[MUYBIEN]);
		_state->removeInventoryItem(5); // Remove 1000 pesetas bill
	}
	if (_state->getFlag(FLAG_SOBORNO_PORTERO) && _state->getFlag(FLAG_GUARDIA_DNI_ENTREGADO)) {
		unlockMuseum();
		return;
	}
}

void PelrockEngine::openMuseumDoor(HotSpot *hotspot) {
	if (!_state->getFlag(FLAG_GUARDIA_PIDECOSAS)) {
		_dialog->say(_res->_ingameTexts[ALTO]);
		return;
	} else if (!_state->getFlag(FLAG_GUARDIA_DNI_ENTREGADO)) {
		_dialog->say(_res->_ingameTexts[NECESITODNI]);
	} else if (!_state->getFlag(FLAG_SOBORNO_PORTERO)) {
		_dialog->say(_res->_ingameTexts[QUE_RECIBO_ACAMBIO]);
	} else {
		openDoor(hotspot, 1, 22, FEMININE, false);
	}
}

void PelrockEngine::useAmuletWithStatue(int inventoryObject, HotSpot *hotspot) {

	if (!_room->hasSticker(24)) {
		_room->addSticker(24);
		_state->removeInventoryItem(7);
		_state->setCurrentRoot(7, 1);
		_alfredState.direction = ALFRED_RIGHT;

		HotSpot *statueHotspot = _room->findHotspotByExtra(91);
		_currentHotspot = statueHotspot;
		walkTo(statueHotspot->x + statueHotspot->w / 2, statueHotspot->y + statueHotspot->h);
		animateStatuePaletteFade(false);
		walkAndAction(statueHotspot, TALK);
		waitForActionEnd();
		animateStatuePaletteFade(true);
	}
}

void PelrockEngine::useSecretCodeWithStatue(int inventoryObject, HotSpot *hotspot) {
	_dialog->say(_res->_ingameTexts[NOESAMIAQUIENDEBES], 1);
}

void PelrockEngine::pickUpLetter(HotSpot *hotspot) {
	addInventoryItem(9);
	_room->setActionMask(hotspot, ACTION_MASK_NONE); // Disable hotspot
}

void PelrockEngine::openLibraryOutdoorsDoor(HotSpot *hotspot) {
	openDoor(hotspot, 0, 26, FEMININE, false);
}

void PelrockEngine::closeLibraryOutdoorsDoor(HotSpot *hotspot) {
	closeDoor(hotspot, 0, 26, FEMININE, false);
}

void PelrockEngine::openLibraryIndoorsDoor(HotSpot *hotspot) {
	openDoor(hotspot, 0, 28, FEMININE, false);
}

void PelrockEngine::closeLibraryIndoorsDoor(HotSpot *hotspot) {
	closeDoor(hotspot, 0, 28, FEMININE, false);
}

void PelrockEngine::pickBooksFromShelf1(HotSpot *hotspot) {
	pickUpBook(1);
}

void PelrockEngine::pickBooksFromShelf2(HotSpot *hotspot) {
	pickUpBook(2);
}

void PelrockEngine::pickBooksFromShelf3(HotSpot *hotspot) {
	pickUpBook(3);
}

void PelrockEngine::giveSecretCodeToLibrarian(int inventoryObject, HotSpot *hotspot) {
	_dialog->say(_res->_ingameTexts[REGALO_LIBRO_RECETAS]);
	_state->removeInventoryItem(8);
	addInventoryItem(59);
}

void PelrockEngine::useBrickWithLibrarian(int inventoryObject, HotSpot *hotspot) {
	_dialog->say(_res->_ingameTexts[YSI_METIRA_MAQUINA]);
}

void PelrockEngine::openNewspaperDoor(HotSpot *hotspot) {
	openDoor(hotspot, 2, 50, MASCULINE, false);
}

void PelrockEngine::closeNewspaperDoor(HotSpot *hotspot) {
	closeDoor(hotspot, 2, 50, MASCULINE, false);
}

void PelrockEngine::openNewspaperBossDor(HotSpot *hotspot) {
	openDoor(hotspot, 1, 52, MASCULINE, true);
}

void PelrockEngine::closeNewspaperBossDoor(HotSpot *hotspot) {
	closeDoor(hotspot, 1, 52, MASCULINE, true);
}

void PelrockEngine::openTravelAgencyDoor(HotSpot *hotspot) {

	if (_state->getFlag(FLAG_AGENCIA_ABIERTA)) {
		openDoor(hotspot, 1, 57, FEMININE, false);
	}
	// The game originally did nothing here
}

void PelrockEngine::closeTravelAgencyDoor(HotSpot *hotspot) {
	closeDoor(hotspot, 1, 57, FEMININE, false);
}

void PelrockEngine::usePumpkinWithRiver(int inventoryObject, HotSpot *hotspot) {
	_sound->playMusicTrack(27);
	_dialog->say(_res->_ingameTexts[PRIMERINGREDIENTE]);
	_dialog->say(_res->_ingameTexts[CUIDADOIMPRUDENTE]);
	_res->loadAlfredSpecialAnim(5);
	_alfredState.animState = ALFRED_SPECIAL_ANIM;
	_alfredState.x -= 10;
	_alfredState.y += 20;
	waitForSpecialAnimation();
	_sound->playSound(_room->_roomSfx[0], 100, 0); // Belch
	bool isPlaying = true;
	while (!shouldQuit() && isPlaying) {
		_events->pollEvent();
		isPlaying = _sound->isPlaying(0);
		_screen->update();
		g_system->delayMillis(10);
	}
	_graphics->fadeToBlack(10);
	// update conversaton state
	_alfredState.x = 300;
	_alfredState.y = 238;
	setScreen(28, ALFRED_DOWN);
	_dialog->say(_res->_ingameTexts[QUEOSCUROESTAESTO]);
}

void PelrockEngine::pickupSunflower(HotSpot *hotspot) {
	if (_state->getFlag(FLAG_PARADOJA_RESUELTA) == false) {
		_dialog->say(_res->_ingameTexts[OIGA]);
		_state->setCurrentRoot(25, 26);
		_state->setFlag(FLAG_RIDDLE_PRESENTED, true);
		walkAndAction(_room->findHotspotByExtra(467), TALK);
	} else {
		addInventoryItem(85);
		_room->disableHotspot(hotspot);
		_state->setCurrentRoot(25, 1);
		_room->addSticker(73);
	}
	checkIngredients();
}

void PelrockEngine::checkIngredients() {
	byte ingredientes = _state->getFlag(FLAG_INGREDIENTES_CONSEGUIDOS);
	int textLine = PRIMERINGREDIENTE + ingredientes;
	_dialog->say(_res->_ingameTexts[textLine]);
	_state->setFlag(FLAG_INGREDIENTES_CONSEGUIDOS, ingredientes + 1);
}

void PelrockEngine::pickUpBook(int i) {
	if (!_state->hasInventoryItem(10)) {
		_dialog->say(_res->_ingameTexts[VENGA_ACA]);
		_state->setCurrentRoot(9, 1);

		if (_state->hasInventoryItem(3)) {
			_state->setCurrentRoot(9, 2);
			addInventoryItem(10);
		}

		_alfredState.isWalkingCancelable = false;
		walkAndAction(_room->findHotspotByExtra(102), TALK);
		// After dialog ends, reenable first dialog root if no photo in inventory
		// Wait for dialog to end to reenable first dialog root
		waitForActionEnd();
		if (!_state->hasInventoryItem(3)) {

			_state->setCurrentRoot(9, 0);
		} else {
			_state->setCurrentRoot(9, 3);
		}
	} else {
		if (_state->libraryShelf == -1) {
			_dialog->say(_res->_ingameTexts[TODOS]);
		} else if (_state->libraryShelf != i) {
			_dialog->say(_res->_ingameTexts[EL_LIBRO_NOESTA_AQUI]);
		} else {
			_state->libraryShelf = -1;
			int booksInInventory = _state->booksInInventory();
			if (booksInInventory == 3) {
				int firstBook = _state->findFirstBookIndex();
				if (firstBook != -1)
					_state->removeInventoryItem(firstBook);
				_dialog->say(_res->_ingameTexts[TENDRE_DEJAR_LIBRO]);
			}
			addInventoryItem(_state->selectedBookIndex);
			_state->selectedBookIndex = -1;
		}
	}
}

void PelrockEngine::pickUpChainsaw(HotSpot *hotspot) {

	_room->addSticker(99);
}

void PelrockEngine::pickUpSpellbook(HotSpot *hotspot) {
	_room->addSticker(97);
}

void PelrockEngine::pickUpBoot(HotSpot *hotspot) {

	_room->addSticker(98);
}

void PelrockEngine::pickupCondoms(HotSpot *hotspot) {

	_room->addSticker(100);
}

void PelrockEngine::performActionTrigger(uint16 actionTrigger) {
	debug("Performing action trigger: %d", actionTrigger);
	switch (actionTrigger) {
	case 257: // look portrait
		_sound->playMusicTrack(25);
		loadExtraScreenAndPresent(9);
		_dialog->say(_res->_ingameTexts[QUEBUENA_ESTA]);
		_screen->markAllDirty();
		_screen->update();
		break;
	case 268: // look at statue
		_dialog->say(_res->_ingameTexts[TUCREES]);
		break;
	case 271: // look at librarian
		_dialog->say(_res->_ingameTexts[TRABAJARIA_MEJOR_SI_NO_ME_MOLESTARA]);
		break;
	case 270:
		// loadExtraScreenAndPresent(9);
		_state->stateGame = COMPUTER;
		break;
	case 280:
		_dialog->say(_res->_ingameTexts[NOVIO2METROS]);
		break;
	case 281:
		_dialog->say(_res->_ingameTexts[GRANIDEA]);
		break;
	case 282:
		_dialog->say(_res->_ingameTexts[SELORECOMIENDO]);
		break;
	}
}

void PelrockEngine::useOnAlfred(int inventoryObject) {

	debug("Using item %d on Alfred", inventoryObject);
	switch (inventoryObject) {
	case 9: // carta
		_dialog->say(_res->_ingameTexts[CORRESPONDENCIA_AJENA]);
		break;
	case 34: // Como hacerse rico...
		_dialog->say(_res->_ingameTexts[PERIODICOSENSACIONALISTA], 1);
		break;
	case 63: // Recipe
		_res->loadAlfredSpecialAnim(1);
		_alfredState.animState = ALFRED_SPECIAL_ANIM;
		waitForSpecialAnimation();

		loadExtraScreenAndPresent(3);
		_state->setCurrentRoot(17, 1);
		_state->setCurrentRoot(18, 4);
		debug("After extra screen");
		_dialog->say(_res->_ingameTexts[QUEASCO]);
		break;
	case 59: // Recipe book
		if (!_state->hasInventoryItem(64)) {
			_res->loadAlfredSpecialAnim(0);
			_alfredState.animState = ALFRED_SPECIAL_ANIM;
			waitForSpecialAnimation();
			_dialog->say(_res->_ingameTexts[HOJAENTREPAGINAS]);
			addInventoryItem(64);
		} else {
			_res->loadAlfredSpecialAnim(0);
			_alfredState.animState = ALFRED_SPECIAL_ANIM;
			waitForSpecialAnimation();
			_dialog->say(_res->_ingameTexts[NOENTIENDONADA]);
		}
		break;
	case 17: // Egyptian book
		_res->loadAlfredSpecialAnim(0);
		_alfredState.animState = ALFRED_SPECIAL_ANIM;
		waitForSpecialAnimation();
		_dialog->say(_res->_ingameTexts[YASEEGIPCIO]);
		_state->setFlag(FLAG_ALFRED_SABE_EGIPCIO, true);
		break;
	case 24:
		if (_state->getFlag(FLAG_RIDDLE_PRESENTED) == true) {
			_dialog->say(_res->_ingameTexts[CAPITULOPARADOJAS]);
			_state->setCurrentRoot(25, 44);
		} else {
			_res->loadAlfredSpecialAnim(0);
			_alfredState.animState = ALFRED_SPECIAL_ANIM;
			waitForSpecialAnimation();
			_dialog->say(_res->_ingameTexts[COSASAPRENDIDO]);
			_state->setFlag(FLAG_ALFRED_INTELIGENTE, true);
			_state->setCurrentRoot(14, 2);
		}
		break;
	case 64:
		_res->loadAlfredSpecialAnim(0);
		_alfredState.animState = ALFRED_SPECIAL_ANIM;
		waitForSpecialAnimation();
		loadExtraScreenAndPresent(5);
		if (_state->getFlag(FLAG_ALFRED_SABE_EGIPCIO)) {
			_dialog->say(_res->_ingameTexts[FORMULAVIAJETIEMPO]);
		} else {
			_dialog->say(_res->_ingameTexts[QUELASTIMA_NOSEEGIPCIO]);
		}
		break;
	case 0: // yellow book
		_res->loadAlfredSpecialAnim(0);
		_alfredState.animState = ALFRED_SPECIAL_ANIM;
		waitForSpecialAnimation();
		_dialog->say(_res->_ingameTexts[CUENTOPARECIDO]);
		break;
	default:
		break;
	}
}

void PelrockEngine::animateStatuePaletteFade(bool reverse) {
	// Structure at JUEGO.EXE offset 0x4C700
	struct StatuePaletteData {
		uint16 x;           // 368
		uint16 y;           // 148
		uint16 type;        // 2
		uint16 padding;     // 0
		byte indices[16];   // Palette indices to modify
		byte source[16][3]; // Source RGB values (6-bit VGA)
		byte target[16][3]; // Target RGB values (6-bit VGA)
	};

	Common::File exeFile;
	if (!exeFile.open("JUEGO.EXE")) {
		warning("Could not open JUEGO.EXE for statue palette animation");
		return;
	}

	// Read the palette data structure from JUEGO.EXE
	exeFile.seek(0x4C700, SEEK_SET);

	StatuePaletteData paletteData;
	paletteData.x = exeFile.readUint16LE();
	paletteData.y = exeFile.readUint16LE();
	paletteData.type = exeFile.readUint16LE();
	paletteData.padding = exeFile.readUint16LE();

	exeFile.read(paletteData.indices, 16);

	for (int i = 0; i < 16; i++) {
		paletteData.source[i][0] = exeFile.readByte();
		paletteData.source[i][1] = exeFile.readByte();
		paletteData.source[i][2] = exeFile.readByte();
	}

	for (int i = 0; i < 16; i++) {
		paletteData.target[i][0] = exeFile.readByte();
		paletteData.target[i][1] = exeFile.readByte();
		paletteData.target[i][2] = exeFile.readByte();
	}

	exeFile.close();

	// Animation parameters
	const int kNumFrames = 7; // 7 step updates total
	const int kDelayMs = 200; // ~12 ticks at 60Hz (~200ms)

	// Get current palette
	byte currentPalette[768];
	memcpy(currentPalette, _room->_roomPalette, 768);

	// Perform the fade animation
	int frame = 0;
	while (!shouldQuit() && frame <= kNumFrames) {
		_events->pollEvent();

		bool didRender = renderScene(OVERLAY_NONE);
		if (didRender) {

			for (int i = 0; i < 16; i++) {
				byte paletteIndex = paletteData.indices[i];

				// Determine source and target based on direction
				byte *srcColor = reverse ? paletteData.target[i] : paletteData.source[i];
				byte *dstColor = reverse ? paletteData.source[i] : paletteData.target[i];

				// Linear interpolation (6-bit VGA values)
				byte r6 = srcColor[0] + ((dstColor[0] - srcColor[0]) * frame) / kNumFrames;
				byte g6 = srcColor[1] + ((dstColor[1] - srcColor[1]) * frame) / kNumFrames;
				byte b6 = srcColor[2] + ((dstColor[2] - srcColor[2]) * frame) / kNumFrames;

				// Convert 6-bit VGA (0-63) to 8-bit (0-255) by shifting left 2 bits
				currentPalette[paletteIndex * 3 + 0] = r6 << 2;
				currentPalette[paletteIndex * 3 + 1] = g6 << 2;
				currentPalette[paletteIndex * 3 + 2] = b6 << 2;
			}

			// Apply the palette
			g_system->getPaletteManager()->setPalette(currentPalette, 0, 256);
			frame++;
		}
		_screen->update();
		g_system->delayMillis(10);
	}
}
/**
 * In order to unlock the second part of the game, we need to ensure
 * we have all we need to solve the game once there
 */
void PelrockEngine::checkObjectsForPart2() {
	if (_state->hasInventoryItem(17) &&
		_state->hasInventoryItem(59) &&
		_state->hasInventoryItem(24)) {
		_room->addStickerToRoom(19, 54, PERSIST_BOTH);
		_room->addStickerToRoom(19, 55, PERSIST_BOTH);
		_room->addStickerToRoom(19, 56, PERSIST_BOTH);
		_room->addStickerToRoom(19, 58, PERSIST_BOTH);
		_state->setFlag(FLAG_AGENCIA_ABIERTA, true);
	}
}

void PelrockEngine::waitForActionEnd() {
	while (!shouldQuit() && _queuedAction.isQueued) {
		_events->pollEvent();
		renderScene(OVERLAY_NONE);
		_screen->update();
	}
}

/**
 * Handler for picking up object with extra_id 472 in Room 28.
 * Loads a special palette from ALFRED.7 at offset 0x1610CE and
 * fades to it using the step-wise palette transition.
 */
void PelrockEngine::pickUpMatches(HotSpot *hotspot) {
	// Load the special palette from ALFRED.7 at offset 0x1610CE
	static const uint32 kRoom28PaletteOffset = 0x1610CE;

	Common::File alfred7;
	if (!alfred7.open(Common::Path("ALFRED.7"))) {
		warning("Could not open ALFRED.7 for room 28 palette");
		return;
	}

	byte targetPalette[768];
	alfred7.seek(kRoom28PaletteOffset, SEEK_SET);
	alfred7.read(targetPalette, 768);
	alfred7.close();

	// Convert 6-bit VGA palette (0-63) to 8-bit (0-255)
	for (int i = 0; i < 768; i++) {
		targetPalette[i] = targetPalette[i] << 2;
	}

	// Fade from current palette to the new palette
	_graphics->fadePaletteToTarget(targetPalette, 25);
	debug("Finished palette fade for room 28 object pickup");
	// Pick up the item
	_room->disableHotspot(hotspot);
	_state->setFlag(FLAG_CROCODILLO_ENCENDIDO, true);
	_room->moveHotspot(_room->findHotspotByExtra(87), 415, 171);
	_room->moveHotspot(_room->findHotspotByExtra(88), 305, 217);
	_room->moveHotspot(_room->findHotspotByExtra(89), 201, 239);
	_room->moveHotspot(_room->findHotspotByExtra(112), 261, 259);
}

/**
 * Original behavior:
 * 1. Stop all sound
 * 2. Loop: corrupt the background buffer pointer with random values,
 *    copy garbage to screen, write sequential memory bytes to PC speaker
 *    port 0x61 to produce noise
 * 3. On keypress: divide by zero -> crash to DOS
 *
 * ScummVM behavior:
 * 1. Stop all sound
 * 2. Loop: fill screen with random pixels, play white noise
 * 3. On keypress: return to launcher
 */
void PelrockEngine::antiPiracyEffect() {
	_sound->stopAllSounds();
	_sound->stopMusic();

	// Generate a buffer of white noise for the PC speaker simulation
	const int kNoiseLength = 16000;                // 1 second at 8kHz
	byte *noiseData = new byte[kNoiseLength + 44]; // WAV header + data

	// Write a minimal WAV header
	memcpy(noiseData, "RIFF", 4);
	WRITE_LE_UINT32(noiseData + 4, kNoiseLength + 36);
	memcpy(noiseData + 8, "WAVE", 4);
	memcpy(noiseData + 12, "fmt ", 4);
	WRITE_LE_UINT32(noiseData + 16, 16);   // chunk size
	WRITE_LE_UINT16(noiseData + 20, 1);    // PCM format
	WRITE_LE_UINT16(noiseData + 22, 1);    // mono
	WRITE_LE_UINT32(noiseData + 24, 8000); // sample rate
	WRITE_LE_UINT32(noiseData + 28, 8000); // byte rate
	WRITE_LE_UINT16(noiseData + 32, 1);    // block align
	WRITE_LE_UINT16(noiseData + 34, 8);    // bits per sample
	memcpy(noiseData + 36, "data", 4);
	WRITE_LE_UINT32(noiseData + 40, kNoiseLength);

	// Fill with random noise (simulating garbage bytes written to port 0x61)
	byte curNoise = (byte)getRandomNumber(255);
	for (int i = 0; i < kNoiseLength; i++) {
		noiseData[44 + i] = curNoise;
		bool changeNoise = getRandomNumber(10) < 2; // 20% chance to change noise value
		if (changeNoise) {
			curNoise = (byte)getRandomNumber(255);
		}
	}

	// Play the noise
	_sound->playSound(noiseData, kNoiseLength + 44, 200);

	byte *screenPixels = (byte *)_screen->getPixels();
	int screenSize = _screen->pitch * _screen->h;

	// Clear any pending key event before starting the loop
	_events->_lastKeyEvent = Common::KEYCODE_INVALID;

	while (!shouldQuit()) {
		_events->pollEvent();

		if (_events->_lastKeyEvent != Common::KEYCODE_INVALID) {
			break;
		}

		// generate random pixels on the screen (simulating corrupted video memory)
		for (int i = 0; i < screenSize; i++) {
			screenPixels[i] = (byte)getRandomNumber(255);
		}

		// Regenerate noise periodically
		for (int i = 0; i < kNoiseLength; i++) {
			noiseData[44 + i] = curNoise;
			bool changeNoise = getRandomNumber(10) < 2; // 20% chance to change noise value
			if (changeNoise) {
				curNoise = (byte)getRandomNumber(255);
			}
		}
		if (!_sound->isPlaying()) {
			_sound->playSound(noiseData, kNoiseLength + 44, 200);
		}

		_screen->markAllDirty();
		_screen->update();
		g_system->delayMillis(50);
	}

	_sound->stopAllSounds();
	delete[] noiseData;

	// Return to launcher
	Engine::quitGame();
}

} // End of namespace Pelrock
