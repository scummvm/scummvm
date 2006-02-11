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
	debug(9, "KyraEngine::seq_demo()");

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
	_sound->stopMusic();
}

void KyraEngine::seq_intro() {
	debug(9, "KyraEngine::seq_intro()");

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
 	snd_setSoundEffectFile(MUSIC_INTRO);
	_text->setTalkCoords(144);
	for (int i = 0; i < ARRAYSIZE(introProcTable) && !seq_skipSequence(); ++i) {
		(this->*introProcTable[i])();
	}
	_text->setTalkCoords(136);
	delay(30 * _tickLength);
	_seq->setCopyViewOffs(false);
	_sound->stopMusic();
	if (_features & GF_TALKIE) {
		_res->unloadPakFile("INTRO.VRM");
	}
	res_unloadResources(RES_INTRO | RES_OUTRO);
}

void KyraEngine::seq_introLogos() {
	debug(9, "KyraEngine::seq_introLogos()");
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
	debug(9, "KyraEngine::seq_introStory()");
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
	debug("skipFlag %i, %i", _skipFlag, _tickLength);
	delay(360 * _tickLength);
}

void KyraEngine::seq_introMalcolmTree() {
	debug(9, "KyraEngine::seq_introMalcolmTree()");
	_screen->_curPage = 0;
	_screen->clearPage(3);
	_seq->playSequence(_seq_MalcolmTree, true);
}

void KyraEngine::seq_introKallakWriting() {
	debug(9, "KyraEngine::seq_introKallakWriting()");
	_seq->makeHandShapes();
	_screen->setAnimBlockPtr(5060);
	_screen->_charWidth = -2;
	_screen->clearPage(3);
	_seq->playSequence(_seq_KallakWriting, true);
}

void KyraEngine::seq_introKallakMalcolm() {
	debug(9, "KyraEngine::seq_introKallakMalcolm()");
	_screen->clearPage(3);
	_seq->playSequence(_seq_KallakMalcolm, true);
}

void KyraEngine::seq_createAmuletJewel(int jewel, int page, int noSound, int drawOnly) {
	debug(9, "seq_createAmuletJewel(%d, %d, %d, %d)", jewel, page, noSound, drawOnly);
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
	debug(9, "seq_brandonHealing()");
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
	setBrandonAnimSeqSize(3, 48);
	snd_playSoundEffect(0x53);
	for (int i = 123; i <= 144; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	for (int i = 125; i >= 123; --i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	resetBrandonAnimSeqSize();
	_currentCharacter->currentAnimFrame = 7;
	animRefreshNPC(0);
	freeShapes123();
	_screen->showMouse();
}

void KyraEngine::seq_brandonHealing2() {
	debug(9, "seq_brandonHealing2()");
	_screen->hideMouse();
	checkAmuletAnimFlags();
	assert(_healingShape2Table);
	setupShapes123(_healingShape2Table, 30, 0);
	resetBrandonPoisonFlags();
	setBrandonAnimSeqSize(3, 48);
	snd_playSoundEffect(0x50);
	for (int i = 123; i <= 152; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	resetBrandonAnimSeqSize();
	_currentCharacter->currentAnimFrame = 7;
	animRefreshNPC(0);
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
	debug(9, "seq_poisonDeathNow(%d)", now);
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
	debug(9, "seq_poisonDeathNowAnim()");
	_screen->hideMouse();
	checkAmuletAnimFlags();
	assert(_posionDeathShapeTable);
	setupShapes123(_posionDeathShapeTable, 20, 0);
	setBrandonAnimSeqSize(8, 48);
	
	_currentCharacter->currentAnimFrame = 124;
	animRefreshNPC(0);
	delayWithTicks(30);
	
	_currentCharacter->currentAnimFrame = 123;
	animRefreshNPC(0);
	delayWithTicks(30);
	
	for (int i = 125; i <= 139; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	
	delayWithTicks(60);
	
	for (int i = 140; i <= 142; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	
	delayWithTicks(60);
	
	resetBrandonAnimSeqSize();
	freeShapes123();
	_animator->restoreAllObjectBackgrounds();
	_currentCharacter->x1 = _currentCharacter->x2 = -1;
	_currentCharacter->y1 = _currentCharacter->y2 = -1;
	_animator->preserveAllBackgrounds();
	_screen->showMouse();
}

void KyraEngine::seq_playFluteAnimation() {
	debug(9, "seq_playFluteAnimation()");
	_screen->hideMouse();
	checkAmuletAnimFlags();
	setupShapes123(_fluteAnimShapeTable, 36, 0);
	setBrandonAnimSeqSize(3, 75);
	for (int i = 123; i <= 130; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
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
		animRefreshNPC(0);
		delayWithTicks(delayTime);
	}
	
	for (int i = 126; i >= 123; --i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(delayTime);
	}
	resetBrandonAnimSeqSize();
	_currentCharacter->currentAnimFrame = 7;
	animRefreshNPC(0);
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
	debug(9, "seq_winterScroll1()");
	_screen->hideMouse();
	checkAmuletAnimFlags();
	assert(_winterScrollTable);
	assert(_winterScroll1Table);
	assert(_winterScroll2Table);
	setupShapes123(_winterScrollTable, 7, 0);
	setBrandonAnimSeqSize(5, 66);
	
	for (int i = 123; i <= 129; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	
	freeShapes123();
	snd_playSoundEffect(0x20);

	uint8 endEncode, midpoint, endpoint;
	if (_features & GF_TALKIE) {
		endEncode = 18;
		midpoint = 135;
		endpoint = 140;
	} else {
		endEncode = 35;
		midpoint = 146;
		endpoint = 157;
	}
	setupShapes123(_winterScroll1Table, endEncode, 0);
	for (int i = 123; i <= midpoint; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
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
	
	for (int i = midpoint+1; i <= endpoint; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
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
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	
	resetBrandonAnimSeqSize();
	_currentCharacter->currentAnimFrame = 7;
	animRefreshNPC(0);
	freeShapes123();
	_screen->showMouse();
}

void KyraEngine::seq_winterScroll2() {
	debug(9, "seq_winterScroll2()");	
	_screen->hideMouse();
	checkAmuletAnimFlags();
	assert(_winterScrollTable);
	setupShapes123(_winterScrollTable, 7, 0);
	setBrandonAnimSeqSize(5, 66);
	
	for (int i = 123; i <= 128; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	
	delayWithTicks(120);
	
	for (int i = 127; i >= 123; --i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	
	resetBrandonAnimSeqSize();
	_currentCharacter->currentAnimFrame = 7;
	animRefreshNPC(0);
	freeShapes123();
	_screen->showMouse();
}

void KyraEngine::seq_makeBrandonInv() {
	debug(9, "seq_makeBrandonInv()");
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
		animRefreshNPC(0);
		delayWithTicks(10);
		_brandonInvFlag += 0x10;
	}
	_brandonStatusBit &= 0xFFBF;
	_screen->showMouse();
}

void KyraEngine::seq_makeBrandonNormal() {
	debug(9, "seq_makeBrandonNormal()");
	_screen->hideMouse();
	_brandonStatusBit |= 0x40;
	snd_playSoundEffect(0x77);
	_brandonInvFlag = 0x100;
	while (_brandonInvFlag >= 0) {
		animRefreshNPC(0);
		delayWithTicks(10);
		_brandonInvFlag -= 0x10;
	}
	_brandonInvFlag = 0;
	_brandonStatusBit &= 0xFF9F;
	_screen->showMouse();
}

void KyraEngine::seq_makeBrandonNormal2() {
	debug(9, "seq_makeBrandonNormal2()");
	_screen->hideMouse();
	assert(_brandonToWispTable);
	setupShapes123(_brandonToWispTable, 26, 0);
	setBrandonAnimSeqSize(5, 48);
	_brandonStatusBit &= 0xFFFD;
	snd_playSoundEffect(0x6C);
	for (int i = 138; i >= 123; --i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	setBrandonAnimSeqSize(4, 48);
	_currentCharacter->currentAnimFrame = 7;
	animRefreshNPC(0);
	if (_currentCharacter->sceneId >= 229 && _currentCharacter->sceneId <= 245) {
		_screen->fadeSpecialPalette(31, 234, 13, 4);
	} else if (_currentCharacter->sceneId >= 118 && _currentCharacter->sceneId <= 186) {
		_screen->fadeSpecialPalette(14, 228, 15, 4);
	}
	freeShapes123();
	_screen->showMouse();
}

void KyraEngine::seq_makeBrandonWisp() {
	debug(9, "seq_makeBrandonWisp()");
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
	setBrandonAnimSeqSize(5, 48);
	snd_playSoundEffect(0x6C);
	for (int i = 123; i <= 138; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	_brandonStatusBit |= 2;
	if (_currentCharacter->sceneId >= 109 && _currentCharacter->sceneId <= 198) {
		setTimerCountdown(14, 18000);
	} else {
		setTimerCountdown(14, 7200);
	}
	_brandonDrawFrame = 113;
	_brandonStatusBit0x02Flag = 1;
	_currentCharacter->currentAnimFrame = 113;
	animRefreshNPC(0);
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
	debug(9, "seq_dispelMagicAnimation()");
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
	setBrandonAnimSeqSize(8, 49);
	snd_playSoundEffect(0x15);
	for (int i = 123; i <= 127; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	
	delayWithTicks(120);
	
	for (int i = 127; i >= 123; --i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(10);
	}
	resetBrandonAnimSeqSize();
	_currentCharacter->currentAnimFrame = 7;
	animRefreshNPC(0);
	freeShapes123();
	_screen->showMouse();
}

void KyraEngine::seq_fillFlaskWithWater(int item, int type) {
	debug(9, "seq_fillFlaskWithWater(%d, %d)", item, type);
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

void KyraEngine::seq_playDrinkPotionAnim(int unk1, int unk2, int flags) {
	debug(9, "KyraEngine::seq_playDrinkPotionAnim(%d, %d, %d)", unk1, unk2, flags);
	// XXX
	_screen->hideMouse();
	checkAmuletAnimFlags();
	_currentCharacter->facing = 5;
	animRefreshNPC(0);
	assert(_drinkAnimationTable);
	setupShapes123(_drinkAnimationTable, 9, flags);
	setBrandonAnimSeqSize(5, 54);
	
	for (int i = 123; i <= 131; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(5);
	}	
	snd_playSoundEffect(0x34);
	for (int i = 0; i < 2; ++i) {
		_currentCharacter->currentAnimFrame = 130;
		animRefreshNPC(0);
		delayWithTicks(7);
		_currentCharacter->currentAnimFrame = 131;
		animRefreshNPC(0);
		delayWithTicks(7);
	}
	
	if (unk2) {
		// XXX
	}
	
	for (int i = 131; i >= 123; --i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(5);
	}
	
	resetBrandonAnimSeqSize();	
	_currentCharacter->currentAnimFrame = 7;
	animRefreshNPC(0);
	freeShapes123();
	_screen->showMouse();
}

int KyraEngine::seq_playEnd() {
	debug(9, "KyraEngine::seq_playEnd()");
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
	debug(9, "KyraEngine::seq_brandonToStone()");
	_screen->hideMouse();
	assert(_brandonStoneTable);
	setupShapes123(_brandonStoneTable, 14, 0);
	setBrandonAnimSeqSize(5, 51);
	for (int i = 123; i <= 136; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	resetBrandonAnimSeqSize();
	freeShapes123();
	_screen->showMouse();
}

void KyraEngine::seq_playEnding() {
	debug(9, "KyraEngine::seq_playEnding()");
	_screen->hideMouse();
	res_unloadResources(RES_INGAME);
	res_loadResources(RES_OUTRO);
	loadBitmap("REUNION.CPS", 3, 3, _screen->_currentPalette);
	_screen->copyRegion(8, 8, 8, 8, 304, 128, 2, 0);
	_screen->_curPage = 0;
	// XXX
	assert(_homeString);
	drawSentenceCommand(_homeString[0], 179);
	_screen->_curPage = 0;
	_screen->fadeToBlack();
	_seq->playSequence(_seq_Reunion, false);
	_screen->fadeToBlack();
	_screen->showMouse();
	seq_playCredits();
}

void KyraEngine::seq_playCredits() {
	debug(9, "KyraEngine::seq_playCredits()");
	static const uint8 colorMap[] = { 0, 0, 0xC, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	_screen->hideMouse();
	uint32 sz = 0;
	if (_features & GF_FLOPPY) {
		_screen->loadFont(Screen::FID_CRED6_FNT, _res->fileData("CREDIT6.FNT", &sz));
		_screen->loadFont(Screen::FID_CRED8_FNT, _res->fileData("CREDIT8.FNT", &sz));
	}
	loadBitmap("CHALET.CPS", 2, 2, _screen->_currentPalette);
	_screen->setScreenPalette(_screen->_currentPalette);
	_screen->setCurPage(0);
	_screen->clearCurPage();
	_screen->copyRegion(8, 8, 8, 8, 304, 128, 2, 0);
	_screen->setTextColorMap(colorMap);
	_screen->_charWidth = -1;
	snd_playWanderScoreViaMap(53, 1);
	// delete
	_screen->updateScreen();
	// XXX
	delay(120 * _tickLength); // wait until user presses escape normally
	_screen->fadeToBlack();
	_screen->clearCurPage();
	_screen->showMouse();
}

bool KyraEngine::seq_skipSequence() const {
	debug(9, "KyraEngine::seq_skipSequence()");
	return _quitFlag || _abortIntroFlag;
}

} // end of namespace Kyra
