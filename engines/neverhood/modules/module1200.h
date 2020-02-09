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

#ifndef NEVERHOOD_MODULES_MODULE1200_H
#define NEVERHOOD_MODULES_MODULE1200_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

class Module1200 : public Module {
public:
	Module1200(NeverhoodEngine *vm, Module *parentModule, int which);
	~Module1200() override;
protected:
	int _sceneNum;
	void createScene(int sceneNum, int which);
	void updateScene();
};

class AsScene1201TntMan;

class Scene1201 : public Scene {
public:
	Scene1201(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene1201() override;
protected:
	Sprite *_asMatch;
	AsScene1201TntMan *_asTntMan;
	Sprite *_asCreature;
	Sprite *_asTntManRope;
	Sprite *_asLeftDoor;
	Sprite *_asRightDoor;
	Sprite *_asTape;
	Sprite *_asKlaymenHead;
	bool _creatureExploded;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1202 : public Scene {
public:
	Scene1202(NeverhoodEngine *vm, Module *parentModule);
	~Scene1202() override;
protected:
	PaletteResource _paletteResource;
	Sprite *_asTntItems[18];
	int _counter;
	int _clickedIndex;
	byte _paletteData[1024];
	bool _isPuzzleSolved;
	bool _soundToggle;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmSolved(int messageNum, const MessageParam &param, Entity *sender);
	bool isSolved();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE1200_H */
