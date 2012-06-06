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
#include "common/md5.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "graphics/thumbnail.h"
#include "graphics/surface.h"
#include "graphics/scaler.h"
namespace Common {

DECLARE_SINGLETON(EventRecorder);

#define RECORD_VERSION 1
#define kDefaultScreenshotPeriod 60000
#define kDefaultBPP 2

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
	event.type = (EventType)_tmpPlaybackFile.readUint32LE();
	switch (event.type) {
	case EVENT_TIMER:
		event.time = _tmpPlaybackFile.readUint32LE();
		break;
	case EVENT_KEYDOWN:
	case EVENT_KEYUP:
		event.time = _tmpPlaybackFile.readUint32LE();
		event.kbd.keycode = (KeyCode)_tmpPlaybackFile.readSint32LE();
		event.kbd.ascii = _tmpPlaybackFile.readUint16LE();
		event.kbd.flags = _tmpPlaybackFile.readByte();
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
		event.time = _tmpPlaybackFile.readUint32LE();
		event.mouse.x = _tmpPlaybackFile.readSint16LE();
		event.mouse.y = _tmpPlaybackFile.readSint16LE();
		break;
	default:
		event.time = _tmpPlaybackFile.readUint32LE();
		break;
	}
}

void EventRecorder::writeEvent(const RecorderEvent &event) {
	if (_recordMode != kRecorderRecord) {
		return;
	}
	_recordCount++;
	_tmpRecordFile.writeUint32LE((uint32)event.type);
	switch (event.type) {
	case EVENT_TIMER:
		_tmpRecordFile.writeUint32LE(event.time);
		break;
	case EVENT_KEYDOWN:
	case EVENT_KEYUP:
		_tmpRecordFile.writeUint32LE(event.time);
		_tmpRecordFile.writeSint32LE(event.kbd.keycode);
		_tmpRecordFile.writeUint16LE(event.kbd.ascii);
		_tmpRecordFile.writeByte(event.kbd.flags);
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
		_tmpRecordFile.writeUint32LE(event.time);
		_tmpRecordFile.writeSint16LE(event.mouse.x);
		_tmpRecordFile.writeSint16LE(event.mouse.y);
		break;
	default:
		_tmpRecordFile.writeUint32LE(event.time);
		break;
	}
	if (_recordCount == kMaxBufferedRecords) {
		dumpRecordsToFile();
	}
}

void EventRecorder::dumpRecordsToFile() {
	if (!_headerDumped) {
		dumpHeaderToFile();
		_headerDumped = true;
	}
	if (_recordCount == 0) {
		return;
	}
	_recordFile->writeUint32LE(MKTAG('E','V','N','T'));
	_recordFile->writeUint32LE(_tmpRecordFile.pos());
	_recordFile->write(_recordBuffer, _tmpRecordFile.pos());
	_tmpRecordFile.seek(0);
	_recordCount = 0;
}

void EventRecorder::dumpHeaderToFile() {
	writeFormatId();
	writeVersion();
	writeHeader();
	writeGameHash();
	writeRandomRecords();
	writeGameSettings();
	writeScreenSettings();
}

EventRecorder::EventRecorder() : _tmpRecordFile(_recordBuffer, kRecordBuffSize), _tmpPlaybackFile(_recordBuffer, kRecordBuffSize) {
	_timeMutex = g_system->createMutex();
	_recorderMutex = g_system->createMutex();
	_recordMode = kPassthrough;
	_timerManager = NULL;
	_bitmapBuff = NULL;
	_playbackFile = NULL;
	_recordFile = NULL;
	_screenshotsFile = NULL;
	initialized = false;
}

EventRecorder::~EventRecorder() {
	g_system->deleteMutex(_timeMutex);
	g_system->deleteMutex(_recorderMutex);
	if (_timerManager != NULL) {
		delete _timerManager;
	}
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
	if (_screenshotsFile != NULL) {
		_screenshotsFile->finalize();
		delete _screenshotsFile;
	}
	if (_recordFile != NULL) {
		dumpRecordsToFile();
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
	if (!initialized) {
		return;
	}
	updateSubsystems();
	if (_recordMode == kRecorderRecord) {
		uint32 _millisDelay;
		_millisDelay = millis - _lastMillis;
		_lastMillis = millis;
		_fakeTimer += _millisDelay;
		RecorderEvent timerEvent;
		timerEvent.type = EVENT_TIMER;
		timerEvent.time = _fakeTimer;
		writeEvent(timerEvent);
		saveScreenShot();
	}

	if (_recordMode == kRecorderPlayback) {
		if (_nextEvent.type == EVENT_TIMER) {
			_fakeTimer = _nextEvent.time;
			getNextEvent();
		}
		millis = _fakeTimer;
	}
	_timerManager->handler();
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
	if (!initialized) {
		return false;
	}
	RecorderEvent e;
	memcpy(&e, &ev, sizeof(ev));
	e.time = _fakeTimer;
	writeEvent(e);

	return false;
}

bool EventRecorder::pollEvent(Event &ev) {
	if (_recordMode != kRecorderPlayback)
		return false;
	if (!initialized) {
		return false;
	}
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
	if (_tmpPlaybackFile.pos() == _eventsSize) {
		ChunkHeader header;
		header.id = 0;
		while (header.id != MKTAG('E','V','N','T')) {
			header = readChunkHeader();
			if (_playbackFile->eos()) {
				break;
			}
			switch (header.id) {
			case MKTAG('E','V','N','T'):
				readEventsToBuffer(header.len);
				break;
			case MKTAG('B','M','H','T'):
				loadScreenShot();
				break;
			case MKTAG('M','D','5',' '):
				checkRecordedMD5();
				break;
			default:
				_playbackFile->skip(header.len);
				break;
			}
		}
	}
	readEvent(_nextEvent);		
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

void EventRecorder::init(Common::String gameId, const ADGameDescription *gameDesc) {
	_recordCount = 0;
	_lastScreenshotTime = 0;
	_bitmapBuffSize = 0;
	_eventsSize = 0;
	_screenshotPeriod = ConfMan.getInt("screenshot_period");
	if (_screenshotPeriod == 0) {
		_screenshotPeriod = kDefaultScreenshotPeriod;
	}
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
	switchTimerManagers();
	if (!openRecordFile(gameId)) {
		_recordMode = kPassthrough;
		return;
	}

	if ((_recordMode == kRecorderRecord) && (gameDesc != NULL)) {
		for (const ADGameFileDescription *fileDesc = gameDesc->filesDescriptions; fileDesc->fileName; fileDesc++) {
			_hashRecords[fileDesc->fileName] = fileDesc->md5;
		}
		_tmpRecordFile.seek(0);
	}

	if (_recordMode == kRecorderPlayback) {
		if (!parsePlaybackFile()) {
			_recordMode = kPassthrough;
			return;
		}
		if ((gameDesc != NULL) && !checkGameHash(gameDesc)) {
			_recordMode = kPassthrough;
			return;
		}
		applyPlaybackSettings();
		getNextEvent();
	}
	switchMixer();
	_fakeTimer = 0;
	_lastMillis = 0;
	_headerDumped = false;
	initialized = true;
}

void EventRecorder::init(const ADGameDescription *desc) {
	init(desc->gameid, desc);
}

/**
 * Opens or creates file depend of recording mode.
 *
 *@param id of recording or playing back game
 *@return true in case of success, false in case of error
 *
 */

bool EventRecorder::openRecordFile(const String &gameId) {
	Common::String fileName;
	if (gameId.empty()) {
		warning("Game id is undefined. Using default record file name.");
		fileName = "record.bin";
	} else {
		fileName = gameId + ".bin";
	}

	if (_recordMode == kRecorderRecord) {
		_recordFile = wrapBufferedWriteStream(g_system->getSavefileManager()->openForSaving(fileName), 128 * 1024);
		if (!_recordFile) {
			warning("Cannot open file %s for recording. Record was switched off", fileName.c_str());
			return false;
		}
	}

	if (_recordMode == kRecorderPlayback) {
		_playbackFile = wrapBufferedSeekableReadStream(g_system->getSavefileManager()->openForLoading(fileName), 128 * 1024, DisposeAfterUse::YES);
		if (!_playbackFile) {
			warning("Cannot open playback file %s. Playback was switched off", fileName.c_str());
			return false;
		}
		_screenshotsFile = wrapBufferedWriteStream(g_system->getSavefileManager()->openForSaving("screenshots.bin"), 128 * 1024);
	}
	return true;
}

bool EventRecorder::checkGameHash(const ADGameDescription *gameDesc) {
	if ((gameDesc == NULL) && (_hashRecords.size() != 0)) {
		warning("Engine doesn't contain description table");
		return false;
	}
	for (const ADGameFileDescription *fileDesc = gameDesc->filesDescriptions; fileDesc->fileName; fileDesc++) {
		if (_hashRecords.find(fileDesc->fileName) == _hashRecords.end()) {
			warning("MD5 hash for file %s not found in record file", fileDesc->fileName);
			return false;
		}
		if (_hashRecords[fileDesc->fileName] != fileDesc->md5) {
			warning("Incorrect version of game file %s. Stored MD5 is %s. MD5 of loaded game is %s", fileDesc->fileName, _hashRecords[fileDesc->fileName].c_str(), fileDesc->md5);
			return false;
		}
	}
	return true;
}

Common::String EventRecorder::findMD5ByFileName(const ADGameDescription *gameDesc, const String &fileName) {
	for (const ADGameFileDescription *fileDesc = gameDesc->filesDescriptions; fileDesc->fileName; fileDesc++) {
		if (fileName.equals(fileDesc->fileName)) {
			return fileDesc->md5;
		}
	}
	return String();
}

void EventRecorder::registerMixerManager(SdlMixerManager *mixerManager) {
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

SdlMixerManager *EventRecorder::getMixerManager() {
	if (_recordMode == kPassthrough) {
		return _realMixerManager;
	} else {
		return _fakeMixerManager;
	}
}

Common::String EventRecorder::getAuthor() {
	return "Unknown Author";
}

Common::String EventRecorder::getComment() {
	return "Empty\ncomment";
}

bool EventRecorder::parsePlaybackFile() {
	ChunkHeader nextChunk;
	_playbackParseState = kFileStateCheckFormat;
	nextChunk = readChunkHeader();
	while ((_playbackParseState != kFileStateDone) && (_playbackParseState != kFileStateError)) {
		if (processChunk(nextChunk)) {
			nextChunk = readChunkHeader();
		}
	}
	return _playbackParseState == kFileStateDone;
}

ChunkHeader EventRecorder::readChunkHeader() {
	ChunkHeader result;
	result.id = _playbackFile->readUint32LE();
	result.len = _playbackFile->readUint32LE();
	return result;
}

bool EventRecorder::processChunk(ChunkHeader &nextChunk) {
	switch (_playbackParseState) {
	case kFileStateCheckFormat:
		if ((nextChunk.id == MKTAG('P','B','C','K')) && (nextChunk.len = _playbackFile->size() - 8)) {
			_playbackParseState = kFileStateCheckVersion;
		} else {
			warning("Unknown playback file signature");
			_playbackParseState = kFileStateError;
		}
		break;
	case kFileStateCheckVersion:
		if ((nextChunk.id == MKTAG('V','E','R','S')) && checkPlaybackFileVersion()) {
			_playbackParseState = kFileStateSelectSection;
		} else {
			_recordMode = kPassthrough;
			_playbackParseState = kFileStateError;
		}
		break;
	case kFileStateSelectSection:
		switch (nextChunk.id) {
		case MKTAG('H','E','A','D'): 
			_playbackParseState = kFileStateProcessHeader;
			break;
		case MKTAG('H','A','S','H'): 
			_playbackParseState = kFileStateProcessHash;
			break;
		case MKTAG('R','A','N','D'): 
			_playbackParseState = kFileStateProcessRandom;
			break;
		case MKTAG('E','V','N','T'): 
			readEventsToBuffer(nextChunk.len);
			_playbackParseState = kFileStateDone;
			return false;
		case MKTAG('B','M','H','T'): 
			loadScreenShot();
			_playbackParseState = kFileStateDone;
			return false;
		case MKTAG('S','E','T','T'):
			_playbackParseState = kFileStateProcessSettings;
			warning("Loading record header");
			break;
		case MKTAG('S','C','R','N'):
			processScreenSettings();
			break;
		default:
			_playbackFile->skip(nextChunk.len);
			break;
		}
		break;
	case kFileStateProcessHeader:
		switch (nextChunk.id) {
		case MKTAG('H','A','U','T'): 
			readAuthor(nextChunk);
			break;
		case MKTAG('H','C','M','T'):
			readComment(nextChunk);
			break;
		default:
			_playbackParseState = kFileStateSelectSection;
			return false;
		}
		break;
	case kFileStateProcessHash:
		if (nextChunk.id == MKTAG('H','R','C','D')) {
			readHashMap(nextChunk);
		} else {
			_playbackParseState = kFileStateSelectSection;
			return false;
		}
		break;
	case kFileStateProcessRandom:
		if (nextChunk.id == MKTAG('R','R','C','D')) {
			processRndSeedRecord(nextChunk);
		} else {
			_playbackParseState = kFileStateSelectSection;
			return false;
		}
		break;
	case kFileStateProcessSettings:
		if (nextChunk.id == MKTAG('S','R','E','C')) {
			if (!processSettingsRecord(nextChunk)) {
				_playbackParseState = kFileStateError;
				return false;
			}
		} else {
			_playbackParseState = kFileStateSelectSection;
			return false;
		}
		break;
	}
	return true;
}

bool EventRecorder::checkPlaybackFileVersion() {
	uint32 version;
	version = _playbackFile->readUint32LE();
	if (version != RECORD_VERSION) {
		warning("Incorrect playback file version. Expected version %d, but got %d.", RECORD_VERSION, version);
		return false;
	}
	return true;
}

void EventRecorder::readAuthor(ChunkHeader chunk) {
	String author = readString(chunk.len);
	warning("Author: %s", author.c_str());
}

void EventRecorder::readComment(ChunkHeader chunk) {
	String comment = readString(chunk.len);
	warning("Comments: %s", comment.c_str());
}

void EventRecorder::processRndSeedRecord(ChunkHeader chunk) {
	String randomSourceName = readString(chunk.len - 4);
	uint32 randomSourceSeed = _playbackFile->readUint32LE();
	_randomSourceRecords[randomSourceName] = randomSourceSeed;
}

void EventRecorder::readHashMap(ChunkHeader chunk) {
	String hashName = readString(chunk.len - 32);
	String hashMd5 = readString(32);
	_hashRecords[hashName] = hashMd5;
}


String EventRecorder::readString(int len) {
	String result;
	char buf[50];
	int readSize = 49;
	while (len > 0)	{
		if (len <= 49) {
			readSize = len;
		}
		_playbackFile->read(buf, readSize);
		buf[readSize] = 0;
		result += buf;
		len -= readSize;
	}
	return result;
}

void EventRecorder::writeFormatId() {
	_recordFile->writeUint32LE(MKTAG('P','B','C','K'));
	_recordFile->writeUint32LE(0);
}

void EventRecorder::writeVersion() {
	_recordFile->writeUint32LE(MKTAG('V','E','R','S'));
	_recordFile->writeUint32LE(4);
	_recordFile->writeUint32LE(RECORD_VERSION);
}

void EventRecorder::writeHeader() {
	String author = getAuthor();
	String comment = getComment();
	uint32 headerSize = 0;
	if (!author.empty()) {
		headerSize = author.size() + 8;
	}
	if (!comment.empty()) {
		headerSize += comment.size() + 8;
	}
	if (headerSize == 0) {
		return;
	}
	_recordFile->writeUint32LE(MKTAG('H','E','A','D'));
	_recordFile->writeUint32LE(headerSize);
	if (!author.empty()) {
		_recordFile->writeUint32LE(MKTAG('H','A','U','T'));
		_recordFile->writeUint32LE(author.size());
		_recordFile->writeString(author);
	}
	if (!comment.empty()) {
		_recordFile->writeUint32LE(MKTAG('H','C','M','T'));
		_recordFile->writeUint32LE(comment.size());
		_recordFile->writeString(comment);
	}
}

void EventRecorder::writeGameHash() {
	uint32 hashSectionSize = 0;
	for (StringMap::iterator i = _hashRecords.begin(); i != _hashRecords.end(); ++i) {
		hashSectionSize = hashSectionSize + i->_key.size() + i->_value.size() + 8;
	}
	if (_hashRecords.size() == 0) {
		return;
	}
	_recordFile->writeUint32LE(MKTAG('H','A','S','H'));
	_recordFile->writeUint32LE(hashSectionSize);
	for (StringMap::iterator i = _hashRecords.begin(); i != _hashRecords.end(); ++i) {
		_recordFile->writeUint32LE(MKTAG('H','R','C','D'));
		_recordFile->writeUint32LE(i->_key.size() + i->_value.size());
		_recordFile->writeString(i->_key);
		_recordFile->writeString(i->_value);
	}
}

void EventRecorder::writeRandomRecords() {
	uint32 randomSectionSize = 0;
	for (randomSeedsDictionary::iterator i = _randomSourceRecords.begin(); i != _randomSourceRecords.end(); ++i) {
		randomSectionSize = randomSectionSize + i->_key.size() + 12;
	}
	if (_randomSourceRecords.size() == 0) {
		return;
	}
	_recordFile->writeUint32LE(MKTAG('R','A','N','D'));
	_recordFile->writeUint32LE(randomSectionSize);
	for (randomSeedsDictionary::iterator i = _randomSourceRecords.begin(); i != _randomSourceRecords.end(); ++i) {
		_recordFile->writeUint32LE(MKTAG('R','R','C','D'));
		_recordFile->writeUint32LE(i->_key.size() + 4);
		_recordFile->writeString(i->_key);
		_recordFile->writeUint32LE(i->_value);
	}
}

void EventRecorder::writeScreenSettings() {
	_recordFile->writeUint32LE(MKTAG('S','C','R','N'));
	//Chunk size = 4 (width(2 bytes) + height(2 bytes))
	_recordFile->writeUint32LE(4);
	_recordFile->writeUint16LE(g_system->getWidth());
	_recordFile->writeSint16LE(g_system->getHeight());
}

void EventRecorder::processScreenSettings() {
	uint16 width = _playbackFile->readUint16LE();
	uint16 height = _playbackFile->readUint16LE();
}


void EventRecorder::writeGameSettings() {
	getConfig();
	if (_settingsSectionSize == 0) {
		return;
	}
	_recordFile->writeUint32LE(MKTAG('S','E','T','T'));
	_recordFile->writeUint32LE(_settingsSectionSize);
	for (StringMap::iterator i = _settingsRecords.begin(); i != _settingsRecords.end(); ++i) {
		_recordFile->writeUint32LE(MKTAG('S','R','E','C'));
		_recordFile->writeUint32LE(i->_key.size() + i->_value.size() + 16);
		_recordFile->writeUint32LE(MKTAG('S','K','E','Y'));
		_recordFile->writeUint32LE(i->_key.size());
		_recordFile->writeString(i->_key);
		_recordFile->writeUint32LE(MKTAG('S','V','A','L'));
		_recordFile->writeUint32LE(i->_value.size());
		_recordFile->writeString(i->_value);
	}
}

void EventRecorder::getConfigFromDomain(ConfigManager::Domain *domain) {
	for (ConfigManager::Domain::iterator entry = domain->begin(); entry!= domain->end(); ++entry) {
		_settingsRecords[entry->_key] = entry->_value;
	}
}

void EventRecorder::getConfig() {
	getConfigFromDomain(ConfMan.getDomain(ConfMan.kApplicationDomain));
	getConfigFromDomain(ConfMan.getActiveDomain());
	getConfigFromDomain(ConfMan.getDomain(ConfMan.kTransientDomain));
	_settingsSectionSize = 0;
	for (StringMap::iterator i = _settingsRecords.begin(); i != _settingsRecords.end(); ++i) {
		_settingsSectionSize = _settingsSectionSize + i->_key.size() + i->_value.size() + 24;
	}
}

bool EventRecorder::processSettingsRecord(ChunkHeader chunk) {
	ChunkHeader keyChunk = readChunkHeader();
	if (keyChunk.id != MKTAG('S','K','E','Y')) {
		warning("Invalid format of settings section");
		return false;
	}
	String key = readString(keyChunk.len);
	ChunkHeader valueChunk = readChunkHeader();
	if (valueChunk.id != MKTAG('S','V','A','L')) {
		warning("Invalid format of settings section");
		return false;
	}
	String value = readString(valueChunk.len);
	_settingsRecords[key] = value;
	return true;
}

void EventRecorder::applyPlaybackSettings() {
	for (StringMap::iterator i = _settingsRecords.begin(); i != _settingsRecords.end(); ++i) {
		String currentValue = ConfMan.get(i->_key);
		if (currentValue != i->_value) {
			warning("Config value <%s>: %s -> %s", i->_key.c_str(), i->_value.c_str(), currentValue.c_str());
			ConfMan.set(i->_key, i->_value);
		}
	}
	removeDifferentEntriesInDomain(ConfMan.getDomain(ConfMan.kApplicationDomain));
	removeDifferentEntriesInDomain(ConfMan.getActiveDomain());
	removeDifferentEntriesInDomain(ConfMan.getDomain(ConfMan.kTransientDomain));
}

void EventRecorder::removeDifferentEntriesInDomain(ConfigManager::Domain *domain) {
	for (ConfigManager::Domain::iterator entry = domain->begin(); entry!= domain->end(); ++entry) {
		if (_settingsRecords.find(entry->_key) == _settingsRecords.end()) {
			warning("Config value <%s>: %s -> (null)", entry->_key.c_str(), entry->_value.c_str());
			domain->erase(entry->_key);
		}
	}
}

void EventRecorder::saveScreenShot() {
	if (((_fakeTimer - _lastScreenshotTime) > _screenshotPeriod) && _headerDumped) {
		dumpRecordsToFile();
		_recordCount = 0;
		_lastScreenshotTime = _fakeTimer;
		uint8 md5[16];
		Graphics::Surface screen;
		if (!grabScreenAndComputeMD5(screen, md5)) {
			return;
		}
		_recordFile->writeUint32LE(MKTAG('M','D','5',' '));
		_recordFile->writeUint32LE(16);
		_recordFile->write(md5, 16);
		Graphics::saveThumbnail(*_recordFile, screen);
		screen.free();
	}
}

//TODO: Implement showing screenshots difference in case of different MD5 hashes
void EventRecorder::loadScreenShot() {
	skipScreenshot();
}

bool EventRecorder::grabScreenAndComputeMD5(Graphics::Surface &screen, uint8 md5[16]) {
	if (!createScreenShot(screen)) {
		warning("Can't save screenshot");
		return false;
	}	
	MemoryReadStream bitmapStream((const byte*)screen.pixels, screen.w * screen.h * screen.format.bytesPerPixel);
	computeStreamMD5(bitmapStream, md5);
	return true;
}

void EventRecorder::skipScreenshot() {
	uint32 screenShotSize;
	uint16 screenShotWidth;
	uint16 screenShotHeight;
	byte screenShotBpp;
	_playbackFile->skip(1); //skip version
	screenShotWidth = _playbackFile->readUint16BE();
	screenShotHeight = _playbackFile->readUint16BE();
	screenShotBpp = _playbackFile->readByte();
	_playbackFile->skip(screenShotWidth*screenShotHeight*screenShotBpp);
}

void EventRecorder::readEventsToBuffer(uint32 size) {
	_playbackFile->read(_recordBuffer, size);
	_tmpPlaybackFile.seek(0);
	_eventsSize = size;	
}

void EventRecorder::checkRecordedMD5() {
	uint8 currentMD5[16];
	uint8 savedMD5[16];
	Graphics::Surface screen;
	if (!grabScreenAndComputeMD5(screen, currentMD5)) {
		return;
	}
	_playbackFile->read(savedMD5, 16);
	if (memcmp(savedMD5, currentMD5, 16) != 0) {
		warning("Recorded and current screenshots are different");
	}
	Graphics::saveThumbnail(*_screenshotsFile, screen);
	screen.free();
}

DefaultTimerManager *EventRecorder::getTimerManager() {
	return _timerManager;
}

void EventRecorder::registerTimerManager(DefaultTimerManager *timerManager) {
	_timerManager = timerManager;
}

void EventRecorder::switchTimerManagers() {
	delete _timerManager;
	if (_recordMode == kPassthrough) {
		_timerManager = new SdlTimerManager();
	} else {
		_timerManager = new DefaultTimerManager();
	}
}

void EventRecorder::updateSubsystems() {
	if (_recordMode == kPassthrough) {
		return;
	}
	RecordMode oldRecordMode = _recordMode;
	_recordMode = kPassthrough;
	_fakeMixerManager->update();
	_recordMode = oldRecordMode;
}

} // End of namespace Common
