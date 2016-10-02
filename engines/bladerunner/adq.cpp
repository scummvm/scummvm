#include "bladerunner/adq.h"

#include "bladerunner/bladerunner.h"

#include "bladerunner/actor.h"
#include "bladerunner/audio_speech.h"
#include "bladerunner/scene.h"

#include "script/script.h"

namespace BladeRunner {

ADQ::ADQEntry::ADQEntry() {
	this->_isNotPause = false;
	this->_isPause = false;
	this->_actorId = -1;
	this->_delay = -1;
	this->_sentenceId = -1;
	this->_animationMode = -1;
}

ADQ::ADQ(BladeRunnerEngine *vm) {
	_vm = vm;
	clear();
}

ADQ::~ADQ() {
}

void ADQ::add(int actorId, int sentenceId, int animationMode) {
	if (actorId == 0 || actorId == 99) {
		animationMode = -1;
	}
	if (_entries.size() < 25) {
		ADQEntry entry;
		entry._isNotPause = true;
		entry._isPause = false;
		entry._actorId = actorId;
		entry._sentenceId = sentenceId;
		entry._animationMode = animationMode;
		entry._delay = -1;

		_entries.push_back(entry);
	}
}

void ADQ::addPause(int delay) {
	if (_entries.size() < 25) {
		ADQEntry entry;
		entry._isNotPause = false;
		entry._isPause = true;
		entry._actorId = -1;
		entry._sentenceId = -1;
		entry._animationMode = -1;
		entry._delay = delay;

		_entries.push_back(entry);
	}
}

void ADQ::flush(int a1, bool callScript) {
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
		_timeLast = 0;
	}
	clear();
	if (callScript) {
		_vm->_script->DialogueQueueFlushed(a1);
	}
}

void ADQ::tick() {
	if (!_vm->_audioSpeech->isPlaying()) {
		if (_isPause) {
			int time = _vm->getTotalPlayTime();
			int timeDiff = time - _timeLast;
			_timeLast = time;
			_delay -= timeDiff;
			if (_delay > 0) {
				return;
			}
			_isPause = false;
			_delay = 0;
			_timeLast = 0;
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
			ADQEntry firstEntry = _entries.remove_at(0);
			if (firstEntry._isNotPause) {
				_animationMode = firstEntry._animationMode;
				if (_vm->_actors[firstEntry._actorId]->getSetId() != _vm->_scene->getSetId()) {
					_animationMode = -1;
				}
				_vm->_actors[firstEntry._actorId]->speechPlay(firstEntry._sentenceId, false);
				_isNotPause = true;
				_actorId = firstEntry._actorId;
				_sentenceId = firstEntry._sentenceId;
				if (_animationMode >= 0) {
					_animationModePrevious = _vm->_actors[firstEntry._actorId]->getAnimationMode();
					_vm->_actors[firstEntry._actorId]->changeAnimationMode(_animationMode, false);
				} else {
					_animationModePrevious = -1;
				}
			} else if (firstEntry._isPause) {
				_isPause = true;
				_delay = firstEntry._delay;
				_timeLast = _vm->getTotalPlayTime();
			}
		}
	}
}

void ADQ::clear() {
	_entries.clear();
	_isNotPause = false;
	_actorId = -1;
	_sentenceId = -1;
	_animationMode = -1;
	_animationModePrevious = -1;
	_isPause = false;
	_delay = 0;
	_timeLast = 0;
}
} // End of namespace BladeRunner
