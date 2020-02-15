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

#include "common/debug.h"

#include "pink/archive.h"
#include "pink/pink.h"
#include "pink/objects/actors/supporting_actor.h"
#include "pink/objects/pages/game_page.h"
#include "pink/objects/sequences/seq_timer.h"
#include "pink/objects/sequences/sequencer.h"

namespace Pink {

SeqTimer::SeqTimer()
	: _sequencer(nullptr), _updatesToMessage(0), _period(0),
	_range(0) {}

void SeqTimer::deserialize(Archive &archive) {
	_actor = archive.readString();
	_period = archive.readDWORD();
	_range = archive.readDWORD();
	_sequencer = static_cast<Sequencer *>(archive.readObject());
}

void SeqTimer::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\tSeqTimer: _actor=%s _period=%u _range=%u", _actor.c_str(), _period, _range);
}

void SeqTimer::update() {
	Page *page = _sequencer->getPage();
	Common::RandomSource &rnd = page->getGame()->getRnd();
	if (_updatesToMessage--)
		return;

	_updatesToMessage = _range ? _period + rnd.getRandomNumber(_range) : _period;

	Actor *actor = page->findActor(_actor);
	if (actor && !_sequencer->findState(_actor)) {
		actor->onTimerMessage();
	}
}

} // End of namespace Pink
