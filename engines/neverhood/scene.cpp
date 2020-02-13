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

#include "neverhood/console.h"
#include "neverhood/scene.h"
#include "neverhood/smackerplayer.h"

namespace Neverhood {

Scene::Scene(NeverhoodEngine *vm, Module *parentModule)
	: Entity(vm, 0), _parentModule(parentModule), _dataResource(vm), _hitRects(NULL),
	_mouseCursorWasVisible(true) {

	_isKlaymenBusy = false;
	_doConvertMessages = false;
	_messageList = NULL;
	_rectType = 0;
	_mouseClickPos.x = 0;
	_mouseClickPos.y = 0;
	_mouseClicked = false;
	_rectList = NULL;
	_klaymen = NULL;
	_mouseCursor = NULL;
	_palette = NULL;
	_background = NULL;
	clearHitRects();
	clearCollisionSprites();
	_vm->_screen->setFps(24);
	_vm->_screen->setSmackerDecoder(NULL);
	_canAcceptInput = true;
	_messageList2 = NULL;
	_smackerPlayer = NULL;
	_isMessageListBusy = false;
	_messageValue = -1;
	_messageListStatus = 0;
	_messageListCount = 0;
	_messageListIndex = 0;

	_backgroundFileHash = _cursorFileHash = 0;

	SetUpdateHandler(&Scene::update);
	SetMessageHandler(&Scene::handleMessage);

	_vm->_screen->clearRenderQueue();
}

Scene::~Scene() {

	_vm->_screen->setSmackerDecoder(NULL);

	if (_palette) {
		removeEntity(_palette);
		delete _palette;
	}

	// Delete all entities
	for (Common::Array<Entity*>::iterator iter = _entities.begin(); iter != _entities.end(); iter++)
		delete *iter;

	// Don't delete surfaces since they always belong to an entity

	// Purge the resources after each scene
	_vm->_res->purgeResources();

}

void Scene::draw() {
	if (_smackerPlayer) {
		if (_smackerPlayer->getSurface())
			_smackerPlayer->getSurface()->draw();
	} else {
		for (Common::Array<BaseSurface*>::iterator iter = _surfaces.begin(); iter != _surfaces.end(); iter++)
			(*iter)->draw();
	}
}

void Scene::addEntity(Entity *entity) {
	int index = 0, insertIndex = -1;
	for (Common::Array<Entity*>::iterator iter = _entities.begin(); iter != _entities.end(); iter++) {
		if ((*iter)->getPriority() > entity->getPriority()) {
			insertIndex = index;
			break;
		}
		index++;
	}
	if (insertIndex >= 0)
		_entities.insert_at(insertIndex, entity);
	else
		_entities.push_back(entity);
}

bool Scene::removeEntity(Entity *entity) {
	for (uint index = 0; index < _entities.size(); index++)
		if (_entities[index] == entity) {
			_entities.remove_at(index);
			return true;
		}
	return false;
}

void Scene::addSurface(BaseSurface *surface) {
	if (surface) {
		int index = 0, insertIndex = -1;
		for (Common::Array<BaseSurface*>::iterator iter = _surfaces.begin(); iter != _surfaces.end(); iter++) {
			if ((*iter)->getPriority() > surface->getPriority()) {
				insertIndex = index;
				break;
			}
			index++;
		}
		if (insertIndex >= 0)
			_surfaces.insert_at(insertIndex, surface);
		else
			_surfaces.push_back(surface);
	}
}

bool Scene::removeSurface(BaseSurface *surface) {
	for (uint index = 0; index < _surfaces.size(); index++) {
		if (_surfaces[index] == surface) {
			_surfaces.remove_at(index);
			return true;
		}
	}
	return false;
}

void Scene::printSurfaces(Console *con) {
	for (uint index = 0; index < _surfaces.size(); index++) {
		NDrawRect drawRect = _surfaces[index]->getDrawRect();
		NRect clipRect = _surfaces[index]->getClipRect();
		int priority = _surfaces[index]->getPriority();
		con->debugPrintf("%d ('%s'): Priority %d, draw rect (%d, %d, %d, %d), clip rect (%d, %d, %d, %d)\n",
			index, _surfaces[index]->getName().c_str(), priority,
			drawRect.x, drawRect.y, drawRect.x2(), drawRect.y2(),
			clipRect.x1, clipRect.y1, clipRect.x2, clipRect.y2);
	}
}

Sprite *Scene::addSprite(Sprite *sprite) {
	addEntity(sprite);
	addSurface(sprite->getSurface());
	return sprite;
}

void Scene::removeSprite(Sprite *sprite) {
	removeSurface(sprite->getSurface());
	removeEntity(sprite);
}

void Scene::setSurfacePriority(BaseSurface *surface, int priority) {
	surface->setPriority(priority);
	if (removeSurface(surface))
		addSurface(surface);
}

void Scene::setSpriteSurfacePriority(Sprite *sprite, int priority) {
	if (sprite)
		setSurfacePriority(sprite->getSurface(), priority);
}

void Scene::deleteSprite(Sprite **sprite) {
	removeCollisionSprite(*sprite);
	removeSurface((*sprite)->getSurface());
	removeEntity(*sprite);
	delete *sprite;
	*sprite = NULL;
}

Background *Scene::addBackground(Background *background) {
	addEntity(background);
	addSurface(background->getSurface());
	return background;
}

void Scene::setBackground(uint32 fileHash) {
	_background = addBackground(new Background(_vm, fileHash, 0, 0));
	_backgroundFileHash = fileHash;
}

void Scene::changeBackground(uint32 fileHash) {
	_background->load(fileHash);
}

void Scene::setPalette(uint32 fileHash) {
	_palette = fileHash ? new Palette(_vm, fileHash) : new Palette(_vm);
	_palette->usePalette();
}

void Scene::setHitRects(uint32 id) {
	setHitRects(_vm->_staticData->getHitRectList(id));
}

void Scene::setHitRects(HitRectList *hitRects) {
	_hitRects = hitRects;
}

Sprite *Scene::insertStaticSprite(uint32 fileHash, int surfacePriority) {
	return addSprite(new StaticSprite(_vm, fileHash, surfacePriority));
}

void Scene::insertScreenMouse(uint32 fileHash, const NRect *mouseRect) {
	NRect rect = NRect::make(-1, -1, -1, -1);
	if (mouseRect)
		rect = *mouseRect;
	insertMouse(new Mouse(_vm, fileHash, rect));
	_cursorFileHash = fileHash;
}

void Scene::insertPuzzleMouse(uint32 fileHash, int16 x1, int16 x2) {
	insertMouse(new Mouse(_vm, fileHash, x1, x2));
	_cursorFileHash = fileHash;
}

void Scene::insertNavigationMouse(uint32 fileHash, int type) {
	insertMouse(new Mouse(_vm, fileHash, type));
	_cursorFileHash = fileHash;
}

void Scene::showMouse(bool visible) {
	_mouseCursor->getSurface()->setVisible(visible);
}

void Scene::changeMouseCursor(uint32 fileHash) {
	_mouseCursor->load(fileHash);
	_mouseCursor->updateCursor();
}

SmackerPlayer *Scene::addSmackerPlayer(SmackerPlayer *smackerPlayer) {
	addEntity(smackerPlayer);
	addSurface(smackerPlayer->getSurface());
	return smackerPlayer;
}

void Scene::update() {

	if (_mouseClicked) {
		if (_klaymen) {
			if (_canAcceptInput &&
				_klaymen->hasMessageHandler() &&
				sendMessage(_klaymen, 0x1008, 0) != 0 &&
				queryPositionSprite(_mouseClickPos.x, _mouseClickPos.y)) {
				_mouseClicked = false;
			} else if (_canAcceptInput &&
				_klaymen->hasMessageHandler() &&
				sendMessage(_klaymen, 0x1008, 0) != 0) {
				_mouseClicked = !queryPositionRectList(_mouseClickPos.x, _mouseClickPos.y);
			}
		} else if (queryPositionSprite(_mouseClickPos.x, _mouseClickPos.y)) {
			_mouseClicked = false;
		}
	}

	processMessageList();

	// Update all entities
	for (Common::Array<Entity*>::iterator iter = _entities.begin(); iter != _entities.end(); iter++)
		(*iter)->handleUpdate();

}

void Scene::leaveScene(uint32 result) {
	sendMessage(_parentModule, 0x1009, result);
}

uint32 Scene::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	switch (messageNum) {
	case NM_MOUSE_MOVE:
		if (_mouseCursor && _mouseCursor->hasMessageHandler())
			sendMessage(_mouseCursor, 0x4002, param);
		break;
	case NM_MOUSE_CLICK:
		_mouseClicked = true;
		_mouseClickPos = param.asPoint();
		break;
	case 0x0006:
		sendMessage(_parentModule, 0x1009, param);
		break;
	case 0x1006:
		// Sent by Klaymen when its animation sequence has finished
		if (_isKlaymenBusy) {
			_isKlaymenBusy = false;
			if (_messageListIndex == _messageListCount) {
				// If the current message list was processed completely,
				// sent Klaymen into the idle state.
				sendMessage(_klaymen, NM_KLAYMEN_STAND_IDLE, 0);
			} else {
				// Else continue with the next message in the current message list
				processMessageList();
			}
		}
		break;
	case 0x1007:
		// This isn't sent by any code, check if it's in a message list
		// This cancels the current message list and sets Klaymen into the idle state.
		if (_isKlaymenBusy) {
			_isKlaymenBusy = false;
			_messageList = NULL;
			sendMessage(_klaymen, NM_KLAYMEN_STAND_IDLE, 0);
		}
		break;
	case NM_MOUSE_HIDE:
		if (_mouseCursor) {
			_mouseCursorWasVisible = _mouseCursor->getSurface()->getVisible();
			_mouseCursor->getSurface()->setVisible(false);
		}
		break;
	case NM_MOUSE_SHOW:
		if (_mouseCursorWasVisible && _mouseCursor) {
			_mouseCursor->getSurface()->setVisible(true);
		}
		break;
	case NM_PRIORITY_CHANGE:
		// Set the sender's surface priority
		setSurfacePriority(((Sprite*)sender)->getSurface(), param.asInteger());
		break;
	default:
		break;
	}
	return 0;
}

bool Scene::queryPositionSprite(int16 mouseX, int16 mouseY) {
	for (uint i = 0; i < _collisionSprites.size(); i++) {
		Sprite *sprite = _collisionSprites[i];
		if (sprite->hasMessageHandler() && sprite->isPointInside(mouseX, mouseY) &&
			sendPointMessage(sprite, 0x1011, _mouseClickPos) != 0) {
			return true;
		}
	}
	return false;
}

bool Scene::queryPositionRectList(int16 mouseX, int16 mouseY) {
	int16 klaymenX = _klaymen->getX();
	int16 klaymenY = _klaymen->getY();
	if (_rectType == 1) {
		RectList &rectList = *_rectList;
		for (uint i = 0; i < rectList.size(); i++) {
			debug(2, "(%d, %d) ? (%d, %d, %d, %d)", klaymenX, klaymenY, rectList[i].rect.x1, rectList[i].rect.y1, rectList[i].rect.x2, rectList[i].rect.y2);
			if (rectList[i].rect.contains(klaymenX, klaymenY)) {
				for (uint j = 0; j < rectList[i].subRects.size(); j++) {
					debug(2, "  (%d, %d) ? (%d, %d, %d, %d)", mouseX, mouseY, rectList[i].subRects[j].rect.x1, rectList[i].subRects[j].rect.y1, rectList[i].subRects[j].rect.x2, rectList[i].subRects[j].rect.y2);
					if (rectList[i].subRects[j].rect.contains(mouseX, mouseY)) {
						return setMessageList2(rectList[i].subRects[j].messageListId);
					}
				}
			}
		}
	} else if (_rectType == 2) {
		MessageList *messageList = _dataResource.getMessageListAtPos(klaymenX, klaymenY, mouseX, mouseY);
		if (messageList && messageList->size())
			setMessageList2(messageList, true, true);
	}
	return true;
}

void Scene::setMessageList(uint32 id, bool canAcceptInput, bool doConvertMessages) {
	setMessageList(_vm->_staticData->getMessageList(id), canAcceptInput, doConvertMessages);
}

void Scene::setMessageList(MessageList *messageList, bool canAcceptInput, bool doConvertMessages) {
	_messageList = messageList;
	_messageListCount = _messageList ? _messageList->size() : 0;
	_messageListIndex = 0;
	_isKlaymenBusy = false;
	_canAcceptInput = canAcceptInput;
	_doConvertMessages = doConvertMessages;
	_messageListStatus = 1;
	sendMessage(_klaymen, 0x101C, 0);
}

bool Scene::setMessageList2(uint32 id, bool canAcceptInput, bool doConvertMessages) {
	return setMessageList2(_vm->_staticData->getMessageList(id), canAcceptInput, doConvertMessages);
}

bool Scene::setMessageList2(MessageList *messageList, bool canAcceptInput, bool doConvertMessages) {
	if (_messageListStatus == 0 ||
		(_messageListStatus == 1 && messageList != _messageList2) ||
		(_messageListStatus == 2 && messageList == _messageList2)) {
		// NOTE Skipped unneeded resource preloading code
		_messageValue = -1;
		_messageList2 = messageList;
		setMessageList(messageList, canAcceptInput, doConvertMessages);
		return true;
	}
	return false;
}

bool Scene::isMessageList2(uint32 id) {
	return _messageList2 == _vm->_staticData->getMessageList(id);
}

void Scene::processMessageList() {
	debug(7, "Scene::processMessageList() _isMessageListBusy = %d; _isKlaymenBusy = %d", _isMessageListBusy, _isKlaymenBusy);

	if (_isMessageListBusy || _isKlaymenBusy)
		return;

	_isMessageListBusy = true;

	if (!_messageList) {
		_messageList2 = NULL;
		_messageListStatus = 0;
	}

	if (_messageList && _klaymen) {

#if 0
		debug("MessageList: %p, %d", (void*)_messageList, _messageList->size());
		for (uint i = 0; i < _messageList->size(); ++i) {
			if (i == _messageListIndex) debugN("**"); else debugN("  ");
			debug("(%08X, %08X)", (*_messageList)[i].messageNum, (*_messageList)[i].messageValue);
		}
		debug("--------------------------------");
#endif

		while (_messageList && _messageListIndex < _messageListCount && !_isKlaymenBusy) {
			uint32 messageNum = (*_messageList)[_messageListIndex].messageNum;
			uint32 messageParam = (*_messageList)[_messageListIndex].messageValue;

			++_messageListIndex;
			if (_messageListIndex == _messageListCount)
				sendMessage(_klaymen, 0x1021, 0);
			if (_doConvertMessages)
				messageNum = convertMessageNum(messageNum);
			if (messageNum == 0x1009 || messageNum == 0x1024) {
				sendMessage(_parentModule, messageNum, messageParam);
			} else if (messageNum == 0x100A) {
				_messageValue = messageParam;
				sendMessage(_parentModule, messageNum, messageParam);
			} else if (messageNum == 0x4001) {
				_isKlaymenBusy = true;
				sendPointMessage(_klaymen, 0x4001, _mouseClickPos);
			} else if (messageNum == 0x100D) {
				if (this->hasMessageHandler() && sendMessage(this, NM_ANIMATION_START, messageParam) != 0)
					continue;
			} else if (messageNum == 0x101A) {
				_messageListStatus = 0;
			} else if (messageNum == 0x101B) {
				_messageListStatus = 2;
			} else if (messageNum == 0x1020) {
				_canAcceptInput = false;
			} else if (messageNum >= 0x2000 && messageNum <= 0x2FFF) {
				if (this->hasMessageHandler() && sendMessage(this, messageNum, messageParam) != 0) {
					_isMessageListBusy = false;
					return;
				}
			} else if (messageNum != 0x4003) {
				_isKlaymenBusy = true;
				if (_klaymen->hasMessageHandler() && sendMessage(_klaymen, messageNum, messageParam) != 0) {
					_isKlaymenBusy = false;
				}
			}
			if (_messageListIndex == _messageListCount) {
				_canAcceptInput = true;
				_messageList = NULL;
			}
		}
	}

	_isMessageListBusy = false;

}

void Scene::cancelMessageList() {
	_isKlaymenBusy = false;
	_messageList = NULL;
	_canAcceptInput = true;
	sendMessage(_klaymen, NM_KLAYMEN_STAND_IDLE, 0);
}

void Scene::setRectList(uint32 id) {
	setRectList(_vm->_staticData->getRectList(id));
}

void Scene::setRectList(RectList *rectList) {
	_rectList = rectList;
	_rectType = 1;
}

void Scene::clearRectList() {
	_rectList = NULL;
	_rectType = 0;
}

void Scene::loadHitRectList() {
	HitRectList *hitRectList = _dataResource.getHitRectList();
	if (hitRectList) {
		_hitRectList = *hitRectList;
		setHitRects(&_hitRectList);
	}
}

void Scene::loadDataResource(uint32 fileHash) {
	_dataResource.load(fileHash);
	_rectType = 2;
	if (_klaymen)
		_klaymen->loadDataResource(fileHash);
}

uint16 Scene::convertMessageNum(uint32 messageNum) {
	switch (messageNum) {
	case 0x00004004:
		return 0x4001;
	case 0x00000083:
		return 0x100A;
	case 0x044001C8:
		return 0x481C;
	case 0x02420480:
		return 0x4818;
	case 0x08004025:
		return 0x100D;
	case 0x04404281:
		return 0x4824;
	case 0x08400880:
		return 0x4825;
	case 0x08209081:
		return 0x4823;
	case 0x24000060:
		return 0x1009;
	case 0x42002200:
		return 0x4004;
	case 0x428D4894:
		return 0x101A;
	default:
		break;
	}
	return 0x1000;
}

void Scene::clearHitRects() {
	_hitRects = NULL;
}

HitRect *Scene::findHitRectAtPos(int16 x, int16 y) {
	static HitRect kDefaultHitRect = {NRect(), 0x5000};
	if (_hitRects)
		for (HitRectList::iterator it = _hitRects->begin(); it != _hitRects->end(); it++)
			if ((*it).rect.contains(x, y))
				return &(*it);
	return &kDefaultHitRect;
}

void Scene::addCollisionSprite(Sprite *sprite) {
	int index = 0, insertIndex = -1;
	for (Common::Array<Sprite*>::iterator iter = _collisionSprites.begin(); iter != _collisionSprites.end(); iter++) {
		if ((*iter)->getPriority() > sprite->getPriority()) {
			insertIndex = index;
			break;
		}
		index++;
	}
	if (insertIndex >= 0)
		_collisionSprites.insert_at(insertIndex, sprite);
	else
		_collisionSprites.push_back(sprite);
}

void Scene::removeCollisionSprite(Sprite *sprite) {
	for (uint index = 0; index < _collisionSprites.size(); index++) {
		if (_collisionSprites[index] == sprite) {
			_collisionSprites.remove_at(index);
			break;
		}
	}
}

void Scene::clearCollisionSprites() {
	_collisionSprites.clear();
}

void Scene::checkCollision(Sprite *sprite, uint16 flags, int messageNum, uint32 messageParam) {
	for (Common::Array<Sprite*>::iterator iter = _collisionSprites.begin(); iter != _collisionSprites.end(); iter++) {
		Sprite *collSprite = *iter;
		if ((sprite->getFlags() & flags) && collSprite->checkCollision(sprite->getCollisionBounds())) {
			sprite->sendMessage(collSprite, messageNum, messageParam);
		}
	}
}

void Scene::insertMouse(Mouse *mouseCursor) {
	if (_mouseCursor)
		deleteSprite((Sprite**)&_mouseCursor);
	_mouseCursor = mouseCursor;
	addEntity(_mouseCursor);
}

// StaticScene

StaticScene::StaticScene(NeverhoodEngine *vm, Module *parentModule, uint32 backgroundFileHash, uint32 cursorFileHash)
	: Scene(vm, parentModule) {

	SetMessageHandler(&StaticScene::handleMessage);

	setBackground(backgroundFileHash);
	setPalette(backgroundFileHash);
	insertPuzzleMouse(cursorFileHash, 20, 620);
}

uint32 StaticScene::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_MOUSE_CLICK:
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620)
			leaveScene(0);
		break;
	default:
		break;
	}
	return 0;
}

} // End of namespace Neverhood
