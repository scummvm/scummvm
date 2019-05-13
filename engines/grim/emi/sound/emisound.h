/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef GRIM_MSS_H
#define GRIM_MSS_H

#include "audio/mixer.h"
#include "common/str.h"
#include "common/stack.h"
#include "common/mutex.h"
#include "common/hashmap.h"
#include "math/vector3d.h"

namespace Grim {

class SoundTrack;
class SaveGame;

struct MusicEntry {
	int _x;
	int _y;
	int _sync;
	int _trim;
	int _id;
	Common::String _type;
	Common::String _name;
	Common::String _filename;
};

// Currently this class only implements the exact functions called in iMuse
// from Actor, to allow for splitting that into EMI-sound and iMuse without
// changing iMuse.
class EMISound {
public:
	EMISound(int fps);
	~EMISound();
	bool startVoice(const Common::String &soundName, int volume = static_cast<int>(Audio::Mixer::kMaxChannelVolume), int pan = 64);
	bool startSfx(const Common::String &soundName, int volume = static_cast<int>(Audio::Mixer::kMaxChannelVolume), int pan = 64);
	bool startSfxFrom(const Common::String &soundName, const Math::Vector3d &pos, int volume = static_cast<int>(Audio::Mixer::kMaxChannelVolume));
	bool getSoundStatus(const Common::String &soundName);
	void stopSound(const Common::String &soundName);
	int32 getPosIn16msTicks(const Common::String &soundName);

	void setVolume(const Common::String &soundName, int volume);
	void setPan(const Common::String &soundName, int pan); /* pan: 0 .. 127 */

	bool loadSfx(const Common::String &soundName, int &id);
	void playLoadedSound(int id, bool looping);
	void playLoadedSoundFrom(int id, const Math::Vector3d &pos, bool looping);
	void setLoadedSoundLooping(int id, bool looping);
	void stopLoadedSound(int id);
	void freeLoadedSound(int id);
	void setLoadedSoundVolume(int id, int volume);
	void setLoadedSoundPan(int id, int pan);
	void setLoadedSoundPosition(int id, const Math::Vector3d &pos);
	bool getLoadedSoundStatus(int id);
	int getLoadedSoundVolume(int id);

	void setMusicState(int stateId);
	void selectMusicSet(int setId);

	bool stateHasLooped(int stateId);
	bool stateHasEnded(int stateId);

	void restoreState(SaveGame *savedState);
	void saveState(SaveGame *savedState);

	void pushStateToStack();
	void popStateFromStack();
	void flushStack();
	void pause(bool paused);
	void flushTracks();

	uint32 getMsPos(int stateId);

	void updateSoundPositions();

private:
	struct StackEntry {
		int _state;
		SoundTrack *_track;
	};

	typedef Common::List<SoundTrack *> TrackList;
	TrackList _playingTracks;
	SoundTrack *_musicTrack;
	MusicEntry *_musicTable;
	Common::String _musicPrefix;
	Common::Stack<StackEntry> _stateStack;
	// A mutex to avoid concurrent modification of the sound channels by the engine thread
	// and the timer callback, which may run in a different thread.
	Common::Mutex _mutex;

	typedef Common::HashMap<int, SoundTrack *> TrackMap;
	TrackMap _preloadedTrackMap;

	int _curMusicState;
	int _numMusicStates;
	int _callbackFps;
	int _curTrackId;

	static void timerHandler(void *refConf);
	void removeItem(SoundTrack *item);
	TrackList::iterator getPlayingTrackByName(const Common::String &name);
	void freeChannel(int32 channel);
	void initMusicTable();

	void callback();
	void updateTrack(SoundTrack *track);
	void freePlayingSounds();
	void freeLoadedSounds();
	SoundTrack *initTrack(const Common::String &soundName, Audio::Mixer::SoundType soundType, const Audio::Timestamp *start = nullptr) const;
	SoundTrack *restartTrack(SoundTrack *track);
	bool startSound(const Common::String &soundName, Audio::Mixer::SoundType soundType, int volume, int pan);
	bool startSoundFrom(const Common::String &soundName, Audio::Mixer::SoundType soundType, const Math::Vector3d &pos, int volume);
	void saveTrack(SoundTrack *track, SaveGame *savedState);
	SoundTrack *restoreTrack(SaveGame *savedState);
	MusicEntry *initMusicTableDemo(const Common::String &filename);
	void initMusicTableRetail(MusicEntry *table, const Common::String filename);
};

extern EMISound *g_emiSound;

}

#endif
