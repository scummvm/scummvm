/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/system.h"
#include "gui/EventRecorder.h"
#include "common/md5.h"
#include "common/recorderfile.h"
#include "common/savefile.h"
#include "common/bufferedstream.h"
#include "graphics/thumbnail.h"
#include "graphics/surface.h"
#include "graphics/scaler.h"

#define RECORD_VERSION 1

namespace Common {

PlaybackFile::PlaybackFile()
	: _tmpBuffer(kRecordBuffSize)
	, _tmpRecordFile(_tmpBuffer.data(), kRecordBuffSize)
	, _tmpPlaybackFile(_tmpBuffer.data(), kRecordBuffSize) {
	_readStream = NULL;
	_writeStream = NULL;
	_screenshotsFile = NULL;
	_mode = kClosed;

	_recordFile = 0;
	_headerDumped = false;
	_recordCount = 0;
	_eventsSize = 0;
	_version = RECORD_VERSION;
	memset(_tmpBuffer.data(), 1, kRecordBuffSize);

	_playbackParseState = kFileStateCheckFormat;
}

PlaybackFile::~PlaybackFile() {
	close();
}

bool PlaybackFile::openWrite(const String &fileName) {
	close();
	_header.fileName = fileName;
	_writeStream = wrapBufferedWriteStream(g_system->getSavefileManager()->openForSaving(fileName), 128 * 1024);
	_headerDumped = false;
	_recordCount = 0;
	if (_writeStream == NULL) {
		return false;
	}
	_mode = kWrite;
	return true;
}

bool PlaybackFile::openRead(const String &fileName) {
	close();
	_header.fileName = fileName;
	_eventsSize = 0;
	_tmpPlaybackFile.seek(0);
	_readStream = wrapBufferedSeekableReadStream(g_system->getSavefileManager()->openForLoading(fileName), 128 * 1024, DisposeAfterUse::YES);
	if (_readStream == NULL) {
		debugC(1, kDebugLevelEventRec, "playback:action=\"Load File\" result=fail reason=\"file %s not found\"", fileName.c_str());
		return false;
	}
	if (!parseHeader()) {
		debugC(1, kDebugLevelEventRec, "playback:action=\"Load File\" result=fail reason=\"header parsing failed\"");
		return false;
	}
	_screenshotsFile = wrapBufferedWriteStream(g_system->getSavefileManager()->openForSaving("screenshots.bin"), 128 * 1024);
	debugC(1, kDebugLevelEventRec, "playback:action=\"Load File\" result=success");
	_mode = kRead;
	return true;
}

void PlaybackFile::close() {
	delete _readStream;
	_readStream = NULL;
	if (_writeStream != NULL) {
		dumpRecordsToFile();
		_writeStream->finalize();
		delete _writeStream;
		_writeStream = NULL;
		updateHeader();
	}
	if (_screenshotsFile != NULL) {
		_screenshotsFile->finalize();
		delete _screenshotsFile;
		_screenshotsFile = NULL;
	}
	for (HashMap<String, SaveFileBuffer>::iterator  i = _header.saveFiles.begin(); i != _header.saveFiles.end(); ++i) {
		free(i->_value.buffer);
	}
	_header.saveFiles.clear();
	_mode = kClosed;
}

bool PlaybackFile::parseHeader() {
	ChunkHeader nextChunk;
	_playbackParseState = kFileStateCheckFormat;
	if (!readChunkHeader(nextChunk)) {
		_playbackParseState = kFileStateError;
		return false;
	}
	while ((_playbackParseState != kFileStateDone) && (_playbackParseState != kFileStateError)) {
		if (processChunk(nextChunk)) {
			if (!readChunkHeader(nextChunk)) {
				warning("Error in header parsing");
				_playbackParseState = kFileStateError;
			}
		}
	}
	return _playbackParseState == kFileStateDone;
}

bool PlaybackFile::checkPlaybackFileVersion() {
	_version = _readStream->readUint32BE();
	switch (_version) {
	case 1:
		break;
	default:
		warning("Unknown playback file version %d. Maximum supported version is %d.", _version, RECORD_VERSION);
		return false;
	}
	if (_version != RECORD_VERSION) {
		warning("This playback file uses version %d, and may not be accurate. You can re-record the file in version %d format using --record-mode=update", _version, RECORD_VERSION);
	}
	return true;
}


String PlaybackFile::readString(int len) {
	String result;
	char buf[50];
	int readSize = 49;
	while (len > 0)	{
		if (len <= 49) {
			readSize = len;
		}
		_readStream->read(buf, readSize);
		buf[readSize] = 0;
		result += buf;
		len -= readSize;
	}
	return result;
}

bool PlaybackFile::readChunkHeader(PlaybackFile::ChunkHeader &nextChunk) {
	nextChunk.id = (FileTag)_readStream->readUint32BE();
	nextChunk.len = _readStream->readUint32BE();
	return !_readStream->err() && !_readStream->eos();
}

bool PlaybackFile::processChunk(ChunkHeader &nextChunk) {
	switch (_playbackParseState) {
	case kFileStateCheckFormat:
		if (nextChunk.id == kFormatIdTag) {
			_playbackParseState = kFileStateCheckVersion;
		} else {
			warning("Unknown playback file signature");
			_playbackParseState = kFileStateError;
		}
		break;
	case kFileStateCheckVersion:
		if ((nextChunk.id == kVersionTag) && checkPlaybackFileVersion()) {
			_playbackParseState = kFileStateSelectSection;
		} else {
			_playbackParseState = kFileStateError;
		}
		break;
	case kFileStateSelectSection:
		switch (nextChunk.id) {
		case kHeaderSectionTag:
			_playbackParseState = kFileStateProcessHeader;
			break;
		case kHashSectionTag:
			_playbackParseState = kFileStateProcessHash;
			break;
		case kRandomSectionTag:
			_playbackParseState = kFileStateProcessRandom;
			break;
		case kEventTag:
		case kScreenShotTag:
			_readStream->seek(-8, SEEK_CUR);
			_playbackParseState = kFileStateDone;
			return false;
		case kSaveTag:
			_playbackParseState = kFileStateProcessSave;
			break;
		case kSettingsSectionTag:
			_playbackParseState = kFileStateProcessSettings;
			warning("Loading record header");
			break;
		default:
			_readStream->skip(nextChunk.len);
			break;
		}
		break;
	case kFileStateProcessSave:
		if (nextChunk.id == kSaveRecordTag) {
			readSaveRecord();
		} else {
			_playbackParseState = kFileStateSelectSection;
			return false;
		}
		break;
	case kFileStateProcessHeader:
		switch (nextChunk.id) {
		case kAuthorTag:
			_header.author = readString(nextChunk.len);
			break;
		case kCommentsTag:
			_header.notes = readString(nextChunk.len);
			break;
		case kNameTag:
			_header.name = readString(nextChunk.len);
			break;
		default:
			_playbackParseState = kFileStateSelectSection;
			return false;
		}
		break;
	case kFileStateProcessHash:
		if (nextChunk.id == kHashRecordTag) {
			readHashMap(nextChunk);
		} else {
			_playbackParseState = kFileStateSelectSection;
			return false;
		}
		break;
	case kFileStateProcessRandom:
		if (nextChunk.id == kRandomRecordTag) {
			processRndSeedRecord(nextChunk);
		} else {
			_playbackParseState = kFileStateSelectSection;
			return false;
		}
		break;
	case kFileStateProcessSettings:
		if (nextChunk.id == kSettingsRecordTag) {
			if (!processSettingsRecord()) {
				_playbackParseState = kFileStateError;
				return false;
			}
		} else {
			_playbackParseState = kFileStateSelectSection;
			return false;
		}
		break;
	default:
		return false;
	}
	return true;
}

void PlaybackFile::returnToChunkHeader() {
	_readStream->seek(-8, SEEK_CUR);
}

void PlaybackFile::readHashMap(ChunkHeader chunk) {
	String hashName = readString(chunk.len - 32);
	String hashMd5 = readString(32);
	_header.hashRecords[hashName] = hashMd5;
}

void PlaybackFile::processRndSeedRecord(ChunkHeader chunk) {
	String randomSourceName = readString(chunk.len - 4);
	uint32 randomSourceSeed = _readStream->readUint32BE();
	_header.randomSourceRecords[randomSourceName] = randomSourceSeed;
}

bool PlaybackFile::processSettingsRecord() {
	ChunkHeader keyChunk;
	if (!readChunkHeader(keyChunk) || (keyChunk.id != kSettingsRecordKeyTag)) {
		warning("Invalid format of settings section");
		return false;
	}
	String key = readString(keyChunk.len);
	ChunkHeader valueChunk;
	if (!readChunkHeader(valueChunk) || (valueChunk.id != kSettingsRecordValueTag)) {
		warning("Invalid format of settings section");
		return false;
	}
	String value = readString(valueChunk.len);
	_header.settingsRecords[key] = value;
	return true;
}


bool PlaybackFile::readSaveRecord() {
	ChunkHeader fileNameChunk;
	if (!readChunkHeader(fileNameChunk) || (fileNameChunk.id != kSaveRecordNameTag)) {
		warning("Invalid format of save section");
		return false;
	}
	String fileName = readString(fileNameChunk.len);
	ChunkHeader saveBufferChunk;
	if (!readChunkHeader(saveBufferChunk) || (saveBufferChunk.id != kSaveRecordBufferTag)) {
		warning("Invalid format of save section");
		return false;
	}
	SaveFileBuffer buf;
	buf.size = saveBufferChunk.len;
	buf.buffer = (byte *)malloc(saveBufferChunk.len);
	_readStream->read(buf.buffer, buf.size);
	_header.saveFiles[fileName] = buf;
	debugC(1, kDebugLevelEventRec, "playback:action=\"Load save file\" filename=%s len=%d", fileName.c_str(), buf.size);
	return true;
}

bool PlaybackFile::hasNextEvent() const {
	if (_readStream->err()) {
		return false;
	}
	return !_readStream->eos();
}

RecorderEvent PlaybackFile::getNextEvent() {
	if (!hasNextEvent()) {
		debug(3, "end of recorder file reached.");
		g_system->quit();
	}

	assert(_mode == kRead);
	if (isEventsBufferEmpty()) {
		PlaybackFile::ChunkHeader header;
		header.id = kFormatIdTag;
		while (header.id != kEventTag) {
			if (!readChunkHeader(header) || _readStream->eos()) {
				break;
			}
			switch (header.id) {
			case kEventTag:
				readEventsToBuffer(header.len);
				break;
			case kScreenShotTag:
				_readStream->seek(-4, SEEK_CUR);
				header.len = _readStream->readUint32BE();
				_readStream->skip(header.len - 8);
				break;
			case kMD5Tag:
				checkRecordedMD5();
				break;
			default:
				_readStream->skip(header.len);
				break;
			}
		}
	}
	RecorderEvent result;
	readEvent(result);
	return result;
}

bool PlaybackFile::isEventsBufferEmpty() {
	return (uint32)_tmpPlaybackFile.pos() == _eventsSize;
}

void PlaybackFile::readEvent(RecorderEvent& event) {
	event.recordedtype = (RecorderEventType)_tmpPlaybackFile.readByte();
	switch (event.recordedtype) {
	case kRecorderEventTypeTimer:
		event.time = _tmpPlaybackFile.readUint32BE();
		break;
	case kRecorderEventTypeTimeDate:
		event.timeDate.tm_sec = _tmpPlaybackFile.readSint32BE();
		event.timeDate.tm_min = _tmpPlaybackFile.readSint32BE();
		event.timeDate.tm_hour = _tmpPlaybackFile.readSint32BE();
		event.timeDate.tm_mday = _tmpPlaybackFile.readSint32BE();
		event.timeDate.tm_mon = _tmpPlaybackFile.readSint32BE();
		event.timeDate.tm_year = _tmpPlaybackFile.readSint32BE();
		event.timeDate.tm_wday = _tmpPlaybackFile.readSint32BE();
		break;
	case kRecorderEventTypeScreenUpdate:
		event.time = _tmpPlaybackFile.readUint32BE();
		break;
	default:
		// fallthrough intended
	case kRecorderEventTypeNormal:
		event.type = (EventType)_tmpPlaybackFile.readUint32BE();
		switch (event.type) {
		case EVENT_KEYDOWN:
			event.kbdRepeat = _tmpPlaybackFile.readByte();
			// fallthrough
		case EVENT_KEYUP:
			event.time = _tmpPlaybackFile.readUint32BE();
			event.kbd.keycode = (KeyCode)_tmpPlaybackFile.readSint32BE();
			event.kbd.ascii = _tmpPlaybackFile.readUint16BE();
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
		case EVENT_X1BUTTONDOWN:
		case EVENT_X1BUTTONUP:
		case EVENT_X2BUTTONDOWN:
		case EVENT_X2BUTTONUP:
			event.time = _tmpPlaybackFile.readUint32BE();
			event.mouse.x = _tmpPlaybackFile.readSint16BE();
			event.mouse.y = _tmpPlaybackFile.readSint16BE();
			break;
		case EVENT_CUSTOM_BACKEND_ACTION_START:
		case EVENT_CUSTOM_BACKEND_ACTION_END:
		case EVENT_CUSTOM_ENGINE_ACTION_START:
		case EVENT_CUSTOM_ENGINE_ACTION_END:
			event.time = _tmpPlaybackFile.readUint32BE();
			event.customType = _tmpPlaybackFile.readUint32BE();
			break;
		case EVENT_JOYAXIS_MOTION:
		case EVENT_JOYBUTTON_UP:
		case EVENT_JOYBUTTON_DOWN:
			event.time = _tmpPlaybackFile.readUint32BE();
			event.joystick.axis = _tmpPlaybackFile.readByte();
			event.joystick.button = _tmpPlaybackFile.readByte();
			event.joystick.position = _tmpPlaybackFile.readSint16BE();
			break;
		default:
			event.time = _tmpPlaybackFile.readUint32BE();
			break;
		}
		break;
	}
	debug(3, "read event of recordedtype: %i, type: %i (time: %u, systemmillis: %u)",
			event.recordedtype, event.type, event.time, g_system->getMillis(true));
}

void PlaybackFile::readEventsToBuffer(uint32 size) {
	_readStream->read(_tmpBuffer.data(), size);
	_tmpPlaybackFile.seek(0);
	_eventsSize = size;
}

void PlaybackFile::saveScreenShot(Graphics::Surface &screen, byte md5[16]) {
	dumpRecordsToFile();
	_writeStream->writeUint32BE(kMD5Tag);
	_writeStream->writeUint32BE(16);
	_writeStream->write(md5, 16);
	Graphics::saveThumbnail(*_writeStream, screen);
}

void PlaybackFile::dumpRecordsToFile() {
	if (!_headerDumped) {
		dumpHeaderToFile();
		_headerDumped = true;
	}
	if (_recordCount == 0) {
		return;
	}
	_writeStream->writeUint32BE(kEventTag);
	_writeStream->writeUint32BE(_tmpRecordFile.pos());
	_writeStream->write(_tmpBuffer.data(), _tmpRecordFile.pos());
	_tmpRecordFile.seek(0);
	_recordCount = 0;
}

void PlaybackFile::dumpHeaderToFile() {
	_writeStream->writeUint32BE(kFormatIdTag);
	// Specify size for first tag as NULL since we cannot calculate
	// size of the file at time of the header dumping
	_writeStream->writeUint32BE(0);
	_writeStream->writeUint32BE(kVersionTag);
	_writeStream->writeUint32BE(4);
	_writeStream->writeUint32BE(RECORD_VERSION);
	writeHeaderSection();
	writeGameHash();
	writeRandomRecords();
	writeGameSettings();
	writeSaveFilesSection();
}

void PlaybackFile::writeHeaderSection() {
	uint32 headerSize = 0;
	if (!_header.author.empty()) {
		headerSize = _header.author.size() + 8;
	}
	if (!_header.notes.empty()) {
		headerSize += _header.notes.size() + 8;
	}
	if (!_header.name.empty()) {
		headerSize += _header.name.size() + 8;
	}
	if (headerSize == 0) {
		return;
	}
	_writeStream->writeUint32BE(kHeaderSectionTag);
	_writeStream->writeUint32BE(headerSize);
	if (!_header.author.empty()) {
		_writeStream->writeUint32BE(kAuthorTag);
		_writeStream->writeUint32BE(_header.author.size());
		_writeStream->writeString(_header.author);
	}
	if (!_header.notes.empty()) {
		_writeStream->writeUint32BE(kCommentsTag);
		_writeStream->writeUint32BE(_header.notes.size());
		_writeStream->writeString(_header.notes);
	}
	if (!_header.name.empty()) {
		_writeStream->writeUint32BE(kNameTag);
		_writeStream->writeUint32BE(_header.name.size());
		_writeStream->writeString(_header.name);
	}
}

void PlaybackFile::writeGameHash() {
	uint32 hashSectionSize = 0;
	for (StringMap::iterator i = _header.hashRecords.begin(); i != _header.hashRecords.end(); ++i) {
		hashSectionSize = hashSectionSize + i->_key.size() + i->_value.size() + 8;
	}
	if (_header.hashRecords.size() == 0) {
		return;
	}
	_writeStream->writeUint32BE(kHashSectionTag);
	_writeStream->writeUint32BE(hashSectionSize);
	for (StringMap::iterator i = _header.hashRecords.begin(); i != _header.hashRecords.end(); ++i) {
		_writeStream->writeUint32BE(kHashRecordTag);
		_writeStream->writeUint32BE(i->_key.size() + i->_value.size());
		_writeStream->writeString(i->_key);
		_writeStream->writeString(i->_value);
	}
}

void PlaybackFile::writeRandomRecords() {
	uint32 randomSectionSize = 0;
	for (RandomSeedsDictionary::iterator i = _header.randomSourceRecords.begin(); i != _header.randomSourceRecords.end(); ++i) {
		randomSectionSize = randomSectionSize + i->_key.size() + 12;
	}
	if (_header.randomSourceRecords.size() == 0) {
		return;
	}
	_writeStream->writeUint32BE(kRandomSectionTag);
	_writeStream->writeUint32BE(randomSectionSize);
	for (RandomSeedsDictionary::iterator i = _header.randomSourceRecords.begin(); i != _header.randomSourceRecords.end(); ++i) {
		_writeStream->writeUint32BE(kRandomRecordTag);
		_writeStream->writeUint32BE(i->_key.size() + 4);
		_writeStream->writeString(i->_key);
		_writeStream->writeUint32BE(i->_value);
	}
}

void PlaybackFile::writeEvent(const RecorderEvent &event) {
	assert(_mode == kWrite);
	_recordCount++;
	_tmpRecordFile.writeByte(event.recordedtype);
	switch (event.recordedtype) {
	case kRecorderEventTypeTimer:
		_tmpRecordFile.writeUint32BE(event.time);
		break;
	case kRecorderEventTypeTimeDate:
		_tmpRecordFile.writeSint32BE(event.timeDate.tm_sec);
		_tmpRecordFile.writeSint32BE(event.timeDate.tm_min);
		_tmpRecordFile.writeSint32BE(event.timeDate.tm_hour);
		_tmpRecordFile.writeSint32BE(event.timeDate.tm_mday);
		_tmpRecordFile.writeSint32BE(event.timeDate.tm_mon);
		_tmpRecordFile.writeSint32BE(event.timeDate.tm_year);
		_tmpRecordFile.writeSint32BE(event.timeDate.tm_wday);
		break;
	case kRecorderEventTypeScreenUpdate:
		_tmpRecordFile.writeUint32BE(event.time);
		break;
	default:
		// fallthrough intended
	case kRecorderEventTypeNormal:
		_tmpRecordFile.writeUint32BE((uint32)event.type);
		switch(event.type) {
		case EVENT_KEYDOWN:
			_tmpRecordFile.writeByte(event.kbdRepeat);
			// fallthrough
		case EVENT_KEYUP:
			_tmpRecordFile.writeUint32BE(event.time);
			_tmpRecordFile.writeSint32BE(event.kbd.keycode);
			_tmpRecordFile.writeUint16BE(event.kbd.ascii);
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
		case EVENT_X1BUTTONDOWN:
		case EVENT_X1BUTTONUP:
		case EVENT_X2BUTTONDOWN:
		case EVENT_X2BUTTONUP:
			_tmpRecordFile.writeUint32BE(event.time);
			_tmpRecordFile.writeSint16BE(event.mouse.x);
			_tmpRecordFile.writeSint16BE(event.mouse.y);
			break;
		case EVENT_CUSTOM_BACKEND_ACTION_START:
		case EVENT_CUSTOM_BACKEND_ACTION_END:
		case EVENT_CUSTOM_ENGINE_ACTION_START:
		case EVENT_CUSTOM_ENGINE_ACTION_END:
			_tmpRecordFile.writeUint32BE(event.time);
			_tmpRecordFile.writeUint32BE(event.customType);
			break;
		case EVENT_JOYAXIS_MOTION:
		case EVENT_JOYBUTTON_UP:
		case EVENT_JOYBUTTON_DOWN:
			_tmpRecordFile.writeUint32BE(event.time);
			_tmpRecordFile.writeByte(event.joystick.axis);
			_tmpRecordFile.writeByte(event.joystick.button);
			_tmpRecordFile.writeSint16BE(event.joystick.position);
			break;
		default:
			_tmpRecordFile.writeUint32BE(event.time);
			break;
		}
		break;
	}
	if (_recordCount == kMaxBufferedRecords) {
		dumpRecordsToFile();
	}
	debug(3, "write event of recordedtype: %i, type: %i (time: %u, systemmillis: %u)",
			event.recordedtype, event.type, event.time, g_system->getMillis(true));
}

void PlaybackFile::writeGameSettings() {
	_writeStream->writeUint32BE(kSettingsSectionTag);
	uint32 settingsSectionSize = 0;
	for (StringMap::iterator i = _header.settingsRecords.begin(); i != _header.settingsRecords.end(); ++i) {
		settingsSectionSize += i->_key.size() + i->_value.size() + 24;
	}
	_writeStream->writeUint32BE(settingsSectionSize);
	for (StringMap::iterator i = _header.settingsRecords.begin(); i != _header.settingsRecords.end(); ++i) {
		_writeStream->writeUint32BE(kSettingsRecordTag);
		_writeStream->writeUint32BE(i->_key.size() + i->_value.size() + 16);
		_writeStream->writeUint32BE(kSettingsRecordKeyTag);
		_writeStream->writeUint32BE(i->_key.size());
		_writeStream->writeString(i->_key);
		_writeStream->writeUint32BE(kSettingsRecordValueTag);
		_writeStream->writeUint32BE(i->_value.size());
		_writeStream->writeString(i->_value);
	}
}

int PlaybackFile::getScreensCount() {
	if (_mode != kRead) {
		return 0;
	}
	_readStream->seek(0);
	int result = 0;
	while (skipToNextScreenshot()) {
		uint32 size = _readStream->readUint32BE();
		_readStream->skip(size - 8);
		++result;
	}
	return result;
}

bool PlaybackFile::skipToNextScreenshot() {
	while (!_readStream->eos() && !_readStream->err()) {
		FileTag id = (FileTag)_readStream->readUint32BE();
		if (_readStream->eos() || _readStream->err()) {
			break;
		}
		if (id == kScreenShotTag) {
			return true;
		}
		uint32 size = _readStream->readUint32BE();
		_readStream->skip(size);
	}
	return false;
}

Graphics::Surface *PlaybackFile::getScreenShot(int number) {
	if (_mode != kRead) {
		return NULL;
	}
	_readStream->seek(0);
	int screenCount = 1;
	while (skipToNextScreenshot()) {
		if (screenCount == number) {
			screenCount++;
			_readStream->seek(-4, SEEK_CUR);
			Graphics::Surface *thumbnail = nullptr;
			return Graphics::loadThumbnail(*_readStream, thumbnail) ? thumbnail : NULL;
		} else {
			uint32 size = _readStream->readUint32BE();
			_readStream->skip(size - 8);
			screenCount++;
		}
	}
	return NULL;
}

void PlaybackFile::updateHeader() {
	if (_mode == kWrite) {
		StringArray dummy;
		g_system->getSavefileManager()->updateSavefilesList(dummy);
		_readStream = g_system->getSavefileManager()->openForLoading(_header.fileName);

		assert (_readStream);
	}
	_readStream->seek(0);
	skipHeader();
	String tmpFilename = "_" + _header.fileName;
	_writeStream = g_system->getSavefileManager()->openForSaving(tmpFilename);
	dumpHeaderToFile();
	uint32 readedSize = 0;
	do {
		readedSize = _readStream->read(_tmpBuffer.data(), kRecordBuffSize);
		_writeStream->write(_tmpBuffer.data(), readedSize);
	} while (readedSize != 0);
	delete _writeStream;
	_writeStream = NULL;
	delete _readStream;
	_readStream = NULL;
	g_system->getSavefileManager()->removeSavefile(_header.fileName);
	g_system->getSavefileManager()->renameSavefile(tmpFilename, _header.fileName);
	if (_mode == kRead) {
		openRead(_header.fileName);
	}
}

void PlaybackFile::skipHeader() {
	while (true) {
		uint32 id = _readStream->readUint32BE();
		if (_readStream->eos()) {
			break;
		}
		if ((id == kScreenShotTag) || (id == kEventTag) || (id == kMD5Tag)) {
			_readStream->seek(-4, SEEK_CUR);
			return;
		}
		else {
			uint32 size = _readStream->readUint32BE();
			_readStream->skip(size);
		}
	}
}

void PlaybackFile::addSaveFile(const String &fileName, InSaveFile *saveStream) {
	uint oldPos = saveStream->pos();
	saveStream->seek(0);
	_header.saveFiles[fileName].buffer = (byte *)malloc(saveStream->size());
	_header.saveFiles[fileName].size = saveStream->size();
	saveStream->read(_header.saveFiles[fileName].buffer, saveStream->size());
	saveStream->seek(oldPos);
}

void PlaybackFile::writeSaveFilesSection() {
	uint size = 0;
	for (HashMap<String, SaveFileBuffer>::iterator  i = _header.saveFiles.begin(); i != _header.saveFiles.end(); ++i) {
		size += i->_value.size + i->_key.size() + 24;
	}
	if (size == 0) {
		return;
	}
	_writeStream->writeSint32BE(kSaveTag);
	_writeStream->writeSint32BE(size);
	for (HashMap<String, SaveFileBuffer>::iterator  i = _header.saveFiles.begin(); i != _header.saveFiles.end(); ++i) {
		_writeStream->writeSint32BE(kSaveRecordTag);
		_writeStream->writeSint32BE(i->_key.size() + i->_value.size + 16);
		_writeStream->writeSint32BE(kSaveRecordNameTag);
		_writeStream->writeSint32BE(i->_key.size());
		_writeStream->writeString(i->_key);
		_writeStream->writeSint32BE(kSaveRecordBufferTag);
		_writeStream->writeSint32BE(i->_value.size);
		_writeStream->write(i->_value.buffer, i->_value.size);
	}
}


void PlaybackFile::checkRecordedMD5() {
	uint8 currentMD5[16];
	uint8 savedMD5[16];
	Graphics::Surface screen;
	_readStream->read(savedMD5, 16);
	if (!g_eventRec.grabScreenAndComputeMD5(screen, currentMD5)) {
		return;
	}
	uint32 seconds = g_system->getMillis(true) / 1000;
	String screenTime = String::format("%.2d:%.2d:%.2d", seconds / 3600 % 24, seconds / 60 % 60, seconds % 60);
	if (memcmp(savedMD5, currentMD5, 16) != 0) {
		debugC(1, kDebugLevelEventRec, "playback:action=\"Check screenshot\" time=%s result = fail", screenTime.c_str());
		warning("Recorded and current screenshots are different");
	} else {
		debugC(1, kDebugLevelEventRec, "playback:action=\"Check screenshot\" time=%s result = success", screenTime.c_str());
	}
	Graphics::saveThumbnail(*_screenshotsFile, screen);
	screen.free();
}


}
