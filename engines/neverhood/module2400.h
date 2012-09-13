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

#ifndef NEVERHOOD_MODULE2400_H
#define NEVERHOOD_MODULE2400_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"
#include "neverhood/gamemodule.h"
#include "neverhood/module1000.h"
#include "neverhood/module1100.h"
#include "neverhood/module2100.h"

namespace Neverhood {

// Module2400

class Module2400 : public Module {
public:
	Module2400(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Module2400();
protected:
	void createScene(int sceneNum, int which);
	void updateScene();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2401WaterSpit : public AnimatedSprite {
public:
	AsScene2401WaterSpit(NeverhoodEngine *vm);
protected:
	int _soundIndex;
	SoundResource _soundResource;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2401FlowingWater : public AnimatedSprite {
public:
	AsScene2401FlowingWater(NeverhoodEngine *vm);
	virtual ~AsScene2401FlowingWater();
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
	SoundResource _soundResource;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void stDoorOpenFinished();
};

class Scene2401 : public Scene {
public:
	Scene2401(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_sprite1;
	Sprite *_asFlowingWater;
	Sprite *_ssButton;
	Sprite *_ssFloorButton;
	Sprite *_asWaterSpit[2];
	Sprite *_ssWaterPipes[10];
	Sprite *_asWaterFlushing[5];
	Sprite *_asDoor;
	bool _soundToggle;
	bool _flag;
	int _countdown1;
	int _countdown2;
	int _pipeStatus;
	int _asWaterSpitIndex;
	SoundResource _soundResource1;
	SoundResource _soundResource2;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void playPipeSound(uint32 fileHash);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULE2400_H */
