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

#ifndef COMMON_EVENTRECORDER_H
#define COMMON_EVENTRECORDER_H

#include "common/scummsys.h"
#include "common/events.h"
#include "common/singleton.h"
#include "common/mutex.h"
#include "common/array.h"
#include "common/memstream.h"
#include "backends/keymapper/keymapper.h"
#include "backends/mixer/sdl/sdl-mixer.h"
#include "backends/mixer/nullmixer/nullsdl-mixer.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "backends/timer/sdl/sdl-timer.h"
#include "backends/timer/default/default-timer.h"
#include "engines/advancedDetector.h"
#include "common/hashmap.h"
#include "common/config-manager.h"

#define g_eventRec (Common::EventRecorder::instance())

//capacity of records buffer
#define kMaxBufferedRecords 10000
#define kRecordBuffSize sizeof(RecorderEvent) * kMaxBufferedRecords

namespace Common {

class RandomSource;
class SeekableReadStream;
class WriteStream;

struct RecorderEvent : Common::Event {
	uint32 time;
	uint32 count;
};

struct ChunkHeader {
	uint32 id;
	uint32 len;
};

/**
 * Our generic event recorder.
 *
 * TODO: Add more documentation.
 */
class EventRecorder : private EventSource, private EventObserver, public Singleton<EventRecorder>, private DefaultEventMapper {
	friend class Singleton<SingletonBaseType>;
	EventRecorder();
	~EventRecorder();
public:
	void init();
	void init(const ADGameDescription *desc);
	void deinit();
	bool processDelayMillis();
	/** TODO: Add documentation, this is only used by the backend */
	void processMillis(uint32 &millis);
	SdlMixerManager *getMixerManager();
	DefaultTimerManager *getTimerManager();
	/** Register random source so it can be serialized in game test purposes */
	uint32 getRandomSeed(const String &name);
	void init(Common::String gameid);
	void processGameDescription(const ADGameDescription *desc);
	void registerMixerManager(SdlMixerManager *mixerManager);
	void registerTimerManager(DefaultTimerManager *timerManager);
	uint32 getTimer() {return _fakeTimer;}
	void updateSubsystems();
	bool isRecording() {
		return initialized;
	}
	void RegisterEventSource();
private:	
	typedef HashMap<String, uint32, IgnoreCase_Hash, IgnoreCase_EqualTo> randomSeedsDictionary;
	enum PlaybackFileState {
		kFileStateCheckFormat,
		kFileStateCheckVersion,
		kFileStateProcessHash,
		kFileStateProcessHeader,
		kFileStateProcessRandom,
		kFileStateReadRnd,
		kFileStateSelectSection,
		kFileStateProcessSettings,
		kFileStateProcessSettingsRecord,
		kFileStateDone,
		kFileStateError
	};
	virtual List<Event> mapEvent(const Event &ev, EventSource *source);
	bool initialized;
	bool parsePlaybackFile();
	void setGameMd5(const ADGameDescription *gameDesc);
	ChunkHeader readChunkHeader();
	void getConfig();
	void applyPlaybackSettings();
	void removeDifferentEntriesInDomain(ConfigManager::Domain *domain);
	void getConfigFromDomain(ConfigManager::Domain *domain);
	bool processChunk(ChunkHeader &nextChunk);
	bool checkPlaybackFileVersion();
	void readAuthor(ChunkHeader chunk);
	void readComment(ChunkHeader chunk);
	void readHashMap(ChunkHeader chunk);
	void processRndSeedRecord(ChunkHeader chunk);
	bool processSettingsRecord(ChunkHeader chunk);
	bool _headerDumped;
	PlaybackFileState _playbackParseState;
	MutexRef _recorderMutex;
	SdlMixerManager *_realMixerManager;
	NullSdlMixerManager *_fakeMixerManager;
	DefaultTimerManager *_timerManager;
	void switchMixer();
	void switchFastMode();
	typedef HashMap<String, uint32, IgnoreCase_Hash, IgnoreCase_EqualTo> randomSeedsDictionary;
	void switchTimerManagers();
	void writeVersion();
	void writeHeader();
	void writeFormatId();
	void loadScreenShot();
	bool grabScreenAndComputeMD5(Graphics::Surface &screen, uint8 md5[16]);
	void skipScreenshot();
	void writeGameHash();
	void writeRandomRecords();
	bool openRecordFile(const String &gameId);
	bool checkGameHash(const ADGameDescription *desc);
	bool notifyEvent(const Event &ev);
	String getAuthor();
	String getComment();
	String findMD5ByFileName(const ADGameDescription *gameDesc, const String &fileName);
	Common::String readString(int len);
	bool notifyPoll();
	bool pollEvent(Event &ev);
	bool allowMapping() const { return false; }
	void getNextEvent();
	void readEvent(RecorderEvent &event);
	void writeEvent(const RecorderEvent &event);
	void checkForKeyCode(const Event &event);
	void writeGameSettings();
	void togglePause();
	void dumpRecordsToFile();
	void dumpHeaderToFile();
	void writeScreenSettings();
	void readEventsToBuffer(uint32 size);
	void processScreenSettings();
	uint32 _bitmapBuffSize;
	byte *_bitmapBuff;
	int _settingsSectionSize;
	RecorderEvent _nextEvent;
	randomSeedsDictionary _randomSourceRecords;
	StringMap _hashRecords;
	StringMap _settingsRecords;
	volatile uint32 _recordCount;
	volatile uint32 _recordSize;
	byte _recordBuffer[kRecordBuffSize];
	SeekableMemoryWriteStream _tmpRecordFile;
	uint32 _eventsSize;
	MemoryReadStream _tmpPlaybackFile;
	WriteStream *_recordFile;
	WriteStream *_screenshotsFile;
	MutexRef _timeMutex;
	volatile uint32 _lastMillis;
	volatile uint32 _fakeTimer;
	volatile uint32 _randomNumber;
	volatile uint32 _playbackDiff;
	volatile bool _hasPlaybackEvent;
	Event _playbackEvent;
	SeekableReadStream *_playbackFile;
	uint32 _lastScreenshotTime;
	uint32 _screenshotPeriod;
	void saveScreenShot();
	void checkRecordedMD5();
	enum RecordMode {
		kPassthrough = 0,
		kRecorderRecord = 1,
		kRecorderPlayback = 2,
		kRecorderPlaybackPause = 3
	};
	volatile RecordMode _recordMode;
	String _recordFileName;
	bool _fastPlayback;
};

} // End of namespace Common

#endif
