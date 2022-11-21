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

#ifndef GUI_EVENTRECORDER_H
#define GUI_EVENTRECORDER_H

#include "common/system.h"

#include "common/events.h"
#include "common/savefile.h"
#include "common/singleton.h"

#include "engines/advancedDetector.h"

#ifdef ENABLE_EVENTRECORDER

#include "common/mutex.h"
#include "common/array.h"
#include "common/memstream.h"
#include "backends/mixer/mixer.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "backends/timer/sdl/sdl-timer.h"
#include "common/config-manager.h"
#include "common/recorderfile.h"
#include "backends/saves/recorder/recorder-saves.h"
#include "backends/mixer/null/null-mixer.h"
#include "backends/saves/default/default-saves.h"


#define g_eventRec (GUI::EventRecorder::instance())

namespace GUI {
	class OnScreenDialog;
}

namespace GUI {
class RandomSource;
class SeekableReadStream;
class WriteStream;


/**
 * Our generic event recorder.
 *
 * TODO: Add more documentation.
 */
class EventRecorder : private Common::EventSource, public Common::Singleton<EventRecorder>, private Common::EventObserver {
	friend class Common::Singleton<SingletonBaseType>;
	EventRecorder();
	~EventRecorder() override;
public:
	/** Specify operation mode of Event Recorder */
	enum RecordMode {
		kPassthrough = 0,		/**< kPassthrough, do nothing */
		kRecorderRecord = 1,		/**< kRecorderRecord, do the recording */
		kRecorderPlayback = 2,		/**< kRecorderPlayback, playback existing recording */
		kRecorderPlaybackPause = 3,	/**< kRecorderPlaybackPause, internal state when user pauses the playback */
		kRecorderUpdate = 4			/**< kRecorderUpdate, playback existing recording and update all hashes */
	};

	void init(const Common::String &recordFileName, RecordMode mode);
	void deinit();
	bool processDelayMillis();
	uint32 getRandomSeed(const Common::String &name);
	void processTimeAndDate(TimeDate &td, bool skipRecord);
	void processMillis(uint32 &millis, bool skipRecord);
	void processScreenUpdate();
	void processGameDescription(const ADGameDescription *desc);
	bool processAutosave();
	Common::SeekableReadStream *processSaveStream(const Common::String & fileName);
	void processHaveDoubleClickTime(bool &haveDoubleClickTime);
	void processDoubleClickTime(uint32 &doubleClickTime);

	/** Hooks for intercepting into GUI processing, so required events could be shoot
	 *  or filtered out */
	void preDrawOverlayGui();
	void postDrawOverlayGui();

	/** Set recording author
	 *
	 *  @see getAuthor
	 */
	void setAuthor(const Common::String &author) {
		_author = author;
	}

	/** Set recording notes
	 *
	 *  @see getNotes
	 */
	void setNotes(const Common::String &desc){
		_desc = desc;
	}

	/** Set descriptive name of the recording
	 *
	 *  @see getName
	 */
	void setName(const Common::String &name) {
		_name = name;
	}

	/** Get recording author
	 *
	 *  @see getAuthor
	 */
	const Common::String getAuthor() {
		return _author;
	}

	/** Get recording notes
	 *
	 *  @see setNotes
	 */
	const Common::String getNotes() {
		return _desc;
	}

	/** Get recording name
	 *
	 *  @see setName
	 */
	const Common::String getName() {
		return _name;
	}
	void setRedraw(bool redraw) {
		_needRedraw = redraw;
	}

	void registerMixerManager(MixerManager *mixerManager);
	void registerTimerManager(DefaultTimerManager *timerManager);

	MixerManager *getMixerManager();
	DefaultTimerManager *getTimerManager();

	void deleteRecord(const Common::String& fileName);
	bool checkForContinueGame();

	void acquireRecording() {
		assert(_acquireCount >= 0);
		if (_acquireCount == 0) {
			_savedState = _initialized;
			_initialized = false;
		}
		_acquireCount += 1;
	}

	void releaseRecording() {
		assert(_acquireCount > 0);
		_acquireCount -= 1;
		if (_acquireCount == 0)
			_initialized = _savedState;
	}

	RecordMode getRecordMode() const {
		return _recordMode;
	}

	Common::StringArray listSaveFiles(const Common::String &pattern);
	Common::String generateRecordFileName(const Common::String &target);

	Common::SaveFileManager *getSaveManager(Common::SaveFileManager *realSaveManager);
	SDL_Surface *getSurface(int width, int height);
	void RegisterEventSource();

	/** Retrieve game screenshot and compute its checksum for comparison */
	bool grabScreenAndComputeMD5(Graphics::Surface &screen, uint8 md5[16]);

	void updateSubsystems();
	bool switchMode();
	void switchFastMode();

private:
	bool pollEvent(Common::Event &ev) override;
	bool notifyEvent(const Common::Event &event) override;
	bool _initialized;
	volatile uint32 _fakeTimer;
	TimeDate _lastTimeDate;
	bool _savedState;
	int _acquireCount;
	bool _needcontinueGame;
	int _temporarySlot;
	Common::String _author;
	Common::String _desc;
	Common::String _name;

	Common::SaveFileManager *_realSaveManager;
	MixerManager *_realMixerManager;
	DefaultTimerManager *_timerManager;
	RecorderSaveFileManager _fakeSaveManager;
	NullMixerManager *_fakeMixerManager;
	GUI::OnScreenDialog *_controlPanel;
	Common::RecorderEvent _nextEvent;

	void setFileHeader();
	void setGameMd5(const ADGameDescription *gameDesc);
	void getConfig();
	void getConfigFromDomain(const Common::ConfigManager::Domain *domain);
	void removeDifferentEntriesInDomain(Common::ConfigManager::Domain *domain);
	void applyPlaybackSettings();

	void switchMixer();
	void switchTimerManagers();

	void togglePause();

	void takeScreenshot();

	bool openRecordFile(const Common::String &fileName);

	bool checkGameHash(const ADGameDescription *desc);

	void checkForKeyCode(const Common::Event &event);
	/**
	 * @return false because we don't want to remap the given event again. This already happened on
	 * recording the event. We record the custom events already, not the raw backend events.
	 */
	bool allowMapping() const override { return false; }

	volatile uint32 _lastMillis;
	uint32 _lastScreenshotTime;
	uint32 _screenshotPeriod;
	Common::PlaybackFile *_playbackFile;
	Common::PlaybackFile *_recordFile;

	void saveScreenShot();
	void checkRecordedMD5();
	void deleteTemporarySave();
	void updateFakeTimer(uint32 millis);
	volatile RecordMode _recordMode;
	Common::String _recordFileName;
	bool _fastPlayback;
	bool _needRedraw;
	bool _processingMillis;
};

} // End of namespace GUI

#endif // ENABLE_EVENTRECORDER

#endif
