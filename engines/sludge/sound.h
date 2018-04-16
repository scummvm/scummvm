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

#ifndef SLUDGE_SOUND_H
#define SLUDGE_SOUND_H

#include "common/file.h"

#include "audio/mixer.h"

#include "sludge/variable.h"

namespace Sludge {

// Sound list stuff
struct SoundList{
	int sound;
	struct SoundList*next;
	struct SoundList*prev;
	int cacheIndex;
	int vol;
};

class SoundManager {
public:
	SoundManager();
	virtual ~SoundManager();

	// Sound list
	void playSoundList(SoundList*s);
	void handleSoundLists(); // to produce the same effects as end of stream call back functions

	// GENERAL...
	void init();
	bool initSoundStuff();
	void killSoundStuff();

	// MUSIC...
	bool playMOD(int, int, int);
	void stopMOD(int);
	void setMusicVolume(int a, int v);
	void setDefaultMusicVolume(int v);

	// SAMPLES...
	int cacheSound(int f);
	bool startSound(int, bool = false);
	void huntKillSound(int a);
	void huntKillFreeSound(int filenum);
	void setSoundVolume(int a, int v);
	void setDefaultSoundVolume(int v);
	void setSoundLoop(int a, int s, int e);
	bool stillPlayingSound(int ch);
	bool getSoundCacheStack(StackHandler *sH);
	int findInSoundCache(int a);

	// Load & save
	void loadSounds(Common::SeekableReadStream *stream);
	void saveSounds(Common::WriteStream *stream);

	uint getSoundSource(int index);

private:
	const static int MAX_SAMPLES;
	const static int MAX_MODS;

	struct SoundThing {
		Audio::SoundHandle handle;
		int fileLoaded, vol;	//Used for wav/ogg sounds only. (sound saving/loading)
		bool looping;			//Used for wav/ogg sounds only. (sound saving/loading)
		bool inSoundList;		//Used for wav/ogg sounds only
	};
	typedef Common::List<SoundList *> SoundListHandles;

	// there's possibility that several sound list played at the same time
	SoundListHandles _soundListHandles;

	bool _soundOK;
	bool _silenceIKillYou;
	bool _isHandlingSoundList;

	SoundThing *_soundCache;
	SoundThing *_modCache;

	int _defVol;
	int _defSoundVol;
	float _modLoudness;

	int _emptySoundSlot;

	void freeSound(int a);
	bool forceRemoveSound();
	bool deleteSoundFromList(SoundList*&s);
	int findEmptySoundSlot();
	int makeSoundAudioStream(int f, Audio::AudioStream *&audiostream, bool loopy);
};

} // End of namespace Sludge

#endif
