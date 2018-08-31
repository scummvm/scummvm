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

#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/fs.h"
#include "common/system.h"
#include "graphics/surface.h"
#include "graphics/screen.h"
#include "graphics/palette.h"
#include "common/timer.h"
#include "audio/mixer.h"

#include "cryo/defs.h"
#include "cryo/cryo.h"
#include "cryo/platdefs.h"
#include "cryo/cryolib.h"
#include "cryo/eden.h"
#include "cryo/sound.h"
#include "cryo/eden_graphics.h"

namespace Cryo {

#define Z_RESET -3400
#define Z_STEP 200
#define Z_UP 1
#define Z_DOWN -1

EdenGame::EdenGame(CryoEngine *vm) : _vm(vm), kMaxMusicSize(2200000) {
	static uint8 statTab2CB1E[8][4] = {
		{ 0x10, 0x81,    1, 0x90},
		{ 0x90,    1, 0x81, 0x10},
		{    1, 0x90, 0x10, 0x81},
		{    1, 0x10, 0x90, 0x81},
		{    1, 0x90, 0x10, 0x81},
		{ 0x81, 0x10, 0x90,    1},
		{ 0x81, 0x10, 0x90,    1},
		{ 0x81, 0x90,    1, 0x10}
	};

	_graphics = nullptr;

	_adamMapMarkPos = Common::Point(-1, -1);

	_scrollPos = _oldScrollPos = 0;
	_frescoTalk = false;
	_torchCursor = false;
	_curBankNum = 0;
	_paletteUpdateRequired = false;
	_cursorSaved = false;
	_backgroundSaved = false;
	_bankData = nullptr;
	_tyranPtr = nullptr;
	_lastAnimFrameNumb = _curAnimFrameNumb = 0;
	_lastAnimTicks = 0;
	_numAnimFrames = _maxPersoDesc = _numImgDesc = 0;
	_restartAnimation = _animationActive = false;
	_animationDelay = _animationIndex = _lastAnimationIndex = 0;
	dword_30724 = dword_30728 = _mouthAnimations = _animationTable = nullptr;
	_characterBankData = nullptr;
	_numTextLines = 0;
	_textOutPtr = textout = nullptr;
	_curSpecialObject = nullptr;
	_lastDialogChoice = false;
	parlemoiNormalFlag = false;
	_closeCharacterDialog = false;
	dword_30B04 = 0;
	_lastPhrasesFile = 0;
	_dialogSkipFlags = 0;
	_voiceSamplesBuffer = nullptr;
	
	_mainBankBuf = nullptr;
	_musicBuf = nullptr;
	_gameLipsync = nullptr;
	_gamePhrases = nullptr;
	_gameDialogs = nullptr;
	_gameConditions = nullptr;
	_placeRawBuf = nullptr;
	_bankDataBuf = nullptr;
	_gameIcons = nullptr;
	_gameRooms = nullptr;
	_glowBuffer = nullptr;
	_gameFont = nullptr;
	_globals = nullptr;
	_mouseCenterY = _mouseCenterX = 0;
	_bufferAllocationErrorFl = _quitFlag2 = _quitFlag3 = false;
	_gameStarted = false;
	_soundAllocated = false;
	_musicChannel = _voiceChannel = nullptr;
	_hnmSoundChannel = nullptr;
	_voiceSound = nullptr;
	_cirsorPanX = 0;
	_inventoryScrollDelay = 0;
	_cursorPosY = _cursorPosX = 0;
	_currCursor = 0;
	_currSpot = _curSpot2 = nullptr;
	_mouseHeld = false;
	_normalCursor = false;
	_specialTextMode = false;
	_voiceSamplesSize = 0;
	_animateTalking = false;
	_personTalking = false;
	_musicFadeFlag = 0;
	_musicPlayingFlag = false;
	_musicSamplesPtr = _musicPatternsPtr = _musSequencePtr = nullptr;
	_musicEnabledFlag = false;
	_currentObjectLocation = nullptr;
	byte_31D64 = false;
	_noPalette = false;
	_gameLoaded = false;
	memset(_tapes, 0, sizeof(_tapes));
	_confirmMode = 0;
	_curSliderValuePtr = nullptr;
	_lastMenuItemIdLo = 0;
	_lastTapeRoomNum = 0;
	_curSliderX = _curSliderY = 0;
	_destinationRoom = 0;
	word_31E7A = 0;
	word_378CC = 0; //TODO: set by CLComputer_Init to 0
	word_378CE = 0;

	_rotationAngleY = _rotationAngleX = _rotationAngleZ = 0;
	_translationY = _translationX = 0.0;	//TODO: never changed, make consts?
	_cursorOldTick = 0;

	_invIconsBase = 19;
//	invIconsCount = (_vm->getPlatform() == Common::kPlatformMacintosh) ? 9 : 11;
	_invIconsCount = 11;
	_roomIconsBase = _invIconsBase + _invIconsCount;

	_codePtr = nullptr;

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 4; j++)
			tab_2CB1E[i][j] = statTab2CB1E[i][j];
	}

	_translationZ = Z_RESET;
	_zDirection = Z_UP;

	_torchTick = 0;
	_glowIndex = 0;
	_torchCurIndex = 0;
	_cursCenter = 11;
}

EdenGame::~EdenGame() {
}

void EdenGame::removeConsole() {
}

void EdenGame::scroll() {
	restoreFriezes();
	_graphics->getMainView()->_normal._srcLeft = _scrollPos;
	_graphics->getMainView()->_zoom._srcLeft = _scrollPos;
}

void EdenGame::resetScroll() {
	_oldScrollPos = _scrollPos;
	_scrollPos = 0;
	restoreFriezes();   //TODO: inlined scroll() ?
	_graphics->getMainView()->_normal._srcLeft = 0;
	_graphics->getMainView()->_zoom._srcLeft = 0;
}

void EdenGame::scrollFrescoes() {
	if (_cursorPosY > 16 && _cursorPosY < 176) {
		if (_cursorPosX >= 0 && _cursorPosX < 32 && _scrollPos > 3)
			_scrollPos -= 4;
		else if (_cursorPosX > 288 && _cursorPosX < 320 && _scrollPos < _globals->_frescoeWidth)
			_scrollPos += 4;
	}
	scroll();
}

bool EdenGame::animationIsActive() {
	return _animationActive;
}

// Original name: afffresques
void EdenGame::displayFrescoes() {
	useBank(_globals->_frescoeImgBank);
	_graphics->drawSprite(0, 0, 16);
	useBank(_globals->_frescoeImgBank + 1);
	_graphics->drawSprite(0, 320, 16);
	_paletteUpdateRequired = true;
}

void EdenGame::setVolume(uint16 vol) {
	_hnmSoundChannel->setVolumeLeft(vol);
	_hnmSoundChannel->setVolumeRight(vol);
}

void EdenGame::gametofresques() {
	_frescoTalk = false;
	_graphics->rundcurs();
	saveFriezes();
	displayFrescoes();
	_globals->_displayFlags = DisplayFlags::dfFrescoes;
}

// Original name: dofresques
void EdenGame::doFrescoes() {
	_cursorSaved = false;
	_torchCursor = true;
	_graphics->setGlowX(-1);
	_graphics->setGlowY(-1);
	_globals->_gameFlags |= GameFlags::gfFlag20;
	_globals->_varD4 = 0;
	_globals->_curObjectId = 0;
	_globals->_iconsIndex = 13;
	_globals->_autoDialog = false;
	gametofresques();
	_globals->_frescoNumber = 3;
}

// Original name: finfresques
void EdenGame::actionEndFrescoes() {
	_torchCursor = false;
	_cursorSaved = true;
	_globals->_displayFlags = DisplayFlags::dfFlag1;
	resetScroll();
	_globals->_var100 = 0xFF;
	updateRoom(_globals->_roomNum);
	if (_globals->_phaseNum == 114)
		_globals->_narratorSequence = 1;
	_globals->_eventType = EventType::etEvent8;
	showEvents();
}

void EdenGame::scrollMirror() {
	if (_cursorPosY > 16 && _cursorPosY < 165) {
		if (_cursorPosX >= 0 && _cursorPosX < 16) {
			if (_scrollPos > 3) {
				_scrollPos--;
				scroll();
			}
		} else if (_cursorPosX > 290 && _cursorPosX < 320) {
			if (_scrollPos < 320) {
				_scrollPos++;
				scroll();
			}
		}
	}
}

// Original name: scrollpano
void EdenGame::scrollPanel() {
	if (_cursorPosY > 16 && _cursorPosY < 165) {
		if (_cursorPosX >= 0 && _cursorPosX < 16 && _scrollPos > 3)
			_scrollPos--;
		else if (_cursorPosX > 290 && _cursorPosX < 320 && _scrollPos < 320)
			_scrollPos++;
	}
	scroll();
}

// Original name: affsuiveur
void EdenGame::displayFollower(Follower *follower, int16 x, int16 y) {
	useBank(follower->_spriteBank);
	_graphics->drawSprite(follower->_spriteNum, x, y + 16);
}

// Original name: persoinmiroir
void EdenGame::characterInMirror() {
	Icon *icon1 = &_gameIcons[3];
	Icon *icon = &_gameIcons[_roomIconsBase];
	Follower *suiveur = _followerList;
	int16 num = 1;
	for (int i = 0; i < 16; i++) {
		if (_globals->_party & (1 << i))
			num++;
	}
	icon += num;
	icon->sx = -1;
	icon--;
	icon->sx = icon1->sx;
	icon->sy = icon1->sy;
	icon->ex = icon1->ex;
	icon->ey = 170;
	icon->_cursorId = icon1->_cursorId;
	icon->_actionId = icon1->_actionId;
	icon->_objectId = icon1->_objectId;
	icon--;
	displayFollower(suiveur, suiveur->sx, suiveur->sy);
	for (; suiveur->_id != -1; suiveur++) {
		perso_t *perso;
		for (perso = _persons; perso != &_persons[PER_UNKN_156]; perso++) {
			if (perso->_id != suiveur->_id)
				continue;

			if (perso->_flags & PersonFlags::pf80)
				continue;

			if ((perso->_flags & PersonFlags::pfInParty) == 0)
				continue;

			if (perso->_roomNum != _globals->_roomNum)
				continue;

			icon->sx = suiveur->sx;
			icon->sy = suiveur->sy;
			icon->ex = suiveur->ex;
			icon->ey = suiveur->ey;
			icon->_cursorId = 8;
			icon->_actionId = perso->_actionId;
			icon--;
			displayFollower(suiveur, suiveur->sx, suiveur->sy);
			break;
		}
	}
}

// Original name: gametomiroir
void EdenGame::gameToMirror(byte arg1) {
	if (_globals->_displayFlags != DisplayFlags::dfFlag2) {
		_graphics->rundcurs();
		restoreFriezes();
		drawTopScreen();
		showObjects();
		saveFriezes();
	}
	int16 bank = _globals->_roomBackgroundBankNum;
	uint16 resNum = bank + 326;
	if ((_vm->getPlatform() == Common::kPlatformMacintosh) && (bank == 76 || bank == 128))
			resNum = 2487;				// PCIMG.HSQ

	useBank(resNum);
	_graphics->drawSprite(0, 0, 16);
	useBank(resNum + 1);
	_graphics->drawSprite(0, 320, 16);
	characterInMirror();
	_paletteUpdateRequired = true;
	_globals->_iconsIndex = 16;
	_globals->_autoDialog = false;
	_globals->_displayFlags = DisplayFlags::dfMirror;
	_globals->_mirrorEffect = arg1;
}

void EdenGame::flipMode() {
	if (_personTalking) {
		endCharacterSpeech();
		if (_globals->_displayFlags == DisplayFlags::dfPerson) {
			if (_globals->_characterPtr == &_persons[PER_TAU] && _globals->_phaseNum >= 80)
				_graphics->displaySubtitles();
			else {
				getDataSync();
				loadCurrCharacter();
				addanim();
				_restartAnimation = true;
				animCharacter();
			}
		} else
			_graphics->displaySubtitles();
		persovox();
	} else {
		if (_globals->_displayFlags != DisplayFlags::dfFrescoes && _globals->_displayFlags != DisplayFlags::dfFlag2) {
			closeRoom();
			if (_globals->_displayFlags & DisplayFlags::dfFlag1)
				gameToMirror(1);
			else {
				quitMirror();
				updateRoom(_globals->_roomNum);
				if (byte_31D64) {
					dialautoon();
					parle_moi();
					byte_31D64 = false;
				}
			}
		}
	}
}

// Original name: quitmiroir
void EdenGame::quitMirror() {
	_graphics->rundcurs();
	display();
	resetScroll();
	saveFriezes();
	_globals->_displayFlags = DisplayFlags::dfFlag1;
	_globals->_var100 = 0xFF;
	_globals->_eventType = EventType::etEventC;
	_globals->_mirrorEffect = 1;
}

void EdenGame::clictimbre() {
	flipMode();
}

// Original name: clicplanval
void EdenGame::actionClickValleyPlan() {
	if ((_globals->_partyOutside & PersonMask::pmDina) && _globals->_phaseNum == 371) {
		quitMirror();
		updateRoom(_globals->_roomNum);
		return;
	}
	if (_globals->_roomNum == 8 || _globals->_roomNum < 16)
		return;

	_graphics->rundcurs();
	display();
	if (_globals->_displayFlags == DisplayFlags::dfMirror)
		quitMirror();
	deplaval((_globals->_roomNum & 0xFF00) | 1); //TODO: check me
}

// Original name: gotolieu
void EdenGame::gotoPlace(Goto *go) {
	_globals->_valleyVidNum = go->_arriveVideoNum;
	_globals->_travelTime = go->_travelTime * 256;
	_globals->_stepsToFindAppleFast = 0;
	_globals->_eventType = EventType::etEvent2;
	setChoiceYes();
	showEvents();
	if (!isAnswerYes())
		return;

	if (_globals->_var113) {
		waitEndSpeak();
		if (!_vm->shouldQuit())
			closeCharacterScreen();
	}
	if (go->_enterVideoNum) {
		_graphics->hideBars();
		_graphics->playHNM(go->_enterVideoNum);
		_graphics->setFade(true);
	}
	initPlace(_globals->_newRoomNum);
	specialoutside();
	faire_suivre(_globals->_newRoomNum);
	closeRoom();
	_adamMapMarkPos.x = -1;
	_adamMapMarkPos.y = -1;
	addTime(_globals->_travelTime);
	_globals->_var100 = _globals->_roomPtr->_id;
	_globals->_roomNum = _globals->_newRoomNum;
	_globals->_areaNum = _globals->_roomNum >> 8;
	_globals->_eventType = EventType::etEvent5;
	_globals->_newMusicType = MusicType::mt2;
	setCharacterHere();
	musique();
	updateRoom1(_globals->_roomNum);
	drawTopScreen();
	_adamMapMarkPos.x = -1;
	_adamMapMarkPos.y = -1;
}

void EdenGame::deplaval(uint16 roomNum) {
	_globals->_newLocation = roomNum & 0xFF;
	_globals->_valleyVidNum = 0;
	_globals->_phaseActionsCount++;
	closeRoom();
	endCharacterSpeech();
	byte c1 = roomNum & 0xFF;
	if (c1 == 0)
		return;

	if (c1 < 0x80) {
		_globals->_displayFlags = DisplayFlags::dfFlag1;
		setChoiceYes();
		_globals->_eventType = EventType::etEvent1;
		showEvents();
		if (!isAnswerYes())
			return;

		if (_globals->_var113) {
			waitEndSpeak();
			if (!_vm->shouldQuit())
				closeCharacterScreen();
		}
		specialout();
		if (_globals->_areaPtr->_type == AreaType::atValley) {
			addTime(32);
			_globals->_stepsToFindAppleFast++;
			_globals->_stepsToFindAppleNormal++;
		}
		faire_suivre((roomNum & 0xFF00) | _globals->_newLocation);
		_globals->_var100 = _globals->_roomPtr->_id;
		_globals->_roomNum = roomNum;
		_globals->_areaNum = roomNum >> 8;
		_globals->_eventType = EventType::etEvent5;
		setCharacterHere();
		_globals->_newMusicType = MusicType::mtNormal;
		musique();
		updateRoom1(roomNum);
		_globals->_chronoFlag = 0;
		_globals->_chrono = 0;
		_globals->_var54 = 0;
		if (_globals->_roomCharacterType == PersonFlags::pftTyrann)
			setChrono(3000);
		return;
	}
	if (c1 == 0xFF) {
		_globals->_eventType = EventType::etEventE;
		showEvents();
		if (!_persons[PER_ELOI]._roomNum && checkEloiReturn())
			setChrono(800);
		return;
	}
	_globals->_stepsToFindAppleFast = 0;
	byte newAreaNum = c1 & 0x7F;
	byte curAreaNum = _globals->_roomNum >> 8;
	int16 newRoomNum = newAreaNum << 8;
	if (curAreaNum == Areas::arTausCave && newAreaNum == Areas::arMo)
		newRoomNum |= 0x16;
	else if (curAreaNum == Areas::arMoorkusLair)
		newRoomNum |= 4;
	else
		newRoomNum |= 1;
	_globals->_newRoomNum = newRoomNum;
	if (newAreaNum == Areas::arTausCave)
		gotoPlace(&_gotos[0]);
	else {
		for (Goto *go = _gotos + 1; go->_curAreaNum != 0xFF; go++) {
			if (go->_curAreaNum == curAreaNum) {
				gotoPlace(go);
				break;
			}
		}
	}
}

// Original name: deplacement
void EdenGame::move(Direction dir) {
	Room *room = _globals->_roomPtr;
	int16 roomNum = _globals->_roomNum;
	debug("move: from room %4X", roomNum);
	char newLoc = 0;
	_graphics->rundcurs();
	display();
	_globals->_prevLocation = roomNum & 0xFF;
	switch (dir) {
	case kCryoNorth:
		newLoc = room->_exits[0];
		break;
	case kCryoEast:
		newLoc = room->_exits[1];
		break;
	case kCryoSouth:
		newLoc = room->_exits[2];
		break;
	case kCryoWest:
		newLoc = room->_exits[3];
		break;
	default:
		break;
	}
	deplaval((roomNum & 0xFF00) | newLoc);
}

// Original name: deplacement2
void EdenGame::move2(Direction dir) {
	Room *room = _globals->_roomPtr;
	int16 roomNum = _globals->_roomNum;
	char newLoc = 0;
	_globals->_prevLocation = roomNum & 0xFF;
	switch (dir) {
	case kCryoNorth:
		newLoc = room->_exits[0];
		break;
	case kCryoEast:
		newLoc = room->_exits[1];
		break;
	case kCryoSouth:
		newLoc = room->_exits[2];
		break;
	case kCryoWest:
		newLoc = room->_exits[3];
		break;
	default:
		break;
	}
	deplaval((roomNum & 0xFF00) | newLoc);
}

// Original name: dinosoufle
void EdenGame::actionDinoBlow() {
	if (_globals->_curObjectId == 0) {
		_graphics->hideBars();
		_graphics->playHNM(148);
		maj2();
	}
}

// Original name: plaquemonk
void EdenGame::actionPlateMonk() {
	if (_globals->_curObjectId != 0) {
		if (_globals->_curObjectId == Objects::obPrism) {
			loseObject(Objects::obPrism);
			_graphics->hideBars();
			_specialTextMode = true;
			_graphics->playHNM(89);
			// CHECKME: Unused code
			// word_2F514 |= 0x8000;
			maj2();
			_globals->_eventType = EventType::etEventB;
			showEvents();
		}
	} else {
		_graphics->hideBars();
		_graphics->playHNM(7);
		maj2();
		_globals->_eventType = EventType::etEvent4;
		showEvents();
	}
}

// Original name: fresquesgraa
void EdenGame::actionGraaFrescoe() {
	if (_globals->_curObjectId == 0) {
		_globals->_frescoeWidth = 320;
		_globals->_frescoeImgBank = 113;
		doFrescoes();
		handleDinaDialog();
	}
}

// Original name: fresqueslasc
void EdenGame::actionLascFrescoe() {
	if (_globals->_curObjectId == 0) {
		_globals->_frescoeWidth = 112;
		_globals->_frescoeImgBank = 315;
		doFrescoes();
	}
}

// Original name: pushpierre
void EdenGame::actionPushStone() {
	if (_globals->_curObjectId == 0) {
		_gameRooms[22]._exits[0] = 17;
		_gameRooms[26]._exits[2] = 9;
		move(kCryoNorth);
	}
}

// Original name: tetemomie
void EdenGame::actionMummyHead() {
	if (_globals->_curObjectId == Objects::obTooth) {
		_globals->_gameFlags |= GameFlags::gfMummyOpened;
		move(kCryoNorth);
	} else if (_globals->_curObjectId == 0) {
		if (_globals->_gameFlags & GameFlags::gfMummyOpened)
			move(kCryoNorth);
		else {
			_globals->_eventType = EventType::etEvent6;
			handleCharacterDialog(PersonId::pidMonk);
			_globals->_eventType = 0;
		}
	}
}

// Original name: tetesquel
void EdenGame::actionSkelettonHead() {
	if (_globals->_curObjectId == Objects::obTooth) {
		_gameRooms[22]._exits[0] = 16;
		_gameRooms[26]._exits[2] = 13;
		_gameIcons[16]._cursorId |= 0x8000;
		loseObject(Objects::obTooth);
		move(kCryoNorth);
	}
}

// Original name: squelmoorkong
void EdenGame::actionSkelettonMoorkong() {
	_globals->_eventType = EventType::etEvent9;
	showEvents();
}

// Original name: choisir
void EdenGame::actionChoose() {
	byte objid = _curSpot2->_objectId;
	byte obj;
	switch (objid) {
	case 0:
		obj = _globals->_giveObj1;
		break;
	case 1:
		obj = _globals->_giveObj2;
		break;
	case 2:
		obj = _globals->_giveObj3;
		break;
	default:
		warning("Unexpected objid in actionChoose()");
		return;
	}
	objectmain(obj);
	winObject(obj);
	_globals->_iconsIndex = 16;
	_globals->_autoDialog = false;
	_globals->_var60 = 0;
	parle_moi();
}

// Original name: dinaparle
void EdenGame::handleDinaDialog() {
	perso_t *perso = &_persons[PER_DINA];
	if (perso->_partyMask & (_globals->_party | _globals->_partyOutside)) {
		if (_globals->_frescoNumber < 3)
			_globals->_frescoNumber = 3;
		_globals->_frescoNumber++;
		if (_globals->_frescoNumber < 15) {
			endCharacterSpeech();
			if (_globals->_frescoNumber == 7 && _globals->_phaseNum == 113)
				incPhase();
			_globals->_characterPtr = perso;
			_globals->_dialogType = DialogType::dtInspect;
			int16 num = (perso->_id << 3) | DialogType::dtInspect; //TODO: combine
			bool res = dialoscansvmas((Dialog *)getElem(_gameDialogs, num));
			_frescoTalk = false;
			if (res) {
				_graphics->restoreUnderSubtitles();
				_frescoTalk = true;
				persovox();
			}
			_globals->_varCA = 0;
			_globals->_dialogType = DialogType::dtTalk;
		} else
			actionEndFrescoes();
	}
}

int16 EdenGame::getCurPosX() {
	return _cursorPosX;
}

int16 EdenGame::getCurPosY() {
	return _cursorPosY;
}

void EdenGame::setCurPosX(int16 xpos) {
	_cursorPosX = xpos;
}

void EdenGame::setCurPosY(int16 ypos) {
	_cursorPosY = ypos;
}

// Original name: roiparle
void EdenGame::handleKingDialog() {
	if (_globals->_phaseNum <= 400)
		handleCharacterDialog(0);
}

// Original name: roiparle1
void EdenGame::actionKingDialog1() {
	if (_globals->_curObjectId == Objects::obSword) {
		_globals->_gameFlags |= GameFlags::gfFlag80;
		_graphics->hideBars();
		_graphics->playHNM(76);
		move2(kCryoNorth);
	} else {
		_globals->_frescoNumber = 1;
		handleKingDialog();
	}
}

// Original name: roiparle2
void EdenGame::actionKingDialog2() {
	_globals->_frescoNumber = 2;
	handleKingDialog();
}

// Original name: roiparle3
void EdenGame::actionKingDialog3() {
	_globals->_frescoNumber = 3;
	handleKingDialog();
}

// Original name: getcouteau
void EdenGame::actionGetKnife() {
	if (_globals->_phaseNum >= 80) {
		_gameRooms[113]._video = 0;
		getObject(Objects::obKnife);
	}
	_globals->_eventType = EventType::etEvent7;
	showEvents();
}

// Original name: getprisme
void EdenGame::actionGetPrism() {
	getObject(Objects::obPrism);
	_globals->_eventType = EventType::etEvent7;
	showEvents();
}

// Original name: getchampb
void EdenGame::actionGetMushroom() {
	getObject(Objects::obShroom);
}

// Original name: getchampm
void EdenGame::actionGetBadMushroom() {
	getObject(Objects::obBadShroom);
}

// Original name: getor
void EdenGame::actionGetGold() {
	getObject(Objects::obGold);
}

// Original name: getnido
void EdenGame::actionGetFullNest() {
	if (_globals->_curObjectId != 0)
		return;
	_globals->_roomPtr->_bank = 282; //TODO: fix me
	_globals->_roomPtr--;
	_globals->_roomPtr->_bank = 281; //TODO: fix me
	_globals->_roomPtr->_id = 3;
	getObject(Objects::obFullNest);
}

// Original name: getnidv
void EdenGame::actionGetEmptyNest() {
	if (_globals->_curObjectId != 0)
		return;
	_globals->_roomPtr->_bank = 282; //TODO: fix me
	_globals->_roomPtr--;
	_globals->_roomPtr->_bank = 281; //TODO: fix me
	_globals->_roomPtr->_id = 3;
	getObject(Objects::obNest);
}

// Original name: getcorne
void EdenGame::actionGetHorn() {
	if (_globals->_curObjectId != 0)
		return;
	getObject(Objects::obHorn);
	_globals->_eventType = EventType::etEvent7;
	showEvents();
	bigphase1();
	setCharacterHere();
	_globals->_roomPtr = getRoom(_globals->_roomNum);
}

// Original name: getsoleil
void EdenGame::actionGetSunStone() {
	if (_globals->_curObjectId != 0)
		return;
	_gameRooms[238]._video = 0;
	_gameRooms[238]._flags = RoomFlags::rf80;
	getObject(Objects::obSunStone);
}

// Original name: getoueuf
void EdenGame::actionGetEgg() {
	if (_globals->_curObjectId != 0)
		return;
	_globals->_roomPtr->_flags = 0;
	_globals->_roomPtr->_video = 0;
	getObject(Objects::obEgg);
}

// Original name: getplaque
void EdenGame::actionGetTablet() {
	if (_globals->_curObjectId != 0 && _globals->_curObjectId < Objects::obTablet1)
		return;
	_globals->_curObjectId = 0;
	getObject(Objects::obTablet2);
	putObject();
	for (int i = 0; i < 6; i++)
		_objects[Objects::obTablet1 - 1 + i]._count = 0;
	_globals->_curObjectFlags = 0;
	_globals->_inventoryScrollPos = 0;
	_globals->_curObjectCursor = 9;
	_gameIcons[16]._cursorId |= 0x8000;
	showObjects();
	_gameRooms[131]._video = 0;
	_graphics->hideBars();
	_graphics->playHNM(149);
	_globals->_varF1 = RoomFlags::rf04;
	_globals->_drawFlags = DrawFlags::drDrawFlag20;
	_normalCursor = true;
	maj2();
}

// Original name: voirlac
void EdenGame::actionLookLake() {
	perso_t *perso = &_persons[PER_MORKUS];
	Room *room = _globals->_roomPtr;
	Area *area = _globals->_areaPtr;
	int16 vid = _globals->_curObjectId == Objects::obApple ? 81 : 54;
	for (++perso; perso->_roomNum != 0xFFFF; perso++) {
		if (perso->_roomNum != _globals->_roomNum)
			continue;
		vid++;
		if (_globals->_curObjectId != Objects::obApple)
			continue;                   //TODO: pc breaks here
		if ((perso->_flags & PersonFlags::pfTypeMask) != PersonFlags::pftMosasaurus)
			continue;
		if (!(perso->_flags & PersonFlags::pf80))
			return;
		perso->_flags &= ~PersonFlags::pf80; //TODO: useless? see above
		area->_flags |= AreaFlags::afFlag8;
		_globals->_curAreaFlags |= AreaFlags::afFlag8;
		room->_id = 3;
	}
	debug("sea monster: room = %X, d0 = %X\n", _globals->_roomNum, _globals->_roomImgBank);
	_graphics->hideBars();
	_graphics->playHNM(vid);
	updateRoom(_globals->_roomNum);           //TODO: getting memory trashed here?
	if (_globals->_curObjectId == Objects::obApple)
		loseObject(Objects::obApple);
	_globals->_eventType = EventType::etEventF;
	showEvents();
}

// Original name: gotohall
void EdenGame::actionGotoHall() {
	_globals->_prevLocation = _globals->_roomNum & 0xFF;
	deplaval((_globals->_roomNum & 0xFF00) | 6);
}

// Original name: demitourlabi
void EdenGame::actionLabyrinthTurnAround() {
	_globals->_prevLocation = _globals->_roomNum & 0xFF;
	_globals->_var100 = 0xFF;
	uint16 target = (_globals->_roomNum & 0xFF00) | _globals->_roomPtr->_exits[2];
	faire_suivre(target);
	_globals->_roomNum = target;
	_globals->_eventType = EventType::etEvent5;
	updateRoom(_globals->_roomNum);
}

// Original name: gotonido
void EdenGame::actionGotoFullNest() {
	_globals->_roomPtr++;
	_globals->_eventType = 0;
	_globals->_roomImgBank = _globals->_roomPtr->_bank;
	_globals->_roomVidNum = _globals->_roomPtr->_video;
	_globals->_curRoomFlags = _globals->_roomPtr->_flags;
	_globals->_varF1 = _globals->_roomPtr->_flags;
	animpiece();
	_globals->_var100 = 0;
	maj2();
}

// Original name: gotoval
void EdenGame::actionGotoVal() {
	uint16 target = _globals->_roomNum;
	_graphics->rundcurs();
	display();
	_scrollPos = 0;
	char obj = _curSpot2->_objectId - 14;    //TODO
	_globals->_prevLocation = target & 0xFF;
	deplaval((target & 0xFF00) | obj);
}

// Original name: visiter
void EdenGame::actionVisit() {
	_graphics->hideBars();
	_graphics->playHNM(144);
	_globals->_varF1 = RoomFlags::rf04;
	maj2();
}

// Original name: final
void EdenGame::actionFinal() {
	if (_globals->_curObjectId != 0)
		return;

	_graphics->hideBars();
	*(int16 *)(_gameRooms + 0x6DC) = 319; //TODO
	_globals->_roomImgBank = 319;
	_graphics->playHNM(97);
	maj2();
	_globals->_eventType = EventType::etEvent12;
	showEvents();
	_globals->_narratorSequence = 54;
}

// Original name: goto_nord
void EdenGame::actionMoveNorth() {
	if (_globals->_curObjectId == 0)
		move(kCryoNorth);
}

// Original name: goto_est
void EdenGame::actionMoveEast() {
	if (_globals->_curObjectId == 0)
		move(kCryoEast);
}

// Original name: goto_sud
void EdenGame::actionMoveSouth() {
	if (_globals->_curObjectId == 0)
		move(kCryoSouth);
}

// Original name: goto_ouest
void EdenGame::actionMoveWest() {
	if (_globals->_curObjectId == 0)
		move(kCryoWest);
}

// Original name: afficher
void EdenGame::display() {
	if (!_globals->_mirrorEffect && !_globals->_var103) {
		if (_paletteUpdateRequired) {
			_paletteUpdateRequired = false;
			_graphics->SendPalette2Screen(256);
		}
		CLBlitter_CopyView2Screen(_graphics->getMainView());
	} else {
		if (_globals->_mirrorEffect)
			_graphics->displayEffect3();
		else
			_graphics->displayEffect2();

		_globals->_var103 = 0;
		_globals->_mirrorEffect = 0;
	}
}

void EdenGame::afficher128() {
	if (_globals->_updatePaletteFlag == 16) {
		_graphics->SendPalette2Screen(129);
		CLBlitter_CopyView2Screen(_graphics->getMainView());
		_globals->_updatePaletteFlag = 0;
	} else {
		_graphics->clearScreen();
		_graphics->fadeToBlackLowPalette(1);
		if (_graphics->getShowBlackBars())
			_graphics->drawBlackBars();
		CLBlitter_CopyView2Screen(_graphics->getMainView());
		_graphics->fadeFromBlackLowPalette(1);
	}
}

// Original name: sauvefrises
void EdenGame::saveFriezes() {
	_graphics->saveTopFrieze(0);
	_graphics->saveBottomFrieze();
}

// Original name: restaurefrises
void EdenGame::restoreFriezes() {
	_graphics->restoreTopFrieze();
	_graphics->restoreBottomFrieze();
}



byte * EdenGame::getBankData() {
	return _bankData;
}

void EdenGame::useMainBank() {
	_bankData = _mainBankBuf;
}

void EdenGame::useCharacterBank() {
	_bankData = _characterBankData;
}

// Original name: use_bank
void EdenGame::useBank(int16 bank) {
	if (bank > 2500)
		error("attempt to load bad bank %d", bank);

	_bankData = _bankDataBuf;
	if (_curBankNum != bank) {
		loadRawFile(bank, _bankDataBuf);
		verifh(_bankDataBuf);
		_curBankNum = bank;
	}
}

void EdenGame::drawTopScreen() {  // Draw  top bar (location / party / map)
	_globals->_drawFlags &= ~DrawFlags::drDrawTopScreen;
	useBank(314);
	_graphics->drawSprite(36, 83, 0);
	_graphics->drawSprite(_globals->_areaPtr->_num - 1, 0, 0);
	_graphics->drawSprite(23, 145, 0);
	for (perso_t *perso = &_persons[PER_DINA]; perso != &_persons[PER_UNKN_156]; perso++) {
		if ((perso->_flags & PersonFlags::pfInParty) && !(perso->_flags & PersonFlags::pf80))
			_graphics->drawSprite(perso->_targetLoc + 18, perso->_lastLoc + 120, 0);
	}
	_adamMapMarkPos.x = -1;
	_adamMapMarkPos.y = -1;
	displayValleyMap();
	_paletteUpdateRequired = true;
}

// Original name: affplanval
void EdenGame::displayValleyMap() { // Draw mini-map
	if (_globals->_areaPtr->_type == AreaType::atValley) {
		_graphics->drawSprite(_globals->_areaPtr->_num + 9, 266, 1);
		for (perso_t *perso = &_persons[PER_UNKN_18C]; perso->_roomNum != 0xFFFF; perso++) {
			if (((perso->_roomNum >> 8) == _globals->_areaNum)
			        && !(perso->_flags & PersonFlags::pf80) && (perso->_flags & PersonFlags::pf20))
				displayMapMark(33, perso->_roomNum & 0xFF);
		}
		if (_globals->_areaPtr->_citadelLevel)
			displayMapMark(34, _globals->_areaPtr->_citadelRoomPtr->_location);
		_graphics->saveTopFrieze(0);
		int16 loc = _globals->_roomNum & 0xFF;
		if (loc >= 16)
			displayAdamMapMark(loc);
		_graphics->restoreTopFrieze();
	} else {
		_graphics->saveTopFrieze(0);
		_graphics->restoreTopFrieze();
	}
}

// Original name: affrepere
void EdenGame::displayMapMark(int16 index, int16 location) {
	_graphics->drawSprite(index, 269 + location % 16 * 4, 2 + (location - 16) / 16 * 3);
}

// Original name: affrepereadam
void EdenGame::displayAdamMapMark(int16 location) {
	int16 x = 269;
	int16 y = 2;
	restoreAdamMapMark();
	if (location > 15 && location < 76) {
		x += (location & 15) * 4;
		y += ((location - 16) >> 4) * 3;
		saveAdamMapMark(x, y);
		byte *pix = _graphics->getUnderBarsView()->_bufferPtr;
		int16 w = _graphics->getUnderBarsView()->_width;
		pix += x + w * y;
		pix[1] = 0xC3;
		pix[2] = 0xC3;
		pix += w;
		pix[0] = 0xC3;
		pix[1] = 0xC3;
		pix[2] = 0xC3;
		pix[3] = 0xC3;
		pix += w;
		pix[1] = 0xC3;
		pix[2] = 0xC3;
	}
}

// Original name: rest_repadam
void EdenGame::restoreAdamMapMark() {
	if (_adamMapMarkPos.x == -1 && _adamMapMarkPos.y == -1)
		return;

	int16 x = _adamMapMarkPos.x;
	int16 y = _adamMapMarkPos.y;
	byte *pix = _graphics->getUnderBarsView()->_bufferPtr;
	int16 w = _graphics->getUnderBarsView()->_width;
	pix += x + w * y;
	pix[1] = _oldPix[0];
	pix[2] = _oldPix[1];
	pix += w;
	pix[0] = _oldPix[2];
	pix[1] = _oldPix[3];
	pix[2] = _oldPix[4];
	pix[3] = _oldPix[5];
	pix += w;
	pix[1] = _oldPix[6];
	pix[2] = _oldPix[7];
}

// Original name: save_repadam
void EdenGame::saveAdamMapMark(int16 x, int16 y) {
	_adamMapMarkPos.x = x;
	_adamMapMarkPos.y = y;
	byte *pix = _graphics->getUnderBarsView()->_bufferPtr;
	int16 w = _graphics->getUnderBarsView()->_width;
	pix += x + w * y;
	_oldPix[0] = pix[1];
	_oldPix[1] = pix[2];
	pix += w;
	_oldPix[2] = pix[0];
	_oldPix[3] = pix[1];
	_oldPix[4] = pix[2];
	_oldPix[5] = pix[3];
	pix += w;
	_oldPix[6] = pix[1];
	_oldPix[7] = pix[2];
}

bool EdenGame::istrice(int16 roomNum) {
	char loc = roomNum & 0xFF;
	int16 area = roomNum & 0xFF00;
	for (perso_t *perso = &_persons[PER_UNKN_18C]; perso != &_persons[PER_UNKN_372]; perso++) {
		if ((perso->_flags & PersonFlags::pf80) || (perso->_flags & PersonFlags::pfTypeMask) != PersonFlags::pftTriceraptor)
			continue;
		if (perso->_roomNum == (area | (loc - 16)))
			return true;
		if (perso->_roomNum == (area | (loc + 16)))
			return true;
		if (perso->_roomNum == (area | (loc - 1)))
			return true;
		if (perso->_roomNum == (area | (loc + 1)))
			return true;
	}
	return false;
}

bool EdenGame::istyran(int16 roomNum) {
	char loc = roomNum & 0xFF;
	int16 area = roomNum & 0xFF00;
	// TODO: orig bug: this ptr is not initialized when first called from getsalle
	// PC version scans _persons[] directly and is not affected
	if (!_tyranPtr)
		return false;

	for (; _tyranPtr->_roomNum != 0xFFFF; _tyranPtr++) {
		if (_tyranPtr->_flags & PersonFlags::pf80)
			continue;
		if (_tyranPtr->_roomNum == (area | (loc - 16)))
			return true;
		if (_tyranPtr->_roomNum == (area | (loc + 16)))
			return true;
		if (_tyranPtr->_roomNum == (area | (loc - 1)))
			return true;
		if (_tyranPtr->_roomNum == (area | (loc + 1)))
			return true;
	}
	return false;
}

void EdenGame::istyranval(Area *area) {
	byte areaNum = area->_num;
	area->_flags &= ~AreaFlags::HasTyrann;
	for (perso_t *perso = &_persons[PER_UNKN_372]; perso->_roomNum != 0xFFFF; perso++) {
		if (perso->_flags & PersonFlags::pf80)
			continue;

		if ((perso->_roomNum >> 8) == areaNum) {
			area->_flags |= AreaFlags::HasTyrann;
			break;
		}
	}
}

char EdenGame::getDirection(perso_t *perso) {
	char dir = -1;
	byte trgLoc = perso->_targetLoc;
	byte curLoc = perso->_roomNum & 0xFF;   //TODO name
	if (curLoc != trgLoc) {
		curLoc &= 0xF;
		trgLoc &= 0xF;
		if (curLoc != trgLoc) {
			dir = 2;
			if (curLoc > trgLoc)
				dir = 5;
		}
		trgLoc = perso->_targetLoc;
		curLoc = perso->_roomNum & 0xFF;
		curLoc &= 0xF0;
		trgLoc &= 0xF0;
		if (curLoc != trgLoc) {
			if (curLoc > trgLoc)
				dir++;
			dir++;
		}
	}
	return dir;
}

// Original name: caselibre
bool EdenGame::canMoveThere(char loc, perso_t *perso) {
	Room *room = _globals->_citaAreaFirstRoom;
	if (loc <= 0x10 || loc > 76 || (loc & 0xF) >= 12 || loc == perso->_lastLoc)
		return false;

	int16 roomNum = (perso->_roomNum & ~0xFF) | loc;   //TODO: danger! signed
	if (roomNum == _globals->_roomNum)
		return false;

	for (; room->_id != 0xFF; room++) {
		if (room->_location != loc)
			continue;
		if (!(room->_flags & RoomFlags::rf01))
			return false;
		for (perso = &_persons[PER_UNKN_18C]; perso->_roomNum != 0xFFFF; perso++) {
			if (perso->_flags & PersonFlags::pf80)
				continue;
			if (perso->_roomNum == roomNum)
				break;
		}
		if (perso->_roomNum != 0xFFFF)
			return false;
		return true;
	}
	return false;
}

// Original name: melange1
void EdenGame::scramble1(uint8 elem[4]) {
	if (_vm->_rnd->getRandomNumber(1) & 1)
		SWAP(elem[1], elem[2]);
}

// Original name melange2
void EdenGame::scramble2(uint8 elem[4]) {
	if (_vm->_rnd->getRandomNumber(1) & 1)
		SWAP(elem[0], elem[1]);

	if (_vm->_rnd->getRandomNumber(1) & 1)
		SWAP(elem[2], elem[3]);
}

// Original name: melangedir
void EdenGame::scrambleDirections() {
	scramble1(tab_2CB1E[0]);
	scramble1(tab_2CB1E[1]);
	scramble1(tab_2CB1E[2]);
	scramble2(tab_2CB1E[3]);
	scramble2(tab_2CB1E[4]);
	scramble1(tab_2CB1E[5]);
	scramble2(tab_2CB1E[6]);
	scramble2(tab_2CB1E[7]);
}

bool EdenGame::naitredino(char persoType) {
	for (perso_t *perso = &_persons[PER_MORKUS]; (++perso)->_roomNum != 0xFFFF;) {
		char areaNum = perso->_roomNum >> 8;
		if (areaNum != _globals->_citadelAreaNum)
			continue;
		if ((perso->_flags & PersonFlags::pf80) && (perso->_flags & PersonFlags::pfTypeMask) == persoType) {
			perso->_flags &= ~PersonFlags::pf80;
			return true;
		}
	}
	return false;
}

// Original name: newcita
void EdenGame::newCitadel(char area, int16 level, Room *room) {
	Citadel *cita = _citadelList;
	while (cita->_id < level)
		cita++;

	uint16 index = ((room->_flags & 0xC0) >> 6);    //TODO: this is very wrong
	if (area == 4 || area == 6)
		index++;

	room->_bank = cita->_bank[index];
	room->_video = cita->_video[index];
	room->_flags |= RoomFlags::rf02;
}

// Original name: citaevol
void EdenGame::evolveCitadel(int16 level) {
	Room *room = _globals->_curAreaPtr->_citadelRoomPtr;
	perso_t *perso = &_persons[PER_UNKN_372];
	byte loc = room->_location;
	if (level >= 80 && !istrice((_globals->_citadelAreaNum << 8) | loc)) {
		room->_level = 79;
		return;
	}

	if (level > 160)
		level = 160;

	if (room->_level < 64 && level >= 64 && naitredino(PersonFlags::pftTriceraptor)) {
		_globals->_curAreaPtr->_flags |= AreaFlags::HasTriceraptors;
		addInfo(_globals->_citadelAreaNum + ValleyNews::vnTriceraptorsIn);
	}
	if (room->_level < 40 && level >= 40 && naitredino(PersonFlags::pftVelociraptor)) {
		_globals->_curAreaPtr->_flags |= AreaFlags::HasVelociraptors;
		addInfo(_globals->_citadelAreaNum + ValleyNews::vnVelociraptorsIn);
	}
	room->_level = level;
	newCitadel(_globals->_citadelAreaNum, level, room);
	byte speed = _dinoSpeedForCitadelLevel[room->_level >> 4];
	for (; perso->_roomNum != 0xFFFF; perso++) {
		if (perso->_flags & PersonFlags::pf80)
			continue;
		if ((perso->_roomNum >> 8) == _globals->_citadelAreaNum && perso->_targetLoc == loc)
			perso->_speed = speed;
	}
}

int16 EdenGame::getCurBankNum() {
	return _curBankNum;
}

// Original name: citacapoute
void EdenGame::destroyCitadelRoom(int16 roomNum) {
	perso_t *perso = &_persons[PER_UNKN_18C];
	Room *room = _globals->_curAreaPtr->_citadelRoomPtr;
	room->_flags |= RoomFlags::rf01;
	room->_flags &= ~RoomFlags::rfHasCitadel;
	room->_bank = 193;
	room->_video = 0;
	room->_level = 0;
	_globals->_curAreaPtr->_citadelLevel = 0;
	_globals->_curAreaPtr->_citadelRoomPtr = 0;
	roomNum = (roomNum & ~0xFF) | room->_location;
	for (; perso->_roomNum != 0xFFFF; perso++) {
		if (perso->_roomNum == roomNum) {
			perso->_flags &= ~PersonFlags::pf80;
			removeInfo((roomNum >> 8) + ValleyNews::vnTyrannIn);
			break;
		}
	}
}

// Original name: buildcita
void EdenGame::narratorBuildCitadel() {
	Area *area = _globals->_areaPtr;
	_globals->_curAreaPtr = _globals->_areaPtr;
	if (area->_citadelRoomPtr)
		destroyCitadelRoom(_globals->_roomNum);
	_globals->_var6A = _globals->_var69;
	_globals->_narratorSequence = _globals->_var69 | 0x80;
	area->_citadelRoomPtr = _globals->_roomPtr;
	_globals->_roomPtr->_flags &= ~RoomFlags::rf01;
	_globals->_roomPtr->_flags |= RoomFlags::rfHasCitadel;
	_globals->_roomPtr->_level = 32;
	newCitadel(_globals->_areaNum, 32, _globals->_roomPtr);
	area->_flags &= ~AreaFlags::TyrannSighted;
	if (!(area->_flags & AreaFlags::afFlag8000)) {
		if (_globals->_phaseNum == 304 || _globals->_phaseNum != 384) //TODO: wha
			handleEloiReturn();
		area->_flags |= AreaFlags::afFlag8000;
	}
	_globals->_roomCharacterPtr->_flags |= PersonFlags::pf80;
	_globals->_citadelAreaNum = _globals->_areaNum;
	naitredino(1);
	removeInfo(_globals->_areaNum + ValleyNews::vnCitadelLost);
	removeInfo(_globals->_areaNum + ValleyNews::vnTyrannLost);
	if (_globals->_phaseNum == 193 && _globals->_areaNum == Areas::arUluru)
		bigphase1();
}

// Original name: citatombe
void EdenGame::citadelFalls(char level) {
	if (level)
		newCitadel(_globals->_citadelAreaNum, level, _globals->_curAreaPtr->_citadelRoomPtr);
	else {
		destroyCitadelRoom(_globals->_citadelAreaNum << 8);
		addInfo(_globals->_citadelAreaNum + ValleyNews::vnCitadelLost);
	}
}

// Original name: constcita
void EdenGame::buildCitadel() {
	if (!_globals->_curAreaPtr->_citadelLevel || !_globals->_curAreaPtr->_citadelRoomPtr)
		return;

	Room *room = _globals->_curAreaPtr->_citadelRoomPtr;
	byte loc = room->_location;
	_tyranPtr = &_persons[PER_UNKN_372];
	if (istyran((_globals->_citadelAreaNum << 8) | loc)) {
		if (!(_globals->_curAreaPtr->_flags & AreaFlags::TyrannSighted)) {
			addInfo(_globals->_citadelAreaNum + ValleyNews::vnTyrannIn);
			_globals->_curAreaPtr->_flags |= AreaFlags::TyrannSighted;
		}
		byte level = room->_level - 1;
		if (level < 32)
			level = 32;
		room->_level = level;
		citadelFalls(level);
	} else {
		_globals->_curAreaPtr->_flags &= ~AreaFlags::TyrannSighted;
		evolveCitadel(room->_level + 1);
	}
}

// Original name: depladino
void EdenGame::moveDino(perso_t *perso) {
	int dir = getDirection(perso);
	if (dir != -1) {
		scrambleDirections();
		uint8 *dirs = tab_2CB1E[dir];
		byte loc = perso->_roomNum & 0xFF;
		uint8 dir2 = dirs[0];
		if (dir2 & 0x80)
			dir2 = -(dir2 & ~0x80);
		dir2 += loc;
		if (!canMoveThere(dir2, perso)) {
			dir2 = dirs[1];
			if (dir2 & 0x80)
				dir2 = -(dir2 & ~0x80);
			dir2 += loc;
			if (!canMoveThere(dir2, perso)) {
				dir2 = dirs[2];
				if (dir2 & 0x80)
					dir2 = -(dir2 & ~0x80);
				dir2 += loc;
				if (!canMoveThere(dir2, perso)) {
					dir2 = dirs[3];
					if (dir2 & 0x80)
						dir2 = -(dir2 & ~0x80);
					dir2 += loc;
					if (!canMoveThere(dir2, perso)) {
						dir2 = perso->_lastLoc;
						perso->_lastLoc = 0;
						if (!canMoveThere(dir2, perso))
							return;
					}
				}
			}
		}

		perso->_lastLoc = perso->_roomNum & 0xFF;
		perso->_roomNum &= ~0xFF;
		perso->_roomNum |= dir2 & 0xFF;
		if ((perso->_targetLoc - 16 == (perso->_roomNum & 0xFF))
			|| (perso->_targetLoc + 16 == (perso->_roomNum & 0xFF))
			|| (perso->_targetLoc - 1 == (perso->_roomNum & 0xFF))
			|| (perso->_targetLoc + 1 == (perso->_roomNum & 0xFF)))
			perso->_targetLoc = 0;
	} else
		perso->_targetLoc = 0;
}

// Original name: deplaalldino
void EdenGame::moveAllDino() {
	for (perso_t *perso = &_persons[PER_UNKN_18C]; perso->_roomNum != 0xFFFF; perso++) {
		if (((perso->_roomNum >> 8) & 0xFF) != _globals->_citadelAreaNum)
			continue;
		if ((perso->_flags & PersonFlags::pf80) || !perso->_targetLoc)
			continue;
		if (--perso->_steps)
			continue;
		perso->_steps = 1;
		if (perso->_roomNum == _globals->_roomNum)
			continue;
		perso->_steps = perso->_speed;
		moveDino(perso);
	}
}

// Original name: newvallee
void EdenGame::newValley() {
	static int16 roomNumList[] = { 2075, 2080, 2119, -1};

	perso_t *perso = &_persons[PER_UNKN_372];
	int16 *ptr = roomNumList;
	int16 roomNum = *ptr++;
	while (roomNum != -1) {
		perso->_roomNum = roomNum;
		perso->_flags &= ~PersonFlags::pf80;
		perso->_flags &= ~PersonFlags::pf20;
		perso++;
		roomNum = *ptr++;
	}
	perso->_roomNum = 0xFFFF;
	_areasTable[7]._flags |= AreaFlags::HasTyrann;
	_globals->_worldHasTyran = 32;
}

char EdenGame::whereIsCita() {
	char res = (char)-1;
	for (Room *room = _globals->_citaAreaFirstRoom; room->_id != 0xFF; room++) {
		if (!(room->_flags & RoomFlags::rfHasCitadel))
			continue;
		res = room->_location;
		break;
	}
	return res;
}

bool EdenGame::isCita(int16 loc) {
	loc &= 0xFF;
	for (Room *room = _globals->_citaAreaFirstRoom; room->_id != 0xFF; room++) {
		if (!(room->_flags & RoomFlags::rfHasCitadel))
			continue;

		if ((room->_location == loc + 16)
			|| (room->_location == loc - 16)
			|| (room->_location == loc - 1)
			|| (room->_location == loc + 1))
			return true;
	}
	return false;
}

// Original name: lieuvava
void EdenGame::placeVava(Area *area) {
	if (area->_type == AreaType::atValley) {
		istyranval(area);
		area->_citadelLevel = 0;
		if (area->_citadelRoomPtr)
			area->_citadelLevel = _globals->_citaAreaFirstRoom->_level;  //TODO: no search?
		byte mask = ~(1 << (area->_num - Areas::arChamaar));
		_globals->_worldTyranSighted &= mask;
		_globals->_var4E &= mask;
		_globals->_worldGaveGold &= mask;
		_globals->_worldHasVelociraptors &= mask;
		_globals->_worldHasTriceraptors &= mask;
		_globals->_worldHasTyran &= mask;
		_globals->_var53 &= mask;
		mask = ~mask;
		if (area->_flags & AreaFlags::TyrannSighted)
			_globals->_worldTyranSighted |= mask;
		if (area->_flags & AreaFlags::afFlag4)
			_globals->_var4E |= mask;
		if (area->_flags & AreaFlags::HasTriceraptors)
			_globals->_worldHasTriceraptors |= mask;
		if (area->_flags & AreaFlags::afGaveGold)
			_globals->_worldGaveGold |= mask;
		if (area->_flags & AreaFlags::HasVelociraptors)
			_globals->_worldHasVelociraptors |= mask;
		if (area->_flags & AreaFlags::HasTyrann)
			_globals->_worldHasTyran |= mask;
		if (area->_flags & AreaFlags::afFlag20)
			_globals->_var53 |= mask;
		if (area == _globals->_areaPtr) {
			_globals->_curAreaFlags = area->_flags;
			_globals->_curCitadelLevel = area->_citadelLevel;
		}
	}
	_globals->_var4D &= _globals->_worldTyranSighted;
}

void EdenGame::vivredino() {
	for (perso_t *perso = &_persons[PER_UNKN_18C]; perso->_roomNum != 0xFFFF; perso++) {
		if (((perso->_roomNum >> 8) & 0xFF) != _globals->_citadelAreaNum)
			continue;
		if (perso->_flags & PersonFlags::pf80)
			continue;
		switch (perso->_flags & PersonFlags::pfTypeMask) {
		case PersonFlags::pftTyrann:
			if (isCita(perso->_roomNum))
				perso->_targetLoc = 0;
			else if (!perso->_targetLoc) {
				char cita = whereIsCita();
				if (cita != (char)-1) {
					perso->_targetLoc = cita;
					perso->_speed = 2;
					perso->_steps = 1;
				}
			}
			break;
		case PersonFlags::pftTriceraptor:
			if (perso->_flags & PersonFlags::pfInParty) {
				if (isCita(perso->_roomNum))
					perso->_targetLoc = 0;
				else if (!perso->_targetLoc) {
					char cita = whereIsCita();
					if (cita != (char)-1) {
						perso->_targetLoc = cita;
						perso->_speed = 3;
						perso->_steps = 1;
					}
				}
			}
			break;
		case PersonFlags::pftVelociraptor:
			if (perso->_flags & PersonFlags::pf10) {
				if (perso->_roomNum == _globals->_roomNum) {
					perso_t *perso2 = &_persons[PER_UNKN_372];
					bool found = false;
					for (; perso2->_roomNum != 0xFFFF; perso2++) {
						if ((perso->_roomNum & ~0xFF) == (perso2->_roomNum & ~0xFF)) {
							if (perso2->_flags & PersonFlags::pf80)
								continue;
							perso->_targetLoc = perso2->_roomNum & 0xFF;
							perso->_steps = 1;
							found = true;
							break;
						}
					}
					if (found)
						continue;
				} else {
					_tyranPtr = &_persons[PER_UNKN_372];
					if (istyran(perso->_roomNum)) {
						if (_globals->_phaseNum < 481 && (perso->_powers & (1 << (_globals->_citadelAreaNum - 3)))) {
							_tyranPtr->_flags |= PersonFlags::pf80;
							_tyranPtr->_roomNum = 0;
							perso->_flags &= ~PersonFlags::pf10;
							perso->_flags |= PersonFlags::pfInParty;
							addInfo(_globals->_citadelAreaNum + ValleyNews::vnTyrannLost);
							removeInfo(_globals->_citadelAreaNum + ValleyNews::vnTyrannIn);
							if (naitredino(PersonFlags::pftTriceraptor))
								addInfo(_globals->_citadelAreaNum + ValleyNews::vnTriceraptorsIn);
							buildCitadel();
							_globals->_curAreaPtr->_flags &= ~AreaFlags::TyrannSighted;
						} else {
							perso->_flags &= ~PersonFlags::pf10;
							perso->_flags &= ~PersonFlags::pfInParty;
							addInfo(_globals->_citadelAreaNum + ValleyNews::vnVelociraptorsLost);
						}
						continue;
					}
				}
			}
			if (!perso->_targetLoc) {
				int16 loc;
				perso->_lastLoc = 0;
				do {
					loc = (_vm->_rnd->getRandomNumber(63) & 63) + 16;
					if ((loc & 0xF) >= 12)
						loc &= ~4;  //TODO: ??? same as -= 4
				} while (!canMoveThere(loc, perso));
				perso->_targetLoc = loc;
				perso->_steps = 1;
			}
			break;
		}
	}
}

void EdenGame::vivreval(int16 areaNum) {
	_globals->_citadelAreaNum = areaNum;
	_globals->_curAreaPtr = &_areasTable[areaNum - 1];
	_globals->_citaAreaFirstRoom = &_gameRooms[_globals->_curAreaPtr->_firstRoomIdx];
	moveAllDino();
	buildCitadel();
	vivredino();
	newMushroom();
	newNestWithEggs();
	newEmptyNest();
	if (_globals->_phaseNum >= 226)
		newGold();
	placeVava(_globals->_curAreaPtr);
}

// Original name: chaquejour
void EdenGame::handleDay() {
	vivreval(3);
	vivreval(4);
	vivreval(5);
	vivreval(6);
	vivreval(7);
	vivreval(8);
	_globals->_drawFlags |= DrawFlags::drDrawTopScreen;
}

// Original name: temps_passe
void EdenGame::addTime(int16 t) {
	int16 days = _globals->_gameDays;
	int16 lo = _globals->_gameHours + t;
	if (lo > 255) {
		days++;
		lo &= 0xFF;
	}

	_globals->_gameHours = lo;
	t = ((t >> 8) & 0xFF) + days;
	t -= _globals->_gameDays;
	if (t) {
		_globals->_gameDays += t;
		while (t--)
			handleDay();
	}
}

byte *EdenGame::getImageDesc() {
	return _imageDesc;
}

// Original name: anim_perso
void EdenGame::animCharacter() {
	if (_curBankNum != _globals->_characterImageBank)
		loadCharacter(_globals->_characterPtr);
	_graphics->restoreUnderSubtitles();
	if (_restartAnimation) {
		_lastAnimTicks = _vm->_timerTicks;
		_restartAnimation = false;
	}
	_curAnimFrameNumb = (_vm->_timerTicks - _lastAnimTicks) >> 2;   // TODO: check me!!!
	if (_curAnimFrameNumb > _numAnimFrames)               // TODO: bug?
		_animateTalking = false;
	if (_globals->_curCharacterAnimPtr && !_globals->_animationFlags && _curAnimFrameNumb != _lastAnimFrameNumb) {
		_lastAnimFrameNumb = _curAnimFrameNumb;
		if (*_globals->_curCharacterAnimPtr == 0xFF)
			getanimrnd();
		useCharacterBank();
		_numImgDesc = 0;
		setCharacterSprite(_globals->_curCharacterAnimPtr);
		_globals->_curCharacterAnimPtr += _numImgDesc + 1;
		_mouthAnimations = _imageDesc + 200;
		removeMouthSprite();
		if (*_mouthAnimations)
			_graphics->displayImage();
		_animationDelay--;
		if (!_animationDelay) {
			_globals->_animationFlags = 1;
			_animationDelay = 8;
		}
	}

	_animationDelay--;
	if (!_animationDelay) {
		getanimrnd();
		//TODO: no reload?
	}
	if (_animateTalking) {
		if (!_animationTable) {
			_animationTable = _gameLipsync + 7262;    //TODO: fix me
			if (!_backgroundSaved) {
				_graphics->saveMouthBackground();
				_backgroundSaved = true;
			}
		}
		if (!_personTalking)
			_curAnimFrameNumb = _numAnimFrames - 1;
		_animationIndex = _animationTable[_curAnimFrameNumb];
		if (_animationIndex == 0xFF)
			_animateTalking = false;
		else if (_animationIndex != _lastAnimationIndex) {
			useCharacterBank();
			_graphics->restoreMouthBackground();
//			debug("perso spr %d", animationIndex);
			setCharacterSprite(_globals->_persoSpritePtr2 + _animationIndex * 2);  //TODO: int16s?
			_mouthAnimations = _imageDesc + 200;
			if (*_mouthAnimations)
				_graphics->displayImage();
			_lastAnimationIndex = _animationIndex;
		}
	}
	_graphics->displaySubtitles();
}

void EdenGame::getanimrnd() {
	_animationDelay = 8;
	int16 rnd = _vm->_rnd->getRandomNumber(65535) & (byte)~0x18;    //TODO
	dword_30724 = _globals->_persoSpritePtr + 16;    //TODO
	_globals->_curCharacterAnimPtr = _globals->_persoSpritePtr + ((dword_30724[1] << 8) + dword_30724[0]);
	_globals->_animationFlags = 1;
	if (rnd >= 8)
		return;
	_globals->_animationFlags = 0;
	if (rnd <= 0)
		return;
	for (rnd *= 8; rnd > 0; rnd--) {
		while (*_globals->_curCharacterAnimPtr)
			_globals->_curCharacterAnimPtr++;
		_globals->_curCharacterAnimPtr++;
	}
}

void EdenGame::addanim() {
	_lastAnimationIndex = 0xFF;
	_lastAnimTicks = 0;
	_globals->_animationFlags = 0xC0;
	_globals->_curCharacterAnimPtr = _globals->_persoSpritePtr;
	getanimrnd();
	_animationActive = true;
	if (_globals->_characterPtr == &_persons[PER_KING])
		return;
	setCharacterSprite(_globals->_persoSpritePtr + READ_LE_UINT16(_globals->_persoSpritePtr));  //TODO: GetElem(0)
	_mouthAnimations = _imageDesc + 200;
	if (_globals->_characterPtr->_id != PersonId::pidCabukaOfCantura && _globals->_characterPtr->_targetLoc != 7) //TODO: targetLoc is minisprite idx
		removeMouthSprite();
	if (*_mouthAnimations)
		_graphics->displayImage();
}

// Original name: virespritebouche
void EdenGame::removeMouthSprite() {
	byte *src = _mouthAnimations + 2;
	byte *dst = src;
	char cnt = _mouthAnimations[0];
	while (cnt--) {
		byte a = *src++;
		byte b = *src++;
		byte c = *src++;
		dst[0] = a;
		dst[1] = b;
		dst[2] = c;
		if (dword_30728[0] != 0xFF) {
			if ((a < dword_30728[0] || a > dword_30728[1])
			        && (a < dword_30728[2] || a > dword_30728[3]))
				dst += 3;
			else
				_mouthAnimations[0]--;
		} else
			dst += 3;
	}
}

// Original name: anim_perfin
void EdenGame::AnimEndCharacter() {
	_globals->_animationFlags &= ~0x80;
	_animationDelay = 0;
	_animationActive = false;
}

// Original name: perso_spr
void EdenGame::setCharacterSprite(byte *spr) {
	byte *img = _imageDesc + 200 + 2;
	int16 count = 0;
	byte c;
	while ((c = *spr++)) {
		byte *src;
		int16 index = 0;
		byte cc = 0;
		if (c == 1) {
			cc = index;
			c = *spr++;
		}
		_numImgDesc++;
		index = (cc << 8) | c;
		index -= 2;

		if (index > _maxPersoDesc)
			index = _maxPersoDesc;
		index *= 2;         //TODO: src = GetElem(ff_C2, index)
		src = _globals->_varC2;
		src += READ_LE_UINT16(src + index);
		while ((c = *src++)) {
			*img++ = c;
			*img++ = *src++;
			*img++ = *src++;
			count++;
		}
	}
	_imageDesc[200] = count & 0xFF;
	_imageDesc[201] = count >> 8;
}

// Original name: af_perso1
void EdenGame::displayCharacter1() {
	setCharacterSprite(_globals->_persoSpritePtr + READ_LE_UINT16(_globals->_persoSpritePtr));
	_graphics->displayImage();
}

// Original name: af_perso
void EdenGame::displayCharacter() {
	loadCurrCharacter();
	displayCharacter1();
}

void EdenGame::ef_perso() {
	_globals->_animationFlags &= 0x3F;
}

// Original name: load_perso
void EdenGame::loadCharacter(perso_t *perso) {
	_characterBankData = nullptr;
	if (!perso->_spriteBank)
		return;

	if (perso->_spriteBank != _globals->_characterImageBank) {
		_graphics->setCurCharRect(&_characterRects[perso->_id]); //TODO: array of int16?
		dword_30728 = _characterArray[perso->_id];
		ef_perso();
		_globals->_characterImageBank = perso->_spriteBank;
		useBank(_globals->_characterImageBank);
		_characterBankData = _bankData;
		byte *ptr = _bankData;
		ptr += READ_LE_UINT16(ptr);
		byte *baseptr = ptr;
		ptr += READ_LE_UINT16(ptr) - 2;
		ptr = baseptr + READ_LE_UINT16(ptr) + 4;
		_gameIcons[0].sx = READ_LE_UINT16(ptr);
		_gameIcons[0].sy = READ_LE_UINT16(ptr + 2);
		_gameIcons[0].ex = READ_LE_UINT16(ptr + 4);
		_gameIcons[0].ey = READ_LE_UINT16(ptr + 6);
		ptr += 8;
		_globals->_varC2 = ptr + 2;
		_maxPersoDesc = READ_LE_UINT16(ptr) / 2;
		ptr += READ_LE_UINT16(ptr);
		baseptr = ptr;
		ptr += READ_LE_UINT16(ptr) - 2;
		_globals->_persoSpritePtr = baseptr;
		_globals->_persoSpritePtr2 = baseptr + READ_LE_UINT16(ptr);
		debug("load perso: b6 len is %d", (int)(_globals->_persoSpritePtr2 - _globals->_persoSpritePtr));
	} else {
		useBank(_globals->_characterImageBank);
		_characterBankData = _bankData;
	}
}

// Original name: load_perso_cour
void EdenGame::loadCurrCharacter() {
	loadCharacter(_globals->_characterPtr);
}

void EdenGame::fin_perso() {
	_globals->_animationFlags &= 0x3F;
	_globals->_curCharacterAnimPtr = nullptr;
	_globals->_varCA = 0;
	_globals->_characterImageBank = -1;
	AnimEndCharacter();
}

void EdenGame::no_perso() {
	if (_globals->_displayFlags == DisplayFlags::dfPerson) {
		_globals->_displayFlags = _globals->_oldDisplayFlags;
		fin_perso();
	}
	endCharacterSpeech();
}

// Original name: close_perso
void EdenGame::closeCharacterScreen() {
	endCharacterSpeech();
	if (_globals->_displayFlags == DisplayFlags::dfPerson && _globals->_characterPtr->_id != PersonId::pidNarrator && _globals->_eventType != EventType::etEventE) {
		_graphics->rundcurs();
		_graphics->setSavedUnderSubtitles(true);
		_graphics->restoreUnderSubtitles();
		display();
		_globals->_var103 = 16;
	}
	if (_globals->_characterPtr->_id == PersonId::pidNarrator)
		_globals->_var103 = 69;
	_globals->_eloiHaveNews &= 1;
	_globals->_varCA = 0;
	_globals->_varF6 = 0;
	if (_globals->_displayFlags == DisplayFlags::dfPerson) {
		_globals->_displayFlags = _globals->_oldDisplayFlags;
		_globals->_animationFlags &= 0x3F;
		_globals->_curCharacterAnimPtr = nullptr;
		AnimEndCharacter();
		if (_globals->_displayFlags & DisplayFlags::dfMirror) {
			gameToMirror(1);
			_scrollPos = _oldScrollPos;
			scroll();
			return;
		}
		if (_globals->_numGiveObjs) {
			if (!(_globals->_displayFlags & DisplayFlags::dfFlag2))
				showObjects();
			_globals->_numGiveObjs = 0;
		}
		if (_globals->_varF2 & 1) {
			_globals->_mirrorEffect = 6; // CHECKME: Verify the value
			_globals->_varF2 &= ~1;
		}
		char oldLoc = _globals->_newLocation;
		_globals->_newLocation = 0;
		if (!(_globals->_narratorSequence & 0x80))
			_globals->_var100 = 0xFF;
		updateRoom(_globals->_roomNum);
		_globals->_newLocation = oldLoc;
	}

	if (_globals->_chrono)
		_globals->_chronoFlag = 1;
}

// Original name: af_fondsuiveur
void EdenGame::displayBackgroundFollower() {
	char id = _globals->_characterPtr->_id;
	for (Follower *follower = _followerList; follower->_id != -1; follower++) {
		if (follower->_id == id) {
			int bank = 326;
			if (follower->sx >= 320)
				bank = 327;
			useBank(bank + _globals->_roomBackgroundBankNum);
			_graphics->drawSprite(0, 0, 16, true);
			break;
		}
	}
}

void EdenGame::displayNoFollower(int16 bank) {
	if (bank) {
		useBank(bank);
		if (_globals->_characterPtr == &_persons[PER_UNKN_156])
			_graphics->drawSprite(0, 0, 16, true);
		else
			_graphics->drawSprite(0, 0, 16);
	}
}

int16 EdenGame::getGameIconX(int16 index) {
	return _gameIcons[index].sx;
}

int16 EdenGame::getGameIconY(int16 index) {
	return _gameIcons[index].sy;
}

// Original name: af_fondperso1
void EdenGame::displayCharacterBackground1() {
	byte bank;
	char *ptab;
	if (_globals->_characterPtr == &_persons[PER_ELOI]) {
		_gameIcons[0].sx = 0;
		_characterRects[PER_ELOI].left = 2;
		bank = _globals->_characterBackgroundBankIdx;
		if (_globals->_eventType == EventType::etEventE) {
			_globals->_var103 = 1;
			displayNoFollower(bank);
			return;
		}
		_gameIcons[0].sx = 60;
		_characterRects[PER_ELOI].left = 62;
	}
	if (_globals->_characterPtr == &_persons[PER_TAU]) {
		bank = 37;
		if (_globals->_curObjectId == Objects::obShell) {
			displayNoFollower(bank);
			return;
		}
	}
	ptab = _personRoomBankTable + _globals->_characterPtr->_roomBankId;
	bank = *ptab++;
	if (!(_globals->_characterPtr->_partyMask & _globals->_party)) {
		while ((bank = *ptab++) != 0xFF) {
			if (bank == (_globals->_roomNum & 0xFF)) { //TODO: signed vs unsigned - chg bank to uns?
				bank = *ptab;
				break;
			}
			ptab++;
		}
		if (bank == 0xFF) {
			ptab = _personRoomBankTable + _globals->_characterPtr->_roomBankId;
			bank = *ptab++;
		}
	}
	displayBackgroundFollower();
	displayNoFollower(bank);
}

// Original name: af_fondperso
void EdenGame::displayCharacterBackground() {
	if (_globals->_characterPtr->_spriteBank) {
		_backgroundSaved = false;
		displayCharacterBackground1();
	}
}

// Original name: setpersoicon
void EdenGame::setCharacterIcon() {
	if (_globals->_iconsIndex == 4)
		return;

	if (_globals->_characterPtr == &_persons[PER_ELOI] && _globals->_eventType == EventType::etEventE) {
		_globals->_iconsIndex = 123;
		return;
	}
	Icon *icon = _gameIcons;
	Icon *icon2 = &_gameIcons[_roomIconsBase];

	*icon2++ = *icon++; //TODO: is this ok?
	*icon2++ = *icon++;
	icon2->sx = -1;
}

// Original name: show_perso
void EdenGame::showCharacter() {
	perso_t *perso = _globals->_characterPtr;
	if (perso->_spriteBank) {
		closeRoom();
		if (_globals->_displayFlags != DisplayFlags::dfPerson) {
			if (_globals->_displayFlags & DisplayFlags::dfMirror)
				resetScroll();
			_globals->_oldDisplayFlags = _globals->_displayFlags;
			_globals->_displayFlags = DisplayFlags::dfPerson;
			loadCharacter(perso);
			setCharacterIcon();
			displayCharacterBackground();
			if (perso == &_persons[PER_TAU] && _globals->_curObjectId == Objects::obShell) {
				_graphics->displaySubtitles();
				updateCursor();
				_paletteUpdateRequired = true;
				display();
				_graphics->rundcurs();
				return;
			}
		}
		loadCurrCharacter();
		addanim();
		if (!_globals->_curCharacterAnimPtr) {
			displayCharacter();
			_graphics->displaySubtitles();
		}
		_restartAnimation = true;
		animCharacter();
		if (perso != &_persons[PER_UNKN_156])
			updateCursor();
		_paletteUpdateRequired = true;
		if (perso != &_persons[PER_UNKN_156])
			_graphics->rundcurs();
		display();
	} else {
		displayPlace();
		_graphics->displaySubtitles();
	}
}

// Original name: showpersopanel
void EdenGame::displayCharacterPanel() {
	perso_t *perso = _globals->_characterPtr;
	loadCurrCharacter();
	addanim();
	if (!_globals->_curCharacterAnimPtr) {
		displayCharacter();
		_graphics->displaySubtitles();
	}
	_restartAnimation = true;
	_paletteUpdateRequired = true;
	if (_globals->_drawFlags & DrawFlags::drDrawFlag8)
		return;
	animCharacter();
	if (perso != &_persons[PER_UNKN_156])
		updateCursor();
	display();
	if (perso != &_persons[PER_UNKN_156])
		_graphics->rundcurs();
	_globals->_drawFlags |= DrawFlags::drDrawFlag8;
	_globals->_iconsIndex = 112;
}

void EdenGame::getDataSync() {
	int16 num = _globals->_textNum;
	if (_globals->_textBankIndex != 1)
		num += 565;
	if (_globals->_textBankIndex == 3)
		num += 707;
	if (num == 144)
		num = 142;
	_animateTalking = ReadDataSync(num - 1);
	if (_animateTalking)
		_numAnimFrames = readFrameNumber();
	else
		_numAnimFrames = 0;
	if (_globals->_textNum == 144)
		_numAnimFrames = 48;
	_animationTable = 0;
}

// Original name: ReadNombreFrames
int16 EdenGame::readFrameNumber() {
	int16 num = 0;
	_animationTable = _gameLipsync + 7260 + 2;    //TODO: fix me
	while (*_animationTable++ != 0xFF)
		num++;
	return num;
}

void EdenGame::waitEndSpeak() {
	for (;;) {
		if (_animationActive)
			animCharacter();
		musicspy();
		display();
		_vm->pollEvents();
		if (_vm->shouldQuit()) {
			closeCharacterScreen();
			edenShudown();
			break;
		}
		if (!_mouseHeld)
			if (_vm->isMouseButtonDown())
				break;
		if (_mouseHeld)
			if (!_vm->isMouseButtonDown())
				_mouseHeld = false;
	}
	_mouseHeld = true;
}

int16 EdenGame::getNumTextLines() {
	return _numTextLines;
}

void EdenGame::my_bulle() {
	if (!_globals->_textNum)
		return;

	byte *icons = phraseIconsBuffer;
	byte *linesp = _sentenceCoordsBuffer;
	byte *sentencePtr = _sentenceBuffer;
	_globals->_numGiveObjs = 0;
	_globals->_giveObj1 = 0;
	_globals->_giveObj2 = 0;
	_globals->_giveObj3 = 0;
	_globals->_textWidthLimit = _subtitlesXWidth;
	byte *textPtr = getPhrase(_globals->_textNum);
	_numTextLines = 0;
	int16 wordsOnLine = 0;
	int16 wordWidth = 0;
	int16 lineWidth = 0;
	byte c;
	while ((c = *textPtr++) != 0xFF) {
		if (c == 0x11 || c == 0x13) {
			if (_globals->_phaseNum <= 272 || _globals->_phaseNum == 386) {
				_globals->_eloiHaveNews = c & 0xF;
				_globals->_var4D = _globals->_worldTyranSighted;
			}
		} else if (c >= 0x80 && c < 0x90)
			SysBeep(1);
		else if (c >= 0x90 && c < 0xA0) {
			while (*textPtr++ != 0xFF) {}
			textPtr--;
		} else if (c >= 0xA0 && c < 0xC0)
			_globals->_textToken1 = c & 0xF;
		else if (c >= 0xC0 && c < 0xD0)
			_globals->_textToken2 = c & 0xF;
		else if (c >= 0xD0 && c < 0xE0) {
			byte c1 = *textPtr++;
			if (c == 0xD2)
#ifdef FAKE_DOS_VERSION
				_globals->_textWidthLimit = c1 + 160;
#else
				_globals->_textWidthLimit = c1 + _subtitlesXCenter; // TODO: signed? 160 in pc ver
#endif
			else {
				byte c2 = *textPtr++;
				switch (_globals->_numGiveObjs) {
				case 0:
					_globals->_giveObj1 = c2;
					break;
				case 1:
					_globals->_giveObj2 = c2;
					break;
				case 2:
					_globals->_giveObj3 = c2;
					break;
				}
				_globals->_numGiveObjs++;
				*icons++ = *textPtr++;
				*icons++ = *textPtr++;
				*icons++ = c2;
			}
		} else if (c >= 0xE0 && c < 0xFF)
			SysBeep(1);
		else if (c != '\r') {
			*sentencePtr++ = c;
			byte width = _gameFont[c];
#ifdef FAKE_DOS_VERSION
			if (c == ' ')
				width = _spaceWidth;
#endif
			wordWidth += width;
			lineWidth += width;
			int16 overrun = lineWidth - _globals->_textWidthLimit;
			if (overrun > 0) {
				_numTextLines++;
				if (c != ' ') {
					*linesp++ = wordsOnLine;
					*linesp++ = wordWidth + _spaceWidth - overrun;
					lineWidth = wordWidth;
				} else {
					*linesp++ = wordsOnLine + 1;
					*linesp++ = _spaceWidth - overrun;   //TODO: checkme
					lineWidth = 0;
				}
				wordWidth = 0;
				wordsOnLine = 0;
			} else {
				if (c == ' ') {
					wordsOnLine++;
					wordWidth = 0;
				}
			}
		}
	}
	_numTextLines++;
	*linesp++ = wordsOnLine + 1;
	*linesp++ = wordWidth;
	*sentencePtr = c;
	if (_globals->_textBankIndex == 2 && _globals->_textNum == 101 && _globals->_prefLanguage == 1)
		patchSentence();
	my_pr_bulle();
	if (!_globals->_numGiveObjs)
		return;
	useMainBank();
	if (_numTextLines < 3)
		_numTextLines = 3;
	icons = phraseIconsBuffer;
	for (byte i = 0; i < _globals->_numGiveObjs; i++) {
		byte x = *icons++;
		byte y = *icons++;
		byte s = *icons++;
		_graphics->drawSprite(52, x + _subtitlesXCenter, y - 1, false, true);
		_graphics->drawSprite(s + 9, x + _subtitlesXCenter + 1, y, false, true);
	}
}

int16 EdenGame::getScrollPos() {
	return _scrollPos;
}

bool EdenGame::getSpecialTextMode() {
	return _specialTextMode;
}

void EdenGame::setSpecialTextMode(bool value) {
	_specialTextMode = value;
}

void EdenGame::my_pr_bulle() {
	CLBlitter_FillView(_graphics->getSubtitlesView(), 0);
	if (_globals->_prefLanguage == 0)
		return;

	byte *coo = _sentenceCoordsBuffer;
	bool done = false;
	textout = _graphics->getSubtitlesViewBuf();
	byte *textPtr = _sentenceBuffer;
	int16 lines = 1;
	while (!done) {
		int16 numWords = *coo++;       // num words on line
		int16 padSize = *coo++;        // amount of extra spacing
		byte *currOut = textout;
		int16 extraSpacing = numWords > 1 ? padSize / (numWords - 1) + 1 : 0;
		if (lines == _numTextLines)
			extraSpacing = 0;
		byte c = *textPtr++;
		while (!done && (numWords > 0)) {
			if (c < 0x80 && c != '\r') {
				if (c == ' ') {
					numWords--;
					if (padSize >= extraSpacing) {
						textout += extraSpacing + _spaceWidth;
						padSize -= extraSpacing;
					} else {
						textout += padSize + _spaceWidth;
						padSize = 0;
					}
				} else {
					int16 charWidth = _gameFont[c];
					if (!(_globals->_drawFlags & DrawFlags::drDrawMenu)) {
						textout += _subtitlesXWidth;
						if (!_specialTextMode)
							drawSubtitleChar(c, 195, charWidth);
						textout++;
						if (!_specialTextMode)
							drawSubtitleChar(c, 195, charWidth);
						textout -= _subtitlesXWidth + 1;
					}
					if (_specialTextMode)
						drawSubtitleChar(c, 250, charWidth);
					else
						drawSubtitleChar(c, 230, charWidth);
					textout += charWidth;
				}
			} else
				error("my_pr_bulle: Unexpected format");

			c = *textPtr++;
			if (c == 0xFF)
				done = true;
		}
		textout = currOut + _subtitlesXWidth * FONT_HEIGHT;
		lines++;
		textPtr--;
	}
}

// Original name: charsurbulle
void EdenGame::drawSubtitleChar(byte c, byte color, int16 width) {
	byte *glyph = _gameFont + 256 + c * FONT_HEIGHT;
	_textOutPtr = textout;
	for (int16 h = 0; h < FONT_HEIGHT; h++) {
		byte bits = *glyph++;
		int16 mask = 0x80;
		for (int16 w = 0; w < width; w++) {
			if (bits & mask)
				*_textOutPtr = color;
			_textOutPtr++;
			mask >>= 1;
		}
		_textOutPtr += _subtitlesXWidth - width;
	}
}

// Original name: patchPhrase
void EdenGame::patchSentence() {
	_sentenceBuffer[36] = 'c';
}

void EdenGame::vavapers() {
	perso_t *perso = _globals->_characterPtr;
	_globals->_curPersoFlags = perso->_flags;
	_globals->_curPersoItems = perso->_items;
	_globals->_curCharacterPowers = perso->_powers;
}

void EdenGame::citadelle() {
	_globals->_var69++;
	_globals->_varF6++;
	parlemoiNormalFlag = true;
	_closeCharacterDialog = true;
}

// Original name: choixzone
void EdenGame::selectZone() {
	if (_globals->_giveObj3)
		_globals->_iconsIndex = 6;
	else
		_globals->_iconsIndex = 10;
	_globals->_autoDialog = false;
	putObject();
}

void EdenGame::showEvents1() {
	_globals->_var113 = 0;
	perso_ici(3);
}

void EdenGame::showEvents() {
	if (_globals->_eventType && _globals->_displayFlags != DisplayFlags::dfPerson)
		showEvents1();
}

void EdenGame::parle_mfin() {
	perso_t *perso = _globals->_characterPtr;
	if (_globals->_curObjectId) {
		char curobj = _globals->_curObjectId;
		if (_globals->_dialogType == DialogType::dtHint)
			return;
		object_t *obj = getObjectPtr(_globals->_curObjectId);
		if (_globals->_dialogType == DialogType::dtDinoItem)
			perso = _globals->_roomCharacterPtr;
		if (isAnswerYes()) {
			loseObject(_globals->_curObjectId);
			perso->_powers |= obj->_powerMask;
		}
		perso->_items |= obj->_itemMask;
		specialObjects(perso, curobj);
		return;
	}
	if (!isAnswerYes())
		return;
	nextInfo();
	if (!_globals->_lastInfo)
		_closeCharacterDialog = true;
	else {
		_globals->_nextDialogPtr = nullptr;
		_closeCharacterDialog = false;
	}
}

void EdenGame::parlemoi_normal() {
	Dialog *dial;
	if (!_globals->_nextDialogPtr) {
		perso_t *perso = _globals->_characterPtr;
		if (perso) {
			int16 num = (perso->_id << 3) | _globals->_dialogType;
			dial = (Dialog *)getElem(_gameDialogs, num);
		} else {
			closeCharacterScreen();
			return;
		}
	} else {
		if (_closeCharacterDialog) {
			closeCharacterScreen();
			return;
		}
		dial = _globals->_nextDialogPtr;
	}
	char ok = dial_scan(dial);
	_globals->_nextDialogPtr = _globals->_dialogPtr;
	_closeCharacterDialog = false;
	if (!ok)
		closeCharacterScreen();
	else
		parle_mfin();
}

void EdenGame::parle_moi() {
	endCharacterSpeech();
	byte r28 = _globals->_varF6;
	_globals->_varF6 = 0;
	if (!r28) {
		setChoiceNo();
		if (_globals->_drawFlags & DrawFlags::drDrawInventory)
			showObjects();
		if (_globals->_drawFlags & DrawFlags::drDrawTopScreen)
			drawTopScreen();
		if (_globals->_curObjectId) {
			if (_globals->_dialogType == DialogType::dtTalk) {
				_globals->_dialogType = DialogType::dtItem;
				_globals->_nextDialogPtr = nullptr;
				_closeCharacterDialog = false;
			}
			parlemoi_normal();
			return;
		}
		if (_globals->_dialogType == DialogType::dtItem) {
			_globals->_dialogType = DialogType::dtTalk;
			if (!_closeCharacterDialog)
				_globals->_nextDialogPtr = nullptr;
		}
		if (parlemoiNormalFlag) {
			parlemoi_normal();
			return;
		}
		Dialog *dial;

		if (!_globals->_lastDialogPtr) {
			int16 num = 160;
			if (_globals->_phaseNum >= 400)
				num++;
			dial = (Dialog *)getElem(_gameDialogs, num);
		} else
			dial = _globals->_lastDialogPtr;
		char ok = dial_scan(dial);
		_globals->_lastDialogPtr = _globals->_dialogPtr;
		parlemoiNormalFlag = false;
		if (!ok) {
			parlemoiNormalFlag = true;
			if (_globals->_var60) {
				if (_globals->_characterPtr == &_persons[PER_ELOI]) {
					_globals->_dialogType = DialogType::dtTalk;
					if (_globals->_eloiHaveNews)
						parlemoi_normal();
					else
						closeCharacterScreen();
				} else
					closeCharacterScreen();
			} else
				parlemoi_normal();
		} else
			parle_mfin();
	} else
		closeCharacterScreen();
}

// Original name: init_perso_ptr
void EdenGame::initCharacterPointers(perso_t *perso) {
	_globals->_metPersonsMask1 |= perso->_partyMask;
	_globals->_metPersonsMask2 |= perso->_partyMask;
	_globals->_nextDialogPtr = nullptr;
	_closeCharacterDialog = false;
	_dialogSkipFlags = DialogFlags::dfSpoken;
	_globals->_var60 = 0;
	_globals->_textToken1 = 0;
}

void EdenGame::perso1(perso_t *perso) {
	_globals->_phaseActionsCount++;
	if (perso == &_persons[PER_TAU])
		_globals->_phaseActionsCount--;
	_globals->_characterPtr = perso;
	initCharacterPointers(perso);
	parle_moi();
}

void EdenGame::perso_normal(perso_t *perso) {
	_globals->_lastDialogPtr = nullptr;
	_globals->_dialogType = DialogType::dtTalk;
	parlemoiNormalFlag = false;
	perso1(perso);
}

// Original name: persoparle
void EdenGame::handleCharacterDialog(int16 pers) {
	perso_t *perso = &_persons[pers];
	_globals->_characterPtr = perso;
	_globals->_dialogType = DialogType::dtInspect;
	uint16 idx = perso->_id * 8 | _globals->_dialogType;
	dialoscansvmas((Dialog *)getElem(_gameDialogs, idx));
	displayPlace();
	_graphics->displaySubtitles();
	persovox();
	_globals->_varCA = 0;
	_globals->_dialogType = DialogType::dtTalk;
}

// Original name: roi
void EdenGame::actionKing()  {
	perso_normal(&_persons[PER_KING]);
}

// Original name: dina
void EdenGame::actionDina() {
	perso_normal(&_persons[PER_DINA]);
}

// Original name: thoo
void EdenGame::actionThoo() {
	perso_normal(&_persons[PER_TAU]);
}

// Original name: monk
void EdenGame::actionMonk() {
	perso_normal(&_persons[PER_MONK]);
}

// Original name: bourreau
void EdenGame::actionTormentor() {
	perso_normal(&_persons[PER_JABBER]);
}

// Original name: messager
void EdenGame::actionMessenger() {
	perso_normal(&_persons[PER_ELOI]);
}

// Original name: mango
void EdenGame::actionMango()    {
	perso_normal(&_persons[PER_MUNGO]);
}

// Original name: eve
void EdenGame::actionEve()  {
	perso_normal(&_persons[PER_EVE]);
}

// Original name: azia
void EdenGame::actionAzia() {
	perso_normal(&_persons[PER_SHAZIA]);
}

// Original name: mammi
void EdenGame::actionMammi() {
	perso_t *perso;
	for (perso = &_persons[PER_MAMMI]; perso->_partyMask == PersonMask::pmLeader; perso++) {
		if (perso->_roomNum == _globals->_roomNum) {
			perso_normal(perso);
			break;
		}
	}
}

// Original name: gardes
void EdenGame::actionGuards()   {
	perso_normal(&_persons[PER_GUARDS]);
}

// Original name: bambou
void EdenGame::actionBamboo()   {
	perso_normal(&_persons[PER_BAMBOO]);
}

// Original name: kabuka
void EdenGame::actionKabuka()   {
	if (_globals->_roomNum == 0x711) perso_normal(&_persons[PER_KABUKA]);
	else actionBamboo();
}

// Original name: fisher
void EdenGame::actionFisher()   {
	if (_globals->_roomNum == 0x902) perso_normal(&_persons[PER_FISHER]);
	else actionKabuka();
}

// Original name: dino
void EdenGame::actionDino() {
	perso_t *perso = _globals->_roomCharacterPtr;
	if (!perso)
		return;
	parlemoiNormalFlag = true;
	_globals->_dialogType = DialogType::dtTalk;
	_globals->_roomCharacterFlags = perso->_flags;
	_globals->_roomPersoItems = perso->_items;
	_globals->_roomCharacterPowers = perso->_powers;
	_globals->_characterPtr = perso;
	initCharacterPointers(perso);
	debug("beg dino talk");
	parle_moi();
	debug("end dino talk");
	if (_globals->_areaNum == Areas::arWhiteArch)
		return;
	if (_globals->_var60)
		waitEndSpeak();
	if (_vm->shouldQuit())
		return;
	perso = &_persons[PER_MUNGO];
	if (!(_globals->_party & PersonMask::pmMungo)) {
		perso = &_persons[PER_DINA];
		if (!(_globals->_party & PersonMask::pmDina)) {
			perso = &_persons[PER_EVE];
			if (!(_globals->_party & PersonMask::pmEve)) {
				perso = &_persons[PER_GUARDS];
			}
		}
	}
	_globals->_dialogType = DialogType::dtDinoAction;
	if (_globals->_curObjectId)
		_globals->_dialogType = DialogType::dtDinoItem;
	perso1(perso);
	if (_globals->_roomCharacterType == PersonFlags::pftMosasaurus && !_globals->_curObjectId) {
		_globals->_areaPtr->_flags |= AreaFlags::afFlag20;
		placeVava(_globals->_areaPtr);
	}
}

// Original name: tyran
void EdenGame::actionTyran() {
	perso_t *perso = _globals->_roomCharacterPtr;
	if (!perso)
		return;

	parlemoiNormalFlag = true;
	_globals->_dialogType = DialogType::dtTalk;
	_globals->_roomCharacterFlags = perso->_flags;
	_globals->_characterPtr = perso;
	initCharacterPointers(perso);
	perso = &_persons[PER_MUNGO];
	if (!(_globals->_party & PersonMask::pmMungo)) {
		perso = &_persons[PER_DINA];
		if (!(_globals->_party & PersonMask::pmDina)) {
			perso = &_persons[PER_EVE];
			if (!(_globals->_party & PersonMask::pmEve)) {
				perso = &_persons[PER_GUARDS];
			}
		}
	}
	_globals->_dialogType = DialogType::dtDinoAction;
	if (_globals->_curObjectId)
		_globals->_dialogType = DialogType::dtDinoItem;
	perso1(perso);
}

// Original name: morkus
void EdenGame::actionMorkus()   {
	perso_normal(&_persons[PER_MORKUS]);
}

void EdenGame::comment() {
	perso_t *perso = &_persons[PER_DINA];
	if (!(_globals->_party & PersonMask::pmDina)) {
		perso = &_persons[PER_EVE];
		if (!(_globals->_party & PersonMask::pmEve)) {
			perso = &_persons[PER_GUARDS];
			if (!(_globals->_party & PersonMask::pmThugg))
				return;
		}
	}
	_globals->_dialogType = DialogType::dtHint;
	perso1(perso);
}

// Original name: adam
void EdenGame::actionAdam() {
	resetScroll();
	switch (_globals->_curObjectId) {
	case Objects::obNone:
		gotoPanel();
		break;
	case Objects::obRoot:
		if (_globals->_roomNum == 2817
		        && _globals->_phaseNum > 496 && _globals->_phaseNum < 512) {
			bigphase1();
			loseObject(Objects::obRoot);
			_globals->_var100 = 0xFF;
			quitMirror();
			updateRoom(_globals->_roomNum);
			removeFromParty(PER_ELOI);
			_globals->_eventType = EventType::etEvent3;
			showEvents();
			waitEndSpeak();
			if (_vm->shouldQuit())
				return;
			closeCharacterScreen();
			removeFromParty(PER_ELOI);
			_globals->_roomNum = 2818;
			_globals->_areaNum = Areas::arWhiteArch;
			_globals->_eventType = EventType::etEvent5;
			_globals->_valleyVidNum = 0;
			_globals->_mirrorEffect = 6; // CHECKME: Verify the value
			_globals->_newMusicType = MusicType::mtNormal;
			updateRoom(_globals->_roomNum);
		} else {
			_globals->_dialogType = DialogType::dtHint;
			perso1(&_persons[PER_EVE]);
		}
		break;
	case Objects::obShell:
		_globals->_dialogType = DialogType::dtHint;
		perso1(&_persons[PER_TAU]);
		break;
	case Objects::obFlute:
	case Objects::obTrumpet:
		if (_globals->_roomCharacterType) {
			quitMirror();
			updateRoom(_globals->_roomNum);
			actionDino();
		} else
			comment();
		break;
	case Objects::obTablet1:
	case Objects::obTablet2:
	case Objects::obTablet3:
	case Objects::obTablet4:
	case Objects::obTablet5:
	case Objects::obTablet6: {
		if ((_globals->_partyOutside & PersonMask::pmDina)
		        && _globals->_curObjectId == Objects::obTablet1 && _globals->_phaseNum == 370)
			incPhase();
		char *objvid = &_tabletView[(_globals->_curObjectId - Objects::obTablet1) * 2];
		object_t *object = getObjectPtr(*objvid++);
		int16 vid = 84;
		if (!object->_count)
			vid = *objvid;
		_graphics->hideBars();
		_specialTextMode = true;
		_graphics->playHNM(vid);
		_paletteUpdateRequired = true;
		_globals->_mirrorEffect = 16; // CHECKME: Verify the value
		_graphics->showBars();
		gameToMirror(0);
		}
		break;
	case Objects::obApple:
	case Objects::obShroom:
	case Objects::obBadShroom:
	case Objects::obNest:
	case Objects::obFullNest:
	case Objects::obDrum:
		if (_globals->_party & PersonMask::pmThugg) {
			_globals->_dialogType = DialogType::dtHint;
			perso1(&_persons[PER_GUARDS]);
		}
		break;
	default:
		comment();
	}
}

// Original name: oui and init_oui
void EdenGame::setChoiceYes()  {
	_lastDialogChoice = true;
}

// Original name: non and init_non
void EdenGame::setChoiceNo()  {
	_lastDialogChoice =  false;
}

// Original name: verif_oui
bool EdenGame::isAnswerYes() {
	return _lastDialogChoice;
}

// Original name: SpcChampi
void EdenGame::specialMushroom(perso_t *perso) {
	perso->_flags |= PersonFlags::pf10;
	_globals->_areaPtr->_flags |= AreaFlags::afFlag2;
	_globals->_curAreaFlags |= AreaFlags::afFlag2;
}

// Original name: SpcNidv
void EdenGame::specialEmptyNest(perso_t *perso) {
	if (!isAnswerYes())
		return;
	perso->_flags |= PersonFlags::pf10;
	_globals->_roomCharacterFlags |= PersonFlags::pf10;
	_globals->_gameFlags |= GameFlags::gfFlag400;
	if (_globals->_characterPtr == &_persons[PER_EVE]) {
		_globals->_areaPtr->_flags |= AreaFlags::afFlag4;
		_globals->_curAreaFlags |= AreaFlags::afFlag4;
		perso->_flags |= PersonFlags::pfInParty;
		_globals->_roomCharacterFlags |= PersonFlags::pfInParty;
		placeVava(_globals->_areaPtr);
	} else {
		perso->_flags &= ~PersonFlags::pf10;
		_globals->_roomCharacterFlags &= ~PersonFlags::pf10;
	}
}

// Original name: SpcNido
void EdenGame::specialNestWithEggs(perso_t *perso) {
	if (perso == &_persons[PER_GUARDS])
		giveObject();
}

// Original name: SpcPomme
void EdenGame::specialApple(perso_t *perso) {
	perso->_flags |= PersonFlags::pf10;
	_globals->_areaPtr->_flags |= AreaFlags::afFlag8;
	_globals->_curAreaFlags |= AreaFlags::afFlag8;
	_globals->_gameFlags |= GameFlags::gfFlag200;
}

// Original name: SpcOr
void EdenGame::specialGold(perso_t *perso) {
	if (!isAnswerYes())
		return;
	perso->_items = _curSpecialObject->_itemMask;
	_globals->_roomPersoItems = _curSpecialObject->_itemMask;
	perso->_flags |= PersonFlags::pf10;
	perso->_flags &= ~PersonFlags::pfInParty;
	perso->_targetLoc = 0;
	_globals->_areaPtr->_flags |= AreaFlags::afGaveGold;
	_globals->_curAreaFlags |= AreaFlags::afGaveGold;
	if (_globals->_phaseNum == 226)
		incPhase();
}

// Original name: SpcPrisme
void EdenGame::specialPrism(perso_t *perso) {
	if (perso == &_persons[PER_DINA]) {
		if (_globals->_partyOutside & PersonMask::pmMonk)
			_globals->_gameFlags |= GameFlags::gfPrismAndMonk;
	}
}

// Original name: SpcTalisman
void EdenGame::specialTalisman(perso_t *perso) {
	if (perso == &_persons[PER_DINA])
		addToParty(PER_DINA);
}

// Original name: SpcMasque
void EdenGame::specialMask(perso_t *perso) {
	if (perso == &_persons[PER_BAMBOO]) {
		dialautoon();
		parlemoiNormalFlag = true;
	}
}

// Original name: SpcSac
void EdenGame::specialBag(perso_t *perso) {
	if (_globals->_textToken1 != 3)
		return;
	if (perso == &_persons[PER_KABUKA] || perso == &_persons[PER_MAMMI_3])
		loseObject(_curSpecialObject->_id);
}

// Original name: SpcTrompet
void EdenGame::specialTrumpet(perso_t *perso) {
	if (!isAnswerYes())
		return;
	_globals->_var54 = 4;
	winObject(Objects::obTrumpet);
	_globals->_drawFlags |= DrawFlags::drDrawInventory;
	_closeCharacterDialog = true;
	tyranDies(_globals->_roomCharacterPtr);
}

// Original name: SpcArmes
void EdenGame::specialWeapons(perso_t *perso) {
	if (!isAnswerYes())
		return;
	perso->_powers = _curSpecialObject->_powerMask;
	_globals->_roomCharacterPowers = _curSpecialObject->_powerMask;
	giveObject();
}

// Original name: SpcInstru
void EdenGame::specialInstrument(perso_t *perso) {
	if (!isAnswerYes())
		return;
	if (perso == &_persons[PER_MONK]) {
		_globals->_partyInstruments &= ~1;   //TODO: check me
		if (_curSpecialObject->_id == Objects::obRing) {
			_globals->_partyInstruments |= 1;
			_globals->_monkGotRing++;                //TODO: |= 1 ?
		}
	}
	if (perso == &_persons[PER_GUARDS]) {
		_globals->_partyInstruments &= ~2;
		if (_curSpecialObject->_id == Objects::obDrum)
			_globals->_partyInstruments |= 2;
	}
	perso->_powers = _curSpecialObject->_powerMask;
	_globals->_curCharacterPowers = _curSpecialObject->_powerMask;
	giveObject();
}

// Original name: SpcOeuf
void EdenGame::specialEgg(perso_t *perso) {
	if (!isAnswerYes())
		return;
	_gameIcons[131]._cursorId &= ~0x8000;
	_globals->_characterBackgroundBankIdx = 62;
	dialautoon();
}

// Original name: TyranMeurt
void EdenGame::tyranDies(perso_t *perso) {
	perso->_flags |= PersonFlags::pf80;
	perso->_roomNum = 0;
	removeInfo(_globals->_areaNum + ValleyNews::vnTyrannIn);
	_globals->_roomCharacterType = 0;
	_globals->_roomCharacterFlags = 0;
	_globals->_chronoFlag = 0;
}

void EdenGame::specialObjects(perso_t *perso, char objid) {

#include "common/pack-start.h"	// START STRUCT PACKING

	struct SpecialObject {
		int8  _characterType;
		int8  _objectId;
		void  (EdenGame::*dispFct)(perso_t *perso);
	};

#include "common/pack-end.h"	// END STRUCT PACKING

	static SpecialObject kSpecialObjectActions[] = {
		//    persoType, objectId, dispFct
		{ PersonFlags::pfType8, Objects::obShroom, &EdenGame::specialMushroom },
		{ PersonFlags::pftTriceraptor, Objects::obNest, &EdenGame::specialEmptyNest },
		{ PersonFlags::pfType0, Objects::obFullNest, &EdenGame::specialNestWithEggs },
		{ PersonFlags::pftMosasaurus, Objects::obApple, &EdenGame::specialApple },
		{ PersonFlags::pftVelociraptor, Objects::obGold, &EdenGame::specialGold },
		{ PersonFlags::pfType0, Objects::obPrism, &EdenGame::specialPrism },
		{ PersonFlags::pfType0, Objects::obTalisman, &EdenGame::specialTalisman },
		{ PersonFlags::pfType2, Objects::obMaskOfDeath, &EdenGame::specialMask },
		{ PersonFlags::pfType2, Objects::obMaskOfBonding, &EdenGame::specialMask },
		{ PersonFlags::pfType2, Objects::obMaskOfBirth, &EdenGame::specialMask },
		{ PersonFlags::pfType0, Objects::obBag, &EdenGame::specialBag },
		{ PersonFlags::pfType2, Objects::obBag, &EdenGame::specialBag },
		{ PersonFlags::pftTyrann, Objects::obTrumpet, &EdenGame::specialTrumpet },
		{ PersonFlags::pftVelociraptor, Objects::obEyeInTheStorm, &EdenGame::specialWeapons },
		{ PersonFlags::pftVelociraptor, Objects::obSkyHammer, &EdenGame::specialWeapons },
		{ PersonFlags::pftVelociraptor, Objects::obFireInTheClouds, &EdenGame::specialWeapons },
		{ PersonFlags::pftVelociraptor, Objects::obWithinAndWithout, &EdenGame::specialWeapons },
		{ PersonFlags::pftVelociraptor, Objects::obEyeInTheCyclone, &EdenGame::specialWeapons },
		{ PersonFlags::pftVelociraptor, Objects::obRiverThatWinds, &EdenGame::specialWeapons },
		{ PersonFlags::pfType0, Objects::obTrumpet, &EdenGame::specialInstrument },
		{ PersonFlags::pfType0, Objects::obDrum, &EdenGame::specialInstrument },
		{ PersonFlags::pfType0, Objects::obRing, &EdenGame::specialInstrument },
		{ PersonFlags::pfType0, Objects::obEgg, &EdenGame::specialEgg },
		{ -1, -1, nullptr }
	};

	char characterType = perso->_flags & PersonFlags::pfTypeMask;
	_curSpecialObject = &_objects[objid - 1];
	for (SpecialObject *spcObj = kSpecialObjectActions; spcObj->_characterType != -1; spcObj++) {
		if (spcObj->_objectId == objid && spcObj->_characterType == characterType) {
			(this->*spcObj->dispFct)(perso);
			break;
		}
	}
}

void EdenGame::dialautoon() {
	_globals->_iconsIndex = 4;
	_globals->_autoDialog = true;
	putObject();
}

void EdenGame::dialautooff() {
	_globals->_iconsIndex = 0x10;
	_globals->_autoDialog = false;
}

void EdenGame::follow() {
	if (_globals->_roomCharacterType == PersonFlags::pfType12) {
		debug("follow: hiding person %d", (int)(_globals->_roomCharacterPtr - _persons));
		_globals->_roomCharacterPtr->_flags |= PersonFlags::pf80;
		_globals->_roomCharacterPtr->_roomNum = 0;
		_globals->_gameFlags |= GameFlags::gfFlag8;
		_gameIcons[123]._objectId = 18;
		_gameIcons[124]._objectId = 35;
		_gameIcons[125]._cursorId &= ~0x8000;
		_globals->_characterBackgroundBankIdx = 56;
	} else
		AddCharacterToParty();
}

void EdenGame::dialonfollow() {
	_globals->_iconsIndex = 4;
	_globals->_autoDialog = true;
	follow();
}

// Original name: abortdial
void EdenGame::abortDialogue() {
	_globals->_varF6++;
	if (_globals->_roomCharacterType != PersonFlags::pftTriceraptor || _globals->_characterPtr != &_persons[PER_EVE])
		return;
	_globals->_areaPtr->_flags |= AreaFlags::afFlag4;
	_globals->_curAreaFlags |= AreaFlags::afFlag4;
	_globals->_roomCharacterPtr->_flags |= PersonFlags::pfInParty;
	_globals->_roomCharacterFlags |= PersonFlags::pfInParty;
	placeVava(_globals->_areaPtr);
}

void EdenGame::subHandleNarrator() {
	_globals->_varF2 &= ~1;  //TODO: check me
	if (_globals->_narratorSequence > 50 && _globals->_narratorSequence <= 80)
		_globals->_endGameFlag = 50;
	if (_globals->_narratorSequence == 3)
		setChrono(1200);
	_globals->_narratorSequence = 0;
}

// Original name: narrateur
void EdenGame::handleNarrator() {
	if (!(_globals->_displayFlags & DisplayFlags::dfFlag1))
		return;
	if (!_globals->_narratorSequence) {
		if (_globals->_var6A == _globals->_var69) {
			subHandleNarrator();
			return;
		}

		narratorBuildCitadel();
	}
	_globals->_varF5 |= 0x80;
	_globals->_varF2 &= ~1;  //TODO: check me
	_globals->_characterPtr = &_persons[PER_UNKN_156];
	_globals->_var60 = 0;
	_globals->_eventType = 0;
	_globals->_var103 = 69;
	if (dialogEvent(&_persons[PER_UNKN_156])) {
		_globals->_narratorDialogPtr = _globals->_dialogPtr;
		dialautoon();
		_globals->_varF2 |= 1;
		waitEndSpeak();
		if (_vm->shouldQuit())
			return;
		endCharacterSpeech();
		while (dialoscansvmas(_globals->_narratorDialogPtr)) {
			_globals->_narratorDialogPtr = _globals->_dialogPtr;
			waitEndSpeak();
			if (_vm->shouldQuit())
				return;
			endCharacterSpeech();
		}
		_globals->_narratorDialogPtr = _globals->_dialogPtr;
		_globals->_mirrorEffect = 0;
		_globals->_var103 = 0;
		closeCharacterScreen();
		placeVava(_globals->_areaPtr);
		if (_globals->_narratorSequence == 8)
			deplaval(134);
	}
	_globals->_var103 = 0;
	if (_globals->_narratorSequence == 10) {
		addToParty(PER_ELOI);
		addToParty(PER_EVE);
		addToParty(PER_MONK);
		addToParty(PER_GUARDS);
		removeFromParty(PER_MUNGO);
		_globals->_eloiHaveNews = 0;
		deplaval(139);
	}
	_globals->_eventType = EventType::etEventD;
	showEvents();
	_globals->_varF5 &= ~0x80;

	subHandleNarrator();
}

// Original name: vrf_phrases_file
void EdenGame::checkPhraseFile() {
	int16 num = 3;
	if (_globals->_dialogPtr < (Dialog *)getElem(_gameDialogs, 48))
		num = 1;
	else if (_globals->_dialogPtr < (Dialog *)getElem(_gameDialogs, 128))
		num = 2;
	_globals->_textBankIndex = num;
	if (_globals->_prefLanguage)
		num += (_globals->_prefLanguage - 1) * 3;
	if (num == _lastPhrasesFile)
		return;
	_lastPhrasesFile = num;
	num += 404;
	loadRawFile(num, _gamePhrases);
	verifh(_gamePhrases);
}

// Original name: gettxtad
byte *EdenGame::getPhrase(int16 id) {
	checkPhraseFile();
	return (byte *)getElem(_gamePhrases, id - 1);
}

// Original name: gotocarte
void EdenGame::actionGotoMap() {
	Goto *go = &_gotos[_curSpot2->_objectId];
	endCharacterSpeech();
	byte newArea = go->_areaNum;
	_globals->_newRoomNum = (go->_areaNum << 8) | 1;
	_globals->_newLocation = 1;
	_globals->_prevLocation = _globals->_roomNum & 0xFF;
	char curArea = _globals->_roomNum >> 8;
	if (curArea == go->_areaNum)
		newArea = 0;
	else {
		for (; go->_curAreaNum != 0xFF; go++) {
			if (go->_curAreaNum == curArea)
				break;
		}

		if (go->_areaNum == 0xFF)
			return;
	}
	_globals->_eventType = EventType::etGotoArea | newArea;
	setChoiceYes();
	showEvents1();
	waitEndSpeak();
	if (_vm->shouldQuit())
		return;

	closeCharacterScreen();
	if (isAnswerYes())
		gotoPlace(go);
}

void EdenGame::record() {
	if (_globals->_curObjectId)
		return;

	if (_globals->_characterPtr >= &_persons[PER_UNKN_18C])
		return;

	if (_globals->_eventType == EventType::etEventE || _globals->_eventType >= EventType::etGotoArea)
		return;

	for (tape_t *tape = _tapes; tape != _tapes + MAX_TAPES; tape++) {
		if (tape->_textNum == _globals->_textNum)
			return;
	}

	tape_t *tape = _tapes;
	for (int16 i = 0; i < MAX_TAPES - 1; i++) {
		tape->_textNum = tape[+1]._textNum;
		tape->_perso = tape[+1]._perso;
		tape->_party = tape[+1]._party;
		tape->_roomNum = tape[+1]._roomNum;
		tape->_backgroundBankNum = tape[+1]._backgroundBankNum;
		tape->_dialog = tape[+1]._dialog;
		tape++;
	}

	perso_t *perso = _globals->_characterPtr;
	if (perso == &_persons[PER_EVE])
		perso = _globals->_phaseNum >= 352 ? &_persons[PER_UNKN_372]
		        : &_persons[PER_UNKN_402];
	tape->_textNum = _globals->_textNum;
	tape->_perso = perso;
	tape->_party = _globals->_party;
	tape->_roomNum = _globals->_roomNum;
	tape->_backgroundBankNum = _globals->_roomBackgroundBankNum;
	tape->_dialog = _globals->_dialogPtr;
}

bool EdenGame::dial_scan(Dialog *dial) {
	if (_globals->_numGiveObjs) {
		if (!(_globals->_displayFlags & DisplayFlags::dfFlag2))
			showObjects();
		_globals->_numGiveObjs = 0;
	}
	_globals->_dialogPtr = dial;
	vavapers();
	_globals->_sentenceBufferPtr = _sentenceBuffer;
	byte hidx, lidx;
	uint16 mask = 0;
	bool skipFl = false;
	for (;; _globals->_dialogPtr++) {
		for (;; _globals->_dialogPtr++) {
			if (_globals->_dialogPtr->_flags == -1 && _globals->_dialogPtr->_condNumLow == -1)
				return false;
			byte flags = _globals->_dialogPtr->_flags;
			_globals->_dialogFlags = flags;
			if (!(flags & DialogFlags::dfSpoken) || (flags & DialogFlags::dfRepeatable)) {
				hidx = (_globals->_dialogPtr->_textCondHiMask >> 6) & 3;
				lidx = _globals->_dialogPtr->_condNumLow;
				if (flags & 0x10)
					hidx |= 4;
				if (testCondition(((hidx << 8) | lidx) & 0x7FF))
					break;
			} else {
				if (flags & _dialogSkipFlags)
					continue;
				hidx = (_globals->_dialogPtr->_textCondHiMask >> 6) & 3;
				lidx = _globals->_dialogPtr->_condNumLow;
				if (flags & 0x10)
					hidx |= 4;
				if (testCondition(((hidx << 8) | lidx) & 0x7FF))
					break;
			}
		}
		char bidx = (_globals->_dialogPtr->_textCondHiMask >> 2) & 0xF;
		if (!bidx) {
			skipFl = true;
			break;
		}

		mask = (_globals->_party | _globals->_partyOutside) & (1 << (bidx - 1));
		if (mask)
			break;
	}

	if (!skipFl) {
		perso_t *perso;
		for (perso = _persons; !(perso->_partyMask == mask && perso->_roomNum == _globals->_roomNum); perso++)
			; //Find matching

		_globals->_characterPtr = perso;
		initCharacterPointers(perso);
		no_perso();
	}

	hidx = _globals->_dialogPtr->_textCondHiMask;
	lidx = _globals->_dialogPtr->_textNumLow;
	_globals->_textNum = ((hidx << 8) | lidx) & 0x3FF;
	if (_globals->_sentenceBufferPtr != _sentenceBuffer) {
		for (int16 i = 0; i < 32; i++)
			SysBeep(1);
	} else
		my_bulle();
	if (!dword_30B04) {
		static void (EdenGame::*talk_subject[])() = {
			&EdenGame::setChoiceYes,
			&EdenGame::setChoiceNo,
			&EdenGame::handleEloiDeparture,
			&EdenGame::dialautoon,
			&EdenGame::dialautooff,
			&EdenGame::characterStayHere,
			&EdenGame::follow,
			&EdenGame::citadelle,
			&EdenGame::dialonfollow,
			&EdenGame::abortDialogue,
			&EdenGame::incPhase,
			&EdenGame::bigphase,
			&EdenGame::giveObject,
			&EdenGame::selectZone,
			&EdenGame::lostObject
		};
		char pnum = _globals->_dialogPtr->_flags & 0xF;
		if (pnum)
			(this->*talk_subject[pnum - 1])();
		_globals->_var60 = 1;
		_globals->_dialogPtr->_flags |= DialogFlags::dfSpoken;
		_globals->_dialogPtr++;
	}
	if (_globals->_dialogType != DialogType::dtInspect) {
		record();
		getDataSync();
		showCharacter();
		persovox();
	}
	return true;
}

bool EdenGame::dialoscansvmas(Dialog *dial) {
	byte oldFlag = _dialogSkipFlags;
	_dialogSkipFlags = DialogFlags::df20;
	bool res = dial_scan(dial);
	_dialogSkipFlags = oldFlag;
	return res;
}

byte *EdenGame::getGameDialogs() {
	return _gameDialogs;
}

// Original name: dialo_even
bool EdenGame::dialogEvent(perso_t *perso) {
	_globals->_characterPtr = perso;
	int num = (perso->_id << 3) | DialogType::dtEvent;
	Dialog *dial = (Dialog *)getElem(_gameDialogs, num);
	bool retVal = dialoscansvmas(dial);
	_globals->_lastDialogPtr = nullptr;
	parlemoiNormalFlag = false;
	return retVal;
}

// Original name: stay_here
void EdenGame::characterStayHere() {
	if (_globals->_characterPtr == &_persons[PER_DINA] && _globals->_roomNum == 260)
		_globals->_gameFlags |= GameFlags::gfFlag1000;
	removeCharacterFromParty();
}

// Original name: mort
void EdenGame::endDeath(int16 vid) {
	_graphics->hideBars();
	_graphics->playHNM(vid);
	_graphics->fadeToBlack(2);
	CLBlitter_FillScreenView(0);
	CLBlitter_FillView(_graphics->getMainView(), 0);
	_graphics->showBars();
	_globals->_narratorSequence = 51;
	_globals->_newMusicType = MusicType::mtNormal;
	musique();
	musicspy();
}

// Original name: evenchrono
void EdenGame::chronoEvent() {
	if (!(_globals->_displayFlags & DisplayFlags::dfFlag1))
		return;

	uint16 oldGameTime = _globals->_gameTime;
	_currentTime = _vm->_timerTicks / 100;
	_globals->_gameTime = _currentTime;
	if (_globals->_gameTime <= oldGameTime)
		return;
	addTime(5);
	if (!(_globals->_chronoFlag & 1))
		return;
	_globals->_chrono -= 200;
	if (_globals->_chrono == 0)
		_globals->_chronoFlag |= 2;
	if (!(_globals->_chronoFlag & 2))
		return;
	_globals->_chronoFlag = 0;
	_globals->_chrono = 0;
	if (_globals->_roomCharacterType == PersonFlags::pftTyrann) {
		int16 vid = 272;
		if (_globals->_curRoomFlags & 0xC0) {
			vid += 2;
			if ((_globals->_curRoomFlags & 0xC0) != 0x80) {
				vid += 2;
				endDeath(vid);
				return;
			}
		}
		if (_globals->_areaNum == Areas::arUluru || _globals->_areaNum == Areas::arTamara)
			endDeath(vid);
		else
			endDeath(vid + 1);
		return;
	}
	if (_globals->_roomNum == 2817) {
		addToParty(PER_ELOI);
		_globals->_gameFlags |= GameFlags::gfFlag40;
		dialautoon();
	} else
		handleEloiReturn();
	_globals->_eventType = EventType::etEvent10;
	showEvents();
}

// Original name: chronoon
void EdenGame::setChrono(int16 t) {
	_globals->_chrono = t;
	_globals->_chronoFlag = 1;
}

void EdenGame::wait(int howlong) {
	int t = g_system->getMillis();

	for (int t2 = t; t2 - t < howlong; t2 = g_system->getMillis())
		g_system->delayMillis(10); // waste time
}

//////

// Original name: ajouinfo
void EdenGame::addInfo(byte info) {
	byte idx = _globals->_nextInfoIdx;
	if (_persons[PER_ELOI]._roomNum)
		info |= 0x80;
	_infoList[idx] = info;
	if (idx == _globals->_lastInfoIdx)
		_globals->_lastInfo = info;
	idx++;
	if (idx == 16)
		idx = 0;
	_globals->_nextInfoIdx = idx;
}

void EdenGame::unlockInfo() {
	for (byte idx = 0; idx < 16; idx++) {
		if (_infoList[idx] != 0xFF)
			_infoList[idx] &= ~0x80;
	}
	_globals->_lastInfo &= ~0x80;
}

void EdenGame::nextInfo() {
	do {
		byte idx = _globals->_lastInfoIdx;
		_infoList[idx] = 0;
		idx++;
		if (idx == 16)
			idx = 0;
		_globals->_lastInfoIdx = idx;
		_globals->_lastInfo = _infoList[idx];
	} while (_globals->_lastInfo == 0xFF);
}

// Original name: delinfo
void EdenGame::removeInfo(byte info) {
	for (byte idx = 0; idx < 16; idx++) {
		if ((_infoList[idx] & ~0x80) == info) {
			_infoList[idx] = 0xFF;
			if (idx == _globals->_lastInfoIdx)
				nextInfo();
			break;
		}
	}
}

void EdenGame::updateInfoList() {
	for (int idx = 0; idx < 16; idx++)
		_infoList[idx] = 0;
}

void EdenGame::initGlobals() {
	_gameIcons[16]._cursorId |= 0x8000;

	_globals->_areaNum = Areas::arMo;
	_globals->_areaVisitCount = 1;
	_globals->_menuItemIdLo = 0;
	_globals->_menuItemIdHi = 0;
	_globals->_randomNumber = 0;
	_globals->_gameTime = 0;
	_globals->_gameDays = 0;
	_globals->_chrono = 0;
	_globals->_eloiDepartureDay = 0;
	_globals->_roomNum = 259;
	_globals->_newRoomNum = 0;
	_globals->_phaseNum = 0;
	_globals->_metPersonsMask1 = 0;
	_globals->_party = 0;
	_globals->_partyOutside = 0;
	_globals->_metPersonsMask2 = 0;
	_globals->_phaseActionsCount = 0;
	_globals->_curAreaFlags = 0;
	_globals->_curItemsMask = 0;
	_globals->_curPowersMask = 0;
	_globals->_curPersoItems = 0;
	_globals->_curCharacterPowers = 0;
	_globals->_wonItemsMask = 0;
	_globals->_wonPowersMask = 0;
	_globals->_stepsToFindAppleFast = 0;
	_globals->_stepsToFindAppleNormal = 0;
	_globals->_roomPersoItems = 0;
	_globals->_roomCharacterPowers = 0;
	_globals->_gameFlags = GameFlags::gfNone;
	_globals->_curVideoNum = 0;
	_globals->_morkusSpyVideoNum1 = 89;
	_globals->_morkusSpyVideoNum2 = 88;
	_globals->_morkusSpyVideoNum3 = 83;
	_globals->_morkusSpyVideoNum4 = 94;
	_globals->_newMusicType = MusicType::mtDontChange;
	_globals->_var43 = 0;
	_globals->_videoSubtitleIndex = 0;
	_globals->_partyInstruments = 0;
	_globals->_monkGotRing = 0;
	_globals->_chronoFlag = 0;
	_globals->_curRoomFlags = 0;
	_globals->_endGameFlag = 0;
	_globals->_lastInfo = 0;
	_globals->_autoDialog = false;
	_globals->_worldTyranSighted = 0;
	_globals->_var4D = 0;
	_globals->_var4E = 0;
	_globals->_worldGaveGold = 0;
	_globals->_worldHasTriceraptors = 0;
	_globals->_worldHasVelociraptors = 0;
	_globals->_worldHasTyran = 0;
	_globals->_var53 = 0;
	_globals->_var54 = 0;
	_globals->_var55 = 0;
	_globals->_gameHours = 0;
	_globals->_textToken1 = 0;
	_globals->_textToken2 = 0;
	_globals->_eloiHaveNews = 0;
	_globals->_dialogFlags = 0;
	_globals->_curAreaType = 0;
	_globals->_curCitadelLevel = 0;
	_globals->_newLocation = 0;
	_globals->_prevLocation = 0;
	_globals->_curPersoFlags = 0;
	_globals->_var60 = 0;
	_globals->_eventType = EventType::etEvent5;
	_globals->_var62 = 0;
	_globals->_curObjectId = 0;
	_globals->_curObjectFlags = 0;
	_globals->_var65 = 1;
	_globals->_roomCharacterType = 0;
	_globals->_roomCharacterFlags = 0;
	_globals->_narratorSequence = 0;
	_globals->_var69 = 0;
	_globals->_var6A = 0;
	_globals->_frescoNumber = 0;
	_globals->_var6C = 0;
	_globals->_var6D = 0;
	_globals->_labyrinthDirections = 0;
	_globals->_labyrinthRoom = 0;
	_globals->_curCharacterAnimPtr = nullptr;
	_globals->_characterImageBank = 0;
	_globals->_roomImgBank = 0;
	_globals->_characterBackgroundBankIdx = 55;
	_globals->_varD4 = 0;
	_globals->_frescoeWidth = 0;
	_globals->_frescoeImgBank = 0;
	_globals->_varDA = 0;
	_globals->_varDC = 0;
	_globals->_roomBaseX = 0;
	_globals->_varE0 = 0;
	_globals->_dialogType = DialogType::dtTalk;
	_globals->_varE4 = 0;
	_globals->_currMusicNum = 0;
	_globals->_textNum = 0;
	_globals->_travelTime = 0;
	_globals->_varEC = 0;
	_globals->_displayFlags = DisplayFlags::dfFlag1;
	_globals->_oldDisplayFlags = 1;
	_globals->_drawFlags = 0;
	_globals->_varF1 = 0;
	_globals->_varF2 = 0;
	_globals->_menuFlags = 0;
	_globals->_varF5 = 0;
	_globals->_varF6 = 0;
	_globals->_varF7 = 0;
	_globals->_varF8 = 0;
	_globals->_varF9 = 0;
	_globals->_varFA = 0;
	_globals->_animationFlags = 0;
	_globals->_giveObj1 = 0;
	_globals->_giveObj2 = 0;
	_globals->_giveObj3 = 0;
	_globals->_var100 = 0;
	_globals->_roomVidNum = 0;
	_globals->_mirrorEffect = 0;
	_globals->_var103 = 0;
	_globals->_roomBackgroundBankNum = 0;
	_globals->_valleyVidNum = 0;
	_globals->_updatePaletteFlag = 0;
	_globals->_inventoryScrollPos = 0;
	_globals->_objCount = 0;
	_globals->_textBankIndex = 69;
	_globals->_citadelAreaNum = 0;
	_globals->_var113 = 0;
	_globals->_lastPlaceNum = 0;
	_globals->_dialogPtr = nullptr;
	_globals->_tapePtr = _tapes;
	_globals->_nextDialogPtr = nullptr;
	_globals->_narratorDialogPtr = nullptr;
	_globals->_lastDialogPtr = nullptr;
	_globals->_nextRoomIcon = nullptr;
	_globals->_sentenceBufferPtr = nullptr;
	_globals->_roomPtr = nullptr;
	_globals->_areaPtr = nullptr;
	_globals->_lastAreaPtr = nullptr;
	_globals->_curAreaPtr = nullptr;
	_globals->_citaAreaFirstRoom = 0;
	_globals->_characterPtr = nullptr;
	_globals->_roomCharacterPtr = 0;
	_globals->_lastInfoIdx = 0;
	_globals->_nextInfoIdx = 0;
	_globals->_iconsIndex = 16;
	_globals->_persoSpritePtr = nullptr;
	_globals->_numGiveObjs = 0;

	_graphics->initRects();

	_graphics->initGlobals();

	_graphics->setSavedUnderSubtitles(false);
}

// Original name: closesalle
void EdenGame::closeRoom() {
	if (_globals->_displayFlags & DisplayFlags::dfPanable) {
		_globals->_displayFlags &= ~DisplayFlags::dfPanable;
		resetScroll();
	}
}

// Original name: aflieu
void EdenGame::displayPlace() {
	no_perso();
	if (!_vm->shouldQuit()) {
		_globals->_iconsIndex = 16;
		_globals->_autoDialog = false;
	}
	_globals->_nextRoomIcon = &_gameIcons[_roomIconsBase];
	_graphics->displayRoom();
	_paletteUpdateRequired = true;
}

// Original name: loadsal
void EdenGame::loadPlace(int16 num) {
	if (num == _globals->_lastPlaceNum)
		return;
	_globals->_lastPlaceNum = num;
	loadRawFile(num + 419, _placeRawBuf);
}

byte EdenGame::getActionCursor(byte value) {
	return _actionCursors[value];
}

byte *EdenGame::getPlaceRawBuf() {
	return _placeRawBuf;
}

void EdenGame::specialoutside() {
	if (_globals->_lastAreaPtr->_type == AreaType::atValley && (_globals->_party & PersonMask::pmLeader))
		perso_ici(5);
}

void EdenGame::specialout() {
	if (_globals->_gameDays - _globals->_eloiDepartureDay > 2) {
		if (checkEloiReturn())
			handleEloiReturn();
	}

	if (_globals->_phaseNum >= 32 && _globals->_phaseNum < 48) {
		if (_globals->_newLocation == 9 || _globals->_newLocation == 4 || _globals->_newLocation == 24) {
			_persons[PER_ELOI]._roomNum = 263;
			return;
		}
	}

	if ((_globals->_phaseNum == 434) && (_globals->_newLocation == 5)) {
		removeFromParty(PER_JABBER);
		_persons[PER_JABBER]._roomNum = 264;
		return;
	}

	if (_globals->_phaseNum < 400) {
		if ((_globals->_gameFlags & GameFlags::gfFlag4000) && _globals->_prevLocation == 1
		        && (_globals->_party & PersonMask::pmEloi) && _globals->_curAreaType == AreaType::atValley)
			handleEloiDeparture();
	}

	if (_globals->_phaseNum == 386) {
		if (_globals->_prevLocation == 1
		        && (_globals->_party & PersonMask::pmEloi) && _globals->_areaNum == Areas::arCantura)
			handleEloiDeparture();
	}
}

void EdenGame::specialin() {
	if (!(_globals->_party & PersonMask::pmEloi) && (_globals->_partyOutside & PersonMask::pmEloi) && (_globals->_roomNum & 0xFF) == 1) {
		addToParty(PER_ELOI);
		_globals->_eloiHaveNews = 1;
	}
	if (_globals->_roomNum == 288)
		_globals->_gameFlags |= GameFlags::gfFlag100 | GameFlags::gfFlag2000;
	if (_globals->_roomNum == 3075 && _globals->_phaseNum == 546) {
		incPhase();
		if (_globals->_curItemsMask & 0x2000) { // Morkus' tablet
			_graphics->hideBars();
			_graphics->playHNM(92);
			_gameRooms[129]._exits[0] = 0;
			_gameRooms[129]._exits[2] = 1;
			_globals->_roomNum = 3074;
			_persons[PER_MUNGO]._roomNum = 3074;
			_globals->_eventType = EventType::etEvent5;
			updateRoom(_globals->_roomNum);
			return;
		}
		_globals->_narratorSequence = 53;
	}
	if (_globals->_roomNum == 1793 && _globals->_phaseNum == 336)
		handleEloiDeparture();
	if (_globals->_roomNum == 259 && _globals->_phaseNum == 129)
		_globals->_narratorSequence = 12;
	if (_globals->_roomNum >= 289 && _globals->_roomNum < 359)
		_globals->_labyrinthDirections = _labyrinthPath[(_globals->_roomNum & 0xFF) - 33];
	if (_globals->_roomNum == 305 && _globals->_prevLocation == 103)
		_globals->_gameFlags &= ~GameFlags::gfFlag2000;
	if (_globals->_roomNum == 304 && _globals->_prevLocation == 105)
		_globals->_gameFlags &= ~GameFlags::gfFlag2000;
	if (_globals->_phaseNum < 226) {
		if (_globals->_roomNum == 842)
			_globals->_gameFlags |= GameFlags::gfFlag2;
		if (_globals->_roomNum == 1072)
			_globals->_gameFlags |= GameFlags::gfFlag4;
		if (_globals->_roomNum == 1329)
			_globals->_gameFlags |= GameFlags::gfFlag8000;
	}
}

void EdenGame::animpiece() {
	Room *room = _globals->_roomPtr;
	if (_globals->_roomVidNum && _globals->_var100 != 0xFF) {
		if (_globals->_valleyVidNum || !room->_level || (room->_flags & RoomFlags::rfHasCitadel)
		        || room->_level == _globals->_var100) {
			_graphics->hideBars();
			_globals->_updatePaletteFlag = 16;
			if (!(_globals->_narratorSequence & 0x80)) //TODO: bug? !() @ 100DC
				_globals->_mirrorEffect = 0;
			if (!_graphics->getFade())
				_graphics->setFade(room->_flags & RoomFlags::rf02);
			_graphics->playHNM(_globals->_roomVidNum);
			return;
		}
	}
	_globals->_varF1 &= ~RoomFlags::rf04;
}

void EdenGame::getdino(Room *room) {
	assert(tab_2CEF0[4] == 0x25);

	room->_flags &= ~0xC;
	for (perso_t *perso = &_persons[PER_UNKN_18C]; perso->_roomNum != 0xFFFF; perso++) {
		if (perso->_flags & PersonFlags::pf80)
			continue;
		if (perso->_roomNum != _globals->_roomNum)
			continue;
		byte persoType = perso->_flags & PersonFlags::pfTypeMask;
		if (persoType == PersonFlags::pftVelociraptor)
			removeInfo(_globals->_citadelAreaNum + ValleyNews::vnVelociraptorsIn);
		if (persoType == PersonFlags::pftTriceraptor)
			removeInfo(_globals->_citadelAreaNum + ValleyNews::vnTriceraptorsIn);
		perso->_flags |= PersonFlags::pf20;
		int16 *tab = tab_2CF70;
		if (_globals->_areaNum != Areas::arUluru && _globals->_areaNum != Areas::arTamara)
			tab = tab_2CEF0;
		byte r27 = (room->_flags & 0xC0) >> 2;    //TODO: check me (like pc)
		persoType = perso->_flags & PersonFlags::pfTypeMask;
		if (persoType == PersonFlags::pftTyrann)
			persoType = 13;
		r27 |= (persoType & 7) << 1;    //TODO: check me 13 & 7 = ???
		tab += r27;
		_globals->_roomVidNum = *tab++;
		int16 bank = *tab;
		if (bank & 0x8000) {
			bank &= ~0x8000;
			room->_flags |= RoomFlags::rf08;
		}
		room->_flags |= RoomFlags::rf04 | RoomFlags::rf02;
		_globals->_roomImgBank = bank;
		break;
	}
}

// Original name: getsalle
Room *EdenGame::getRoom(int16 loc) { //TODO: byte?
	debug("get room for %X, starting from %d, looking for %X", loc, _globals->_areaPtr->_firstRoomIdx, _globals->_partyOutside);
	Room *room = &_gameRooms[_globals->_areaPtr->_firstRoomIdx];
	loc &= 0xFF;
	for (;; room++) {
		for (; room->_location != loc; room++) {
			if (room->_id == 0xFF)
				return nullptr;
		}
		if (_globals->_partyOutside == room->_party || room->_party == 0xFFFF)
			break;
	}
	debug("found room: party = %X, bank = %X", room->_party, room->_bank);
	_globals->_roomImgBank = room->_bank;
	_globals->_labyrinthRoom = 0;
	if (_globals->_roomImgBank > 104 && _globals->_roomImgBank <= 112)
		_globals->_labyrinthRoom = _globals->_roomImgBank - 103;
	if (_globals->_valleyVidNum)
		_globals->_roomVidNum = _globals->_valleyVidNum;
	else
		_globals->_roomVidNum = room->_video;
	if ((room->_flags & 0xC0) == RoomFlags::rf40 || (room->_flags & RoomFlags::rf01))
		getdino(room);
	if (room->_flags & RoomFlags::rfHasCitadel) {
		removeInfo(_globals->_areaNum + ValleyNews::vnCitadelLost);
		removeInfo(_globals->_areaNum + ValleyNews::vnTyrannIn);
		removeInfo(_globals->_areaNum + ValleyNews::vnTyrannLost);
		removeInfo(_globals->_areaNum + ValleyNews::vnVelociraptorsLost);
	}
	if (istyran(_globals->_roomNum))
		_globals->_gameFlags |= GameFlags::gfFlag10;
	else
		_globals->_gameFlags &= ~GameFlags::gfFlag10;
	return room;
}

// Original name: initlieu
void EdenGame::initPlace(int16 roomNum) {
	_globals->_gameFlags |= GameFlags::gfFlag4000;
	_gameIcons[18]._cursorId |= 0x8000;
	_globals->_lastAreaPtr = _globals->_areaPtr;
	_globals->_areaPtr = &_areasTable[((roomNum >> 8) & 0xFF) - 1];
	Area *area = _globals->_areaPtr;
	area->_visitCount++;
	_globals->_areaVisitCount = area->_visitCount;
	_globals->_curAreaFlags = area->_flags;
	_globals->_curAreaType = area->_type;
	_globals->_curCitadelLevel = area->_citadelLevel;
	if (_globals->_curAreaType == AreaType::atValley)
		_gameIcons[18]._cursorId &= ~0x8000;
	loadPlace(area->_placeNum);
}

void EdenGame::maj2() {
	displayPlace();
	assert(_vm->_screenView->_pitch == 320);
	if (_globals->_roomNum == 273 && _globals->_prevLocation == 18)
		_globals->_mirrorEffect = 1;
	if (_globals->_eventType == EventType::etEventC) {
		drawTopScreen();
		showObjects();
	}
	FRDevents();
	assert(_vm->_screenView->_pitch == 320);
	bool r30 = false;
	if (_globals->_curAreaType == AreaType::atValley && !(_globals->_displayFlags & DisplayFlags::dfPanable))
		r30 = true;
	//TODO: ^^ inlined func?

	if (_globals->_mirrorEffect || _globals->_var103)
		display();
	else if (_globals->_varF1 == (RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01)) {
		_graphics->drawBlackBars();
		_graphics->displayEffect1();
	} else if (_globals->_varF1 && !(_globals->_varF1 & RoomFlags::rf04) && !r30) {
		if (!(_globals->_displayFlags & DisplayFlags::dfPanable))
			_graphics->drawBlackBars();
		else if (_globals->_valleyVidNum)
			_graphics->drawBlackBars();
		_graphics->displayEffect1();
	} else if (r30 && !(_globals->_varF1 & RoomFlags::rf04))
		_graphics->effetpix();
	else
		afficher128();
	musique();
	if (_globals->_eventType != EventType::etEventC) {
		drawTopScreen();
		showObjects();
	}
	_graphics->showBars();
	showEvents();
	_globals->_labyrinthDirections = 0;
	specialin();
}

// Original name: majsalle1
void EdenGame::updateRoom1(int16 roomNum) {
	Room *room = getRoom(roomNum & 0xFF);
	_globals->_roomPtr = room;
	debug("DrawRoom: room 0x%X, arg = 0x%X", _globals->_roomNum, roomNum);
	_globals->_curRoomFlags = room->_flags;
	_globals->_varF1 = room->_flags;
	animpiece();
	_globals->_var100 = 0;
	maj2();
}

// Original name: maj_salle
void EdenGame::updateRoom(uint16 roomNum) {
	setCharacterHere();
	updateRoom1(roomNum);
}

// Original name: initbuf
void EdenGame::allocateBuffers() {
#define ALLOC(ptr, size, typ) if (!((ptr) = (typ*)malloc(size))) _bufferAllocationErrorFl = true;
	ALLOC(_gameRooms, 0x4000, Room);
	ALLOC(_gameIcons, 0x4000, Icon);
	ALLOC(_bankDataBuf, 0x10000, byte);
	ALLOC(_globals, sizeof(*_globals), global_t);
	ALLOC(_placeRawBuf, 2048, byte);
	ALLOC(_gameConditions, 0x4800, byte);
	ALLOC(_gameDialogs, 0x2800, byte);
	ALLOC(_gamePhrases, 0x10000, byte);
	ALLOC(_mainBankBuf, 0x9400, byte);
	ALLOC(_glowBuffer, 0x2800, byte);
	ALLOC(_gameFont, 0x900, byte);
	ALLOC(_gameLipsync, 0x205C, byte);
	ALLOC(_musicBuf, kMaxMusicSize, byte);
#undef ALLOC
}

void EdenGame::freebuf() {
	delete(_bigfileHeader);
	_bigfileHeader = nullptr;

	free(_gameRooms);
	free(_gameIcons);
	free(_bankDataBuf);
	free(_globals);
	free(_placeRawBuf);
	free(_gameConditions);
	free(_gameDialogs);
	free(_gamePhrases);
	free(_mainBankBuf);
	free(_glowBuffer);
	free(_gameFont);
	free(_gameLipsync);
	free(_musicBuf);
}

void EdenGame::EmergencyExit() {
	SysBeep(1);
}

void EdenGame::stopMusic() {
	_musicChannel->stop();
}

void EdenGame::run() {
	_invIconsCount = (_vm->getPlatform() == Common::kPlatformMacintosh) ? 9 : 11;
	_roomIconsBase = _invIconsBase + _invIconsCount;

	word_378CE = 0;
	CRYOLib_ManagersInit();
	_vm->_video->setupSound(11025, false, false);
	_vm->_video->setForceZero2Black(true);
	_vm->_video->setupTimer(12.5);
	_voiceSound = new Sound(0, 11025 * 65536.0, 8, 0);
	_hnmSoundChannel = _vm->_video->getSoundChannel();
	_voiceSound->setWantsDesigned(1); // CHECKME: Used?

	_musicChannel = new CSoundChannel(_vm->_mixer, 11025, false);
	_voiceChannel = new CSoundChannel(_vm->_mixer, 11025, false);
	_graphics = new EdenGraphics(this,_vm->_video);
	_graphics->setSavedUnderSubtitles(false);

	allocateBuffers();
	openbigfile();
	_graphics->openWindow();
	loadpermfiles();

	if (!_bufferAllocationErrorFl) {
		LostEdenMac_InitPrefs();
		if (_vm->getPlatform() == Common::kPlatformMacintosh)
			initCubeMac();
		else
			initCubePC();

		while (!_quitFlag2) {
			initGlobals();
			_quitFlag3 = false;
			_normalCursor = true;
			_torchCursor = false;
			_graphics->setCursKeepPos(-1,-1);
			if (!_gameLoaded)
				intro();
			edmain();
			startmusique(1);
			_graphics->drawBlackBars();
			display();
			_graphics->fadeToBlack(3);
			_graphics->clearScreen();
			_graphics->playHNM(95);
			if (_globals->_endGameFlag == 50) {
				loadrestart();
				_gameLoaded = false;
			}
			fademusica0(2);
			_musicChannel->stop();
			_musicPlayingFlag = false;
			_musicEnabledFlag = false;
		}
		// LostEdenMac_SavePrefs();
	}

	_graphics->fadeToBlack(4);

	delete _voiceChannel;
	delete _musicChannel;
	delete _graphics;

	closebigfile();
	freebuf();
	CRYOLib_ManagersDone();
}

void EdenGame::edmain() {
	//TODO
	enterGame();
	while (!_bufferAllocationErrorFl && !_quitFlag3 && _globals->_endGameFlag != 50) {
		if (!_gameStarted) {
			// if in demo mode, reset game after a while
			_demoCurrentTicks = _vm->_timerTicks;
			if (_demoCurrentTicks - _demoStartTicks > 3000) {
				_graphics->rundcurs();
				display();
				fademusica0(2);
				_graphics->fadeToBlack(3);
				CLBlitter_FillScreenView(0);
				CLBlitter_FillView(_graphics->getMainView(), 0);
				_musicChannel->stop();
				_musicPlayingFlag = false;
				_musicEnabledFlag = false;
				intro();
				enterGame();
			}
		}
		_graphics->rundcurs();
		musicspy();
		FRDevents();
		handleNarrator();
		chronoEvent();
		if (_globals->_drawFlags & DrawFlags::drDrawInventory)
			showObjects();
		if (_globals->_drawFlags & DrawFlags::drDrawTopScreen)
			drawTopScreen();
		if ((_globals->_displayFlags & DisplayFlags::dfPanable) && (_globals->_displayFlags != DisplayFlags::dfPerson))
			scrollPanel();
		if ((_globals->_displayFlags & DisplayFlags::dfMirror) && (_globals->_displayFlags != DisplayFlags::dfPerson))
			scrollMirror();
		if ((_globals->_displayFlags & DisplayFlags::dfFrescoes) && (_globals->_displayFlags != DisplayFlags::dfPerson))
			scrollFrescoes();
		if (_globals->_displayFlags & DisplayFlags::dfFlag2)
			noclicpanel();
		if (_animationActive)
			animCharacter();
		updateCursor();
		display();
	}
}

void EdenGame::intro() {
	if (_vm->getPlatform() == Common::kPlatformMacintosh) {
		// Play intro videos in HQ
		_hnmSoundChannel->stop();
		_vm->_video->closeSound();
		_vm->_video->setupSound(22050, false, true);
		_hnmSoundChannel = _vm->_video->getSoundChannel();
		_graphics->playHNM(2012);
		_graphics->playHNM(171);
		CLBlitter_FillScreenView(0);
		_specialTextMode = false;
		_graphics->playHNM(2001);
		_hnmSoundChannel->stop();
		_vm->_video->closeSound();
		_vm->_video->setupSound(11025, false, false);
		_hnmSoundChannel = _vm->_video->getSoundChannel();
	} else {
		if (_vm->isDemo()) {
			_graphics->playHNM(171);	// Virgin logo
			_graphics->playHNM(98);	// Cryo logo
		}
		else {
			_graphics->playHNM(98);	// Cryo logo
			_graphics->playHNM(171);	// Virgin logo
		}
		CLBlitter_FillScreenView(0);
		_specialTextMode = false;
		startmusique(2);	// INTRO.MUS is played during intro video
		_graphics->playHNM(170);	// Intro video
	}
}

void EdenGame::enterGame() {
	char flag = 0;
	_currentTime = _vm->_timerTicks / 100;
	_globals->_gameTime = _currentTime;
	_demoStartTicks = _vm->_timerTicks;
	_gameStarted = false;
	if (!_gameLoaded) {
		_globals->_roomNum = 279;
		_globals->_areaNum = Areas::arMo;
		_globals->_var100 = 0xFF;
		initPlace(_globals->_roomNum);
		_globals->_currMusicNum = 0;
		startmusique(1);
	} else {
		flag = _globals->_autoDialog;    //TODO
		initafterload();
		byte lastMusicNum = _globals->_currMusicNum;   //TODO: ???
		_globals->_currMusicNum = 0;
		startmusique(lastMusicNum);
		_globals->_inventoryScrollPos = 0;
		_gameStarted = true;
	}
	showObjects();
	drawTopScreen();
	saveFriezes();
	_graphics->setShowBlackBars(true);
	_globals->_mirrorEffect = 1;
	updateRoom(_globals->_roomNum);
	if (flag) {
		_globals->_iconsIndex = 4;
		_globals->_autoDialog = true;
		parle_moi();
	}
}

void EdenGame::signon(const char *s) {
}

void EdenGame::FRDevents() {
	_vm->pollEvents();

	int16 mouseY;
	int16 mouseX;
	_vm->getMousePosition(&mouseX, &mouseY);
	mouseX -= _mouseCenterX;
	mouseY -= _mouseCenterY;
	_vm->setMousePosition(_mouseCenterX , _mouseCenterY);
	_cursorPosX += mouseX;
	_cursorPosX = CLIP<int16>(_cursorPosX, 4, 292);
	_cursorPosY += mouseY;

	int16 maxY = _globals->_displayFlags == DisplayFlags::dfFlag2 ? 190 : 170;
	_cursorPosY = CLIP<int16>(_cursorPosY, 4, maxY);
	_cirsorPanX = _cursorPosX;

	if (_cursorPosY >= 10 && _cursorPosY <= 164 && !(_globals->_displayFlags & DisplayFlags::dfFrescoes))
		_cirsorPanX += _scrollPos;
	if (_normalCursor) {
		_currCursor = 0;
		_currSpot = scan_icon_list(_cirsorPanX + _cursCenter, _cursorPosY + _cursCenter, _globals->_iconsIndex);
		if (_currSpot)
			_currCursor = _currSpot->_cursorId;
	}
	if (_cursCenter == 0 && _currCursor != 53) {
		_cursCenter = 11;
		_cursorPosX -= 11;
	}
	if (_cursCenter == 11 && _currCursor == 53) {
		_cursCenter = 0;
		_cursorPosX += 11;
	}
	if (_globals->_displayFlags & DisplayFlags::dfPanable) {
		//TODO: _currSpot may be zero (due to scan_icon_list failure) if cursor slips between hot areas.
		//fix me here or above?
		if (_currSpot) { // ok, plug it here
			_curSpot2 = _currSpot;
			displayAdamMapMark(_curSpot2->_actionId - 14);
		}
	}
	if (_globals->_displayFlags == DisplayFlags::dfFlag2 && _currSpot)
		_curSpot2 = _currSpot;
	if (_globals->_displayFlags & DisplayFlags::dfFrescoes) {
		if (_frescoTalk)
			_graphics->restoreUnderSubtitles();
		if (_currCursor == 9 && !_torchCursor) {
			_graphics->rundcurs();
			_torchCursor = true;
			_graphics->setGlowX(-1);
		}
		if (_currCursor != 9 && _torchCursor) {
			_graphics->unglow();
			_torchCursor = false;
			_cursorSaved = false;
		}
	}
	if (_vm->isMouseButtonDown()) {
		if (!_mouseHeld) {
			_mouseHeld = true;
			_gameStarted = true;
			mouse();
		}
	} else
		_mouseHeld = false;
	if (_globals->_displayFlags != DisplayFlags::dfFlag2) {
		if (--_inventoryScrollDelay <= 0) {
			if (_globals->_objCount > _invIconsCount && _cursorPosY > 164) {
				if (_cursorPosX > 284 && _globals->_inventoryScrollPos + _invIconsCount < _globals->_objCount) {
					_globals->_inventoryScrollPos++;
					_inventoryScrollDelay = 20;
					showObjects();
				}

				if (_cursorPosX < 30 && _globals->_inventoryScrollPos != 0) {
					_globals->_inventoryScrollPos--;
					_inventoryScrollDelay = 20;
					showObjects();
				}
			}
		}
	}
	if (_inventoryScrollDelay < 0)
		_inventoryScrollDelay = 0;

	if (_vm->shouldQuit())
		edenShudown();
}

Icon *EdenGame::scan_icon_list(int16 x, int16 y, int16 index) {
	for (Icon *icon = &_gameIcons[index]; icon->sx >= 0; icon++) {
		if (icon->_cursorId & 0x8000)
			continue;
#if 0
		// MAC version use this check. Same check is present in PC version, but never used
		// Because of x >= clause two adjacent rooms has 1-pixel wide dead zone between them
		// On valley view screens if cursor slips in this zone a crash in FRDevents occurs
		// due to lack of proper checks
		if (x < icon->ff_0 || x >= icon->ff_4
		        || y < icon->ff_2 || y >= icon->ff_6)
#else
		// PC version has this check inlined in FRDevents
		// Should we keep it or fix edge coordinates in afroom() instead?
		if (x < icon->sx || x > icon->ex
		        || y < icon->sy || y > icon->ey)
#endif
			continue;
		return icon;
	}
	return nullptr;
}

void EdenGame::updateCursor() {
	if (++_torchTick > 3)
		_torchTick = 0;
	if (!_torchTick) {
		_torchCurIndex++;
		_glowIndex++;
	}
	if (_torchCurIndex > 8)
		_torchCurIndex = 0;
	if (_glowIndex > 4)
		_glowIndex = 0;

	if (!_torchCursor) {
		useMainBank();
		_graphics->sundcurs(_cursorPosX + _scrollPos, _cursorPosY);
		if (_currCursor != 53 && _currCursor < 10) { //TODO: cond
			if (_vm->getPlatform() == Common::kPlatformMacintosh)
				engineMac();
			else
				enginePC();
		} else
			_graphics->drawSprite(_currCursor, _cursorPosX + _scrollPos, _cursorPosY);
		_graphics->setGlowX(1);
	} else {
		useBank(117);
		if (_cursorPosX > 294)
			_cursorPosX = 294;
		_graphics->unglow();
		_graphics->glow(_glowIndex);
		_graphics->drawSprite(_torchCurIndex, _cursorPosX + _scrollPos, _cursorPosY);
		if (_frescoTalk)
			_graphics->displaySubtitles();
	}
}

void EdenGame::mouse() {
	static void (EdenGame::*mouse_actions[])() = {
		&EdenGame::actionMoveNorth,
		&EdenGame::actionMoveEast,
		&EdenGame::actionMoveSouth,
		&EdenGame::actionMoveWest,
		&EdenGame::actionPlateMonk,
		&EdenGame::actionGraaFrescoe,
		&EdenGame::actionPushStone,
		&EdenGame::actionSkelettonHead,
		&EdenGame::actionMummyHead,
		&EdenGame::actionMoveNorth,
		&EdenGame::actionKingDialog1,
		&EdenGame::actionKingDialog2,
		&EdenGame::actionKingDialog3,
		&EdenGame::actionGotoHall,
		&EdenGame::actionLabyrinthTurnAround,
		&EdenGame::actionSkelettonMoorkong,
		&EdenGame::actionGotoFullNest,
		&EdenGame::actionLookLake,
		&EdenGame::actionNop,
		&EdenGame::actionNop,
		&EdenGame::actionNop,
		&EdenGame::actionFinal,
		&EdenGame::actionMoveNorth,
		&EdenGame::actionMoveSouth,
		&EdenGame::actionVisit,
		&EdenGame::actionDinoBlow,
		&EdenGame::actionLascFrescoe,
		&EdenGame::actionNop,
		&EdenGame::actionNop,
		&EdenGame::actionNop,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		&EdenGame::actionGotoVal,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		&EdenGame::actionNop,
		&EdenGame::actionNop,
		&EdenGame::actionNop,
		&EdenGame::actionNop,
		&EdenGame::actionNop,
		&EdenGame::actionGetPrism,
		&EdenGame::actionNop,
		&EdenGame::actionNop,
		&EdenGame::actionGetEgg,
		&EdenGame::actionNop,
		&EdenGame::actionNop,
		&EdenGame::actionGetMushroom,
		&EdenGame::actionGetBadMushroom,
		&EdenGame::actionGetKnife,
		&EdenGame::actionGetEmptyNest,
		&EdenGame::actionGetFullNest,
		&EdenGame::actionGetGold,
		nullptr,
		&EdenGame::actionNop,
		&EdenGame::actionGetSunStone,
		&EdenGame::actionGetHorn,
		&EdenGame::actionNop,
		&EdenGame::actionNop,
		&EdenGame::actionNop,
		&EdenGame::actionNop,
		&EdenGame::actionNop,
		&EdenGame::actionNop,
		&EdenGame::actionNop,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		&EdenGame::actionGetTablet,
		&EdenGame::actionClickValleyPlan,
		&EdenGame::actionEndFrescoes,
		&EdenGame::actionChoose,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		&EdenGame::actionKing,
		&EdenGame::actionDina,
		&EdenGame::actionThoo,
		&EdenGame::actionMonk,
		&EdenGame::actionTormentor,
		&EdenGame::actionMessenger,
		&EdenGame::actionMango,
		&EdenGame::actionEve,
		&EdenGame::actionAzia,
		&EdenGame::actionMammi,
		&EdenGame::actionGuards,
		&EdenGame::actionFisher,
		&EdenGame::actionDino,
		&EdenGame::actionTyran,
		&EdenGame::actionMorkus,
		&EdenGame::actionNop,
		&EdenGame::parle_moi,
		&EdenGame::actionAdam,
		&EdenGame::actionTakeObject,
		&EdenGame::putObject,
		&EdenGame::clictimbre,
		&EdenGame::handleDinaDialog,
		&EdenGame::closeCharacterScreen,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		&EdenGame::generique,
		&EdenGame::choseSubtitleOption,
		&EdenGame::edenQuit,
		&EdenGame::restart,
		&EdenGame::cancel2,
		&EdenGame::testvoice,
		&EdenGame::changeVolume,
		&EdenGame::load,
		&EdenGame::save,
		&EdenGame::clickTapeCursor,
		&EdenGame::playtape,
		&EdenGame::stopTape,
		&EdenGame::rewindtape,
		&EdenGame::forwardTape,
		&EdenGame::confirmYes,
		&EdenGame::confirmNo,
		&EdenGame::actionGotoMap
	};

	if (!(_currSpot = scan_icon_list(_cirsorPanX + _cursCenter,
	                                    _cursorPosY + _cursCenter, _globals->_iconsIndex)))
		return;
	_curSpot2 = _currSpot;
	debug("invoking mouse action %d", _currSpot->_actionId);
	if (mouse_actions[_currSpot->_actionId])
		(this->*mouse_actions[_currSpot->_actionId])();
}

////// sound.c
void EdenGame::musique() {
	if (_globals->_newMusicType == MusicType::mtDontChange)
		return;

	Dialog *dial = (Dialog *)getElem(_gameDialogs, 128);
	for (;;dial++) {
		if (dial->_flags == -1 && dial->_condNumLow == -1)
			return;
		byte flag = dial->_flags;
		byte hidx = (dial->_textCondHiMask & 0xC0) >> 6;
		byte lidx = dial->_condNumLow;            //TODO: fixme - unsigned = signed
		if (flag & 0x10)
			hidx |= 4;
		if (testCondition(((hidx << 8) | lidx) & 0x7FF))
			break;
	}
	byte mus = dial->_textNumLow;
	_globals->_newMusicType = MusicType::mtDontChange;
	if (mus != 0 && mus != 2 && mus < 50)
		startmusique(mus);
}

void EdenGame::startmusique(byte num) {
	if (num == _globals->_currMusicNum)
		return;

	if (_musicPlayingFlag) {
		fademusica0(1);
		_musicChannel->stop();
	}
	loadmusicfile(num);
	_globals->_currMusicNum = num;
	_musSequencePtr = _musicBuf + 32;  //TODO: rewrite it properly
	int16 seq_size = READ_LE_UINT16(_musicBuf + 30);
	_musicPatternsPtr = _musicBuf + 30 + seq_size;
	int16 pat_size = READ_LE_UINT16(_musicBuf + 27);
	_musicSamplesPtr = _musicBuf + 32 + 4 + pat_size;
	int16 freq = READ_LE_UINT16(_musicSamplesPtr - 2);

	delete _musicChannel;
	_musicChannel = new CSoundChannel(_vm->_mixer, freq == 166 ? 11025 : 22050, false);
	_musicEnabledFlag = true;

	_musicSequencePos = 0;
	_musicLeftVol = _globals->_prefMusicVol[0];
	_musicRightVol = _globals->_prefMusicVol[1];
	_musicChannel->setVolume(_musicLeftVol, _musicRightVol);
}

void EdenGame::musicspy() {
	if (!_musicEnabledFlag)
		return;
	_musicLeftVol = _globals->_prefMusicVol[0];
	_musicRightVol = _globals->_prefMusicVol[1];
	if (_musicFadeFlag & 3)
		fademusicup();
	if (_personTalking && !_voiceChannel->numQueued())
		_musicFadeFlag = 3;
	if (_musicChannel->numQueued() < 3) {
		byte patnum = _musSequencePtr[(int)_musicSequencePos];
		if (patnum == 0xFF) {
			// rewind
			_musicSequencePos = 0;
			patnum = _musSequencePtr[(int)_musicSequencePos];
		}
		_musicSequencePos++;
		byte *patptr = _musicPatternsPtr + patnum * 6;
		int ofs = patptr[0] + (patptr[1] << 8) + (patptr[2] << 16);
		int len = patptr[3] + (patptr[4] << 8) + (patptr[5] << 16);
		_musicChannel->queueBuffer(_musicSamplesPtr + ofs, len);
		_musicPlayingFlag = true;
	}
}

void EdenGame::persovox() {
	int16 num = _globals->_textNum;
	if (_globals->_textBankIndex != 1)
		num += 565;
	if (_globals->_textBankIndex == 3)
		num += 707;
	_voiceSamplesSize = loadSound(num);
	int16 volumeLeft = _globals->_prefSoundVolume[0];
	int16 volumeRight = _globals->_prefSoundVolume[1];
	int16 stepLeft = _musicChannel->_volumeLeft < volumeLeft ? 1 : -1;
	int16 stepRight = _musicChannel->_volumeRight < volumeRight ? 1 : -1;
	do {
		if (volumeLeft != _musicChannel->_volumeLeft)
			_musicChannel->setVolumeLeft(_musicChannel->_volumeLeft + stepLeft);
		if (volumeRight != _musicChannel->_volumeRight)
			_musicChannel->setVolumeRight(_musicChannel->_volumeRight + stepRight);
	} while (_musicChannel->_volumeLeft != volumeLeft || _musicChannel->_volumeRight != volumeRight);
	volumeLeft = _globals->_prefVoiceVol[0];
	volumeRight = _globals->_prefVoiceVol[1];
	_voiceChannel->setVolume(volumeLeft, volumeRight);
	_voiceChannel->queueBuffer(_voiceSamplesBuffer, _voiceSamplesSize, true);
	_personTalking = true;
	_musicFadeFlag = 0;
	_lastAnimTicks = _vm->_timerTicks;
}

bool EdenGame::personIsTalking() {
	return _personTalking;
}

perso_t *EdenGame::personSubtitles() {
	perso_t *perso = nullptr;
	switch (_globals->_curVideoNum) {
	case 170:
		perso = &_persons[PER_UNKN_156];
		break;
	case 83:
		perso = &_persons[PER_MORKUS];
		break;
	case 88:
		perso = &_persons[PER_MORKUS];
		break;
	case 89:
		perso = &_persons[PER_MORKUS];
		break;
	case 94:
		perso = &_persons[PER_MORKUS];
		break;
	default:
		return perso;
	}
	return perso;
}


// Original name: endpersovox
void EdenGame::endCharacterSpeech() {
	_graphics->restoreUnderSubtitles();
	if (_personTalking) {
		_voiceChannel->stop();
		_personTalking = false;
		_musicFadeFlag = 3;
	}

	if (_soundAllocated) {
		free(_voiceSamplesBuffer);
		_voiceSamplesBuffer = nullptr;
		_soundAllocated = false;
	}
}

void EdenGame::fademusicup() {
	if (_musicFadeFlag & 2) {
		int16 vol = _musicChannel->_volumeLeft;
		if (vol < _musicLeftVol) {
			vol += 8;
			if (vol > _musicLeftVol)
				vol = _musicLeftVol;
		} else {
			vol -= 8;
			if (vol < _musicLeftVol)
				vol = _musicLeftVol;
		}
		_musicChannel->setVolumeLeft(vol);
		if (vol == _musicLeftVol)
			_musicFadeFlag &= ~2;
	}
	if (_musicFadeFlag & 1) {
		int16 vol = _musicChannel->_volumeRight;
		if (vol < _musicRightVol) {
			vol += 8;
			if (vol > _musicRightVol)
				vol = _musicRightVol;
		} else {
			vol -= 8;
			if (vol < _musicRightVol)
				vol = _musicRightVol;
		}
		_musicChannel->setVolumeRight(vol);
		if (vol == _musicRightVol)
			_musicFadeFlag &= ~1;
	}
}

void EdenGame::fademusica0(int16 delay) {
	int16 volume;
	while ((volume = _musicChannel->getVolume()) > 2) {
		volume -= 2;
		if (volume < 2)
			volume = 2;
		_musicChannel->setVolume(volume, volume);
		wait(delay);
	}
}

//// obj.c

// Original name: getobjaddr
object_t *EdenGame::getObjectPtr(int16 id) {
	int i;
	for (i = 0; i < MAX_OBJECTS; i++) {
		if (_objects[i]._id == id)
			break;
	}

	return &_objects[i];
}

void EdenGame::countObjects() {
	int16 index = 0;
	byte total = 0;
	for (int i = 0; i < MAX_OBJECTS; i++) {
		int16 count = _objects[i]._count;
		if (count == 0)
			continue;

		if (_objects[i]._flags & ObjectFlags::ofInHands)
			count--;

		if (count) {
			total += count;
			while (count--)
				_ownObjects[index++] = _objects[i]._id;
		}
	}
	_globals->_objCount = total;
}

void EdenGame::showObjects() {
	Icon *icon = &_gameIcons[_invIconsBase];
	_globals->_drawFlags &= ~(DrawFlags::drDrawInventory | DrawFlags::drDrawFlag2);
	countObjects();
	int16 total = _globals->_objCount;
	for (int16 i = _invIconsCount; i--; icon++) {
		if (total) {
			icon->_cursorId &= ~0x8000;
			total--;
		} else
			icon->_cursorId |= 0x8000;
	}
	useMainBank();
	_graphics->drawSprite(55, 0, 176);
	icon = &_gameIcons[_invIconsBase];
	total = _globals->_objCount;
	int16 index = _globals->_inventoryScrollPos;
	for (int16 i = _invIconsCount; total-- && i--; icon++) {
		char obj = _ownObjects[index++];
		icon->_objectId = obj;
		_graphics->drawSprite(obj + 9, icon->sx, 178);
	}
	_paletteUpdateRequired = true;
	if ((_globals->_displayFlags & DisplayFlags::dfMirror) || (_globals->_displayFlags & DisplayFlags::dfPanable)) {
		_graphics->saveBottomFrieze();
		scroll();
	}
}


byte * EdenGame::getGlowBuffer() {
	return _glowBuffer;
}

void EdenGame::setMusicFade(byte value) {
	_musicFadeFlag = value;
}



void EdenGame::winObject(int16 id) {
	object_t *object = getObjectPtr(id);
	object->_flags |= ObjectFlags::ofFlag1;
	object->_count++;
	_globals->_curItemsMask |= object->_itemMask;
	_globals->_wonItemsMask |= object->_itemMask;
	_globals->_curPowersMask |= object->_powerMask;
	_globals->_wonPowersMask |= object->_powerMask;
}

void EdenGame::loseObject(int16 id) {
	object_t *object = getObjectPtr(id);
	if (object->_count > 0)
		object->_count--;
	if (!object->_count) {
		object->_flags &= ~ObjectFlags::ofFlag1;
		_globals->_curItemsMask &= ~object->_itemMask;
		_globals->_curPowersMask &= ~object->_powerMask;
	}
	_globals->_curObjectId = 0;
	_globals->_curObjectFlags = 0;
	_globals->_curObjectCursor = 9;
	_gameIcons[16]._cursorId |= 0x8000;
	object->_flags &= ~ObjectFlags::ofInHands;
	_normalCursor = true;
	_currCursor = 0;
	_torchCursor = false;
}

void EdenGame::lostObject() {
	parlemoiNormalFlag = true;
	if (_globals->_curObjectId)
		loseObject(_globals->_curObjectId);
}

// Original name: objecthere
bool EdenGame::isObjectHere(int16 id) {
	object_t *object = getObjectPtr(id);
	for (_currentObjectLocation = &_objectLocations[object->_locations]; *_currentObjectLocation != 0xFFFF; _currentObjectLocation++) {
		if (*_currentObjectLocation == _globals->_roomNum)
			return true;
	}
	return false;
}

void EdenGame::objectmain(int16 id) {
	object_t *object = getObjectPtr(id);
	_gameIcons[16]._cursorId &= ~0x8000;
	_globals->_curObjectId = object->_id;
	_globals->_curObjectCursor = _globals->_curObjectId + 9;
	object->_flags |= ObjectFlags::ofInHands;
	_globals->_curObjectFlags = object->_flags;
	_currCursor = _globals->_curObjectId + 9;
	_normalCursor = false;
}

void EdenGame::getObject(int16 id) {
	Room *room = _globals->_roomPtr;
	if (_globals->_curObjectId)
		return;
	if (!isObjectHere(id))
		return;
	*_currentObjectLocation |= 0x8000;
	objectmain(id);
	winObject(id);
	showObjects();
	_globals->_roomImgBank = room->_bank;
	_globals->_roomVidNum = room->_video;
	displayPlace();
}

void EdenGame::putObject() {
	if (!_globals->_curObjectId)
		return;
	_gameIcons[16]._cursorId |= 0x8000;
	object_t *object = getObjectPtr(_globals->_curObjectId);
	_globals->_curObjectCursor = 9;
	_globals->_curObjectId = 0;
	_globals->_curObjectFlags = 0;
	object->_flags &= ~ObjectFlags::ofInHands;
	_globals->_nextDialogPtr = nullptr;
	_closeCharacterDialog = false;
	_globals->_dialogType = DialogType::dtTalk;
	showObjects();
	_normalCursor = true;
}

void EdenGame::newObject(int16 id, int16 arg2) {
	object_t *object = getObjectPtr(id);
	uint16 e, *t = &_objectLocations[object->_locations];
	while ((e = *t) != 0xFFFF) {
		e &= ~0x8000;
		if ((e >> 8) == arg2)
			*t = e;
		t++;
	}
}

void EdenGame::giveobjectal(int16 id) {
	if (id == Objects::obKnife)
		_objectLocations[2] = 0;
	if (id == Objects::obApple)
		_globals->_stepsToFindAppleNormal = 0;
	if (id >= Objects::obEyeInTheStorm && id < (Objects::obRiverThatWinds + 1) && _globals->_roomCharacterType == PersonFlags::pftVelociraptor) {
		//TODO: fix that cond above
		object_t *object = getObjectPtr(id);
		_globals->_roomCharacterPtr->_powers &= ~object->_powerMask;
	}
	winObject(id);
}

void EdenGame::giveObject() {
	byte id = _globals->_giveObj1;
	if (id) {
		_globals->_giveObj1 = 0;
		giveobjectal(id);
	}
	id = _globals->_giveObj2;
	if (id) {
		_globals->_giveObj2 = 0;
		giveobjectal(id);
	}
	id = _globals->_giveObj3;
	if (id) {
		_globals->_giveObj3 = 0;
		giveobjectal(id);
	}
}

// Original name: takeObject
void EdenGame::actionTakeObject() {
	objectmain(_curSpot2->_objectId);
	_globals->_nextDialogPtr = nullptr;
	_closeCharacterDialog = false;
	_globals->_dialogType = DialogType::dtTalk;
	if (_globals->_inventoryScrollPos)
		_globals->_inventoryScrollPos--;
	showObjects();
}
////

// Original name: newchampi
void EdenGame::newMushroom() {
	if (_objects[Objects::obShroom - 1]._count == 0) {
		newObject(Objects::obShroom, _globals->_citadelAreaNum);
		newObject(Objects::obBadShroom, _globals->_citadelAreaNum);
	}
}

// Original name: newnidv
void EdenGame::newEmptyNest() {
	Room *room = _globals->_citaAreaFirstRoom;
	if (_objects[Objects::obNest - 1]._count)
		return;
	object_t *obj = getObjectPtr(Objects::obNest);
	for (uint16 *ptr = _objectLocations + obj->_locations; *ptr != 0xFFFF; ptr++) {
		if ((*ptr & ~0x8000) >> 8 != _globals->_citadelAreaNum)
			continue;
		*ptr &= ~0x8000;
		for (; room->_id != 0xFF; room++) {
			if (room->_location == (*ptr & 0xFF)) {
				room->_bank = 279;
				room->_id = 9;
				room++;
				room->_bank = 280;
				return;
			}
		}
	}
}

// Original name: newnido
void EdenGame::newNestWithEggs() {
	Room *room = _globals->_citaAreaFirstRoom;
	if (_objects[Objects::obFullNest - 1]._count)
		return;
	if (_objects[Objects::obNest - 1]._count)
		return;
	object_t *obj = getObjectPtr(Objects::obFullNest);
	for (uint16 *ptr = _objectLocations + obj->_locations; *ptr != 0xFFFF; ptr++) {
		if ((*ptr & ~0x8000) >> 8 != _globals->_citadelAreaNum)
			continue;
		*ptr &= ~0x8000;
		for (; room->_id != 0xFF; room++) {
			if (room->_location == (*ptr & 0xFF)) {
				room->_bank = 277;
				room->_id = 9;
				room++;
				room->_bank = 278;
				return;
			}
		}
	}
}

// Original name: newor
void EdenGame::newGold() {
	if (_objects[Objects::obGold - 1]._count == 0)
		newObject(Objects::obGold, _globals->_citadelAreaNum);
}

void EdenGame::gotoPanel() {
	if (_vm->shouldQuit())
		byte_31D64 = _globals->_autoDialog;  //TODO: check me
	_noPalette = false;
	_globals->_iconsIndex = 85;
	_globals->_characterPtr = nullptr;
	_globals->_drawFlags |= DrawFlags::drDrawMenu;
	_globals->_displayFlags = DisplayFlags::dfFlag2;
	_globals->_menuFlags = 0;
	displayPanel();
	_graphics->fadeToBlack(3);
	displayTopPanel();
	CLBlitter_CopyView2Screen(_graphics->getMainView());
	_graphics->SendPalette2Screen(256);
	_cursorPosX = 320 / 2;
	_cursorPosY = 200 / 2;
	_vm->setMousePosition(_mouseCenterX, _mouseCenterY);
}

void EdenGame::setMouseCenterX(uint16 xpos) {
	_mouseCenterX = xpos;
}

void EdenGame::setMouseCenterY(uint16 ypos) {
	_mouseCenterY = ypos;
}

uint16 EdenGame::getMouseCenterX() {
	return _mouseCenterX;
}

uint16 EdenGame::getMouseCenterY() {
	return _mouseCenterY;
}

void EdenGame::noclicpanel() {
	if (_globals->_menuFlags & MenuFlags::mfFlag4) {
		moveTapeCursor();
		return;
	}
	if (_globals->_drawFlags & DrawFlags::drDrawFlag8)
		return;
	if (_globals->_menuFlags & MenuFlags::mfFlag1) {
		changervol();
		return;
	}
	byte num;
	if (_curSpot2 >= &_gameIcons[119]) {
		debug("noclic: objid = %p, glob3,2 = %2X %2X", (void *)_curSpot2, _globals->_menuItemIdHi, _globals->_menuItemIdLo);
		if (_curSpot2->_objectId == (uint16)((_globals->_menuItemIdLo + _globals->_menuItemIdHi) << 8)) //TODO: check me
			return;
	} else {
		int idx = _curSpot2 - &_gameIcons[105];
		if (idx == 0) {
			_globals->_menuItemIdLo = 1;
			num = 1;
			goto skip;
		}
		num = (idx & 0x7F) + 1;
		if (num >= 5)
			num = 1;
		if (num == _globals->_var43)
			return;
		_globals->_var43 = 0;
	}
	num = _globals->_menuItemIdLo;
	_globals->_menuItemIdLo = _curSpot2->_objectId & 0xFF;
skip:
	;
	_globals->_menuItemIdHi = (_curSpot2->_objectId & 0xFF00) >> 8;
	debug("noclic: new glob3,2 = %2X %2X", _globals->_menuItemIdHi, _globals->_menuItemIdLo);
	displayResult();
	num &= 0xF0;
	if (num != 0x30)
		num = _globals->_menuItemIdLo & 0xF0;
	if (num == 0x30)
		displayCursors();
}

void EdenGame::generique() {
	_graphics->drawBlackBars();
	display();
	_graphics->fadeToBlack(3);
	_graphics->clearScreen();
	int oldmusic = _globals->_currMusicNum;
	_graphics->playHNM(95);
	displayPanel();
	displayTopPanel();
	_paletteUpdateRequired = true;
	startmusique(oldmusic);
}

void EdenGame::cancel2() {
	drawTopScreen();
	showObjects();
	_globals->_iconsIndex = 16;
	_globals->_drawFlags &= ~DrawFlags::drDrawMenu;
	gameToMirror(1);
}

byte *EdenGame::getCurKeepBuf() {
	return _cursKeepBuf;
}

bool EdenGame::isMouseHeld() {
	return _mouseHeld;
}

void EdenGame::setMouseHeld() {
	_mouseHeld = true;
}

void EdenGame::setMouseNotHeld() {
	_mouseHeld = false;
}

void EdenGame::testvoice() {
	_globals->_frescoNumber = 0;
	_globals->_characterPtr = _persons;
	_globals->_dialogType = DialogType::dtInspect;
	int16 num = (_persons[PER_KING]._id << 3) | _globals->_dialogType;
	dialoscansvmas((Dialog *)getElem(_gameDialogs, num));
	_graphics->restoreUnderSubtitles();
	_graphics->displaySubtitles();
	persovox();
	waitEndSpeak();
	endCharacterSpeech();
	_globals->_varCA = 0;
	_globals->_dialogType = DialogType::dtTalk;
}

void EdenGame::load() {
	char name[132];
	_gameLoaded = false;
	byte oldMusic = _globals->_currMusicNum;   //TODO: from uint16 to byte?!
	fademusica0(1);
	desktopcolors();
	FlushEvents(-1, 0);
//	if(OpenDialog(0, 0)) //TODO: write me
	{
		// TODO
		strcpy(name, "edsave1.000");
		loadgame(name);
	}
	_vm->hideMouse();
	CLBlitter_FillScreenView(0xFFFFFFFF);
	_graphics->fadeToBlack(3);
	CLBlitter_FillScreenView(0);
	if (!_gameLoaded) {
		_musicFadeFlag = 3;
		musicspy();
		_paletteUpdateRequired = true;
		return;
	}
	if ((oldMusic & 0xFF) != _globals->_currMusicNum) { //TODO: r30 is uns char/bug???
		oldMusic = _globals->_currMusicNum;
		_globals->_currMusicNum = 0;
		startmusique(oldMusic);
	} else {
		_musicFadeFlag = 3;
		musicspy();
	}
	bool talk = _globals->_autoDialog;    //TODO check me
	initafterload();
	_graphics->fadeToBlack(3);
	CLBlitter_FillScreenView(0);
	CLBlitter_FillView(_graphics->getMainView(), 0);
	drawTopScreen();
	_globals->_inventoryScrollPos = 0;
	showObjects();
	updateRoom(_globals->_roomNum);
	if (talk) {
		_globals->_iconsIndex = 4;
		_globals->_autoDialog = true;
		parle_moi();
	}

}

void EdenGame::initafterload() {
	_globals->_characterImageBank = 0;
	_globals->_lastPlaceNum = 0;
	loadPlace(_globals->_areaPtr->_placeNum);
	_gameIcons[18]._cursorId |= 0x8000;
	if (_globals->_curAreaType == AreaType::atValley)
		_gameIcons[18]._cursorId &= ~0x8000;
	_personRoomBankTable[30] = 27;
	if (_globals->_phaseNum >= 352)
		_personRoomBankTable[30] = 26;
	_animateTalking = false;
	_animationActive = false;
	_globals->_var100 = 0;
	_globals->_eventType = EventType::etEventC;
	_globals->_valleyVidNum = 0;
	_globals->_drawFlags &= ~DrawFlags::drDrawMenu;
	_currentTime = _vm->_timerTicks / 100;
	_globals->_gameTime = _currentTime;
	if (_globals->_roomCharacterType == PersonFlags::pftTyrann)
		setChrono(3000);
	_adamMapMarkPos.x = -1;
	_adamMapMarkPos.y = -1;
}

void EdenGame::save() {
	char name[260];
	fademusica0(1);
	desktopcolors();
	FlushEvents(-1, 0);
	//SaveDialog(byte_37150, byte_37196->ff_A);
	//TODO
	strcpy(name, "edsave1.000");
	saveGame(name);
	_vm->hideMouse();
	CLBlitter_FillScreenView(0xFFFFFFFF);
	_graphics->fadeToBlack(3);
	CLBlitter_FillScreenView(0);
	_musicFadeFlag = 3;
	musicspy();
	_paletteUpdateRequired = true;
}

void EdenGame::desktopcolors() {
	_graphics->fadeToBlack(3);
	CLBlitter_FillScreenView(0xFFFFFFFF);
	CLPalette_BeSystem();
	_vm->showMouse();
}

void EdenGame::panelrestart() {
	_gameLoaded = false;
	byte curmus = _globals->_currMusicNum;
	byte curlng = _globals->_prefLanguage;
	loadrestart();
	_globals->_prefLanguage = curlng;
	if (!_gameLoaded) //TODO always?
		return;
	_globals->_characterImageBank = 0;
	_globals->_lastPlaceNum = 0;
	loadPlace(_globals->_areaPtr->_placeNum);
	_globals->_displayFlags = DisplayFlags::dfFlag1;
	_gameIcons[18]._cursorId |= 0x8000;
	if (_globals->_curAreaType == AreaType::atValley)
		_gameIcons[18]._cursorId &= ~0x8000;
	_personRoomBankTable[30] = 27;
	if (_globals->_phaseNum >= 352)
		_personRoomBankTable[30] = 26;
	_animateTalking = false;
	_animationActive = false;
	_globals->_var100 = 0;
	_globals->_eventType = 0;
	_globals->_valleyVidNum = 0;
	_globals->_drawFlags &= ~DrawFlags::drDrawMenu;
	_globals->_inventoryScrollPos = 0;
	_adamMapMarkPos.x = -1;
	_adamMapMarkPos.y = -1;
	if (curmus != _globals->_currMusicNum) {
		curmus = _globals->_currMusicNum;
		_globals->_currMusicNum = 0;
		startmusique(curmus);
	}
	_graphics->fadeToBlack(3);
	CLBlitter_FillScreenView(0);
	CLBlitter_FillView(_graphics->getMainView(), 0);
	drawTopScreen();
	showObjects();
	saveFriezes();
	_graphics->setShowBlackBars(true);
	updateRoom(_globals->_roomNum);
}

void EdenGame::reallyquit() {
	_quitFlag3 = true;
	_quitFlag2 = true;
}

void EdenGame::confirmer(char mode, char yesId) {
	_globals->_iconsIndex = 119;
	_gameIcons[119]._objectId = yesId;
	_confirmMode = mode;
	useBank(65);
	_graphics->drawSprite(12, 117, 74);
	_cursorPosX = 156;
	if (_vm->shouldQuit())
		_cursorPosX = 136;
	_cursorPosY = 88;
}

void EdenGame::confirmYes() {
	displayPanel();
	_globals->_iconsIndex = 85;
	switch (_confirmMode) {
	case 1:
		panelrestart();
		break;
	case 2:
		reallyquit();
		break;
	}
}

void EdenGame::confirmNo() {
	displayPanel();
	_globals->_iconsIndex = 85;
//	pomme_q = false;
}

void EdenGame::restart() {
	confirmer(1, _curSpot2->_objectId);
}

void EdenGame::edenQuit() {
	confirmer(2, _curSpot2->_objectId);
}

// Original name: choixsubtitle
void EdenGame::choseSubtitleOption() {
	byte lang = _curSpot2->_objectId & 0xF;
	if (lang == _globals->_prefLanguage)
		return;
	if (lang > 5)
		return;
	_globals->_prefLanguage = lang;
	_graphics->langbuftopanel();
	displayLanguage();
}

// Original name: reglervol
void EdenGame::changeVolume() {
	byte *valptr = &_globals->_prefMusicVol[_curSpot2->_objectId & 7];
	_cursorPosY = 104 - ((*valptr >> 2) & 0x3F); // TODO: check me
	_curSliderValuePtr = valptr;
	_globals->_menuFlags |= MenuFlags::mfFlag1;
	if (_curSpot2->_objectId & 8)
		_globals->_menuFlags |= MenuFlags::mfFlag2;
	_curSliderX = _curSpot2->sx;
	_curSliderY = _cursorPosY;
}

void EdenGame::changervol() {
	if (_mouseHeld) {
		restrictCursorArea(_curSliderX - 1, _curSliderX + 3, 40, 110);
		int16 delta = _curSliderY - _cursorPosY;
		if (delta == 0)
			return;
		newvol(_curSliderValuePtr, delta);
		if (_globals->_menuFlags & MenuFlags::mfFlag2)
			newvol(_curSliderValuePtr + 1, delta);
		_graphics->cursbuftopanel();
		displayCursors();
		_curSliderY = _cursorPosY;
	} else
		_globals->_menuFlags &= ~(MenuFlags::mfFlag1 | MenuFlags::mfFlag2);
}

void EdenGame::newvol(byte *volptr, int16 delta) {
	int16 vol = *volptr / 4;
	vol += delta;
	if (vol < 0)
		vol = 0;
	if (vol > 63)
		vol = 63;
	*volptr = vol * 4;
	_musicChannel->setVolume(_globals->_prefMusicVol[0], _globals->_prefMusicVol[1]);
}

void EdenGame::playtape() {
	if (_globals->_menuItemIdHi & 8)
		_globals->_tapePtr++;
	for (;; _globals->_tapePtr++) {
		if (_globals->_tapePtr == &_tapes[MAX_TAPES]) {
			_globals->_tapePtr--;
			stopTape();
			return;
		}
		if (_globals->_tapePtr->_textNum)
			break;
	}
	_globals->_menuFlags |= MenuFlags::mfFlag8;
	_globals->_drawFlags &= ~DrawFlags::drDrawMenu;
	uint16 oldRoomNum = _globals->_roomNum;
	uint16 oldParty = _globals->_party;
	byte oldBack = _globals->_roomBackgroundBankNum;
	perso_t *oldPerso = _globals->_characterPtr;
	_globals->_party = _globals->_tapePtr->_party;
	_globals->_roomNum = _globals->_tapePtr->_roomNum;
	_globals->_roomBackgroundBankNum = _globals->_tapePtr->_backgroundBankNum;
	_globals->_dialogPtr = _globals->_tapePtr->_dialog;
	_globals->_characterPtr = _globals->_tapePtr->_perso;
	endCharacterSpeech();
	displayTapeCursor();
	if (_globals->_characterPtr != oldPerso
	        || _globals->_roomNum != _lastTapeRoomNum) {
		_lastTapeRoomNum = _globals->_roomNum;
		_globals->_curCharacterAnimPtr = nullptr;
		_globals->_varCA = 0;
		_globals->_characterImageBank = -1;
		AnimEndCharacter();
		loadCurrCharacter();
	}
	displayCharacterBackground();
	_globals->_textNum = _globals->_tapePtr->_textNum;
	my_bulle();
	getDataSync();
	displayCharacterPanel();
	persovox();
	_globals->_roomBackgroundBankNum = oldBack;
	_globals->_party = oldParty;
	_globals->_roomNum = oldRoomNum;
}

void EdenGame::rewindtape() {
	if (_globals->_tapePtr > _tapes) {
		_globals->_tapePtr--;
		_globals->_menuFlags &= ~MenuFlags::mfFlag8;
		displayTapeCursor();
	}
}

// Original name: depcurstape
void EdenGame::moveTapeCursor() {
	if (_mouseHeld) {
		restrictCursorArea(95, 217, 179, 183);
		int idx = (_cursorPosX - 97);
		if (idx < 0)
			idx = 0;

		idx /= 8;
		tape_t *tape = _tapes + idx;
		if (tape >= _tapes + 16)
			tape = _tapes + 16 - 1;

		if (tape != _globals->_tapePtr) {
			_globals->_tapePtr = tape;
			displayTapeCursor();
			_globals->_menuFlags &= ~MenuFlags::mfFlag8;
		}
	} else
		_globals->_menuFlags &= ~MenuFlags::mfFlag4;
}

void EdenGame::setCursorSaved(bool cursorSaved) {
	_cursorSaved = cursorSaved;
}

bool EdenGame::getCursorSaved() {
	return _cursorSaved;
}

bool EdenGame::getNoPalette() {
	return _noPalette;
}

// Original name: affcurstape
void EdenGame::displayTapeCursor() {
	if (_globals->_drawFlags & DrawFlags::drDrawFlag8)
		_noPalette = true;
	useBank(65);
	_graphics->drawSprite(2, 0, 176);
	int x = (_globals->_tapePtr - _tapes) * 8 + 97;
	_gameIcons[112].sx = x - 3;
	_gameIcons[112].ex = x + 3;
	_graphics->drawSprite(5, x, 179);
	_noPalette = false;
}

void EdenGame::forwardTape() {
	if (_globals->_tapePtr < _tapes + 16) {
		_globals->_tapePtr++;
		_globals->_menuFlags &= ~MenuFlags::mfFlag8;
		displayTapeCursor();
	}
}

void EdenGame::stopTape() {
	if (!(_globals->_drawFlags & DrawFlags::drDrawFlag8))
		return;
	_globals->_menuFlags &= ~MenuFlags::mfFlag8;
	_globals->_drawFlags &= ~DrawFlags::drDrawFlag8;
	_globals->_menuFlags |= MenuFlags::mfFlag10;
	_globals->_iconsIndex = 85;
	_globals->_characterPtr = nullptr;
	_lastTapeRoomNum = 0;
	endCharacterSpeech();
	fin_perso();
	displayPanel();
	displayTopPanel();
	_paletteUpdateRequired = true;
}

void EdenGame::clickTapeCursor() {
	_globals->_menuFlags |= MenuFlags::mfFlag4;
}

// Original name: affpanel
void EdenGame::displayPanel() {
	useBank(65);
	_graphics->drawSprite(0, 0, 16);
	_graphics->paneltobuf();
	displayLanguage();
	displayCursors();
	displayTapeCursor();
}

// Original name: afflangue
void EdenGame::displayLanguage() {
	useBank(65);
	if (_globals->_prefLanguage > 5)
		return;
	_graphics->drawSprite(6,  8, _globals->_prefLanguage * 9 + 43);  //TODO: * FONT_HEIGHT
	_graphics->drawSprite(7, 77, _globals->_prefLanguage * 9 + 44);
}

// Original name: affcursvol
void EdenGame::displayVolCursor(int16 x, int16 vol1, int16 vol2) {
	int16 slider = 3;
	if (_lastMenuItemIdLo && (_lastMenuItemIdLo & 9) != 1) //TODO check me
		slider = 4;
	_graphics->drawSprite(slider, x, 104 - vol1);
	slider = 3;
	if ((_lastMenuItemIdLo & 9) != 0)
		slider = 4;
	_graphics->drawSprite(slider, x + 12, 104 - vol2);
}

// Original name: affcurseurs
void EdenGame::displayCursors() {
	useBank(65);
	if (_globals->_drawFlags & DrawFlags::drDrawFlag8)
		return;
	selectCursor(48);
	displayVolCursor(114, _globals->_prefMusicVol[0] / 4, _globals->_prefMusicVol[1] / 4);
	selectCursor(50);
	displayVolCursor(147, _globals->_prefVoiceVol[0] / 4, _globals->_prefVoiceVol[1] / 4);
	selectCursor(52);
	displayVolCursor(179, _globals->_prefSoundVolume[0] / 4, _globals->_prefSoundVolume[1] / 4);
}

// Original name: curseurselect
void EdenGame::selectCursor(int itemId) {
	_lastMenuItemIdLo = _globals->_menuItemIdLo;
	if ((_lastMenuItemIdLo & ~9) != itemId)
		_lastMenuItemIdLo = 0;
}

// Original name: afftoppano
void EdenGame::displayTopPanel() {
	_graphics->drawSprite(1, 0, 0);
}

// Original name: affresult
void EdenGame::displayResult() {
	_graphics->restoreUnderSubtitles();
	_globals->_characterPtr = &_persons[19];
	_globals->_dialogType = DialogType::dtInspect;
	int16 num = (_persons[PER_UNKN_156]._id << 3) | _globals->_dialogType;
	if (dialoscansvmas((Dialog *)getElem(_gameDialogs, num)))
		_graphics->displaySubtitles();
	_globals->_varCA = 0;
	_globals->_dialogType = DialogType::dtTalk;
	_globals->_characterPtr = nullptr;
}

// Original name: limitezonecurs
void EdenGame::restrictCursorArea(int16 xmin, int16 xmax, int16 ymin, int16 ymax) {
	_cursorPosX = CLIP(_cursorPosX, xmin, xmax);
	_cursorPosY = CLIP(_cursorPosY, ymin, ymax);
}

// Original name: PommeQ
void EdenGame::edenShudown() {
	Icon *icon = &_gameIcons[85];
	if (_globals->_displayFlags & DisplayFlags::dfFrescoes) {
		_torchCursor = false;
		_cursorSaved = true;
		if (_globals->_displayFlags & DisplayFlags::dfPerson)
			closeCharacterScreen();
		_globals->_displayFlags = DisplayFlags::dfFlag1;
		resetScroll();
		_globals->_var100 = 0xFF;
		updateRoom(_globals->_roomNum);
	}
	if (_globals->_displayFlags & DisplayFlags::dfPerson)
		closeCharacterScreen();
	if (_globals->_displayFlags & DisplayFlags::dfPanable)
		resetScroll();
	if (_globals->_displayFlags & DisplayFlags::dfMirror)
		resetScroll();
	if (_globals->_drawFlags & DrawFlags::drDrawFlag8)
		stopTape();
	if (_personTalking)
		endCharacterSpeech();
	_globals->_var103 = 0;
	_globals->_mirrorEffect = 0;
	putObject();
	_currCursor = 53;
	if (_globals->_displayFlags != DisplayFlags::dfFlag2)
		gotoPanel();
	_curSpot2 = icon + 7;   //TODO
	edenQuit();
}

void EdenGame::habitants(perso_t *perso) {
	char persType = perso->_flags & PersonFlags::pfTypeMask; //TODO rename
	if (persType && persType != PersonFlags::pfType2) {
		_globals->_roomCharacterPtr = perso;
		_globals->_roomCharacterType = persType;
		_globals->_roomCharacterFlags = perso->_flags;
		_globals->_roomPersoItems = perso->_items;
		_globals->_roomCharacterPowers = perso->_powers;
		_globals->_partyOutside |= perso->_partyMask;
		if (_globals->_roomCharacterType == PersonFlags::pftTriceraptor)
			removeInfo(_globals->_areaNum + ValleyNews::vnTriceraptorsIn);
		else if (_globals->_roomCharacterType == PersonFlags::pftVelociraptor)
			removeInfo(_globals->_areaNum + ValleyNews::vnVelociraptorsIn);
	} else if (!(perso->_flags & PersonFlags::pfInParty))
		_globals->_partyOutside |= perso->_partyMask;
}

void EdenGame::suiveurs(perso_t *perso) {
	char persType = perso->_flags & PersonFlags::pfTypeMask;
	if (persType == 0 || persType == PersonFlags::pfType2) {
		if (perso->_flags & PersonFlags::pfInParty)
			_globals->_party |= perso->_partyMask;
	}
}

void EdenGame::evenements(perso_t *perso) {
	if (_globals->_var113)
		return;

	if (perso >= &_persons[PER_UNKN_18C])
		return;

	if (!dialogEvent(perso))
		return;

	_globals->_var113++;
	_globals->_oldDisplayFlags = 1;
	perso = _globals->_characterPtr;
	initCharacterPointers(perso);
	if (!(perso->_partyMask & PersonMask::pmLeader))
		_globals->_var60 = 1;
	_globals->_eventType = 0;
}

void EdenGame::followme(perso_t *perso) {
	if (perso->_flags & PersonFlags::pfTypeMask)
		return;
	if (perso->_flags & PersonFlags::pfInParty)
		perso->_roomNum = _destinationRoom;
}

void EdenGame::rangermammi(perso_t *perso, Room *room) {
	Room *found_room = nullptr;
	if (!(perso->_partyMask & PersonMask::pmLeader))
		return;
	for (; room->_id != 0xFF; room++) {
		if (room->_flags & RoomFlags::rfHasCitadel) {
			found_room = room;
			break;
		}
		if (room->_party != 0xFFFF && (room->_party & PersonMask::pmLeader))
			found_room = room;  //TODO: no brk?
	}
	if (!found_room)
		return;
	perso->_roomNum &= ~0xFF;
	perso->_roomNum |= found_room->_location;
	perso->_flags &= ~PersonFlags::pfInParty;
	_globals->_party &= ~perso->_partyMask;
}

void EdenGame::perso_ici(int16 action) {
	perso_t *perso = &_persons[PER_UNKN_156];
//	room_t *room = p_global->last_area_ptr->room_ptr;    //TODO: compiler opt bug? causes access to zero ptr??? last_area_ptr == 0
	switch (action) {
	case 0:
		suiveurs(perso);
		break;
	case 1:
		habitants(perso);
		break;
	case 3:
		evenements(perso);
		break;
	case 4:
		followme(perso);
		break;
	case 5:
		rangermammi(perso, _globals->_lastAreaPtr->_citadelRoomPtr);
		break;
	}
	perso = _persons;
	do {
		if (perso->_roomNum == _globals->_roomNum && !(perso->_flags & PersonFlags::pf80)) {
			switch (action) {
			case 0:
				suiveurs(perso);
				break;
			case 1:
				habitants(perso);
				break;
			case 3:
				evenements(perso);
				break;
			case 4:
				followme(perso);
				break;
			case 5:
				rangermammi(perso, _globals->_lastAreaPtr->_citadelRoomPtr);
				break;
			}
		}
		perso++;
	} while (perso->_roomNum != 0xFFFF);
}

// Original name: setpersohere
void EdenGame::setCharacterHere() {
	debug("setCharacterHere, perso is %d", (int)(_globals->_characterPtr - _persons));
	_globals->_partyOutside = 0;
	_globals->_party = 0;
	_globals->_roomCharacterPtr = nullptr;
	_globals->_roomCharacterType = 0;
	_globals->_roomCharacterFlags = 0;
	perso_ici(1);
	perso_ici(0);
	if (_globals->_roomCharacterType == PersonFlags::pftTyrann)
		removeInfo(_globals->_areaNum + ValleyNews::vnTyrannIn);
	if (_globals->_roomCharacterType == PersonFlags::pftTriceraptor)
		removeInfo(_globals->_areaNum + ValleyNews::vnTriceraptorsIn);
	if (_globals->_roomCharacterType == PersonFlags::pftVelociraptor) {
		removeInfo(_globals->_areaNum + ValleyNews::vnTyrannIn);
		removeInfo(_globals->_areaNum + ValleyNews::vnTyrannLost);
		removeInfo(_globals->_areaNum + ValleyNews::vnVelociraptorsLost);
	}
}

void EdenGame::faire_suivre(int16 roomNum) {
	_destinationRoom = roomNum;
	perso_ici(4);
}

// Original name: suis_moi5
void EdenGame::AddCharacterToParty() {
	debug("adding person %d to party", (int)(_globals->_characterPtr - _persons));
	_globals->_characterPtr->_flags |= PersonFlags::pfInParty;
	_globals->_characterPtr->_roomNum = _globals->_roomNum;
	_globals->_party |= _globals->_characterPtr->_partyMask;
	_globals->_drawFlags |= DrawFlags::drDrawTopScreen;
}

// Original name: suis_moi
void EdenGame::addToParty(int16 index) {
	perso_t *old_perso = _globals->_characterPtr;
	_globals->_characterPtr = &_persons[index];
	AddCharacterToParty();
	_globals->_characterPtr = old_perso;
}

// Original name: reste_ici5
void EdenGame::removeCharacterFromParty() {
	debug("removing person %d from party", (int)(_globals->_characterPtr - _persons));
	_globals->_characterPtr->_flags &= ~PersonFlags::pfInParty;
	_globals->_partyOutside |= _globals->_characterPtr->_partyMask;
	_globals->_party &= ~_globals->_characterPtr->_partyMask;
	_globals->_drawFlags |= DrawFlags::drDrawTopScreen;
}

// Original name: reste_ici
void EdenGame::removeFromParty(int16 index) {
	perso_t *old_perso = _globals->_characterPtr;
	_globals->_characterPtr = &_persons[index];
	removeCharacterFromParty();
	_globals->_characterPtr = old_perso;
}

// Original name: eloipart
void EdenGame::handleEloiDeparture() {
	removeFromParty(PER_ELOI);
	_globals->_gameFlags &= ~GameFlags::gfFlag4000;
	_persons[PER_ELOI]._roomNum = 0;
	_globals->_partyOutside &= ~_persons[PER_ELOI]._partyMask;
	if (_globals->_roomNum == 2817)
		setChrono(3000);
	_globals->_eloiDepartureDay = _globals->_gameDays;
	_globals->_eloiHaveNews = 0;
	unlockInfo();
}

// Original name: eloirevientq
bool EdenGame::checkEloiReturn() {
	if (_globals->_phaseNum < 304)
		return true;
	if ((_globals->_phaseNum <= 353) || (_globals->_phaseNum == 370) || (_globals->_phaseNum == 384))
		return false;
	if (_globals->_areaNum != Areas::arShandovra)
		return true;
	if (_globals->_phaseNum < 480)
		return false;
	return true;
}

// Original name: eloirevient
void EdenGame::handleEloiReturn() {
	if (_globals->_areaPtr->_type == AreaType::atValley && !_persons[PER_ELOI]._roomNum)
		_persons[PER_ELOI]._roomNum = (_globals->_roomNum & 0xFF00) + 1;
}
//// phase.c
void EdenGame::incPhase() {
	static phase_t phases[] = {
		{ 65, &EdenGame::dialautoon },
		{ 113, &EdenGame::phase113 },
		{ 129, &EdenGame::dialautoon },
		{ 130, &EdenGame::phase130 },
		{ 161, &EdenGame::phase161 },
		{ 211, &EdenGame::dialautoon },
		{ 226, &EdenGame::phase226 },
		{ 257, &EdenGame::phase257 },
		{ 353, &EdenGame::phase353 },
		{ 369, &EdenGame::phase369 },
		{ 371, &EdenGame::phase371 },
		{ 385, &EdenGame::phase385 },
		{ 386, &EdenGame::dialonfollow },
		{ 418, &EdenGame::phase418 },
		{ 433, &EdenGame::phase433 },
		{ 434, &EdenGame::phase434 },
		{ 449, &EdenGame::dialautoon },
		{ 497, &EdenGame::dialautoon },
		{ 513, &EdenGame::phase513 },
		{ 514, &EdenGame::phase514 },
		{ 529, &EdenGame::phase529 },
		{ 545, &EdenGame::phase545 },
		{ 561, &EdenGame::phase561 },
		{ -1, nullptr }
	};

	_globals->_phaseNum++;
	debug("!!! next phase - %4X , room %4X", _globals->_phaseNum, _globals->_roomNum);
	_globals->_phaseActionsCount = 0;
	for (phase_t *phase = phases; phase->_id != -1; phase++) {
		if (_globals->_phaseNum == phase->_id) {
			(this->*phase->disp)();
			break;
		}
	}
}

void EdenGame::phase113() {
	removeFromParty(PER_DINA);
	_persons[PER_DINA]._roomNum = 274;
}

void EdenGame::phase130() {
	dialautoon();
	removeFromParty(PER_MONK);
}

void EdenGame::phase161() {
	Area *area = _globals->_areaPtr;
	addToParty(PER_MAMMI);
	_persons[PER_MAMMI]._flags |= PersonFlags::pf10;
	area->_flags |= AreaFlags::afFlag1;
	_globals->_curAreaFlags |= AreaFlags::afFlag1;
}

void EdenGame::phase226() {
	newObject(16, 3);
	newObject(16, 4);
	newObject(16, 5);
}

void EdenGame::phase257() {
	_gameIcons[127]._cursorId &= ~0x8000;
	_globals->_characterBackgroundBankIdx = 58;
	dialautooff();
}

void EdenGame::phase353() {
	removeFromParty(PER_DINA);
	_persons[PER_DINA]._roomNum = 0;
	_tabletView[1] = 88;
}

void EdenGame::phase369() {
	addToParty(PER_ELOI);
	_globals->_narratorSequence = 2;
	_gameRooms[334]._exits[0] = 134;
	_gameRooms[335]._exits[0] = 134;
}

void EdenGame::phase371() {
	handleEloiReturn();
	_gameIcons[128]._cursorId &= ~0x8000;
	_gameIcons[129]._cursorId &= ~0x8000;
	_gameIcons[127]._cursorId |= 0x8000;
	_globals->_characterBackgroundBankIdx = 59;
	_gameRooms[334]._exits[0] = 0xFF;
	_gameRooms[335]._exits[0] = 0xFF;
	_gameIcons[123]._objectId = 9;
	_gameIcons[124]._objectId = 26;
	_gameIcons[125]._objectId = 42;
	_gameIcons[126]._objectId = 56;
}

void EdenGame::phase385() {
	dialautooff();
	handleEloiReturn();
	_globals->_nextInfoIdx = 0;
	_globals->_lastInfoIdx = 0;
	updateInfoList();
	_globals->_lastInfo = 0;
}

void EdenGame::phase418() {
	loseObject(Objects::obHorn);
	dialautoon();
	addToParty(PER_JABBER);
}

void EdenGame::phase433() {
	dialautoon();
	_persons[PER_MAMMI_4]._flags &= ~PersonFlags::pf80;
	_persons[PER_JABBER]._flags &= ~PersonFlags::pf80;
	setCharacterHere();
	_globals->_chronoFlag = 0;
	_globals->_chrono = 0;
}

void EdenGame::phase434() {
	_globals->_roomNum = 275;
	_gameRooms[16]._bank = 44;
	_gameRooms[18]._bank = 44;
	_gameIcons[132]._cursorId &= ~0x8000;
	_globals->_characterBackgroundBankIdx = 61;
	_gameRooms[118]._exits[2] = 0xFF;
	abortDialogue();
	_gameRooms[7]._bank = 322;
	removeFromParty(PER_EVE);
	removeFromParty(PER_MONK);
	removeFromParty(PER_ELOI);
	removeFromParty(PER_GUARDS);
	removeFromParty(PER_JABBER);
	_globals->_drawFlags |= DrawFlags::drDrawTopScreen;
}

void EdenGame::phase513() {
	_globals->_lastDialogPtr = nullptr;
	parlemoiNormalFlag = false;
	dialautoon();
}

void EdenGame::phase514() {
	_gameRooms[123]._exits[2] = 1;
}

void EdenGame::phase529() {
	_gameIcons[133]._cursorId &= ~0x8000;
	_globals->_characterBackgroundBankIdx = 63;
}

void EdenGame::phase545() {
}

void EdenGame::phase561() {
	_globals->_narratorSequence = 10;
}

void EdenGame::bigphase1() {
	static void (EdenGame::*bigphases[])() = {
		&EdenGame::phase16,
		&EdenGame::phase32,
		&EdenGame::phase48,
		&EdenGame::phase64,
		&EdenGame::phase80,
		&EdenGame::phase96,
		&EdenGame::phase112,
		&EdenGame::phase128,
		&EdenGame::phase144,
		&EdenGame::phase160,
		&EdenGame::phase176,
		&EdenGame::phase192,
		&EdenGame::phase208,
		&EdenGame::phase224,
		&EdenGame::phase240,
		&EdenGame::phase256,
		&EdenGame::phase272,
		&EdenGame::phase288,
		&EdenGame::phase304,
		&EdenGame::phase320,
		&EdenGame::phase336,
		&EdenGame::phase352,
		&EdenGame::phase368,
		&EdenGame::phase384,
		&EdenGame::phase400,
		&EdenGame::phase416,
		&EdenGame::phase432,
		&EdenGame::phase448,
		&EdenGame::phase464,
		&EdenGame::phase480,
		&EdenGame::phase496,
		&EdenGame::phase512,
		&EdenGame::phase528,
		&EdenGame::phase544,
		&EdenGame::phase560
	};

	int16 phase = (_globals->_phaseNum & ~3) + 0x10;   //TODO: check me
	debug("!!! big phase - %4X", phase);
	_globals->_phaseActionsCount = 0;
	_globals->_phaseNum = phase;
	if (phase > 560)
		return;
	phase >>= 4;
	(this->*bigphases[phase - 1])();
}

void EdenGame::bigphase() {
	if (!(_globals->_dialogPtr->_flags & DialogFlags::dfSpoken))
		bigphase1();
}

void EdenGame::phase16() {
	dialautoon();
}

void EdenGame::phase32() {
	word_31E7A &= ~0x8000;
}

void EdenGame::phase48() {
	_gameRooms[8]._exits[1] = 22;
	dialautoon();
}

void EdenGame::phase64() {
	addToParty(PER_DINA);
	_persons[PER_ELOI]._roomNum = 259;
}

void EdenGame::phase80() {
	_persons[PER_TAU]._roomNum = 0;
}

void EdenGame::phase96() {
}

void EdenGame::phase112() {
	giveObject();
}

void EdenGame::phase128() {
	addToParty(PER_DINA);
	giveObject();
}

void EdenGame::phase144() {
	addToParty(PER_ELOI);
	_gameRooms[113]._video = 0;
	_gameRooms[113]._bank = 317;
}

void EdenGame::phase160() {
}

void EdenGame::phase176() {
	dialonfollow();
}

void EdenGame::phase192() {
	Area *area = _globals->_areaPtr;
	addToParty(PER_MAMMI_1);
	_persons[PER_MAMMI_1]._flags |= PersonFlags::pf10;
	dialautoon();
	area->_flags |= AreaFlags::afFlag1;
	_globals->_curAreaFlags |= AreaFlags::afFlag1;
}

void EdenGame::phase208() {
	handleEloiReturn();
}

void EdenGame::phase224() {
	_gameIcons[126]._cursorId &= ~0x8000;
	_globals->_characterBackgroundBankIdx = 57;
	dialautooff();
}

void EdenGame::phase240() {
	Area *area = _globals->_areaPtr;
	addToParty(PER_MAMMI_2);
	_persons[PER_MAMMI_2]._flags |= PersonFlags::pf10;
	area->_flags |= AreaFlags::afFlag1;
	_globals->_curAreaFlags |= AreaFlags::afFlag1;
}

void EdenGame::phase256() {
	dialautoon();
}

void EdenGame::phase272() {
	dialautoon();
	_globals->_eloiHaveNews = 0;
}

void EdenGame::phase288() {
	setChoiceYes();
	_persons[PER_MUNGO]._roomNum = 0;
	removeFromParty(PER_MUNGO);
	addToParty(PER_ELOI);
	_globals->_narratorSequence = 8;
}

void EdenGame::phase304() {
	Area *area = _globals->_areaPtr;
	addToParty(PER_EVE);
	addToParty(PER_MAMMI_5);
	_persons[PER_MAMMI_5]._flags |= PersonFlags::pf10;
	dialautoon();
	area->_flags |= AreaFlags::afFlag1;
	_globals->_curAreaFlags |= AreaFlags::afFlag1;
}

void EdenGame::phase320() {
	dialonfollow();
}

void EdenGame::phase336() {
	_gameRooms[288]._exits[0] = 135;
	_gameRooms[289]._exits[0] = 135;
	loseObject(_globals->_curObjectId);
	dialautoon();
}

void EdenGame::phase352() {
	_personRoomBankTable[30] = 26;
	_persons[PER_EVE]._spriteBank = 9;
	_persons[PER_EVE]._targetLoc = 8;
	_followerList[13]._spriteNum = 2;
	dialautoon();
	_gameRooms[288]._exits[0] = 0xFF;
	_gameRooms[289]._exits[0] = 0xFF;
	_gameRooms[288]._flags &= ~RoomFlags::rf02;
	_gameRooms[289]._flags &= ~RoomFlags::rf02;
}

void EdenGame::phase368() {
	removeFromParty(PER_EVE);
	dialautoon();
	_persons[PER_ELOI]._roomNum = 1811;
	_persons[PER_DINA]._roomNum = 1607;
}

void EdenGame::phase384() {
	Area *area = _globals->_areaPtr;
	addToParty(PER_EVE);
	removeFromParty(PER_DINA);
	dialautoon();
	area->_flags |= AreaFlags::afFlag1;
	_globals->_curAreaFlags |= AreaFlags::afFlag1;
	handleEloiDeparture();
}

void EdenGame::phase400() {
	dialonfollow();
	_persons[PER_KING]._roomNum = 0;
	_persons[PER_MONK]._roomNum = 259;
	_globals->_eloiHaveNews = 0;
	_objectLocations[20] = 259;
}

void EdenGame::phase416() {
	addToParty(PER_MONK);
	_gameIcons[130]._cursorId &= ~0x8000;
	_globals->_characterBackgroundBankIdx = 60;
	_gameRooms[0]._exits[0] = 138;
}

void EdenGame::phase432() {
	_globals->_narratorSequence = 3;
	_persons[PER_MAMMI_4]._flags |= PersonFlags::pf80;
	_persons[PER_JABBER]._flags |= PersonFlags::pf80;
	_persons[PER_ELOI]._roomNum = 257;
	_gameRooms[0]._exits[0] = 0xFF;
	_globals->_drawFlags |= DrawFlags::drDrawTopScreen;
}

void EdenGame::phase448() {
	dialautoon();
	handleEloiDeparture();
}

void EdenGame::phase464() {
	_globals->_areaPtr->_flags |= AreaFlags::afFlag1;
	_globals->_curAreaFlags |= AreaFlags::afFlag1;
	_persons[PER_MAMMI_6]._flags |= PersonFlags::pf10;
	addToParty(PER_SHAZIA);
	_globals->_citadelAreaNum = _globals->_areaNum;
	naitredino(8);
}

void EdenGame::phase480() {
	giveObject();
	newValley();
	handleEloiReturn();
	_tabletView[1] = 94;
}

void EdenGame::phase496() {
	dialautoon();
	_globals->_lastDialogPtr = nullptr;
	parlemoiNormalFlag = false;
}

void EdenGame::phase512() {
	removeFromParty(PER_MONK);
	removeFromParty(PER_EVE);
	removeFromParty(PER_SHAZIA);
	removeFromParty(PER_GUARDS);
}

void EdenGame::phase528() {
	_globals->_narratorSequence = 11;
	addToParty(PER_MONK);
	addToParty(PER_ELOI);
	addToParty(PER_EVE);
	addToParty(PER_SHAZIA);
	addToParty(PER_GUARDS);
}

void EdenGame::phase544() {
	handleEloiDeparture();
	dialautoon();
	removeFromParty(PER_SHAZIA);
	removeFromParty(PER_GUARDS);
}

void EdenGame::phase560() {
	_persons[PER_ELOI]._roomNum = 3073;
	_gameRooms[127]._exits[1] = 0;
}

void EdenGame::saveGame(char *name) {
	Common::OutSaveFile *fh = g_system->getSavefileManager()->openForSaving(name);
	if (!fh)
		return;

	Common::Serializer s(nullptr, fh);

	syncGame(s);

	delete fh;
}

void EdenGame::syncGame(Common::Serializer s) {
	syncGlobalPointers(s);
	syncGlobalValues(s);

	// _gameIcons
	// CHECKME: only from #123 to #133?
	for (int i = 123; i < 134; i++) {
		s.syncAsSint16LE(_gameIcons[i].sx);
		s.syncAsSint16LE(_gameIcons[i].sy);
		s.syncAsSint16LE(_gameIcons[i].ex);
		s.syncAsSint16LE(_gameIcons[i].ey);
		s.syncAsUint16LE(_gameIcons[i]._cursorId);
		s.syncAsUint16LE(_gameIcons[i]._actionId);
		s.syncAsUint16LE(_gameIcons[i]._objectId);
	}

	syncCitadelRoomPointers(s);

	// _areasTable
	for (int i = 0; i < 12; i++) {
		s.syncAsByte(_areasTable[i]._num);
		s.syncAsByte(_areasTable[i]._type);
		s.syncAsUint16LE(_areasTable[i]._flags);
		s.syncAsUint16LE(_areasTable[i]._firstRoomIdx);
		s.syncAsByte(_areasTable[i]._citadelLevel);
		s.syncAsByte(_areasTable[i]._placeNum);
		s.syncAsSint16LE(_areasTable[i]._visitCount);
	}

	// _gameRooms
	for (int i = 0; i < 423; i++) {
		s.syncAsByte(_gameRooms[i]._id);
		for (int j = 0; j < 4; j++)
			s.syncAsByte(_gameRooms[i]._exits[j]);
		s.syncAsByte(_gameRooms[i]._flags);
		s.syncAsUint16LE(_gameRooms[i]._bank);
		s.syncAsUint16LE(_gameRooms[i]._party);
		s.syncAsByte(_gameRooms[i]._level);
		s.syncAsByte(_gameRooms[i]._video);
		s.syncAsByte(_gameRooms[i]._location);
		s.syncAsByte(_gameRooms[i]._backgroundBankNum);
	}

	// _Objects
	for (int i = 0; i < 42; i++) {
		s.syncAsByte(_objects[i]._id);
		s.syncAsByte(_objects[i]._flags);
		s.syncAsSint16LE(_objects[i]._locations);
		s.syncAsUint16LE(_objects[i]._itemMask);
		s.syncAsUint16LE(_objects[i]._powerMask);
		s.syncAsSint16LE(_objects[i]._count);
	}

	for (int i = 0; i < 45; i++)
		s.syncAsUint16LE(_objectLocations[i]);

	// _followerList[13]
	// CHECKME: Only #13?
	s.syncAsByte(_followerList[13]._id);
	s.syncAsByte(_followerList[13]._spriteNum);
	s.syncAsSint16LE(_followerList[13].sx);
	s.syncAsSint16LE(_followerList[13].sy);
	s.syncAsSint16LE(_followerList[13].ex);
	s.syncAsSint16LE(_followerList[13].ey);
	s.syncAsSint16LE(_followerList[13]._spriteBank);
	s.syncAsSint16LE(_followerList[13].ff_C);
	s.syncAsSint16LE(_followerList[13].ff_E);

	// _persons
	for (int i = 0; i < 58; i++) {
		s.syncAsUint16LE(_persons[i]._roomNum);
		s.syncAsUint16LE(_persons[i]._actionId);
		s.syncAsUint16LE(_persons[i]._partyMask);
		s.syncAsByte(_persons[i]._id);
		s.syncAsByte(_persons[i]._flags);
		s.syncAsByte(_persons[i]._roomBankId);
		s.syncAsByte(_persons[i]._spriteBank);
		s.syncAsUint16LE(_persons[i]._items);
		s.syncAsUint16LE(_persons[i]._powers);
		s.syncAsByte(_persons[i]._targetLoc);
		s.syncAsByte(_persons[i]._lastLoc);
		s.syncAsByte(_persons[i]._speed);
		s.syncAsByte(_persons[i]._steps);
	}

	syncTapePointers(s);

	// _tapes
	for (int i = 0; i < MAX_TAPES; i++) {
		s.syncAsSint16LE(_tapes[i]._textNum);
		s.syncAsSint16LE(_tapes[i]._party);
		s.syncAsSint16LE(_tapes[i]._roomNum);
		s.syncAsSint16LE(_tapes[i]._backgroundBankNum);
	}

	// _tabletView
	// CHECKME: Only 6 out of 12?
	for (int i = 0; i < 6; i++)
		s.syncAsByte(_tabletView[i]);

	// _gameDialogs
	for (int i = 0; i < 10240; i++)
		s.syncAsByte(_gameDialogs[i]);
}

void EdenGame::loadrestart() {
	_quitFlag3 = true;
/*
	assert(0);  //TODO: this won't work atm - all snapshots are BE
	int32 offs = 0;
	int32 size;
	size = (char *)(&_globals->_saveEnd) - (char *)(_globals);
	loadpartoffile(2495, _globals, offs, size);
	offs += size;
	vavaoffsetin();
	size = (char *)(&_gameIcons[134]) - (char *)(&_gameIcons[123]);
	loadpartoffile(2495, &_gameIcons[123], offs, size);
	offs += size;
	size = (char *)(&_areasTable[12]) - (char *)(&_areasTable[0]);
	loadpartoffile(2495, &_areasTable[0], offs, size);
	offs += size;
	lieuoffsetin();
	size = (char *)(&_gameRooms[423]) - (char *)(&_gameRooms[0]);
	loadpartoffile(2495, &_gameRooms[0], offs, size);
	offs += size;
	size = (char *)(&_objects[42]) - (char *)(&_objects[0]);
	loadpartoffile(2495,  &_objects[0], offs, size);
	offs += size;
	size = (char *)(&_objectLocations[45]) - (char *)(&_objectLocations[0]);
	loadpartoffile(2495,  &_objectLocations[0], offs, size);
	offs += size;
	size = (char *)(&_followerList[14]) - (char *)(&_followerList[13]);
	loadpartoffile(2495,  &_followerList[13], offs, size);
	offs += size;
	size = (char *)(&_persons[PER_UNKN_3DE]) - (char *)(&_persons[PER_KING]);
	loadpartoffile(2495,  &_persons[PER_KING], offs, size);
	offs += size;
	size = (char *)(&_tapes[16]) - (char *)(&_tapes[0]);
	loadpartoffile(2495,  &_tapes[0], offs, size);
	offs += size;
	bandeoffsetin();
	size = (char *)(&_tabletView[6]) - (char *)(&_tabletView[0]);
	loadpartoffile(2495, &_tabletView[0], offs, size);
	offs += size;
	size = (char *)(&_gameDialogs[10240]) - (char *)(&_gameDialogs[0]); //TODO: const size 10240
	loadpartoffile(2495,  &_gameDialogs[0], offs, size);
	_gameLoaded = true;
	*/
}

void EdenGame::loadgame(char *name) {
	Common::InSaveFile *fh = g_system->getSavefileManager()->openForLoading(name);
	if (!fh)
		return;

	Common::Serializer s(fh, nullptr);
	syncGame(s);

	delete fh;
	_gameLoaded = true;
}

#define NULLPTR 0xFFFFFF
#define IDXOUT(val, base, typ, idx) do { if (val)      (idx) = ((byte*)val - (byte*)base) / sizeof(typ); else (idx) = NULLPTR; } while (false)
#define OFSIN(val, base, typ) do { if ((void*)(val) != NULLPTR)   (val) = (typ*)((char*)(val) + (size_t)(base)); else (val) = 0; } while (false)

void EdenGame::syncGlobalPointers(Common::Serializer s) {
	uint32 dialogIdx, nextDialogIdx, narratorDialogIdx, lastDialogIdx, tapeIdx, nextRoomIconIdx, roomIdx;
	uint32 citaAreaFirstRoomIdx, areaIdx, lastAreaIdx, curAreaIdx, characterIdx, roomCharacterIdx;

	if (s.isSaving()) {
		IDXOUT(_globals->_dialogPtr, _gameDialogs, Dialog, dialogIdx);
		IDXOUT(_globals->_nextDialogPtr, _gameDialogs, Dialog, nextDialogIdx);
		IDXOUT(_globals->_narratorDialogPtr, _gameDialogs, Dialog, narratorDialogIdx);
		IDXOUT(_globals->_lastDialogPtr, _gameDialogs, Dialog, lastDialogIdx);
		IDXOUT(_globals->_tapePtr, _tapes, tape_t, tapeIdx);
		IDXOUT(_globals->_nextRoomIcon, _gameIcons, Icon, nextRoomIconIdx);
		IDXOUT(_globals->_roomPtr, _gameRooms, Room, roomIdx);
		IDXOUT(_globals->_citaAreaFirstRoom, _gameRooms, Room, citaAreaFirstRoomIdx);
		IDXOUT(_globals->_areaPtr, _areasTable, Area, areaIdx);
		IDXOUT(_globals->_lastAreaPtr, _areasTable, Area, lastAreaIdx);
		IDXOUT(_globals->_curAreaPtr, _areasTable, Area, curAreaIdx);
		IDXOUT(_globals->_characterPtr, _persons, perso_t, characterIdx);
		IDXOUT(_globals->_roomCharacterPtr, _persons, perso_t, roomCharacterIdx);
	}

	s.syncAsUint32LE(dialogIdx);
	s.syncAsUint32LE(nextDialogIdx);
	s.syncAsUint32LE(narratorDialogIdx);
	s.syncAsUint32LE(lastDialogIdx);
	s.syncAsUint32LE(tapeIdx);
	s.syncAsUint32LE(nextRoomIconIdx);
	s.syncAsUint32LE(roomIdx);
	s.syncAsUint32LE(citaAreaFirstRoomIdx);
	s.syncAsUint32LE(areaIdx);
	s.syncAsUint32LE(lastAreaIdx);
	s.syncAsUint32LE(curAreaIdx);
	s.syncAsUint32LE(characterIdx);
	s.syncAsUint32LE(roomCharacterIdx);

	if (s.isLoading()) {
		_globals->_dialogPtr = (dialogIdx == NULLPTR) ? nullptr : (Dialog *)getElem(_gameDialogs, dialogIdx);
		_globals->_nextDialogPtr = (nextDialogIdx == NULLPTR) ? nullptr : (Dialog *)getElem(_gameDialogs, nextDialogIdx);
		_globals->_narratorDialogPtr = (narratorDialogIdx == NULLPTR) ? nullptr : (Dialog *)getElem(_gameDialogs, narratorDialogIdx);
		_globals->_lastDialogPtr = (lastDialogIdx == NULLPTR) ? nullptr : (Dialog *)getElem(_gameDialogs, lastDialogIdx);
		_globals->_tapePtr = (tapeIdx == NULLPTR) ? nullptr : &_tapes[tapeIdx];
		_globals->_nextRoomIcon = (nextRoomIconIdx == NULLPTR) ? nullptr : &_gameIcons[nextRoomIconIdx];
		_globals->_roomPtr = (roomIdx == NULLPTR) ? nullptr : &_gameRooms[roomIdx];
		_globals->_citaAreaFirstRoom = (citaAreaFirstRoomIdx == NULLPTR) ? nullptr : &_gameRooms[citaAreaFirstRoomIdx];
		_globals->_areaPtr = (areaIdx == NULLPTR) ? nullptr : &_areasTable[areaIdx];
		_globals->_lastAreaPtr = (lastAreaIdx == NULLPTR) ? nullptr : &_areasTable[lastAreaIdx];
		_globals->_curAreaPtr = (curAreaIdx == NULLPTR) ? nullptr : &_areasTable[curAreaIdx];
		_globals->_characterPtr = (characterIdx == NULLPTR) ? nullptr : &_persons[characterIdx];
		_globals->_roomCharacterPtr = (roomCharacterIdx == NULLPTR) ? nullptr : &_persons[roomCharacterIdx];
	}
}

void EdenGame::syncGlobalValues(Common::Serializer s) {
	s.syncAsByte(_globals->_areaNum);
	s.syncAsByte(_globals->_areaVisitCount);
	s.syncAsByte(_globals->_menuItemIdLo);
	s.syncAsByte(_globals->_menuItemIdHi);
	s.syncAsUint16LE(_globals->_randomNumber);
	s.syncAsUint16LE(_globals->_gameTime);
	s.syncAsUint16LE(_globals->_gameDays);
	s.syncAsUint16LE(_globals->_chrono);
	s.syncAsUint16LE(_globals->_eloiDepartureDay);
	s.syncAsUint16LE(_globals->_roomNum);
	s.syncAsUint16LE(_globals->_newRoomNum);
	s.syncAsUint16LE(_globals->_phaseNum);
	s.syncAsUint16LE(_globals->_metPersonsMask1);
	s.syncAsUint16LE(_globals->_party);
	s.syncAsUint16LE(_globals->_partyOutside);
	s.syncAsUint16LE(_globals->_metPersonsMask2);
	s.syncAsUint16LE(_globals->_var1C);
	s.syncAsUint16LE(_globals->_phaseActionsCount);
	s.syncAsUint16LE(_globals->_curAreaFlags);
	s.syncAsUint16LE(_globals->_curItemsMask);
	s.syncAsUint16LE(_globals->_curPowersMask);
	s.syncAsUint16LE(_globals->_curPersoItems);
	s.syncAsUint16LE(_globals->_curCharacterPowers);
	s.syncAsUint16LE(_globals->_wonItemsMask);
	s.syncAsUint16LE(_globals->_wonPowersMask);
	s.syncAsUint16LE(_globals->_stepsToFindAppleFast);
	s.syncAsUint16LE(_globals->_stepsToFindAppleNormal);
	s.syncAsUint16LE(_globals->_roomPersoItems);
	s.syncAsUint16LE(_globals->_roomCharacterPowers);
	s.syncAsUint16LE(_globals->_gameFlags);
	s.syncAsUint16LE(_globals->_curVideoNum);
	s.syncAsUint16LE(_globals->_morkusSpyVideoNum1);
	s.syncAsUint16LE(_globals->_morkusSpyVideoNum2);
	s.syncAsUint16LE(_globals->_morkusSpyVideoNum3);
	s.syncAsUint16LE(_globals->_morkusSpyVideoNum4);
	s.syncAsByte(_globals->_newMusicType);
	s.syncAsByte(_globals->_var43);
	s.syncAsByte(_globals->_videoSubtitleIndex);
	s.syncAsByte(_globals->_partyInstruments);
	s.syncAsByte(_globals->_monkGotRing);
	s.syncAsByte(_globals->_chronoFlag);
	s.syncAsByte(_globals->_curRoomFlags);
	s.syncAsByte(_globals->_endGameFlag);
	s.syncAsByte(_globals->_lastInfo);

	byte autoDialog;
	if (s.isSaving())
		autoDialog = _globals->_autoDialog ? 1 : 0;
	s.syncAsByte(autoDialog);
	if (s.isLoading())
		_globals->_autoDialog = (autoDialog == 1);

	s.syncAsByte(_globals->_worldTyranSighted);
	s.syncAsByte(_globals->_var4D);
	s.syncAsByte(_globals->_var4E);
	s.syncAsByte(_globals->_worldGaveGold);
	s.syncAsByte(_globals->_worldHasTriceraptors);
	s.syncAsByte(_globals->_worldHasVelociraptors);
	s.syncAsByte(_globals->_worldHasTyran);
	s.syncAsByte(_globals->_var53);
	s.syncAsByte(_globals->_var54);
	s.syncAsByte(_globals->_var55);
	s.syncAsByte(_globals->_gameHours);
	s.syncAsByte(_globals->_textToken1);
	s.syncAsByte(_globals->_textToken2);
	s.syncAsByte(_globals->_eloiHaveNews);
	s.syncAsByte(_globals->_dialogFlags);
	s.syncAsByte(_globals->_curAreaType);
	s.syncAsByte(_globals->_curCitadelLevel);
	s.syncAsByte(_globals->_newLocation);
	s.syncAsByte(_globals->_prevLocation);
	s.syncAsByte(_globals->_curPersoFlags);
	s.syncAsByte(_globals->_var60);
	s.syncAsByte(_globals->_eventType);
	s.syncAsByte(_globals->_var62);
	s.syncAsByte(_globals->_curObjectId);
	s.syncAsByte(_globals->_curObjectFlags);
	s.syncAsByte(_globals->_var65);
	s.syncAsByte(_globals->_roomCharacterType);
	s.syncAsByte(_globals->_roomCharacterFlags);
	s.syncAsByte(_globals->_narratorSequence);
	s.syncAsByte(_globals->_var69);
	s.syncAsByte(_globals->_var6A);
	s.syncAsByte(_globals->_frescoNumber);
	s.syncAsByte(_globals->_var6C);
	s.syncAsByte(_globals->_var6D);
	s.syncAsByte(_globals->_labyrinthDirections);
	s.syncAsByte(_globals->_labyrinthRoom);

/*
	CHECKME: *_sentenceBufferPtr
*/

	s.syncAsByte(_globals->_lastInfoIdx);
	s.syncAsByte(_globals->_nextInfoIdx);

/*
	CHECKME
		* _persoSpritePtr
		* _persoSpritePtr2
		* _curCharacterAnimPtr
		* _varC2
*/

	s.syncAsSint16LE(_globals->_iconsIndex);
	s.syncAsSint16LE(_globals->_curObjectCursor);
	s.syncAsSint16LE(_globals->_varCA);
	s.syncAsSint16LE(_globals->_varCC);
	s.syncAsSint16LE(_globals->_characterImageBank);
	s.syncAsUint16LE(_globals->_roomImgBank);
	s.syncAsUint16LE(_globals->_characterBackgroundBankIdx);
	s.syncAsUint16LE(_globals->_varD4);
	s.syncAsUint16LE(_globals->_frescoeWidth);
	s.syncAsUint16LE(_globals->_frescoeImgBank);
	s.syncAsUint16LE(_globals->_varDA);
	s.syncAsUint16LE(_globals->_varDC);
	s.syncAsUint16LE(_globals->_roomBaseX);
	s.syncAsUint16LE(_globals->_varE0);
	s.syncAsUint16LE(_globals->_dialogType);
	s.syncAsUint16LE(_globals->_varE4);
	s.syncAsUint16LE(_globals->_currMusicNum);
	s.syncAsSint16LE(_globals->_textNum);
	s.syncAsUint16LE(_globals->_travelTime);
	s.syncAsUint16LE(_globals->_varEC);
	s.syncAsByte(_globals->_displayFlags);
	s.syncAsByte(_globals->_oldDisplayFlags);
	s.syncAsByte(_globals->_drawFlags);
	s.syncAsByte(_globals->_varF1);
	s.syncAsByte(_globals->_varF2);
	s.syncAsByte(_globals->_menuFlags);
	s.syncAsByte(_globals->_varF4);
	s.syncAsByte(_globals->_varF5);
	s.syncAsByte(_globals->_varF6);
	s.syncAsByte(_globals->_varF7);
	s.syncAsByte(_globals->_varF8);
	s.syncAsByte(_globals->_varF9);
	s.syncAsByte(_globals->_varFA);
	s.syncAsByte(_globals->_animationFlags);
	s.syncAsByte(_globals->_giveObj1);
	s.syncAsByte(_globals->_giveObj2);
	s.syncAsByte(_globals->_giveObj3);
	s.syncAsByte(_globals->_var100);
	s.syncAsByte(_globals->_roomVidNum);
	s.syncAsByte(_globals->_mirrorEffect);
	s.syncAsByte(_globals->_var103);
	s.syncAsByte(_globals->_roomBackgroundBankNum);
	s.syncAsByte(_globals->_valleyVidNum);
	s.syncAsByte(_globals->_updatePaletteFlag);
	s.syncAsByte(_globals->_inventoryScrollPos);
	s.syncAsByte(_globals->_objCount);
	s.syncAsByte(_globals->_textBankIndex);
	s.syncAsByte(_globals->_prefLanguage);
	for (int i = 0; i < 2; i++) {
		s.syncAsByte(_globals->_prefMusicVol[i]);
		s.syncAsByte(_globals->_prefVoiceVol[i]);
		s.syncAsByte(_globals->_prefSoundVolume[i]);
	}
	s.syncAsByte(_globals->_citadelAreaNum);
	s.syncAsByte(_globals->_var113);
	s.syncAsByte(_globals->_lastPlaceNum);
	s.syncAsByte(_globals->_saveEnd);
}

void EdenGame::syncCitadelRoomPointers(Common::Serializer s) {
	uint32 citadelRoomIdx;
	for (int i = 0; i < 12; i++) {
		if (s.isSaving()) {
			IDXOUT(_areasTable[i]._citadelRoomPtr, _gameRooms, Room, citadelRoomIdx);
		}
		s.syncAsUint32LE(citadelRoomIdx);
		if (s.isLoading())
			_areasTable[i]._citadelRoomPtr = (citadelRoomIdx == NULLPTR) ? nullptr : &_gameRooms[citadelRoomIdx];
	}
}

void EdenGame::syncTapePointers(Common::Serializer s) {
	int persoIdx;

	for (int i = 0; i < 16; i++) {
		int index, subIndex;
		if (s.isSaving()) {
			index = NULLPTR;
			char *closerPtr = nullptr;
			for (int j = (getElem((char *)_gameDialogs, 0) - (char *)_gameDialogs) / sizeof(char *) - 1; j >= 0; j--) {
				char *tmpPtr = getElem((char *)_gameDialogs, j);
				if ((tmpPtr <= (char *)_tapes[i]._dialog) && (tmpPtr > closerPtr)) {
					index = j;
					closerPtr = tmpPtr;
				}
			}

			subIndex = NULLPTR;
			if (index != NULLPTR)
				subIndex = ((char *)_tapes[i]._dialog - closerPtr);

			IDXOUT(_tapes[i]._perso, _persons, perso_t, persoIdx);
		}

		s.syncAsUint32LE(persoIdx);
		s.syncAsUint32LE(index);
		s.syncAsUint32LE(subIndex);

		if (s.isLoading()) {
			_tapes[i]._perso = (persoIdx == NULLPTR) ? nullptr : &_persons[persoIdx];
			char *tmpPtr = nullptr;

			if (index != NULLPTR) {
				tmpPtr = getElem((char *)_gameDialogs, index);
				if (subIndex != NULLPTR)
					tmpPtr += subIndex;
			}
			_tapes[i]._dialog = (Dialog *)tmpPtr;
		}
	}
}

char EdenGame::testCondition(int16 index) {
	bool endFl = false;
	uint16 stack[32];
	uint16 *sp = stack;
	assert(index > 0);
	_codePtr = (byte *)getElem(_gameConditions, (index - 1));
	uint16 value;
	do {
		value = fetchValue();
		for (;;) {
			byte op = *_codePtr++;
			if (op == 0xFF) {
				endFl = true;
				break;
			}
			if ((op & 0x80) == 0) {
				uint16 value2 = fetchValue();
				value = operation(op, value, value2);
			} else {
				assert(sp < stack + 32);
				*sp++ = value;
				*sp++ = op;
				break;
			}
		}
	} while (!endFl);

	if (sp != stack) {
		*sp++ = value;
		uint16 *sp2 = stack;
		value = *sp2++;
		do {
			byte op = *sp2++;
			uint16 value2 = *sp2++;
			value = operation(op, value, value2);
		} while (sp2 != sp);
	}
//	if (value)
	debug("cond %d(-1) returns %s", index, value ? "TRUE" : "false");
//	if (index == 402) debug("(glob_61.b == %X) & (glob_12.w == %X) & (glob_4C.b == %X) & (glob_4E.b == %X)", p_global->eventType, p_global->phaseNum, p_global->worldTyrannSighted, p_global->ff_4E);
	return value != 0;
}

// Original name: opera_add
uint16 EdenGame::operAdd(uint16 v1, uint16 v2) {
	return v1 + v2;
}

// Original name: opera_sub
uint16 EdenGame::operSub(uint16 v1, uint16 v2) {
	return v1 - v2;
}

// Original name: opera_and
uint16 EdenGame::operLogicalAnd(uint16 v1, uint16 v2) {
	return v1 & v2;
}

// Original name: opera_or
uint16 EdenGame::operLogicalOr(uint16 v1, uint16 v2) {
	return v1 | v2;
}

// Original name: opera_egal
uint16 EdenGame::operIsEqual(uint16 v1, uint16 v2) {
	return v1 == v2 ? -1 : 0;
}

// Original name: opera_petit
uint16 EdenGame::operIsSmaller(uint16 v1, uint16 v2) {
	return v1 < v2 ? -1 : 0;    //TODO: all comparisons are unsigned!
}

// Original name: opera_grand
uint16 EdenGame::operIsGreater(uint16 v1, uint16 v2) {
	return v1 > v2 ? -1 : 0;
}

// Original name: opera_diff
uint16 EdenGame::operIsDifferent(uint16 v1, uint16 v2) {
	return v1 != v2 ? -1 : 0;
}

// Original name: opera_petega
uint16 EdenGame::operIsSmallerOrEqual(uint16 v1, uint16 v2) {
	return v1 <= v2 ? -1 : 0;
}

// Original name: opera_graega
uint16 EdenGame::operIsGreaterOrEqual(uint16 v1, uint16 v2) {
	return v1 >= v2 ? -1 : 0;
}

// Original name: opera_faux
uint16 EdenGame::operFalse(uint16 v1, uint16 v2) {
	return 0;
}

uint16 EdenGame::operation(byte op, uint16 v1, uint16 v2) {
	static uint16(EdenGame::*operations[16])(uint16, uint16) = {
		&EdenGame::operIsEqual,
		&EdenGame::operIsSmaller,
		&EdenGame::operIsGreater,
		&EdenGame::operIsDifferent,
		&EdenGame::operIsSmallerOrEqual,
		&EdenGame::operIsGreaterOrEqual,
		&EdenGame::operAdd,
		&EdenGame::operSub,
		&EdenGame::operLogicalAnd,
		&EdenGame::operLogicalOr,
		&EdenGame::operFalse,
		&EdenGame::operFalse,
		&EdenGame::operFalse,
		&EdenGame::operFalse,
		&EdenGame::operFalse,
		&EdenGame::operFalse
	};
	return (this->*operations[(op & 0x1F) >> 1])(v1, v2);
}

#define VAR(ofs, var) case ofs: return _globals->var;

uint8 EdenGame::getByteVar(uint16 offset) {
	switch (offset) {
		VAR(0, _areaNum);
		VAR(1, _areaVisitCount);
		VAR(2, _menuItemIdLo);
		VAR(3, _menuItemIdHi);   //TODO: pad?
		VAR(0x42, _newMusicType);
		VAR(0x43, _var43);
		VAR(0x44, _videoSubtitleIndex);
		VAR(0x45, _partyInstruments);   // &1 - Bell for Monk, &2 - Drum for Thugg
		VAR(0x46, _monkGotRing);
		VAR(0x47, _chronoFlag);
		VAR(0x48, _curRoomFlags);
		VAR(0x49, _endGameFlag);
		VAR(0x4A, _lastInfo);
		VAR(0x4B, _autoDialog);
		VAR(0x4C, _worldTyranSighted);
		VAR(0x4D, _var4D);
		VAR(0x4E, _var4E);
		VAR(0x4F, _worldGaveGold);
		VAR(0x50, _worldHasTriceraptors);
		VAR(0x51, _worldHasVelociraptors);
		VAR(0x52, _worldHasTyran);
		VAR(0x53, _var53);
		VAR(0x54, _var54);  //CHEKME: Used?
		VAR(0x55, _var55);  //TODO: pad?
		VAR(0x56, _gameHours);
		VAR(0x57, _textToken1);
		VAR(0x58, _textToken2); //TODO: pad?
		VAR(0x59, _eloiHaveNews);
		VAR(0x5A, _dialogFlags);
		VAR(0x5B, _curAreaType);
		VAR(0x5C, _curCitadelLevel);
		VAR(0x5D, _newLocation);
		VAR(0x5E, _prevLocation);
		VAR(0x5F, _curPersoFlags);
		VAR(0x60, _var60);
		VAR(0x61, _eventType);
		VAR(0x62, _var62);  //TODO: pad?
		VAR(0x63, _curObjectId);
		VAR(0x64, _curObjectFlags);
		VAR(0x65, _var65);  //TODO: pad?
		VAR(0x66, _roomCharacterType);
		VAR(0x67, _roomCharacterFlags);
		VAR(0x68, _narratorSequence);
		VAR(0x69, _var69);
		VAR(0x6A, _var6A);
		VAR(0x6B, _frescoNumber);
		VAR(0x6C, _var6C);  //TODO: pad?
		VAR(0x6D, _var6D);  //TODO: pad?
		VAR(0x6E, _labyrinthDirections);
		VAR(0x6F, _labyrinthRoom);
	default:
		error("Undefined byte variable access (0x%X)", offset);
	}
	return 0;
}

uint16 EdenGame::getWordVar(uint16 offset) {
	switch (offset) {
		VAR(4, _randomNumber);   //TODO: this is randomized in pc ver and used by some conds. always zero on mac
		VAR(6, _gameTime);
		VAR(8, _gameDays);
		VAR(0xA, _chrono);
		VAR(0xC, _eloiDepartureDay);
		VAR(0xE, _roomNum);        // current room number
		VAR(0x10, _newRoomNum);     // target room number selected on world map
		VAR(0x12, _phaseNum);
		VAR(0x14, _metPersonsMask1);
		VAR(0x16, _party);
		VAR(0x18, _partyOutside);
		VAR(0x1A, _metPersonsMask2);
		VAR(0x1C, _var1C);    //TODO: write-only?
		VAR(0x1E, _phaseActionsCount);
		VAR(0x20, _curAreaFlags);
		VAR(0x22, _curItemsMask);
		VAR(0x24, _curPowersMask);
		VAR(0x26, _curPersoItems);
		VAR(0x28, _curCharacterPowers);
		VAR(0x2A, _wonItemsMask);
		VAR(0x2C, _wonPowersMask);
		VAR(0x2E, _stepsToFindAppleFast);
		VAR(0x30, _stepsToFindAppleNormal);
		VAR(0x32, _roomPersoItems); //TODO: write-only?
		VAR(0x34, _roomCharacterPowers);    //TODO: write-only?
		VAR(0x36, _gameFlags);
		VAR(0x38, _curVideoNum);
		VAR(0x3A, _morkusSpyVideoNum1); //TODO: pad?
		VAR(0x3C, _morkusSpyVideoNum2); //TODO: pad?
		VAR(0x3E, _morkusSpyVideoNum3); //TODO: pad?
		VAR(0x40, _morkusSpyVideoNum4); //TODO: pad?
	default:
		error("Undefined word variable access (0x%X)", offset);
	}
	return 0;
}

#undef VAR

// Original name: cher_valeur
uint16 EdenGame::fetchValue() {
	uint16 val;
	byte typ = *_codePtr++;
	if (typ < 0x80) {
		byte ofs = *_codePtr++;
		val = (typ == 1) ? getByteVar(ofs) : getWordVar(ofs);
	} else if (typ == 0x80)
		val = *_codePtr++;
	else {
		val = READ_LE_UINT16(_codePtr);
		_codePtr += 2;
	}
	return val;
}

// Original name: ret
void EdenGame::actionNop() {
}

//// cube.c
// Original name: make_tabcos
void EdenGame::initSinCosTable() {
	for (int i = 0; i < 361; i++) {
		_cosTable[i] = (int)(cos(3.1416 * i / 180.0) * 255.0);
		_sinTable[i] = (int)(sin(3.1416 * i / 180.0) * 255.0);
	}
}

void EdenGame::makeMatriceFix() {
	int16 rotAngleTheta = _rotationAngleX;
	int16 rotAnglePhi = _rotationAngleY;
	int16 rotAnglePsi = _rotationAngleZ;

	_passMat31 = (_cosTable[rotAnglePhi] * _cosTable[rotAngleTheta]) >> 8;
	_passMat32 = (_sinTable[rotAnglePhi] * _cosTable[rotAngleTheta]) >> 8;
	_passMat33 = -_sinTable[rotAngleTheta];
	_passMat21 = ((-_sinTable[rotAnglePhi] * _cosTable[rotAnglePsi]) >> 8)
	              + ((_sinTable[rotAnglePsi] * ((_cosTable[rotAnglePhi] * _sinTable[rotAngleTheta]) >> 8)) >> 8);
	_passMat22 = ((_cosTable[rotAnglePhi] * _cosTable[rotAnglePsi]) >> 8)
	              + ((_sinTable[rotAnglePsi] * ((_sinTable[rotAnglePhi] * _sinTable[rotAngleTheta]) >> 8)) >> 8);
	_passMat23 = (_cosTable[rotAngleTheta] * _sinTable[rotAnglePsi]) >> 8;
	_passMat11 = ((_sinTable[rotAnglePhi] * _sinTable[rotAnglePsi]) >> 8)
	              + ((_cosTable[rotAnglePsi] * ((_cosTable[rotAnglePhi] * _sinTable[rotAngleTheta]) >> 8)) >> 8);
	_passMat12 = ((-_cosTable[rotAnglePhi] * _sinTable[rotAnglePsi]) >> 8)
	              + ((_cosTable[rotAnglePsi] * ((_sinTable[rotAnglePhi] * _sinTable[rotAngleTheta]) >> 8)) >> 8);
	_passMat13 = (_cosTable[rotAngleTheta] * _cosTable[rotAnglePsi]) >> 8;
}

void EdenGame::projectionFix(Cube *cubep, int n) {
	for (int i = 0; i < n; i++) {
		int x = cubep->_vertices[i].x;
		int y = cubep->_vertices[i].y;
		int z = cubep->_vertices[i].z;

		int transformX = _passMat31 * x + _passMat21 * y + _passMat11 * z + (int)(_translationX * 256.0F);
		int transformY = _passMat32 * x + _passMat22 * y + _passMat12 * z + (int)(_translationY * 256.0F);
		int transformZ = _passMat33 * x + _passMat23 * y + _passMat13 * z + (int)(_translationZ * 256.0F);

		transformZ >>= 8;
		if (transformZ == -256)
			transformZ++;
		cubep->_projection[i].x = transformX / (transformZ + 256) + _cursorPosX + 14 + _scrollPos;
		cubep->_projection[i].y = transformY / (transformZ + 256) + _cursorPosY + 14;
		cubep->_projection[i].z = transformZ;
	}
}

// Original name init_cube
void EdenGame::initCubeMac() {
	loadMap(2493, _cubeTexture);
	NEWcharge_objet_mob(&_cube, 2494, _cubeTexture);
	initSinCosTable();
}

void EdenGame::engineMac() {
	Eden_dep_and_rot();
	makeMatriceFix();
	projectionFix(&_cube, _cubeFaces);
	displayObject(&_cube);
}

// Original name: affiche_objet
void EdenGame::displayObject(Cube *cubep) {
	for (int i = 0; i < cubep->_num; i++)
		displayPolygoneMapping(cubep, cubep->_faces[i]);
}

// Original name: NEWcharge_map
void EdenGame::loadMap(int file_id, byte *buffer) {
	if (_vm->getPlatform() == Common::kPlatformMacintosh) {
		loadpartoffile(file_id, buffer, 32, 256 * 3);

		_graphics->setPaletteColor(buffer);

		loadpartoffile(file_id, buffer, 32 + 256 * 3, 0x4000);
	} else {
#if 0
// Fake Mac cursor on PC
			Common::File f;
			if (f.open("curs.raw")) {
				f.seek(32);
				f.read(buffer, 256 * 3);

				for (i = 0; i < 256; i++) {
					color3_t color;
					color.r = buffer[i * 3] << 8;
					color.g = buffer[i * 3 + 1] << 8;
					color.b = buffer[i * 3 + 2] << 8;
					CLPalette_SetRGBColor(global_palette, i, &color);
				}
				CLPalette_Send2Screen(global_palette, 0, 256);

				f.read(buffer, 0x4000);

				f.close();
			}
			else
				error("can not load cursor texture");
#endif
	}
}

void EdenGame::NEWcharge_objet_mob(Cube *cubep, int fileNum, byte *texturePtr) {
	char *tmp1 = new char[454];
	if (_vm->getPlatform() == Common::kPlatformMacintosh)
		loadpartoffile(fileNum, tmp1, 0, 454);
	else {
#if 0
// Fake Mac cursor on PC
		Common::File f;
		if (f.open("curseden.mob")) {
			f.read(tmp1, 454);
			f.close();
		}
		else
			::error("can not load cursor model");
#endif
	}

	char *next = tmp1;
	char error;
	_cubeFaces = nextVal(&next, &error);
	Point3D *vertices = (Point3D *)malloc(_cubeFaces * sizeof(*vertices));
	Point3D *projection = (Point3D *)malloc(_cubeFaces * sizeof(*projection));
	for (int i = 0; i < _cubeFaces; i++) {
		vertices[i].x = nextVal(&next, &error);
		vertices[i].y = nextVal(&next, &error);
		vertices[i].z = nextVal(&next, &error);
	}
	int count2 = nextVal(&next, &error);
	CubeFace **tmp4 = (CubeFace **)malloc(count2 * sizeof(*tmp4));
	for (int i = 0; i < count2; i++) {
		tmp4[i] = (CubeFace *)malloc(sizeof(CubeFace));
		tmp4[i]->tri = 3;
		char textured = nextVal(&next, &error);
		tmp4[i]->ff_5 = nextVal(&next, &error);
		tmp4[i]->_indices = (uint16 *)malloc(3 * sizeof(*tmp4[i]->_indices));
		tmp4[i]->_uv = (int16 *)malloc(3 * 2 * sizeof(*tmp4[i]->_uv));
		for (int j = 0; j < 3; j++) {
			tmp4[i]->_indices[j] = nextVal(&next, &error);
			if (textured) {
				tmp4[i]->_uv[j * 2] = nextVal(&next, &error);
				tmp4[i]->_uv[j * 2 + 1] = nextVal(&next, &error);
			}
		}
		if (textured) {
			tmp4[i]->ff_4 = 3;
			tmp4[i]->_texturePtr = texturePtr;
		} else
			tmp4[i]->ff_4 = 0;
	}
	delete[] tmp1;
	cubep->_num = count2;
	cubep->_faces = tmp4;
	cubep->_projection = projection;
	cubep->_vertices = vertices;
}

int EdenGame::nextVal(char **ptr, char *error) {
	char c = 0;
	char *p = *ptr;
	int val = strtol(p, 0, 10);
	while ((*p >= '0' && *p <= '9' && *p != 0) || *p == '-')
		p++;
	while ((*p == 13 || *p == 10 || *p == ',' || *p == ' ') && *p)
		c = *p++;
	*error = c == 10;
	*ptr = p;
	return val;
}

void EdenGame::selectMap(int16 num) {
	_cursCurPCMap = num;
	int16 k = 0;
	int mode = _mapMode[num];
	int16 x = (num & 7) * 32;
	int16 y = (num & 0x18) * 4;
	for (int i = 0; i < 6 * 2; i++) {
		for (int j = 0; j < 3; j++) {
			_cube._faces[i]->_uv[j * 2    ] = x + _cubeTextureCoords[mode][k++];
			_cube._faces[i]->_uv[j * 2 + 1] = y + _cubeTextureCoords[mode][k++];
		}
	}
}

void EdenGame::Eden_dep_and_rot() {
	int16 curs = _currCursor;
	if (_normalCursor && (_globals->_drawFlags & DrawFlags::drDrawFlag20))
		curs = 10;
	selectMap(curs);
	_cursorNewTick = g_system->getMillis();
	if (_cursorNewTick - _cursorOldTick < 1)
		return;

	_cursorOldTick = _cursorNewTick;
	switch (_currCursor) {
	case 0:
		_rotationAngleZ = (_rotationAngleZ + 2) % 360;
		_rotationAngleX = (_rotationAngleX + 2) % 360;
		restoreZDEP();
		break;
	case 1:
		_rotationAngleZ = 0;
		_rotationAngleX -= 2;
		if (_rotationAngleX < 0)
			_rotationAngleX += 360;
		restoreZDEP();
		break;
	case 2:
		_rotationAngleZ = (_rotationAngleZ + 2) % 360;
		_rotationAngleX = 0;
		restoreZDEP();
		break;
	case 3:
		_rotationAngleZ -= 2;
		if (_rotationAngleZ < 0)
			_rotationAngleZ += 360;
		_rotationAngleX = 0;
		restoreZDEP();
		break;
	case 4:
		_rotationAngleZ = 0;
		_rotationAngleX = (_rotationAngleX + 2) % 360;
		restoreZDEP();
		break;
	case 5:
		_rotationAngleZ = 0;
		_rotationAngleX = 0;
		_translationZ += _zDirection * Z_STEP;
		if ((_translationZ < -3600 + Z_RESET) || _translationZ > Z_RESET)
			_zDirection = -_zDirection;
		break;
	case 6:
		_rotationAngleZ = 0;
		_rotationAngleX = 0;
		_translationZ = Z_RESET;
		break;
	case 7:
		_rotationAngleZ -= 2;
		if (_rotationAngleZ < 0)
			_rotationAngleZ += 360;
		_rotationAngleX = 0;
		restoreZDEP();
		break;
	case 8:
		_rotationAngleZ = 0;
		_rotationAngleX = 0;
		_translationZ = Z_RESET;
		break;
	case 9:
		_rotationAngleZ = 0;
		_rotationAngleX = 0;
		_translationZ = Z_RESET;
		break;
	}
}

void EdenGame::restoreZDEP() {
	_zDirection = Z_UP;
	if (_translationZ < Z_RESET)
		_translationZ += _zDirection * Z_STEP;
	if (_translationZ > Z_RESET)
		_translationZ -= _zDirection * Z_STEP;
}

// Original name: affiche_polygone_mapping
void EdenGame::displayPolygoneMapping(Cube *cubep, CubeFace *face) {
	uint16 *indices = face->_indices;
	int idx = indices[0];
	int16 projX0 = cubep->_projection[idx].x;
	int16 projY0 = cubep->_projection[idx].y;

	idx = indices[1];
	int16 projX1 = cubep->_projection[idx].x;
	int16 projY1 = cubep->_projection[idx].y;

	idx = indices[2];
	int16 projX2 = cubep->_projection[idx].x;
	int16 projY2 = cubep->_projection[idx].y;

	if ((projY1 - projY0) * (projX2 - projX0) - (projY2 - projY0) * (projX1 - projX0) > 0)
		return;

	int16 *uv = face->_uv;
	int16 ymin = 200; // min y
	int16 ymax = 0;   // max y
	idx = indices[0];
	int16 r20 = cubep->_projection[idx].x;
	int16 r30 = cubep->_projection[idx].y;
	int16 r19 = *uv++;
	int16 r18 = *uv++;
	indices++;
	for (int i = 0; i < face->tri - 1; i++, indices++) {
		idx = indices[0];
		int16 r26 = cubep->_projection[idx].x;
		int16 r31 = cubep->_projection[idx].y;
		uint16 r25 = *uv++;    //TODO: unsigned
		int16 r24 = *uv++;    //TODO: unsigned
		ymin = MIN(r30, ymin);
		ymax = MAX(r30, ymax);
		ymin = MIN(r31, ymin);
		ymax = MAX(r31, ymax);
		drawMappingLine(r20, r30, r26, r31, r19, r18, r25, r24, _lines);
		r20 = r26;
		r30 = r31;
		r19 = r25;
		r18 = r24;
	}
	idx = face->_indices[0];
	int16 r26 = cubep->_projection[idx].x;
	int16 r31 = cubep->_projection[idx].y;
	uv = face->_uv;
	uint16 r25 = *uv++;
	int16 r24 = *uv;
	ymin = MIN(r30, ymin);
	ymax = MAX(r30, ymax);
	ymin = MIN(r31, ymin);
	ymax = MAX(r31, ymax);
	drawMappingLine(r20, r30, r26, r31, r19, r18, r25, r24, _lines);
	displayMappingLine(ymin, ymax, _graphics->getMainView()->_bufferPtr, face->_texturePtr);
}

// Original name: trace_ligne_mapping
void EdenGame::drawMappingLine(int16 r3, int16 r4, int16 r5, int16 r6, int16 r7, int16 r8, int16 r9, int16 r10, int16 *linesp) {
	int16 r26 = r6 - r4;
	if (r26 <= 0) {
		if (r26 == 0) {
			linesp += r4 * 8;
			if (r5 - r3 > 0) {
				linesp[0] = r3;
				linesp[1] = r5;
				linesp[4] = r7;
				linesp[5] = r9;
				linesp[6] = r8;
				linesp[7] = r10;
			} else {
				linesp[0] = r5;
				linesp[1] = r3;
				linesp[4] = r9;
				linesp[5] = r7;
				linesp[6] = r10;
				linesp[7] = r8;
			}
			return;
		}
		int16 t = r3;
		r3 = r5;
		r5 = t;
		t = r7;
		r7 = r9;
		r9 = t;
		t = r8;
		r8 = r10;
		r10 = t;
		linesp += r6 * 8;
		r26 = -r26;
	} else
		linesp += r4 * 8 + 1;    //TODO wha???

	int r30 = r3 << 16;
	int r29 = r7 << 16;
	int r28 = r8 << 16;

	int r25 = ((r5 - r3) << 16) / r26;
	int r24 = ((r9 - r7) << 16) / r26;
	int r23 = ((r10 - r8) << 16) / r26;

	for (int i = 0; i < r26; i++) {
		linesp[0] = r30 >> 16;
		linesp[4] = r29 >> 16;
		linesp[6] = r28 >> 16;

		r30 += r25;
		r29 += r24;
		r28 += r23;
		linesp += 8;
	}
}

// Original name: affiche_ligne_mapping
void EdenGame::displayMappingLine(int16 r3, int16 r4, byte *target, byte *texture) {
	int16 height = r4 - r3;
	byte *trg_line = _graphics->getMainView()->_bufferPtr + r3 * 640;    //TODO: target??
	int16 *line = &_lines[r3 * 8];
	//	debug("curs: beg draw %d - %d", r3, r4);
	for (int r22 = height; r22; r22--, line += 8, trg_line += 640) {
		int16 r29 = line[0];
		int16 r28 = line[1];
		int16 len = r28 - r29;
		if (len < 0)
			break;
		if (len == 0)
			continue;

		//	debug("curs: lin draw %d", r4 - height);
		uint16 r31 = line[4] << 8;
		uint16 r30 = line[6] << 8;

		int16 r21 = line[5] - line[4];
		int16 r20 = line[7] - line[6];

		int16 r26 = (r21 << 8) / len;
		int16 r25 = (r20 << 8) / len;
		byte *trg = trg_line + r29;
#if 1
		while (r29++ < r28) {
			*trg++ = texture[(r30 & 0xFF00) | (r31 >> 8)];
			r31 += r26;
			r30 += r25;
		}
#endif
	}
}

// PC cursor
CubeCursor _cursorsPC[9] = {
		{ { 0, 0, 0, 0, 0, 0 }, 3, 2 },
		{ { 1, 1, 0, 1, 1, 0 }, 2, -2 },
		{ { 2, 2, 2, 2, 2, 2 }, 1, 2 },
		{ { 3, 3, 3, 3, 3, 3 }, 1, -2 },
		{ { 4, 4, 4, 4, 4, 4 }, 2, 2 },
		{ { 5, 5, 5, 5, 5, 5 }, 4, 0 },
		{ { 6, 6, 6, 6, 6, 6 }, 1, 2 },
		{ { 7, 7, 7, 7, 7, 7 }, 1, -2 },
//		{ { 0, 8, 0, 0, 8, 8 }, 2, 2 },
		{ { 0, 8, 0, 0, 8, 8 }, 2, 2 }
};

XYZ _cubePC[6][3] = {
		{ { -15, -15, -15 }, { -15, 15, -15 }, { 15, 15, -15 } },
		{ { -15, -15, 15 }, { -15, 15, 15 }, { -15, 15, -15 } },
		{ { -15, -15, 15 }, { -15, -15, -15 }, { 15, -15, -15 } },
		{ { 15, -15, 15 }, { 15, 15, 15 }, { -15, 15, 15 } },
		{ { 15, -15, -15 }, { 15, 15, -15 }, { 15, 15, 15 } },
		{ { 15, 15, 15 }, { 15, 15, -15 }, { -15, 15, -15 } }
};

signed short cosineTable[] = {
	// = cos(n) << 7; n += 10;
	128, 126, 120, 111, 98, 82, 64, 44, 22, 0, -22, -44, -64, -82, -98, -111, -120, -126,
	-128, -126, -120, -111, -98, -82, -64, -44, -22, 0, 22, 44, 64, 82, 98, 111, 120, 126,
	128, 126, 120, 111, 98, 82, 64, 44, 22, 0
};

void EdenGame::makeTables() {
	for (int i = -15; i < 15; i++) {
		int v = (i * 11) / 15 + 11;
		tab1[i + 15] = v;
		tab2[i + 15] = v * 22;
	}

	for (int i = 0; i < 36; i++) {
		for (int j = -35; j < 36; j++)
			tab3[i][j + 35] = (cosineTable[i] * j) >> 7;
	}
}

void EdenGame::getSinCosTables(unsigned short angle, signed char **cos_table, signed char **sin_table) {
	angle /= 2;
	*cos_table = tab3[angle] + 35;

	angle += 9;
	if (angle >= 36)
		angle -= 36;

	*sin_table = tab3[angle] + 35;
}


void EdenGame::rotatePoint(XYZ *point, XYZ *rpoint) {
	// see http://www.cprogramming.com/tutorial/3d/rotation.html
	XYZ xrot;

	xrot.x = point->x;
	xrot.y = _cosX[point->y] + _sinX[point->z];
	xrot.z = _sinX[-point->y] + _cosX[point->z];

	rpoint->x = _cosY[xrot.x] + _sinY[-xrot.z];
	rpoint->y = xrot.y;
	rpoint->z = _sinY[xrot.x] + _cosY[xrot.z];

	rpoint->z += _zoomZ;
}

void EdenGame::mapPoint(XYZ *point, short *x, short *y) {
	*y = ((12800 / point->z) * point->y) >> 7;
	*x = ((12800 / point->z) * point->x) >> 7;
}

short EdenGame::calcFaceArea(XYZ *face) {
	XYZ rpoint;
	short x[3], y[3];

	for (int i = 0; i < 3; i++) {
		rotatePoint(&face[i], &rpoint);
		mapPoint(&rpoint, &x[i], &y[i]);
	}

	short area = (y[1] - y[0]) * (x[2] - x[0]) - (y[2] - y[0]) * (x[1] - x[0]);

	return area;
}

void EdenGame::paintPixel(XYZ *point, unsigned char pixel) {
	short x, y;
	mapPoint(point, &x, &y);
	_cursorCenter[y * 40 + x] = pixel;
}

void EdenGame::paintFace0(XYZ *point) {
	XYZ rpoint;
	for (int y = -15; y < 15; y++) {
		for (int x = -15; x < 15; x++) {
			point->x = x;
			point->y = y;
			rotatePoint(point, &rpoint);
			paintPixel(&rpoint, _face[0][tab1[x + 15] + tab2[y + 15]]);
		}
	}
}

void EdenGame::paintFace1(XYZ *point) {
	XYZ rpoint;
	for (int y = -15; y < 15; y++) {
		for (int x = -15; x < 15; x++) {
			point->y = y;
			point->z = -x;
			rotatePoint(point, &rpoint);
			paintPixel(&rpoint, _face[1][tab1[x + 15] + tab2[y + 15]]);
		}
	}
}

void EdenGame::paintFace2(XYZ *point) {
	XYZ rpoint;
	for (int y = -15; y < 15; y++) {
		for (int x = -15; x < 15; x++) {
			point->x = x;
			point->z = -y;
			rotatePoint(point, &rpoint);
			paintPixel(&rpoint, _face[2][tab1[x + 15] + tab2[y + 15]]);
		}
	}
}

void EdenGame::paintFace3(XYZ *point) {
	XYZ rpoint;
	for (int y = -15; y < 15; y++) {
		for (int x = -15; x < 15; x++) {
			point->x = -x;
			point->y = -y;
			rotatePoint(point, &rpoint);
			paintPixel(&rpoint, _face[3][tab1[x + 15] + tab2[y + 15]]);
		}
	}
}

void EdenGame::paintFace4(XYZ *point) {
	XYZ rpoint;
	for (int y = -15; y < 15; y++) {
		for (int x = -15; x < 15; x++) {
			point->y = y;
			point->z = x;
			rotatePoint(point, &rpoint);
			paintPixel(&rpoint, _face[4][tab1[x + 15] + tab2[y + 15]]);
		}
	}
}

void EdenGame::paintFace5(XYZ *point) {
	XYZ rpoint;
	for (int y = -15; y < 15; y++) {
		for (int x = -15; x < 15; x++) {
			point->x = x;
			point->z = y;
			rotatePoint(point, &rpoint);
			paintPixel(&rpoint, _face[5][tab1[x + 15] + tab2[y + 15]]);
		}
	}
}

void EdenGame::paintFaces() {
	XYZ point;
	if (!(_faceSkip & 1)) {
		point.z = -15;
		paintFace0(&point);
	}
	if (!(_faceSkip & 2)) {
		point.x = -15;
		paintFace1(&point);
	}
	if (!(_faceSkip & 4)) {
		point.y = -15;
		paintFace2(&point);
	}
	if (!(_faceSkip & 8)) {
		point.z = 15;
		paintFace3(&point);
	}
	if (!(_faceSkip & 16)) {
		point.x = 15;
		paintFace4(&point);
	}
	if (!(_faceSkip & 32)) {
		point.y = 15;
		paintFace5(&point);
	}
}

void EdenGame::renderCube() {
	for (uint16 i = 0; i < sizeof(_cursor); i++)
		_cursor[i] = 0;
	_cursorCenter = &_cursor[40 * 20 + 20];

	getSinCosTables(_angleX, &_cosX, &_sinX);
	getSinCosTables(_angleY, &_cosY, &_sinY);
	getSinCosTables(_angleZ, &_cosZ, &_sinZ);

	for (int i = 0; i < 6; i++) {
		int area = calcFaceArea(_cubePC[i]);
		if (area <= 0) {
			_face[i] = _newface[i];	// set new texture for invisible area,
			_faceSkip |= 1 << i;	// but don't draw it just yet
		} else
			_faceSkip &= ~(1 << i);
	}

	paintFaces();

	const int xshift = -5;		// TODO: temporary fix to decrease left margin
	unsigned char *cur = _cursor;
	unsigned char *scr = _graphics->getMainView()->_bufferPtr + _cursorPosX + _scrollPos  + xshift + _cursorPosY * _graphics->getMainView()->_pitch;

	for (int y = 0; y < 40; y++) {
		for (int x = 0; x < 40; x++) {
			if (x + _cursorPosX + _scrollPos + xshift < _graphics->getMainView()->_pitch && y + _cursorPosY < _graphics->getMainView()->_height)
				if (*cur)
					*scr = *cur;
			scr++;
			cur++;
		}
		scr += _graphics->getMainView()->_pitch - 40;
	}
}


void EdenGame::incAngleX(int step) {
	_angleX += step;
	if (_angleX == 70 + 2)
		_angleX = 0;
	else if (_angleX == 0 - 2)
		_angleX = 70;
}

void EdenGame::decAngleX() {
	if (_angleX != 0)
		_angleX -= (_angleX > 4) ? 4 : 2;
}

void EdenGame::incAngleY(int step) {
	_angleY += step;
	if (_angleY == 70 + 2)
		_angleY = 0;
	else if (_angleY == 0 - 2)
		_angleY = 70;
}

void EdenGame::decAngleY() {
	if (_angleY != 0)
		_angleY -= (_angleY > 4) ? 4 : 2;
}

void EdenGame::incZoom() {
	if (_zoomZ == 170)
		_zoomZStep = 40;
	else if (_zoomZ == 570)
		_zoomZStep = -40;
	_zoomZ += _zoomZStep;
}

void EdenGame::decZoom() {
	if (_zoomZ == 170)
		return;

	if (_zoomZ < 170)
		_zoomZ = 170;
	else
		_zoomZ -= 40;
}

void EdenGame::initCubePC() {
	_zoomZ = 170;
	_zoomZStep = 40;
	_angleX = _angleY = _angleZ = 0;
	_pcCursor = &_cursorsPC[0];
	_cursCurPCMap = -1;
	makeTables();
}

void EdenGame::selectPCMap(int16 num) {
	if (num != _cursCurPCMap) {
		_pcCursor = &_cursorsPC[num];
		unsigned char *bank = _mainBankBuf + READ_LE_UINT16(_mainBankBuf);
		for (int i = 0; i < 6; i++) {
			_newface[i] = 4 + (unsigned char*)getElem(bank, _pcCursor->_sides[i]);
			if (_cursCurPCMap == -1)
				_face[i] = _newface[i];
		}
		_cursCurPCMap = num;
	}
}

void EdenGame::enginePC() {
	int16 curs = _currCursor;
	if (_normalCursor && (_globals->_drawFlags & DrawFlags::drDrawFlag20))
		curs = 9;
	selectPCMap(curs);
	_cursorNewTick = g_system->getMillis();
	if (_cursorNewTick - _cursorOldTick < 1)
		return;
	_cursorOldTick = _cursorNewTick;
	int step = _pcCursor->_speed;
	switch (_pcCursor->_kind) {
	case 0:
		break;
	case 1:	// rot up-down
		decAngleY();
		decZoom();
		incAngleX(step);
		break;
	case 2: // rot left-right
		decAngleX();
		decZoom();
		incAngleY(step);
		break;
	case 3: // rotate random
		decZoom();
		incAngleX(step);
		incAngleY(step);
		break;
	case 4: // zoom in-out
		_face[0] = _newface[0];
		decAngleY();
		decAngleX();
		incZoom();
		break;
	}
	renderCube();
}

void EdenGame::LostEdenMac_InitPrefs() {
	_globals->_prefLanguage = 1;
	_globals->_prefMusicVol[0] = 192;
	_globals->_prefMusicVol[1] = 192;
	_globals->_prefVoiceVol[0] = 255;
	_globals->_prefVoiceVol[1] = 255;
	_globals->_prefSoundVolume[0] = 32;
	_globals->_prefSoundVolume[1] = 32;
}

}   // namespace Cryo
