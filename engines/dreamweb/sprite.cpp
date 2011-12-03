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

Sprite *DreamGenContext::spriteTable() {
	Sprite *sprite = (Sprite *)getSegment(data.word(kBuffers)).ptr(kSpritetable, 16 * sizeof(Sprite));
	return sprite;
}

void DreamGenContext::printSprites() {
	for (size_t priority = 0; priority < 7; ++priority) {
		Sprite *sprites = spriteTable();
		for (size_t j = 0; j < 16; ++j) {
			const Sprite &sprite = sprites[j];
			if (sprite.updateCallback() == 0x0ffff)
				continue;
			if (priority != sprite.priority)
				continue;
			if (sprite.hidden == 1)
				continue;
			printASprite(&sprite);
		}
	}
}

void DreamGenContext::printASprite(const Sprite *sprite) {
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
	showFrame((const Frame *)getSegment(sprite->frameData()).ptr(0, 0), x, y, sprite->frameNumber, c);
}

void DreamGenContext::clearSprites() {
	memset(spriteTable(), 0xff, sizeof(Sprite) * 16);
}

Sprite *DreamGenContext::makeSprite(uint8 x, uint8 y, uint16 updateCallback, uint16 frameData, uint16 somethingInDi) {
	Sprite *sprite = spriteTable();
	while (sprite->frameNumber != 0xff) { // NB: No boundchecking in the original code either
		++sprite;
	}

	sprite->setUpdateCallback(updateCallback);
	sprite->x = x;
	sprite->y = y;
	sprite->setFrameData(frameData);
	WRITE_LE_UINT16(&sprite->w8, somethingInDi);
	sprite->w2 = 0xffff;
	sprite->frameNumber = 0;
	sprite->delay = 0;
	return sprite;
}

void DreamGenContext::spriteUpdate() {
	Sprite *sprites = spriteTable();
	sprites[0].hidden = data.byte(kRyanon);

	Sprite *sprite = sprites;
	for (size_t i=0; i < 16; ++i) {
		uint16 updateCallback = sprite->updateCallback();
		if (updateCallback != 0xffff) {
			sprite->w24 = sprite->w2;
			if (updateCallback == addr_mainman) // NB : Let's consider the callback as an enum while more code is not ported to C++
				mainMan(sprite);
			else {
				assert(updateCallback == addr_backobject);
				backObject(sprite);
			}
		}
	
		if (data.byte(kNowinnewroom) == 1)
			break;
		++sprite;
	}
}

void DreamGenContext::initMan() {
	Sprite *sprite = makeSprite(data.byte(kRyanx), data.byte(kRyany), addr_mainman, data.word(kMainsprites), 0);
	sprite->priority = 4;
	sprite->speed = 0;
	sprite->walkFrame = 0;
}

void DreamGenContext::mainMan() {
	assert(false);
}

void DreamGenContext::mainMan(Sprite *sprite) {
	push(es);
	push(ds);

	// Recover es:bx from sprite
	es = data.word(kBuffers);
	bx = kSpritetable;
	Sprite *sprites = (Sprite *)es.ptr(bx, sizeof(Sprite) * 16);
	bx += 32 * (sprite - sprites);
	//

	if (data.byte(kResetmanxy) == 1) {
		data.byte(kResetmanxy) = 0;
		sprite->x = data.byte(kRyanx);
		sprite->y = data.byte(kRyany);
		sprite->walkFrame = 0;
	}
	--sprite->speed;
	if (sprite->speed != 0xff) {
		ds = pop();
		es = pop();
		return;
	}
	sprite->speed = 0;
	if (data.byte(kTurntoface) != data.byte(kFacing)) {
		aboutTurn(sprite);
	} else {
		if ((data.byte(kTurndirection) != 0) && (data.byte(kLinepointer) == 254)) {
			data.byte(kReasseschanges) = 1;
			if (data.byte(kFacing) == data.byte(kLeavedirection))
				checkForExit();
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
						checkForExit();
				}
			}
		}
	}
	static const uint8 facelist[] = { 0,60,33,71,11,82,22,93 };
	sprite->frameNumber = sprite->walkFrame + facelist[data.byte(kFacing)];
	data.byte(kRyanx) = sprite->x;
	data.byte(kRyany) = sprite->y;

	ds = pop();
	es = pop();
}

void DreamGenContext::walking(Sprite *sprite) {
	uint8 comp;
	if (data.byte(kLinedirection) != 0) {
		--data.byte(kLinepointer);
		comp = 200;
	} else {
		++data.byte(kLinepointer);
		comp = data.byte(kLinelength);
	}
	if (data.byte(kLinepointer) < comp) {
		sprite->x = data.byte(kLinedata + data.byte(kLinepointer) * 2 + 0);
		sprite->y = data.byte(kLinedata + data.byte(kLinepointer) * 2 + 1);
		return;
	}

	data.byte(kLinepointer) = 254;
	data.byte(kManspath) = data.byte(kDestination);
	if (data.byte(kDestination) == data.byte(kFinaldest)) {
		faceRightWay();
		return;
	}
	data.byte(kDestination) = data.byte(kFinaldest);
	push(es);
	push(bx);
	autoSetWalk();
	bx = pop();
	es = pop();
}

void DreamGenContext::aboutTurn(Sprite *sprite) {
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

void DreamGenContext::backObject() {
	assert(false);
}

void DreamGenContext::backObject(Sprite *sprite) {
	SetObject *objData = (SetObject *)getSegment(data.word(kSetdat)).ptr(sprite->objData(), 0);

	if (sprite->delay != 0) {
		--sprite->delay;
		return;
	}

	sprite->delay = objData->delay;
	if (objData->type == 6)
		wideDoor(sprite, objData);
	else if (objData->type == 5)
		random(sprite, objData);
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

void DreamGenContext::constant(Sprite *sprite, SetObject *objData) {
	++sprite->animFrame;
	if (objData->frames[sprite->animFrame] == 255) {
		sprite->animFrame = 0;
	}
	uint8 frame = objData->frames[sprite->animFrame];
	objData->index = frame;
	sprite->frameNumber = frame;
}

void DreamGenContext::random(Sprite *sprite, SetObject *objData) {
	randomNum1();
	uint16 r = ax;
	sprite->frameNumber = objData->frames[r&7];
}

void DreamGenContext::doorway(Sprite *sprite, SetObject *objData) {
	Common::Rect check(-24, -30, 10, 10);
	doDoor(sprite, objData, check);
}

void DreamGenContext::wideDoor(Sprite *sprite, SetObject *objData) {
	Common::Rect check(-24, -30, 24, 24);
	doDoor(sprite, objData, check);
}

void DreamGenContext::doDoor(Sprite *sprite, SetObject *objData, Common::Rect check) {
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

void DreamGenContext::steady(Sprite *sprite, SetObject *objData) {
	uint8 frame = objData->frames[0];
	objData->index = frame;
	sprite->frameNumber = frame;
}

void DreamGenContext::lockedDoorway(Sprite *sprite, SetObject *objData) {
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

void DreamGenContext::liftSprite(Sprite *sprite, SetObject *objData) {
	uint8 liftFlag = data.byte(kLiftflag);
	if (liftFlag == 0) { //liftclosed
		turnPathOff(data.byte(kLiftpath));

		if (data.byte(kCounttoopen) != 0) {
			_dec(data.byte(kCounttoopen));
			if (data.byte(kCounttoopen) == 0)
				data.byte(kLiftflag) = 3;
		}
		sprite->animFrame = 0;
		sprite->frameNumber = objData->index = objData->frames[sprite->animFrame];
	}
	else if (liftFlag == 1) {  //liftopen
		turnPathOn(data.byte(kLiftpath));

		if (data.byte(kCounttoclose) != 0) {
			_dec(data.byte(kCounttoclose));
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
			al = 2;
			liftNoise();
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
			al = 3;
			liftNoise();
		}
		sprite->frameNumber = objData->index = objData->frames[sprite->animFrame];
	}
}

void DreamGenContext::faceRightWay() {
	PathNode *paths = getRoomsPaths()->nodes;
	uint8 dir = paths[data.byte(kManspath)].dir;
	data.byte(kTurntoface) = dir;
	data.byte(kLeavedirection) = dir;
}

// Locate the reel segment (reel1, reel2, reel3) this frame is stored in.
// The return value is a pointer to the start of the segment.
// data.word(kCurrentframe) - data.word(kTakeoff) is the number of the frame
// inside that segment
Frame *DreamGenContext::findSource() {
	uint16 currentFrame = data.word(kCurrentframe);
	if (currentFrame < 160) {
		data.word(kTakeoff) = 0;
		return (Frame *)getSegment(data.word(kReel1)).ptr(0, 0);
	} else if (currentFrame < 320) {
		data.word(kTakeoff) = 160;
		return (Frame *)getSegment(data.word(kReel2)).ptr(0, 0);
	} else {
		data.word(kTakeoff) = 320;
		return (Frame *)getSegment(data.word(kReel3)).ptr(0, 0);
	}
}

Reel *DreamGenContext::getReelStart() {
	Reel *reel = (Reel *)getSegment(data.word(kReels)).ptr(kReellist + data.word(kReelpointer) * sizeof(Reel) * 8, sizeof(Reel));
	return reel;
}

void DreamGenContext::showReelFrame(Reel *reel) {
	uint16 x = reel->x + data.word(kMapadx);
	uint16 y = reel->y + data.word(kMapady);
	data.word(kCurrentframe) = reel->frame();
	Frame *source = findSource();
	uint16 frame = data.word(kCurrentframe) - data.word(kTakeoff);
	showFrame(source, x, y, frame, 8);
}

void DreamGenContext::showGameReel() {
	showGameReel((ReelRoutine *)es.ptr(bx, sizeof(ReelRoutine)));
}

void DreamGenContext::showGameReel(ReelRoutine *routine) {
	uint16 reelPointer = routine->reelPointer();
	if (reelPointer >= 512)
		return;
	data.word(kReelpointer) = reelPointer;
	plotReel();
	routine->setReelPointer(data.word(kReelpointer));
}

const Frame *DreamGenContext::getReelFrameAX(uint16 frame) {
	data.word(kCurrentframe) = frame;
	Frame *source = findSource();
	uint16 offset = data.word(kCurrentframe) - data.word(kTakeoff);
	return source + offset;
}

void DreamGenContext::showRain() {
	Rain *rain = (Rain *)getSegment(data.word(kBuffers)).ptr(kRainlist, 0);

	// Do nothing if there's no rain at all
	if (rain->x == 255)
		return;

	ds = data.word(kMainsprites);
	si = 6*58; // Frame 58
	ax = ds.word(si+2); // Frame::ptr
	si = ax + 2080;

	for (; rain->x != 255; ++rain) {
		uint16 y = rain->y + data.word(kMapady) + data.word(kMapystart);
		uint16 x = rain->x + data.word(kMapadx) + data.word(kMapxstart);
		uint16 size = rain->size;
		ax = ((uint16)(rain->w3() - rain->b5)) & 511;
		rain->setW3(ax);
		const uint8 *src = ds.ptr(si, 0) + ax;
		uint8 *dst = workspace() + y * 320 + x;
		for(uint16 i = 0; i < size; ++i) {
			uint8 v = src[i];
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

static void (DreamGenContext::*reelCallbacks[57])() = {
	NULL, NULL,
	NULL, &DreamGenContext::edenInBath,
	NULL, &DreamGenContext::smokeBloke,
	&DreamGenContext::manAsleep, &DreamGenContext::drunk,
	&DreamGenContext::receptionist, NULL,
	NULL, &DreamGenContext::louis,
	&DreamGenContext::louisChair, &DreamGenContext::soldier1,
	&DreamGenContext::bossMan, &DreamGenContext::interviewer,
	&DreamGenContext::heavy, &DreamGenContext::manAsleep2,
	&DreamGenContext::manSatStill, &DreamGenContext::drinker,
	&DreamGenContext::bartender, NULL,
	&DreamGenContext::tattooMan, &DreamGenContext::attendant,
	&DreamGenContext::keeper, &DreamGenContext::candles1,
	&DreamGenContext::smallCandle, &DreamGenContext::security,
	&DreamGenContext::copper, &DreamGenContext::poolGuard,
	&DreamGenContext::rockstar, &DreamGenContext::businessMan,
	&DreamGenContext::train, &DreamGenContext::aide,
	&DreamGenContext::mugger, &DreamGenContext::helicopter,
	&DreamGenContext::introMagic1, &DreamGenContext::introMusic,
	&DreamGenContext::introMagic2, &DreamGenContext::candles2,
	&DreamGenContext::gates, &DreamGenContext::introMagic3,
	&DreamGenContext::introMonks1, &DreamGenContext::candles,
	&DreamGenContext::introMonks2, &DreamGenContext::handClap,
	&DreamGenContext::monkAndRyan, &DreamGenContext::endGameSeq,
	&DreamGenContext::priest, &DreamGenContext::madman,
	&DreamGenContext::madmansTelly, &DreamGenContext::alleyBarkSound,
	&DreamGenContext::foghornSound, &DreamGenContext::carParkDrip,
	&DreamGenContext::carParkDrip, &DreamGenContext::carParkDrip,
	&DreamGenContext::carParkDrip
};

static void (DreamGenContext::*reelCallbacksCPP[57])(ReelRoutine &) = {
	&DreamGenContext::gamer, &DreamGenContext::sparkyDrip,
	&DreamGenContext::eden, /*&DreamGenContext::edenInBath*/NULL,
	&DreamGenContext::sparky, /*&DreamGenContext::smokeBloke*/NULL,
	/*&DreamGenContext::manAsleep*/NULL, /*&DreamGenContext::drunk*/NULL,
	/*&DreamGenContext::receptionist*/NULL, &DreamGenContext::maleFan,
	&DreamGenContext::femaleFan, /*&DreamGenContext::louis*/NULL,
	/*&DreamGenContext::louisChair*/NULL, /*&DreamGenContext::soldier1*/NULL,
	/*&DreamGenContext::bossMan*/NULL, /*&DreamGenContext::interviewer*/NULL,
	/*&DreamGenContext::heavy*/NULL, /*&DreamGenContext::manAsleep2*/NULL,
	/*&DreamGenContext::manSatStill*/NULL, /*&DreamGenContext::drinker*/NULL,
	/*&DreamGenContext::bartender*/NULL, &DreamGenContext::otherSmoker,
	/*&DreamGenContext::tattooMan*/NULL, /*&DreamGenContext::attendant*/NULL,
	/*&DreamGenContext::keeper*/NULL, /*&DreamGenContext::candles1*/NULL,
	/*&DreamGenContext::smallcandle*/NULL, /*&DreamGenContext::security*/NULL,
	/*&DreamGenContext::copper*/NULL, /*&DreamGenContext::poolGuard*/NULL,
	/*&DreamGenContext::rockstar*/NULL, /*&DreamGenContext::businessMan*/NULL,
	/*&DreamGenContext::train*/NULL, /*&DreamGenContext::aide*/NULL,
	/*&DreamGenContext::mugger*/NULL, /*&DreamGenContext::helicopter*/NULL,
	/*&DreamGenContext::introMagic1*/NULL, /*&DreamGenContext::introMusic*/NULL,
	/*&DreamGenContext::introMagic2*/NULL, /*&DreamGenContext::candles2*/NULL,
	/*&DreamGenContext::gates*/NULL, /*&DreamGenContext::introMagic3*/NULL,
	/*&DreamGenContext::intromonks1*/NULL, /*&DreamGenContext::candles*/NULL,
	/*&DreamGenContext::intromonks2*/NULL, /*&DreamGenContext::handClap*/NULL,
	/*&DreamGenContext::monkAndRyan*/NULL, /*&DreamGenContext::endGameSeq*/NULL,
	/*&DreamGenContext::priest*/NULL, /*&DreamGenContext::madman*/NULL,
	/*&DreamGenContext::madmansTelly*/NULL, /*&DreamGenContext::alleyBarkSound*/NULL,
	/*&DreamGenContext::foghornSound*/NULL, /*&DreamGenContext::carParkDrip*/NULL,
	/*&DreamGenContext::carParkDrip*/NULL, /*&DreamGenContext::carParkDrip*/NULL,
	/*&DreamGenContext::carParkDrip*/NULL
};

void DreamGenContext::updatePeople() {
	data.word(kListpos) = kPeoplelist;
	memset(getSegment(data.word(kBuffers)).ptr(kPeoplelist, 12 * sizeof(People)), 0xff, 12 * sizeof(People));
	++data.word(kMaintimer);

	// The original callbacks are called with es:bx pointing to their reelRoutine entry.
	// The new callbacks take a mutable ReelRoutine parameter.

	es = cs;
	ReelRoutine *r = (ReelRoutine *)cs.ptr(kReelroutines, 0);

	for (int i = 0; r[i].reallocation != 255; ++i) {
		bx = kReelroutines + 8*i;
		if (r[i].reallocation == data.byte(kReallocation) &&
		        r[i].mapX == data.byte(kMapx) &&
		        r[i].mapY == data.byte(kMapy)) {
			if (reelCallbacks[i]) {
				assert(!reelCallbacksCPP[i]);
				(this->*(reelCallbacks[i]))();
			} else {
				assert(reelCallbacksCPP[i]);
				(this->*(reelCallbacksCPP[i]))(r[i]);
			}
		}
	}
}

void DreamGenContext::madmanText() {
	if (isCD()) {
		if (data.byte(kSpeechcount) >= 63)
			return;
		if (data.byte(kCh1playing) != 255)
			return;
		al = data.byte(kSpeechcount);
		++data.byte(kSpeechcount);
	} else {
		if (data.byte(kCombatcount) >= 61)
			return;
		al = data.byte(kCombatcount);
		_and(al, 3);
		if (!flags.z())
			return;
		al = data.byte(kCombatcount) / 4;
	}
	setupTimedTemp(47 + al, 82, 72, 80, 90, 1);
}

void DreamGenContext::madman() {
	ReelRoutine *routine = (ReelRoutine *)es.ptr(bx, 0);
	data.word(kWatchingtime) = 2;
	if (checkSpeed(routine)) {
		ax = routine->reelPointer();
		if (ax >= 364) {
			data.byte(kMandead) = 2;
			showGameReel(routine);
			return;
		}
		if (ax == 10) {
			loadTempText("DREAMWEB.T82");
			data.byte(kCombatcount) = (uint8)-1;
			data.byte(kSpeechcount) = 0;
		}
		++ax;
		if (ax == 294) {
			if (data.byte(kWongame) == 1)
				return;
			data.byte(kWongame) = 1;
			push(es);
			push(bx);
			getRidOfTempText();
			bx = pop();
			es = pop();
			return;
		}
		if (ax == 66) {
			++data.byte(kCombatcount);
			push(es);
			push(bx);
			madmanText();
			bx = pop();
			es = pop();
			ax = 53;
			if (data.byte(kCombatcount) >= (isCD() ? 64 : 62)) {
				if (data.byte(kCombatcount) == (isCD() ? 70 : 68))
					ax = 310;
				else {
					if (data.byte(kLastweapon) == 8) {
						data.byte(kCombatcount) = isCD() ? 72 : 70;
						data.byte(kLastweapon) = (uint8)-1;
						data.byte(kMadmanflag) = 1;
						ax = 67;
					}
				}
			}
		}
		routine->setReelPointer(ax);
	}
	showGameReel(routine);
	routine->mapX = data.byte(kMapx);
	madMode();
}

void DreamGenContext::madMode() {
	data.word(kWatchingtime) = 2;
	data.byte(kPointermode) = 0;
	if (data.byte(kCombatcount) < (isCD() ? 65 : 63))
		return;
	if (data.byte(kCombatcount) >= (isCD() ? 70 : 68))
		return;
	data.byte(kPointermode) = 2;
}

void DreamGenContext::moveMap(uint8 param) {
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

void DreamGenContext::checkOne(uint8 x, uint8 y, uint8 *flag, uint8 *flagEx, uint8 *type, uint8 *flagX, uint8 *flagY) {
	*flagX = x / 16;
	*flagY = y / 16;
	const uint8 *tileData = getSegment(data.word(kBuffers)).ptr(kMapflags + (*flagY * 11 + *flagX) * 3, 3);
	*flag = tileData[0];
	*flagEx = tileData[1];
	*type = tileData[2];
}

void DreamGenContext::getBlockOfPixel() {
	al = getBlockOfPixel(cl, ch);
}

uint8 DreamGenContext::getBlockOfPixel(uint8 x, uint8 y) {
	uint8 flag, flagEx, type, flagX, flagY;
	checkOne(x + data.word(kMapxstart), y + data.word(kMapystart), &flag, &flagEx, &type, &flagX, &flagY);
	if (flag & 1)
		return 0;
	else
		return type;
}

void DreamGenContext::addToPeopleList() {
	addToPeopleList((ReelRoutine *)es.ptr(bx, sizeof(ReelRoutine)));
}

void DreamGenContext::addToPeopleList(ReelRoutine *routine) {
	uint16 routinePointer = (const uint8 *)routine - cs.ptr(0, 0);

	People *people = (People *)getSegment(data.word(kBuffers)).ptr(data.word(kListpos), sizeof(People));
	people->setReelPointer(routine->reelPointer());
	people->setRoutinePointer(routinePointer);
	people->b4 = routine->b7;
	data.word(kListpos) += sizeof(People);
}

Rain *DreamGenContext::splitIntoLines(uint8 x, uint8 y, Rain *rain) {
	do {
		// Look for line start
		while (!getBlockOfPixel(x, y)) {
			--x;
			++y;
			if (x == 0 || y >= data.byte(kMapysize))
				return rain;
		}

		rain->x = x;
		rain->y = y;

		uint8 length = 1;

		// Look for line end
		while (getBlockOfPixel(x, y)) {
			--x;
			++y;
			if (x == 0 || y >= data.byte(kMapysize))
				break;
			++length;
		}

		rain->size = length;
		rain->w3_lo = engine->randomNumber();
		rain->w3_hi = engine->randomNumber();
		rain->b5 = (engine->randomNumber() & 3) + 4;
		++rain;
	} while (x > 0 && y < data.byte(kMapysize));

	return rain;
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

void DreamGenContext::initRain() {
	const RainLocation *r = rainLocationList;
	Rain *rainList = (Rain *)getSegment(data.word(kBuffers)).ptr(kRainlist, 0);
	Rain *rain = rainList;

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
		rain->x = 0xff;
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

		rain = splitIntoLines(x, 0, rain);
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

		rain = splitIntoLines(data.byte(kMapxsize) - 1, y, rain);
	} while (true);

	rain->x = 0xff;
}

void DreamGenContext::textForEnd() {
	if (data.byte(kIntrocount) == 20)
		al = 0;
	else if (data.byte(kIntrocount) == (isCD() ? 50 : 65))
		al = 1;
	else if (data.byte(kIntrocount) == (isCD() ? 85 : 110))
		al = 2;
	else
		return;

	bl = 34;
	bh = 20;
	cx = 60;
	dx = 1;
	ah = 83;
	setupTimedTemp();
}

void DreamGenContext::textForMonk() {
	if (data.byte(kIntrocount) == 1) {
		al = 19;
		bl = 68;
		bh = 154;
		cx = 120;
	} else if (data.byte(kIntrocount) == 5) {
		al = 20;
		bl = 68;
		bh = 38;
		cx = 120;
	} else if (data.byte(kIntrocount) == 9) {
		al = 21;
		bl = 48;
		bh = 154;
		cx = 120;
	} else if (data.byte(kIntrocount) == 13) {
		al = 22;
		bl = 68;
		bh = 38;
		cx = 120;
	} else if (data.byte(kIntrocount) == (isCD() ? 15 : 17)) {
		al = 23;
		bl = 68;
		bh = 154;
		cx = 120;
	} else if (data.byte(kIntrocount) == 21) {
		al = 24;
		bl = 68;
		bh = 38;
		cx = 120;
	} else if (data.byte(kIntrocount) == 25) {
		al = 25;
		bl = 68;
		bh = 154;
		cx = 120;
	} else if (data.byte(kIntrocount) == 29) {
		al = 26;
		bl = 68;
		bh = 38;
		cx = 120;
	} else if (data.byte(kIntrocount) == 33) {
		al = 27;
		bl = 68;
		bh = 154;
		cx = 120;
	} else if (data.byte(kIntrocount) == 37) {
		al = 28;
		bl = 68;
		bh = 154;
		cx = 120;
	} else if (data.byte(kIntrocount) == 41) {
		al = 29;
		bl = 68;
		bh = 38;
		cx = 120;
	} else if (data.byte(kIntrocount) == 45) {
		al = 30;
		bl = 68;
		bh = 154;
		cx = 120;
	} else if (data.byte(kIntrocount) == (isCD() ? 52 : 49)) {
		al = 31;
		bl = 68;
		bh = 154;
		cx = 220;
	} else if (data.byte(kIntrocount) == 53) {
		fadeScreenDowns();
		if (isCD()) {
			data.byte(kVolumeto) = 7;
			data.byte(kVolumedirection) = 1;
		}
		return;
	} else {
		return;	
	}

	dx = 1;
	ah = 82;
	if (isCD() && data.byte(kCh1playing) != 255) {
		data.byte(kIntrocount)--;
		return;
	}

	setupTimedTemp();
}

void DreamGenContext::reelsOnScreen() {
	reconstruct();
	updatePeople();
	watchReel();
	showRain();
	useTimedText();
}

void DreamGenContext::reconstruct() {
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

void DreamGenContext::checkSpeed() {
	ReelRoutine *routine = (ReelRoutine *)es.ptr(bx, sizeof(ReelRoutine));
	flags._z = checkSpeed(routine);
}

bool DreamGenContext::checkSpeed(ReelRoutine *routine) {
	if (data.byte(kLastweapon) != (uint8)-1)
		return true;
	++routine->counter;
	if (routine->counter != routine->period)
		return false;
	routine->counter = 0;
	return true;
}

void DreamGenContext::sparkyDrip(ReelRoutine &routine) {
	if (checkSpeed(&routine))
		playChannel0(14, 0);
}

void DreamGenContext::otherSmoker(ReelRoutine &routine) {
	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamGenContext::gamer(ReelRoutine &routine) {
	if (checkSpeed(&routine)) {
		uint8 v;
		do {
			v = 20 + engine->randomNumber() % 5;
		} while (v == routine.reelPointer());
		routine.setReelPointer(v);
	}

	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamGenContext::eden(ReelRoutine &routine) {
	if (data.byte(kGeneraldead))
		return;
	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamGenContext::femaleFan(ReelRoutine &routine) {
	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamGenContext::maleFan(ReelRoutine &routine) {
	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamGenContext::sparky(ReelRoutine &routine) {
	if (data.word(kCard1money))
		routine.b7 = 3;
	if (checkSpeed(&routine)) {
		if (routine.reelPointer() != 34) {
			if (engine->randomNumber() < 30)
				routine.incReelPointer();
			else
				routine.setReelPointer(27);
		} else {
			if (routine.reelPointer() != 48)
				routine.incReelPointer();
			else
				routine.setReelPointer(27);
		}
	}
	showGameReel(&routine);
	addToPeopleList(&routine);
	if (routine.b7 & 128)
		data.byte(kTalkedtosparky) = 1;
}

} /*namespace dreamgen */

