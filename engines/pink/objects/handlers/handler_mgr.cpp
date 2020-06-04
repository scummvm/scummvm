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

#include "pink/archive.h"
#include "pink/pink.h"
#include "pink/objects/inventory.h"
#include "pink/objects/handlers/handler.h"
#include "pink/objects/handlers/handler_mgr.h"

namespace Pink {

void HandlerMgr::deserialize(Archive &archive) {
	_leftClickHandlers.deserialize(archive);
	_useClickHandlers.deserialize(archive);
	_timerHandlers.deserialize(archive);
}

void HandlerMgr::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "HandlerMgr:");
	for (uint i = 0; i < _leftClickHandlers.size(); ++i) {
		_leftClickHandlers[i]->toConsole();
	}
	for (uint i = 0; i < _useClickHandlers.size(); ++i) {
		_useClickHandlers[i]->toConsole();
	}
	for (uint i = 0; i < _timerHandlers.size(); ++i) {
		_timerHandlers[i]->toConsole();
	}
}

void HandlerMgr::onTimerMessage(Actor *actor) {
	Handler *handler = findSuitableHandlerTimer(actor);
	if (handler)
		handler->handle(actor);
}

void HandlerMgr::onLeftClickMessage(Actor *actor) {
	Handler *handler = findSuitableHandlerLeftClick(actor);
	assert(handler);
	handler->handle(actor);
}

void HandlerMgr::onUseClickMessage(Actor *actor, InventoryItem *item, InventoryMgr *mgr) {
	HandlerUseClick *handler = findSuitableHandlerUseClick(actor, item->getName());
	assert(handler);
	if (!handler->getRecepient().empty())
		mgr->setItemOwner(handler->getRecepient(), item);
	handler->handle(actor);
}

Handler *HandlerMgr::findSuitableHandlerTimer(const Actor *actor) {
	for (uint i = 0; i < _timerHandlers.size(); ++i) {
		if (_timerHandlers[i]->isSuitable(actor))
			return _timerHandlers[i];
	}

	return nullptr;
}

HandlerLeftClick *HandlerMgr::findSuitableHandlerLeftClick(const Actor *actor) const {
	for (uint i = 0; i < _leftClickHandlers.size(); ++i) {
		if (_leftClickHandlers[i]->isSuitable(actor))
			return _leftClickHandlers[i];
	}

	return nullptr;
}

HandlerUseClick *HandlerMgr::findSuitableHandlerUseClick(const Actor *actor, const Common::String &itemName) const {
	for (uint i = 0; i < _useClickHandlers.size(); ++i) {
		if (itemName == _useClickHandlers[i]->getInventoryItem() && _useClickHandlers[i]->isSuitable(actor))
			return _useClickHandlers[i];
	}

	return nullptr;
}

HandlerMgr::~HandlerMgr() {
	for (uint i = 0; i < _leftClickHandlers.size(); ++i) {
		delete _leftClickHandlers[i];
	}
	for (uint j = 0; j < _useClickHandlers.size(); ++j) {
		delete _useClickHandlers[j];
	}
	for (uint k = 0; k < _timerHandlers.size(); ++k) {
		delete _timerHandlers[k];
	}
}

} // End of namespace Pink
