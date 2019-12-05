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

#include "neverhood/modules/module2600_sprites.h"

namespace Neverhood {

SsScene2609Button::SsScene2609Button(NeverhoodEngine *vm, Scene *parentScene)
	: StaticSprite(vm, 1400), _parentScene(parentScene), _countdown(0) {

	SetUpdateHandler(&SsScene2609Button::update);
	SetMessageHandler(&SsScene2609Button::handleMessage);

	loadSprite(0x825A6923, kSLFDefDrawOffset | kSLFDefPosition | kSLFDefCollisionBoundsOffset, 400);
	if (!getGlobalVar(V_WATER_RUNNING))
		setVisible(false);
	loadSound(0, 0x10267160);
	loadSound(1, 0x7027FD64);
	loadSound(2, 0x44043000);
	loadSound(3, 0x44045000);
}

void SsScene2609Button::update() {
	updatePosition();
	if (_countdown != 0 && (--_countdown == 0)) {
		if (getGlobalVar(V_WATER_RUNNING)) {
			setGlobalVar(V_WATER_RUNNING, 0);
			sendMessage(_parentScene, 0x2001, 0);
		} else {
			setGlobalVar(V_WATER_RUNNING, 1);
			sendMessage(_parentScene, NM_POSITION_CHANGE, 0);
		}
	}
}

uint32 SsScene2609Button::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown == 0) {
			sendMessage(_parentScene, 0x2000, 0);
			if (getGlobalVar(V_WATER_RUNNING)) {
				setVisible(false);
				playSound(3);
				playSound(1);
				_countdown = 12;
			} else {
				setVisible(true);
				playSound(2);
				playSound(0);
				_countdown = 96;
			}
		}
		messageResult = 1;
		break;
	default:
		break;
	}
	return messageResult;
}

AsScene2609Water::AsScene2609Water(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1000) {

	_x = 240;
	_y = 420;
	setDoDeltaX(1);
	createSurface1(0x9C210C90, 1200);
	setClipRect(260, 260, 400, 368);
	_vm->_soundMan->addSound(0x08526C36, 0xDC2769B0);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2609Water::handleMessage);
	if (getGlobalVar(V_WATER_RUNNING))
		sendMessage(this, NM_POSITION_CHANGE, 0);
}

AsScene2609Water::~AsScene2609Water() {
	_vm->_soundMan->deleteSoundGroup(0x08526C36);
}

uint32 AsScene2609Water::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2001:
		stopAnimation();
		setVisible(false);
		_vm->_soundMan->stopSound(0xDC2769B0);
		break;
	case NM_POSITION_CHANGE:
		startAnimation(0x9C210C90, 0, -1);
		setVisible(true);
		_vm->_soundMan->playSoundLooping(0xDC2769B0);
		break;
	default:
		break;
	}
	return messageResult;
}

} // End of namespace Neverhood
