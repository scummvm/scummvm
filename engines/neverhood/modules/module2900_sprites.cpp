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

#include "neverhood/modules/module2900_sprites.h"

namespace Neverhood {

static const uint32 kSsScene2901LocationButtonFileHashes[] = {
	0x2311326A,
	0x212323AC,
	0x10098138,
	0x25213167,
	0x1119A363,
	0x94452612,
	0x39464212,
	0x01860450,
	0x53002104,
	0x58E68412,
	0x18600300,
	0xB650A890,
	0x2452A7C4,
	0xA0232748,
	0x08862B02,
	0x2491E648,
	0x0010EB46,
	0x214C8A11,
	0x16A31921,
	0x0AC33A00,
	0x238028AA,
	0x26737A21,
	0x063039A8,
	0x51286C60,
	0x464006B4,
	0x42242538,
	0x20716010,
	0x4A2000AE,
	0x225124A6,
	0x28E82E45,
	0x58652C04,
	0xC82210A4,
	0x62A84060,
	0xC0693CB4,
	0x22212C64,
	0x5034EA71
};

static const NPoint kSsScene2901LocationButtonPoints[] = {
	{525, 120}, {576, 149}, {587, 205},
	{538, 232}, {484, 205}, {479, 153}
};

static const uint32 kSsScene2901LocationButtonLightFileHashes1[] = {
	0x03136246,
	0x2106216E,
	0x4025A13A,
	0x21816927,
	0x110B2202,
	0xCC0522B2,
	0x3CC24258,
	0x59C600F0,
	0x534A2480,
	0x50E61019,
	0x34400150,
	0x225BA090,
	0xB059AFC4,
	0xE093A741,
	0x0086BF09,
	0x3281E760,
	0xA048AB42,
	0x20649C01,
	0x14611904,
	0x26E33850,
	0x23A52A68,
	0xA2733024,
	0x10203880,
	0x1B2DE860,
	0x0644A6EC,
	0x426E20BC,
	0x80292014,
	0x4360B02E,
	0x22742664,
	0x98682705,
	0x0925B82C,
	0x5C2918A4,
	0xD2284920,
	0x41083CA6,
	0x6824A864,
	0x50266B10
};

static const uint32 kSsScene2901LocationButtonLightFileHashes2[] = {
	0x43C46D4C,
	0x43C4AD4C,
	0x43C52D4C,
	0x43C62D4C,
	0x43C02D4C,
	0x43CC2D4C
};

static const uint32 kSsScene2901BrokenButtonFileHashes[] = {
	0x3081BD3A,
	0xD3443003,
	0x0786A320,
	0xE3A22029,
	0x61611814,
	0x425848E2
};

static const uint32 kSsScene2901BigButtonFileHashes[] = {
	0x010D7748,
	0x9D02019A,
	0x351A2F43,
	0x448138E5,
	0x02788CF0,
	0x71718024
};

SsScene2901LocationButton::SsScene2901LocationButton(NeverhoodEngine *vm, Scene *parentScene, int which, uint index)
	: StaticSprite(vm, 900), _parentScene(parentScene), _index(index), _countdown1(0) {

	const NPoint &pt = kSsScene2901LocationButtonPoints[_index];

	loadSprite(kSsScene2901LocationButtonFileHashes[which * 6 + index], kSLFDefDrawOffset | kSLFDefPosition, 800);
	_collisionBounds.set(pt.x - 25, pt.y - 25, pt.x + 25, pt.y + 25);
	setVisible(false);
	loadSound(0, 0x440430C0);
	SetUpdateHandler(&SsScene2901LocationButton::update);
	SetMessageHandler(&SsScene2901LocationButton::handleMessage);
}

void SsScene2901LocationButton::update() {
	updatePosition();
	if (_countdown1 != 0 && (--_countdown1) == 0) {
		setVisible(false);
	}
}

uint32 SsScene2901LocationButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown1 == 0) {
			playSound(0);
			setVisible(true);
			_countdown1 = 4;
			sendMessage(_parentScene, 0x2001, _index);
		}
		messageResult = 1;
		break;
	default:
		break;
	}
	return messageResult;
}

SsScene2901LocationButtonLight::SsScene2901LocationButtonLight(NeverhoodEngine *vm, int which, uint index)
	: StaticSprite(vm, 900), _index(index) {

	loadSprite(kSsScene2901LocationButtonLightFileHashes1[which * 6 + index], kSLFDefDrawOffset | kSLFDefPosition, 900);
	setVisible(false);
	loadSound(0, kSsScene2901LocationButtonLightFileHashes2[_index]);
}

void SsScene2901LocationButtonLight::show() {
	playSound(0);
	setVisible(true);
	updatePosition();
}

void SsScene2901LocationButtonLight::hide() {
	setVisible(false);
	updatePosition();
}

SsScene2901BrokenButton::SsScene2901BrokenButton(NeverhoodEngine *vm, int which)
	: StaticSprite(vm, 900) {

	loadSprite(kSsScene2901BrokenButtonFileHashes[which], kSLFDefDrawOffset | kSLFDefPosition, 900);
}

SsScene2901BigButton::SsScene2901BigButton(NeverhoodEngine *vm, Scene *parentScene, int which)
	: StaticSprite(vm, 900), _parentScene(parentScene), _which(which), _countdown1(0) {

	loadSprite(kSsScene2901BigButtonFileHashes[which], kSLFDefDrawOffset | kSLFDefPosition, 400);
	_collisionBounds.set(62, 94, 322, 350);
	setVisible(false);
	loadSound(0, 0xF3D420C8);
	SetUpdateHandler(&SsScene2901BigButton::update);
	SetMessageHandler(&SsScene2901BigButton::handleMessage);
}

void SsScene2901BigButton::update() {
	updatePosition();
	if (_countdown1 != 0 && (--_countdown1) == 0) {
		setVisible(false);
		sendMessage(_parentScene, 0x2000, 0);
	}
}

uint32 SsScene2901BigButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown1 == 0) {
			playSound(0);
			setVisible(true);
			_countdown1 = 4;
		}
		messageResult = 1;
		break;
	default:
		break;
	}
	return messageResult;
}

} // End of namespace Neverhood
