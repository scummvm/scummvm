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

#ifndef NEVERHOOD_MODULE2800_H
#define NEVERHOOD_MODULE2800_H

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
	bool _flag;
	uint32 _fileHash;
	void createScene(int sceneNum, int which);
	void updateScene();
};

class Scene2801 : public Scene {
public:
	Scene2801(NeverhoodEngine *vm, Module *parentModule, int which);
	~Scene2801();
protected:
	Sprite *_sprite1;
	Sprite *_sprite2;
	Sprite *_asTape;
	uint32 _paletteHash;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Class488 : public AnimatedSprite {
public:
	Class488(NeverhoodEngine *vm, Scene *parentScene, uint32 fileHash1, uint32 fileHash2, int16 x, int16 y);
	void sub434380();
	void sub4343C0();
	void setFileHashes(uint32 fileHash1, uint32 fileHash2);
protected:
	Scene *_parentScene;
	SoundResource _soundResource;
	uint32 _fileHash1, _fileHash2;
	bool _flag1, _flag2;	
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	uint32 handleMessage434340(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene2803b : public Scene {
public:
	Scene2803b(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	int _palStatus;
	Sprite *_sprite1;
	Sprite *_sprite2;
	Sprite *_sprite3;
	Sprite *_sprite4;
	Sprite *_sprite5;
	Sprite *_sprite6;
	Sprite *_sprite7;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void update45FCB0();
	void update45FD50();
	void sub460090();
	void sub460110();
	void sub460170();
	void sub460190();
	void sub4601B0();
	void sub4601D0();
	void sub4601F0(bool flag);
};

class Scene2804;

class SsScene2804RedButton : public StaticSprite {
public:
	SsScene2804RedButton(NeverhoodEngine *vm, Scene2804 *parentScene);
protected:
	SoundResource _soundResource;
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
protected:
	SoundResource _soundResource;
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
	SoundResource _soundResource;
};

class SsScene2804CrystalButton : public StaticSprite {
public:
	SsScene2804CrystalButton(NeverhoodEngine *vm, Scene2804 *parentScene, AsScene2804Crystal *asCrystal, uint crystalIndex);
protected:
	SoundResource _soundResource;
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
protected:
	SoundResource _soundResource;
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
	void initCrystalColors();
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
	SoundResource _soundResource;	
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene2806 : public Scene {
public:
	Scene2806(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	NPointArray *_pointList;
	int _pointIndex;
	NRect _clipRects[4];
	bool _fieldEC;
	Sprite *_sprite1;
	Sprite *_sprite2;
	Sprite *_sprite3;
	Sprite *_sprite4;
	Sprite *_asSpew;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void update();
	void findClosestPoint();
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
	SoundResource _soundResource1;
	SoundResource _soundResource2;
	SoundResource _soundResource3;
	SoundResource _soundResource4;
	SoundResource _soundResource5;
	SoundResource _soundResource6;
	SoundResource _soundResource7;
	SoundResource _soundResource8;
	SoundResource _soundResource9;
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
	SoundResource _soundResource;
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
	SoundResource _soundResource;
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
	SoundResource _soundResource;
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

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULE2800_H */
