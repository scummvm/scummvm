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

#include "seq_timer.h"
#include <engines/pink/archive.h>
#include "./sequencer.h"
#include <common/debug.h>
#include <engines/pink/objects/actors/supporting_actor.h>
#include "pink/objects/pages/game_page.h"
#include "pink/pink.h"

namespace Pink {

SeqTimer::SeqTimer()
        : _updatesToMessage(0) {

}

void SeqTimer::deserialize(Archive &archive) {
    archive >> _actor;
    _period = archive.readDWORD();
    _range = archive.readDWORD();
    _sequencer = static_cast<Sequencer*>(archive.readObject());
}

void SeqTimer::toConsole() {
    debug("\tSeqTimer: _actor=%s _period=%u _range=%u", _actor.c_str(), _period, _range);
}

void SeqTimer::update() {
    Common::RandomSource &random =_sequencer->_page->getGame()->getRnd();
    if (_updatesToMessage--)
        return;

    calculateUpdatesCount();
    SupportingActor *actor = static_cast<SupportingActor*>(_sequencer->_page->findActor(_actor));
    if (!_sequencer->findSequenceActorState(actor->getName())){
        actor->onTimerMessage();
    }
}

void SeqTimer::calculateUpdatesCount() {
    Common::RandomSource &random =_sequencer->_page->getGame()->getRnd();
    _updatesToMessage = _range ? _period + random.getRandomNumber(_range) : _period;
}

} // End of namespace Pink