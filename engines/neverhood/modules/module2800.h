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

#ifndef NEVERHOOD_MODULES_MODULE2800_H
#define NEVERHOOD_MODULES_MODULE2800_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

// Module2800

class Module2800 : public Module {
public:
	Module2800(NeverhoodEngine *vm, Module *parentModule, int which);
	~Module2800() override;
protected:
	int _sceneNum;
	uint32 _currentMusicFileHash;
	MusicResource *_musicResource;
	void createScene(int sceneNum, int which);
	void updateScene();
	void updateMusic(bool halfVolume);
};

class Scene2801 : public Scene {
public:
	Scene2801(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene2801() override;
protected:
	Sprite *_asTape;
	uint32 _paletteHash;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene2802 : public Scene {
public:
	Scene2802(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene2802() override;
protected:
	SmackerPlayer *_smackerPlayer;
	uint _currRadioMusicIndex;
	int _currTuneStatus;
	int _countdown1;
	int _countdown2;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void incRadioMusicIndex(int delta);
	void changeTuneStatus(int prevTuneStatus, int newTuneStatus);
};

class AsScene2803LightCord;

class Scene2803 : public Scene {
public:
	Scene2803(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	StaticSprite *_asTestTubeOne;
	StaticSprite *_asTestTubeTwo;
	StaticSprite *_asTestTubeThree;
	Sprite *_asRope;
	AsScene2803LightCord *_asLightCord;
	StaticSprite *_sprite3;
	StaticSprite *_sprite4;
	StaticSprite *_sprite5;
	StaticSprite *_sprite6;
	StaticSprite *_sprite7;
	StaticSprite *_sprite8;
	StaticSprite *_sprite9;
	Sprite *_sprite10;
	NRect _clipRectsFloor[2];
	NRect _clipRectsStairs[3];
	int _paletteArea;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void upKlaymenStairs();
	void klaymenStairs();
	void klaymenFloor();
	void toggleBackground();
	void changeBackground();
	void setPaletteArea0();
	void setPaletteArea1();
	void updatePaletteArea();
};

class Scene2803Small : public Scene {
public:
	Scene2803Small(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	int _paletteArea;
	Sprite *_sprite1;
	Sprite *_sprite2;
	Sprite *_sprite3;
	Sprite *_sprite4;
	Sprite *_sprite5;
	Sprite *_sprite6;
	Sprite *_sprite7;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void upKlaymenSlope();
	void upKlaymenFloor();
	void klaymenSlope();
	void klaymenFloor();
	void setPaletteArea0();
	void setPaletteArea1();
	void setPaletteArea2();
	void setPaletteArea3();
	void updatePaletteArea(bool instantly);
};

class SsScene2804Flash;
class AsScene2804Crystal;

class Scene2804 : public Scene {
public:
	Scene2804(NeverhoodEngine *vm, Module *parentModule, int which);
	bool isWorking() const { return _isWorking; }
protected:
	int _countdown1;
	int _countdown2;
	int _countdown3;
	int _beamStatus;
	bool _isSolved;
	bool _isWorking;
	Sprite *_ssRedButton;
	Sprite *_asCoil;
	Sprite *_asTarget;
	SsScene2804Flash *_ssFlash;
	AsScene2804Crystal *_asCrystals[5];
	Sprite *_ssCrystalButtons[5];
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene2805 : public Scene {
public:
	Scene2805(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_sprite1;
	Sprite *_sprite2;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene2806 : public Scene {
public:
	Scene2806(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	NPointArray *_pointList;
	int _pointIndex;
	NRect _clipRects[4];
	Sprite *_sprite1;
	Sprite *_sprite2;
	Sprite *_sprite3;
	Sprite *_sprite4;
	Sprite *_asSpew;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void update();
	void findClosestPoint();
};

class Scene2807 : public Scene {
public:
	Scene2807(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2808Flow;
class AsScene2808TestTube;

class Scene2808 : public Scene {
public:
	Scene2808(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	int _countdown;
	int _testTubeSetNum;
	AsScene2808Flow *_asFlow;
	int _leaveResult;
	bool _isFlowing;
	AsScene2808TestTube *_asTestTubes[3];
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void update();
	bool isMixtureGood();
	bool isAnyTestTubeFilled();
};

class Scene2809 : public Scene {
public:
	Scene2809(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	NPointArray *_pointList;
	int _pointIndex;
	NRect _clipRects[4];
	Sprite *_sprite1;
	Sprite *_sprite2;
	Sprite *_sprite3;
	Sprite *_sprite4;
	Sprite *_asSpew;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void findClosestPoint();
};

class Scene2810 : public Scene {
public:
	Scene2810(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene2810() override;
protected:
	Sprite *_sprite1;
	Sprite *_sprite2;
	Sprite *_sprite3;
	Sprite *_asRope;
	Sprite *_sprite4;
	Sprite *_asTape;
	Sprite *_sprite5;
	Sprite *_sprite6;
	bool _isRopingDown;
	NRect _clipRects[2];
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void insertKlaymenLadder();
};

class Scene2812 : public Scene {
public:
	Scene2812(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_asWinch;
	Sprite *_asTrapDoor;
	Sprite *_asRope;
	Sprite *_sprite3;
	Sprite *_sprite2;
	Sprite *_sprite4;
	Sprite *_ssTape;
	Sprite *_asKey;
	Sprite *_sprite1;
	bool _isRopingDown;
	int _paletteArea;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void setPaletteArea0(bool instantly);
	void setPaletteArea1(bool instantly);
	void setPaletteArea2(bool instantly);
	void updatePaletteArea(bool instantly);
};

class Scene2822 : public Scene {
public:
	Scene2822(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_ssButton;
	int _scrollIndex;
	int _countdown;
	int _countdownStatus;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE2800_H */
