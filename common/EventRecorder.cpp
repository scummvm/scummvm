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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/EventRecorder.h"
#include "backends/timer/sdl/sdl-timer.h"

#include "common/bufferedstream.h"
#include "common/config-manager.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/textconsole.h"

namespace Common {

DECLARE_SINGLETON(EventRecorder);

#define RECORD_SIGNATURE 0x54455354
#define RECORD_VERSION 1

uint32 readTime(ReadStream *inFile) {
	uint32 d = inFile->readByte();
	if (d == 0xff) {
		d = inFile->readUint32LE();
	}

	return d;
}

void writeTime(WriteStream *outFile, uint32 d) {
		//Simple RLE compression
	if (d >= 0xff) {
		outFile->writeByte(0xff);
		outFile->writeUint32LE(d);
	} else {
		outFile->writeByte(d);
	}
}

void EventRecorder::readEvent(RecorderEvent &event) {
	if (_recordMode != kRecorderPlayback) {
		return;
	}
	_recordCount++;

	event.type = (EventType)_playbackFile->readUint32LE();
	switch (event.type) {
	case EVENT_TIMER:
		event.time = _playbackFile->readUint32LE();
		event.count = _playbackFile->readUint32LE();
		break;
	case EVENT_KEYDOWN:
	case EVENT_KEYUP:
		event.time = _playbackFile->readUint32LE();
		event.kbd.keycode = (KeyCode)_playbackFile->readSint32LE();
		event.kbd.ascii = _playbackFile->readUint16LE();
		event.kbd.flags = _playbackFile->readByte();
		break;
	case EVENT_MOUSEMOVE:
	case EVENT_LBUTTONDOWN:
	case EVENT_LBUTTONUP:
	case EVENT_RBUTTONDOWN:
	case EVENT_RBUTTONUP:
	case EVENT_WHEELUP:
	case EVENT_WHEELDOWN:
	case EVENT_MBUTTONDOWN:
	case EVENT_MBUTTONUP:
		event.time = _playbackFile->readUint32LE();
		event.mouse.x = _playbackFile->readSint16LE();
		event.mouse.y = _playbackFile->readSint16LE();
		break;
	default:
		event.time = _playbackFile->readUint32LE();
		break;
	}
}

void EventRecorder::writeEvent(const RecorderEvent &event) {
	if (_recordMode != kRecorderRecord) {
		return;
	}

	_recordFile->writeUint32LE((uint32)event.type);
	_recordCount++;

	switch (event.type) {
	case EVENT_TIMER:
		_recordFile->writeUint32LE((uint32)_fakeTimer);
		_recordFile->writeUint32LE((uint32)_eventCount);
		break;
	case EVENT_KEYDOWN:
	case EVENT_KEYUP:
		_recordFile->writeUint32LE((uint32)_fakeTimer);
		_recordFile->writeSint32LE(event.kbd.keycode);
		_recordFile->writeUint16LE(event.kbd.ascii);
		_recordFile->writeByte(event.kbd.flags);
		break;
	case EVENT_MOUSEMOVE:
	case EVENT_LBUTTONDOWN:
	case EVENT_LBUTTONUP:
	case EVENT_RBUTTONDOWN:
	case EVENT_RBUTTONUP:
	case EVENT_WHEELUP:
	case EVENT_WHEELDOWN:
	case EVENT_MBUTTONDOWN:
	case EVENT_MBUTTONUP:
		_recordFile->writeUint32LE((uint32)_fakeTimer);
		_recordFile->writeSint16LE(event.mouse.x);
		_recordFile->writeSint16LE(event.mouse.y);
		break;
	default:
		_recordFile->writeUint32LE((uint32)_fakeTimer);
		break;
	}
}


EventRecorder::EventRecorder() {
	_recordFile = NULL;
	_playbackFile = NULL;
	_timeMutex = g_system->createMutex();
	_recorderMutex = g_system->createMutex();

	_eventCount = 1;
	_lastEventCount = 0;
	_lastMillis = 0;
	_lastEventMillis = 0;

	_recordMode = kPassthrough;
}

EventRecorder::~EventRecorder() {
	deinit();

	g_system->deleteMutex(_timeMutex);
	g_system->deleteMutex(_recorderMutex);
}

void EventRecorder::init() {
	String recordModeString = ConfMan.get("record_mode");
	if (recordModeString.compareToIgnoreCase("record") == 0) {
		_recordMode = kRecorderRecord;

		debug(3, "EventRecorder: record");
	} else {
		if (recordModeString.compareToIgnoreCase("playback") == 0) {
			_recordMode = kRecorderPlayback;
			debug(3, "EventRecorder: playback");
		} else {
			_recordMode = kPassthrough;
			debug(3, "EventRecorder: passthrough");
		}
	}

	_recordFileName = ConfMan.get("record_file_name");
	if (_recordFileName.empty()) {
		_recordFileName = "record.bin";
	}

	_recordCount = 0;
	// recorder stuff
	if (_recordMode == kRecorderRecord) {
		_recordFile = wrapBufferedWriteStream(g_system->getSavefileManager()->openForSaving(_recordFileName), 128 * 1024);
	}

	_fakeTimer = 0;

	uint32 sign;
	if (_recordMode == kRecorderPlayback) {
		_playbackFile = wrapBufferedSeekableReadStream(g_system->getSavefileManager()->openForLoading(_recordFileName), 128 * 1024, DisposeAfterUse::YES);

		if (!_playbackFile) {
			warning("Cannot open playback file %s. Playback was switched off", _recordFileName.c_str());
			_recordMode = kPassthrough;
		}
	}

	if (_recordMode == kRecorderRecord) {
		_recordFile->writeUint32LE(RECORD_SIGNATURE);
		_recordFile->writeUint32LE(RECORD_VERSION);
	}

	if (_recordMode == kRecorderPlayback) {
		sign = _playbackFile->readUint32LE();
		if (sign != RECORD_SIGNATURE) {
			error("Unknown record file signature");
		}
		_playbackFile->readUint32LE(); // version
		getNextEvent();
	}

	g_system->getEventManager()->getEventDispatcher()->registerSource(this, false);
	g_system->getEventManager()->getEventDispatcher()->registerObserver(this, EventManager::kEventRecorderPriority, false, true);
}

void EventRecorder::deinit() {
	debug(3, "EventRecorder: deinit");

	g_system->getEventManager()->getEventDispatcher()->unregisterSource(this);
	g_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);

	g_system->lockMutex(_timeMutex);
	g_system->lockMutex(_recorderMutex);
	_recordMode = kPassthrough;
	if (_playbackFile != NULL) {
		delete _playbackFile;
	}

	if (_recordFile != NULL) {
		_recordFile->finalize();
		delete _recordFile;
	}
	g_system->unlockMutex(_timeMutex);
	g_system->unlockMutex(_recorderMutex);
}

void EventRecorder::registerRandomSource(RandomSource &rnd, const String &name) {
	if (_recordMode == kRecorderRecord) {
		RandomSourceRecord rec;
		rec.name = name;
		rec.seed = rnd.getSeed();
		_randomSourceRecords.push_back(rec);
	}

	if (_recordMode == kRecorderPlayback) {
		for (uint i = 0; i < _randomSourceRecords.size(); ++i) {
			if (_randomSourceRecords[i].name == name) {
				rnd.setSeed(_randomSourceRecords[i].seed);
				_randomSourceRecords.remove_at(i);
				break;
			}
		}
	}
}


void EventRecorder::processMillis(uint32 &millis) {
	if (_recordMode == kPassthrough) {
		return;
	}
	g_system->lockMutex(_timeMutex);
	if (_recordMode == kRecorderRecord) {
		uint32 _millisDelay;
		_millisDelay = millis - _lastMillis;
		_lastMillis = millis;
		_fakeTimer += _millisDelay;
		RecorderEvent timerEvent;
		timerEvent.type = EVENT_TIMER;
		timerEvent.time = _fakeTimer;
		writeEvent(timerEvent);
	}

	if (_recordMode == kRecorderPlayback) {
		if (_nextEvent.type == EVENT_TIMER) {
			_fakeTimer = _nextEvent.time;
			getNextEvent();
		}
		millis = _fakeTimer;
	}
	g_system->unlockMutex(_timeMutex);
}

bool EventRecorder::processDelayMillis() {
	return _fastPlayback;
}

void EventRecorder::checkForKeyCode(const Event &event) {
	if (event.type == EVENT_KEYDOWN) {
		if ((event.kbd.ascii == '/')) {
			togglePause();
		}
	}
}

bool EventRecorder::notifyEvent(const Event &ev) {
	StackLock lock(_recorderMutex);
	checkForKeyCode(ev);
	if (_recordMode != kRecorderRecord)
		return false;

	RecorderEvent e;
	memcpy(&e, &ev, sizeof(ev));
	writeEvent(e);

	return false;
}

bool EventRecorder::pollEvent(Event &ev) {
	if (_recordMode != kRecorderPlayback)
		return false;
	StackLock lock(_recorderMutex);
	
	RecorderEvent nextEvent;
	readEvent(nextEvent);

	if ((_nextEvent.type ==  EVENT_INVALID) || (_nextEvent.type == EVENT_TIMER)) {
		return false;
	}

	switch (_nextEvent.type) {
	case EVENT_MOUSEMOVE:
	case EVENT_LBUTTONDOWN:
	case EVENT_LBUTTONUP:
	case EVENT_RBUTTONDOWN:
	case EVENT_RBUTTONUP:
	case EVENT_WHEELUP:
	case EVENT_WHEELDOWN:
		g_system->warpMouse(_nextEvent.mouse.x, _nextEvent.mouse.y);
		break;
	default:
		break;
	}
	ev = _nextEvent;
	getNextEvent();
	return true;
}

void EventRecorder::switchFastMode() {
	if (_recordMode == kRecorderPlaybackPause) {
		_fastPlayback = !_fastPlayback;
	}
}

void EventRecorder::getNextEvent() {
	if(!_playbackFile->eos()) {
		readEvent(_nextEvent);
	}
}

void EventRecorder::togglePause() {
	switch (_recordMode) {
	case kRecorderPlayback:
		_recordMode = kRecorderPlaybackPause;
		break;
	case kRecorderPlaybackPause:
		_recordMode = kRecorderPlayback;
		break;
	}
}

} // End of namespace Common
