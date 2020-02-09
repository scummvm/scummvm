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

#ifndef NEVERHOOD_MODULES_MODULE1100_SPRITES_H
#define NEVERHOOD_MODULES_MODULE1100_SPRITES_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

class SsScene1105Button : public StaticSprite {
public:
	SsScene1105Button(NeverhoodEngine *vm, Scene *parentScene, uint32 fileHash, NRect &collisionBounds);
protected:
	Scene *_parentScene;
	int _countdown;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class SsScene1105Symbol : public StaticSprite {
public:
	SsScene1105Symbol(NeverhoodEngine *vm, uint32 fileHash, int16 x, int16 y);
	void hide();
};

class SsScene1105SymbolDie : public StaticSprite {
public:
	SsScene1105SymbolDie(NeverhoodEngine *vm, uint dieIndex, int16 x, int16 y);
	void hide();
protected:
	uint _dieIndex;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void loadSymbolSprite();
};

class AsScene1105TeddyBear : public AnimatedSprite {
public:
	AsScene1105TeddyBear(NeverhoodEngine *vm, Scene *parentScene);
	void show();
	void hide();
protected:
	Scene *_parentScene;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class SsScene1105OpenButton : public StaticSprite {
public:
	SsScene1105OpenButton(NeverhoodEngine *vm, Scene *parentScene);
protected:
	Scene *_parentScene;
	int _countdown;
	bool _isClicked;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class KmScene1109 : public Klaymen {
public:
	KmScene1109(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y);
protected:
	uint32 xHandleMessage(int messageNum, const MessageParam &param) override;
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE1100_SPRITES_H */
