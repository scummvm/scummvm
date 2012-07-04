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
#include "common/config-manager.h"
#include "common/recorderfile.h"


#define g_eventRec (Common::EventRecorder::instance())



namespace Common {

class RandomSource;
class SeekableReadStream;
class WriteStream;


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
	enum RecordMode {
		kPassthrough = 0,
		kRecorderRecord = 1,
		kRecorderPlayback = 2,
		kRecorderPlaybackPause = 3
	};
	void init();
	void init(Common::String recordFileName, RecordMode mode);
	void init(const ADGameDescription *desc, RecordMode mode);
	void deinit();
	bool processDelayMillis();
	/** TODO: Add documentation, this is only used by the backend */
	void processMillis(uint32 &millis);
	SdlMixerManager *getMixerManager();
	DefaultTimerManager *getTimerManager();
	void setAuthor(const Common::String &author);
	void setNotes(const Common::String &desc);
	void setName(const Common::String &name);
	/** Register random source so it can be serialized in game test purposes */
	uint32 getRandomSeed(const String &name);
	void processGameDescription(const ADGameDescription *desc);
	void registerMixerManager(SdlMixerManager *mixerManager);
	void registerTimerManager(DefaultTimerManager *timerManager);
	uint32 getTimer() {return _fakeTimer;}
	void deleteRecord(const String& fileName);
	void updateSubsystems();
	bool isRecording() {
		return _initialized;
	}
	bool _savedState;
	void suspendRecording() {
		_savedState = _initialized;
		_initialized = false;
	}
	void resumeRecording() {
		_initialized = _savedState;
	}
	void RegisterEventSource();
	Common::String generateRecordFileName(const String &target);
private:	
	virtual List<Event> mapEvent(const Event &ev, EventSource *source);
	bool _initialized;
	void setGameMd5(const ADGameDescription *gameDesc);
	void getConfig();
	void applyPlaybackSettings();
	void removeDifferentEntriesInDomain(ConfigManager::Domain *domain);
	void getConfigFromDomain(ConfigManager::Domain *domain);
	MutexRef _recorderMutex;
	SdlMixerManager *_realMixerManager;
	NullSdlMixerManager *_fakeMixerManager;
	DefaultTimerManager *_timerManager;
	void switchMixer();
	void switchFastMode();
	typedef HashMap<String, uint32, IgnoreCase_Hash, IgnoreCase_EqualTo> randomSeedsDictionary;
	void switchTimerManagers();
	bool grabScreenAndComputeMD5(Graphics::Surface &screen, uint8 md5[16]);
	bool openRecordFile(const String &fileName);
	bool checkGameHash(const ADGameDescription *desc);
	bool notifyEvent(const Event &ev);
	String findMD5ByFileName(const ADGameDescription *gameDesc, const String &fileName);
	bool notifyPoll();
	bool pollEvent(Event &ev);
	bool allowMapping() const { return false; }
	void checkForKeyCode(const Event &event);
	void writeGameSettings();
	void togglePause();
	RecorderEvent _nextEvent;
	WriteStream *_screenshotsFile;
	MutexRef _timeMutex;
	volatile uint32 _lastMillis;
	volatile uint32 _fakeTimer;
	uint32 _lastScreenshotTime;
	uint32 _screenshotPeriod;
	PlaybackFile _playbackFile;
	void saveScreenShot();
	void checkRecordedMD5();
	volatile RecordMode _recordMode;
	String _recordFileName;
	bool _fastPlayback;
};

} // End of namespace Common

#endif
