/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 * $URL$
 * $Id$
 *
 */

#ifndef COMMON_EVENTRECORDER_H
#define COMMON_EVENTRECORDER_H

#include "common/sys.h"
#include "common/events.h"
#include "common/singleton.h"
#include "common/savefile.h"
#include "common/mutex.h"
#include "common/array.h"

#define g_eventRec (Common::EventRecorder::instance())

namespace Common {

/**
 * Our generic event recorder.
 *
 * TODO: Add more documentation.
 */
class EventRecorder : private EventSource, private EventObserver, public Singleton<EventRecorder> {
	friend class Common::Singleton<SingletonBaseType>;
	EventRecorder();
	~EventRecorder();
public:
	void init();
	void deinit();

	/** Register random source so it can be serialized in game test purposes */
	void registerRandomSource(Common::RandomSource &rnd, const char *name);

	/** TODO: Add documentation, this is only used by the backend */
	void processMillis(uint32 &millis);

private:
	bool notifyEvent(const Common::Event &ev);
	bool pollEvent(Common::Event &ev);
	bool allowMapping() const { return false; }

	class RandomSourceRecord {
	public:
		Common::String name;
		uint32 seed;
	};
	Common::Array<RandomSourceRecord> _randomSourceRecords;

	bool _recordSubtitles;
	volatile uint32 _recordCount;
	volatile uint32 _lastRecordEvent;
	volatile uint32 _recordTimeCount;
	Common::OutSaveFile *_recordFile;
	Common::OutSaveFile *_recordTimeFile;
	Common::MutexRef _timeMutex;
	Common::MutexRef _recorderMutex;
	volatile uint32 _lastMillis;

	volatile uint32 _playbackCount;
	volatile uint32 _playbackDiff;
	volatile bool _hasPlaybackEvent;
	volatile uint32 _playbackTimeCount;
	Common::Event _playbackEvent;
	Common::InSaveFile *_playbackFile;
	Common::InSaveFile *_playbackTimeFile;

	volatile uint32 _eventCount;
	volatile uint32 _lastEventCount;

	enum RecordMode {
		kPassthrough = 0,
		kRecorderRecord = 1,
		kRecorderPlayback = 2
	};
	volatile RecordMode _recordMode;
	Common::String _recordFileName;
	Common::String _recordTempFileName;
	Common::String _recordTimeFileName;
};

} // end of namespace Common

#endif

