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
 */

#include "dreamweb/dreamweb.h"

namespace DreamGen {

void DreamBase::printSprites() {
	for (size_t priority = 0; priority < 7; ++priority) {
		Common::List<Sprite>::const_iterator i;
		for (i = _spriteTable.begin(); i != _spriteTable.end(); ++i) {
			const Sprite &sprite = *i;
			assert(sprite._updateCallback != 0x0ffff);
			if (priority != sprite.priority)
				continue;
			if (sprite.hidden == 1)
				continue;
			printASprite(&sprite);
		}
	}
}

void DreamBase::printASprite(const Sprite *sprite) {
	uint16 x, y;
	if (sprite->y >= 220) {
		y = data.word(kMapady) - (256 - sprite->y);
	} else {
		y = sprite->y + data.word(kMapady);
	}

	if (sprite->x >= 220) {
		x = data.word(kMapadx) - (256 - sprite->x);
	} else {
		x = sprite->x + data.word(kMapadx);
	}
	
	uint8 c;
	if (sprite->walkFrame != 0)
		c = 8;
	else
		c = 0;
	showFrame((const Frame *)getSegment(sprite->_frameData).ptr(0, 0), x, y, sprite->frameNumber, c);
}

void DreamBase::clearSprites() {
	_spriteTable.clear();
}

Sprite *DreamBase::makeSprite(uint8 x, uint8 y, uint16 updateCallback, uint16 frameData, uint16 somethingInDi) {
	// Note: the original didn't append sprites here, but filled up the
	// first unused entry. This can change the order of entries, but since they
	// are drawn based on the priority field, this shouldn't matter.
	_spriteTable.push_back(Sprite());
	Sprite *sprite = &_spriteTable.back();

	memset(sprite, 0xff, sizeof(Sprite));

	sprite->_updateCallback = updateCallback;
	sprite->x = x;
	sprite->y = y;
	sprite->_frameData = frameData;
	sprite->w8 = somethingInDi;
	sprite->w2 = 0xffff;
	sprite->frameNumber = 0;
	sprite->delay = 0;
	return sprite;
}

void DreamBase::spriteUpdate() {
	// During the intro the sprite table can be empty
	if (!_spriteTable.empty())
		_spriteTable.front().hidden = data.byte(kRyanon);

	Common::List<Sprite>::iterator i;
	for (i = _spriteTable.begin(); i != _spriteTable.end(); ++i) {
		Sprite &sprite = *i;
		assert(sprite._updateCallback != 0xffff);

		sprite.w24 = sprite.w2;
		if (sprite._updateCallback == addr_mainman) // NB : Let's consider the callback as an enum while more code is not ported to C++
			mainMan(&sprite);
		else {
			assert(sprite._updateCallback == addr_backobject);
			backObject(&sprite);
		}
	
		if (data.byte(kNowinnewroom) == 1)
			break;
	}
}

void DreamBase::initMan() {
	Sprite *sprite = makeSprite(data.byte(kRyanx), data.byte(kRyany), addr_mainman, data.word(kMainsprites), 0);
	sprite->priority = 4;
	sprite->speed = 0;
	sprite->walkFrame = 0;
}

void DreamBase::mainMan(Sprite *sprite) {
	if (data.byte(kResetmanxy) == 1) {
		data.byte(kResetmanxy) = 0;
		sprite->x = data.byte(kRyanx);
		sprite->y = data.byte(kRyany);
		sprite->walkFrame = 0;
	}

	--sprite->speed;
	if (sprite->speed != 0xff)
		return;
	sprite->speed = 0;

	if (data.byte(kTurntoface) != data.byte(kFacing)) {
		aboutTurn(sprite);
	} else {
		if ((data.byte(kTurndirection) != 0) && (data.byte(kLinepointer) == 254)) {
			data.byte(kReasseschanges) = 1;
			if (data.byte(kFacing) == data.byte(kLeavedirection))
				checkForExit(sprite);
		}
		data.byte(kTurndirection) = 0;
		if (data.byte(kLinepointer) == 254) {
			sprite->walkFrame = 0;
		} else {
			++sprite->walkFrame;
			if (sprite->walkFrame == 11)
				sprite->walkFrame = 1;
			walking(sprite);
			if (data.byte(kLinepointer) != 254) {
				if ((data.byte(kFacing) & 1) == 0)
					walking(sprite);
				else if ((sprite->walkFrame != 2) && (sprite->walkFrame != 7))
					walking(sprite);
			}
			if (data.byte(kLinepointer) == 254) {
				if (data.byte(kTurntoface) == data.byte(kFacing)) {
					data.byte(kReasseschanges) = 1;
					if (data.byte(kFacing) == data.byte(kLeavedirection))
						checkForExit(sprite);
				}
			}
		}
	}
	static const uint8 facelist[] = { 0,60,33,71,11,82,22,93 };
	sprite->frameNumber = sprite->walkFrame + facelist[data.byte(kFacing)];
	data.byte(kRyanx) = sprite->x;
	data.byte(kRyany) = sprite->y;
}

void DreamBase::walking(Sprite *sprite) {
	uint8 comp;
	if (data.byte(kLinedirection) != 0) {
		--data.byte(kLinepointer);
		comp = 200;
	} else {
		++data.byte(kLinepointer);
		comp = data.byte(kLinelength);
	}
	if (data.byte(kLinepointer) < comp) {
		sprite->x = (uint8)_lineData[data.byte(kLinepointer)].x;
		sprite->y = (uint8)_lineData[data.byte(kLinepointer)].y;
		return;
	}

	data.byte(kLinepointer) = 254;
	data.byte(kManspath) = data.byte(kDestination);
	if (data.byte(kDestination) == data.byte(kFinaldest)) {
		faceRightWay();
		return;
	}
	data.byte(kDestination) = data.byte(kFinaldest);
	autoSetWalk();
}

void DreamBase::aboutTurn(Sprite *sprite) {
	bool incdir = true;

	if (data.byte(kTurndirection) == 1)
		incdir = true;
	else if ((int8)data.byte(kTurndirection) == -1)
		incdir = false;
	else {
		if (data.byte(kFacing) < data.byte(kTurntoface)) {
			uint8 delta = data.byte(kTurntoface) - data.byte(kFacing);
			if (delta >= 4)
				incdir = false;
			else
				incdir = true;
		} else {
			uint8 delta = data.byte(kFacing) - data.byte(kTurntoface);
			if (delta >= 4)
				incdir = true;
			else
				incdir = false;
		}
	}

	if (incdir) {
		data.byte(kTurndirection) = 1;
		data.byte(kFacing) = (data.byte(kFacing) + 1) & 7;
		sprite->walkFrame = 0;
	} else {
		data.byte(kTurndirection) = (uint8)-1;
		data.byte(kFacing) = (data.byte(kFacing) - 1) & 7;
		sprite->walkFrame = 0;
	}
}

void DreamBase::backObject(Sprite *sprite) {
	SetObject *objData = (SetObject *)getSegment(data.word(kSetdat)).ptr(sprite->_objData, 0);

	if (sprite->delay != 0) {
		--sprite->delay;
		return;
	}

	sprite->delay = objData->delay;
	if (objData->type == 6)
		wideDoor(sprite, objData);
	else if (objData->type == 5)
		randomSprite(sprite, objData);
	else if (objData->type == 4)
		lockedDoorway(sprite, objData);
	else if (objData->type == 3)
		liftSprite(sprite, objData);
	else if (objData->type == 2)
		doorway(sprite, objData);
	else if (objData->type == 1)
		constant(sprite, objData);
	else
		steady(sprite, objData);
}

void DreamBase::constant(Sprite *sprite, SetObject *objData) {
	++sprite->animFrame;
	if (objData->frames[sprite->animFrame] == 255) {
		sprite->animFrame = 0;
	}
	uint8 frame = objData->frames[sprite->animFrame];
	objData->index = frame;
	sprite->frameNumber = frame;
}

void DreamBase::randomSprite(Sprite *sprite, SetObject *objData) {
	uint8 r = engine->randomNumber();
	sprite->frameNumber = objData->frames[r&7];
}

void DreamBase::doorway(Sprite *sprite, SetObject *objData) {
	Common::Rect check(-24, -30, 10, 10);
	doDoor(sprite, objData, check);
}

void DreamBase::wideDoor(Sprite *sprite, SetObject *objData) {
	Common::Rect check(-24, -30, 24, 24);
	doDoor(sprite, objData, check);
}

void DreamBase::doDoor(Sprite *sprite, SetObject *objData, Common::Rect check) {
	int ryanx = data.byte(kRyanx);
	int ryany = data.byte(kRyany);

	// Automatically opening doors: check if Ryan is in range

	check.translate(sprite->x, sprite->y);
	bool openDoor = check.contains(ryanx, ryany);

	if (openDoor) {

		if ((data.byte(kThroughdoor) == 1) && (sprite->animFrame == 0))
			sprite->animFrame = 6;

		++sprite->animFrame;
		if (sprite->animFrame == 1) { // doorsound2
			uint8 soundIndex;
			if (data.byte(kReallocation) == 5) // hoteldoor2
				soundIndex = 13;
			else
				soundIndex = 0;
			playChannel1(soundIndex);
		}
		if (objData->frames[sprite->animFrame] == 255)
			--sprite->animFrame;

		sprite->frameNumber = objData->index = objData->frames[sprite->animFrame];
		data.byte(kThroughdoor) = 1;

	} else {
		// shut door

		if (sprite->animFrame == 5) { // doorsound1;
			uint8 soundIndex;
			if (data.byte(kReallocation) == 5) // hoteldoor1
				soundIndex = 13;
			else
				soundIndex = 1;
			playChannel1(soundIndex);
		}
		if (sprite->animFrame != 0)
			--sprite->animFrame;

		sprite->frameNumber = objData->index = objData->frames[sprite->animFrame];
		if (sprite->animFrame == 5) // nearly
			data.byte(kThroughdoor) = 0;
	}
}

void DreamBase::steady(Sprite *sprite, SetObject *objData) {
	uint8 frame = objData->frames[0];
	objData->index = frame;
	sprite->frameNumber = frame;
}

void DreamBase::lockedDoorway(Sprite *sprite, SetObject *objData) {
	int ryanx = data.byte(kRyanx);
	int ryany = data.byte(kRyany);

	Common::Rect check(-24, -30, 10, 12);
	check.translate(sprite->x, sprite->y);
	bool openDoor = check.contains(ryanx, ryany);

	if (data.byte(kThroughdoor) != 1 && data.byte(kLockstatus) == 1)
		openDoor = false;

	if (openDoor) {

		if (sprite->animFrame == 1) {
			playChannel1(0);
		}

		if (sprite->animFrame == 6)
			turnPathOn(data.byte(kDoorpath));

		if (data.byte(kThroughdoor) == 1 && sprite->animFrame == 0)
			sprite->animFrame = 6;

		++sprite->animFrame;
		if (objData->frames[sprite->animFrame] == 255)
			--sprite->animFrame;

		sprite->frameNumber = objData->index = objData->frames[sprite->animFrame];
		if (sprite->animFrame == 5)
			data.byte(kThroughdoor) = 1;

	} else {
		// shut door

		if (sprite->animFrame == 5) {
			playChannel1(1);
		}

		if (sprite->animFrame != 0)
			--sprite->animFrame;
	
		data.byte(kThroughdoor) = 0;
		sprite->frameNumber = objData->index = objData->frames[sprite->animFrame];

		if (sprite->animFrame == 0) {
			turnPathOff(data.byte(kDoorpath));
			data.byte(kLockstatus) = 1;
		}
	}
}

void DreamBase::liftSprite(Sprite *sprite, SetObject *objData) {
	uint8 liftFlag = data.byte(kLiftflag);
	if (liftFlag == 0) { //liftclosed
		turnPathOff(data.byte(kLiftpath));

		if (data.byte(kCounttoopen) != 0) {
			data.byte(kCounttoopen)--;
			if (data.byte(kCounttoopen) == 0)
				data.byte(kLiftflag) = 3;
		}
		sprite->animFrame = 0;
		sprite->frameNumber = objData->index = objData->frames[sprite->animFrame];
	}
	else if (liftFlag == 1) {  //liftopen
		turnPathOn(data.byte(kLiftpath));

		if (data.byte(kCounttoclose) != 0) {
			data.byte(kCounttoclose)--;
			if (data.byte(kCounttoclose) == 0)
				data.byte(kLiftflag) = 2;
		}
		sprite->animFrame = 12;
		sprite->frameNumber = objData->index = objData->frames[sprite->animFrame];
	}	
	else if (liftFlag == 3) { //openlift
		if (sprite->animFrame == 12) {
			data.byte(kLiftflag) = 1;
			return;
		}
		++sprite->animFrame;
		if (sprite->animFrame == 1) {
			liftNoise(2);
		}
		sprite->frameNumber = objData->index = objData->frames[sprite->animFrame];
	} else { //closeLift
		assert(liftFlag == 2);
		if (sprite->animFrame == 0) {
			data.byte(kLiftflag) = 0;
			return;
		}
		--sprite->animFrame;
		if (sprite->animFrame == 11) {
			liftNoise(3);
		}
		sprite->frameNumber = objData->index = objData->frames[sprite->animFrame];
	}
}

Reel *DreamBase::getReelStart(uint16 reelPointer) {
	Reel *reel = (Reel *)getSegment(data.word(kReels)).ptr(kReellist + reelPointer * sizeof(Reel) * 8, sizeof(Reel));
	return reel;
}

// Locate the reel segment (reel1, reel2, reel3) this frame is stored in,
// and adjust the frame number relative to this segment.
const Frame *DreamBase::findSource(uint16 &frame) {
	uint16 base;
	if (frame < 160) {
		base = data.word(kReel1);
	} else if (frame < 320) {
		frame -= 160;
		base = data.word(kReel2);
	} else {
		frame -= 320;
		base = data.word(kReel3);
	}
	return (const Frame *)getSegment(base).ptr(0, (frame+1)*sizeof(Frame));
}

void DreamBase::showReelFrame(Reel *reel) {
	uint16 x = reel->x + data.word(kMapadx);
	uint16 y = reel->y + data.word(kMapady);
	uint16 frame = reel->frame();
	const Frame *base = findSource(frame);
	showFrame(base, x, y, frame, 8);
}

void DreamBase::showGameReel(ReelRoutine *routine) {
	uint16 reelPointer = routine->reelPointer();
	if (reelPointer >= 512)
		return;
	plotReel(reelPointer);
	routine->setReelPointer(reelPointer);
}

const Frame *DreamBase::getReelFrameAX(uint16 frame) {
	const Frame *base = findSource(frame);
	return base + frame;
}

void DreamBase::showRain() {
	Common::List<Rain>::iterator i;

	// Do nothing if there's no rain at all
	if (_rainList.empty())
		return;

	const Frame *frame = (const Frame *)getSegment(data.word(kMainsprites)).ptr(58 * sizeof(Frame), sizeof(Frame));
	const uint8 *frameData = getSegment(data.word(kMainsprites)).ptr(kFrframes + frame->ptr(), 512);

	for (i = _rainList.begin(); i != _rainList.end(); ++i) {
		Rain &rain = *i;
		uint16 y = rain.y + data.word(kMapady) + data.word(kMapystart);
		uint16 x = rain.x + data.word(kMapadx) + data.word(kMapxstart);
		uint16 size = rain.size;
		uint16 offset = (rain.w3 - rain.b5) & 511;
		rain.w3 = offset;
		const uint8 *src = frameData + offset;
		uint8 *dst = workspace() + y * 320 + x;
		for (uint16 j = 0; j < size; ++j) {
			uint8 v = src[j];
			if (v != 0)
				*dst = v;
			dst += 320-1; // advance diagonally
		}
	}

	if (data.word(kCh1blockstocopy) != 0)
		return;
	if (data.byte(kReallocation) == 2 && data.byte(kBeenmugged) != 1)
		return;
	if (data.byte(kReallocation) == 55)
		return;

	if (engine->randomNumber() >= 1) // play thunder with 1 in 256 chance
		return;

	uint8 soundIndex;
	if (data.byte(kCh0playing) != 6)
		soundIndex = 4;
	else
		soundIndex = 7;
	playChannel1(soundIndex);
}

void DreamBase::moveMap(uint8 param) {
	switch (param) {
	case 32:
		data.byte(kMapy) -= 20;
		break;
	case 16:
		data.byte(kMapy) -= 10;
		break;
	case 8:
		data.byte(kMapy) += 10;
		break;
	case 2:
		data.byte(kMapx) += 11;
		break;
	default:
		data.byte(kMapx) -= 11;
		break;
	}
	data.byte(kNowinnewroom) = 1;
}

void DreamGenContext::checkOne() {
	uint8 flag, flagEx, type, flagX, flagY;
	checkOne(cl, ch, &flag, &flagEx, &type, &flagX, &flagY);

	cl = flag;
	ch = flagEx;
	dl = flagX;
	dh = flagY;
	al = type;
}

void DreamBase::checkOne(uint8 x, uint8 y, uint8 *flag, uint8 *flagEx, uint8 *type, uint8 *flagX, uint8 *flagY) {
	*flagX = x / 16;
	*flagY = y / 16;
	const uint8 *tileData = &_mapFlags[(*flagY * 11 + *flagX) * 3];
	*flag = tileData[0];
	*flagEx = tileData[1];
	*type = tileData[2];
}

uint8 DreamBase::getBlockOfPixel(uint8 x, uint8 y) {
	uint8 flag, flagEx, type, flagX, flagY;
	checkOne(x + data.word(kMapxstart), y + data.word(kMapystart), &flag, &flagEx, &type, &flagX, &flagY);
	if (flag & 1)
		return 0;
	else
		return type;
}

void DreamBase::splitIntoLines(uint8 x, uint8 y) {
	do {
		Rain rain;

		// Look for line start
		while (!getBlockOfPixel(x, y)) {
			--x;
			++y;
			if (x == 0 || y >= data.byte(kMapysize))
				return;
		}

		rain.x = x;
		rain.y = y;

		uint8 length = 1;

		// Look for line end
		while (getBlockOfPixel(x, y)) {
			--x;
			++y;
			if (x == 0 || y >= data.byte(kMapysize))
				break;
			++length;
		}

		rain.size = length;
		rain.w3 = (engine->randomNumber() << 8) | engine->randomNumber();
		rain.b5 = (engine->randomNumber() & 3) + 4;
		_rainList.push_back(rain);
	} while (x > 0 && y < data.byte(kMapysize));
}

struct RainLocation {
	uint8 location;
	uint8 x, y;
	uint8 rainSpacing;
};

static const RainLocation rainLocationList[] = {
	{ 1,44,10,16 },
	{ 4,11,30,14 },
	{ 4,22,30,14 },
	{ 3,33,10,14 },
	{ 10,33,30,14 },
	{ 10,22,30,24 },
	{ 9,22,10,14 },
	{ 2,33,0,14 },
	{ 2,22,0,14 },
	{ 6,11,30,14 },
	{ 7,11,20,18 },
	{ 7,0,20,18 },
	{ 7,0,30,18 },
	{ 55,44,0,14 },
	{ 5,22,30,14 },

	{ 8,0,10,18 },
	{ 8,11,10,18 },
	{ 8,22,10,18 },
	{ 8,33,10,18 },
	{ 8,33,20,18 },
	{ 8,33,30,18 },
	{ 8,33,40,18 },
	{ 8,22,40,18 },
	{ 8,11,40,18 },

	{ 21,44,20,18 },
	{ 255,0,0,0 }
};

void DreamBase::initRain() {
	const RainLocation *r = rainLocationList;
	_rainList.clear();

	uint8 rainSpacing = 0;

	// look up location in rainLocationList to determine rainSpacing
	for (r = rainLocationList; r->location != 0xff; ++r) {
		if (r->location == data.byte(kReallocation) &&
		        r->x == data.byte(kMapx) && r->y == data.byte(kMapy)) {
			rainSpacing = r->rainSpacing;
			break;
		}
	}

	if (rainSpacing == 0) {
		// location not found in rainLocationList: no rain
		return;
	}

	// start lines of rain from top of screen
	uint8 x = 4;
	do {
		uint8 delta;
		do {
			delta = (engine->randomNumber() & 31) + 3;
		} while (delta >= rainSpacing);

		x += delta;
		if (x >= data.byte(kMapxsize))
			break;

		splitIntoLines(x, 0);
	} while (true);

	// start lines of rain from side of screen
	uint8 y = 0;
	do {
		uint8 delta;
		do {
			delta = (engine->randomNumber() & 31) + 3;
		} while (delta >= rainSpacing);

		y += delta;
		if (y >= data.byte(kMapysize))
			break;

		splitIntoLines(data.byte(kMapxsize) - 1, y);
	} while (true);
}

void DreamBase::intro1Text() {
	if (data.byte(kIntrocount) != 2 && data.byte(kIntrocount) != 4 && data.byte(kIntrocount) != 6)
		return;

	if (isCD() && data.byte(kCh1playing) != 255) {
		data.byte(kIntrocount)--;
	} else {
		if (data.byte(kIntrocount) == 2)
			setupTimedTemp(40, 82, 34, 130, 90, 1);
		else if (data.byte(kIntrocount) == 4)
			setupTimedTemp(41, 82, 34, 130, 90, 1);
		else if (data.byte(kIntrocount) == 6)
			setupTimedTemp(42, 82, 34, 130, 90, 1);
	}
}

void DreamBase::intro2Text(uint16 nextReelPointer) {
	if (nextReelPointer == 5)
		setupTimedTemp(43, 82, 34, 40, 90, 1);
	else if (nextReelPointer == 15)
		setupTimedTemp(44, 82, 34, 40, 90, 1);
}

void DreamBase::intro3Text(uint16 nextReelPointer) {
	if (nextReelPointer == 107)
		setupTimedTemp(45, 82, 36, 56, 100, 1);
	else if (nextReelPointer == (isCD() ? 108 : 109))
		setupTimedTemp(46, 82, 36, 56, 100, 1);
}

void DreamBase::rollEndCredits() {
	playChannel0(16, 255);
	data.byte(kVolume) = 7;
	data.byte(kVolumeto) = 0;
	data.byte(kVolumedirection) = (byte)-1;

	multiGet(mapStore(), 75, 20, 160, 160);

	const uint8 *string = getTextInFile1(3);
	const int linespacing = data.word(kLinespacing);

	for (int i = 0; i < 254; ++i) {
		// Output the text, initially with an offset of 10 pixels,
		// then move it up one pixel until we shifted it by a complete
		// line of text.
		for (int j = 0; j < linespacing; ++j) {
			vSync();
			multiPut(mapStore(), 75, 20, 160, 160);
			vSync();

			// Output up to 18 lines of text
			uint16 y = 10 - j;
			const uint8 *tmp_str = string;
			for (int k = 0; k < 18; ++k) {
				DreamBase::printDirect(&tmp_str, 75, &y, 160 + 1, true);
				y += linespacing;
			}

			vSync();
			multiDump(75, 20, 160, 160);
		}

		// Skip to the next text line
		byte c;
		do {
			c = *string++;
		} while (c != ':' && c != 0);
	}
	hangOn(100);
	panelToMap();
	fadeScreenUpHalf();
}


void DreamBase::monks2text() {
	bool isGermanCD = isCD() && engine->getLanguage() == Common::DE_DEU;

	if (data.byte(kIntrocount) == 1)
		setupTimedTemp(8, 82, 36, 160, 120, 1);
	else if (data.byte(kIntrocount) == (isGermanCD ? 5 : 4))
		setupTimedTemp(9, 82, 36, 160, 120, 1);
	else if (data.byte(kIntrocount) == (isGermanCD ? 9 : 7))
		setupTimedTemp(10, 82, 36, 160, 120, 1);
	else if (data.byte(kIntrocount) == 10 && !isGermanCD) {
		if (isCD())
			data.byte(kIntrocount) = 12;
		setupTimedTemp(11, 82, 0, 105, 120, 1);
	} else if (data.byte(kIntrocount) == 13 && isGermanCD) {
		data.byte(kIntrocount) = 14;
		setupTimedTemp(11, 82, 0, 105, 120, 1);
	} else if (data.byte(kIntrocount) == 13 && !isGermanCD) {
		if (isCD())
			data.byte(kIntrocount) = 17;
		else
			setupTimedTemp(12, 82, 0, 120, 120, 1);
	} else if (data.byte(kIntrocount) == 16 && !isGermanCD)
		setupTimedTemp(13, 82, 0, 135, 120, 1);
	else if (data.byte(kIntrocount) == 19)
		setupTimedTemp(14, 82, 36, 160, 100, 1);
	else if (data.byte(kIntrocount) == (isGermanCD ? 23 : 22))
		setupTimedTemp(15, 82, 36, 160, 120, 1);
	else if (data.byte(kIntrocount) == (isGermanCD ? 27 : 25))
		setupTimedTemp(16, 82, 36, 160, 120, 1);
	else if (data.byte(kIntrocount) == (isCD() ? 27 : 28) && !isGermanCD)
		setupTimedTemp(17, 82, 36, 160, 120, 1);
	else if (data.byte(kIntrocount) == 30 && isGermanCD)
		setupTimedTemp(17, 82, 36, 160, 120, 1);
	else if (data.byte(kIntrocount) == (isGermanCD ? 35 : 31))
		setupTimedTemp(18, 82, 36, 160, 120, 1);
}

void DreamBase::textForEnd() {
	if (data.byte(kIntrocount) == 20)
		setupTimedTemp(0, 83, 34, 20, 60, 1);
	else if (data.byte(kIntrocount) == (isCD() ? 50 : 65))
		setupTimedTemp(1, 83, 34, 20, 60, 1);
	else if (data.byte(kIntrocount) == (isCD() ? 85 : 110))
		setupTimedTemp(2, 83, 34, 20, 60, 1);
}

void DreamBase::textForMonkHelper(uint8 textIndex, uint8 voiceIndex, uint8 x, uint8 y, uint16 countToTimed, uint16 timeCount) {
	if (isCD() && data.byte(kCh1playing) != 255)
		data.byte(kIntrocount)--;
	else
		setupTimedTemp(textIndex, voiceIndex, x, y, countToTimed, timeCount);
}

void DreamBase::textForMonk() {
	if (data.byte(kIntrocount) == 1)
		textForMonkHelper(19, 82, 68, 154, 120, 1);
	else if (data.byte(kIntrocount) == 5)
		textForMonkHelper(20, 82, 68, 38, 120, 1);
	else if (data.byte(kIntrocount) == 9)
		textForMonkHelper(21, 82, 48, 154, 120, 1);
	else if (data.byte(kIntrocount) == 13)
		textForMonkHelper(22, 82, 68, 38, 120, 1);
	else if (data.byte(kIntrocount) == (isCD() ? 15 : 17))
		textForMonkHelper(23, 82, 68, 154, 120, 1);
	else if (data.byte(kIntrocount) == 21)
		textForMonkHelper(24, 82, 68, 38, 120, 1);
	else if (data.byte(kIntrocount) == 25)
		textForMonkHelper(25, 82, 68, 154, 120, 1);
	else if (data.byte(kIntrocount) == 29)
		textForMonkHelper(26, 82, 68, 38, 120, 1);
	else if (data.byte(kIntrocount) == 33)
		textForMonkHelper(27, 82, 68, 154, 120, 1);
	else if (data.byte(kIntrocount) == 37)
		textForMonkHelper(28, 82, 68, 154, 120, 1);
	else if (data.byte(kIntrocount) == 41)
		textForMonkHelper(29, 82, 68, 38, 120, 1);
	else if (data.byte(kIntrocount) == 45)
		textForMonkHelper(30, 82, 68, 154, 120, 1);
	else if (data.byte(kIntrocount) == (isCD() ? 52 : 49))
		textForMonkHelper(31, 82, 68, 154, 220, 1);
	else if (data.byte(kIntrocount) == 53) {
		fadeScreenDowns();
		if (isCD()) {
			data.byte(kVolumeto) = 7;
			data.byte(kVolumedirection) = 1;
		}
	}
}

void DreamBase::reelsOnScreen() {
	reconstruct();
	updatePeople();
	watchReel();
	showRain();
	useTimedText();
}

void DreamBase::reconstruct() {
	if (data.byte(kHavedoneobs) == 0)
		return;
	data.byte(kNewobs) = 1;
	drawFloor();
	spriteUpdate();
	printSprites();
	if ((data.byte(kForeignrelease) != 0) && (data.byte(kReallocation) == 20))
		underTextLine();
	data.byte(kHavedoneobs) = 0;
}



struct ReelSound {
	uint8 _sample;
	uint16 _reelPointer;
};

static const ReelSound g_roomSound0[] = {
	{ 255,0 }
};

static const ReelSound g_roomSound1[] = {
	{ 15, 257 },
	{ 255,0 }
};

static const ReelSound g_roomSound2[] = {
	{ 12, 5 },
	{ 13, 21 },
	{ 15, 35 }, // hitting floor?
	{ 17, 50 },
	{ 18, 103 },
	{ 19, 108 },
	{ 255,0 }
};

static const ReelSound g_roomSound6[] = {
	{ 18, 19 },
	{ 19, 23 },
	{ 255,0 }
};
static const ReelSound g_roomSound8[] = {
	
	{ 12, 51 },
	{ 13, 53 },
	{ 14, 14 },
	{ 15, 20 },
	{ 0, 78 },
	{ 255,0 }
};
static const ReelSound g_roomSound9[] = {

	{ 12, 119 },
	{ 12, 145 },
	{ 255,0 }
};

static const ReelSound g_roomSound10[] = {
	{ 13, 16 },
	{ 255,0 }
};
	
static const ReelSound g_roomSound11[] = {
	{ 13, 20 },
	{ 255,0 }
};

static const ReelSound g_roomSound12[] = {
	{ 14, 16 },
	{ 255,0 }
};

static const ReelSound g_roomSound13[] = {
	{ 15, 4 },
	{ 16, 8 },
	{ 17, 134 },
	{ 18, 153 },
	{ 255,0 }
};

static const ReelSound g_roomSound14[] = {
	{ 13, 108 },
	{ 15, 326 },
	{ 15, 331 },
	{ 15, 336 },
	{ 15, 342 },
	{ 15, 348 },
	{ 15, 354 },
	{ 18, 159 },
	{ 18, 178 },
	{ 19+128, 217 },
	{ 20+64, 228 },
	{ 255,0 }
};

static const ReelSound g_roomSound20[] = {
	{ 13, 20 },
	{ 13, 21 },
	{ 15, 34 },
	{ 13, 52 },
	{ 13, 55 },
	{ 25, 57 },
	{ 21, 73 },
	{ 255,0 }
};

static const ReelSound g_roomSound22[] = {
	{ 13, 196 },
	{ 13, 234 },
	{ 13, 156 },
	{ 14, 129 },
	{ 13, 124 },
	{ 15, 162 },
	{ 15, 200 },
	{ 15, 239 },
	{ 17, 99 },
	{ 12, 52 },
	{ 255,0 }
};

static const ReelSound g_roomSound23[] = {
	{ 15, 56 },
	{ 16, 64 },
	{ 19, 22 },
	{ 20, 33 },
	{ 255,0 }
};

static const ReelSound g_roomSound25[] = {
	{ 20, 11 },
	{ 20, 15 },
	{ 15, 28 },
	{ 13, 80 },
	{ 21, 82 },
	{ 19+128, 87 },
	{ 23+64, 128 },
	{ 255,0 }
};

static const ReelSound g_roomSound26[] = {
	{ 12, 13 },
	{ 14, 39 },
	{ 12, 67 },
	{ 12, 75 },
	{ 12, 83 },
	{ 12, 91 },
	{ 15, 102 }, // was 90, should be mine cart
	{ 255,0 }
};
	
static const ReelSound g_roomSound27[] = {
	{ 22, 36 },
	{ 13, 125 },
	{ 18, 88 },
	{ 15, 107 },
	{ 14, 127 },
	{ 14, 154 },
	{ 19+128, 170 },
	{ 23+64, 232 },
	{ 255,0 }
};

static const ReelSound g_roomSound28[] = {
	{ 21, 16 },
	{ 21, 72 },
	{ 21, 205 },
	{ 22, 63 }, // 65
	{ 23+128, 99 },
	{ 24+64, 158 },
	{ 255,0 }
};

static const ReelSound g_roomSound29[] = {
	{ 13, 21 },
	{ 14, 24 },
	{ 19+128, 50 },
	{ 23+64, 75 },
	{ 24, 128 },
	{ 255,0 }
};

static const ReelSound g_roomSound29_German[] = {
	{ 13, 21 },
	{ 14, 24 },
	{ 19+128, 50 },
	{ 23+64, 75 },
	{ 255,0 }
};


static const ReelSound g_roomSound45[] = {
	{ 19+64, 46 },
	{ 16, 167 },
	{ 255,0 }
};

static const ReelSound g_roomSound46[] = {
	{ 16, 19 },
	{ 14, 36 },
	{ 16, 50 },
	{ 14, 65 },
	{ 16, 81 },
	{ 14, 96 },
	{ 16, 114 },
	{ 14, 129 },
	{ 16, 147 },
	{ 14, 162 },
	{ 16, 177 },
	{ 14, 191 },
	{ 255,0 }
};

static const ReelSound g_roomSound47[] = {
	{ 13, 48 },
	{ 14, 41 },
	{ 15, 78 },
	{ 16, 92 },
	{ 255,0 }
};

static const ReelSound g_roomSound52[] = {
	{ 16, 115 },
	{ 255,0 }
};

static const ReelSound g_roomSound53[] = {
	{ 21, 103 },
	{ 20, 199 },
	{ 255,0 }
};

static const ReelSound g_roomSound55[] = {
	{ 17, 53 },
	{ 17, 54 },
	{ 17, 55 },
	{ 17, 56 },
	{ 17, 57 },
	{ 17, 58 },
	{ 17, 59 },
	{ 17, 61 },
	{ 17, 63 },
	{ 17, 64 },
	{ 17, 65 },
	{ 255,0 }
};

static const ReelSound *g_roomByRoom[] = {
	g_roomSound0,g_roomSound1,g_roomSound2,g_roomSound0,g_roomSound0,
	g_roomSound0,g_roomSound6,g_roomSound0,g_roomSound8,g_roomSound9,
	g_roomSound10,g_roomSound11,g_roomSound12,g_roomSound13,g_roomSound14,
	g_roomSound0,g_roomSound0,g_roomSound0,g_roomSound0,g_roomSound0,
	g_roomSound20,g_roomSound0,g_roomSound22,g_roomSound23,g_roomSound0,
	g_roomSound25,g_roomSound26,g_roomSound27,g_roomSound28,g_roomSound29,
	g_roomSound0,g_roomSound0,g_roomSound0,g_roomSound0,g_roomSound0,
	g_roomSound0,g_roomSound0,g_roomSound0,g_roomSound0,g_roomSound0,
	g_roomSound0,g_roomSound0,g_roomSound0,g_roomSound0,g_roomSound0,
	g_roomSound45,g_roomSound46,g_roomSound47,g_roomSound0,g_roomSound0,
	g_roomSound0,g_roomSound0,g_roomSound52,g_roomSound53,g_roomSound0,
	g_roomSound55
};


void DreamBase::soundOnReels(uint16 reelPointer) {
	const ReelSound *r = g_roomByRoom[data.byte(kReallocation)];

	if (engine->getLanguage() == Common::DE_DEU && r == g_roomSound29)
		r = g_roomSound29_German;

	for (; r->_sample != 255; ++r) {
		if (r->_reelPointer != reelPointer)
			continue;
		if (r->_reelPointer == data.word(kLastsoundreel))
			continue;
		data.word(kLastsoundreel) = r->_reelPointer;
		if (r->_sample < 64) {
			playChannel1(r->_sample);
			return;
		}
		if (r->_sample < 128) {
			playChannel0(r->_sample & 63, 0);
			return;
		}
		playChannel0(r->_sample & 63, 255);
	}

	if (data.word(kLastsoundreel) != reelPointer)
		data.word(kLastsoundreel) = (uint16)-1;
}

void DreamBase::clearBeforeLoad() {
	if (data.byte(kRoomloaded) != 1)
		return /* (noclear) */;

	clearReels();

	//clearRest
	uint8 *dst = (uint8 *)getSegment(data.word(kMapdata)).ptr(0, 0);
	memset(dst, 0, kMaplen);
	deallocateMem(data.word(kBackdrop));
	deallocateMem(data.word(kSetframes));
	deallocateMem(data.word(kReels));
	deallocateMem(data.word(kPeople));
	deallocateMem(data.word(kSetdesc));
	deallocateMem(data.word(kBlockdesc));
	deallocateMem(data.word(kRoomdesc));
	deallocateMem(data.word(kFreeframes));
	deallocateMem(data.word(kFreedesc));

	data.byte(kRoomloaded) = 0;
}

void DreamBase::clearReels() {
	deallocateMem(data.word(kReel1));
	deallocateMem(data.word(kReel2));
	deallocateMem(data.word(kReel3));
}

void DreamBase::getRidOfReels() {
	if (data.byte(kRoomloaded) == 0)
		return /* (dontgetrid) */;

	deallocateMem(data.word(kReel1));
	deallocateMem(data.word(kReel2));
	deallocateMem(data.word(kReel3));
}

void DreamBase::liftNoise(uint8 index) {
	if (data.byte(kReallocation) == 5 || data.byte(kReallocation) == 21)
		playChannel1(13);	// hiss noise
	else
		playChannel1(index);
}

void DreamBase::checkForExit(Sprite *sprite) {
	uint8 flag, flagEx, type, flagX, flagY;
	checkOne(data.byte(kRyanx) + 12, data.byte(kRyany) + 12, &flag, &flagEx, &type, &flagX, &flagY);
	data.byte(kLastflag) = flag;

	if (flag & 64) {
		data.byte(kAutolocation) = flagEx;
		return;
	}

	if (!(flag & 32)) {
		if (flag & 4) {
			// adjust left
			data.byte(kLastflag) = 0;
			data.byte(kMapx) -= 11;
			sprite->x = 16 * flagEx;
			data.byte(kNowinnewroom) = 1;
		} else if (flag & 2) {
			// adjust right
			data.byte(kMapx) += 11;
			sprite->x = 16 * flagEx - 2;
			data.byte(kNowinnewroom) = 1;
		} else if (flag & 8) {
			// adjust down
			data.byte(kMapy) += 10;
			sprite->y = 16 * flagEx;
			data.byte(kNowinnewroom) = 1;
		} else if (flag & 16) {
			// adjust up
			data.byte(kMapy) -= 10;
			sprite->y = 16 * flagEx;
			data.byte(kNowinnewroom) = 1;
		}

		return;
	}

	if (data.byte(kReallocation) == 2) {
		// Can't leave Louis' until you found shoes

		int shoeCount = 0;
		if (isRyanHolding("WETA")) shoeCount++;
		if (isRyanHolding("WETB")) shoeCount++;

		if (shoeCount < 2) {
			uint8 text = shoeCount ? 43 : 42;
			setupTimedUse(text, 80, 10, 68, 64);

			data.byte(kTurntoface) = (data.byte(kFacing) + 4) & 7;
			return;
		}

	}

	data.byte(kNeedtotravel) = 1;
}

} // End of namespace DreamGen
