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

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/mixer.h"
#include "graphics/paletteman.h"

#include "pelrock.h"
#include "pelrock/actions.h"
#include "pelrock/backgroundbook.h"
#include "pelrock/cdplayer.h"
#include "pelrock/offsets.h"
#include "pelrock/pelrock.h"
#include "pelrock/spellbook.h"
#include "pelrock/util.h"

namespace Pelrock {

static const uint32 kStatuePaletteDataOffset = 0x4C700; // JUEGO.EXE — statue palette animation data

#define MASCULINE true
#define FEMININE false

const ActionEntry actionTable[] = {
	// Room 0
	{261, OPEN, &PelrockEngine::openRoomDrawer},
	{261, CLOSE, &PelrockEngine::closeRoomDrawer},
	{263, OPEN, &PelrockEngine::openClosedDrawer},

	{268, OPEN, &PelrockEngine::openRoomDoor},
	{268, CLOSE, &PelrockEngine::closeRoomDoor},
	{3, PICKUP, &PelrockEngine::pickUpPhoto},
	{0, PICKUP, &PelrockEngine::pickYellowBook}, // Generic pickup for other items
	// Room 1
	{4, PICKUP, &PelrockEngine::pickUpBrick}, // Brick
	{277, OPEN, &PelrockEngine::openIceCreamShopDoor},
	{273, PICKUP, &PelrockEngine::pickupGarbageCan},
	// Room 2
	{282, OPEN, &PelrockEngine::openMcDoor},
	{282, CLOSE, &PelrockEngine::closeMcDoor},
	{283, PICKUP, &PelrockEngine::pickupBush},
	{284, PICKUP, &PelrockEngine::pickupBush},

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
	{288, OPEN, &PelrockEngine::openLamppost},
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
	{312, OPEN, &PelrockEngine::openMuseumDoor},
	{316, PICKUP, &PelrockEngine::pickCables},
	{312, CLOSE, &PelrockEngine::closeMuseumDoor},
	{310, PICKUP, &PelrockEngine::pickupFruit},
	{311, PICKUP, &PelrockEngine::pickupFruit},

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

	// Room 29
	{434, OPEN, &PelrockEngine::openEgyptMuseumDoor},
	{434, CLOSE, &PelrockEngine::closeEgyptMuseumDoor},

	// Room 30
	{435, PUSH, &PelrockEngine::pushSymbol1},
	{436, PUSH, &PelrockEngine::pushSymbol2},
	{437, PUSH, &PelrockEngine::pushSymbol3},
	{438, PUSH, &PelrockEngine::pushSymbol4},

	// Room 38
	{81, PICKUP, &PelrockEngine::pickUpHairStrand},

	// Room 31
	{462, OPEN, &PelrockEngine::openJailFloorTile},
	// Room 32
	{473, OPEN, &PelrockEngine::openTunnelDrawer},
	// Room 33
	{651, OPEN, &PelrockEngine::openSafe},
	{465, OPEN, &PelrockEngine::openTunnelDoor},
	{465, CLOSE, &PelrockEngine::closeTunnelDoor},

	// Room 37
	{90, PICKUP, &PelrockEngine::pickUpStone},

	// Room 39
	{700, PICKUP, &PelrockEngine::swimmingPoolCutscene},

	// Room 41
	{605, PICKUP, &PelrockEngine::pickUpStones},
	{606, PICKUP, &PelrockEngine::pickUpStones},
	{607, PICKUP, &PelrockEngine::pickUpStones},
	{608, PICKUP, &PelrockEngine::pickUpMud},

	// Room 44
	{613, OPEN, &PelrockEngine::openPyramidDoor},

	// Room 46
	{621, OPEN, &PelrockEngine::openArchitectDoor},
	{621, CLOSE, &PelrockEngine::closeArchitectDoor},

	// Room 47
	{628, PICKUP, &PelrockEngine::pickupPyramidMap},
	{800, OPEN, &PelrockEngine::openArchitectDoorFromInside},
	{800, CLOSE, &PelrockEngine::closeArchitectDoorFromInside},

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
	{1, 309, &PelrockEngine::showIdToGuard},
	{5, 309, &PelrockEngine::giveMoneyToGuard},
	{7, 353, &PelrockEngine::useAmuletWithStatue},
	{8, 347, &PelrockEngine::useSecretCodeWithStatue},
	{8, 358, &PelrockEngine::giveSecretCodeToLibrarian},
	{4, 358, &PelrockEngine::useBrickWithLibrarian},
	{76, 469, &PelrockEngine::usePumpkinWithRiver},
	{100, 650, &PelrockEngine::useKeyWithPortrait},
	{83, 461, &PelrockEngine::useDollWithBed},
	{84, 503, &PelrockEngine::giveMagazineToGuard},
	{86, 500, &PelrockEngine::giveWaterToGuard},
	{91, 601, &PelrockEngine::giveStoneToSlaves},
	{92, 601, &PelrockEngine::giveStoneToSlaves},

	// Room 35 (cauldron)
	{90, 506, &PelrockEngine::magicFormula},
	{85, 506, &PelrockEngine::magicFormula},
	{86, 506, &PelrockEngine::magicFormula},
	{81, 506, &PelrockEngine::magicFormula},
	{99, 506, &PelrockEngine::useWigWithPot},

	{76, 617, &PelrockEngine::usePumpkinWithPond},

	{86, 614, &PelrockEngine::useWaterOnFakeStone},
	// End marker
	{WILDCARD, WILDCARD, nullptr}};

/**
 * Convenience methods for open/close doors which usually have the same behavior
 */
void PelrockEngine::openDoor(HotSpot *hotspot, int exitIndex, int sticker, bool masculine, bool stayClosed) {
	if (_room->hasSticker(sticker)) {
		int text = masculine == MASCULINE ? kTextYaAbiertoM : kTextYaAbiertaF;
		_dialog->say(_res->_ingameTexts[text]);
		return;
	}
	_room->enableExit(exitIndex, stayClosed ? PERSIST_TEMP : PERSIST_BOTH);
	_room->addSticker(sticker, stayClosed ? PERSIST_TEMP : PERSIST_BOTH);
	_sound->playSound(_room->_roomSfx[0]);
}

void PelrockEngine::closeDoor(HotSpot *hotspot, int exitIndex, int sticker, bool masculine, bool stayOpen) {
	if (!_room->hasSticker(sticker)) {
		int text = masculine == MASCULINE ? kTextYaCerradoM : kTextYaCerradaF;
		_dialog->say(_res->_ingameTexts[text]);
		return;
	}
	_room->disableExit(exitIndex, stayOpen ? PERSIST_TEMP : PERSIST_BOTH);
	_room->removeSticker(sticker);
	_sound->playSound(_room->_roomSfx[1]);
}

/**
 * Adds item to inventory but also shows the item overlay
 */
void PelrockEngine::addInventoryItem(int item) {
	if (_state->inventoryItems.empty()) {
		_state->selectedInventoryItem = item;
	}
	_newItem = item;
	int frameCounter = 0;
	_state->addInventoryItem(item);
	while (frameCounter < kIconFlashDuration) {
		_events->pollEvent();

		bool didRender = renderScene(OVERLAY_PICKUP_ICON);
		_screen->update();
		if (didRender) {
			frameCounter++;
		}
		g_system->delayMillis(10);
	}

	checkObjectsForPart2();
}

/**
 * Every object from the store removes bill, adds item, cutscene happens.
 */
void PelrockEngine::buyFromStore(HotSpot *hotspot, int stickerId) {
	if (_state->hasInventoryItem(5) == false) {
		_dialog->say(_res->_ingameTexts[kTextNoTengoDinero]);
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
		_dialog->say(_res->_ingameTexts[kTextCuesta1000]);
		_dialog->say(_res->_ingameTexts[kTextAquiTiene]);
		_dialog->say(_res->_ingameTexts[kTextMuyBien]);
		_state->removeInventoryItem(5); // Remove 1000 pesetas bill
	}
}

// F8 commands on conversations
void PelrockEngine::dialogActionTrigger(uint16 actionTrigger, byte room, byte rootIndex) {
	switch (actionTrigger) {
	case 328:
		_state->setCurrentRoot(room, rootIndex + 1, 0);
		break;
	case 329:
		_state->setFlag(FLAG_PUTA_250_VECES, true);
		break;
	case 258:
		_state->setFlag(FLAG_GUARDIA_PIDECOSAS, true);
		_state->setCurrentRoot(4, 2, 0);
		break;
	case 259:
		_dialog->say(_res->_ingameTexts[kTextDePiedraNoDeHielo], 1);
		_dialog->say(_res->_ingameTexts[kTextNoEmpecemos]);
		break;
	case 260:
		_dialog->say(_res->_ingameTexts[kTextCuerpoDanone], 1);
		_dialog->say(_res->_ingameTexts[kTextCabezaHueca]);
		break;
	case 261:
		_dialog->say(_res->_ingameTexts[kTextEsoLoSerasTu], 1);
		break;
	case 262:
		_dialog->say(_res->_ingameTexts[kTextDemasiadoNoPuedoPensar], 1);
		break;
	case 263:
		_dialog->say(_res->_ingameTexts[kTextUnPocoRespeto]);
		break;
	case 264:
		// skip to root after the next one
		_state->setCurrentRoot(room, rootIndex + 2, 0);
		break;
	case 267: {
		_state->setCurrentRoot(7, 2, 0);
		addInventoryItem(8);
		animateStatuePaletteFade(true);
		break;
	}
	case 272:
		_state->setCurrentRoot(room, rootIndex + 1, 0);
		break;
	case 273: {
		WalkBox w1 = {3, 436, 356, 4, 14, 0};
		WalkBox w2 = {4, 440, 368, 148, 2, 0};

		_room->addWalkbox(w1);
		_room->addWalkbox(w2);
		break;
	}
	case 274:
	case 275:
	case 276:
		_state->setCurrentRoot(room, rootIndex + 1, 0);
		break;
	case 277:
		_state->setCurrentRoot(room, rootIndex + 1, 0);
		_state->setFlag(FLAG_JEFE_INGRESA_PASTA, true);
		break;
	case 278:
		_state->setCurrentRoot(room, rootIndex + 1, 0);
		break;
	case 279: {
		_state->removeInventoryItem(75);
		travelToEgypt();

		break;
		// merchants
	}
	case 330:
		// Two oranges
		addInventoryItem(102);
		break;
	case 331:
		_dialog->say(_res->_ingameTexts[kTextHechoElPrimo]);
		break;
	case 332:
		// psychologist card
		if (!_state->hasInventoryItem(104)) {
			addInventoryItem(104);
		}
		break;
	case 333:
		_dialog->say(_res->_ingameTexts[kTextMeHanTomadoElPelo]);
		break;
	case 334:
		addInventoryItem(76);
		_state->setCurrentRoot(room, rootIndex + 1, 0);
		break;
	case 335:
		// many oranges
		addInventoryItem(103);
		break;
	case 336:
		_dialog->say(_res->_ingameTexts[kTextPesadoUnRato]);
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
		_state->setCurrentRoot(room, rootIndex + 1, 0);
		break;
	case 348: {
		antiPiracyEffect();
		break;
	}
	case 349:
		_state->setFlag(FLAG_CONSIGNAS_VENDEDOR, _state->getFlag(FLAG_CONSIGNAS_VENDEDOR) + 1);
		if (_state->getFlag(FLAG_CONSIGNAS_VENDEDOR) == 2) {
			_state->setCurrentRoot(room, rootIndex + 1, 1);
		}
		break;
	case 350:
		_state->setFlag(FLAG_CONSIGNAS_VENDEDOR, _state->getFlag(FLAG_CONSIGNAS_VENDEDOR) + 1);
		if (_state->getFlag(FLAG_CONSIGNAS_VENDEDOR) == 2) {
			_state->setCurrentRoot(room, rootIndex + 1, 1);
		}
		break;
	case 351:
		_dialog->say(_res->_ingameTexts[kTextTraidor], 0);
		_dialog->say(_res->_ingameTexts[kTextTuTia], 1);
		_dialog->say(_res->_ingameTexts[kTextLaTuya], 0);
		_dialog->say(_res->_ingameTexts[kTextGordo], 1);
		_dialog->say(_res->_ingameTexts[kTextFideo], 0);
		_dialog->say(_res->_ingameTexts[kTextLimpiaculos], 1);
		_dialog->say(_res->_ingameTexts[kTextConTuTurbante], 0);
		_dialog->say(_res->_ingameTexts[kTextOso], 1);
		_dialog->say(_res->_ingameTexts[kTextComadreja], 0);
		_dialog->say(_res->_ingameTexts[kTextCabezon], 1);
		_dialog->say(_res->_ingameTexts[kTextTuAbuelo], 0);
		_dialog->say(_res->_ingameTexts[kTextTuMujer], 1);
		_dialog->say(_res->_ingameTexts[kTextPerdedor], 0);
		_dialog->say(_res->_ingameTexts[kTextSoyMejorQueTu], 1);
		_dialog->say(_res->_ingameTexts[kTextTramposo], 0);
		_dialog->say(_res->_ingameTexts[kTextMalPerdedor], 1);
		_dialog->say(_res->_ingameTexts[kTextParaUnaVez], 0);
		_dialog->say(_res->_ingameTexts[kTextMejorMeLargo], 1);
		break;
		// end merchants
	case 353:
		_state->setCurrentRoot(room, rootIndex + 2, 0);
		break;
	case 354:
		if (_state->hasInventoryItem(105)) {
			addInventoryItem(105);
		}
		break;
	case 352:
	case 355:
	case 291:
		toJail();
		break;
	case 356:
		_state->setCurrentRoot(room, 3, 0);
		break;
	// end hooker
	//  hermit
	case 366:
		_state->setCurrentRoot(room, rootIndex + 1, 0);
		break;
	case 363:
		toJail();
		break;
	case 367: // accept riddle
		_state->setCurrentRoot(room, 27, 0);
		walkAndAction(_room->findHotspotByExtra(467), TALK);
		waitForActionEnd();
		break;

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
		_state->setCurrentRoot(room, rootIndex + 1, 0);
		advanceQuotesConversation(rootIndex, room);
		break;
	}
	case 361: // "no sé" (I don't know): no counter change, just advance
		_state->setCurrentRoot(room, rootIndex + 1, 0);
		break;
	case 362:
		_state->setFlag(FLAG_CHEAT_CODE_ENABLED, 1);
		advanceQuotesConversation(rootIndex, room);
		break;
	case 364: // riddle wrong answer: advance to next riddle
	{
		int targetIndex = rootIndex + 1;
		if (rootIndex == 43) {
			targetIndex = 27; // skip riddle explanation
		}
		_state->setCurrentRoot(room, targetIndex, 0);
		break;
	}
	case 365: // riddle correct: set riddle-solved flag
		_state->setFlag(FLAG_PARADOJA_RESUELTA, 1);
		_state->setCurrentRoot(room, 1, 0);
		break;
	case 292:
		_state->setCurrentRoot(room, rootIndex + 1, 0);
		break;
	case 293:
		_room->disableHotspot(_room->findHotspotByExtra(451)); // Disable talk hotspot for the riddle
		_state->setCurrentRoot(room, rootIndex + 1, 0);
		break;
	// merchants
	case 370:
		addInventoryItem(111);
		break;
	case 371:
		addInventoryItem(111);
		addInventoryItem(110);
		_state->setCurrentRoot(room, rootIndex + 1, 0);
		break;
	case 286:
		addInventoryItem(83);
		_state->setCurrentRoot(room, rootIndex + 1, 0);
		break;
	case 287:
		addInventoryItem(77);
		addInventoryItem(107);
		givenItems();
		break;
	case 288:
		addInventoryItem(78);
		givenItems();
		break;
	case 289:
		addInventoryItem(79);
		addInventoryItem(108);
		givenItems();
		break;
	case 290:
		addInventoryItem(80);
		addInventoryItem(109);
		givenItems();
		break;
	// end merchants
	case 369:
	case 383:
		_state->setCurrentRoot(room, rootIndex + 1, 0);
		break;
	case 295:
		addInventoryItem(84);
		_state->setCurrentRoot(room, rootIndex + 1, 0);
		break;
	case 285:
		toJail();
		break;
	case 374:
	case 372:
	case 373:
		_state->setCurrentRoot(room, rootIndex + 1, 0);
		break;
	case 297: {
		// sprite moves to right
		_room->enableExit(0);
		Sprite *sprite = _room->findSpriteByIndex(0);
		sprite->animData[0].movementFlags = 0x1C; // Move right
		// Basic loop to wait until the sprite has reached the door
		while (!shouldQuit()) {
			_events->pollEvent();
			renderScene();
			if (sprite->x >= 206) {
				break;
			}
			_screen->update();
			g_system->delayMillis(10);
		}
		_room->disableSprite(0);
		_state->setCurrentRoot(room, 2, 0);
	} break;
	case 313:
		_state->setCurrentRoot(room, rootIndex + 1, 0);
		break;
	case 308: {
		int targetBranch = rootIndex + 1;
		if (targetBranch > 17) {
			targetBranch = 2;
		}
		_state->setCurrentRoot(room, targetBranch, 0);
		break;
	}
		/* pyramid merchant*/
	case 314:
		addInventoryItem(93);
		break;
	case 316:
		addInventoryItem(94);
		break;
	case 317:
		// CD
		addInventoryItem(95);
		break;
	case 318:
		// bg book
		addInventoryItem(96);
		break;
	case 319:
		// add pyramid map
		addInventoryItem(97);
		_state->setCurrentRoot(room, 2, 0);
		break;
	case 320:
		_state->setCurrentRoot(room, 2, 0);
		break;
	case 324: {
		HotSpot hotspot = HotSpot();
		hotspot.extra = 628;
		hotspot.x = 221;
		hotspot.y = 202;
		hotspot.w = 50;
		hotspot.h = 56;
		hotspot.isEnabled = true;
		hotspot.isSprite = false;
		hotspot.actionFlags = 8;
		hotspot.index = 9;
		hotspot.innerIndex = 5;
		_room->changeHotspot(47, hotspot);
		_state->setCurrentRoot(room, 2, 0);
		break;
	}
	// girls in pond
	case 321:
		_state->setCurrentRoot(45, 1, 0);
		_sound->playSound("TWANGZZZ.SMP", 0);
		break;
	case 376: {
		playAlfredSpecialAnim(14);
		loadExtraScreenAndPresent(12);
		_state->setCurrentRoot(45, 2, 0);
	} break;
	case 377:
		_state->setCurrentRoot(45, 3, 0);
		break;
	case 30840:
		break;

	case 323:
		_state->setCurrentRoot(47, 1, 0);
		_state->setCurrentRoot(43, 3, 0);
		break;
	case 325:
		_state->setFlag(FLAG_ESQUELETO_RECONOCE, _state->getFlag(FLAG_ESQUELETO_RECONOCE) + 1);
		if (_state->getFlag(FLAG_ESQUELETO_RECONOCE) == 2) {
			_state->setCurrentRoot(49, 1, 0);
		}
		break;
	case 326:
		_state->setCurrentRoot(49, 2, 0);
		break;
	case 378:
		_state->setCurrentRoot(49, 3, 0);
		break;
	case 380:
		turnLightsOff();
		break;
	default:
		debug("Got actionTrigger %d in dialogActionTrigger, but no handler defined", actionTrigger);
		break;
	}
}

/**
 * Turns the screen all dark by changing all palette colors except those of the dialog text for both
 * Alfred and the princess.
 */
void PelrockEngine::turnLightsOff() {
	_currentBackground.clear(0);
	_compositeBuffer.clear(0);
	_screen->clear(0);
	byte darkPalette[768] = {};
	darkPalette[238 * 3 + 0] = 60 << 2; // R = 240
	darkPalette[238 * 3 + 1] = 57 << 2; // G = 228
	darkPalette[238 * 3 + 2] = 57 << 2; // B = 228
	darkPalette[13 * 3 + 0] = 63 << 2;  // R = 252
	darkPalette[13 * 3 + 1] = 21 << 2;  // G = 84
	darkPalette[13 * 3 + 2] = 63 << 2;  // B = 252
	g_system->getPaletteManager()->setPalette(darkPalette, 0, 256);
	memcpy(_room->_roomPalette, darkPalette, 768);
	_screen->markAllDirty();
	_screen->update();
}

void PelrockEngine::givenItems() {
	_state->setFlag(FLAG_MERCHANT_GIVENITEMS, _state->getFlag(FLAG_MERCHANT_GIVENITEMS) + 1);
	if (_state->getFlag(FLAG_MERCHANT_GIVENITEMS) == 4) {
		_state->setCurrentRoot(27, 2, 1);
	}
}

void PelrockEngine::advanceQuotesConversation(byte rootIndex, byte room) {
	int targetRoot = rootIndex + 1;
	if (targetRoot == 26) {
		targetRoot = 2;
	}
	_state->setCurrentRoot(room, targetRoot, 0);
}

void PelrockEngine::toJail() {
	_graphics->fadeToBlack(10);
	_alfredState.x = 342;
	_alfredState.y = 277;
	setScreenAndPrepare(31, ALFRED_DOWN);
	_state->setFlag(FLAG_A_LA_CARCEL, true);
	_room->moveHotspot(_room->findHotspotByExtra(101), 444, 166);
}

void PelrockEngine::noOpAction(HotSpot *hotspot) {
}

void PelrockEngine::noOpItem(int item, HotSpot *hotspot) {
	// Original game checked against 47 instead of 58
	// These are books in the library being returned to the librarian
	if (item >= 11 && item <= 58 && hotspot->extra == 358) {
		_state->removeInventoryItem(item);
		_dialog->say(_res->_ingameTexts[kTextDeAcuerdo2]);
		return;
	}
	_alfredState.direction = ALFRED_DOWN;
	sayRandomIncorrectResponse();
}

void PelrockEngine::openRoomDoor(HotSpot *hotspot) {
	openDoor(hotspot, 0, 93, FEMININE, true);
}

void PelrockEngine::openRoomDrawer(HotSpot *hotspot) {
	if (_room->hasSticker(91)) {
		_dialog->say(_res->_ingameTexts[kTextYaAbiertoM]);
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
	_dialog->say(_res->_ingameTexts[kTextHeladeraCerrada]);
}

void PelrockEngine::pickupGarbageCan(HotSpot *hotspot) {
	_dialog->say(_res->_ingameTexts[kTextPobreNoHeLlegadoAEso]);
}

void PelrockEngine::closeRoomDrawer(HotSpot *hotspot) {
	if (!_room->hasSticker(91)) {
		_dialog->say(_res->_ingameTexts[kTextYaCerradoM]);
		return;
	}
	_room->removeSticker(91);
	_room->enableHotspot(hotspot);
}

void PelrockEngine::openClosedDrawer(HotSpot *hotspot) {
	_dialog->say(_res->_ingameTexts[kTextEstanCerrados]);
}

void PelrockEngine::useCardWithATM(int inventoryObject, HotSpot *hotspot) {
	debug("Withdrawing money from ATM using card (inv obj %d)", inventoryObject);
	if (_state->getFlag(FLAG_JEFE_INGRESA_PASTA)) {
		_state->setFlag(FLAG_JEFE_INGRESA_PASTA, false);
		addInventoryItem(75);
		_state->setCurrentRoot(20, 2, 0);
	} else {
		int billCount = 0;
		for (uint i = 0; i < _state->inventoryItems.size(); i++) {
			if (_state->inventoryItems[i] == 5) {
				billCount++;
			}
		}
		if (billCount < 13) {
			addInventoryItem(5);
			// 1 in every 15 times, the hooker will say "fancy a good time?"
			bool sayLine = getRandomNumber(15) == 1;
			if (sayLine) {
				_dialog->say(_res->_ingameTexts[kTextTeApeteceBuenRato]);
			}
		} else {
			_dialog->say(_res->_ingameTexts[kTextNoTengoMasDinero]);
		}
	}
}

void PelrockEngine::openMcDoor(HotSpot *hotspot) {
	openDoor(hotspot, 2, 7, FEMININE, false);
}

void PelrockEngine::closeMcDoor(HotSpot *hotspot) {
	closeDoor(hotspot, 2, 7, FEMININE, false);
}

void PelrockEngine::pickupBush(HotSpot *hotspot) {
	_dialog->say(_res->_ingameTexts[kTextMeHeVueltoAPinchar]);
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
		_dialog->say(_res->_ingameTexts[kTextQuitaEsasManos]);
	} else if (!_state->getFlag(FLAG_RECIPE_TAKEN)) {
		_state->setFlag(FLAG_RECIPE_TAKEN, true);
		_room->addSticker(36);
		addInventoryItem(63); // Add recipe
		_dialog->say(_res->_ingameTexts[kTextQuesEstoReceta]);
	} else {
		// Already took the recipe
		_dialog->say(_res->_ingameTexts[kTextYaEstaAbierto]);
	}
}

void PelrockEngine::openKitchenDoorFromInside(HotSpot *hotspot) {
	openDoor(hotspot, 0, 34, MASCULINE, true);
}

void PelrockEngine::useSpicySauceWithBurger(int inventoryObject, HotSpot *hotspot) {
	_state->setFlag(FLAG_PUESTA_SALSA_PICANTE, true);
	_sound->playSound(_room->_roomSfx[2]);
	_dialog->say(_res->_ingameTexts[kTextVaestarPocoFuerte]);
}

void PelrockEngine::openShopDoor(HotSpot *hotspot) {
	if (!_state->getFlag(FLAG_TIENDA_ABIERTA)) {
		_dialog->say(_res->_ingameTexts[kTextTiendaCerrada]);
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

	playAlfredSpecialAnim(4);

	Sprite *brickSprite = _room->findSpriteByIndex(7);
	HotSpot *windowHotspot = _room->findHotspotByExtra(294);
	brickSprite->x = 420;
	brickSprite->y = 241;
	brickSprite->zOrder = 10;
	while (!shouldQuit()) {
		_events->pollEvent();
		renderScene(OVERLAY_NONE);
		_room->findSpriteByIndex(7)->y -= 10;
		if (_room->findSpriteByIndex(7)->y <= 70) {
			_room->findSpriteByIndex(7)->zOrder = 255;
			break;
		}
		_screen->update();
		g_system->delayMillis(10);
	}

	debug("Brick hit the window");
	_room->addSticker(11);
	_sound->playSound(_room->_roomSfx[2]); // Play glass breaking sound

	// Remove brick from inventory
	_state->removeInventoryItem(4);

	int16 x = 639; // put at the very edge of the screen so it gets adjusted
	int16 y = windowHotspot->y;
	// Play the NPC dialog sequence
	int16 dialog1y = y + 22;
	int16 dialog2y = dialog1y + 10 + _largeFont->getFontHeight();
	_dialog->say(_res->_ingameTexts[kTextQueHaSidoEso], x, dialog1y);
	_dialog->say(_res->_ingameTexts[kTextQuienAndaAhi], x, dialog2y);
	_dialog->say(_res->_ingameTexts[kTextYoMeVoy]);

	_state->setFlag(FLAG_TIENDA_ABIERTA, true);
	_room->addStickerToRoom(_room->_currentRoomNumber, 9, PERSIST_PERM);
	_room->addStickerToRoom(_room->_currentRoomNumber, 10, PERSIST_PERM);
	_room->disableHotspot(_room->findHotspotByExtra(295));
	_room->disableHotspot(_room->findHotspotByExtra(294)); // Disable window hotspot
	_room->enableSprite(5, 100, PERSIST_PERM);             // Enable fake teeth sprite
	_disableAction = true;                                 // Prevent player from doing anything until the action is done
	walkTo(630, _alfredState.y);
}

void PelrockEngine::moveCable(HotSpot *hotspot) {
	_room->addSticker(15);
	_room->addSticker(16);
	_room->addSticker(17);
	_room->addStickerToRoom(4, 20);
	_state->setFlag(FLAG_CABLES_PUESTOS, true);
}

void PelrockEngine::useBrickWithShopWindow(int inventoryObject, HotSpot *hotspot) {
	_dialog->say(_res->_ingameTexts[kTextNoseEnteraria]);
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
		_dialog->say(_res->_ingameTexts[kTextPrimeroAbrirlo]);
	} else {
		if (_state->getFlag(FLAG_CABLES_PUESTOS)) {
			_room->addSticker(19);
			_room->moveHotspot(_room->findHotspotByIndex(6), 391, 381);
			_state->removeInventoryItem(6);
		}
	}
}

void PelrockEngine::pickCables(HotSpot *hotspot) {
	if (_room->hasSticker(21)) {
		_dialog->say(_res->_ingameTexts[kTextQueLosCojaSupadre]);
		return;
	}
	// Duck to pick cables
	playAlfredSpecialAnim(2);
	// electric shock
	int prevX = _alfredState.x;
	_alfredState.x -= 20;
	// original incorrectly played door closing sound here!
	_sound->playSound("ELEC3ZZZ.SMP", 0);
	playAlfredSpecialAnim(3);
	_alfredState.x = prevX;

	// Stand up (reverse of duck)
	playAlfredSpecialAnim(2, true);
	_room->addSticker(21);

	_dialog->say(_res->_ingameTexts[kTextRelojHaCambiado]);
	_state->setCurrentRoot(4, 1, 0);
}

void PelrockEngine::showIdToGuard(int inventoryObject, HotSpot *hotspot) {
	if (!_state->getFlag(FLAG_GUARDIA_PIDECOSAS)) {
		_dialog->say(_res->_ingameTexts[kTextCuandoMeLoPida]);
		return;
	}

	if (!_state->getFlag(FLAG_GUARDIA_DNI_ENTREGADO)) {
		_state->setFlag(FLAG_GUARDIA_DNI_ENTREGADO, true);
		_dialog->say(_res->_ingameTexts[kTextDeAcuerdo]);
	}
	if (_state->getFlag(FLAG_SOBORNO_PORTERO) && _state->getFlag(FLAG_GUARDIA_DNI_ENTREGADO)) {
		unlockMuseum();
		return;
	}
}

void PelrockEngine::unlockMuseum() {
	_state->setCurrentRoot(4, 3, 0);
	_room->enableSprite(2, 100, PERSIST_PERM);
	_room->enableSprite(3, 100, PERSIST_PERM);
	_room->addStickerToRoom(4, 87, PERSIST_PERM);
	_room->addStickerToRoom(4, 88, PERSIST_PERM);
	_room->addStickerToRoom(4, 89, PERSIST_PERM);
	_room->addStickerToRoom(4, 90, PERSIST_PERM);
}

void PelrockEngine::giveMoneyToGuard(int inventoryObject, HotSpot *hotspot) {
	if (!_state->getFlag(FLAG_GUARDIA_PIDECOSAS)) {
		_dialog->say(_res->_ingameTexts[kTextPretendeUstedSobornarme]);
		return;
	} else if (!_state->getFlag(FLAG_SOBORNO_PORTERO)) {
		_state->setFlag(FLAG_SOBORNO_PORTERO, true);
		_dialog->say(_res->_ingameTexts[kTextMuyBien]);
		_state->removeInventoryItem(5);
	}
	if (_state->getFlag(FLAG_SOBORNO_PORTERO) && _state->getFlag(FLAG_GUARDIA_DNI_ENTREGADO)) {
		unlockMuseum();
		return;
	}
}

void PelrockEngine::openMuseumDoor(HotSpot *hotspot) {
	if (!_state->getFlag(FLAG_GUARDIA_PIDECOSAS)) {
		_dialog->say(_res->_ingameTexts[kTextAlto]);
		return;
	} else if (!_state->getFlag(FLAG_GUARDIA_DNI_ENTREGADO)) {
		_dialog->say(_res->_ingameTexts[kTextNecesitaDni]);
	} else if (!_state->getFlag(FLAG_SOBORNO_PORTERO)) {
		_dialog->say(_res->_ingameTexts[kTextQueReciboACambio]);
	} else {
		openDoor(hotspot, 1, 22, FEMININE, false);
	}
}

void PelrockEngine::closeMuseumDoor(HotSpot *hotspot) {
	closeDoor(hotspot, 1, 22, FEMININE, false);
}

void PelrockEngine::pickupFruit(HotSpot *hotspot) {
	_dialog->say(_res->_ingameTexts[kTextNoTheyMakeyouFat]);
}

void PelrockEngine::useAmuletWithStatue(int inventoryObject, HotSpot *hotspot) {

	if (!_room->hasSticker(24)) {
		_room->addSticker(24);
		_state->removeInventoryItem(7);
		_state->setCurrentRoot(7, 1, 0);
		_alfredState.direction = ALFRED_RIGHT;

		HotSpot *statueHotspot = _room->findHotspotByExtra(347);
		_currentHotspot = statueHotspot;
		walkTo(statueHotspot->x + statueHotspot->w / 2, statueHotspot->y + statueHotspot->h);
		_sound->playSound(_room->_roomSfx[0]); // Magic sound
		animateStatuePaletteFade(false);
		_dialog->say(_res->_ingameTexts[kTextAnda]);
		walkAndAction(statueHotspot, TALK);
		waitForActionEnd();
	}
}

void PelrockEngine::useSecretCodeWithStatue(int inventoryObject, HotSpot *hotspot) {
	_dialog->say(_res->_ingameTexts[kTextNoEsAMiAQuienDebes]);
	_dialog->say(_res->_ingameTexts[kTextAQuienEntonces]);
	_dialog->say(_res->_ingameTexts[kTextLibrosSecretos]);
}

void PelrockEngine::pickUpLetter(HotSpot *hotspot) {
	addInventoryItem(9);
	_room->setActionMask(hotspot, kActionMaskNone); // Disable hotspot
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
	_dialog->say(_res->_ingameTexts[kTextRegaloLibroRecetas]);
	_state->removeInventoryItem(8);
	addInventoryItem(59);
}

void PelrockEngine::useBrickWithLibrarian(int inventoryObject, HotSpot *hotspot) {
	_dialog->say(_res->_ingameTexts[kTextYSiMeTiraMaquina]);
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
	// The game originally already did nothing here
}

void PelrockEngine::closeTravelAgencyDoor(HotSpot *hotspot) {
	closeDoor(hotspot, 1, 57, FEMININE, false);
}

void PelrockEngine::usePumpkinWithRiver(int inventoryObject, HotSpot *hotspot) {
	_state->removeInventoryItem(76);
	addInventoryItem(86);
	if (_state->getFlag(FLAG_CROCODILLO_ENCENDIDO) == false) {
		_sound->playMusicTrack(27);
		checkIngredients();
		_dialog->say(_res->_ingameTexts[kTextCuidadoImprudente]);
		_alfredState.x -= 10;
		_alfredState.y += 20;
		playAlfredSpecialAnim(5);
		_alfredState.animState = ALFRED_SKIP_DRAWING;
		_sound->playSound(_room->_roomSfx[0], 0); // Belch
		waitForSoundEnd();
		_graphics->fadeToBlack(10);
		_alfredState.x = 300;
		_alfredState.y = 238;
		waitForSoundEnd();
		_alfredState.animState = ALFRED_IDLE;
		setScreenAndPrepare(28, ALFRED_DOWN);
		_dialog->say(_res->_ingameTexts[kTextQueOscuroEstaEsto]);
	}
}

void PelrockEngine::playAlfredSpecialAnim(int anim, bool reverse) {
	_res->loadAlfredSpecialAnim(anim, reverse);
	_alfredState.animState = ALFRED_SPECIAL_ANIM;
	waitForSpecialAnimation();
}

void PelrockEngine::waitForSoundEnd(int channel) {
	while (!shouldQuit() && _sound->isPlaying(channel)) {
		_events->pollEvent();
		renderScene(OVERLAY_NONE);
		_screen->update();
		g_system->delayMillis(10);
	}
}

void PelrockEngine::pickupSunflower(HotSpot *hotspot) {
	if (_state->getFlag(FLAG_PARADOJA_RESUELTA) == false) {
		if (_state->getFlag(FLAG_RIDDLE_PRESENTED)) {
			// try to take the sunflower before solving the riddle
			_dialog->say(_res->_ingameTexts[kTextLeEstoyVigilando]);
		} else {
			_dialog->say(_res->_ingameTexts[kTextOiga]);
			_state->setCurrentRoot(25, 26, 0);
			walkAndAction(_room->findHotspotByExtra(467), TALK);
			_state->setFlag(FLAG_RIDDLE_PRESENTED, true);
		}
	} else {
		addInventoryItem(85);
		_room->disableHotspot(hotspot);
		_state->setCurrentRoot(25, 1, 0);
		_room->addSticker(73);
		checkIngredients();
	}
}

void PelrockEngine::checkIngredients() {
	byte ingredientes = _state->getFlag(FLAG_INGREDIENTES_CONSEGUIDOS);
	int textLine = kTextPrimerIngrediente + ingredientes;
	_dialog->say(_res->_ingameTexts[textLine]);
	_state->setFlag(FLAG_INGREDIENTES_CONSEGUIDOS, ingredientes + 1);
}

void PelrockEngine::pickUpBook(int i) {
	if (!_state->hasInventoryItem(10)) {
		_dialog->say(_res->_ingameTexts[kTextVengaAca]);
		_state->setCurrentRoot(9, 1, 0);

		if (_state->hasInventoryItem(3)) {
			_state->setCurrentRoot(9, 2, 0);
			addInventoryItem(10);
		}

		_alfredState.isWalkingCancelable = false;
		walkAndAction(_room->findHotspotByExtra(358), TALK);
		if (!_state->hasInventoryItem(3)) {

			_state->setCurrentRoot(9, 0, 0);
		} else {
			_state->setCurrentRoot(9, 3, 0);
		}
	} else {
		if (_state->libraryShelf == -1) {
			_dialog->say(_res->_ingameTexts[kTextTodos]);
		} else if (_state->libraryShelf != i) {
			_dialog->say(_res->_ingameTexts[kTextElLibroNoEstaAqui]);
		} else {
			_state->libraryShelf = -1;
			int booksInInventory = _state->booksInInventory();
			if (booksInInventory == 3) {
				int firstBook = _state->findFirstBookIndex();
				if (firstBook != -1)
					_state->removeInventoryItem(firstBook);
				_dialog->say(_res->_ingameTexts[kTextTendreDejarLibro]);
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

void PelrockEngine::openEgyptMuseumDoor(HotSpot *hotspot) {
	openDoor(hotspot, 0, 59, MASCULINE, false);
}

void PelrockEngine::closeEgyptMuseumDoor(HotSpot *hotspot) {
	closeDoor(hotspot, 0, 59, MASCULINE, false);
}

void PelrockEngine::pushSymbol1(HotSpot *hotspot) {
	if (_state->getFlag(FLAG_MIRA_SIMBOLO_FUERA_MUSEO) == true) {
		byte symbolsPulled = _state->getFlag(FLAG_SYMBOLS_PUSHED);
		_state->setFlag(FLAG_SYMBOLS_PUSHED, symbolsPulled | 0x1);
		checkAllSymbols();
	}
}

void PelrockEngine::pushSymbol2(HotSpot *hotspot) {
	if (_state->getFlag(FLAG_MIRA_SIMBOLO_FUERA_MUSEO) == true) {
		byte symbolsPulled = _state->getFlag(FLAG_SYMBOLS_PUSHED);
		_state->setFlag(FLAG_SYMBOLS_PUSHED, symbolsPulled | 0x2);
		checkAllSymbols();
	}
}

void PelrockEngine::pushSymbol3(HotSpot *hotspot) {
	if (_state->getFlag(FLAG_MIRA_SIMBOLO_FUERA_MUSEO) == true) {
		byte symbolsPulled = _state->getFlag(FLAG_SYMBOLS_PUSHED);
		_state->setFlag(FLAG_SYMBOLS_PUSHED, symbolsPulled | 0x4);
		checkAllSymbols();
	}
}

void PelrockEngine::pushSymbol4(HotSpot *hotspot) {
	if (_state->getFlag(FLAG_MIRA_SIMBOLO_FUERA_MUSEO) == true) {
		byte symbolsPulled = _state->getFlag(FLAG_SYMBOLS_PUSHED);
		_state->setFlag(FLAG_SYMBOLS_PUSHED, symbolsPulled | 0x8);
		checkAllSymbols();
	}
}

void PelrockEngine::checkAllSymbols() {
	byte symbolsPulled = _state->getFlag(FLAG_SYMBOLS_PUSHED);
	if (symbolsPulled == 0x0F) {
		// Activates animation
		_sound->playSound(_room->_roomSfx[0]);
		_room->enableSprite(4, 100, PERSIST_TEMP);
		_room->enableExit(0, PERSIST_BOTH);
		_room->addSticker(61);
		_room->addSticker(63);
	}
}

void PelrockEngine::pickUpHairStrand(HotSpot *hotspot) {
	checkIngredients();
	_state->setFlag(FLAG_ROBA_PELO_PRINCESA, true);
}

void PelrockEngine::openJailFloorTile(HotSpot *hotspot) {
	if (_room->hasSticker(77)) {
		_dialog->say(_res->_ingameTexts[kTextYaAbiertoM]);
		return;
	}
	_room->enableExit(0, PERSIST_BOTH);
	_room->addSticker(77, PERSIST_BOTH);
}

void PelrockEngine::openTunnelDrawer(HotSpot *hotspot) {
	_room->addSticker(78, PERSIST_BOTH);
	_room->disableHotspot(hotspot);
}

void PelrockEngine::useKeyWithPortrait(int inventoryObject, HotSpot *hotspot) {
	_room->disableHotspot(hotspot);
	_room->addSticker(101);
}

void PelrockEngine::openSafe(HotSpot *hotspot) {
	if (_state->getFlag(FLAG_CLAVE_CAJA_FUERTE) == true) {
		_room->addSticker(102);
		_dialog->say(_res->_ingameTexts[kTextGranCantidadDinero]);
		addInventoryItem(82);
		_state->setCurrentRoot(27, 1, 0);
		_state->setCurrentRoot(27, 1, 1);
	} else {
		_dialog->say(_res->_ingameTexts[kTextSiSupieraCombinacion]);
	}
}

void PelrockEngine::openTunnelDoor(HotSpot *hotspot) {
	openDoor(hotspot, 0, 66, MASCULINE, true);
}

void PelrockEngine::closeTunnelDoor(HotSpot *hotspot) {
	closeDoor(hotspot, 0, 66, MASCULINE, true);
}

void PelrockEngine::useDollWithBed(int inventoryObject, HotSpot *hotspot) {
	int x = _alfredState.x;
	int y = _alfredState.y;
	_alfredState.x -= 36;
	_alfredState.y += 7;
	playAlfredSpecialAnim(11);
	_alfredState.x -= 4;
	_alfredState.y += 12;
	playAlfredSpecialAnim(12);
	_alfredState.direction = ALFRED_DOWN;
	_state->setFlag(FLAG_SE_HA_PUESTO_EL_MUNECO, true);
	_state->removeInventoryItem(83);
	_room->addSticker(126);
	_alfredState.x = x;
	_alfredState.y = y;
}

void PelrockEngine::giveMagazineToGuard(int inventoryObject, HotSpot *hotspot) {
	_state->removeInventoryItem(84);
	_state->setCurrentRoot(34, 4, 0);
	walkAndAction(hotspot, TALK);
}

void PelrockEngine::giveWaterToGuard(int inventoryObject, HotSpot *hotspot) {
	_state->setFlag(FLAG_VIGILANTE_BEBE_AGUA, _state->getFlag(FLAG_VIGILANTE_BEBE_AGUA) + 1);

	_dialog->say(_res->_ingameTexts[kTextAlaConUsted]);
	_state->removeInventoryItem(86);
	addInventoryItem(76);
	if (_state->getFlag(FLAG_VIGILANTE_BEBE_AGUA) == 3) {
		_dialog->say(_res->_ingameTexts[kTextMeMeo]);
		_state->setFlag(FLAG_VIGILANTE_MEANDO, true);
		guardMovement();
		_room->disableSprite(36, 0, PERSIST_BOTH);
		_room->enableExit(0, PERSIST_BOTH);
	} else {
		_state->setCurrentRoot(36, _state->getCurrentRoot(36, 0) + 1, 0);
	}
}

void PelrockEngine::guardMovement() {

	// guard running
	Sprite *sprite = _room->findSpriteByIndex(0);
	sprite->animData[0].nframes = 5;
	sprite->animData[0].movementFlags = 0x1C; // Move right
	byte state = 0;
	// Basic loop to wait until the sprite has reached the door
	while (!shouldQuit()) {
		_events->pollEvent();
		renderScene();
		// has the guard move right, up, left, then disappear behind the pyramid
		if (sprite->x >= 339 && state == 0) {
			state = 1;
			sprite->animData[0].movementFlags = 0x240; // Move up
		}
		if (sprite->y <= 188 && state == 1) {
			state = 2;
			sprite->animData[0].movementFlags = 0x14; // Move left
		}
		if (sprite->x <= 327 && state == 2) {
			sprite->zOrder = 255; // Hide sprite
			break;
		}
		_screen->update();
		g_system->delayMillis(10);
	}
}

void PelrockEngine::pickUpStone(HotSpot *hotspot) {
	_room->addSticker(117);
	_state->setFlag(FLAG_PIRAMIDE_JODIDA, true);
	_sound->playSound("QUAKE2ZZ.SMP", 0, -1);
	_shakeEffectState.enable();
	checkIngredients();
	_dialog->say(_res->_ingameTexts[kTextAyAyAy]);
	_alfredState.direction = ALFRED_DOWN;
	_dialog->say(_res->_ingameTexts[kTextNadieLaHaVisto]);

	_alfredState.direction = ALFRED_LEFT;
	_disableAction = true;
	walkTo(592, 306);
}

void PelrockEngine::playSpecialAnim(uint32 offset, bool compressed, int x, int y, int width, int height, int numFrames) {
	size_t frameSize = width * height;
	size_t bufSize = frameSize * numFrames;
	byte *animData = new byte[bufSize];
	_res->loadOtherSpecialAnim(offset, compressed, animData, bufSize);

	Graphics::Surface animSurface;
	animSurface.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	int curFrame = 0;
	while (!shouldQuit()) {
		_events->pollEvent();

		bool didRender = renderScene(OVERLAY_NONE);
		if (didRender) {
			memset(animSurface.getPixels(), 0, frameSize);
			extractSingleFrame(animData, (byte *)animSurface.getPixels(), curFrame, width, height);
			_screen->transBlitFrom(animSurface, Common::Point(x, y), 255);
			if (_chrono->getFrameCount() % 2 == 0) {
				curFrame++;
				if (curFrame >= numFrames) {
					_screen->markAllDirty();
					_screen->update();
					break;
				}
			}
		}
		_screen->markAllDirty();
		_screen->update();
		g_system->delayMillis(10);
	}
	animSurface.free();
	delete[] animData;
}

void PelrockEngine::giveStoneToSlaves(int inventoryObject, HotSpot *hotspot) {
	// Remove whichever stone item was used (91 = stone, 92 = mud/clay)
	_state->removeInventoryItem(inventoryObject);

	Sprite *masters = _room->findSpriteByExtra(600);
	byte zIndex = masters->zOrder;
	// Capture coordinates now, before any playSpecialAnim loop runs renderScene and
	// sortAnimsByZOrder reorders _currentRoomAnims (which would make the pointer stale).
	int16 mastersX = masters->x;
	int16 mastersY = masters->y;

	// Slaves take stone then chant
	playSpecialAnim(1600956, false, 0, 298, 208, 102, 7);
	_sound->playSound(_room->_roomSfx[0], 0);
	waitForSoundEnd();

	_dialog->say(_res->_ingameTexts[kTextHayQueCelebrarlo]);

	byte counter = _state->getFlag(FLAG_DA_PIEDRA);
	// drinking animation and sound
	_sound->playSound(_room->_roomSfx[1], 2);

	_room->findSpriteByIndex(0)->zOrder = 255;

	playSpecialAnim(1473360, true, mastersX - 6, mastersY - 1, 152, 83, 7);

	// Increment stone delivery counter (tracks 0→1→2→3)
	debug("Current stone delivery count: %d", counter);
	if (counter < 3) {
		_state->setFlag(FLAG_DA_PIEDRA, ++counter);
		_room->findSpriteByIndex(0)->zOrder = zIndex;
	}
	debug("New stone delivery count: %d", _state->getFlag(FLAG_DA_PIEDRA));
	// At 2nd stone delivery: masters starts singing (conversation root 2)
	if (counter == 2) {
		_state->setCurrentRoot(41, 2, 0);
	}

	// At 3rd stone delivery: masters get wasted
	if (counter == 3) {
		_room->disableSprite(0, PERSIST_BOTH);
		playSpecialAnim(1512060, true, mastersX - 28, mastersY - 6, 172, 96, 3);
		_room->addSticker(116);
		WalkBox w1 = {3, 187, 374, 5, 17, 0};
		WalkBox w2 = {4, 141, 374, 46, 4, 0};
		_room->addWalkbox(w1);
		_room->addWalkbox(w2);
		_state->setFlag(FLAG_GUARDIAS_BORRACHOS, true);
	}
}

/**
 * Naked girls swim underwater, guard enters the scene.
 * "diving" animation is stored in Alfred.7 as a single RLE chunk with multiple continuous sprite sizes.
 */
void PelrockEngine::swimmingPoolCutscene(HotSpot *hotspot) {
	byte *buffer = nullptr;
	size_t bufSize = 0;

	struct SwimmerInfo {
		int spriteIndex;
		int16 x;
		int16 y;
		int w;
		int h;
		int nFrames;
		int totalSize;
		uint16 movementFlags;
	};

	SwimmerInfo swimmers[4] = {
		{3, 3, -17, 93, 88, 9, 93 * 88 * 9, 0x02FF}, // Move right and up
		{4, 1, 0, 68, 31, 7, 68 * 31 * 7, 0},
		{5, -14, -18, 79, 95, 9, 79 * 95 * 9, 0},
		{6, -1, -8, 54, 42, 8, 54 * 42 * 8, 0}};

	_res->loadOtherSpecialAnim(1446862, true, buffer, bufSize);

	int acc = 0;
	for (int i = 0; i < 4; i++) {
		Sprite *sprite = _room->findSpriteByIndex(swimmers[i].spriteIndex);
		sprite->x = sprite->x + swimmers[i].x;
		sprite->y = sprite->y + swimmers[i].y;
		sprite->w = swimmers[i].w;
		sprite->h = swimmers[i].h;
		sprite->animData[0].nframes = swimmers[i].nFrames;
		sprite->animData[0].movementFlags = swimmers[i].movementFlags;
		sprite->animData[0].curFrame = 0;
		sprite->animData[0].loopCount = 0;
		sprite->animData[0].curLoop = 0;
		sprite->animData[0].speed = 0;
		sprite->animData[0].elpapsedFrames = 0;
		sprite->disableAfterSequence = true;

		sprite->animData[0].animData = new byte *[sprite->animData[0].nframes];
		for (int j = 0; j < sprite->animData[0].nframes; j++) {
			sprite->animData[0].animData[j] = new byte[sprite->w * sprite->h];
			extractSingleFrame(buffer + acc, sprite->animData[0].animData[j], j, sprite->w, sprite->h);
		}
		acc += sprite->w * sprite->h * sprite->animData[0].nframes;
	}

	_sound->stopMusic();
	_sound->playMusicTrack(28);
	while (!shouldQuit()) {
		_events->pollEvent();
		bool didRender = renderScene(OVERLAY_NONE);

		if (didRender) {
			if (_room->findSpriteByIndex(swimmers[0].spriteIndex)->zOrder == 255) {
				break;
			}
		}
		_screen->update();
		g_system->delayMillis(10);
	}
	Sprite *guard = _room->findSpriteByIndex(0);
	guard->animData[0].movementFlags = 0x14;
	while (!shouldQuit()) {
		_events->pollEvent();
		renderScene();
		if (guard->x <= _alfredState.x + 70) {
			break;
		}
		_screen->update();
		g_system->delayMillis(10);
	}

	guard->animData[0].movementFlags = 0;
	guard->animData[0].curFrame = 0;
	guard->animData[0].nframes = 1;
	// copy idle frame from talking animation
	guard->animData[0].animData[0] = _room->_talkingAnims.animA[0];
	_alfredState.direction = ALFRED_RIGHT;
	walkAndAction(_room->findHotspotByExtra(guard->extra), TALK);
	if (shouldQuit()) {
		return;
	}
	_graphics->fadeToBlack(10);
	_alfredState.x = 271;
	_alfredState.y = 385;
	setScreenAndPrepare(40, ALFRED_UP);
	walkAndAction(_room->findHotspotByExtra(640), TALK);
	if (shouldQuit()) {
		return;
	}
	_graphics->fadeToBlack(10);
	_alfredState.x = 271;
	_alfredState.y = 385;
	setScreenAndPrepare(41, ALFRED_UP);
}

void PelrockEngine::pickUpStones(HotSpot *hotspot) {
	if (_state->hasInventoryItem(91)) {
		_dialog->say(_res->_ingameTexts[kTextPesaDemasiado]);
		return;
	}
	if (_state->getFlag(FLAG_PIEDRAS_COGIDAS) >= 2) {
		_dialog->say(_res->_ingameTexts[kTextNingunaTamanhoApropiado]);
		return;
	} else {
		addInventoryItem(91);
		_state->setFlag(FLAG_PIEDRAS_COGIDAS, _state->getFlag(FLAG_PIEDRAS_COGIDAS) + 1);
	}
}

void PelrockEngine::pickUpMud(HotSpot *hotspot) {
	if (_state->getFlag(FLAG_PIEDRAS_COGIDAS) != 2) {
		_dialog->say(_res->_ingameTexts[kTextParaQueCogeBarro]);
		return;
	} else {
		addInventoryItem(92);
		_state->setFlag(FLAG_PIEDRAS_COGIDAS, _state->getFlag(FLAG_PIEDRAS_COGIDAS) + 1);
		_dialog->say(_res->_ingameTexts[kTextBuenoCogereUnPoco]);
	}
}

void PelrockEngine::openPyramidDoor(HotSpot *hotspot) {
	_dialog->say(_res->_ingameTexts[kTextAbsolutamenteCerrado]);
}

void PelrockEngine::usePumpkinWithPond(int inventoryObject, HotSpot *hotspot) {
	_state->removeInventoryItem(76);
	addInventoryItem(86);
}

void PelrockEngine::useWaterOnFakeStone(int inventoryObject, HotSpot *hotspot) {

	int count = _state->getFlag(FLAG_PIEDRA_FAKE_MOJADA);
	if (count != 3) {
		_state->removeInventoryItem(86);
		addInventoryItem(76);
		switch (count) {
		case 0:
			_room->addSticker(120);
			break;
		case 1:
			_room->addSticker(121);
			break;
		case 2:
			_room->addSticker(122);
			_room->enableExit(1);
			break;
		default:
			break;
		}
		count++;
		_state->setFlag(FLAG_PIEDRA_FAKE_MOJADA, count);
	}
}

void PelrockEngine::useWigWithPot(int inventoryObject, HotSpot *hotspot) {
	_dialog->say(_res->_ingameTexts[kTextNoEraAutentico]);
	_state->removeInventoryItem(99);
}

void PelrockEngine::magicFormula(int inventoryObject, HotSpot *hotspot) {
	_state->removeInventoryItem(inventoryObject);
	if (inventoryObject == 86) {
		addInventoryItem(76);
	}
	_state->setFlag(FLAG_FORMULA_MAGICA, _state->getFlag(FLAG_FORMULA_MAGICA) + 1);
	if (_state->getFlag(FLAG_FORMULA_MAGICA) == 4) {
		smokeAnimation(-1);
		_alfredState.setState(ALFRED_IDLE);
		_state->clearInventory();
		_state->addInventoryItem(88);
		_state->addInventoryItem(76);
		_state->addInventoryItem(82);

		setScreenAndPrepare(39, ALFRED_UP);
	}
}

void PelrockEngine::smokeAnimation(int spriteIndex, bool hide) {
	size_t frameSize = 98 * 138;
	size_t bufSize = frameSize * 11;
	byte *smokeFrames = new byte[bufSize];
	_res->loadOtherSpecialAnim(1526432, true, smokeFrames, bufSize);
	Graphics::Surface smokeSurface;
	smokeSurface.create(98, 138, Graphics::PixelFormat::createFormatCLUT8());
	int curFrame = 0;
	int x = _alfredState.x;
	int y = _alfredState.y - _alfredState.h;
	if (spriteIndex >= 0) {
		x = _room->findSpriteByIndex(spriteIndex)->x;
		y = _room->findSpriteByIndex(spriteIndex)->y;
	}
	while (!shouldQuit()) {
		_events->pollEvent();

		bool didRender = renderScene(OVERLAY_NONE);

		memset(smokeSurface.getPixels(), 0, frameSize);
		extractSingleFrame(smokeFrames, (byte *)smokeSurface.getPixels(), curFrame, 98, 138);
		_screen->transBlitFrom(smokeSurface, Common::Point(x, y), 255);
		if (curFrame == 5) {
			if (spriteIndex == -1) {
				_alfredState.setState(ALFRED_SKIP_DRAWING);
			} else {
				if (hide) {
					_room->disableSprite(spriteIndex);
				} else {
					_room->enableSprite(spriteIndex, 200);
				}
			}
		}
		if (didRender && _chrono->getFrameCount()) {
			curFrame++;

			if (curFrame >= 11) {
				break;
			}
		}
		_screen->update();
		g_system->delayMillis(10);
	}
}

void PelrockEngine::openArchitectDoor(HotSpot *hotspot) {
	openDoor(hotspot, 2, 124, FEMININE, true);
}

void PelrockEngine::closeArchitectDoor(HotSpot *hotspot) {
	closeDoor(hotspot, 2, 124, FEMININE, true);
}

void PelrockEngine::pickupPyramidMap(HotSpot *hotspot) {
	addInventoryItem(98);
}

void PelrockEngine::openArchitectDoorFromInside(HotSpot *hotspot) {
	if (!_room->hasSticker(104)) {
		_dialog->say(_res->_ingameTexts[kTextYaAbiertaF]);
		return;
	}
	_room->enableExit(0, PERSIST_TEMP);
	_room->removeSticker(104);
	_sound->playSound(_room->_roomSfx[0]);
}

void PelrockEngine::closeArchitectDoorFromInside(HotSpot *hotspot) {
	if (_room->hasSticker(104)) {
		_dialog->say(_res->_ingameTexts[kTextYaCerradaF]);
		return;
	}
	_room->disableExit(0, PERSIST_TEMP);
	_room->addSticker(104, PERSIST_TEMP);
	_sound->playSound(_room->_roomSfx[1]);
}

void PelrockEngine::performActionTrigger(uint16 actionTrigger) {
	switch (actionTrigger) {
	case 257: // look portrait
		_sound->playMusicTrack(25, false);
		loadExtraScreenAndPresent(9);
		_dialog->say(_res->_ingameTexts[kTextQueBuenaEsta]);
		_screen->markAllDirty();
		_screen->update();
		break;
	case 268: // look at statue
		_dialog->say(_res->_ingameTexts[kTextTuCrees]);
		break;
	case 271: // look at librarian
		_dialog->say(_res->_ingameTexts[kTextTrabajariaMejorSiNoMeMolestara]);
		break;
	case 270:
		_state->stateGame = COMPUTER;
		break;
	case 280:
		_dialog->say(_res->_ingameTexts[kTextNovio2Metros]);
		break;
	case 281:
		_dialog->say(_res->_ingameTexts[kTextGranIdea]);
		break;
	case 282:
		_dialog->say(_res->_ingameTexts[kTextSeLorecomiendo]);
		break;
	case 327:
		_state->setFlag(FLAG_MIRA_SIMBOLO_FUERA_MUSEO, true);
		break;
	case 294: {
		HotSpot *floorTile = _room->findHotspotByExtra(462);
		floorTile->actionFlags = kActionMaskOpen;
		_room->changeHotSpot(*floorTile);
		break;
	}

	case 307: {
		HotSpot *stone = _room->findHotspotByExtra(90);
		stone->actionFlags = kActionMaskPickup;
		_room->changeHotSpot(*stone);
		break;
	}

	case 322: {
		_dialog->say(_res->_ingameTexts[kTextNoSeTeCurraCercarte]);
		break;
	}
	case 375: {
		teleportToPrincess();
		break;
	}
	}
}

// Bresenham line draw using a 256-byte palette remap table (semi-transparent effect).
// Each pixel on the line is replaced by remapTable[existing_color] instead of a flat color.
static void drawRemappedLine(byte *buf, int x0, int y0, int x1, int y1, const byte *remapTable) {
	int dx = ABS(x1 - x0);
	int dy = ABS(y1 - y0);
	int sx = (x0 < x1) ? 1 : -1;
	int sy = (y0 < y1) ? 1 : -1;
	int err = dx - dy;
	while (true) {
		if (x0 >= 0 && x0 < 640 && y0 >= 0 && y0 < 400) {
			int idx = y0 * 640 + x0;
			buf[idx] = remapTable[buf[idx]];
		}
		if (x0 == x1 && y0 == y1)
			break;
		int e2 = 2 * err;
		if (e2 > -dy) {
			err -= dy;
			x0 += sx;
		}
		if (e2 < dx) {
			err += dx;
			y0 += sy;
		}
	}
}

void PelrockEngine::teleportToPrincess() {
	int phase = 0;

	int lines[5][4] = {
		{57, 176, 301, 322},
		{138, 159, 283, 292},
		{213, 156, 325, 277},
		{460, 163, 370, 292},
		{530, 167, 353, 320}};

	int stickers[5] = {113, 114, 110, 111, 112};

	while (phase < 5) {
		Sprite *thisSprite = _room->findSpriteByIndex(phase + 1);
		thisSprite->animData[0].curFrame = 0;
		thisSprite->zOrder = 200;

		while (!shouldQuit() && _room->findSpriteByIndex(phase + 1)->zOrder != 255) {
			_events->pollEvent();
			renderScene(OVERLAY_NONE);
			_screen->update();
		}

		_sound->playSound(_room->_roomSfx[3], 0);

		// Draw 19 semi-transparent remapped lines
		_graphics->copyBackgroundToBuffer();
		_graphics->placeStickersFirstPass();
		updateAnimations();
		_graphics->presentFrame();
		_screen->update();

		for (int i = 0; i < 19; i++) {
			if (shouldQuit())
				return;
			int x1 = lines[phase][0];
			int y1 = lines[phase][1];
			int x2 = lines[phase][2] + i;
			int y2 = lines[phase][3];
			drawRemappedLine((byte *)_compositeBuffer.getPixels(), x1, y1, x2, y2, _room->_paletteRemaps[1]);
		}

		updateAnimations();
		_graphics->presentFrame();
		_screen->update();

		_events->pollEvent();
		if (shouldQuit())
			return;

		// Restore clean frame with sticker (lines gone)
		_room->addSticker(stickers[phase]);
		_graphics->copyBackgroundToBuffer();
		_graphics->placeStickersFirstPass();
		updateAnimations();
		_graphics->presentFrame();
		_screen->update();
		phase++;
	}
	// small delay before last sticker
	int delay = 10;
	while (!shouldQuit() && delay > 0) {
		_events->pollEvent();
		bool didRender = renderScene(OVERLAY_NONE);
		_screen->update();
		g_system->delayMillis(10);
		if (didRender) {
			delay--;
		}
	}

	_room->addSticker(115);
	_graphics->copyBackgroundToBuffer();
	_graphics->placeStickersFirstPass();
	updateAnimations();
	_graphics->presentFrame();
	_screen->update();

	_dialog->say(_res->_ingameTexts[kTextMareDeDou]);

	smokeAnimation(-1, true);
	_state->setFlag(FLAG_END_OF_GAME, true);
	_state->setCurrentRoot(48, 1, 0);
	_alfredState.x = 138;
	_alfredState.y = 255;
	setScreenAndPrepare(48, ALFRED_DOWN);
}

void PelrockEngine::useOnAlfred(int inventoryObject) {

	switch (inventoryObject) {
	case 9: // Letter
		_dialog->say(_res->_ingameTexts[kTextCorrespondenciaAjena]);
		break;
	case 34: // How to become rich book
		_dialog->say(_res->_ingameTexts[kTextPeriodicoSensacionalista], 1);
		break;
	case 63: // Recipe
		playAlfredSpecialAnim(1);

		loadExtraScreenAndPresent(3);
		_state->setCurrentRoot(17, 1, 0);
		_state->setCurrentRoot(18, 4, 0);
		debug("After extra screen");
		_dialog->say(_res->_ingameTexts[kTextQueAsco]);
		break;
	case 59: // Recipe book
		if (!_state->hasInventoryItem(64)) {
			playAlfredSpecialAnim(0);
			_dialog->say(_res->_ingameTexts[kTextHojaEntrePaginas]);
			addInventoryItem(64);
		} else {
			playAlfredSpecialAnim(0);
			_dialog->say(_res->_ingameTexts[kTextNoEntiendonada]);
		}
		break;
	case 17: // Egyptian book
		playAlfredSpecialAnim(0);
		_dialog->say(_res->_ingameTexts[kTextYaSeEgipcio]);
		_state->setFlag(FLAG_ALFRED_SABE_EGIPCIO, true);
		break;
	case 24: // Encyclopedia
		if (_state->getFlag(FLAG_RIDDLE_PRESENTED) == true) {
			_dialog->say(_res->_ingameTexts[kTextCapituloParadojas]);
			_state->setCurrentRoot(25, 44, 0);
		} else {
			playAlfredSpecialAnim(0);
			_dialog->say(_res->_ingameTexts[kTextCosasAprendido]);
			_state->setFlag(FLAG_ALFRED_INTELIGENTE, true);
			_state->setCurrentRoot(14, 2, 0);
		}
		break;
	case 64: // Formula for time travel
		playAlfredSpecialAnim(0);
		loadExtraScreenAndPresent(5);
		if (_state->getFlag(FLAG_ALFRED_SABE_EGIPCIO)) {
			_dialog->say(_res->_ingameTexts[kTextFormulaViajeAlTiempo]);
		} else {
			_dialog->say(_res->_ingameTexts[kTextQueLastimaNoSeeEgipcio]);
		}
		break;
	case 88: { // spellbook
		if (_room->_currentRoomNumber != 28 &&
			(_room->_currentRoomNumber < 51 || _room->_currentRoomNumber > 54)) {
			_dialog->say(_res->_ingameTexts[kTextAquiNoNecesito]);
			break;
		}
		SpellBook spellBook(_events, _res);
		playAlfredSpecialAnim(0);

		Spell *spell = spellBook.run();
		if (spell) {
			_alfredState.direction = ALFRED_LEFT;
			switch (_room->_currentRoomNumber) {
			case 28: {
				_dialog->say(_res->_ingameTexts[kTextDiosHalcon + spell->page], 1);
				if (spell->page == 12) {
					_graphics->clearScreen();
					int waitFrames = 0;
					// blank screen for half a second
					while (!shouldQuit() && waitFrames < 20) {
						_events->pollEvent();
						_chrono->updateChrono();
						if (_chrono->_gameTick) {
							waitFrames++;
						}
						_screen->markAllDirty();
						_screen->update();
						g_system->delayMillis(10);
					}

					_alfredState.x = 145;
					_alfredState.y = 312;
					setScreenAndPrepare(25, ALFRED_RIGHT);
					_dialog->say(_res->_ingameTexts[kTextMenudaAventura]);
				}
				break;
			}
			case 51:
			case 52:
			case 53:
			case 54: {
				_sound->playSound(_room->_roomSfx[8], 0);
				_dialog->say(_res->_ingameTexts[kTextDiosHalcon + spell->page], 1);
				int flightIndex = _room->_currentRoomNumber - 51;
				if (_fightSorcererAppeared && !_fightInBlockingAnim && spell->page == kFightRooms[flightIndex].spellPage) {
					_state->setFlag(FLAG_COMO_ESTAN_LOS_DIOSES, _state->getFlag(FLAG_COMO_ESTAN_LOS_DIOSES) | (1 << flightIndex));
					_sound->playSound(_room->_roomSfx[1], 0);
					smokeAnimation(kFightRooms[flightIndex].spriteIdx, true);
					_room->addStickerToRoom(_room->_currentRoomNumber, 127 + flightIndex);
					_room->addStickerToRoom(52, 106 + flightIndex);

					if (_state->getFlag(FLAG_COMO_ESTAN_LOS_DIOSES) == 15) { // all 4 spells successful
						HotSpot hotspot = HotSpot();
						hotspot.actionFlags = 0;
						hotspot.extra = 999;
						hotspot.x = 320;
						hotspot.y = 288;
						hotspot.w = 35;
						hotspot.h = 21;
						hotspot.innerIndex = 0;
						hotspot.index = 8;
						_room->changeHotspot(52, hotspot);
					}
				} else {
					_sound->playSound(_room->_roomSfx[2], 0);
				}
				break;
			}
			default:
				break;
			}
		}
		break;
	}
	case 0: // yellow book
		playAlfredSpecialAnim(0);
		_dialog->say(_res->_ingameTexts[kTextCuentoParecido]);
		break;
	case 101: // combination
		playAlfredSpecialAnim(1);
		_dialog->say(_res->_ingameTexts[kTextPareceCombinacionCajaFuerte]);
		_state->setFlag(FLAG_CLAVE_CAJA_FUERTE, true);
		break;
	case 108: // glue + patches
	case 109: {

		if (_state->hasInventoryItem(110) == true) {
			if (_state->hasInventoryItem(109) == true && _state->hasInventoryItem(108) == true) {
				_state->removeInventoryItem(110);
				_state->removeInventoryItem(109);
				_state->removeInventoryItem(108);
				addInventoryItem(83);
				_dialog->say(_res->_ingameTexts[kTextMuecoArreglado]);
				return;
			} else if (_state->hasInventoryItem(109) == true) {
				_dialog->say(_res->_ingameTexts[kTextNoTengoParches]);

			} else if (_state->hasInventoryItem(108) == true) {
				_dialog->say(_res->_ingameTexts[kTextNoTengoPegamento]);
			}
		} else {
			sayRandomIncorrectResponse();
		}
		break;
	}
	case 84: {
		playAlfredSpecialAnim(1);
		loadExtraScreenAndPresent(7);
		break;
	}
	case 97: { // pyramid map
		playAlfredSpecialAnim(1);
		loadExtraScreenAndPresent(11);
		_dialog->say(_res->_ingameTexts[kTextMeHanTomadoElPelo]);
		_state->setCurrentRoot(43, 1, 0);
		break;
	}
	case 98: { // correct pyramid blueprints
		chooseCorrectDoor();
		break;
	}
	case 87: {
		_dialog->say(_res->_ingameTexts[kTextNecesitaGasolina]);
		break;
	}
	case 95: {
		CDPlayer player(_events, _res, _sound);
		player.run();
		break;
	}
	case 96: {
		BackgroundBook book(_events, _res, _room);
		book.run();
		break;
	}
	default: {
		// Original game incorrectly checked until object 47; Reading any book.
		if (inventoryObject >= 11 && inventoryObject <= 58) {
			playAlfredSpecialAnim(0);
			_dialog->say(_res->_ingameTexts[kTextLibroAburrido]);
			return;
		}
		sayRandomIncorrectResponse();
		break;
	}
	}
}

void PelrockEngine::sayRandomIncorrectResponse() {
	byte response = (byte)getRandomNumber(15);
	_dialog->say(_res->_ingameTexts[154 + response]);
}

void PelrockEngine::chooseCorrectDoor() {
	playAlfredSpecialAnim(1);
	byte puertaBuena = _state->getFlag(FLAG_PUERTA_BUENA);
	if (puertaBuena == 0) { // if not set yet, choose randomly
		int choice = getRandomNumber(1);
		_state->setFlag(FLAG_PUERTA_BUENA, choice + 1);
	}
	puertaBuena = _state->getFlag(FLAG_PUERTA_BUENA);
	Common::String doorText = _res->_izquierda;
	if (puertaBuena == 1) {
		doorText = _res->_izquierda;
	} else if (puertaBuena == 2) {
		doorText = _res->_derecha;
	}
	Common::StringArray fullTextArray = _res->_ingameTexts[kTextPuertaAutenticaIzquierda];
	fullTextArray[0] = fullTextArray[0].substr(0, 45);
	fullTextArray[0].append(doorText);
	_dialog->say(fullTextArray);
}

/**
 * When using amulet with statue, the statue turns red-ish and then starts to talk.
 */
void PelrockEngine::animateStatuePaletteFade(bool reverse) {
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
	exeFile.seek(kStatuePaletteDataOffset, SEEK_SET);

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

	const int numFrames = 7;

	byte currentPalette[768];
	memcpy(currentPalette, _room->_roomPalette, 768);

	// Perform the fade animation
	int frame = 0;
	while (!shouldQuit() && frame <= numFrames) {
		_events->pollEvent();

		bool didRender = renderScene(OVERLAY_NONE);
		if (didRender) {

			for (int i = 0; i < 16; i++) {
				byte paletteIndex = paletteData.indices[i];

				byte *srcColor = reverse ? paletteData.target[i] : paletteData.source[i];
				byte *dstColor = reverse ? paletteData.source[i] : paletteData.target[i];

				// Linear interpolation (6-bit VGA values)
				byte r6 = srcColor[0] + ((dstColor[0] - srcColor[0]) * frame) / numFrames;
				byte g6 = srcColor[1] + ((dstColor[1] - srcColor[1]) * frame) / numFrames;
				byte b6 = srcColor[2] + ((dstColor[2] - srcColor[2]) * frame) / numFrames;

				currentPalette[paletteIndex * 3 + 0] = r6 << 2;
				currentPalette[paletteIndex * 3 + 1] = g6 << 2;
				currentPalette[paletteIndex * 3 + 2] = b6 << 2;
			}

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
		if (_state->getFlag(FLAG_AGENCIA_ABIERTA) == false) {
			_room->addStickerToRoom(19, 54, PERSIST_BOTH);
			_room->addStickerToRoom(19, 55, PERSIST_BOTH);
			_room->addStickerToRoom(19, 56, PERSIST_BOTH);
			_room->addStickerToRoom(19, 58, PERSIST_BOTH);
			_state->setFlag(FLAG_AGENCIA_ABIERTA, true);
		}
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
 * Picking up matches in room 28 has the palette change to "lighten it up".
 */
void PelrockEngine::pickUpMatches(HotSpot *hotspot) {

	byte targetPalette[768];
	_res->getPaletteForRoom28(targetPalette);

	// Fade from current palette to the new palette
	_graphics->fadePaletteToTarget(targetPalette, 25);
	debug("Finished palette fade for room 28 object pickup");
	// Pick up the item
	_room->disableHotspot(hotspot);
	Common::copy(targetPalette, targetPalette + 768, _room->_roomPalette);
	_state->setFlag(FLAG_CROCODILLO_ENCENDIDO, true);
	_room->moveHotspot(_room->findHotspotByExtra(87), 415, 171);
	_room->moveHotspot(_room->findHotspotByExtra(88), 305, 217);
	_room->moveHotspot(_room->findHotspotByExtra(89), 201, 239);
	_room->moveHotspot(_room->findHotspotByExtra(112), 261, 259);
}

/**
 *  Simulates white noise and "crashes" the game
 */
void PelrockEngine::antiPiracyEffect() {
	_sound->stopAllSounds();
	_sound->stopMusic();

	// Generate noise
	const int kNoiseLength = 8000; // ~1 second at 8000 Hz, will loop
	byte *noiseData = (byte *)malloc(kNoiseLength);
	for (int i = 0; i < kNoiseLength; i++) {
		noiseData[i] = (byte)((getRandomNumber(255)));
	}

	// Create a looping raw audio stream with the random noise data
	Audio::SeekableAudioStream *rawStream = Audio::makeRawStream(noiseData, kNoiseLength, 8000, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
	Audio::AudioStream *loopStream = Audio::makeLoopingAudioStream(rawStream, 0);

	Audio::SoundHandle noiseHandle;
	g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &noiseHandle, loopStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES);

	// Set a grayscale palette so random pixel values produce snow effect
	byte grayPalette[256 * 3];
	for (int i = 0; i < 256; i++) {
		grayPalette[i * 3 + 0] = (byte)i; // R
		grayPalette[i * 3 + 1] = (byte)i; // G
		grayPalette[i * 3 + 2] = (byte)i; // B
	}
	g_system->getPaletteManager()->setPalette(grayPalette, 0, 256);

	byte *screenPixels = (byte *)_screen->getPixels();

	while (!shouldQuit()) {
		_events->pollEvent();

		if (_events->_lastKeyEvent != Common::KEYCODE_INVALID) {
			g_system->getMixer()->stopHandle(noiseHandle);
			g_system->getPaletteManager()->setPalette(_room->_roomPalette, 0, 256);
			// Original divides by zero to intentionally crash to DOS.
			// We exit the game instead
			g_engine->quitGame();
		}

		// generate white-noise like scene
		int screenSize = 640 * 400;
		for (int i = 0; i < screenSize; i++) {
			screenPixels[i] = (byte)getRandomNumber(255);
		}

		_screen->markAllDirty();
		_screen->update();
		g_system->delayMillis(10);
	}
}

} // End of namespace Pelrock
