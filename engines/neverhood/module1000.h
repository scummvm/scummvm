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

#ifndef NEVERHOOD_MODULE1000_H
#define NEVERHOOD_MODULE1000_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"

namespace Neverhood {

// Module1000

class Module1000 : public Module {
public:
	Module1000(NeverhoodEngine *vm, Module *parentModule, int which);
	virtual ~Module1000();
protected:
	uint32 _musicFileHash;
	// TODO ResourceTable _resourceTable1;
	// TODO ResourceTable _resourceTable2;
	// TODO ResourceTable _resourceTable3;
	// TODO ResourceTable _resourceTable4;
	void createScene1001(int which);			
	void createScene1002(int which);			
	void createScene1003(int which);			
	void createScene1004(int which);			
	void createScene1005(int which);			
	void updateScene1001();			
	void updateScene1002();			
	void updateScene1003();			
	void updateScene1004();			
	void updateScene1005();			
};

// Scene1001

class Class509 : public AnimatedSprite {
public:
	Class509(NeverhoodEngine *vm);
protected:
	SoundResource _soundResource1;	
	SoundResource _soundResource2;	
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void handleMessage2000h();
	void callback1();
	void callback2();
	void callback3();
};

class Scene1001 : public Scene {
public:
	Scene1001(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	Sprite *_class508;
	Sprite *_class509;
	Sprite *_class510;
	Sprite *_class511;
	Sprite *_class608;
	int16 _fieldE4;
	int16 _fieldE6;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULE1000_H */
