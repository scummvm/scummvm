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


#include "handler_timer.h"
#include <common/debug.h>
#include <engines/pink/archive.h>
#include <engines/pink/objects/condition.h>
#include <engines/pink/objects/sequences/sequence.h>
#include <engines/pink/objects/side_effect.h>
#include <engines/pink/objects/actors/lead_actor.h>
#include <engines/pink/objects/pages/game_page.h>
#include <engines/pink/pink.h>


namespace Pink {


void HandlerTimerActions::deserialize(Archive &archive) {
    Handler::deserialize(archive);
    archive >> _actions;
}

void HandlerTimerActions::toConsole() {
    debug("HandlerTimerActions:");

    debug("\tSideEffects:");
    for (int i = 0; i < _sideEffects.size(); ++i) {
        _sideEffects[i]->toConsole();
    }

    debug("\tConditions:");
    for (int i = 0; i < _conditions.size(); ++i) {
        _conditions[i]->toConsole();
    }

    debug("\tActions:");
    for (int i = 0; i < _actions.size(); ++i) {
        debug("\t\t%s", _actions[i].c_str());
    }
}

void HandlerTimerActions::handle(Actor *actor) {
    Handler::handle(actor);
    assert(_actions.size());
    if (!actor->isPlaying()){
        Common::RandomSource &rnd = actor->getPage()->getGame()->getRnd();
        uint index = rnd.getRandomNumber(_actions.size() - 1);
        Action *action = actor->findAction(_actions[index]);
        assert(action);
        actor->setAction(action, 0);
    }
}


void HandlerTimerSequences::execute(Sequence *sequence) {
    debug("HandlerTimerSequences function is not implemented");
}

void HandlerTimerSequences::toConsole() {
    debug("HandlerTimerSequences:");

    debug("\tSideEffects:");
    for (int i = 0; i < _sideEffects.size(); ++i) {
        _sideEffects[i]->toConsole();
    }

    debug("\tConditions:");
    for (int i = 0; i < _conditions.size(); ++i) {
        _conditions[i]->toConsole();
    }

    debug("\tSequences:");
    for (int i = 0; i < _sequences.size(); ++i) {
        debug("\t\t%s", _sequences[i].c_str());
    }
}

} // End of namespace Pink