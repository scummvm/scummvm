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

#ifndef NEVERHOOD_MODULES_MODULE1300_H
#define NEVERHOOD_MODULES_MODULE1300_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

class SmackerPlayer;

class Module1300 : public Module {
public:
	Module1300(NeverhoodEngine *vm, Module *parentModule, int which);
	~Module1300() override;
protected:
	int _sceneNum;
	uint32 _musicFileHash;
	void createScene(int sceneNum, int which);
	void updateScene();
};

class Scene1302 : public Scene {
public:
	Scene1302(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_asVenusFlyTrap;
	Sprite *_asBridge;
	Sprite *_ssFence;
	Sprite *_asRing1;
	Sprite *_asRing2;
	Sprite *_asRing3;
	Sprite *_asRing4;
	Sprite *_asRing5;
	Sprite *_class595;
	Sprite *_sprite1;
	Sprite *_sprite2;
	Sprite *_sprite3;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1303 : public Scene {
public:
	Scene1303(NeverhoodEngine *vm, Module *parentModule);
protected:
	Sprite *_sprite1;
	Sprite *_asBalloon;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1304 : public Scene {
public:
	Scene1304(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_sprite1;
	Sprite *_asKey;
	Sprite *_asNeedle;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1305 : public Scene {
public:
	Scene1305(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1306 : public Scene {
public:
	Scene1306(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene1306() override;
protected:
	Sprite *_ssButton;
	Sprite *_asTape;
	AnimatedSprite *_asElevatorDoor;
	Sprite *_asElevator;
	Sprite *_sprite1;
	Sprite *_asKey;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage416EB0(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1307 : public Scene {
public:
	Scene1307(NeverhoodEngine *vm, Module *parentModule);
protected:
	NPointArray *_keyHolePoints;
	NRect _keyHoleRects[16];
	NRect _clipRects[4];
	Sprite *_asKeys[3];
	int _countdown;
	Sprite *_asCurrKey;
	bool _isInsertingKey;
	bool _doLeaveScene;
	bool _isPuzzleSolved;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1308 : public Scene {
public:
	Scene1308(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_asTape;
	Sprite *_asJaggyDoor;
	Sprite *_asLightWallSymbols;
	Sprite *_ssNumber1;
	Sprite *_ssNumber2;
	Sprite *_ssNumber3;
	AnimatedSprite *_asProjector;
	Sprite *_sprite1;
	Sprite *_sprite2;
	Sprite *_sprite3;
	Sprite *_sprite4;
	Sprite *_sprite5;
	bool _isProjecting;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene1317 : public Scene {
public:
	Scene1317(NeverhoodEngine *vm, Module *parentModule);
protected:
	SmackerPlayer *_smackerPlayer;
	bool _klaymenBlinks;
	int _klaymenBlinkCountdown;
	int _decisionCountdown;
	uint32 _smackerFileHash;
	bool _keepLastSmackerFrame;
	void update();
	void upChooseKing();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmChooseKing(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmHoborgAsKing(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmEndMovie(int messageNum, const MessageParam &param, Entity *sender);
	void stChooseKing();
	void stNoDecisionYet();
	void stHoborgAsKing();
	void stKlaymenAsKing();
	void stEndMovie();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE1300_H */
