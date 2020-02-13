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

#ifndef NEVERHOOD_MODULES_MODULE1000_H
#define NEVERHOOD_MODULES_MODULE1000_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

class Module1000 : public Module {
public:
	Module1000(NeverhoodEngine *vm, Module *parentModule, int which);
	~Module1000() override;
protected:
	int _sceneNum;
	uint32 _musicFileHash;
	void createScene(int sceneNum, int which);
	void updateScene();
};

class Scene1001 : public Scene {
public:
	Scene1001(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene1001() override;
protected:
	Sprite *_asHammer;
	Sprite *_asDoor;
	Sprite *_asWindow;
	Sprite *_asLever;
	Sprite *_ssButton;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1002 : public Scene {
public:
	Scene1002(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene1002() override;
protected:
	Sprite *_asRing1;
	Sprite *_asRing2;
	Sprite *_asRing3;
	Sprite *_asRing4;
	Sprite *_asRing5;
	Sprite *_asDoor;
	Sprite *_asDoorSpy;
	Sprite *_asVenusFlyTrap;
	Sprite *_ssLadderArch;
	Sprite *_ssLadderArchPart1;
	Sprite *_ssLadderArchPart2;
	Sprite *_ssLadderArchPart3;
	Sprite *_ssCeiling;
	Sprite *_asKlaymenLadderHands;
	Sprite *_asKlaymenPeekHand;
	Sprite *_asOutsideDoorBackground;
	Sprite *_ssPressButton;
	bool _isKlaymenFloor;
	bool _isClimbingLadder;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1004 : public Scene {
public:
	Scene1004(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_asKlaymenLadderHands;
	Sprite *_asTrashCan;
	int _paletteAreaStatus;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void updatePaletteArea();
};

class Scene1005 : public Scene {
public:
	Scene1005(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void drawTextToBackground();
	uint32 getTextIndex();
	uint32 getTextIndex1();
	uint32 getKloggsTextIndex();
	uint32 getTextIndex3();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE1000_H */
