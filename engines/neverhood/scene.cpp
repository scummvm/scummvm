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

namespace Neverhood {

Scene::Scene(NeverhoodEngine *vm, Module *parentModule, bool clearHitRects)
	: Entity(vm, 0), _parentModule(parentModule) {
	
	_messageListFlag1 = false;
	_systemCallbackFlag = false;
	_messageList = NULL;
	// TODO _rectType = 0;
	_mouseClickPos.x = 0;
	_mouseClickPos.y = 0;
	_mouseClicked = false;
	// TODO _rectList = NULL;
	// TODO _someRects = NULL;
	// TODO _playerSprite = NULL;
	// TODO _mouseSprite = NULL;
	_palette = NULL;
	_background = NULL;
	// TODO _field_8E = -1;
	if (clearHitRects) {
		// TODO g_Class700->setHitRects(NULL, 0);
		// TODO g_Class700->clear();
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
		if (_surfaceFlag) {
			// TODO g_screen->copyDirtyRects();
			for (Common::Array<BaseSurface*>::iterator iter = _surfaces.begin(); iter != _surfaces.end(); iter++)
				(*iter)->addDirtyRect();
			// TODO g_screen->addDirtyRects();
		}
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
	// TODO g_Class700->removeSprite(*sprite);
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
		//**** ALL TODO
		//_smackerPlayer = new SmackerPlayer(this, _smkFileHash, true, 0);
		_savedUpdateHandlerCb = _updateHandlerCb;
		_savedMessageHandlerCb = _messageHandlerCb;
		SetUpdateHandler(&Scene::smackerUpdate);  
		SetMessageHandler(&Scene::smackerHandleMessage);
		_smackerDone = false;
		// smackerUpdate();
		// g_screen->smackerPlayer = _smackerPlayer;  
		_smkFileHash = 0;
	} else {
		if (_mouseClicked) {
			//** ALL TODO
#if 0			
			if (_playerSprite) {
				// TODO: Merge later
				if (_playerSprite->hasMessageHandler() && 
					_playerSprite->sendMessage(0x1008, 0, this) != 0 &&
					_messageListFlag &&
					queryPositionClass400(_mouseClickPos.x, _mouseClickPos.y)) {
					_mouseClicked = false;
				} else if (_playerSprite->hasMessageHandler() && 
					_playerSprite->sendMessage(0x1008, 0, this) != 0 &&
					_messageListFlag) {
					_mouseClicked = !queryPositionRectList(_mouseClickPos.x, _mouseClickPos.y);
				}
			} else if (queryPositionClass400(_mouseClickPos.x, _mouseClickPos.y)) {
				_mouseClicked = false;
			}
#endif			
		}
	
		// TODO runMessageList();

		// Update all entities		
		for (Common::Array<Entity*>::iterator iter = _entities.begin(); iter != _entities.end(); iter++)
			(*iter)->handleUpdate();
	
	}

}

uint32 Scene::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	// TODO
#if 0	
	switch (messageNum) {
	case 0: // mouse moved
		if (_mouseSprite && _mouseSprite->hasMessageHandler())
			_mouseSprite->sendMessage(0x4002, param, this);
		queryPositionSomeRects(param._point.x, param._point.y);			
		break;
	case 1: // mouse clicked
		_mouseClicked = true;
		_mouseClickPos = param._point;
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
		broadcastObjectMessage5();		
		break;
	case 6:
		_parentModule->sendMessage(0x1009, param, this);		
		break;
	case 0x1006:
		if (_messageListFlag1) {
			_messageListFlag1 = false;
			if (_messageListIndex == _messageListCount)
				_playerSprite->sendMessage(0x4004, 0, this);
			else
				runMessageList();
		}
		break;
	case 0x1007:
		if (_messageListFlag1) {
			_messageListFlag1 = false;
			_messageList = NULL;
			_playerSprite->sendMessage(0x4004, 0, this);
		}
		break;
	case 0x101D:
		if (_mouseSprite) {
			_prevVisible = _mouseSprite->_drawSurface->_visible;
			_mouseSprite->_drawSurface->_visible = false;
		}
		break;
	case 0x101E:
		if (_prevVisible && _mouseSprite) {
			_mouseSprite->_drawSurface->_visible = true;
			_mouseSprite->sendMessage(0x4002, g_Screen->_mousePos, this);
		}
		break;
	case 0x1022:
		setSurfacePriority(((Sprite*)sender)->_surface, param._integer);
		break;
	}
#endif	
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
	if (messageNum == 0x3002)
		_smackerDone = true;
	return 0;
}

} // End of namespace Neverhood
