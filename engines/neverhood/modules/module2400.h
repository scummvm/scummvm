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

#ifndef NEVERHOOD_MODULES_MODULE2400_H
#define NEVERHOOD_MODULES_MODULE2400_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"
#include "neverhood/gamemodule.h"
#include "neverhood/diskplayerscene.h"

namespace Neverhood {

// Module2400

class Module2400 : public Module {
public:
	Module2400(NeverhoodEngine *vm, Module *parentModule, int which);
	~Module2400() override;
protected:
	int _sceneNum;
	void createScene(int sceneNum, int which);
	void updateScene();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
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
	bool _unkFlag;
	int _countdown1;
	int _countdown2;
	int _pipeStatus;
	int _asWaterSpitIndex;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void playPipeSound(uint32 fileHash);
};

class Scene2402 : public Scene {
public:
	Scene2402(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene2402() override;
protected:
	Sprite *_asDoor;
	Sprite *_ssButton;
	Sprite *_asTape;
	StaticSprite *_ssDoorFrame;
	int _pipeStatus;
	int _countdown;
	bool _soundToggle;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void playPipeSound(uint32 fileHash);
};

class Scene2403 : public Scene {
public:
	Scene2403(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_ssButton;
	Sprite *_asTape;
	Sprite *_asKey;
	Sprite *_asLightCord;
	bool _isClimbingLadder;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene2406 : public Scene {
public:
	Scene2406(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_asTape;
	Sprite *_asKey;
	NRect _clipRects[2];
	bool _isClimbingLadder;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE2400_H */
