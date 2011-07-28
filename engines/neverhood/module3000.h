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
	void createScene3002(int which);
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
	void updateScene3010();			
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
