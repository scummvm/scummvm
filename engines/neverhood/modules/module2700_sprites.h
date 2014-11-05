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

#ifndef NEVERHOOD_MODULES_MODULE2700_SPRITES_H
#define NEVERHOOD_MODULES_MODULE2700_SPRITES_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

class SsCommonTrackShadowBackground : public StaticSprite {
public:
	SsCommonTrackShadowBackground(NeverhoodEngine *vm, uint32 fileHash);
};

class AsCommonCarShadow : public AnimatedSprite {
public:
	AsCommonCarShadow(NeverhoodEngine *vm, AnimatedSprite *asCar, BaseSurface *shadowSurface, uint index);
protected:
	uint _index;
	AnimatedSprite *_asCar;
	uint32 _animFileHash;
	void update();
	void updateShadow();
};

class AsCommonCarConnectorShadow : public AnimatedSprite {
public:
	AsCommonCarConnectorShadow(NeverhoodEngine *vm, Sprite *asCar, BaseSurface *shadowSurface, uint index);
protected:
	uint _index;
	Sprite *_asCar;
	void update();
};

class AsCommonCarTrackShadow : public AnimatedSprite {
public:
	AsCommonCarTrackShadow(NeverhoodEngine *vm, Sprite *asCar, BaseSurface *shadowSurface, int16 frameIndex);
protected:
	Sprite *_asCar;
	void update();
};

class KmScene2732 : public Klaymen {
public:
	KmScene2732(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE2700_SPRITES_H */
