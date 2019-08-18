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

#include "bladerunner/actor_dialogue_queue.h"

#include "bladerunner/bladerunner.h"

#include "bladerunner/actor.h"
#include "bladerunner/audio_speech.h"
#include "bladerunner/savefile.h"
#include "bladerunner/scene.h"
#include "bladerunner/time.h"

#include "bladerunner/script/scene_script.h"

namespace BladeRunner {

ActorDialogueQueue::Entry::Entry() {
	isNotPause = false;
	isPause = false;
	actorId = -1;
	delay = -1;
	sentenceId = -1;
	animationMode = -1;
}

ActorDialogueQueue::ActorDialogueQueue(BladeRunnerEngine *vm) {
	_vm = vm;
	clear();
}

ActorDialogueQueue::~ActorDialogueQueue() {
}

void ActorDialogueQueue::add(int actorId, int sentenceId, int animationMode) {
	if (actorId == kActorMcCoy || actorId == kActorVoiceOver) {
		animationMode = -1;
	}
	if (_entries.size() < kMaxEntries) {
		Entry entry;
		entry.isNotPause = true;
		entry.isPause = false;
		entry.actorId = actorId;
		entry.sentenceId = sentenceId;
		entry.animationMode = animationMode;
		entry.delay = -1;

		_entries.push_back(entry);
	}
}

void ActorDialogueQueue::addPause(int32 delay) {
	if (_entries.size() < kMaxEntries) {
		Entry entry;
		entry.isNotPause = false;
		entry.isPause = true;
		entry.actorId = -1;
		entry.sentenceId = -1;
		entry.animationMode = -1;
		entry.delay = delay;

		_entries.push_back(entry);
	}
}

void ActorDialogueQueue::flush(int a1, bool callScript) {
	if (_isNotPause && _vm->_audioSpeech->isPlaying()) {
		_vm->_audioSpeech->stopSpeech();
		if (_animationModePrevious >= 0) {
			_vm->_actors[_actorId]->changeAnimationMode(_animationModePrevious, false);
			_animationModePrevious = -1;
		}
		_isNotPause = false;
		_actorId = -1;
		_sentenceId = -1;
		_animationMode = -1;
	}
	if (_isPause) {
		_isPause = false;
		_delay = 0;
		_timeLast = 0u;
	}
	clear();
	if (callScript) {
		_vm->_sceneScript->dialogueQueueFlushed(a1);
	}
}

/**
* return true when queue is empty and object is flushed
*/
bool ActorDialogueQueue::isEmpty() {
	return _entries.empty() \
	        && !_isNotPause \
	        && !_isPause \
	        && _actorId == -1 \
	        && _sentenceId == -1 \
	        && _animationMode == -1 \
	        && _animationModePrevious == -1 \
	        && _delay == 0 \
	        && _timeLast == 0u;
}

void ActorDialogueQueue::tick() {
	if (!_vm->_audioSpeech->isPlaying()) {
		if (_isPause) {
			uint32 time = _vm->_time->current();
			uint32 timeDiff = time - _timeLast; // unsigned difference is intentional
			_timeLast = time;
			_delay = (_delay < 0 || ((uint32)_delay < timeDiff) ) ? 0 : ((uint32)_delay - timeDiff);
			if (_delay > 0) {
				return;
			}
			_isPause = false;
			_delay = 0;
			_timeLast = 0u;
			if (_entries.empty()) {
				flush(0, true);
			}
		}
		if (_isNotPause) {
			if (_animationModePrevious >= 0) {
				_vm->_actors[_actorId]->changeAnimationMode(_animationModePrevious, false);
				_animationModePrevious = -1;
			}
			_isNotPause = false;
			_actorId = -1;
			_sentenceId = -1;
			_animationMode = -1;
			if (_entries.empty()) {
				flush(0, true);
			}
		}
		if (!_entries.empty()) {
			Entry firstEntry = _entries.remove_at(0);
			if (firstEntry.isNotPause) {
				_animationMode = firstEntry.animationMode;
				if (_vm->_actors[firstEntry.actorId]->getSetId() != _vm->_scene->getSetId()) {
					_animationMode = -1;
				}
				_vm->_actors[firstEntry.actorId]->speechPlay(firstEntry.sentenceId, false);
				_isNotPause = true;
				_actorId = firstEntry.actorId;
				_sentenceId = firstEntry.sentenceId;
				if (_animationMode >= 0) {
					_animationModePrevious = _vm->_actors[firstEntry.actorId]->getAnimationMode();
					_vm->_actors[firstEntry.actorId]->changeAnimationMode(_animationMode, false);
				} else {
					_animationModePrevious = -1;
				}
			} else if (firstEntry.isPause) {
				_isPause = true;
				_delay = firstEntry.delay;
				_timeLast = _vm->_time->current();
			}
		}
	}
}

void ActorDialogueQueue::save(SaveFileWriteStream &f) {
	int count = (int)_entries.size();
	f.writeInt(count);
	for (int i = 0; i < count; ++i) {
		Entry &e = _entries[i];
		f.writeBool(e.isNotPause);
		f.writeBool(e.isPause);
		f.writeInt(e.actorId);
		f.writeInt(e.sentenceId);
		f.writeInt(e.animationMode);
		f.writeInt(e.delay);
	}
	f.padBytes((kMaxEntries - count) * 24);

	f.writeBool(_isNotPause);
	f.writeInt(_actorId);
	f.writeInt(_sentenceId);
	f.writeInt(_animationMode);
	f.writeInt(_animationModePrevious);
	f.writeBool(_isPause);
	f.writeInt(_delay);
	// f.write(_timeLast);
}

void ActorDialogueQueue::load(SaveFileReadStream &f) {
	_entries.clear();
	uint count = f.readInt();
	assert(count <= kMaxEntries);
	_entries.resize(count);
	for (uint i = 0; i < count; ++i) {
		Entry &e = _entries[i];
		e.isNotPause = f.readBool();
		e.isPause = f.readBool();
		e.actorId = f.readInt();
		e.sentenceId = f.readInt();
		e.animationMode = f.readInt();
		e.delay = f.readInt();
	}

	f.skip((kMaxEntries - count) * 24);

	_isNotPause = f.readBool();
	_actorId = f.readInt();
	_sentenceId = f.readInt();
	_animationMode = f.readInt();
	_animationModePrevious = f.readInt();
	_isPause = f.readBool();
	_delay = f.readInt();
	_timeLast = 0u;
}

void ActorDialogueQueue::clear() {
	_entries.clear();
	_isNotPause = false;
	_actorId = -1;
	_sentenceId = -1;
	_animationMode = -1;
	_animationModePrevious = -1;
	_isPause = false;
	_delay = 0;
	_timeLast = 0u;
}

} // End of namespace BladeRunner
