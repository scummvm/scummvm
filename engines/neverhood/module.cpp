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

#include "neverhood/module.h"
#include "neverhood/navigationscene.h"
#include "neverhood/smackerscene.h"

namespace Neverhood {

Module::Module(NeverhoodEngine *vm, Module *parentModule)
	: Entity(vm, 0), _parentModule(parentModule), _childObject(NULL),
	_done(false) {
	
	SetMessageHandler(&Module::handleMessage);
	
}

Module::~Module() {
	delete _childObject;
}

void Module::draw() {
	if (_childObject)
		_childObject->draw();
}

uint32 Module::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	switch (messageNum) {
	case 0x0008:
		if (_parentModule)
			sendMessage(_parentModule, 8, 0);
		return 0;
	case 0x1009:
		_moduleResult = param.asInteger();
		_done = true;
		return 0;
	case 0x100A:
	case 0x1023:
	case 0x1024:
		// Unused resource preloading messages
		return 0;
	default:
		if (_childObject && sender == _parentModule)
			return sender->sendMessage(_childObject, messageNum, param);
	}
	return 0;
}

NavigationScene *Module::navigationScene() {
	// Not so nice
	return (NavigationScene*)_childObject;
}

void Module::createNavigationScene(uint32 navigationListId, int navigationIndex, const byte *itemsTypes) {
	_childObject = new NavigationScene(_vm, this, navigationListId, navigationIndex, itemsTypes);
}

void Module::createSmackerScene(uint32 fileHash, bool doubleSurface, bool flag1, bool canAbort) {
	SmackerScene *smackerScene;
	smackerScene = new SmackerScene(_vm, this, doubleSurface, flag1, canAbort);
	smackerScene->setFileHash(fileHash);
	smackerScene->nextVideo();
	_childObject = smackerScene;
}

bool Module::updateChild() {
	if (_childObject) {
		_childObject->handleUpdate();
		if (_done) {
			_done = false;
			delete _childObject;
			_childObject = NULL;
			return false;
		}
	}
	return true;
}

} // End of namespace Neverhood
