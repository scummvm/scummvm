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

Sprite *DreamGenContext::spritetable() {
	Sprite *sprite = (Sprite *)segRef(data.word(kBuffers)).ptr(kSpritetable, 16 * sizeof(Sprite));
	return sprite;
}

void DreamGenContext::printsprites() {
	for (size_t priority = 0; priority < 7; ++priority) {
		Sprite *sprites = spritetable();
		for (size_t j = 0; j < 16; ++j) {
			const Sprite &sprite = sprites[j];
			if (sprite.updateCallback() == 0x0ffff)
				continue;
			if (priority != sprite.priority)
				continue;
			if (sprite.hidden == 1)
				continue;
			printasprite(&sprite);
		}
	}
}

void DreamGenContext::printasprite(const Sprite *sprite) {
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
	showframe((const Frame *)segRef(sprite->frameData()).ptr(0, 0), x, y, sprite->frameNumber, c);
}

void DreamGenContext::clearsprites() {
	memset(spritetable(), 0xff, sizeof(Sprite) * 16);
}

Sprite *DreamGenContext::makesprite(uint8 x, uint8 y, uint16 updateCallback, uint16 frameData, uint16 somethingInDi) {
	Sprite *sprite = spritetable();
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

void DreamGenContext::spriteupdate() {
	Sprite *sprites = spritetable();
	sprites[0].hidden = data.byte(kRyanon);

	Sprite *sprite = sprites;
	for (size_t i=0; i < 16; ++i) {
		uint16 updateCallback = sprite->updateCallback();
		if (updateCallback != 0xffff) {
			sprite->w24 = sprite->w2;
			if (updateCallback == addr_mainman) // NB : Let's consider the callback as an enum while more code is not ported to C++
				mainman(sprite);
			else {
				assert(updateCallback == addr_backobject);
				backobject(sprite);
			}
		}
	
		if (data.byte(kNowinnewroom) == 1)
			break;
		++sprite;
	}
}

void DreamGenContext::initman() {
	Sprite *sprite = makesprite(data.byte(kRyanx), data.byte(kRyany), addr_mainman, data.word(kMainsprites), 0);
	sprite->priority = 4;
	sprite->speed = 0;
	sprite->walkFrame = 0;
}

void DreamGenContext::mainman() {
	assert(false);
}

void DreamGenContext::mainman(Sprite *sprite) {
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
		aboutturn(sprite);
	} else {
		if ((data.byte(kTurndirection) != 0) && (data.byte(kLinepointer) == 254)) {
			data.byte(kReasseschanges) = 1;
			if (data.byte(kFacing) == data.byte(kLeavedirection))
				checkforexit();
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
						checkforexit();
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
		facerightway();
		return;
	}
	data.byte(kDestination) = data.byte(kFinaldest);
	push(es);
	push(bx);
	autosetwalk();
	bx = pop();
	es = pop();
}

void DreamGenContext::aboutturn(Sprite *sprite) {
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

void DreamGenContext::backobject() {
	assert(false);
}

void DreamGenContext::backobject(Sprite *sprite) {
	SetObject *objData = (SetObject *)segRef(data.word(kSetdat)).ptr(sprite->objData(), 0);

	if (sprite->delay != 0) {
		--sprite->delay;
		return;
	}

	sprite->delay = objData->delay;
	if (objData->type == 6)
		widedoor(sprite, objData);
	else if (objData->type == 5)
		random(sprite, objData);
	else if (objData->type == 4)
		lockeddoorway(sprite, objData);
	else if (objData->type == 3)
		liftsprite(sprite, objData);
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
	randomnum1();
	uint16 r = ax;
	sprite->frameNumber = objData->frames[r&7];
}

void DreamGenContext::doorway(Sprite *sprite, SetObject *objData) {
	Common::Rect check(-24, -30, 10, 10);
	dodoor(sprite, objData, check);
}

void DreamGenContext::widedoor(Sprite *sprite, SetObject *objData) {
	Common::Rect check(-24, -30, 24, 24);
	dodoor(sprite, objData, check);
}

void DreamGenContext::dodoor(Sprite *sprite, SetObject *objData, Common::Rect check) {

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
			playchannel1(soundIndex);
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
			playchannel1(soundIndex);
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

void DreamGenContext::lockeddoorway(Sprite *sprite, SetObject *objData) {

	int ryanx = data.byte(kRyanx);
	int ryany = data.byte(kRyany);

	Common::Rect check(-24, -30, 10, 12);
	check.translate(sprite->x, sprite->y);
	bool openDoor = check.contains(ryanx, ryany);

	if (data.byte(kThroughdoor) != 1 && data.byte(kLockstatus) == 1)
		openDoor = false;

	if (openDoor) {

		if (sprite->animFrame == 1) {
			playchannel1(0);
		}

		if (sprite->animFrame == 6)
			turnpathon(data.byte(kDoorpath));

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
			playchannel1(1);
		}

		if (sprite->animFrame != 0)
			--sprite->animFrame;
	
		data.byte(kThroughdoor) = 0;
		sprite->frameNumber = objData->index = objData->frames[sprite->animFrame];

		if (sprite->animFrame == 0) {
			turnpathoff(data.byte(kDoorpath));
			data.byte(kLockstatus) = 1;
		}
	}
}

void DreamGenContext::liftsprite(Sprite *sprite, SetObject *objData) {
	uint8 liftFlag = data.byte(kLiftflag);
	if (liftFlag == 0) { //liftclosed
		turnpathoff(data.byte(kLiftpath));

		if (data.byte(kCounttoopen) != 0) {
			_dec(data.byte(kCounttoopen));
			if (data.byte(kCounttoopen) == 0)
				data.byte(kLiftflag) = 3;
		}
		sprite->animFrame = 0;
		sprite->frameNumber = objData->index = objData->frames[sprite->animFrame];
	}
	else if (liftFlag == 1) {  //liftopen
		turnpathon(data.byte(kLiftpath));

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
			liftnoise();
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
			liftnoise();
		}
		sprite->frameNumber = objData->index = objData->frames[sprite->animFrame];
	}
}

void DreamGenContext::facerightway() {
	PathNode *paths = getroomspaths()->nodes;
	uint8 dir = paths[data.byte(kManspath)].dir;
	data.byte(kTurntoface) = dir;
	data.byte(kLeavedirection) = dir;
}

// Locate the reel segment (reel1, reel2, reel3) this frame is stored in.
// The return value is a pointer to the start of the segment.
// data.word(kCurrentframe) - data.word(kTakeoff) is the number of the frame
// inside that segment
Frame *DreamGenContext::findsource() {
	uint16 currentFrame = data.word(kCurrentframe);
	if (currentFrame < 160) {
		data.word(kTakeoff) = 0;
		return (Frame *)segRef(data.word(kReel1)).ptr(0, 0);
	} else if (currentFrame < 320) {
		data.word(kTakeoff) = 160;
		return (Frame *)segRef(data.word(kReel2)).ptr(0, 0);
	} else {
		data.word(kTakeoff) = 320;
		return (Frame *)segRef(data.word(kReel3)).ptr(0, 0);
	}
}

Reel *DreamGenContext::getreelstart() {
	Reel *reel = (Reel *)segRef(data.word(kReels)).ptr(kReellist + data.word(kReelpointer) * sizeof(Reel) * 8, sizeof(Reel));
	return reel;
}

void DreamGenContext::showreelframe(Reel *reel) {
	uint16 x = reel->x + data.word(kMapadx);
	uint16 y = reel->y + data.word(kMapady);
	data.word(kCurrentframe) = reel->frame();
	Frame *source = findsource();
	uint16 frame = data.word(kCurrentframe) - data.word(kTakeoff);
	showframe(source, x, y, frame, 8);
}

void DreamGenContext::showgamereel() {
	showgamereel((ReelRoutine *)es.ptr(bx, sizeof(ReelRoutine)));
}

void DreamGenContext::showgamereel(ReelRoutine *routine) {
	uint16 reelpointer = routine->reelPointer();
	if (reelpointer >= 512)
		return;
	data.word(kReelpointer) = reelpointer;
	plotreel();
	routine->setReelPointer(data.word(kReelpointer));
}

const Frame *DreamGenContext::getreelframeax(uint16 frame) {
	data.word(kCurrentframe) = frame;
	Frame *source = findsource();
	uint16 offset = data.word(kCurrentframe) - data.word(kTakeoff);
	return source + offset;
}

void DreamGenContext::showrain() {
	Rain *rain = (Rain *)segRef(data.word(kBuffers)).ptr(kRainlist, 0);

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
	playchannel1(soundIndex);
}

static void (DreamGenContext::*reelCallbacks[57])() = {
	NULL, NULL,
	NULL, &DreamGenContext::edeninbath,
	&DreamGenContext::sparky, &DreamGenContext::smokebloke,
	&DreamGenContext::manasleep, &DreamGenContext::drunk,
	&DreamGenContext::receptionist, &DreamGenContext::malefan,
	&DreamGenContext::femalefan, &DreamGenContext::louis,
	&DreamGenContext::louischair, &DreamGenContext::soldier1,
	&DreamGenContext::bossman, &DreamGenContext::interviewer,
	&DreamGenContext::heavy, &DreamGenContext::manasleep2,
	&DreamGenContext::mansatstill, &DreamGenContext::drinker,
	&DreamGenContext::bartender, NULL,
	&DreamGenContext::tattooman, &DreamGenContext::attendant,
	&DreamGenContext::keeper, &DreamGenContext::candles1,
	&DreamGenContext::smallcandle, &DreamGenContext::security,
	&DreamGenContext::copper, &DreamGenContext::poolguard,
	&DreamGenContext::rockstar, &DreamGenContext::businessman,
	&DreamGenContext::train, &DreamGenContext::aide,
	&DreamGenContext::mugger, &DreamGenContext::helicopter,
	&DreamGenContext::intromagic1, &DreamGenContext::intromusic,
	&DreamGenContext::intromagic2, &DreamGenContext::candles2,
	&DreamGenContext::gates, &DreamGenContext::intromagic3,
	&DreamGenContext::intromonks1, &DreamGenContext::candles,
	&DreamGenContext::intromonks2, &DreamGenContext::handclap,
	&DreamGenContext::monkandryan, &DreamGenContext::endgameseq,
	&DreamGenContext::priest, &DreamGenContext::madman,
	&DreamGenContext::madmanstelly, &DreamGenContext::alleybarksound,
	&DreamGenContext::foghornsound, &DreamGenContext::carparkdrip,
	&DreamGenContext::carparkdrip, &DreamGenContext::carparkdrip,
	&DreamGenContext::carparkdrip
};

static void (DreamGenContext::*reelCallbacksCPP[57])(ReelRoutine &) = {
	&DreamGenContext::gamer, &DreamGenContext::sparkydrip,
	&DreamGenContext::eden, /*&DreamGenContext::edeninbath*/NULL,
	/*&DreamGenContext::sparky*/NULL, /*&DreamGenContext::smokebloke*/NULL,
	/*&DreamGenContext::manasleep*/NULL, /*&DreamGenContext::drunk*/NULL,
	/*&DreamGenContext::receptionist*/NULL, /*&DreamGenContext::malefan*/NULL,
	/*&DreamGenContext::femalefan*/NULL, /*&DreamGenContext::louis*/NULL,
	/*&DreamGenContext::louischair*/NULL, /*&DreamGenContext::soldier1*/NULL,
	/*&DreamGenContext::bossman*/NULL, /*&DreamGenContext::interviewer*/NULL,
	/*&DreamGenContext::heavy*/NULL, /*&DreamGenContext::manasleep2*/NULL,
	/*&DreamGenContext::mansatstill*/NULL, /*&DreamGenContext::drinker*/NULL,
	/*&DreamGenContext::bartender*/NULL, &DreamGenContext::othersmoker,
	/*&DreamGenContext::tattooman*/NULL, /*&DreamGenContext::attendant*/NULL,
	/*&DreamGenContext::keeper*/NULL, /*&DreamGenContext::candles1*/NULL,
	/*&DreamGenContext::smallcandle*/NULL, /*&DreamGenContext::security*/NULL,
	/*&DreamGenContext::copper*/NULL, /*&DreamGenContext::poolguard*/NULL,
	/*&DreamGenContext::rockstar*/NULL, /*&DreamGenContext::businessman*/NULL,
	/*&DreamGenContext::train*/NULL, /*&DreamGenContext::aide*/NULL,
	/*&DreamGenContext::mugger*/NULL, /*&DreamGenContext::helicopter*/NULL,
	/*&DreamGenContext::intromagic1*/NULL, /*&DreamGenContext::intromusic*/NULL,
	/*&DreamGenContext::intromagic2*/NULL, /*&DreamGenContext::candles2*/NULL,
	/*&DreamGenContext::gates*/NULL, /*&DreamGenContext::intromagic3*/NULL,
	/*&DreamGenContext::intromonks1*/NULL, /*&DreamGenContext::candles*/NULL,
	/*&DreamGenContext::intromonks2*/NULL, /*&DreamGenContext::handclap*/NULL,
	/*&DreamGenContext::monkandryan*/NULL, /*&DreamGenContext::endgameseq*/NULL,
	/*&DreamGenContext::priest*/NULL, /*&DreamGenContext::madman*/NULL,
	/*&DreamGenContext::madmanstelly*/NULL, /*&DreamGenContext::alleybarksound*/NULL,
	/*&DreamGenContext::foghornsound*/NULL, /*&DreamGenContext::carparkdrip*/NULL,
	/*&DreamGenContext::carparkdrip*/NULL, /*&DreamGenContext::carparkdrip*/NULL,
	/*&DreamGenContext::carparkdrip*/NULL
};

void DreamGenContext::updatepeople() {
	data.word(kListpos) = kPeoplelist;
	memset(segRef(data.word(kBuffers)).ptr(kPeoplelist, 12 * sizeof(People)), 0xff, 12 * sizeof(People));
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

void DreamGenContext::madmantext() {
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
	setuptimedtemp(47 + al, 82, 72, 80, 90, 1);
}

void DreamGenContext::madman() {
	ReelRoutine *routine = (ReelRoutine *)es.ptr(bx, 0);
	data.word(kWatchingtime) = 2;
	if (checkspeed(routine)) {
		ax = routine->reelPointer();
		if (ax >= 364) {
			data.byte(kMandead) = 2;
			showgamereel(routine);
			return;
		}
		if (ax == 10) {
			loadtemptext("DREAMWEB.T82");
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
			getridoftemptext();
			bx = pop();
			es = pop();
			return;
		}
		if (ax == 66) {
			++data.byte(kCombatcount);
			push(es);
			push(bx);
			madmantext();
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
	showgamereel(routine);
	routine->mapX = data.byte(kMapx);
	madmode();
}

void DreamGenContext::madmode() {
	data.word(kWatchingtime) = 2;
	data.byte(kPointermode) = 0;
	if (data.byte(kCombatcount) < (isCD() ? 65 : 63))
		return;
	if (data.byte(kCombatcount) >= (isCD() ? 70 : 68))
		return;
	data.byte(kPointermode) = 2;
}

void DreamGenContext::movemap(uint8 param) {
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

void DreamGenContext::checkone() {
	uint8 flag, flagEx, type, flagX, flagY;
	checkone(cl, ch, &flag, &flagEx, &type, &flagX, &flagY);

	cl = flag;
	ch = flagEx;
	dl = flagX;
	dh = flagY;
	al = type;
}

void DreamGenContext::checkone(uint8 x, uint8 y, uint8 *flag, uint8 *flagEx, uint8 *type, uint8 *flagX, uint8 *flagY) {
	*flagX = x / 16;
	*flagY = y / 16;
	const uint8 *tileData = segRef(data.word(kBuffers)).ptr(kMapflags + (*flagY * 11 + *flagX) * 3, 3);
	*flag = tileData[0];
	*flagEx = tileData[1];
	*type = tileData[2];
}

void DreamGenContext::getblockofpixel() {
	al = getblockofpixel(cl, ch);
}

uint8 DreamGenContext::getblockofpixel(uint8 x, uint8 y) {
	uint8 flag, flagEx, type, flagX, flagY;
	checkone(x + data.word(kMapxstart), y + data.word(kMapystart), &flag, &flagEx, &type, &flagX, &flagY);
	if (flag & 1)
		return 0;
	else
		return type;
}

void DreamGenContext::addtopeoplelist() {
	addtopeoplelist((ReelRoutine *)es.ptr(bx, sizeof(ReelRoutine)));
}

void DreamGenContext::addtopeoplelist(ReelRoutine *routine) {
	uint16 routinePointer = (const uint8 *)routine - cs.ptr(0, 0);

	People *people = (People *)segRef(data.word(kBuffers)).ptr(data.word(kListpos), sizeof(People));
	people->setReelPointer(routine->reelPointer());
	people->setRoutinePointer(routinePointer);
	people->b4 = routine->b7;
	data.word(kListpos) += sizeof(People);
}

Rain *DreamGenContext::splitintolines(uint8 x, uint8 y, Rain *rain) {
	do {
		// Look for line start
		while (!getblockofpixel(x, y)) {
			--x;
			++y;
			if (x == 0 || y >= data.byte(kMapysize))
				return rain;
		}

		rain->x = x;
		rain->y = y;

		uint8 length = 1;

		// Look for line end
		while (getblockofpixel(x, y)) {
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

void DreamGenContext::initrain() {
	const RainLocation *r = rainLocationList;
	Rain *rainList = (Rain *)segRef(data.word(kBuffers)).ptr(kRainlist, 0);
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

		rain = splitintolines(x, 0, rain);
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

		rain = splitintolines(data.byte(kMapxsize) - 1, y, rain);
	} while (true);

	rain->x = 0xff;
}

void DreamGenContext::textforend() {
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
	setuptimedtemp();
}

void DreamGenContext::textformonk() {
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
		fadescreendowns();
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

	setuptimedtemp();
}

void DreamGenContext::reelsonscreen() {
	reconstruct();
	updatepeople();
	watchreel();
	showrain();
	usetimedtext();
}

void DreamGenContext::reconstruct() {
	if (data.byte(kHavedoneobs) == 0)
		return;
	data.byte(kNewobs) = 1;
	drawfloor();
	spriteupdate();
	printsprites();
	if ((data.byte(kForeignrelease) != 0) && (data.byte(kReallocation) == 20))
		undertextline();
	data.byte(kHavedoneobs) = 0;
}

void DreamGenContext::checkspeed() {
	ReelRoutine *routine = (ReelRoutine *)es.ptr(bx, sizeof(ReelRoutine));
	flags._z = checkspeed(routine);
}

bool DreamGenContext::checkspeed(ReelRoutine *routine) {
	if (data.byte(kLastweapon) != (uint8)-1)
		return true;
	++routine->counter;
	if (routine->counter != routine->period)
		return false;
	routine->counter = 0;
	return true;
}

void DreamGenContext::sparkydrip(ReelRoutine &routine) {
	if (checkspeed(&routine))
		playchannel0(14, 0);
}

void DreamGenContext::othersmoker(ReelRoutine &routine) {
	showgamereel(&routine);
	addtopeoplelist(&routine);
}

void DreamGenContext::gamer(ReelRoutine &routine) {
	if (checkspeed(&routine)) {
		uint8 v;
		do {
			v = 20 + engine->randomNumber() % 5;
		} while (v == routine.reelPointer());
		routine.setReelPointer(v);
	}

	showgamereel(&routine);
	addtopeoplelist(&routine);
}

void DreamGenContext::eden(ReelRoutine &routine) {
	if (data.byte(kGeneraldead))
		return;
	showgamereel(&routine);
	addtopeoplelist(&routine);
}

} /*namespace dreamgen */

