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
#include "pink/objects/side_effect.h"
#include "pink/objects/condition.h"
#include "pink/objects/actors/actor.h"
#include "pink/objects/handlers/handler.h"
#include "pink/objects/pages/game_page.h"
#include "pink/objects/sequences/sequencer.h"
#include "pink/objects/sequences/sequence.h"

namespace Pink {

void Handler::deserialize(Archive &archive) {
	_conditions.deserialize(archive);
	_sideEffects.deserialize(archive);
}

bool Handler::isSuitable(Actor *actor) {
	for (uint i = 0; i < _conditions.size(); ++i) {
		if (!_conditions[i]->evaluate(actor))
			return false;
	}
	return true;
}

void Handler::executeSideEffects(Actor *actor) {
	for (uint i = 0; i < _sideEffects.size(); ++i) {
		_sideEffects[i]->execute(actor);
	}
}

void Handler::handle(Actor *actor) {
	executeSideEffects(actor);
}

Handler::~Handler() {
	for (uint i = 0; i < _sideEffects.size(); ++i) {
		delete _sideEffects[i];
	}
	for (uint i = 0; i < _conditions.size(); ++i) {
		delete _conditions[i];
	}
}

void HandlerSequences::deserialize(Archive &archive) {
	Handler::deserialize(archive);
	_sequences.deserialize(archive);
}

void HandlerSequences::handle(Actor *actor) {
	Handler::handle(actor);
	Sequencer *sequencer = actor->getPage()->getSequencer();

	assert(!_sequences.empty());

	Common::RandomSource &rnd = actor->getPage()->getGame()->getRnd();
	uint index = rnd.getRandomNumber(_sequences.size() - 1);

	Sequence *sequence = sequencer->findSequence(_sequences[index]);

	assert(sequence);
	sequencer->authorSequence(sequence, 0);

	execute(sequence);
}

void HandlerStartPage::execute(Sequence *sequence) {
	sequence->allowSkipping();
}

void HandlerStartPage::toConsole() {
	debugC(6, kPinkDebugLoadingObjects, "HandlerStartPage:");

	debugC(6, kPinkDebugLoadingObjects, "\tSideEffects:");
	for (uint i = 0; i < _sideEffects.size(); ++i) {
		_sideEffects[i]->toConsole();
	}

	debugC(6, kPinkDebugLoadingObjects, "\tConditions:");
	for (uint i = 0; i < _conditions.size(); ++i) {
		_conditions[i]->toConsole();
	}

	debugC(6, kPinkDebugLoadingObjects, "\tSequences:");
	for (uint i = 0; i < _sequences.size(); ++i) {
		debugC(6, kPinkDebugLoadingObjects, "\t\t%s", _sequences[i].c_str());
	}
}

void HandlerLeftClick::toConsole() {
	debugC(6, kPinkDebugLoadingObjects, "HandlerLeftClick:");

	debugC(6, kPinkDebugLoadingObjects, "\tSideEffects:");
	for (uint i = 0; i < _sideEffects.size(); ++i) {
		_sideEffects[i]->toConsole();
	}

	debugC(6, kPinkDebugLoadingObjects, "\tConditions:");
	for (uint i = 0; i < _conditions.size(); ++i) {
		_conditions[i]->toConsole();
	}

	debugC(6, kPinkDebugLoadingObjects, "\tSequences:");
	for (uint i = 0; i < _sequences.size(); ++i) {
		debugC(6, kPinkDebugLoadingObjects, "\t\t%s", _sequences[i].c_str());
	}
}

void HandlerUseClick::deserialize(Archive &archive) {
	HandlerSequences::deserialize(archive);
	_inventoryItem = archive.readString();
	_recepient = archive.readString();
}

void HandlerUseClick::toConsole() {
	debugC(6, kPinkDebugLoadingObjects, "HandlerUseClick: _inventoryItem=%s, _recepient=%s", _inventoryItem.c_str(), _recepient.c_str());
	debugC(6, kPinkDebugLoadingObjects, "\tSideEffects:");
	for (uint i = 0; i < _sideEffects.size(); ++i) {
		_sideEffects[i]->toConsole();
	}

	debugC(6, kPinkDebugLoadingObjects, "\tConditions:");
	for (uint i = 0; i < _conditions.size(); ++i) {
		_conditions[i]->toConsole();
	}

	debugC(6, kPinkDebugLoadingObjects, "\tSequences:");
	for (uint i = 0; i < _sequences.size(); ++i) {
		debugC(6, kPinkDebugLoadingObjects, "\t\t%s", _sequences[i].c_str());
	}
}

} // End of namespace Pink
