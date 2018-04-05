//
// Created by andrei on 3/21/18.
//

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

}
