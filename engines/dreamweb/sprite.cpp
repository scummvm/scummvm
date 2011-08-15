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
#include "engines/util.h"
#include "graphics/surface.h"

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
	if (sprite->b29 != 0)
		c = 8;
	else
		c = 0;
	uint8 width, height;
	showframe((const Frame *)segRef(sprite->frameData()).ptr(0, 0), x, y, sprite->b15, c, &width, &height);
}

void DreamGenContext::clearsprites() {
	memset(spritetable(), 0xff, sizeof(Sprite) * 16);
}

Sprite *DreamGenContext::makesprite(uint8 x, uint8 y, uint16 updateCallback, uint16 frameData, uint16 somethingInDi) {
	Sprite *sprite = spritetable();
	while (sprite->b15 != 0xff) { // NB: No boundchecking in the original code either
		++sprite;
	}

	sprite->setUpdateCallback(updateCallback);
	sprite->x = x;
	sprite->y = y;
	sprite->setFrameData(frameData);
	WRITE_LE_UINT16(&sprite->w8, somethingInDi);
	sprite->w2 = 0xffff;
	sprite->b15 = 0;
	sprite->delay = 0;
	return sprite;
}

void DreamGenContext::makesprite() { // NB: returns new sprite in es:bx 
	Sprite *sprite = makesprite(si & 0xff, si >> 8, cx, dx, di);

	// Recover es:bx from sprite
	es = data.word(kBuffers);
	bx = kSpritetable;
	Sprite *sprites = (Sprite *)es.ptr(bx, sizeof(Sprite) * 16);
	bx += sizeof(Sprite) * (sprite - sprites);
	//
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
	sprite->b22 = 0;
	sprite->b29 = 0;
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
		sprite->b29 = 0;
	}
	--sprite->b22;
	if (sprite->b22 != 0xff) {
		ds = pop();
		es = pop();
		return;
	}
	sprite->b22 = 0;
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
			sprite->b29 = 0;
		} else {
			++sprite->b29;
			if (sprite->b29 == 11)
				sprite->b29 = 1;
			walking(sprite);
			if (data.byte(kLinepointer) != 254) {
				if ((data.byte(kFacing) & 1) == 0)
					walking(sprite);
				else if ((sprite->b29 != 2) && (sprite->b29 != 7))
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
	sprite->b15 = sprite->b29 + facelist[data.byte(kFacing)];
	data.byte(kRyanx) = sprite->x;
	data.byte(kRyany) = sprite->y;

	ds = pop();
	es = pop();
}

void DreamGenContext::walking() {
	Sprite *sprite = (Sprite *)es.ptr(bx, sizeof(Sprite));
	walking(sprite);
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
		sprite->b29 = 0;
	} else {
		data.byte(kTurndirection) = -1;
		data.byte(kFacing) = (data.byte(kFacing) - 1) & 7;
		sprite->b29 = 0;
	}
}

void DreamGenContext::backobject() {
	assert(false);
}

void DreamGenContext::backobject(Sprite *sprite) {
	ObjData *objData = (ObjData *)segRef(data.word(kSetdat)).ptr(sprite->objData(), 0);

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

void DreamGenContext::constant(Sprite *sprite, ObjData *objData) {
	++sprite->frame;
	if (objData->b18[sprite->frame] == 255) {
		sprite->frame = 0;
	}
	uint8 b18 = objData->b18[sprite->frame];
	objData->b17 = b18;
	sprite->b15 = b18;
}

void DreamGenContext::random(Sprite *sprite, ObjData *objData) {
	randomnum1();
	uint16 r = ax;
	sprite->b15 = objData->b18[r&7];
}

void DreamGenContext::doorway(Sprite *sprite, ObjData *objData) {
	data.byte(kDoorcheck1) = -24;
	data.byte(kDoorcheck2) = 10;
	data.byte(kDoorcheck3) = -30;
	data.byte(kDoorcheck4) = 10;
	dodoor(sprite, objData);
}

void DreamGenContext::widedoor(Sprite *sprite, ObjData *objData) {
	data.byte(kDoorcheck1) = -24;
	data.byte(kDoorcheck2) = 24;
	data.byte(kDoorcheck3) = -30;
	data.byte(kDoorcheck4) = 24;
	dodoor(sprite, objData);
}

void DreamGenContext::dodoor() {
	Sprite *sprite = (Sprite *)es.ptr(bx, sizeof(Sprite));
	ObjData *objData = (ObjData *)ds.ptr(di, 0);
	dodoor(sprite, objData);
}

void DreamGenContext::dodoor(Sprite *sprite, ObjData *objData) {
	uint8 ryanx = data.byte(kRyanx);
	uint8 ryany = data.byte(kRyany);
	int8 deltax = ryanx - sprite->x;
	int8 deltay = ryany - sprite->y;
	if (ryanx < sprite->x) {
		if (deltax < (int8)data.byte(kDoorcheck1))
			goto shutdoor;
	} else {
		if (deltax >= data.byte(kDoorcheck2))
			goto shutdoor;
	}
	if (ryany < sprite->y) {
		if (deltay < (int8)data.byte(kDoorcheck3))
			goto shutdoor;
	} else {
		if (deltay >= data.byte(kDoorcheck4))
			goto shutdoor;
	}
//opendoor:
	if ((data.byte(kThroughdoor) == 1) && (sprite->frame == 0))
		sprite->frame = 6;

	++sprite->frame;
	if (sprite->frame == 1) { //doorsound2
		if (data.byte(kReallocation) == 5) //hoteldoor2
			al = 13;
		else
			al = 0;
		playchannel1();
	}
	if (objData->b18[sprite->frame] == 255) {
		--sprite->frame;
	}
	sprite->b15 = objData->b17 = objData->b18[sprite->frame];
	data.byte(kThroughdoor) = 1;
	return;
shutdoor:
	if (sprite->frame == 5) { //doorsound1;
		if (data.byte(kReallocation) == 5) //hoteldoor1
			al = 13;
		else
			al = 1;
		playchannel1();
	}
	if (sprite->frame != 0) {
		--sprite->frame;
	}
	sprite->b15 = objData->b17 = objData->b18[sprite->frame];
	if (sprite->frame == 5) //nearly
		data.byte(kThroughdoor) = 0;
}

void DreamGenContext::steady(Sprite *sprite, ObjData *objData) {
	uint8 b18 = objData->b18[0];
	objData->b17 = b18;
	sprite->b15 = b18;
}

void DreamGenContext::lockeddoorway(Sprite *sprite, ObjData *objData) {
	if (data.byte(kRyanx) < sprite->x) {
		if (sprite->x - data.byte(kRyanx) > 24)
			goto shutdoor2;
	} else {
		if (data.byte(kRyanx) - sprite->x >= 10)
			goto shutdoor2;
	}

	if (data.byte(kRyany) < sprite->y) {
		if (sprite->y - data.byte(kRyany) > 30)
			goto shutdoor2;
	} else {
		if (data.byte(kRyany) - sprite->y >= 12)
			goto shutdoor2;
	}

	if (data.byte(kThroughdoor) != 1) {
		if (data.byte(kLockstatus) == 1)
			goto shutdoor2;
	}

	if (sprite->frame == 1) {
		al = 0;
		playchannel1();
	}

	if (sprite->frame == 6) {
		turnpathonCPP(data.byte(kDoorpath));
	}

	if ((data.byte(kThroughdoor) == 1) && (sprite->frame == 0)) {
		sprite->frame = 6;
	}

	++sprite->frame;
	if (objData->b18[sprite->frame] == 255) {
		--sprite->frame;
	}

	sprite->b15 = objData->b17 = objData->b18[sprite->frame];
	if (sprite->frame == 5)
		data.byte(kThroughdoor) = 1;
	return;

shutdoor2:
	if (sprite->frame == 5) {
		al = 1;
		playchannel1();
	}

	if (sprite->frame != 0) {
		--sprite->frame;
	}

	data.byte(kThroughdoor) = 0;
	sprite->b15 = objData->b17 = objData->b18[sprite->frame];

	if (sprite->frame == 0) {
		turnpathoffCPP(data.byte(kDoorpath));
		data.byte(kLockstatus) = 1;
	}
}

void DreamGenContext::liftsprite(Sprite *sprite, ObjData *objData) {
	uint8 liftFlag = data.byte(kLiftflag);
	if (liftFlag == 0) { //liftclosed
		turnpathoffCPP(data.byte(kLiftpath));

		if (data.byte(kCounttoopen) != 0) {
			_dec(data.byte(kCounttoopen));
			if (data.byte(kCounttoopen) == 0)
				data.byte(kLiftflag) = 3;
		}
		sprite->frame = 0;
		sprite->b15 = objData->b17 = objData->b18[sprite->frame];
	}
	else if (liftFlag == 1) {  //liftopen
		turnpathonCPP(data.byte(kLiftpath));

		if (data.byte(kCounttoclose) != 0) {
			_dec(data.byte(kCounttoclose));
			if (data.byte(kCounttoclose) == 0)
				data.byte(kLiftflag) = 2;
		}
		sprite->frame = 12;
		sprite->b15 = objData->b17 = objData->b18[sprite->frame];
	}	
	else if (liftFlag == 3) { //openlift
		if (sprite->frame == 12) {
			data.byte(kLiftflag) = 1;
			return;
		}
		++sprite->frame;
		if (sprite->frame == 1) {
			al = 2;
			liftnoise();
		}
		sprite->b15 = objData->b17 = objData->b18[sprite->frame];
	} else { //closeLift
		assert(liftFlag == 2);
		if (sprite->frame == 0) {
			data.byte(kLiftflag) = 0;
			return;
		}
		--sprite->frame;
		if (sprite->frame == 11) {
			al = 3;
			liftnoise();
		}
		sprite->b15 = objData->b17 = objData->b18[sprite->frame];
	}
}

void DreamGenContext::facerightway() {
	uint8 *paths = getroomspathsCPP();
	uint8 dir = paths[8 * data.byte(kManspath) + 7];
	data.byte(kTurntoface) = dir;
	data.byte(kLeavedirection) = dir;
}

void DreamGenContext::findsource() {
	uint16 currentFrame = data.word(kCurrentframe);
	if (currentFrame < 160) {
		ds = data.word(kReel1);
		data.word(kTakeoff) = 0;
	} else if (currentFrame < 320) {
		ds = data.word(kReel2);
		data.word(kTakeoff) = 160;
	} else {
		ds = data.word(kReel3);
		data.word(kTakeoff) = 320;
	}
}

Frame *DreamGenContext::findsourceCPP() {
	push(ds);
	findsource();
	Frame *result = (Frame *)ds.ptr(0, 0);
	ds = pop();
	return result;
}

Reel *DreamGenContext::getreelstartCPP() {
	Reel *reel = (Reel *)segRef(data.word(kReels)).ptr(kReellist + data.word(kReelpointer) * sizeof(Reel) * 8, sizeof(Reel));
	return reel;
}

void DreamGenContext::showreelframe() {
	Reel *reel = (Reel *)es.ptr(si, sizeof(Reel));
	showreelframe(reel);
}

void DreamGenContext::showreelframe(Reel *reel) {
	uint16 x = reel->x + data.word(kMapadx);
	uint16 y = reel->y + data.word(kMapady);
	data.word(kCurrentframe) = reel->frame();
	Frame *source = findsourceCPP();
	uint16 frame = data.word(kCurrentframe) - data.word(kTakeoff);
	uint8 width, height;
	showframe(source, x, y, frame, 8, &width, &height);
}

void DreamGenContext::showgamereel() {
	uint16 reelpointer = es.word(bx+3);
	if (reelpointer >= 512)
		return;
	data.word(kReelpointer) = reelpointer;
	push(es);
	push(bx);
	plotreel();
	bx = pop();
	es = pop();
	es.word(bx+3) = data.word(kReelpointer);
}

void DreamGenContext::getreelframeax() {
	push(ds);
	data.word(kCurrentframe) = ax;
	findsource();
	es = ds;
	ds = pop();
	cx = (data.word(kCurrentframe) - data.word(kTakeoff)) * 2;
	bx = (data.word(kCurrentframe) - data.word(kTakeoff)) * 6;
}

void DreamGenContext::showrain() {
	ds = data.word(kMainsprites);
	si = 6*58;
	ax = ds.word(si+2);
	si = ax + 2080;
	Rain *rain = (Rain *)segRef(data.word(kBuffers)).ptr(kRainlist, 0);
	if (rain->x == 255)
		return;
	while (true) {
		if (rain->x == 255) {
			if (data.word(kCh1blockstocopy) != 0)
				return;
			if ((data.byte(kReallocation) == 2) && (data.byte(kBeenmugged) != 1))
					return;
			if (data.byte(kReallocation) == 55)
				return;
			randomnum1();
			if (al >= 1)
				return;
			if (data.byte(kCh0playing) != 6)
				al = 4;
			else
				al = 7;
			playchannel1();
			return;
		}
		uint16 y = rain->y + data.word(kMapady) + data.word(kMapystart);
		uint16 x = rain->x + data.word(kMapadx) + data.word(kMapxstart);
		uint16 size = rain->size;
		ax = ((uint16)(rain->w3() - rain->b5)) & 511;
		rain->setW3(ax);
		++rain;
		const uint8 *src = ds.ptr(si, 0) + ax;
		uint8 *dst = workspace() + y * 320 + x;
		for(uint16 i = 0; i < size; ++i) {
			uint8 v = src[i];
			if (v != 0)
				*dst = v;
			dst += 320-1;
		}
	}
}

} /*namespace dreamgen */

