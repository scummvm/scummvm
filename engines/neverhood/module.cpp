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

#include "neverhood/module.h"
#include "neverhood/navigationscene.h"
#include "neverhood/smackerscene.h"
#include "neverhood/modules/module1000.h"
#include "neverhood/modules/module1500.h"

namespace Neverhood {

Module::Module(NeverhoodEngine *vm, Module *parentModule)
	: Entity(vm, 0), _parentModule(parentModule), _childObject(NULL),
	_done(false), _sceneType(kSceneTypeNormal) {

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
		sendMessage(_parentModule, 0x0008, 0);
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
	return (NavigationScene*)_childObject;
}

void Module::createNavigationScene(uint32 navigationListId, int navigationIndex, const byte *itemsTypes) {
	_sceneType = kSceneTypeNavigation;
	_childObject = new NavigationScene(_vm, this, navigationListId, navigationIndex, itemsTypes);
}

void Module::createSmackerScene(uint32 fileHash, bool doubleSurface, bool canSkip, bool canAbort) {
	_sceneType = kSceneTypeSmacker;
	SmackerScene *smackerScene = new SmackerScene(_vm, this, doubleSurface, canSkip, canAbort);
	smackerScene->setFileHash(fileHash);
	smackerScene->nextVideo();
	_childObject = smackerScene;
}

void Module::createSmackerScene(const uint32 *fileHashList, bool doubleSurface, bool canSkip, bool canAbort) {
	_sceneType = kSceneTypeSmacker;
	SmackerScene *smackerScene = new SmackerScene(_vm, this, doubleSurface, canSkip, canAbort);
	smackerScene->setFileHashList(fileHashList);
	smackerScene->nextVideo();
	_childObject = smackerScene;
}

void Module::createStaticScene(uint32 backgroundFileHash, uint32 cursorFileHash) {
	_childObject = new StaticScene(_vm, this, backgroundFileHash, cursorFileHash);
}

void Module::createDemoScene() {
	_childObject = new Scene1501(_vm, this, 0x0009B624, 0, 288, 0);
}

bool Module::updateChild() {
	if (_childObject) {
		_childObject->handleUpdate();
		if (_done) {
			_done = false;
			// Save the last area type if it's a NavigationScene for further processing
			if (_sceneType == kSceneTypeNavigation)
				_navigationAreaType = navigationScene()->getNavigationAreaType();
			delete _childObject;
			_childObject = NULL;
			_sceneType = kSceneTypeNormal;
			return false;
		}
	}
	return true;
}

void Module::leaveModule(uint32 result) {
	sendMessage(_parentModule, 0x1009, result);
}

} // End of namespace Neverhood
