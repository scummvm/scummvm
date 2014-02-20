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

// TODO: I couldn't come up with a better name than 'Module' so far

#ifndef NEVERHOOD_MODULES_MODULE1500_H
#define NEVERHOOD_MODULES_MODULE1500_H

#include "neverhood/neverhood.h"
#include "neverhood/module.h"
#include "neverhood/scene.h"
#include "neverhood/smackerscene.h"

namespace Neverhood {

class Module1500 : public Module {
public:
	Module1500(NeverhoodEngine *vm, Module *parentModule, int which);
protected:
	int _sceneNum;
	void createScene(int sceneNum, int which);
	void updateScene();
};

class Scene1501 : public Scene {
public:
	Scene1501(NeverhoodEngine *vm, Module *parentModule, uint32 backgroundFileHash, uint32 soundFileHash, int countdown2, int countdown3);
protected:
	int _countdown1;
	int _countdown2;
	int _countdown3;
	bool _skip;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULES_MODULE1500_H */
