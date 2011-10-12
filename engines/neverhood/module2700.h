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

#ifndef NEVERHOOD_MODULE2700_H
#define NEVERHOOD_MODULE2700_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

// Module2700

class Module2700 : public Module {
public:
	Module2700(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Module2700();
protected:
	SoundResource _soundResource1;
	SoundResource _soundResource2;
	SoundResource _soundResource3;
	SoundResource _soundResource4;
	int _soundIndex;
	bool _flag1;
	uint32 _scene2711StaticSprites[6];
	void createScene(int sceneNum, int which);
	void updateScene();
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void createScene2703(int which, uint32 sceneInfoId, const uint32 *staticSprites = NULL, const NRect *clipRect = NULL);
	void createScene2704(int which, uint32 sceneInfoId, int16 value, const uint32 *staticSprites = NULL, const NRect *clipRect = NULL);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULE2700_H */
