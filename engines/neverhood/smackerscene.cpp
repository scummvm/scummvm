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

#include "neverhood/smackerscene.h"

namespace Neverhood {

SmackerScene::SmackerScene(NeverhoodEngine *vm, Module *parentModule, bool doubleSurface, bool flag1, bool canAbort)
	: Scene(vm, parentModule, true), _doubleSurface(doubleSurface), _flag1(flag1), _canAbort(canAbort), _fieldDF(false),
	_fileHashListIndex(-1), _fileHashList(NULL), _playNextVideoFlag(false) {

	debug("SmackerScene::SmackerScene(%d, %d, %d)", doubleSurface, flag1, canAbort);

	// NOTE: Merged from SmackerScene::init, maybe split again if needed (incl. parameter flags)
	
	if (getGlobalVar(V_SMACKER_CAN_ABORT)) {
		_flag1 = true;
		_canAbort = true;
	}
	
	if (_doubleSurface) {
		_vm->_screen->clear();
	}

	_fileHash[0] = 0; 
	_fileHash[1] = 0;

	SetUpdateHandler(&SmackerScene::update);
	SetMessageHandler(&SmackerScene::handleMessage);

}

SmackerScene::~SmackerScene() {

}

void SmackerScene::setFileHash(uint32 fileHash) {
	debug("SmackerScene::setFileHash(%08X)", fileHash);
	_fileHash[0] = fileHash;
	_fileHashList = _fileHash;
}

void SmackerScene::setFileHashList(const uint32 *fileHashList) {
	debug("SmackerScene::setFileHashList(...)");
	_fileHashList = fileHashList;
}

void SmackerScene::nextVideo() {
	debug("SmackerScene::nextVideo()");

	_fileHashListIndex++;
	
	if (_fileHashList && _fileHashList[_fileHashListIndex] != 0) {
		uint32 smackerFileHash = _fileHashList[_fileHashListIndex];
		if (_vm->_res->getResourceTypeByHash(smackerFileHash) != 10) {
			// Not a Smacker file
			_vm->_screen->setSmackerDecoder(NULL);
			sendMessage(_parentModule, 0x1009, 0);
			return;
		}
		_fieldDF = getSubVar(0x00800410, smackerFileHash);
		if (!_fieldDF) {
			setSubVar(0x00800410, smackerFileHash, 1);
		}
		if (_fileHashListIndex == 0)
			_smackerPlayer = addSmackerPlayer(new SmackerPlayer(_vm, this, smackerFileHash, _doubleSurface, false));
		else
			_smackerPlayer->open(smackerFileHash, false);
		_vm->_screen->setSmackerDecoder(_smackerPlayer->getSmackerDecoder());
	} else {
		_vm->_screen->setSmackerDecoder(NULL);
		sendMessage(_parentModule, 0x1009, 0);
	}

}

void SmackerScene::update() {
	if (_playNextVideoFlag) {
		nextVideo();
		_playNextVideoFlag = false;
	}
	Scene::update();
}

uint32 SmackerScene::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0009:
		if ((_fieldDF && _flag1) || (_canAbort && _flag1))
			_playNextVideoFlag = true;
		break;
	case 0x000C:
		if (_canAbort)
			sendMessage(_parentModule, 0x1009, 0);
		break;
	case 0x3002:
		_playNextVideoFlag = true;
		break;
	}
	return messageResult;
}

} // End of namespace Neverhood
