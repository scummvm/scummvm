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

class Scene2805 : public Scene {
public:
	Scene2805(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_sprite1;
	Sprite *_sprite2;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULE2800_H */
