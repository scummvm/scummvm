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

#ifndef NEVERHOOD_MODULE3000_H
#define NEVERHOOD_MODULE3000_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"
#include "neverhood/module1200.h"

namespace Neverhood {

class Module3000 : public Module {
public:
	Module3000(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Module3000();
protected:
	bool _moduleDone;
	int _moduleDoneStatus;
	int _soundVolume;
	bool _flag;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void createScene3002(int which);
	void createScene3002b(int which);
	void createScene3003(int which);
	void createScene3004(int which);
	void createScene3005(int which);
	void createScene3006(int which);
	void createScene3007(int which);
	void createScene3008(int which);
	void createScene3009(int which);
	void createScene3010(int which);
	void createScene3011(int which);
	void createScene3012(int which);
	void createScene3013(int which);
	void updateScene3002();			
	void updateScene3002b();			
	void updateScene3003();			
	void updateScene3004();			
	void updateScene3005();			
	void updateScene3006();			
	void updateScene3007();			
	void updateScene3009();			
	void updateScene3010();			
};

// Scene3009

class Scene3009;

class Class438 : public StaticSprite {
public:
	Class438(NeverhoodEngine *vm, Scene3009 *parentScene);
protected:
	Scene3009 *_parentScene;
	SoundResource _soundResource;
	bool _flag1;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Class439 : public StaticSprite {
public:
	Class439(NeverhoodEngine *vm, int index);
	void show();
	void hide();
	void startBlinking();
protected:
	int _blinkCountdown;
	bool _blinkToggle;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Class440 : public StaticSprite {
public:
	Class440(NeverhoodEngine *vm, int index);
};

class Class522 : public AnimatedSprite {
public:
	Class522(NeverhoodEngine *vm, Scene3009 *parentScene, int index);
	void show();
protected:
	Scene3009 *_parentScene;
	bool _enabled;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene3009 : public Scene {
public:
	Scene3009(NeverhoodEngine *vm, Module *parentModule, int which);
	bool sub462E90();
protected:
	int _countdown1;
	int _countdown2;
	SmackerPlayer *_smackerPlayer;
	Sprite *_class438;
	Class439 *_class439Array[2];
	Class440 *_class440Array[2];
	Class522 *_class522;
#if 0	
	Class523 *_class523;
	Class524 *_class524Array[6];
#endif	
	uint32 _varValue;
	uint32 _varValueArray[6];
	bool _flag1;
	bool _flag2;
	bool _flag3;
	bool _flag4;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void sub462DC0();
	bool sub462E10();
	bool sub462E50();
};

// Scene3010

class SsScene3010DeadBoltButton : public StaticSprite {
public:
	SsScene3010DeadBoltButton(NeverhoodEngine *vm, Scene *parentScene, int buttonIndex, int initCountdown, bool initDisabled);
	void setCountdown(int count);
protected:
	Scene *_parentScene;
	SoundResource _soundResource1;
	SoundResource _soundResource2;
	SoundResource _soundResource3;
	int _buttonIndex;
	bool _buttonEnabled;
	bool _buttonLocked;
	int _countdown1;
	int _countdown2;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void disableButton();
	void setSprite(uint32 fileHash);
};

class AsScene3010DeadBolt : public AnimatedSprite {
public:
	AsScene3010DeadBolt(NeverhoodEngine *vm, Scene *parentScene, int boltIndex, bool initUnlocked);
	void setCountdown(int count);
	void lock();
	void unlock(bool skipAnim);
protected:
	Scene *_parentScene;
	SoundResource _soundResource1;
	SoundResource _soundResource2;
	SoundResource _soundResource3;
	int _boltIndex;
	int _countdown;
	bool _soundToggle;
	bool _unlocked;
	bool _locked;
	void update();
	uint32 hmAnimation(int messageNum, const MessageParam &param, Entity *sender);
	void stIdle();
	void stIdleMessage();
	void stDisabled();
	void stDisabledMessage();
};

class Scene3010 : public Scene {
public:
	Scene3010(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	SoundResource _soundResource;
	int _countdown;
	bool _doorUnlocked;
	bool _checkUnlocked;
	SsScene3010DeadBoltButton *_ssDeadBoltButtons[3];
	AsScene3010DeadBolt *_asDeadBolts[3];
	bool _boltUnlocked[3];
	bool _boltUnlocking[3];
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULE3000_H */
