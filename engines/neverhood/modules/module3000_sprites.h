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

#ifndef NEVERHOOD_MODULES_MODULE3000_SPRITES_H
#define NEVERHOOD_MODULES_MODULE3000_SPRITES_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"
#include "neverhood/modules/module1200.h"

namespace Neverhood {

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

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE3000_SPRITES_H */
