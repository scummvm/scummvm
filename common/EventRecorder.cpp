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
#include "common/debug-channels.h"
#include "backends/timer/sdl/sdl-timer.h"
#include "backends/mixer/sdl/sdl-mixer.h"
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
	_timeMutex = g_system->createMutex();
	_recorderMutex = g_system->createMutex();
	_recordMode = kPassthrough;
}

EventRecorder::~EventRecorder() {
	deinit();

	g_system->deleteMutex(_timeMutex);
	g_system->deleteMutex(_recorderMutex);
}

void EventRecorder::init() {
	_fakeMixerManager = new NullSdlMixerManager();
	_fakeMixerManager->init();
	_fakeMixerManager->suspendAudio();
	DebugMan.addDebugChannel(kDebugLevelEventRec, "EventRec", "Event recorder debug level");
}

void EventRecorder::deinit() {
	debugC(3, kDebugLevelEventRec, "EventRecorder: deinit");

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



void EventRecorder::processMillis(uint32 &millis) {
	if (_recordMode == kPassthrough) {
		return;
	}
	_fakeMixerManager->update();
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

void EventRecorder::RegisterEventSource() {
	g_system->getEventManager()->getEventDispatcher()->registerSource(this, false);
	g_system->getEventManager()->getEventDispatcher()->registerObserver(this, EventManager::kEventRecorderPriority, false, true);
}

uint32 EventRecorder::getRandomSeed(const String &name) {
	uint32 result = g_system->getMillis();
	if (_recordMode == kRecorderRecord) {
		_randomSourceRecords[name] = result;
	} else if (_recordMode == kRecorderPlayback) {
		result = _randomSourceRecords[name];
	}
	return result;
}

void EventRecorder::init(Common::String gameId, const ADGameDescription* desc) {
	_playbackFile = NULL;
	_recordFile = NULL;
	String recordModeString = ConfMan.get("record_mode");
	if (recordModeString.compareToIgnoreCase("record") == 0) {
		_recordMode = kRecorderRecord;
		debugC(3, kDebugLevelEventRec, "EventRecorder: record");
	} else {
		if (recordModeString.compareToIgnoreCase("playback") == 0) {
			_recordMode = kRecorderPlayback;
			debugC(3, kDebugLevelEventRec, "EventRecorder: playback");
		} else {
			_recordMode = kPassthrough;
			debugC(3, kDebugLevelEventRec, "EventRecorder: passthrough");
		}
	}	
	openRecordFile(gameId);
	if (desc != NULL) {
		if (_recordMode == kRecorderRecord) {
			writeGameHash(desc);
			_recordFile->writeString(getAutor());
			_recordFile->writeByte(0);
			_recordFile->writeString(getComment());
			_recordFile->writeByte(0);
		}
		if (_recordMode == kRecorderPlayback) {
			checkGameHash(desc);
			warning("%s", readString().c_str());
			warning("%s", readString().c_str());
			getNextEvent();
		}
	}
	switchMixer();
	_fakeTimer = 0;
	_recordCount = 0;
	_lastMillis = 0;
	_eventCount = 1;
	_lastEventCount = 0;
	_lastEventMillis = 0;
}

void EventRecorder::openRecordFile(Common::String gameId) {
	Common::String fileName;
	if (gameId.empty()) {
		warning("Game id is undefined. Using default record file name.");
		fileName = "record.bin";
	} else {
		fileName = gameId + ".bin";
	}

	if (_recordMode == kRecorderRecord) {
		_recordFile = wrapBufferedWriteStream(g_system->getSavefileManager()->openForSaving(fileName), 128 * 1024);
		_recordFile->writeUint32LE(RECORD_SIGNATURE);
		_recordFile->writeUint32LE(RECORD_VERSION);
	}

	if (_recordMode == kRecorderPlayback) {
		uint32 sign;
		uint32 version;

		_playbackFile = wrapBufferedSeekableReadStream(g_system->getSavefileManager()->openForLoading(fileName), 128 * 1024, DisposeAfterUse::YES);
		if (!_playbackFile) {
			warning("Cannot open playback file %s. Playback was switched off", fileName.c_str());
			_recordMode = kPassthrough;
			return;
		}

		sign = _playbackFile->readUint32LE();
		if (sign != RECORD_SIGNATURE) {
			warning("Unknown playback file signature");
			_recordMode = kPassthrough;
		}

		version = _playbackFile->readUint32LE(); 
		if (version != RECORD_VERSION) {
			warning("Incorrect playback file version. Current version is %d", RECORD_VERSION);
			_recordMode = kPassthrough;
		}
	}
}

void EventRecorder::checkGameHash(const ADGameDescription* gameDesc) {
	uint8 md5RecordsCount = _playbackFile->readByte();
	if ((gameDesc == NULL) && (md5RecordsCount != 0)) {
		warning("Engine doesn't contain description table");
		_recordMode = kPassthrough;
		return;
	}
	for (int i = 0; i < md5RecordsCount; ++i) {
		String storedFileName = readString();
		String storedMd5Hash = readString();
		String engineMd5Hash = findMd5ByFileName(gameDesc, storedFileName);
		if (!engineMd5Hash.empty()) {
			if (!storedMd5Hash.equals(engineMd5Hash)) {
				warning("Incorrect version of game file %s. Stored md5 is %s. Md5 of loaded game is %s", storedFileName, storedMd5Hash, engineMd5Hash);
				_recordMode = kPassthrough;
				return;
			}
		} else {
			warning("Md5 hash for file %s not found", storedFileName);
		}
	}
}

Common::String EventRecorder::findMd5ByFileName(const ADGameDescription* gameDesc, String fileName) {
	for (const ADGameFileDescription *fileDesc = gameDesc->filesDescriptions; fileDesc->fileName; fileDesc++) {
		if (fileName.equals(fileDesc->fileName)) {
			return fileDesc->md5;
		}
	}
	return String();
}

void EventRecorder::writeGameHash(const ADGameDescription* gameDesc) {
	byte md5RecordsCount = 0;
	for (const ADGameFileDescription *fileDesc = gameDesc->filesDescriptions; fileDesc->fileName; fileDesc++) {
		md5RecordsCount++;
	}
	_recordFile->writeByte(md5RecordsCount);
	for (const ADGameFileDescription *fileDesc = gameDesc->filesDescriptions; fileDesc->fileName; fileDesc++) {
		_recordFile->writeString(fileDesc->fileName);
		_recordFile->writeByte(0);
		_recordFile->writeString(fileDesc->md5);
		_recordFile->writeByte(0);
	}
}

String EventRecorder::readString() {
	String result;
	char buf[50];
	int count = 0;
	char *ptr = buf;
	while (*ptr = _playbackFile->readByte()) {
		ptr++;
		count++;
		if (count > 48) {
			*ptr = 0;
			result += buf;
			ptr = buf;
		}
	}
	*ptr = 0;
	result += buf;
	return result;
}

void EventRecorder::registerMixerManager(SdlMixerManager* mixerManager) {
	_realMixerManager = mixerManager;
}

void EventRecorder::switchMixer() {
	if (_recordMode == kPassthrough) {
		_fakeMixerManager->suspendAudio();
		_realMixerManager->resumeAudio();
	} else {
		_realMixerManager->suspendAudio();
		_fakeMixerManager->resumeAudio();
	}
}

SdlMixerManager* EventRecorder::getMixerManager() {
	if (_recordMode == kPassthrough) {
		return _realMixerManager;
	} else {
		return _fakeMixerManager;
	}
}

Common::String EventRecorder::getAutor() {
	return "Unknown Autor";
}

Common::String EventRecorder::getComment() {
	return "Empty\ncomment";
}

} // End of namespace Common
