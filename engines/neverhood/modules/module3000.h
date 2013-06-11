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

#ifndef NEVERHOOD_MODULES_MODULE3000_H
#define NEVERHOOD_MODULES_MODULE3000_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"
#include "neverhood/modules/module1200.h"

namespace Neverhood {

class Module3000 : public Module {
public:
	Module3000(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Module3000();
protected:
	int _soundVolume;
	bool _isWallBroken;
	void createScene(int sceneNum, int which);
	void updateScene();
};

// Scene3009

class Scene3009;

class SsScene3009FireCannonButton : public StaticSprite {
public:
	SsScene3009FireCannonButton(NeverhoodEngine *vm, Scene3009 *parentScene);
protected:
	Scene3009 *_parentScene;
	bool _isClicked;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class SsScene3009SymbolEdges : public StaticSprite {
public:
	SsScene3009SymbolEdges(NeverhoodEngine *vm, int index);
	void show();
	void hide();
	void startBlinking();
protected:
	int _blinkCountdown;
	bool _blinkToggle;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class SsScene3009TargetLine : public StaticSprite {
public:
	SsScene3009TargetLine(NeverhoodEngine *vm, int index);
	void show();
};

class SsScene3009SymbolArrow : public StaticSprite {
public:
	SsScene3009SymbolArrow(NeverhoodEngine *vm, Sprite *asSymbol, int index);
	void hide();
protected:
	Sprite *_asSymbol;
	int _index;
	int _incrDecr;
	bool _enabled;
	int _countdown;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene3009VerticalIndicator : public AnimatedSprite {
public:
	AsScene3009VerticalIndicator(NeverhoodEngine *vm, Scene3009 *parentScene, int index);
	void show();
protected:
	Scene3009 *_parentScene;
	bool _enabled;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene3009HorizontalIndicator : public AnimatedSprite {
public:
	AsScene3009HorizontalIndicator(NeverhoodEngine *vm, Scene3009 *parentScene, uint32 cannonTargetStatus);
	void show();
	void stMoveLeft();
	void stMoveRight();
protected:
	Scene3009 *_parentScene;
	bool _enabled;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void suMoveLeft();
	void suMoveRight();
};

class AsScene3009Symbol : public AnimatedSprite {
public:
	AsScene3009Symbol(NeverhoodEngine *vm, Scene3009 *parentScene, int symbolPosition);
	void hide();
protected:
	Scene3009 *_parentScene;
	int _symbolPosition;
	uint32 _symbolIndex;
	SsScene3009SymbolArrow *_ssArrowPrev;
	SsScene3009SymbolArrow *_ssArrowNext;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Scene3009 : public Scene {
public:
	Scene3009(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Scene3009();
	bool isTurning();
protected:
	int _lockSymbolsPart1Countdown;
	int _lockSymbolsPart2Countdown;
	SmackerPlayer *_cannonSmackerPlayer;
	Sprite *_ssFireCannonButton;
	SsScene3009SymbolEdges *_ssSymbolEdges[2];
	SsScene3009TargetLine *_ssTargetLines[2];
	AsScene3009VerticalIndicator *_asVerticalIndicator;
	AsScene3009HorizontalIndicator *_asHorizontalIndicator;
	AsScene3009Symbol *_asSymbols[6];
	uint32 _cannonTargetStatus;
	uint32 _correctSymbols[6];
	bool _keepVideo;
	bool _moveCannonLeftFirst;
	bool _isTurning;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void playActionVideo();
	bool isSymbolsPart1Solved();
	bool isSymbolsPart2Solved();
	void openSmacker(uint32 fileHash, bool keepLastFrame);
};

// Scene3010

class SsScene3010DeadBoltButton : public StaticSprite {
public:
	SsScene3010DeadBoltButton(NeverhoodEngine *vm, Scene *parentScene, int buttonIndex, int initCountdown, bool initDisabled);
	void setCountdown(int count);
protected:
	Scene *_parentScene;
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

// Scene3011

class SsScene3011Button : public StaticSprite {
public:
	SsScene3011Button(NeverhoodEngine *vm, Scene *parentScene, bool flag);
protected:
	Scene *_parentScene;
	int _countdown;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class AsScene3011Symbol : public AnimatedSprite {
public:
	AsScene3011Symbol(NeverhoodEngine *vm, int symbolIndex, bool largeSymbol);
	void show(bool isNoisy);
	void hide();
	void stopSymbolSound();
	void change(int symbolIndex, bool isNoisy);
	int getSymbolIndex() { return _largeSymbol ? _symbolIndex : _symbolIndex - 12; }
protected:
	bool _largeSymbol;
	bool _isNoisy;
	int _symbolIndex;
};

class Scene3011 : public Scene {
public:
	Scene3011(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_ssButton;
	AsScene3011Symbol *_asSymbols[12];
	int _updateStatus;
	bool _buttonClicked;
	int _countdown;
	int _noisySymbolIndex;
	int _currentSymbolIndex;   
	int _noisyRandomSymbolIndex;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void fadeIn();
	void fadeOut();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE3000_H */
