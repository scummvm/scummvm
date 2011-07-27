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

#include "neverhood/scene.h"
#include "neverhood/collisionman.h"

namespace Neverhood {

Scene::Scene(NeverhoodEngine *vm, Module *parentModule, bool clearHitRects)
	: Entity(vm, 0), _parentModule(parentModule), _dataResource(vm) {
	
	_messageListFlag1 = false;
	_systemCallbackFlag = false;
	_messageList = NULL;
	_rectType = 0;
	_mouseClickPos.x = 0;
	_mouseClickPos.y = 0;
	_mouseClicked = false;
	_rectList = NULL;
	// TODO _someRects = NULL;
	_klayman = NULL;
	_mouseCursor = NULL;
	_palette = NULL;
	_background = NULL;
	// TODO _field_8E = -1;
	if (clearHitRects) {
		_vm->_collisionMan->clearHitRects();
		_vm->_collisionMan->clearSprites();
	}
	_vm->_screen->setFps(24);
	// TODO g_screen->hSmack = NULL;	
	// TODO g_screen->field_24 = 0;
	// TODO g_screen->field_26 = 0;
	// TODO g_screen->resetDirtyRects();	
	_messageListFlag = true;
	_surfaceFlag = false;
	_messageList2 = NULL;
	_smackerPlayer = NULL;
	_smkFileHash = 0;
	_messageListFlag2 = false;
	_messageValue = -1;
	
	SetUpdateHandler(&Scene::update);
	SetMessageHandler(&Scene::handleMessage);
}

Scene::~Scene() {

	if (_palette) {
		removeEntity(_palette);
		delete _palette;
	}

	// Delete all entities
	for (Common::Array<Entity*>::iterator iter = _entities.begin(); iter != _entities.end(); iter++)
		delete *iter;

	// Don't delete surfaces since they always belong to an entity

}

void Scene::draw() {
	if (_smackerPlayer) {
		if (_surfaceFlag) {
			// TODO g_screen->resetDirtyRects();
			// TODO g_screen->copyDirtyRects();
			// TODO g_screen->addDirtyRects();
		}
		if (_smackerPlayer->getSurface())
			_smackerPlayer->getSurface()->draw();
	} else {
#if 0	
		if (_surfaceFlag) {
			// TODO g_screen->copyDirtyRects();
			for (Common::Array<BaseSurface*>::iterator iter = _surfaces.begin(); iter != _surfaces.end(); iter++)
				(*iter)->addDirtyRect();
			// TODO g_screen->addDirtyRects();
		}
#endif		
		for (Common::Array<BaseSurface*>::iterator iter = _surfaces.begin(); iter != _surfaces.end(); iter++) {
			//debug(4, "priority = %d", (*iter)->getPriority());
			(*iter)->draw();
		}
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
	for (uint index = 0; index < _entities.size(); index++) {
		if (_entities[index] == entity) {
			_entities.remove_at(index);
			return true;
		}
	}
	return false; 
}

void Scene::addSurface(BaseSurface *surface) {
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

bool Scene::removeSurface(BaseSurface *surface) {
	for (uint index = 0; index < _surfaces.size(); index++) {
		if (_surfaces[index] == surface) {
			_surfaces.remove_at(index);
			return true;
		}
	}
	return false; 
}

Sprite *Scene::addSprite(Sprite *sprite) {
	addEntity(sprite);
	addSurface(sprite->getSurface());
	return sprite;
}

void Scene::setSurfacePriority(BaseSurface *surface, int priority) {
	surface->setPriority(priority);
	if (removeSurface(surface))
		addSurface(surface);
}

void Scene::deleteSprite(Sprite **sprite) {
	_vm->_collisionMan->removeSprite(*sprite);
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

void Scene::update() {

	if (_smkFileHash != 0) {
		// TODO
		_smackerPlayer = new SmackerPlayer(_vm, this, _smkFileHash, true, 0);
		_savedUpdateHandlerCb = _updateHandlerCb;
		_savedMessageHandlerCb = _messageHandlerCb;
		SetUpdateHandler(&Scene::smackerUpdate);  
		SetMessageHandler(&Scene::smackerHandleMessage);
		_smackerDone = false;
		smackerUpdate();
		// g_screen->smackerPlayer = _smackerPlayer;  
		_smkFileHash = 0;
	} else {
		if (_mouseClicked) {
			if (_klayman) {
				// TODO: Merge later
				if (_klayman->hasMessageHandler() && 
					_klayman->sendMessage(0x1008, 0, this) != 0 &&
					_messageListFlag &&
					queryPositionSprite(_mouseClickPos.x, _mouseClickPos.y)) {
					_mouseClicked = false;
				} else if (_klayman->hasMessageHandler() && 
					_klayman->sendMessage(0x1008, 0, this) != 0 &&
					_messageListFlag) {
					_mouseClicked = !queryPositionRectList(_mouseClickPos.x, _mouseClickPos.y);
				}
			} else if (queryPositionSprite(_mouseClickPos.x, _mouseClickPos.y)) {
				_mouseClicked = false;
			}
		}

		runMessageList();

		// Update all entities		
		for (Common::Array<Entity*>::iterator iter = _entities.begin(); iter != _entities.end(); iter++)
			(*iter)->handleUpdate();
	
	}

}

uint32 Scene::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	switch (messageNum) {
	case 0: // mouse moved
		if (_mouseCursor && _mouseCursor->hasMessageHandler())
			_mouseCursor->sendMessage(0x4002, param, this);
		// TODO queryPositionSomeRects(param.asPoint().x, param.asPoint().y);			
		break;
	case 1: // mouse clicked
		debug("mouse clicked");
		_mouseClicked = true;
		_mouseClickPos = param.asPoint();
		break;
	/* ORIGINAL DEBUG		
	case 3:
		drawSurfaceRects();		
		break;
	*/		
	/* ORIGINAL DEBUG		
	case 4:
		drawRectListRects();		
		break;
	*/		
	case 5:
#if 0	
		broadcastObjectMessage5();		
#endif	
		break;
	case 6:
		_parentModule->sendMessage(0x1009, param, this);		
		break;
	case 0x1006:
		if (_messageListFlag1) {
			_messageListFlag1 = false;
			if (_messageListIndex == _messageListCount)
				_klayman->sendMessage(0x4004, 0, this);
			else {
				runMessageList();
			}
		}
		break;
	case 0x1007:
		if (_messageListFlag1) {
			_messageListFlag1 = false;
			_messageList = NULL;
			_klayman->sendMessage(0x4004, 0, this);
		}
		break;
	case 0x101D:
		if (_mouseCursor) {
			_prevVisible = _mouseCursor->getSurface()->getVisible();
			_mouseCursor->getSurface()->setVisible(false);
		}
		break;
	case 0x101E:
		if (_prevVisible && _mouseCursor) {
			_mouseCursor->getSurface()->setVisible(false);
			// TODO _mouseCursor->sendMessage(0x4002, g_Screen->_mousePos, this);
		}
		break;
	case 0x1022:
		setSurfacePriority(((Sprite*)sender)->getSurface(), param.asInteger());
		break;
	}
	return 0;
}

void Scene::smackerUpdate() {
	//**ALL TODO
#if 0	
	_smackerPlayer->handleUpdate();
	if (_smackerDone) {
		delete _smackerPlayer;
		_smackerPlayer = NULL;
		_updateHandlerCb = _savedUpdateHandlerCb;
		_messageHandlerCb = _savedMessageHandlerCb;
		if (_palette)
			_palette->usePalette();
		// TODO _background->restore();
		// TODO g_screen->smackerPlayer = NULL;		
	}
#endif	
}

uint32 Scene::smackerHandleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	switch (messageNum) {
	case 0x3002:
		_smackerDone = true;
		break;
	}
	return 0;
}

bool Scene::queryPositionSprite(int16 mouseX, int16 mouseY) {
	debug("Scene::queryPositionSprite(%d, %d)", mouseX, mouseY);
	for (uint i = 0; i < _vm->_collisionMan->getSpriteCount(); i++) {
		Sprite *sprite = _vm->_collisionMan->getSprite(i);
		if (sprite->hasMessageHandler() && sprite->isPointInside(mouseX, mouseY) && 
			sprite->sendPointMessage(0x1011, _mouseClickPos, this) != 0) {
			return true;
		}
	}
	return false;
}

bool Scene::queryPositionRectList(int16 mouseX, int16 mouseY) {
	if (_rectType == 1) {
		RectList &rectList = *_rectList;
		int16 klaymanX = _klayman->getX();
		int16 klaymanY = _klayman->getY();
		for (uint i = 0; i < rectList.size(); i++) {
			debug("(%d, %d) ? (%d, %d, %d, %d)", klaymanX, klaymanY, rectList[i].rect.x1, rectList[i].rect.y1, rectList[i].rect.x2, rectList[i].rect.y2);
			if (klaymanX >= rectList[i].rect.x1 && klaymanX <= rectList[i].rect.x2 && 
				klaymanY >= rectList[i].rect.y1 && klaymanY <= rectList[i].rect.y2) {
				for (uint j = 0; j < rectList[i].subRects.size(); j++) {
					debug("  (%d, %d) ? (%d, %d, %d, %d)", mouseX, mouseY, rectList[i].subRects[j].rect.x1, rectList[i].subRects[j].rect.y1, rectList[i].subRects[j].rect.x2, rectList[i].subRects[j].rect.y2);
					if (mouseX >= rectList[i].subRects[j].rect.x1 && mouseX <= rectList[i].subRects[j].rect.x2 && 
						mouseY >= rectList[i].subRects[j].rect.y1 && mouseY <= rectList[i].subRects[j].rect.y2) {
						debug("Scene::queryPositionRectList() -> %08X", rectList[i].subRects[j].messageListId);
						return setMessageList2(rectList[i].subRects[j].messageListId);
					}
				}
			}
		}
	}
	return true;
}

void Scene::setMessageList(uint32 id, bool messageListFlag, bool systemCallbackFlag) {
	setMessageList(_vm->_staticData->getMessageList(id), messageListFlag, systemCallbackFlag);
}

void Scene::setMessageList(MessageList *messageList, bool messageListFlag, bool systemCallbackFlag) {
	debug("Scene::setMessageList(%p)", (void*)messageList);
	_messageList = messageList;
	_messageListCount = _messageList ? _messageList->size() : 0;
	_messageListIndex = 0;
	_messageListFlag1 = false;
	_messageListFlag = messageListFlag;
	_systemCallbackFlag = systemCallbackFlag;
	_messageListStatus = 1;
	_klayman->sendMessage(0x101C, 0, this);
	
	// DEBUG: Show message list
	for (uint i = 0; i < messageList->size(); i++) {
		debug("A: %02d: %04X, %08X", i, (*messageList)[i].messageNum, (*messageList)[i].messageValue);
	}
	debug("A: ================================================================");
	
}

bool Scene::setMessageList2(uint32 id, bool messageListFlag, bool systemCallbackFlag) {
	return setMessageList2(_vm->_staticData->getMessageList(id), messageListFlag, systemCallbackFlag);
}

bool Scene::setMessageList2(MessageList *messageList, bool messageListFlag, bool systemCallbackFlag) {
	bool result = false;
	
	debug("Scene::setMessageList2(%p)", (void*)messageList);

	// DEBUG: Show message list
	for (uint i = 0; i < messageList->size(); i++) {
		debug("B: %02d: %04X, %08X", i, (*messageList)[i].messageNum, (*messageList)[i].messageValue);
	}
	debug("B: ================================================================");
	
	if (_messageListStatus == 1) {
		if (messageList != _messageList2) {
			if (_messageValue >= 0) {
				_parentModule->sendMessage(0x1023, _messageValue, this);
				_messageValue = -1;
			}
			_messageList2 = messageList;
			setMessageList(messageList, messageListFlag, systemCallbackFlag);
			result = true;
		}
	} else if (_messageListStatus == 2) {
		if (messageList == _messageList2) {
			if (_messageValue >= 0) {
				_parentModule->sendMessage(0x1023, _messageValue, this);
				_messageValue = -1;
			}
			_messageList2 = messageList;
			setMessageList(messageList, messageListFlag, systemCallbackFlag);
			result = true;
		}
	} else {
		if (_messageValue >= 0) {
			_parentModule->sendMessage(0x1023, _messageValue, this);
			_messageValue = -1;
		}
		_messageList2 = messageList;
		setMessageList(messageList, messageListFlag, systemCallbackFlag);
		result = true;
	}
	return result;
}

void Scene::runMessageList() {
	debug(7, "Scene::runMessageList() _messageListFlag2 = %d; _messageListFlag1 = %d", _messageListFlag2, _messageListFlag1);

	if (_messageListFlag2 || _messageListFlag1)
		return;

	_messageListFlag2 = true;

	if (!_messageList) {
		_messageList2 = NULL;
		_messageListStatus = 0;
	}
	
	if (_messageList && _klayman) {
	
		while (_messageList && _messageListIndex < _messageListCount && !_messageListFlag1) {
			int messageNum = (*_messageList)[_messageListIndex].messageNum;
			uint32 messageParam = (*_messageList)[_messageListIndex].messageValue;
			
			debug("Scene::runMessageList() %04X, %08X", messageNum, messageParam);
			
			_messageListIndex++;
			if (_messageListIndex == _messageListCount) {
				_klayman->sendMessage(0x1021, 0, this);
			}
			if (_systemCallbackFlag) {
				// TODO messageNum = systemConvertMessageCb(messageNum);
			}
			if (messageNum != 0x4003) {
				if (messageNum == 0x1009 || messageNum == 0x1024) {
					_parentModule->sendMessage(messageNum, messageParam, this);
				} else if (messageNum == 0x100A) {
					_messageValue = messageParam;
					_parentModule->sendMessage(messageNum, messageParam, this);
				} else if (messageNum == 0x4001) {
					_messageListFlag1 = true;
					_klayman->sendPointMessage(0x4001, _mouseClickPos, this);
				} else if (messageNum == 0x100D) {
					if (this->hasMessageHandler() && this->sendMessage(0x100D, messageParam, this) != 0)
						continue;
				} else if (messageNum == 0x101A) {
					_messageListStatus = 0;
				} else if (messageNum == 0x101B) {
					_messageListStatus = 2;
				} else if (messageNum == 0x1020) {
					_messageListFlag = false;
				} else if (messageNum >= 0x2000 && messageNum <= 0x2FFF) {
					if (this->hasMessageHandler() && this->sendMessage(messageNum, messageParam, this) != 0) {
						_messageListFlag2 = false;
						return;
					}
				} else {
					_messageListFlag1 = true;
					if (_klayman->hasMessageHandler() && _klayman->sendMessage(messageNum, messageParam, this) != 0) {
						_messageListFlag1 = false;
					}
				} 
			}
			if (_messageListIndex == _messageListCount) {
				_messageListFlag = true;
				_messageList = NULL;
			}
		}
	}

	_messageListFlag2 = false;
	
}

void Scene::messageList402220() {
	_messageListFlag1 = false;
	_messageList = NULL;
	_messageListFlag = true;
	_klayman->sendMessage(0x4004, 0, this);
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

void Scene::loadDataResource(uint32 fileHash) {
	_dataResource.load(fileHash);
	if (_klayman)
		_klayman->loadDataResource(fileHash);
}

} // End of namespace Neverhood
