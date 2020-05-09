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

#ifndef COMMON_RECORDERFILE_H
#define COMMON_RECORDERFILE_H

#include "common/scummsys.h"
#include "common/events.h"
#include "common/mutex.h"
#include "common/memstream.h"
#include "common/config-manager.h"
#include "common/savefile.h"

//capacity of records buffer
#define kMaxBufferedRecords 10000
#define kRecordBuffSize sizeof(RecorderEvent) * kMaxBufferedRecords

namespace Common {

enum RecorderEventType {
	kRecorderEventTypeNormal = 0,
	kRecorderEventTypeTimer = 1
};

struct RecorderEvent : Event {
	RecorderEventType recordedtype;
	uint32 time;

	RecorderEvent() {
		recordedtype = kRecorderEventTypeNormal;
		time = 0;
	}

	RecorderEvent(const Event &e) : Event(e) {
		recordedtype = kRecorderEventTypeNormal;
		time = 0;
	}
};



class PlaybackFile {
	typedef HashMap<String, uint32, IgnoreCase_Hash, IgnoreCase_EqualTo> RandomSeedsDictionary;
	enum fileMode {
		kRead = 0,
		kWrite = 1,
		kClosed = 2
	};
	enum PlaybackFileState {
		kFileStateCheckFormat,
		kFileStateCheckVersion,
		kFileStateProcessHash,
		kFileStateProcessHeader,
		kFileStateProcessRandom,
		kFileStateSelectSection,
		kFileStateProcessSettings,
		kFileStateProcessSave,
		kFileStateDone,
		kFileStateError
	};
	enum FileTag {
		kFormatIdTag = MKTAG('P','B','C','K'),
		kVersionTag = MKTAG('V','E','R','S'),
		kHeaderSectionTag = MKTAG('H','E','A','D'),
		kHashSectionTag = MKTAG('H','A','S','H'),
		kRandomSectionTag = MKTAG('R','A','N','D'),
		kEventTag = MKTAG('E','V','N','T'),
		kScreenShotTag = MKTAG('B','M','H','T'),
		kSettingsSectionTag = MKTAG('S','E','T','T'),
		kAuthorTag = MKTAG('H','A','U','T'),
		kCommentsTag = MKTAG('H','C','M','T'),
		kNameTag = MKTAG('H','N','A','M'),
		kHashRecordTag = MKTAG('H','R','C','D'),
		kRandomRecordTag = MKTAG('R','R','C','D'),
		kSettingsRecordTag = MKTAG('S','R','E','C'),
		kSettingsRecordKeyTag = MKTAG('S','K','E','Y'),
		kSettingsRecordValueTag = MKTAG('S','V','A','L'),
		kSaveTag = MKTAG('S','A','V','E'),
		kSaveRecordTag = MKTAG('R','S','A','V'),
		kSaveRecordNameTag = MKTAG('S','N','A','M'),
		kSaveRecordBufferTag = MKTAG('S','B','U','F'),
		kMD5Tag = MKTAG('M','D','5',' ')
	};
	struct ChunkHeader {
		FileTag id;
		uint32 len;
	};
public:
	struct SaveFileBuffer {
		byte *buffer;
		uint32 size;
	};
	struct PlaybackFileHeader {
		String fileName;
		String author;
		String name;
		String notes;
		String description;
		StringMap hashRecords;
		StringMap settingsRecords;
		HashMap<String, SaveFileBuffer> saveFiles;
		RandomSeedsDictionary randomSourceRecords;
	};
	PlaybackFile();
	~PlaybackFile();

	bool openWrite(const String &fileName);
	bool openRead(const String &fileName);
	void close();

	RecorderEvent getNextEvent();
	void writeEvent(const RecorderEvent &event);

	void saveScreenShot(Graphics::Surface &screen, byte md5[16]);
	Graphics::Surface *getScreenShot(int number);
	int getScreensCount();

	bool isEventsBufferEmpty();
	PlaybackFileHeader &getHeader() {return _header;}
	void updateHeader();
	void addSaveFile(const String &fileName, InSaveFile *saveStream);
private:
	WriteStream *_recordFile;
	WriteStream *_writeStream;
	WriteStream *_screenshotsFile;
	MemoryReadStream _tmpPlaybackFile;
	SeekableReadStream *_readStream;
	SeekableMemoryWriteStream _tmpRecordFile;

	fileMode _mode;
	bool _headerDumped;
	int _recordCount;
	uint32 _eventsSize;
	byte _tmpBuffer[kRecordBuffSize];
	PlaybackFileHeader _header;
	PlaybackFileState _playbackParseState;

	void skipHeader();
	bool parseHeader();
	bool processChunk(ChunkHeader &nextChunk);
	void returnToChunkHeader();

	bool readSaveRecord();
	void checkRecordedMD5();
	bool readChunkHeader(ChunkHeader &nextChunk);
	void processRndSeedRecord(ChunkHeader chunk);
	bool processSettingsRecord();

	bool checkPlaybackFileVersion();

	void dumpHeaderToFile();
	void writeSaveFilesSection();
	void writeGameSettings();
	void writeHeaderSection();
	void writeGameHash();
	void writeRandomRecords();

	void dumpRecordsToFile();

	String readString(int len);
	void readHashMap(ChunkHeader chunk);

	bool skipToNextScreenshot();
	void readEvent(RecorderEvent& event);
	void readEventsToBuffer(uint32 size);
	bool grabScreenAndComputeMD5(Graphics::Surface &screen, uint8 md5[16]);
};

} // End of namespace Common

#endif
