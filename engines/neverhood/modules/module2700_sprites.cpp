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

#include "neverhood/modules/module2700_sprites.h"

namespace Neverhood {

static const NPoint kCarShadowOffsets[] = {
	{-63,  3}, {-48, 40}, {-33, 58},
	{  0, 65}, { 40, 53}, { 56, 27},
	{ 63,  0}, {-30, 26}, {  0, 30},
	{ 26, 25}
};

SsCommonTrackShadowBackground::SsCommonTrackShadowBackground(NeverhoodEngine *vm, uint32 fileHash)
	: StaticSprite(vm, 0) {

	loadSprite(fileHash, kSLFDefDrawOffset | kSLFDefPosition, 0);
}

AsCommonCarShadow::AsCommonCarShadow(NeverhoodEngine *vm, AnimatedSprite *asCar, BaseSurface *shadowSurface, uint index)
	: AnimatedSprite(vm, 1100), _asCar(asCar), _index(index), _animFileHash(0) {

	SetUpdateHandler(&AsCommonCarShadow::update);
	createShadowSurface(shadowSurface, 211, 147, 100);
	updateShadow();
}

void AsCommonCarShadow::update() {
	updateShadow();
	AnimatedSprite::update();
}

void AsCommonCarShadow::updateShadow() {
	if (_asCar->getFrameIndex() != _currFrameIndex || _asCar->getCurrAnimFileHash() != _animFileHash) {
		uint32 fileHash = _asCar->getCurrAnimFileHash();
		if (fileHash == 0x35698F78 || fileHash == 0x192ADD30 || fileHash == 0x9C220DA4 ||
			fileHash == 0x9966B138 || fileHash == 0xB579A77C || fileHash == 0xA86A9538 ||
			fileHash == 0xD4220027 || fileHash == 0xD00A1364 || fileHash == 0xD4AA03A4 ||
			fileHash == 0xF46A0324) {
			startAnimation(fileHash, _asCar->getFrameIndex(), -1);
			_newStickFrameIndex = _asCar->getFrameIndex();
		}
		_animFileHash = fileHash;
	}
	_x = _asCar->getX() + kCarShadowOffsets[_index].x;
	_y = _asCar->getY() + kCarShadowOffsets[_index].y;
	if (!_asCar->getVisible()) {
		startAnimation(0x1209E09F, 0, -1);
		_newStickFrameIndex = 0;
	}
	setDoDeltaX(_asCar->isDoDeltaX() ? 1 : 0);
}

AsCommonCarConnectorShadow::AsCommonCarConnectorShadow(NeverhoodEngine *vm, Sprite *asCar, BaseSurface *shadowSurface, uint index)
	: AnimatedSprite(vm, 1100), _asCar(asCar), _index(index) {

	SetUpdateHandler(&AsCommonCarConnectorShadow::update);
	createShadowSurface1(shadowSurface, 0x60281C10, 150);
	startAnimation(0x60281C10, -1, -1);
	_newStickFrameIndex = STICK_LAST_FRAME;
}

void AsCommonCarConnectorShadow::update() {
	_x = _asCar->getX() + kCarShadowOffsets[_index].x;
	_y = _asCar->getY() + kCarShadowOffsets[_index].y;
	AnimatedSprite::update();
}

AsCommonCarTrackShadow::AsCommonCarTrackShadow(NeverhoodEngine *vm, Sprite *asCar, BaseSurface *shadowSurface, int16 frameIndex)
	: AnimatedSprite(vm, 1100), _asCar(asCar) {

	SetUpdateHandler(&AsCommonCarTrackShadow::update);
	createShadowSurface1(shadowSurface, 0x0759129C, 100);
	startAnimation(0x0759129C, frameIndex, -1);
	_newStickFrameIndex = frameIndex;
}

void AsCommonCarTrackShadow::update() {
	_x = _asCar->getX();
	_y = _asCar->getY();
	AnimatedSprite::update();
}

KmScene2732::KmScene2732(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene2732::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4804:
		GotoState(&Klaymen::stPeekInside);
		break;
	case 0x483C:
		GotoState(&Klaymen::stPeekInsideReturn);
		break;
	default:
		break;
	}
	return 0;
}

} // End of namespace Neverhood
