//
// Created by andrei on 3/21/18.
//

#include "handler_mgr.h"
#include "handler.h"
#include "handler_timer.h"
#include <pink/archive.h>
#include <common/debug.h>

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
        return 1;
    }
    return 0;
}

bool HandlerMgr::onUseClickMessage(Actor *actor) {
    Handler *handler = findSuitableHandlerUseClick(actor);
    if (handler) {
        handler->handle(actor);
        return 1;
    }
    return 0;
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
