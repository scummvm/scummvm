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
#include "common/savefile.h"
#include "backends/keymapper/keymapper.h"
#include "backends/mixer/sdl/sdl-mixer.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "backends/timer/sdl/sdl-timer.h"
#include "backends/timer/default/default-timer.h"
#include "engines/advancedDetector.h"
#include "common/config-manager.h"
#ifdef SDL_BACKEND
#include "common/recorderfile.h"
#include "backends/saves/recorder/recorder-saves.h"
#include "backends/mixer/nullmixer/nullsdl-mixer.h"
#endif
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
class EventRecorder : private Common::EventSource, public Common::Singleton<EventRecorder>, private Common::DefaultEventMapper {
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

	void init(Common::String recordFileName, RecordMode mode);
	void deinit();
	bool processDelayMillis();
	uint32 getRandomSeed(const Common::String &name);
	void processMillis(uint32 &millis, bool skipRecord);
	bool processAudio(uint32 &samples, bool paused);
	void processGameDescription(const ADGameDescription *desc);
	Common::SeekableReadStream *processSaveStream(const Common::String & fileName);

	void preDrawOverlayGui();
	void postDrawOverlayGui();

#ifdef SDL_BACKEND
	void setAuthor(const Common::String &author) {
		_author = author;
	}
	void setNotes(const Common::String &desc){
		_desc = desc;
	}
	void setName(const Common::String &name) {
		_name = name;
	}
	const Common::String getAuthor() {
		return _author;
	}
	const Common::String getNotes() {
		return _desc;
	}
	const Common::String getName() {
		return _name;
	}
	void setRedraw(bool redraw) {
		_needRedraw = redraw;
	}
#endif

	void registerMixerManager(SdlMixerManager *mixerManager);
	void registerTimerManager(DefaultTimerManager *timerManager);

	SdlMixerManager *getMixerManager();
	DefaultTimerManager *getTimerManager();

	void deleteRecord(const Common::String& fileName);
	bool checkForContinueGame();

	void suspendRecording() {
#ifdef SDL_BACKEND
		_savedState = _initialized;
		_initialized = false;
#endif
	}

	void resumeRecording() {
#ifdef SDL_BACKEND
		_initialized = _savedState;
#endif
	}

	Common::StringArray listSaveFiles(const Common::String &pattern);
	Common::String generateRecordFileName(const Common::String &target);

	Common::SaveFileManager *getSaveManager(Common::SaveFileManager *realSaveManager);
	SDL_Surface *getSurface(int width, int height);
	void RegisterEventSource();

	bool grabScreenAndComputeMD5(Graphics::Surface &screen, uint8 md5[16]);

	void updateSubsystems();
#ifdef SDL_BACKEND
	bool switchMode();
	void switchFastMode();
#endif
private:
	virtual Common::List<Common::Event> mapEvent(const Common::Event &ev, Common::EventSource *source);
	bool notifyPoll();
	bool pollEvent(Common::Event &ev);
#ifdef SDL_BACKEND
	bool _initialized;
	volatile uint32 _fakeTimer;
	bool _savedState;
	bool _needcontinueGame;
	int _temporarySlot;
	Common::String _author;
	Common::String _desc;
	Common::String _name;
#endif
	Common::SaveFileManager *_realSaveManager;
	SdlMixerManager *_realMixerManager;
	DefaultTimerManager *_timerManager;
#ifdef SDL_BACKEND
	RecorderSaveFileManager _fakeSaveManager;
	NullSdlMixerManager *_fakeMixerManager;
	GUI::OnScreenDialog *controlPanel;
	Common::RecorderEvent _nextEvent;

	void setFileHeader();
	void setGameMd5(const ADGameDescription *gameDesc);
	void getConfig();
	void getConfigFromDomain(Common::ConfigManager::Domain *domain);
	void removeDifferentEntriesInDomain(Common::ConfigManager::Domain *domain);
	void applyPlaybackSettings();

	void switchMixer();
	void switchTimerManagers();

	void togglePause();

	void takeScreenshot();

	bool openRecordFile(const Common::String &fileName);

	bool checkGameHash(const ADGameDescription *desc);

	void checkForKeyCode(const Common::Event &event);
	bool allowMapping() const { return false; }

	volatile uint32 _lastMillis;
	uint32 _lastScreenshotTime;
	uint32 _screenshotPeriod;
	Common::PlaybackFile *_playbackFile;

	void saveScreenShot();
	void checkRecordedMD5();
	void deleteTemporarySave();
	volatile RecordMode _recordMode;
	Common::String _recordFileName;
	bool _fastPlayback;
	bool _needRedraw;
#endif
};

} // End of namespace Common

#endif
