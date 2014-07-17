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

#include "common/str.h"
#include "common/stack.h"
#include "common/mutex.h"

namespace Grim {

class SoundTrack;

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
	struct StackEntry {
		int _state;
		SoundTrack *_track;
	};

	SoundTrack **_channels;
	int32 _musicChannel;
	MusicEntry *_musicTable;
	Common::String _musicPrefix;
	Common::Stack<StackEntry> _stateStack;
	Common::Mutex _mutex;

	static void timerHandler(void *refConf);
	void removeItem(SoundTrack *item);
	int32 getFreeChannel();
	int32 getChannelByName(const Common::String &name);
	void freeChannel(int32 channel);
	void initMusicTable();
public:
	EMISound(int fps);
	~EMISound();
	bool startVoice(const char *soundName, int volume = 127, int pan = 64);
	bool getSoundStatus(const char *soundName);
	void stopSound(const char *soundName);
	int32 getPosIn16msTicks(const char *soundName);

	void setVolume(const char *soundName, int volume);
	void setPan(const char *soundName, int pan); /* pan: 0 .. 127 */

	void setMusicState(int stateId);
	void selectMusicSet(int setId);

	bool stateHasLooped(int stateId);

	void restoreState(SaveGame *savedState);
	void saveState(SaveGame *savedState);

	void pushStateToStack();
	void popStateFromStack();
	void flushStack();

	void callback();
	void updateTrack(SoundTrack *track);

	uint32 getMsPos(int stateId);
private:
	int _curMusicState;
	int _callbackFps;
	void freeAllChannels();
	bool initTrack(const Common::String &filename, const Common::String &soundName, SoundTrack *track, const Audio::Timestamp *start = nullptr);
	SoundTrack *createEmptyMusicTrack() const;
};

}

#endif
