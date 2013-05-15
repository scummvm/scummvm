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
	virtual ~Module2800();
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
	virtual ~Scene2801();
protected:
	Sprite *_asTape;
	uint32 _paletteHash;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene2802 : public Scene {
public:
	Scene2802(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Scene2802();
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

class AsScene2803LightCord : public AnimatedSprite {
public:
	AsScene2803LightCord(NeverhoodEngine *vm, Scene *parentScene, uint32 fileHash1, uint32 fileHash2, int16 x, int16 y);
	void stPulled();
	void stIdle();
	void setFileHashes(uint32 fileHash1, uint32 fileHash2);
protected:
	Scene *_parentScene;
	uint32 _fileHash1, _fileHash2;
	bool _isPulled, _isBusy;	
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmPulled(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2803TestTubeOne : public AnimatedSprite {
public:
	AsScene2803TestTubeOne(NeverhoodEngine *vm, uint32 fileHash1, uint32 fileHash2);
protected:
	uint32 _fileHash1, _fileHash2;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2803Rope : public AnimatedSprite {
public:
	AsScene2803Rope(NeverhoodEngine *vm, Scene *parentScene, int16 x);
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmReleased(int messageNum, const MessageParam &param, Entity *sender);
	void stReleased();
	void stHide();
};

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

class Scene2804;

class SsScene2804RedButton : public StaticSprite {
public:
	SsScene2804RedButton(NeverhoodEngine *vm, Scene2804 *parentScene);
protected:
	Scene2804 *_parentScene;
	int _countdown;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class SsScene2804LightCoil : public StaticSprite {
public:
	SsScene2804LightCoil(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class SsScene2804BeamCoilBody : public StaticSprite {
public:
	SsScene2804BeamCoilBody(NeverhoodEngine *vm);
};

class SsScene2804LightTarget : public StaticSprite {
public:
	SsScene2804LightTarget(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class SsScene2804Flash : public StaticSprite {
public:
	SsScene2804Flash(NeverhoodEngine *vm);
	void show();
};

class AsScene2804CrystalWaves : public AnimatedSprite {
public:
	AsScene2804CrystalWaves(NeverhoodEngine *vm, uint crystalIndex);
	void show();
	void hide();
protected:
	uint _crystalIndex;
};

class AsScene2804Crystal : public AnimatedSprite {
public:
	AsScene2804Crystal(NeverhoodEngine *vm, AsScene2804CrystalWaves *asCrystalWaves, uint crystalIndex);
	void show();
	void hide();
	void activate();
	int16 getColorNum() const { return _colorNum; }
protected:
	AsScene2804CrystalWaves *_asCrystalWaves;
	uint _crystalIndex;
	int16 _colorNum;
	bool _isLightOn;
	bool _isShowing;
};

class SsScene2804CrystalButton : public StaticSprite {
public:
	SsScene2804CrystalButton(NeverhoodEngine *vm, Scene2804 *parentScene, AsScene2804Crystal *asCrystal, uint crystalIndex);
protected:
	Scene2804 *_parentScene;
	AsScene2804Crystal *_asCrystal;
	uint _crystalIndex;
	int _countdown;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2804BeamCoil : public AnimatedSprite {
public:
	AsScene2804BeamCoil(NeverhoodEngine *vm, Scene *parentScene, SsScene2804BeamCoilBody *ssBeamCoilBody);
	virtual ~AsScene2804BeamCoil();
protected:
	Scene *_parentScene;
	SsScene2804BeamCoilBody *_ssBeamCoilBody;
	int _countdown;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void show();
	void hide();
	void stBeaming();
	uint32 hmBeaming(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2804BeamTarget : public AnimatedSprite {
public:
	AsScene2804BeamTarget(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

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

class AsScene2806Spew : public AnimatedSprite {
public:
	AsScene2806Spew(NeverhoodEngine *vm);
protected:
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

class SsScene2808Dispenser : public StaticSprite {
public:
	SsScene2808Dispenser(NeverhoodEngine *vm, Scene *parentScene, int testTubeSetNum, int testTubeIndex);
	void startCountdown(int index);
protected:
	Scene *_parentScene;
	int _countdown;
	int _testTubeSetNum, _testTubeIndex;
	void update();	
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2808TestTube : public AnimatedSprite {
public:
	AsScene2808TestTube(NeverhoodEngine *vm, int testTubeSetNum, int testTubeIndex, SsScene2808Dispenser *ssDispenser);
	void fill();
	void flush();
	uint32 getFillLevel() const { return _fillLevel; }
protected:
	SsScene2808Dispenser *_ssDispenser;
	int _testTubeSetNum;
	uint32 _fillLevel;
	int _testTubeIndex;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2808Handle : public AnimatedSprite {
public:
	AsScene2808Handle(NeverhoodEngine *vm, Scene *parentScene, int testTubeSetNum);
	void activate();
	void stActivated();
protected:
	Scene *_parentScene;
	int _testTubeSetNum;
	bool _isActivated;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmActivating(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2808Flow : public AnimatedSprite {
public:
	AsScene2808Flow(NeverhoodEngine *vm, Scene *parentScene, int testTubeSetNum);
	void start();
	void stKeepFlowing();
protected:
	Scene *_parentScene;
	int _testTubeSetNum;
	uint32 hmFlowing(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2808LightEffect : public AnimatedSprite {
public:
	AsScene2808LightEffect(NeverhoodEngine *vm, int which);
protected:
	int _countdown;
	void update();
};

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

class AsScene2809Spew : public AnimatedSprite {
public:
	AsScene2809Spew(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
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

class AsScene2810Rope : public AnimatedSprite {
public:
	AsScene2810Rope(NeverhoodEngine *vm, Scene *parentScene, int16 x);
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene2810 : public Scene {
public:
	Scene2810(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Scene2810();
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

class AsScene2812Winch : public AnimatedSprite {
public:
	AsScene2812Winch(NeverhoodEngine *vm);
	virtual ~AsScene2812Winch();
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene2812Rope : public AnimatedSprite {
public:
	AsScene2812Rope(NeverhoodEngine *vm, Scene *parentScene);
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 hmRopingDown(int messageNum, const MessageParam &param, Entity *sender);
	void stRopingDown();
};

class AsScene2812TrapDoor : public AnimatedSprite {
public:
	AsScene2812TrapDoor(NeverhoodEngine *vm);
protected:
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
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
