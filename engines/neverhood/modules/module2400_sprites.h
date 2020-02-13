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

#ifndef NEVERHOOD_MODULES_MODULE2400_SPRITES_H
#define NEVERHOOD_MODULES_MODULE2400_SPRITES_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"
#include "neverhood/gamemodule.h"

namespace Neverhood {

class AsScene2401WaterSpit : public AnimatedSprite {
public:
	AsScene2401WaterSpit(NeverhoodEngine *vm);
protected:
	int _soundIndex;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2401FlowingWater : public AnimatedSprite {
public:
	AsScene2401FlowingWater(NeverhoodEngine *vm);
	~AsScene2401FlowingWater() override;
protected:
	bool _isWaterFlowing;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2401WaterFlushing : public AnimatedSprite {
public:
	AsScene2401WaterFlushing(NeverhoodEngine *vm, int16 x, int16 y);
protected:
	int _countdown;
	int _flushLoopCount;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2401Door : public AnimatedSprite {
public:
	AsScene2401Door(NeverhoodEngine *vm, bool isOpen);
protected:
	int _countdown;
	bool _isOpen;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void stDoorOpenFinished();
};

class AsScene2402Door : public AnimatedSprite {
public:
	AsScene2402Door(NeverhoodEngine *vm, Scene *parentScene, bool isOpen);
protected:
	Scene *_parentScene;
	int _countdown;
	bool _isOpen;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void stDoorClosingFinished();
};

class AsScene2402TV : public AnimatedSprite {
public:
	AsScene2402TV(NeverhoodEngine *vm, Klaymen *klaymen);
	~AsScene2402TV() override;
protected:
	Klaymen *_klaymen;
	int _countdown1;
	int _countdown2;
	void upWait();
	void upFocusKlaymen();
	void stJokeFinished();
	uint32 hmJoke(int messageNum, const MessageParam &param, Entity *sender);
};

class KmScene2401 : public Klaymen {
public:
	KmScene2401(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	bool _canSpitPipe;
	bool _contSpitPipe;
	bool _readyToSpit;
	uint32 _spitPipeIndex;
	uint32 _spitDestPipeIndex;
	uint32 _spitContDestPipeIndex;

	void spitIntoPipe();
	void stTrySpitIntoPipe();
	void stContSpitIntoPipe();
	uint32 hmSpit(int messageNum, const MessageParam &param, Entity *sender);

	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene2402 : public Klaymen {
public:
	KmScene2402(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene2403 : public Klaymen {
public:
	KmScene2403(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

class KmScene2406 : public Klaymen {
public:
	KmScene2406(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y, NRect *clipRects, int clipRectsCount);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE2400_SPRITES_H */
