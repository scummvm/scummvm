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

#ifndef NEVERHOOD_MODULES_MODULE2900_SPRITES_H
#define NEVERHOOD_MODULES_MODULE2900_SPRITES_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

// Module2900

class SsScene2901LocationButton : public StaticSprite {
public:
	SsScene2901LocationButton(NeverhoodEngine *vm, Scene *parentScene, int which, uint index);
protected:
	Scene *_parentScene;
	uint _index;
	int _countdown1;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class SsScene2901LocationButtonLight : public StaticSprite {
public:
	SsScene2901LocationButtonLight(NeverhoodEngine *vm, int which, uint index);
	void show();
	void hide();
protected:
	uint _index;
};

class SsScene2901BrokenButton : public StaticSprite {
public:
	SsScene2901BrokenButton(NeverhoodEngine *vm, int which);
};

class SsScene2901BigButton : public StaticSprite {
public:
	SsScene2901BigButton(NeverhoodEngine *vm, Scene *parentScene, int which);
protected:
	Scene *_parentScene;
	int _which;
	int _countdown1;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE2900_SPRITES_H */
