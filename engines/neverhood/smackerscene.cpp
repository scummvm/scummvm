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

#include "neverhood/smackerscene.h"
#include "neverhood/smackerplayer.h"

namespace Neverhood {

SmackerScene::SmackerScene(NeverhoodEngine *vm, Module *parentModule, bool doubleSurface, bool canSkip, bool canAbort)
	: Scene(vm, parentModule), _doubleSurface(doubleSurface), _canSkip(canSkip), _canAbort(canAbort), _videoPlayedBefore(false),
	_fileHashListIndex(-1), _fileHashList(NULL), _playNextVideoFlag(false) {

	debug(0, "SmackerScene::SmackerScene(%d, %d, %d)", doubleSurface, canSkip, canAbort);

	// NOTE: Merged from SmackerScene::init, maybe split again if needed (incl. parameter flags)

	if (getGlobalVar(V_SMACKER_CAN_ABORT)) {
		_canSkip = true;
		_canAbort = true;
	}

	if (!_doubleSurface)
		_vm->_screen->clear();

	_fileHash[0] = 0;
	_fileHash[1] = 0;

	SetUpdateHandler(&SmackerScene::update);
	SetMessageHandler(&SmackerScene::handleMessage);

}

SmackerScene::~SmackerScene() {

}

void SmackerScene::setFileHash(uint32 fileHash) {
	debug(0, "SmackerScene::setFileHash(%08X)", fileHash);
	_fileHash[0] = fileHash;
	_fileHashList = _fileHash;
}

void SmackerScene::setFileHashList(const uint32 *fileHashList) {
	debug(0, "SmackerScene::setFileHashList(...)");
	_fileHashList = fileHashList;
}

void SmackerScene::nextVideo() {
	debug(0, "SmackerScene::nextVideo()");

	_fileHashListIndex++;

	if (_fileHashList && _fileHashList[_fileHashListIndex] != 0) {
		uint32 smackerFileHash = _fileHashList[_fileHashListIndex];
		ResourceHandle resourceHandle;
		_vm->_res->queryResource(smackerFileHash, resourceHandle);
		if (resourceHandle.type() != kResTypeVideo) {
			// Not a Smacker file
			_vm->_screen->setSmackerDecoder(NULL);
			sendMessage(_parentModule, 0x1009, 0);
			return;
		}
		_videoPlayedBefore = getSubVar(VA_SMACKER_PLAYED, smackerFileHash);
		if (!_videoPlayedBefore)
			setSubVar(VA_SMACKER_PLAYED, smackerFileHash, 1);
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
	case NM_KEYPRESS_SPACE:
		if ((_videoPlayedBefore && _canSkip) || (_canAbort && _canSkip))
			_playNextVideoFlag = true;
		break;
	case NM_KEYPRESS_ESC:
		if (_canAbort)
			sendMessage(_parentModule, 0x1009, 0);
		break;
	case NM_ANIMATION_STOP:
		_playNextVideoFlag = true;
		break;
	default:
		break;
	}
	return messageResult;
}

} // End of namespace Neverhood
