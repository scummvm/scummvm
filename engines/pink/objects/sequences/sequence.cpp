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
#include "pink/sound.h"
#include "pink/objects/actors/actor.h"
#include "pink/objects/pages/game_page.h"
#include "pink/objects/sequences/sequence.h"
#include "pink/objects/sequences/sequence_context.h"
#include "pink/objects/sequences/sequencer.h"

namespace Pink {

Sequence::Sequence()
		: _canBeSkipped(0), _context(nullptr),
		  _sequencer(nullptr) {}

Sequence::~Sequence() {
	for (uint i = 0; i < _items.size(); ++i) {
		delete _items[i];
	}
}

void Sequence::deserialize(Archive &archive) {
	NamedObject::deserialize(archive);
	_sequencer = static_cast<Sequencer *>(archive.readObject());
	_items.deserialize(archive);
}

void Sequence::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\t\tSequence %s", _name.c_str());
	debugC(6, kPinkDebugLoadingObjects, "\t\t\tItems:");
	for (uint i = 0; i < _items.size(); ++i) {
		_items[i]->toConsole();
	}
}

void Sequence::start(bool loadingSave) {
	uint nextItemIndex = _context->getNextItemIndex();
	if (nextItemIndex >= _items.size() ||
		!_items[nextItemIndex]->execute(_context->getSegment(), this, loadingSave)) {
		debugC(6, kPinkDebugScripts, "Sequence %s ended", _name.c_str());
		end();
		return;
	}

	uint i = nextItemIndex + 1;
	while (i < _items.size()) {
		if (_items[i]->isLeader()) {
			break;
		}
		_items[i++]->execute(_context->getSegment(), this, loadingSave);
	}

	_context->execute(i, loadingSave);
}

void Sequence::update() {
	if (!_context->getActor()->isPlaying()) {
		debugC(6, kPinkDebugScripts, "SubSequence of %s Sequence ended", _name.c_str());
		start(0);
	}
}

void Sequence::end() {
	_context->setActor(nullptr);
	_canBeSkipped = 1;
	_sequencer->removeContext(_context);
}

void Sequence::restart() {
	_context->setNextItemIndex(0);
	_context->clearDefaultActions();
	start(0);
}

void Sequence::skip() {
	if (_context->getNextItemIndex() >= _items.size())
		return;

	for (int i = _items.size() - 1; i >= 0; --i) {
		if (_items[i]->isLeader()) {
			_context->setNextItemIndex(i);
			_context->clearDefaultActions();
			for (int j = 0; j < i; ++j) {
				_items[j]->skip(this);
			}
			start(0);
			break;
		}
	}
}

void Sequence::skipSubSequence() {
	if (_context->getNextItemIndex() < _items.size())
		this->start(0);
}

void Sequence::forceEnd() {
	skip();
	end();
}

void Sequence::init(bool loadingSave) {
	start(loadingSave);
}

void SequenceAudio::deserialize(Archive &archive) {
	Sequence::deserialize(archive);
	_soundName = archive.readString();
}

void SequenceAudio::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\t\tSequenceAudio %s : _sound = %s", _name.c_str(), _soundName.c_str());
	debugC(6, kPinkDebugLoadingObjects, "\t\t\tItems:");
	for (uint i = 0; i < _items.size(); ++i) {
		_items[i]->toConsole();
	}
}

void SequenceAudio::start(bool loadingSave) {
	Sequence::start(loadingSave);
	uint index = _context->getNextItemIndex();
	if (index < _items.size()) {
		_leader = (SequenceItemLeaderAudio *)_items[index];
	} else {
		_leader = nullptr;
	}
}

void SequenceAudio::end() {
	_sound.stop();
	Sequence::end();
}

void SequenceAudio::update() {
	if (!_sound.isPlaying())
		end();
	else if (_leader && _leader->getSample() <= _sound.getCurrentSample())
		start(0);
}

void SequenceAudio::init(bool loadingSave) {
	_leader = nullptr;
	_sound.play(_sequencer->getPage()->getResourceStream(_soundName), Audio::Mixer::kMusicSoundType);
	start(loadingSave);
}

void SequenceAudio::restart() {
	_leader = nullptr;
	_sound.play(_sequencer->getPage()->getResourceStream(_soundName), Audio::Mixer::kMusicSoundType);
	Sequence::restart();
}

void SequenceAudio::skip() {
	end();
}

} // End of namespace Pink
