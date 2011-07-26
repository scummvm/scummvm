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

#ifndef NEVERHOOD_MODULE1700_H
#define NEVERHOOD_MODULE1700_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"
#include "neverhood/smackerscene.h"

namespace Neverhood {

class Module1700 : public Module {
public:
	Module1700(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Module1700();
protected:
	SoundResource _soundResource;
	void update();
	void createScene1701(int which);
	void createScene1702(int which);
	void createScene1703(int which);
	void createScene1704(int which);
	void createScene1705(int which);
	void updateScene1701();			
	void updateScene1702();			
	void updateScene1703();			
	void updateScene1705();			
};

// Scene1705

class Class602 : public StaticSprite {
public:
	Class602(NeverhoodEngine *vm, uint32 fileHash, int index);
};

class Class606 : public StaticSprite {
public:
	Class606(NeverhoodEngine *vm, Scene *parentScene, int index, int surfacePriority, int16 x, int16 y, uint32 fileHash);
protected:	
	Scene *_parentScene;
	int _index;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1705 : public Scene {
public:
	Scene1705(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_sprite;
	Sprite *_class606;
	int _paletteArea;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULE1700_H */
