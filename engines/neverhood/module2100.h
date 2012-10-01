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

#ifndef NEVERHOOD_MODULE2100_H
#define NEVERHOOD_MODULE2100_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

class Module2100 : public Module {
public:
	Module2100(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Module2100();
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void createScene(int sceneNum, int which);
	void updateScene();
};

// Scene1901

class Class538 : public AnimatedSprite {
public:
	Class538(NeverhoodEngine *vm, bool flag);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void openDoor();
	void closeDoor();
	void hide();
};

class Class539 : public AnimatedSprite {
public:
	Class539(NeverhoodEngine *vm, Sprite *klayman);
protected:
	Sprite *_klayman;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Class427 : public StaticSprite {
public:
	Class427(NeverhoodEngine *vm, Scene *parentScene, uint32 fileHash1, uint32 fileHash2, int surfacePriority, uint32 soundFileHash);
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
	Sprite *_sprite1;
	Sprite *_class427;
	Sprite *_asTape1;
	Sprite *_asTape2;
	Sprite *_class538;
	Sprite *_class539;
	int _countdown1;
	int _value1;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULE2100_H */
