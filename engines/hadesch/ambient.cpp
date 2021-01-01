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
 * Copyright 2020 Google
 *
 */
#include "hadesch/hadesch.h"
#include "hadesch/video.h"
#include "hadesch/ambient.h"

namespace Hadesch {
class AmbientAnimStarter : public EventHandler {
public:
	void operator()() override {
		_anim.play(true);
	}

	AmbientAnimStarter(AmbientAnim anim) {
		_anim = anim;
	}
private:
	AmbientAnim _anim;
};

class AmbientAnimPlayEnded : public EventHandler {
public:
	void operator()() override {
		_anim.playFinished(_reschedule);
	}

	AmbientAnimPlayEnded(AmbientAnim anim, bool reschedule) {
		_anim = anim;
		_reschedule = reschedule;
	}
private:
	AmbientAnim _anim;
	bool _reschedule;
};

AmbientAnim::AmbientAnim(const Common::String &animName,
			 const Common::String &sound, int zValue,
			 int minint, int maxint, AnimType loop,
			 Common::Point offset, PanType pan) {
	_internal = Common::SharedPtr<AmbiantAnimInternal>(
		new AmbiantAnimInternal());
	_internal->_descs.push_back(AmbientDesc(animName, sound));
	_internal->_minInterval = minint;
	_internal->_maxInterval = maxint;
	_internal->_offset = offset;
	_internal->_loopType = loop;
	_internal->_zValue = zValue;
	_internal->_paused = false;
	_internal->_playing = false;
	_internal->_pan = pan;
	_internal->_isFwd = true;
}

AmbientAnim::AmbientAnim(const Common::Array<AmbientDesc> &descs, int zValue,
			 int minint, int maxint, AnimType loop,
			 Common::Point offset, PanType pan) {
	_internal = Common::SharedPtr<AmbiantAnimInternal>(
		new AmbiantAnimInternal());
	_internal->_descs = descs;
	_internal->_minInterval = minint;
	_internal->_maxInterval = maxint;
	_internal->_offset = offset;
	_internal->_loopType = loop;
	_internal->_zValue = zValue;
	_internal->_paused = false;
	_internal->_playing = false;
	_internal->_pan = pan;
	_internal->_isFwd = true;
}

AmbientAnim::AmbientAnim() {
	_internal = Common::SharedPtr<AmbiantAnimInternal>(
		new AmbiantAnimInternal());
	_internal->_minInterval = 0;
	_internal->_maxInterval = 0;
	_internal->_loopType = KEEP_LOOP;
	_internal->_zValue = 0;
	_internal->_paused = false;
	_internal->_playing = false;
	_internal->_isFwd = true;
}

void AmbientAnim::pause() {
	_internal->_paused = true;
}

void AmbientAnim::unpause() {
	_internal->_paused = false;
}

void AmbientAnim::hide() {
	pause();
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	room->stopAnim(_internal->_descs[0]._animName);
	_internal->_playing = false;
	_internal->_paused = true;
}

void AmbientAnim::selectFirstFrame() {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	room->selectFrame(_internal->_descs[0]._animName, _internal->_zValue,
			  0, _internal->_offset);
}

void AmbientAnim::unpauseAndFirstFrame() {
	unpause();
	selectFirstFrame();
}

bool AmbientAnim::isPanOK() {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();

	if (_internal->_pan == PAN_ANY)
		return true;

	if (_internal->_pan == PAN_LEFT && room->isPanLeft())
		return true;

	if (_internal->_pan == PAN_RIGHT && room->isPanRight())
		return true;

	return false;
}

bool AmbientAnim::isReady() {
 	return !_internal->_paused && !_internal->_playing && isPanOK();
}

void AmbientAnim::play(bool reschedule) {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	if (_internal->_paused || _internal->_playing || !isPanOK()) {
		if (reschedule)
			schedule();
		return;
	}
	_internal->_playing = true;
	unsigned variant = 0;
	if (_internal->_descs.size() > 1) {
		variant = g_vm->getRnd().getRandomNumberRng(0, _internal->_descs.size() - 1);
		for (unsigned i = 0; i < _internal->_descs.size(); i++) {
			if (i != variant)
				room->stopAnim(_internal->_descs[i]._animName);
		}
	}

	PlayAnimParams params = PlayAnimParams::disappear();

	switch (_internal->_loopType) {
	case DISAPPEAR:
		params = PlayAnimParams::disappear();
		break;
	case KEEP_LOOP:
		params = PlayAnimParams::keepLastFrame();
		break;
	case BACK_AND_FORTH:
		if (_internal->_isFwd) {
			params = PlayAnimParams::keepLastFrame();
		} else {
			params = PlayAnimParams::disappear().backwards();
		}
		_internal->_isFwd = !_internal->_isFwd;
		break;
	}

	room->playAnim(_internal->_descs[variant]._animName, _internal->_zValue,
		       params,
		       Common::SharedPtr<EventHandler>(new AmbientAnimPlayEnded(*this, reschedule)),
		       _internal->_offset);		
	
	if (_internal->_descs[variant]._soundName != "")
		room->playSFX(_internal->_descs[variant]._soundName, -1);
}
	
void AmbientAnim::schedule() {
	if (_internal->_minInterval >= 0 && _internal->_maxInterval >= 0)
		g_vm->addTimer(
			Common::SharedPtr<EventHandler>(new AmbientAnimStarter(*this)),
			g_vm->getRnd().getRandomNumberRng(_internal->_minInterval,
							  _internal->_maxInterval));
}

void AmbientAnim::playFinished(bool reschedule) {
	_internal->_playing = false;
	if (reschedule)
		schedule();
}

void AmbientAnim::start() {
	if (_internal->_loopType == KEEP_LOOP) {
		selectFirstFrame();
	}
	schedule();
}

void AmbientAnimWeightedSet::readTableFilePriamSFX(const TextTable &table) {
	for (int row = 0; row < table.size(); row++) {
		AmbientAnimWeightedSetElement el;
		el.name = table.get(row, "name");
		el.weight = table.get(row, "weight").asUint64();
		el.valid = table.get(row, "anim") != "";
		if (el.valid)
			el.anim = AmbientAnim(table.get(row, "anim"),
					      table.get(row, "sound"),
					      table.get(row, "depth").asUint64(),
					      -1, -1, AmbientAnim::KEEP_LOOP, Common::Point(0, 0),
					      AmbientAnim::PAN_ANY);
		// TODO: volume
		_elements.push_back(el);
	}
}

void AmbientAnimWeightedSet::readTableFileSFX(const TextTable &table, AmbientAnim::PanType pan) {
	for (int row = 0; row < table.size(); row++) {
		AmbientAnimWeightedSetElement el;
		el.name = table.get(row, "anim");
		el.weight = 1;
		el.valid = table.get(row, "anim") != "";
		if (el.valid)
			el.anim = AmbientAnim(table.get(row, "anim"),
					      table.get(row, "sound"),
					      table.get(row, "Z").asUint64(),
					      -1, -1, AmbientAnim::KEEP_LOOP, Common::Point(
						      table.get(row, "X").asUint64(),
						      table.get(row, "Y").asUint64()),
					      pan);
		// TODO: volume
		_elements.push_back(el);
	}
}

void AmbientAnimWeightedSet::firstFrame() {
	for (unsigned i = 0; i < _elements.size(); i++) {
		if (_elements[i].valid)
			_elements[i].anim.selectFirstFrame();
	}
}

void AmbientAnimWeightedSet::tick() {
	int chosen = -1, chosenWeight = -1;
	// This is not how weighted random should be
	// but this is howoriginal game generates it and
	// it's probably slightly wrong from mathematical
	// point of view
	for (unsigned i = 0; i < _elements.size(); i++) {
		if (!_elements[i].anim.isReady())
			continue;
		int curWeight = _elements[i].weight * g_vm->getRnd().getRandomNumberRng(
			0, 100);
		if (curWeight > chosenWeight) {
			chosen = i;
			chosenWeight = curWeight;
		}
	}
	if (chosen < 0 || !_elements[chosen].valid)
		return;
	_elements[chosen].anim.play(false);
}

void AmbientAnimWeightedSet::pause(const Common::String &name) {
	for (unsigned i = 0; i < _elements.size(); i++) {
		if (_elements[i].name == name && _elements[i].valid) {
			_elements[i].anim.pause();
		}
	}
}

void AmbientAnimWeightedSet::unpause(const Common::String &name) {
	for (unsigned i = 0; i < _elements.size(); i++) {
		if (_elements[i].name == name && _elements[i].valid) {
			_elements[i].anim.unpause();
		}
	}
}

void AmbientAnimWeightedSet::play(const Common::String &name, bool reschedule) {
	for (unsigned i = 0; i < _elements.size(); i++) {
		if (_elements[i].name == name && _elements[i].valid) {
			_elements[i].anim.play(reschedule);
		}
	}
}

void AmbientAnimWeightedSet::unpauseAndFirstFrame(const Common::String &name) {
	for (unsigned i = 0; i < _elements.size(); i++) {
		if (_elements[i].name == name && _elements[i].valid) {
			_elements[i].anim.unpauseAndFirstFrame();
		}
	}
}

void AmbientAnimWeightedSet::hide(const Common::String &name) {
	for (unsigned i = 0; i < _elements.size(); i++) {
		if (_elements[i].name == name && _elements[i].valid) {
			_elements[i].anim.hide();
		}
	}
}

void AnimClickables::playChosen(const Common::String &name, int counter, const EventHandlerWrapper &event) {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	int rk = _table.rowCount(name);
	if (rk == 0) {
		event();
		return;
	}
	counter %= rk;
	Common::String smacker = _table.get(name, "smacker", counter);
	Common::String anim = _table.get(name, "anim", counter);
	Common::String sound = _table.get(name, "sound", counter);
	int zValue = _table.get(name, "Z", counter).asUint64();
	if (smacker != "")
		room->playVideo(smacker.substr(1), zValue, event,
				Common::Point(_table.get(name, "smackerX", counter).asUint64(),
					      _table.get(name, "smackerY", counter).asUint64()));
	else if (anim != "")
		room->playAnimWithSpeech(
			anim, TranscribedSound::make(sound.c_str(), _transcriptions[sound].c_str()), zValue, PlayAnimParams::disappear(), event,
			Common::Point(_table.get(name, "X", counter).asUint64(),
				      _table.get(name, "Y", counter).asUint64()));
	else if (sound != "")
		room->playSpeech(TranscribedSound::make(sound.c_str(), _transcriptions[sound].c_str()), event);
	else
		event();
}

// TODO: should counters be persistent?
void AnimClickables::playNext(const Common::String &name, const EventHandlerWrapper &event) {
	playChosen(name, _counters[name], event);
	_counters[name]++;
}

void AnimClickables::readTable(Common::SharedPtr<Hadesch::VideoRoom> room, const Common::String &name, const TranscribedSound *transcriptions) {
	_table = TextTable(Common::SharedPtr<Common::SeekableReadStream>(room->openFile(name)), 14);

	for (const TranscribedSound *t = transcriptions; t->soundName; t++) {
		_transcriptions[t->soundName] = t->transcript;
	}
}

}
