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

#include "handler_mgr.h"
#include "handler.h"
#include "handler_timer.h"
#include <pink/archive.h>
#include <common/debug.h>
#include <pink/objects/inventory.h>

namespace Pink {

void HandlerMgr::deserialize(Archive &archive) {
    archive >> _leftClickHandlers >> _useClickHandlers >> _timerHandlers;
}

void HandlerMgr::toConsole() {
    debug("HandlerMgr:");
    for (int i = 0; i < _leftClickHandlers.size(); ++i) {
        _leftClickHandlers[i]->toConsole();
    }
    for (int i = 0; i < _useClickHandlers.size(); ++i) {
        _useClickHandlers[i]->toConsole();
    }
    for (int i = 0; i < _timerHandlers.size(); ++i) {
        _timerHandlers[i]->toConsole();
    }
}

bool HandlerMgr::isLeftClickHandler(Actor *actor) {
    for (int i = 0; i < _leftClickHandlers.size(); ++i) {
        if (_leftClickHandlers[i]->isSuitable(actor))
            return true;
    }

    return false;
}

void HandlerMgr::onTimerMessage(Actor *actor) {
    Handler *handler = findSuitableHandlerTimer(actor);
    if (handler)
        handler->handle(actor);
}

bool HandlerMgr::onLeftClickMessage(Actor *actor) {
    Handler *handler = findSuitableHandlerLeftClick(actor);
    if (handler) {
        handler->handle(actor);
        return true;
    }
    return false;
}

bool HandlerMgr::onUseClickMessage(Actor *actor, InventoryItem *item, InventoryMgr *mgr) {
    HandlerUseClick *handler = (HandlerUseClick*) findSuitableHandlerUseClick(actor);
    if (handler) {
        handler->handle(actor);
        mgr->setItemOwner(handler->getRecepient(), item);
        handler->handle(actor);
        return true;
    }
    return false;
}

Handler *HandlerMgr::findSuitableHandlerTimer(Actor *actor) {
    for (int i = 0; i < _timerHandlers.size(); ++i) {
        if (_timerHandlers[i]->isSuitable(actor))
            return _timerHandlers[i];
    }

    return nullptr;
}

Handler *HandlerMgr::findSuitableHandlerLeftClick(Actor *actor) {
    for (int i = 0; i < _leftClickHandlers.size(); ++i) {
        if (_leftClickHandlers[i]->isSuitable(actor))
            return _leftClickHandlers[i];
    }

    return nullptr;
}

Handler *HandlerMgr::findSuitableHandlerUseClick(Actor *actor) {
    for (int i = 0; i < _useClickHandlers.size(); ++i) {
        if (_useClickHandlers[i]->isSuitable(actor))
            return _useClickHandlers[i];
    }

    return nullptr;
}

HandlerMgr::~HandlerMgr() {
    for (int i = 0; i < _leftClickHandlers.size(); ++i) {
        delete _leftClickHandlers[i];
    }
    for (int j = 0; j < _useClickHandlers.size(); ++j) {
        delete _useClickHandlers[j];
    }
    for (int k = 0; k < _timerHandlers.size(); ++k) {
        delete _timerHandlers[k];
    }
}

}
