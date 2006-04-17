/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "kyra/kyra.h"
#include "kyra/seqplayer.h"
#include "kyra/screen.h"
#include "kyra/resource.h"
#include "kyra/sound.h"
#include "kyra/sprites.h"
#include "kyra/wsamovie.h"
#include "kyra/animator.h"
#include "kyra/text.h"

#include "common/system.h"
#include "common/savefile.h"

namespace Kyra {

void KyraEngine::seq_demo() {
	debugC(9, kDebugLevelMain, "KyraEngine::seq_demo()");

	snd_playTheme(MUSIC_INTRO, 2);

	loadBitmap("START.CPS", 7, 7, _screen->_currentPalette);
	_screen->copyRegion(0, 0, 0, 0, 320, 200, 6, 0);
	_system->copyRectToScreen(_screen->getPagePtr(0), 320, 0, 0, 320, 200);
	_screen->fadeFromBlack();
	delay(60 * _tickLength);
	_screen->fadeToBlack();

	_screen->clearPage(0);
	loadBitmap("TOP.CPS", 7, 7, NULL);
	loadBitmap("BOTTOM.CPS", 5, 5, _screen->_currentPalette);
	_screen->copyRegion(0, 91, 0, 8, 320, 103, 6, 0);
	_screen->copyRegion(0, 0, 0, 111, 320, 64, 6, 0);
	_system->copyRectToScreen(_screen->getPagePtr(0), 320, 0, 0, 320, 200);
	_screen->fadeFromBlack();
	
	_seq->playSequence(_seq_WestwoodLogo, true);
	delay(60 * _tickLength);
	_seq->playSequence(_seq_KyrandiaLogo, true);

	_screen->fadeToBlack();
	_screen->clearPage(2);
	_screen->clearPage(0);

	_seq->playSequence(_seq_Demo1, true);

	_screen->clearPage(0);
	_seq->playSequence(_seq_Demo2, true);

	_screen->clearPage(0);
	_seq->playSequence(_seq_Demo3, true);

	_screen->clearPage(0);
	_seq->playSequence(_seq_Demo4, true);

	_screen->clearPage(0);
	loadBitmap("FINAL.CPS", 7, 7, _screen->_currentPalette);
	_screen->_curPage = 0;
	_screen->copyRegion(0, 0, 0, 0, 320, 200, 6, 0);
	_system->copyRectToScreen(_screen->getPagePtr(0), 320, 0, 0, 320, 200);
	_screen->fadeFromBlack();
	delay(60 * _tickLength);
	_screen->fadeToBlack();
	_sound->haltTrack();
}

void KyraEngine::seq_intro() {
	debugC(9, kDebugLevelMain, "KyraEngine::seq_intro()");

	if (_features & GF_TALKIE) {
		_res->loadPakFile("INTRO.VRM");
	}
	
	static const IntroProc introProcTable[] = {
		&KyraEngine::seq_introLogos,
		&KyraEngine::seq_introStory,
		&KyraEngine::seq_introMalcolmTree,
		&KyraEngine::seq_introKallakWriting,
		&KyraEngine::seq_introKallakMalcolm
	};

	Common::InSaveFile *in;
	if ((in = _saveFileMan->openForLoading(getSavegameFilename(0)))) {
		delete in;
		_skipIntroFlag = true;
	} else
		_skipIntroFlag = false;

	_seq->setCopyViewOffs(true);
	_screen->setFont(Screen::FID_8_FNT);
	snd_playTheme(MUSIC_INTRO, 2);
	_text->setTalkCoords(144);
	for (int i = 0; i < ARRAYSIZE(introProcTable) && !seq_skipSequence(); ++i) {
		(this->*introProcTable[i])();
	}
	_text->setTalkCoords(136);
	delay(30 * _tickLength);
	_seq->setCopyViewOffs(false);
	_sound->haltTrack();
	if (_features & GF_TALKIE) {
		_res->unloadPakFile("INTRO.VRM");
	}
}

void KyraEngine::seq_introLogos() {
	debugC(9, kDebugLevelMain, "KyraEngine::seq_introLogos()");
	_screen->clearPage(0);
	loadBitmap("TOP.CPS", 7, 7, NULL);
	loadBitmap("BOTTOM.CPS", 5, 5, _screen->_currentPalette);
	_screen->_curPage = 0;
	_screen->copyRegion(0, 91, 0, 8, 320, 103, 6, 0);
	_screen->copyRegion(0, 0, 0, 111, 320, 64, 6, 0);
	_system->copyRectToScreen(_screen->getPagePtr(0), 320, 0, 0, 320, 200);
	_screen->fadeFromBlack();
	
	if (_seq->playSequence(_seq_WestwoodLogo, _skipFlag)) {
		_screen->fadeToBlack();
		_screen->clearPage(0);
		return;
	}
	delay(60 * _tickLength);
	if (_seq->playSequence(_seq_KyrandiaLogo, _skipFlag) && !seq_skipSequence()) {
		_screen->fadeToBlack();
		_screen->clearPage(0);
		return;
	}
	_screen->fillRect(0, 179, 319, 199, 0);

	int y1 = 8;
	int h1 = 175;
	int y2 = 176;
	int h2 = 0;
	int32 start, now;
	int wait;
	_screen->copyRegion(0, 91, 0, 8, 320, 103, 6, 2);
	_screen->copyRegion(0, 0, 0, 111, 320, 64, 6, 2);
	do {
		start = (int32)_system->getMillis();
		if (h1 > 0) {
			_screen->copyRegion(0, y1, 0, 8, 320, h1, 2, 0);
		}
		++y1;
		--h1;
		if (h2 > 0) {
			_screen->copyRegion(0, 64, 0, y2, 320, h2, 4, 0);
		}
		--y2;
		++h2;
		_screen->updateScreen();
		now = (int32)_system->getMillis();
		wait = _tickLength - (now - start);
		if (wait > 0) {
			delay(wait);
		}
	} while (y2 >= 64);

	_seq->playSequence(_seq_Forest, true);
}

void KyraEngine::seq_introStory() {
	debugC(9, kDebugLevelMain, "KyraEngine::seq_introStory()");
	_screen->clearPage(3);
	_screen->clearPage(0);
	if (_features & GF_TALKIE) {
		return;
	} else if (_features & GF_ENGLISH) {
		loadBitmap("TEXT.CPS", 3, 3, 0);
	} else if (_features & GF_GERMAN) {
		loadBitmap("TEXT_GER.CPS", 3, 3, 0);
	} else if (_features & GF_FRENCH) {
		loadBitmap("TEXT_FRE.CPS", 3, 3, 0);
	} else if (_features & GF_SPANISH) {
		loadBitmap("TEXT_SPA.CPS", 3, 3, 0);
	} else {
		warning("no story graphics file found");
	}
	_screen->copyRegion(0, 0, 0, 0, 320, 200, 3, 0);
	_screen->updateScreen();
	//debugC(0, kDebugLevelMain, "skipFlag %i, %i", _skipFlag, _tickLength);
	delay(360 * _tickLength);
}

void KyraEngine::seq_introMalcolmTree() {
	debugC(9, kDebugLevelMain, "KyraEngine::seq_introMalcolmTree()");
	_screen->_curPage = 0;
	_screen->clearPage(3);
	_seq->playSequence(_seq_MalcolmTree, true);
}

void KyraEngine::seq_introKallakWriting() {
	debugC(9, kDebugLevelMain, "KyraEngine::seq_introKallakWriting()");
	_seq->makeHandShapes();
	_screen->setAnimBlockPtr(5060);
	_screen->_charWidth = -2;
	_screen->clearPage(3);
	_seq->playSequence(_seq_KallakWriting, true);
}

void KyraEngine::seq_introKallakMalcolm() {
	debugC(9, kDebugLevelMain, "KyraEngine::seq_introKallakMalcolm()");
	_screen->clearPage(3);
	_seq->playSequence(_seq_KallakMalcolm, true);
}

void KyraEngine::seq_createAmuletJewel(int jewel, int page, int noSound, int drawOnly) {
	debugC(9, kDebugLevelMain, "seq_createAmuletJewel(%d, %d, %d, %d)", jewel, page, noSound, drawOnly);
	static const uint16 specialJewelTable[] = {
		0x167, 0x162, 0x15D, 0x158, 0x153, 0xFFFF
	};
	static const uint16 specialJewelTable1[] = {
		0x14F, 0x154, 0x159, 0x15E, 0x163, 0xFFFF
	};
	static const uint16 specialJewelTable2[] = {
		0x150, 0x155, 0x15A, 0x15F, 0x164, 0xFFFF
	};
	static const uint16 specialJewelTable3[] = {
		0x151, 0x156, 0x15B, 0x160, 0x165, 0xFFFF
	};
	static const uint16 specialJewelTable4[] = {
		0x152, 0x157, 0x15C, 0x161, 0x166, 0xFFFF
	};
	if (!noSound)
		snd_playSoundEffect(0x5F);
	_screen->hideMouse();
	if (!drawOnly) {
		for (int i = 0; specialJewelTable[i] != 0xFFFF; ++i) {
			_screen->drawShape(page, _shapes[4+specialJewelTable[i]], _amuletX2[jewel], _amuletY2[jewel], 0, 0);
			_screen->updateScreen();
			delayWithTicks(3);
		}
		
		const uint16 *opcodes = 0;
		switch (jewel - 1) {
			case 0:
				opcodes = specialJewelTable1;
				break;
				
			case 1:
				opcodes = specialJewelTable2;
				break;
				
			case 2:
				opcodes = specialJewelTable3;
				break;
				
			case 3:
				opcodes = specialJewelTable4;
				break;
		}
		
		if (opcodes) {
			for (int i = 0; opcodes[i] != 0xFFFF; ++i) {
				_screen->drawShape(page, _shapes[4+opcodes[i]], _amuletX2[jewel], _amuletY2[jewel], 0, 0);
				_screen->updateScreen();
				delayWithTicks(3);
			}
		}
	}
	_screen->drawShape(page, _shapes[327+jewel], _amuletX2[jewel], _amuletY2[jewel], 0, 0);
	_screen->updateScreen();
	_screen->showMouse();
	setGameFlag(0x55+jewel);
}

void KyraEngine::seq_brandonHealing() {
	debugC(9, kDebugLevelMain, "seq_brandonHealing()");
	if (!(_deathHandler & 8))
		return;
	if (_currentCharacter->sceneId == 210) {
		if (_beadStateVar == 4 || _beadStateVar == 6)
			return;
	}
	_screen->hideMouse();
	checkAmuletAnimFlags();
	assert(_healingShapeTable);
	setupShapes123(_healingShapeTable, 22, 0);
	_animator->setBrandonAnimSeqSize(3, 48);
	snd_playSoundEffect(0x53);
	for (int i = 123; i <= 144; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}
	for (int i = 125; i >= 123; --i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}
	_animator->resetBrandonAnimSeqSize();
	_currentCharacter->currentAnimFrame = 7;
	_animator->animRefreshNPC(0);
	freeShapes123();
	_screen->showMouse();
}

void KyraEngine::seq_brandonHealing2() {
	debugC(9, kDebugLevelMain, "seq_brandonHealing2()");
	_screen->hideMouse();
	checkAmuletAnimFlags();
	assert(_healingShape2Table);
	setupShapes123(_healingShape2Table, 30, 0);
	resetBrandonPoisonFlags();
	_animator->setBrandonAnimSeqSize(3, 48);
	snd_playSoundEffect(0x50);
	for (int i = 123; i <= 152; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}
	_animator->resetBrandonAnimSeqSize();
	_currentCharacter->currentAnimFrame = 7;
	_animator->animRefreshNPC(0);
	freeShapes123();
	_screen->showMouse();
	assert(_poisonGone);
	if (_features & GF_TALKIE) {
		snd_voiceWaitForFinish();
		snd_playVoiceFile(2010);
	}
	characterSays(_poisonGone[0], 0, -2);
	if (_features & GF_TALKIE) {
		snd_voiceWaitForFinish();
		snd_playVoiceFile(2011);
	}
	characterSays(_poisonGone[1], 0, -2);
}

void KyraEngine::seq_poisonDeathNow(int now) {
	debugC(9, kDebugLevelMain, "seq_poisonDeathNow(%d)", now);
	if (!(_brandonStatusBit & 1))
		return;
	++_poisonDeathCounter;
	if (now)
		_poisonDeathCounter = 2;
	if (_poisonDeathCounter >= 2) {
		snd_playWanderScoreViaMap(1, 1);
		assert(_thePoison);
		if (_features & GF_TALKIE) {
			snd_voiceWaitForFinish();
			snd_playVoiceFile(7000);
		}
		characterSays(_thePoison[0], 0, -2);
		if (_features & GF_TALKIE) {
			snd_voiceWaitForFinish();
			snd_playVoiceFile(7001);
		}
		characterSays(_thePoison[1], 0, -2);
		seq_poisonDeathNowAnim();
		_deathHandler = 3;
	} else {
		assert(_thePoison);
		if (_features & GF_TALKIE) {
			snd_voiceWaitForFinish();
			snd_playVoiceFile(7002);
		}
		characterSays(_thePoison[2], 0, -2);
		if (_features & GF_TALKIE) {
			snd_voiceWaitForFinish();
			snd_playVoiceFile(7004);
		}
		characterSays(_thePoison[3], 0, -2);
	}
}

void KyraEngine::seq_poisonDeathNowAnim() {
	debugC(9, kDebugLevelMain, "seq_poisonDeathNowAnim()");
	_screen->hideMouse();
	checkAmuletAnimFlags();
	assert(_posionDeathShapeTable);
	setupShapes123(_posionDeathShapeTable, 20, 0);
	_animator->setBrandonAnimSeqSize(8, 48);
	
	_currentCharacter->currentAnimFrame = 124;
	_animator->animRefreshNPC(0);
	delayWithTicks(30);
	
	_currentCharacter->currentAnimFrame = 123;
	_animator->animRefreshNPC(0);
	delayWithTicks(30);
	
	for (int i = 125; i <= 139; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}
	
	delayWithTicks(60);
	
	for (int i = 140; i <= 142; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}
	
	delayWithTicks(60);
	
	_animator->resetBrandonAnimSeqSize();
	freeShapes123();
	_animator->restoreAllObjectBackgrounds();
	_currentCharacter->x1 = _currentCharacter->x2 = -1;
	_currentCharacter->y1 = _currentCharacter->y2 = -1;
	_animator->preserveAllBackgrounds();
	_screen->showMouse();
}

void KyraEngine::seq_playFluteAnimation() {
	debugC(9, kDebugLevelMain, "seq_playFluteAnimation()");
	_screen->hideMouse();
	checkAmuletAnimFlags();
	setupShapes123(_fluteAnimShapeTable, 36, 0);
	_animator->setBrandonAnimSeqSize(3, 75);
	for (int i = 123; i <= 130; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(2);
	}
	
	int delayTime = 0, soundType = 0;
	if (queryGameFlag(0x85)) {
		snd_playSoundEffect(0x63);
		delayTime = 9;
		soundType = 3;
	} else if (!queryGameFlag(0x86)) {
		snd_playSoundEffect(0x61);
		delayTime = 2;
		soundType = 1;
		setGameFlag(0x86);
	} else {
		snd_playSoundEffect(0x62);
		delayTime = 2;
		soundType = 2;
	}
	
	for (int i = 131; i <= 158; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(delayTime);
	}
	
	for (int i = 126; i >= 123; --i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(delayTime);
	}
	_animator->resetBrandonAnimSeqSize();
	_currentCharacter->currentAnimFrame = 7;
	_animator->animRefreshNPC(0);
	freeShapes123();
	_screen->showMouse();
	
	if (soundType == 1) {
		assert(_fluteString);
		if (_features & GF_TALKIE) {
			snd_voiceWaitForFinish();
			snd_playVoiceFile(1000);
		}
		characterSays(_fluteString[0], 0, -2);
	} else if (soundType == 2) {
		assert(_fluteString);
		if (_features & GF_TALKIE) {
			snd_voiceWaitForFinish();
			snd_playVoiceFile(1001);
		}
		characterSays(_fluteString[1], 0, -2);
	}
}

void KyraEngine::seq_winterScroll1() {
	debugC(9, kDebugLevelMain, "seq_winterScroll1()");
	_screen->hideMouse();
	checkAmuletAnimFlags();
	assert(_winterScrollTable);
	assert(_winterScroll1Table);
	assert(_winterScroll2Table);
	setupShapes123(_winterScrollTable, 7, 0);
	_animator->setBrandonAnimSeqSize(5, 66);
	
	for (int i = 123; i <= 129; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}
	
	freeShapes123();
	snd_playSoundEffect(0x20);

	uint8 numFrames, midpoint;
	if (_features & GF_TALKIE) {
		numFrames = 18;
		midpoint = 136;
	} else {
		numFrames = 35;
		midpoint = 147;
	}
	setupShapes123(_winterScroll1Table, numFrames, 0);
	for (int i = 123; i < midpoint; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}
	
	if (_currentCharacter->sceneId == 41 && !queryGameFlag(0xA2)) {
		snd_playSoundEffect(0x20);
		_sprites->_anims[0].play = false;
		_animator->sprites()[0].active = 0;
		_sprites->_anims[1].play = true;
		_animator->sprites()[1].active = 1;
		setGameFlag(0xA2);
	}
	
	for (int i = midpoint; i < 123 + numFrames; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}
	
	if (_currentCharacter->sceneId == 117 && !queryGameFlag(0xB3)) {
		for (int i = 0; i <= 7; ++i) {
			_sprites->_anims[i].play = false;
			_animator->sprites()[i].active = 0;
		}
		uint8 tmpPal[768];
		memcpy(tmpPal, _screen->_currentPalette, 768);
		memcpy(&tmpPal[684], palTable2()[0], 60);
		_screen->fadePalette(tmpPal, 72);
		memcpy(&_screen->_currentPalette[684], palTable2()[0], 60);
		_screen->setScreenPalette(_screen->_currentPalette);
		setGameFlag(0xB3);
	} else {
		delayWithTicks(120);
	}
	
	freeShapes123();
	setupShapes123(_winterScroll2Table, 4, 0);
	
	for (int i = 123; i <= 126; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}
	
	_animator->resetBrandonAnimSeqSize();
	_currentCharacter->currentAnimFrame = 7;
	_animator->animRefreshNPC(0);
	freeShapes123();
	_screen->showMouse();
}

void KyraEngine::seq_winterScroll2() {
	debugC(9, kDebugLevelMain, "seq_winterScroll2()");	
	_screen->hideMouse();
	checkAmuletAnimFlags();
	assert(_winterScrollTable);
	setupShapes123(_winterScrollTable, 7, 0);
	_animator->setBrandonAnimSeqSize(5, 66);
	
	for (int i = 123; i <= 128; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}
	
	delayWithTicks(120);
	
	for (int i = 127; i >= 123; --i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}
	
	_animator->resetBrandonAnimSeqSize();
	_currentCharacter->currentAnimFrame = 7;
	_animator->animRefreshNPC(0);
	freeShapes123();
	_screen->showMouse();
}

void KyraEngine::seq_makeBrandonInv() {
	debugC(9, kDebugLevelMain, "seq_makeBrandonInv()");
	if (_deathHandler == 8)
		return;

	if (_currentCharacter->sceneId == 210) {
		if (_beadStateVar == 4 || _beadStateVar == 6)
			return;
	}
	
	_screen->hideMouse();
	checkAmuletAnimFlags();
	_brandonStatusBit |= 0x20;
	setTimerCountdown(18, 2700);
	_brandonStatusBit |= 0x40;
	snd_playSoundEffect(0x77);
	_brandonInvFlag = 0;
	while (_brandonInvFlag <= 0x100) {
		_animator->animRefreshNPC(0);
		delayWithTicks(10);
		_brandonInvFlag += 0x10;
	}
	_brandonStatusBit &= 0xFFBF;
	_screen->showMouse();
}

void KyraEngine::seq_makeBrandonNormal() {
	debugC(9, kDebugLevelMain, "seq_makeBrandonNormal()");
	_screen->hideMouse();
	_brandonStatusBit |= 0x40;
	snd_playSoundEffect(0x77);
	_brandonInvFlag = 0x100;
	while (_brandonInvFlag >= 0) {
		_animator->animRefreshNPC(0);
		delayWithTicks(10);
		_brandonInvFlag -= 0x10;
	}
	_brandonInvFlag = 0;
	_brandonStatusBit &= 0xFF9F;
	_screen->showMouse();
}

void KyraEngine::seq_makeBrandonNormal2() {
	debugC(9, kDebugLevelMain, "seq_makeBrandonNormal2()");
	_screen->hideMouse();
	assert(_brandonToWispTable);
	setupShapes123(_brandonToWispTable, 26, 0);
	_animator->setBrandonAnimSeqSize(5, 48);
	_brandonStatusBit &= 0xFFFD;
	snd_playSoundEffect(0x6C);
	for (int i = 138; i >= 123; --i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}
	_animator->setBrandonAnimSeqSize(4, 48);
	_currentCharacter->currentAnimFrame = 7;
	_animator->animRefreshNPC(0);
	if (_currentCharacter->sceneId >= 229 && _currentCharacter->sceneId <= 245) {
		_screen->fadeSpecialPalette(31, 234, 13, 4);
	} else if (_currentCharacter->sceneId >= 118 && _currentCharacter->sceneId <= 186) {
		_screen->fadeSpecialPalette(14, 228, 15, 4);
	}
	freeShapes123();
	_screen->showMouse();
}

void KyraEngine::seq_makeBrandonWisp() {
	debugC(9, kDebugLevelMain, "seq_makeBrandonWisp()");
	if (_deathHandler == 8)
		return;
	
	if (_currentCharacter->sceneId == 210) {
		if (_beadStateVar == 4 || _beadStateVar == 6)
			return;
	}	
	_screen->hideMouse();
	checkAmuletAnimFlags();
	assert(_brandonToWispTable);
	setupShapes123(_brandonToWispTable, 26, 0);
	_animator->setBrandonAnimSeqSize(5, 48);
	snd_playSoundEffect(0x6C);
	for (int i = 123; i <= 138; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}
	_brandonStatusBit |= 2;
	if (_currentCharacter->sceneId >= 109 && _currentCharacter->sceneId <= 198) {
		setTimerCountdown(14, 18000);
	} else {
		setTimerCountdown(14, 7200);
	}
	_animator->_brandonDrawFrame = 113;
	_brandonStatusBit0x02Flag = 1;
	_currentCharacter->currentAnimFrame = 113;
	_animator->animRefreshNPC(0);
	_animator->updateAllObjectShapes();
	if (_currentCharacter->sceneId >= 229 && _currentCharacter->sceneId <= 245) {
		_screen->fadeSpecialPalette(30, 234, 13, 4);
	} else if (_currentCharacter->sceneId >= 118 && _currentCharacter->sceneId <= 186) {
		_screen->fadeSpecialPalette(14, 228, 15, 4);
	}
	freeShapes123();
	_screen->showMouse();
}

void KyraEngine::seq_dispelMagicAnimation() {
	debugC(9, kDebugLevelMain, "seq_dispelMagicAnimation()");
	if (_deathHandler == 8)
		return;
	if (_currentCharacter->sceneId == 210) {
		if (_beadStateVar == 4 || _beadStateVar == 6)
			return;
	}
	_screen->hideMouse();
	if (_currentCharacter->sceneId == 210 && _currentCharacter->sceneId < 160)
		_currentCharacter->facing = 3;
	if (_malcolmFlag == 7 && _beadStateVar == 3) {
		_beadStateVar = 6;
		_unkEndSeqVar5 = 2;
		_malcolmFlag = 10;
	}
	checkAmuletAnimFlags();
	setGameFlag(0xEE);
	assert(_magicAnimationTable);
	setupShapes123(_magicAnimationTable, 5, 0);
	_animator->setBrandonAnimSeqSize(8, 49);
	snd_playSoundEffect(0x15);
	for (int i = 123; i <= 127; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}
	
	delayWithTicks(120);
	
	for (int i = 127; i >= 123; --i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(10);
	}
	_animator->resetBrandonAnimSeqSize();
	_currentCharacter->currentAnimFrame = 7;
	_animator->animRefreshNPC(0);
	freeShapes123();
	_screen->showMouse();
}

void KyraEngine::seq_fillFlaskWithWater(int item, int type) {
	debugC(9, kDebugLevelMain, "seq_fillFlaskWithWater(%d, %d)", item, type);
	int newItem = -1;
	static const uint8 flaskTable1[] = { 0x46, 0x48, 0x4A, 0x4C };
	static const uint8 flaskTable2[] = { 0x47, 0x49, 0x4B, 0x4D };
	
	if (item >= 60 && item <= 77) {
		assert(_flaskFull);
		if (_features & GF_TALKIE) {
			snd_voiceWaitForFinish();
			snd_playVoiceFile(8006);
		}
		characterSays(_flaskFull[0], 0, -2);
	} else if (item == 78) {
		assert(type >= 0 && type < ARRAYSIZE(flaskTable1));
		newItem = flaskTable1[type];
	} else if (item == 79) {
		assert(type >= 0 && type < ARRAYSIZE(flaskTable2));
		newItem = flaskTable2[type];
	}
	
	if (newItem == -1)
		return;
	
	_screen->hideMouse();
	setMouseItem(newItem);
	_screen->showMouse();
	_itemInHand = newItem;
	assert(_fullFlask);
	assert(type < _fullFlask_Size && type >= 0);
	if (_features & GF_TALKIE) {
		snd_voiceWaitForFinish();
		static const uint16 voiceEntries[] = {
			0x1F40, 0x1F41, 0x1F42, 0x1F45
		};
		assert(type < ARRAYSIZE(voiceEntries));
		snd_playVoiceFile(voiceEntries[type]);
	}
	characterSays(_fullFlask[type], 0, -2);
}

void KyraEngine::seq_playDrinkPotionAnim(int item, int unk2, int flags) {
	debugC(9, kDebugLevelMain, "KyraEngine::seq_playDrinkPotionAnim(%d, %d, %d)", item, unk2, flags);
	uint8 red, green, blue;
	
	switch (item) {
		case 60:
		case 61:
			red = 63;
			green = blue = 6;
			break;
		case 62:
		case 63:
			red = green = 0;
			blue = 67;
			break;
		case 64:
		case 65:
			red = 84;
			green = 78;
			blue = 14;
			break;
		case 66:
			red = blue = 0;
			green = 48;
			break;
		case 67:
			red = 100;
			green = 48;
			blue = 23;
			break;
		case 68:
			red = 73;
			green = 0;
			blue = 89;
			break;
		case 69:
			red = green = 73;
			blue = 86;
			break;
		default:
			red = 33;
			green = 66;
			blue = 100;
	}
	red   = (uint8)((double)red   * 0.63);
	green = (uint8)((double)green * 0.63);
	blue  = (uint8)((double)blue  * 0.63);
	
	_screen->setPaletteIndex(0xFE, red, green, blue);
	
	_screen->hideMouse();
	checkAmuletAnimFlags();
	_currentCharacter->facing = 5;
	_animator->animRefreshNPC(0);
	assert(_drinkAnimationTable);
	setupShapes123(_drinkAnimationTable, 9, flags);
	_animator->setBrandonAnimSeqSize(5, 54);
	
	for (int i = 123; i <= 131; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(5);
	}	
	snd_playSoundEffect(0x34);
	for (int i = 0; i < 2; ++i) {
		_currentCharacter->currentAnimFrame = 130;
		_animator->animRefreshNPC(0);
		delayWithTicks(7);
		_currentCharacter->currentAnimFrame = 131;
		_animator->animRefreshNPC(0);
		delayWithTicks(7);
	}
	
	if (unk2) {
		// XXX
	}
	
	for (int i = 131; i >= 123; --i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(5);
	}
	
	_animator->resetBrandonAnimSeqSize();	
	_currentCharacter->currentAnimFrame = 7;
	_animator->animRefreshNPC(0);
	freeShapes123();
	_screen->setPaletteIndex(0xFE, 30, 30, 30);
	_screen->showMouse();
}

int KyraEngine::seq_playEnd() {
	debugC(9, kDebugLevelMain, "KyraEngine::seq_playEnd()");
	if (_endSequenceSkipFlag) {
		return 0;
	}
	if (_deathHandler == 8) {
		return 0;
	}
	_screen->_curPage = 2;
	if (_endSequenceNeedLoading) {
		snd_playWanderScoreViaMap(50, 1);
		setupPanPages();
		_finalA = new WSAMovieV1(this);
		assert(_finalA);
		_finalA->open("finala.wsa", 1, 0);
		_finalB = new WSAMovieV1(this);
		assert(_finalB);
		_finalB->open("finalb.wsa", 1, 0);
		_finalC = new WSAMovieV1(this);
		assert(_finalC);
		_endSequenceNeedLoading = 0;
		_finalC->open("finalc.wsa", 1, 0);
		_screen->_curPage = 0;
		_beadStateVar = 0;
		_malcolmFlag = 0;
		// wired stuff with _unkEndSeqVar2 which needs timer handling
		_screen->copyRegion(312, 0, 312, 0, 8, 136, 0, 2);
	}
	if (handleMalcolmFlag()) {
		_beadStateVar = 0;
		_malcolmFlag = 12;
		handleMalcolmFlag();
		handleBeadState();
		closeFinalWsa();
		if (_deathHandler == 8) {
			_screen->_curPage = 0;
			checkAmuletAnimFlags();
			seq_brandonToStone();
			delay(60 * _tickLength);
			return 1;
		} else {
			_endSequenceSkipFlag = 1;
			if (_text->printed()) {
				_text->restoreTalkTextMessageBkgd(2, 0);
			}
			_screen->_curPage = 0;
			_screen->hideMouse();
			_screen->fadeSpecialPalette(32, 228, 20, 60);
			delay(60 * _tickLength);
			loadBitmap("GEMHEAL.CPS", 3, 3, _screen->_currentPalette);
			_screen->setScreenPalette(_screen->_currentPalette);
			_screen->shuffleScreen(8, 8, 304, 128, 2, 0, 1, 0);
			uint32 nextTime = _system->getMillis() + 120 * _tickLength;
			_finalA = new WSAMovieV1(this);
			assert(_finalA);
			_finalA->open("finald.wsa", 1, 0);
			_finalA->_x = _finalA->_y = 8;
			_finalA->_drawPage = 0;
			while (_system->getMillis() < nextTime) {}
			snd_playSoundEffect(0x40);
			for (int i = 0; i < 22; ++i) {
				while (_system->getMillis() < nextTime) {}
				if (i == 4) {
					snd_playSoundEffect(0x3E);
				} else if (i == 20) {
					snd_playSoundEffect(0x0E);
				}
				nextTime = _system->getMillis() + 8 * _tickLength;
				_finalA->displayFrame(i);
				_screen->updateScreen();
			}
			delete _finalA;
			_finalA = 0;
			seq_playEnding();
			return 1;
		}
	} else {
		handleBeadState();
		_screen->bitBlitRects();
		_screen->updateScreen();
		_screen->_curPage = 0;
	}
	return 0;
}

void KyraEngine::seq_brandonToStone() {
	debugC(9, kDebugLevelMain, "KyraEngine::seq_brandonToStone()");
	_screen->hideMouse();
	assert(_brandonStoneTable);
	setupShapes123(_brandonStoneTable, 14, 0);
	_animator->setBrandonAnimSeqSize(5, 51);
	for (int i = 123; i <= 136; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}
	_animator->resetBrandonAnimSeqSize();
	freeShapes123();
	_screen->showMouse();
}

void KyraEngine::seq_playEnding() {
	debugC(9, kDebugLevelMain, "KyraEngine::seq_playEnding()");
	_screen->hideMouse();
	_screen->_curPage = 0;
	_screen->fadeToBlack();
	loadBitmap("REUNION.CPS", 3, 3, _screen->_currentPalette);
	_screen->copyRegion(8, 8, 8, 8, 304, 128, 2, 0);
	_screen->_curPage = 0;
	// XXX
	assert(_homeString);
	drawSentenceCommand(_homeString[0], 179);
	memset(_screen->getPalette(0), 0, sizeof(uint8)*768);
	_screen->setScreenPalette(_screen->getPalette(0));
	_seq->playSequence(_seq_Reunion, false);
	_screen->fadeToBlack();
	_screen->showMouse();
	seq_playCredits();
}

void KyraEngine::seq_playCredits() {
	debugC(9, kDebugLevelMain, "KyraEngine::seq_playCredits()");
	static const uint8 colorMap[] = { 0, 0, 0xC, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	static const char stringTerms[] = { 0x5, 0xd, 0x0};
	static const int numStrings = 250;
	
	struct {
		int16 x, y;
		uint8 code;
		uint8 unk1;
		Screen::FontId font;
		uint8 *str;
	} strings[numStrings];
	
	memset(strings, 0, sizeof(strings));
	
	_screen->hideMouse();
	uint32 sz = 0;
	if (_features & GF_FLOPPY) {
		_screen->loadFont(Screen::FID_CRED6_FNT, _res->fileData("CREDIT6.FNT", &sz));
		_screen->loadFont(Screen::FID_CRED8_FNT, _res->fileData("CREDIT8.FNT", &sz));
	} else
		_screen->setFont(Screen::FID_8_FNT);
	
	loadBitmap("CHALET.CPS", 4, 4, _screen->_currentPalette);
	_screen->setScreenPalette(_screen->_currentPalette);
	
	_screen->setCurPage(0);
	_screen->clearCurPage();
	_screen->setTextColorMap(colorMap);
	_screen->_charWidth = -1;
	snd_playWanderScoreViaMap(53, 1);

	uint8 *buffer = 0;
	uint32 size;
	
	buffer = _res->fileData("CREDITS.TXT", &size);
	assert(buffer);

	uint8 *nextString = buffer;
	uint8 *currentString = buffer;
	int currentY = 200;
	
	for (int i = 0; i < numStrings; i++) {
		if (*nextString == 0)
			break;
			
		currentString = nextString;
		nextString = (uint8 *)strpbrk((const char *)currentString, stringTerms);
		if (!nextString)
			nextString = (uint8 *)strchr((const char *)currentString, 0);
		
		strings[i].code = nextString[0];
		*nextString = 0;
		if (strings[i].code != 0)
			nextString++;
		
		if (*currentString == 3 || *currentString == 4) {
			strings[i].unk1 = *currentString;
			currentString++;
		}
		
		if (*currentString == 1) {
		   	currentString++;
			if (_features & GF_FLOPPY)
				_screen->setFont(Screen::FID_CRED6_FNT);
		} else {
			if (*currentString == 2)
				currentString++;
			if (_features & GF_FLOPPY)
				_screen->setFont(Screen::FID_CRED8_FNT);
		}
		strings[i].font = _screen->_currentFont;

		if (strings[i].unk1 == 3) 
			strings[i].x = 157 - _screen->getTextWidth((const char *)currentString);
		else if (strings[i].unk1 == 4)
			strings[i].x = 161;
		else
			strings[i].x = (320  - _screen->getTextWidth((const char *)currentString)) / 2 + 1;
		
		strings[i].y = currentY;
		if (strings[i].code != 5)
			currentY += 10;
		
		strings[i].str = currentString;
	}

	_screen->setCurPage(2);

	OSystem::Event event;	
	bool finished = false;
	int bottom = 201;
	while (!finished) {
		uint32 startLoop = _system->getMillis();
		if (bottom > 175) {
			_screen->copyRegion(8, 32, 8, 32, 312, 128, 4, 2);
			bottom = 0;
			
			for (int i = 0; i < numStrings; i++) {
				if (strings[i].y < 200 && strings[i].y > 0) {
					if (strings[i].font != _screen->_currentFont)
						_screen->setFont(strings[i].font);
					_screen->printText((const char *)strings[i].str, strings[i].x, strings[i].y, 15, 0);
				}
				strings[i].y--;
				if (strings[i].y > bottom)
					bottom = strings[i].y;
			}
			_screen->copyRegion(8, 32, 8, 32, 312, 128, 2, 0);
			_screen->updateScreen();
		}

		while (_system->pollEvent(event)) {
			switch (event.type) {
			case OSystem::EVENT_KEYDOWN:
				finished = true;
				break;
			case OSystem::EVENT_QUIT:
				quitGame();
				break;
			default:
				break;
			}
		}

		uint32 now = _system->getMillis();
		uint32 nextLoop = startLoop + _tickLength * 5;

		if (nextLoop > now)
			_system->delayMillis(nextLoop - now);
	}
	
	delete[] buffer;
	
	_screen->fadeToBlack();
	_screen->clearCurPage();
	_screen->showMouse();
}

bool KyraEngine::seq_skipSequence() const {
	debugC(9, kDebugLevelMain, "KyraEngine::seq_skipSequence()");
	return _quitFlag || _abortIntroFlag;
}

int KyraEngine::handleMalcolmFlag() {
	debugC(9, kDebugLevelMain, "KyraEngine::handleMalcolmFlag()");
	static uint16 frame = 0;
	static uint32 timer1 = 0;
	static uint32 timer2 = 0;
	
	switch (_malcolmFlag) {
		case 1:
			frame = 0;
			_malcolmFlag = 2;
			timer2 = 0;
		case 2:
			if (_system->getMillis() >= timer2) {
				_finalA->_x = 8;
				_finalA->_y = 46;
				_finalA->_drawPage = 0;
				_finalA->displayFrame(frame);
				_screen->updateScreen();
				timer2 = _system->getMillis() + 8 * _tickLength;
				++frame;
				if (frame > 13) {
					_malcolmFlag = 3;
					timer1 = _system->getMillis() + 180 * _tickLength;
				}
			}
			break;
		
		case 3:
			if (_system->getMillis() < timer1) {
				if (_system->getMillis() >= timer2) {
					frame = _rnd.getRandomNumberRng(14, 17);
					_finalA->_x = 8;
					_finalA->_y = 46;
					_finalA->_drawPage = 0;
					_finalA->displayFrame(frame);
					_screen->updateScreen();
					timer2 = _system->getMillis() + 8 * _tickLength;
				}
			} else {
				_malcolmFlag = 4;
				frame = 18;
			}
			break;
		
		case 4:
			if (_system->getMillis() >= timer2) {
				_finalA->_x = 8;
				_finalA->_y = 46;
				_finalA->_drawPage = 0;
				_finalA->displayFrame(frame);
				_screen->updateScreen();
				timer2 = _system->getMillis() + 8 * _tickLength;
				++frame;
				if (frame > 25) {
					frame = 26;
					_malcolmFlag = 5;
					_beadStateVar = 1;
				}
			}
			break;
		
		case 5:
			if (_system->getMillis() >= timer2) {
				_finalA->_x = 8;
				_finalA->_y = 46;
				_finalA->_drawPage = 0;
				_finalA->displayFrame(frame);
				_screen->updateScreen();
				timer2 = _system->getMillis() + 8 * _tickLength;
				++frame;
				if (frame > 31) {
					frame = 32;
					_malcolmFlag = 6;
				}
			}
			break;
			
		case 6:
			if (_unkEndSeqVar4) {
				if (frame <= 33 && _system->getMillis() >= timer2) {
					_finalA->_x = 8;
					_finalA->_y = 46;
					_finalA->_drawPage = 0;
					_finalA->displayFrame(frame);
					_screen->updateScreen();
					timer2 = _system->getMillis() + 8 * _tickLength;
					++frame;
					if (frame > 33) {
						_malcolmFlag = 7;
						frame = 32;
						_unkEndSeqVar5 = 0;
					}
				}
			}
			break;
		
		case 7:
			if (_unkEndSeqVar5 == 1) {
				_malcolmFlag = 8;
				frame = 34;
			} else if (_unkEndSeqVar5 == 2) {
				_malcolmFlag = 3;
				timer1 = _system->getMillis() + 180 * _tickLength;
			}
			break;
		
		case 8:
			if (_system->getMillis() >= timer2) {
				_finalA->_x = 8;
				_finalA->_y = 46;
				_finalA->_drawPage = 0;
				_finalA->displayFrame(frame);
				_screen->updateScreen();
				timer2 = _system->getMillis() + 8 * _tickLength;
				++frame;
				if (frame > 37) {
					_malcolmFlag = 0;
					_deathHandler = 8;
					return 1;
				}
			}
			break;
		
		case 9:
			snd_playSoundEffect(12);
			snd_playSoundEffect(12);
			_finalC->_x = 16;
			_finalC->_y = 50;
			_finalC->_drawPage = 0;
			for (int i = 0; i < 18; ++i) {
				timer2 = _system->getMillis() + 4 * _tickLength;
				_finalC->displayFrame(i);
				_screen->updateScreen();
				while (_system->getMillis() < timer2) {}
			}
			snd_playWanderScoreViaMap(51, 1);
			delay(60*_tickLength);
			_malcolmFlag = 0;
			return 1;
			break;
		
		case 10:
			if (!_beadStateVar) {
				handleBeadState();
				_screen->bitBlitRects();
				assert(_veryClever);
				_text->printTalkTextMessage(_veryClever[0], 60, 31, 5, 0, 2);
				timer2 = _system->getMillis() + 180 * _tickLength;
				_malcolmFlag = 11;
			}
			break;
		
		case 11:
			if (_system->getMillis() >= timer2) {
				_text->restoreTalkTextMessageBkgd(2, 0);
				_malcolmFlag = 3;
				timer1 = _system->getMillis() + 180 * _tickLength;
			}
			break;
		
		default:
			break;
	}
	
	return 0;
}

int KyraEngine::handleBeadState() {
	debugC(9, kDebugLevelMain, "KyraEngine::handleBeadState()");
	static uint32 timer1 = 0;
	static uint32 timer2 = 0;
	static BeadState beadState1 = { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	static BeadState beadState2 = {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	
	static const int table1[] = {
		-1, -2, -4, -5, -6, -7, -6, -5,
		-4, -2, -1,  0,  1,  2,  4,  5,
		 6,  7,  6,  5,  4,  2,  1,  0, 0
	};
	static const int table2[] = {
		0, 0, 1, 1, 2, 2, 3, 3,
		4, 4, 5, 5, 5, 5, 4, 4,
		3, 3, 2, 2, 1, 1, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	
	switch (_beadStateVar) {
		case 0:
			if (beadState1.x != -1 && _endSequenceBackUpRect) {
				_screen->copyFromCurPageBlock(beadState1.x >> 3, beadState1.y, beadState1.width, beadState1.height, _endSequenceBackUpRect);
				_screen->addBitBlitRect(beadState1.x, beadState1.y, beadState1.width2, beadState1.height);
			} else {
				beadState1.x = -1;
				beadState1.tableIndex = 0;
				timer1 = 0;
				timer2 = 0;
				_lastDisplayedPanPage = 0;
				return 1;
			}
		
		case 1:
			if (beadState1.x != -1) {
				if (_endSequenceBackUpRect) {
					_screen->copyFromCurPageBlock(beadState1.x >> 3, beadState1.y, beadState1.width, beadState1.height, _endSequenceBackUpRect);
					_screen->addBitBlitRect(beadState1.x, beadState1.y, beadState1.width2, beadState1.height);
				}
				beadState1.x = -1;
				beadState1.tableIndex = 0;
			}
			_beadStateVar = 2;
			break;
		
		case 2:
			if (_system->getMillis() >= timer1) {
				int x = 0, y = 0;
				timer1 = _system->getMillis() + 4 * _tickLength;
				if (beadState1.x == -1) {
					assert(_panPagesTable);
					beadState1.width2 = _animator->fetchAnimWidth(_panPagesTable[19], 256);
					beadState1.width = ((beadState1.width2 + 7) >> 3) + 1;
					beadState1.height = _animator->fetchAnimHeight(_panPagesTable[19], 256);
					if (!_endSequenceBackUpRect) {
						_endSequenceBackUpRect = new uint8[(beadState1.width * beadState1.height) << 3];
						assert(_endSequenceBackUpRect);
						memset(_endSequenceBackUpRect, 0, ((beadState1.width * beadState1.height) << 3) * sizeof(uint8));
					}
					x = beadState1.x = 60;
					y = beadState1.y = 40;
					initBeadState(x, y, x, 25, 8, &beadState2);
				} else {
					if (processBead(beadState1.x, beadState1.y, x, y, &beadState2)) {
						_beadStateVar = 3;
						timer2 = _system->getMillis() + 240 * _tickLength;
						_unkEndSeqVar4 = 0;
						beadState1.dstX = beadState1.x;
						beadState1.dstY = beadState1.y;
						return 0;
					} else {
						_screen->copyFromCurPageBlock(beadState1.x >> 3, beadState1.y, beadState1.width, beadState1.height, _endSequenceBackUpRect);
						_screen->addBitBlitRect(beadState1.x, beadState1.y, beadState1.width2, beadState1.height);
						beadState1.x = x;
						beadState1.y = y;
					}
				}
				_screen->copyCurPageBlock(x >> 3, y, beadState1.width, beadState1.height, _endSequenceBackUpRect);
				_screen->drawShape(2, _panPagesTable[_lastDisplayedPanPage++], x, y, 0, 0);
				if (_lastDisplayedPanPage > 17)
					_lastDisplayedPanPage = 0;
				_screen->addBitBlitRect(x, y, beadState1.width2, beadState1.height);
			}
			break;
		
		case 3:
			if (_system->getMillis() >= timer1) {
				timer1 = _system->getMillis() + 4 * _tickLength;
				_screen->copyFromCurPageBlock(beadState1.x >> 3, beadState1.y, beadState1.width, beadState1.height, _endSequenceBackUpRect);
				_screen->addBitBlitRect(beadState1.x, beadState1.y, beadState1.width2, beadState1.height);
				beadState1.x = beadState1.dstX + table1[beadState1.tableIndex];
				beadState1.y = beadState1.dstY + table2[beadState1.tableIndex];
				_screen->copyCurPageBlock(beadState1.x >> 3, beadState1.y, beadState1.width, beadState1.height, _endSequenceBackUpRect);
				_screen->drawShape(2, _panPagesTable[_lastDisplayedPanPage++], beadState1.x, beadState1.y, 0, 0);
				if (_lastDisplayedPanPage >= 17) {
					_lastDisplayedPanPage = 0;
				}
				_screen->addBitBlitRect(beadState1.x, beadState1.y, beadState1.width2, beadState1.height);
				++beadState1.tableIndex;
				if (beadState1.tableIndex > 24) {
					beadState1.tableIndex = 0;
					_unkEndSeqVar4 = 1;
				}
				if (_system->getMillis() > timer2 && _malcolmFlag == 7 && !_unkAmuletVar && !_text->printed()) {
					snd_playSoundEffect(0x0B);
					if (_currentCharacter->x1 > 233 && _currentCharacter->x1 < 305 && _currentCharacter->y1 > 85 && _currentCharacter->y1 < 105 &&
						(_brandonStatusBit & 0x20)) {
						beadState1.unk8 = 290;
						beadState1.unk9 = 40;
						_beadStateVar = 5;
					} else {
						_beadStateVar = 4;
						beadState1.unk8 = _currentCharacter->x1 - 4;
						beadState1.unk9 = _currentCharacter->y1 - 30;
					}
					
					if (_text->printed()) {
						_text->restoreTalkTextMessageBkgd(2, 0);
					}
					initBeadState(beadState1.x, beadState1.y, beadState1.unk8, beadState1.unk9, 6, &beadState2);
					_lastDisplayedPanPage = 18;
				}
			}
			break;
			
		case 4:
			if (_system->getMillis() >= timer1) {
				int x = 0, y = 0;
				timer1 = _system->getMillis();
				if (processBead(beadState1.x, beadState1.y, x, y, &beadState2)) {
					if (_brandonStatusBit & 20) {
						_unkEndSeqVar5 = 2;
						_beadStateVar = 6;
					} else {
						snd_playWanderScoreViaMap(52, 1);
						snd_playSoundEffect(0x0C);
						_unkEndSeqVar5 = 1;
						_beadStateVar = 0;
					}
				} else {
					_screen->copyFromCurPageBlock(beadState1.x >> 3, beadState1.y, beadState1.width, beadState1.height, _endSequenceBackUpRect);
					_screen->addBitBlitRect(beadState1.x, beadState1.y, beadState1.width2, beadState1.height);
					beadState1.x = x;
					beadState1.y = y;
					_screen->copyCurPageBlock(beadState1.x >> 3, beadState1.y, beadState1.width, beadState1.height, _endSequenceBackUpRect);
					_screen->drawShape(2, _panPagesTable[_lastDisplayedPanPage++], x, y, 0, 0);
					if (_lastDisplayedPanPage > 17) {
						_lastDisplayedPanPage = 0;
					}
					_screen->addBitBlitRect(beadState1.x, beadState1.y, beadState1.width2, beadState1.height);
				}
			}
			break;
		
		case 5:
			if (_system->getMillis() >= timer1) {
				timer1 = _system->getMillis();
				int x = 0, y = 0;
				if (processBead(beadState1.x, beadState1.y, x, y, &beadState2)) {
					if (beadState1.dstX == 290) {
						_screen->copyFromCurPageBlock(beadState1.x >> 3, beadState1.y, beadState1.width, beadState1.height, _endSequenceBackUpRect);
						uint32 nextRun = 0;
						_finalB->_x = 224;
						_finalB->_y = 8;
						_finalB->_drawPage = 0;
						for (int i = 0; i < 8; ++i) {
							nextRun = _system->getMillis() + _tickLength;
							_finalB->displayFrame(i);
							_screen->updateScreen();
							while (_system->getMillis() < nextRun) {}
						}
						snd_playSoundEffect(0x0D);
						for (int i = 7; i >= 0; --i) {
							nextRun = _system->getMillis() + _tickLength;
							_finalB->displayFrame(i);
							_screen->updateScreen();
							while (_system->getMillis() < nextRun) {}
						}
						initBeadState(beadState1.x, beadState1.y, 63, 60, 6, &beadState2);
					} else {
						_screen->copyFromCurPageBlock(beadState1.x >> 3, beadState1.y, beadState1.width, beadState1.height, _endSequenceBackUpRect);
						_screen->addBitBlitRect(beadState1.x, beadState1.y, beadState1.width2, beadState1.height);
						beadState1.x = -1;
						beadState1.tableIndex = 0;
						_beadStateVar = 0;
						_malcolmFlag = 9;
					}
				} else {
					_screen->copyFromCurPageBlock(beadState1.x >> 3, beadState1.y, beadState1.width, beadState1.height, _endSequenceBackUpRect);
					_screen->addBitBlitRect(beadState1.x, beadState1.y, beadState1.width2, beadState1.height);
					beadState1.x = x;
					beadState1.y = y;
					_screen->copyCurPageBlock(beadState1.x >> 3, beadState1.y, beadState1.width, beadState1.height, _endSequenceBackUpRect);
					_screen->drawShape(2, _panPagesTable[_lastDisplayedPanPage++], x, y, 0, 0);
					if (_lastDisplayedPanPage > 17) {
						_lastDisplayedPanPage = 0;
					}
					_screen->addBitBlitRect(beadState1.x, beadState1.y, beadState1.width2, beadState1.height);
				}
			}
			break;
		
		case 6:
			_screen->drawShape(2, _panPagesTable[19], beadState1.x, beadState1.y, 0, 0);
			_screen->addBitBlitRect(beadState1.x, beadState1.y, beadState1.width2, beadState1.height);
			_beadStateVar = 0;
			break;
		
		default:
			break;
	}
	return 0;
}

void KyraEngine::initBeadState(int x, int y, int x2, int y2, int unk, BeadState *ptr) {
	debugC(9, kDebugLevelMain, "KyraEngine::initBeadState(%d, %d, %d, %d, %d, %p)", x, y, x2, y2, unk, (const void *)ptr);
	ptr->unk9 = unk;
	int xDiff = x2 - x;
	int yDiff = y2 - y;
	int unk1 = 0, unk2 = 0;
	if (xDiff > 0) {
		unk1 = 1;
	} else if (xDiff == 0) {
		unk1 = 0;
	} else {
		unk1 = -1;
	}
	
	if (yDiff > 0) {
		unk2 = 1;
	} else if (yDiff == 0) {
		unk2 = 0;
	} else {
		unk2 = -1;
	}
	
	xDiff = abs(xDiff);
	yDiff = abs(yDiff);
	
	ptr->y = 0;
	ptr->x = 0;
	ptr->width = xDiff;
	ptr->height = yDiff;
	ptr->dstX = x2;
	ptr->dstY = y2;
	ptr->width2 = unk1;
	ptr->unk8 = unk2;
}

int KyraEngine::processBead(int x, int y, int &x2, int &y2, BeadState *ptr) {
	debugC(9, kDebugLevelMain, "KyraEngine::processBead(%d, %d, %p, %p, %p)", x, y, (const void *)&x2, (const void *)&y2, (const void *)ptr);
	if (x == ptr->dstX && y == ptr->dstY) {
		return 1;
	}
	
	int xPos = x, yPos = y;
	if (ptr->width >= ptr->height) {
		for (int i = 0; i < ptr->unk9; ++i) {
			ptr->y += ptr->height;
			if (ptr->y >= ptr->width) {
				ptr->y -= ptr->width;
				yPos += ptr->unk8;
			}
			xPos += ptr->width2;
		}
	} else {
		for (int i = 0; i < ptr->unk9; ++i) {
			ptr->x += ptr->width;
			if (ptr->x >= ptr->height) {
				ptr->x -= ptr->height;
				xPos += ptr->width2;
			}
			yPos += ptr->unk8;
		}
	}
	
	int temp = abs(x - ptr->dstX);
	if (ptr->unk9 > temp) {
		xPos = ptr->dstX;
	}
	temp = abs(y - ptr->dstY);
	if (ptr->unk9 > temp) {
		yPos = ptr->dstY;
	}
	x2 = xPos;
	y2 = yPos;
	return 0;
}

void KyraEngine::setupPanPages() {
	debugC(9, kDebugLevelMain, "KyraEngine::setupPanPages()");
	loadBitmap("bead.cps", 3, 3, 0);
	for (int i = 0; i <= 19; ++i) {
		_panPagesTable[i] = _seq->setPanPages(3, i);
	}
}

void KyraEngine::freePanPages() {
	debugC(9, kDebugLevelMain, "KyraEngine::freePanPages()");
	delete _endSequenceBackUpRect;
	_endSequenceBackUpRect = 0;
	for (int i = 0; i <= 19; ++i) {
		free(_panPagesTable[i]);
		_panPagesTable[i] = NULL;
	}
}

void KyraEngine::closeFinalWsa() {
	debugC(9, kDebugLevelMain, "KyraEngine::closeFinalWsa()");
	delete _finalA;
	_finalA = 0;
	delete _finalB;
	_finalB = 0;
	delete _finalC;
	_finalC = 0;
	freePanPages();
	_endSequenceNeedLoading = 1;
}

void KyraEngine::updateKyragemFading() {
	static const uint8 kyraGemPalette[0x28] = {
		0x3F, 0x3B, 0x38, 0x34, 0x32, 0x2F, 0x2C, 0x29, 0x25, 0x22,
		0x1F, 0x1C, 0x19, 0x16, 0x12, 0x0F, 0x0C, 0x0A, 0x06, 0x03,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	
	if (_system->getMillis() < _kyragemFadingState.timerCount)
		return;
	
	_kyragemFadingState.timerCount = _system->getMillis() + 4 * _tickLength;
	int palPos = 684;
	for (int i = 0; i < 20; ++i) {
		_screen->_currentPalette[palPos++] = kyraGemPalette[i + _kyragemFadingState.rOffset];
		_screen->_currentPalette[palPos++] = kyraGemPalette[i + _kyragemFadingState.gOffset];
		_screen->_currentPalette[palPos++] = kyraGemPalette[i + _kyragemFadingState.bOffset];
	}
	_screen->setScreenPalette(_screen->_currentPalette);
	_animator->_updateScreen = true;
	switch (_kyragemFadingState.nextOperation) {
		case 0:
			--_kyragemFadingState.bOffset;
			if (_kyragemFadingState.bOffset >= 1)
				return;
			_kyragemFadingState.nextOperation = 1;
			break;

		case 1:
			++_kyragemFadingState.rOffset;
			if (_kyragemFadingState.rOffset < 19)
				return;
			_kyragemFadingState.nextOperation = 2;
			break;

		case 2:
			--_kyragemFadingState.gOffset;
			if (_kyragemFadingState.gOffset >= 1)
				return;
			_kyragemFadingState.nextOperation = 3;
			break;
		
		case 3:
			++_kyragemFadingState.bOffset;
			if (_kyragemFadingState.bOffset < 19)
				return;
			_kyragemFadingState.nextOperation = 4;
			break;
		
		case 4:
			--_kyragemFadingState.rOffset;
			if (_kyragemFadingState.rOffset >= 1)
				return;
			_kyragemFadingState.nextOperation = 5;
			break;
		
		case 5:
			++_kyragemFadingState.gOffset;
			if (_kyragemFadingState.gOffset < 19)
				return;
			_kyragemFadingState.nextOperation = 0;
			break;
			
		default:
			break;
	}
	
	_kyragemFadingState.timerCount = _system->getMillis() + 120 * _tickLength;
}

void KyraEngine::drawJewelPress(int jewel, int drawSpecial) {
	debugC(9, kDebugLevelMain, "KyraEngine::drawJewelPress(%d, %d)", jewel, drawSpecial);
	_screen->hideMouse();
	int shape = 0;
	if (drawSpecial) {
		shape = 0x14E;
	} else {
		shape = jewel + 0x149;
	}
	snd_playSoundEffect(0x45);
	_screen->drawShape(0, _shapes[4+shape], _amuletX2[jewel], _amuletY2[jewel], 0, 0);
	_screen->updateScreen();
	delayWithTicks(2);
	if (drawSpecial) {
		shape = 0x148;
	} else {
		shape = jewel + 0x143;
	}
	_screen->drawShape(0, _shapes[4+shape], _amuletX2[jewel], _amuletY2[jewel], 0, 0);
	_screen->updateScreen();
	_screen->showMouse();
}

void KyraEngine::drawJewelsFadeOutStart() {
	debugC(9, kDebugLevelMain, "KyraEngine::drawJewelsFadeOutStart()");
	static const uint16 jewelTable1[] = { 0x164, 0x15F, 0x15A, 0x155, 0x150, 0xFFFF };
	static const uint16 jewelTable2[] = { 0x163, 0x15E, 0x159, 0x154, 0x14F, 0xFFFF };
	static const uint16 jewelTable3[] = { 0x166, 0x160, 0x15C, 0x157, 0x152, 0xFFFF };
	static const uint16 jewelTable4[] = { 0x165, 0x161, 0x15B, 0x156, 0x151, 0xFFFF };
	for (int i = 0; jewelTable1[i] != 0xFFFF; ++i) {
		if (queryGameFlag(0x57)) {
			_screen->drawShape(0, _shapes[4+jewelTable1[i]], _amuletX2[2], _amuletY2[2], 0, 0);
		}
		if (queryGameFlag(0x59)) {
			_screen->drawShape(0, _shapes[4+jewelTable3[i]], _amuletX2[4], _amuletY2[4], 0, 0);
		}
		if (queryGameFlag(0x56)) {
			_screen->drawShape(0, _shapes[4+jewelTable2[i]], _amuletX2[1], _amuletY2[1], 0, 0);
		}
		if (queryGameFlag(0x58)) {
			_screen->drawShape(0, _shapes[4+jewelTable4[i]], _amuletX2[3], _amuletY2[3], 0, 0);
		}
		_screen->updateScreen();
		delayWithTicks(3);
	}
}

void KyraEngine::drawJewelsFadeOutEnd(int jewel) {
	debugC(9, kDebugLevelMain, "KyraEngine::drawJewelsFadeOutEnd(%d)", jewel);
	static const uint16 jewelTable[] = { 0x153, 0x158, 0x15D, 0x162, 0x148, 0xFFFF };
	int newDelay = 0;
	switch (jewel-1) {
		case 2:
			if (_currentCharacter->sceneId >= 109 && _currentCharacter->sceneId <= 198) {
				newDelay = 18900;
			} else {
				newDelay = 8100;
			}
			break;
			
		default:
			newDelay = 3600;
			break;
	}
	setGameFlag(0xF1);
	setTimerCountdown(19, newDelay);
	_screen->hideMouse();
	for (int i = 0; jewelTable[i] != 0xFFFF; ++i) {
		uint16 shape = jewelTable[i];
		if (queryGameFlag(0x57)) {
			_screen->drawShape(0, _shapes[4+shape], _amuletX2[2], _amuletY2[2], 0, 0);
		}
		if (queryGameFlag(0x59)) {
			_screen->drawShape(0, _shapes[4+shape], _amuletX2[4], _amuletY2[4], 0, 0);
		}
		if (queryGameFlag(0x56)) {
			_screen->drawShape(0, _shapes[4+shape], _amuletX2[1], _amuletY2[1], 0, 0);
		}
		if (queryGameFlag(0x58)) {
			_screen->drawShape(0, _shapes[4+shape], _amuletX2[3], _amuletY2[3], 0, 0);
		}
		_screen->updateScreen();
		delayWithTicks(3);
	}
	_screen->showMouse();
}

} // end of namespace Kyra
