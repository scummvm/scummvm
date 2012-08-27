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

void EventRecorder::readEvent(Event &event) {
	if (_recordMode != kRecorderPlayback) {
		return;
	}

	event.type = (EventType)_playbackFile->readUint32LE();

	switch (event.type) {
	case EVENT_KEYDOWN:
	case EVENT_KEYUP:
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
		event.mouse.x = _playbackFile->readSint16LE();
		event.mouse.y = _playbackFile->readSint16LE();
		break;
	default:
		break;
	}
}

void EventRecorder::writeEvent(const Event &event) {
	if (_recordMode != kRecorderRecord) {
		return;
	}

	_recordFile->writeUint32LE((uint32)event.type);

	switch (event.type) {
	case EVENT_KEYDOWN:
	case EVENT_KEYUP:
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
		_recordFile->writeSint16LE(event.mouse.x);
		_recordFile->writeSint16LE(event.mouse.y);
		break;
	default:
		break;
	}
}

EventRecorder::EventRecorder() {
	_recordFile = NULL;
	_playbackFile = NULL;
	_timeMutex = g_system->createMutex();
	_recorderMutex = g_system->createMutex();

	_eventCount = 0;
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

	// recorder stuff
	if (_recordMode == kRecorderRecord) {
		_recordCount = 0;
		_recordFile = wrapBufferedWriteStream(g_system->getSavefileManager()->openForSaving(_recordFileName), 128 * 1024);
	}

	uint32 sign;
	if (_recordMode == kRecorderPlayback) {
		_playbackFile = wrapBufferedSeekableReadStream(g_system->getSavefileManager()->openForLoading(_recordFileName), 128 * 1024, DisposeAfterUse::YES);

		if (!_playbackFile) {
			warning("Cannot open playback file %s. Playback was switched off", _recordFileName.c_str());
			_recordMode = kPassthrough;
		}
	}

	if (_recordMode == kRecorderRecord)	{
		_recordFile->writeUint32LE(RECORD_SIGNATURE);
		_recordFile->writeUint32LE(RECORD_VERSION);
	}

	if (_recordMode == kRecorderPlayback) {
		sign = _playbackFile->readUint32LE();
		if (sign != RECORD_SIGNATURE) {
			error("Unknown record file signature");
		}
		_playbackFile->readUint32LE(); // version
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
	g_system->unlockMutex(_timeMutex);
	g_system->unlockMutex(_recorderMutex);

	if (_playbackFile != NULL) {
		delete _playbackFile;
	}

	if (_recordFile != NULL) {
		_recordFile->finalize();
		delete _recordFile;
	}
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
}

bool EventRecorder::processDelayMillis(uint &msecs) {
	if (_recordMode == kRecorderPlayback) {
		_recordMode = kPassthrough;

		uint32 millis = g_system->getMillis();

		_recordMode = kRecorderPlayback;

		if (_lastMillis > millis) {
			// Skip delay if we're getting late
			return true;
		}
	}

	return false;
}

bool EventRecorder::notifyEvent(const Event &ev) {
	if (_recordMode != kRecorderRecord)
		return false;

	writeEvent(ev);

	return false;
}

bool EventRecorder::pollEvent(Event &ev) {
	if (_recordMode != kRecorderPlayback)
		return false;
	StackLock lock(_recorderMutex);

	readEvent(ev);

	switch (ev.type) {
	case EVENT_MOUSEMOVE:
	case EVENT_LBUTTONDOWN:
	case EVENT_LBUTTONUP:
	case EVENT_RBUTTONDOWN:
	case EVENT_RBUTTONUP:
	case EVENT_WHEELUP:
	case EVENT_WHEELDOWN:
		g_system->warpMouse(ev.mouse.x, ev.mouse.y);
		break;
	default:
		break;
	}
	return true;
}

} // End of namespace Common
