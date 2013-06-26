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

#ifndef NEVERHOOD_MODULES_MODULE2100_H
#define NEVERHOOD_MODULES_MODULE2100_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

class Module2100 : public Module {
public:
	Module2100(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Module2100();
protected:
	int _sceneNum;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void createScene(int sceneNum, int which);
	void updateScene();
};

// Scene1901

class AsScene2101Door : public AnimatedSprite {
public:
	AsScene2101Door(NeverhoodEngine *vm, bool isOpen);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void stOpenDoor();
	void stCloseDoor();
	void stCloseDoorDone();
};

class AsScene2101HitByDoorEffect : public AnimatedSprite {
public:
	AsScene2101HitByDoorEffect(NeverhoodEngine *vm, Sprite *klaymen);
protected:
	Sprite *_klaymen;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class SsCommonFloorButton : public StaticSprite {
public:
	SsCommonFloorButton(NeverhoodEngine *vm, Scene *parentScene, uint32 fileHash1, uint32 fileHash2, int surfacePriority, uint32 soundFileHash);
protected:
	Scene *_parentScene;
	uint32 _soundFileHash;
	uint32 _fileHash1, _fileHash2;
	int16 _countdown;
	void update();	
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene2101 : public Scene {
public:
	Scene2101(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_ssFloorButton;
	Sprite *_asTape1;
	Sprite *_asTape2;
	Sprite *_asDoor;
	Sprite *_asHitByDoorEffect;
	int _countdown1;
	int _doorStatus;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE2100_H */
