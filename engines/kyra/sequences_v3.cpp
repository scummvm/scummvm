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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "kyra/kyra_v3.h"

namespace Kyra {

void KyraEngine_v3::showBadConscience() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::showBadConscience()");
	if (_badConscienceShown)
		return;

	_badConscienceShown = true;
	_badConscienceAnim = _rnd.getRandomNumberRng(0, 2);
	if (_currentChapter == 2)
		_badConscienceAnim = 5;
	else if (_currentChapter == 3)
		_badConscienceAnim = 3;
	else if (_currentChapter == 4 && _rnd.getRandomNumberRng(1, 100) <= 25)
		_badConscienceAnim = 6;
	else if (_currentChapter == 5 && _rnd.getRandomNumberRng(1, 100) <= 25)
		_badConscienceAnim = 7;
	else if (_malcolmShapes == 9)
		_badConscienceAnim = 4;

	_badConsciencePosition = (_mainCharacter.x1 <= 160);

	//if (_goodConscienceShown)
	//	_badConsciencePosition = !_goodConsciencePosition;
	
	int anim = _badConscienceAnim + (_badConsciencePosition ? 0 : 8);
	TalkObject &talkObject = _talkObjectList[1];

	if (_badConsciencePosition)
		talkObject.x = 290;
	else
		talkObject.x = 30;
	talkObject.y = 30;

	static const char *animFilenames[] = {
		"GUNFL00.WSA", "GUNFL01.WSA", "GUNFL02.WSA", "GUNFL03.WSA", "GUNFL04.WSA", "GUNFL05.WSA", "GUNFL06.WSA", "GUNFL07.WSA",
		"GUNFR00.WSA", "GUNFR01.WSA", "GUNFR02.WSA", "GUNFR03.WSA", "GUNFR04.WSA", "GUNFR05.WSA", "GUNFR06.WSA", "GUNFR07.WSA"
	};

	setupSceneAnimObject(0x0E, 9, 0, 187, -1, -1, -1, -1, 0, 0, 0, -1, animFilenames[anim]);
	for (uint i = 0; i <= _badConscienceFrameTable[_badConscienceAnim]; ++i) {
		if (i == 8)
			playSoundEffect(0x1B, 0xC8);
		updateSceneAnim(0x0E, i);
		delay(3*_tickLength, true);
	}

	if (_mainCharacter.animFrame < 50 || _mainCharacter.animFrame > 87)
		return;

	if (_mainCharacter.y1 == -1 || (_mainCharacter.x1 != -1 && _mainCharacter.animFrame == 87) || _mainCharacter.animFrame == 87) {
		_mainCharacter.animFrame = 87;
	} else {
		if (_badConsciencePosition)
			_mainCharacter.facing = 3;
		else
			_mainCharacter.facing = 5;
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
	}

	updateCharacterAnim(0);
	refreshAnimObjectsIfNeed();
}

void KyraEngine_v3::hideBadConscience() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::hideBadConscience()");
	if (!_badConscienceShown)
		return;

	_badConscienceShown = false;
	for (int frame = _badConscienceFrameTable[_badConscienceAnim+8]; frame >= 0; --frame) {
		if (frame == 15)
			playSoundEffect(0x31, 0xC8);
		updateSceneAnim(0x0E, frame);
		delay(1*_tickLength, true);
	}

	updateSceneAnim(0x0E, -1);
	update();
	removeSceneAnimObject(0x0E, 1);
	//setNextIdleAnimTimer();
}

} // end of namespace Kyra

