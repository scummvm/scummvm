//
// Created by andrei on 3/21/18.
//

#include "handler_mgr.h"
#include "handler.h"
#include "handler_timer.h"
#include <pink/archive.h>
#include <common/debug.h>

namespace Pink {

void HandlerMgr::deserialize(Pink::Archive &archive) {
    archive >> _leftClickHandlers >> _useClickHandlers >> _timerHandlers;
}

void HandlerMgr::toConsole() {
    debug("HandlerMgr:");
    for (int i = 0; i < _leftClickHandlers.size(); ++i) {
        _leftClickHandlers[i]->toConsole();
    }
    for (int i = 0; i < _leftClickHandlers.size(); ++i) {
        _useClickHandlers[i]->toConsole();
    }
    for (int i = 0; i < _leftClickHandlers.size(); ++i) {
        _timerHandlers[i]->toConsole();
    }
}

}
