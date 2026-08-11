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

#ifndef LASTEXPRESS_SOUND_H
#define LASTEXPRESS_SOUND_H

#include "lastexpress/shared.h"

#include "common/str.h"

#include "audio/mixer.h"
#include "audio/audiostream.h"

namespace Audio {
class AudioStream;
class Mixer;
class QueuingAudioStream;
class SoundHandle;
} // namespace Audio

namespace LastExpress {

class CVCRFile;
class LastExpressEngine;
class SoundQueue;
class Slot;

class SoundManager {
	friend class Slot;
	friend class Logic;
	friend class LogicManager;
	friend class GraphicsManager;
	friend class Menu;
	friend class NISManager;
	friend class LastExpressEngine;

public:
	SoundManager(LastExpressEngine *engine);
	~SoundManager();

	void mix(Slot *slot, int16 *outBuf);
	void mixEngine();
	void mixChannels(int numChannels, int16 *buffer);

	int playSoundFile(const char *sndName, int typeFlags, int character, int delay);
	void startAmbient();
	void startSteam(int cityIndex);
	void endAmbient();
	void killAmbient();
	void raiseAmbient(int level, int delay);
	void levelAmbient(int delay);
	Slot *findSlotWho(int32 character);
	Slot *findSlotName(char *name);
	void ambientAI(int id);
	void soundThread();
	void killAllSlots();
	void killAllExcept(int tag1, int tag2, int tag3, int tag4, int tag5, int tag6, int tag7);
	void saveSoundInfo(CVCRFile *file);
	void destroyAllSound();
	void loadSoundInfo(CVCRFile *file, bool skipSoundLoading);
	void addSlot(Slot *entry);
	void removeSlot(Slot *entry);
	void NISFadeOut();
	void NISFadeIn();
	int getMasterVolume();
	void setMasterVolume(int volume);

	// DRIVER
	void soundDriverInit();
	void soundDriverCopyBuffersToDevice();
	int soundDriverGetVolume();
	void soundDriverSetVolume(int volume);
	int32 getSoundDriverTicks();
	void setSoundDriverTicks(int32 value);
	int32 getSoundDriver30HzCounter();
	int32 getSoundDriverFlags();
	void addSoundDriverFlags(int32 flags);
	void removeSoundDriverFlags(int32 flags);
	bool isCopyingDataToSoundDriver();

private:
	LastExpressEngine *_engine;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _channelHandle;
	Audio::QueuingAudioStream *_stream;

	int32 _sound30HzCounter = 0;
	Slot *_soundCache = nullptr;
	int32 _curSoundSlotTag = kSoundTagFirstNormal;
	int32 _soundDriverFlags = kSoundDriverInitState;
	Slot *_soundSlotChannels[6];
	int32 _numActiveChannels = 0;
	bool _scanAnySoundLoopingSection = false;
	int32 _soundSlotAmbientFlag = 0;
	int32 _soundAmbientFadeLevel = 0;
	int32 _soundAmbientFadeTime = 0;
	int32 _soundCacheCount = 0;
	int32 _loopingSoundDuration = 0;
	int32 _inSoundThreadFunction = 0;
	bool _copyingDataToSoundDriver = false;
	int16 _soundChannelsMixBuffers[6][1470];
	int16 _soundMixBuffer[1470];
	bool _soundEngineToggle = false;
	int32 _soundDriverTicks = 0;
	int32 _soundDriverVolume = 0;
	uint32 _maxQueuedStreams = 0;

	const char *_cities[17] = {
		"EPERNAY",
		"CHALONS",
		"BARLEDUC",
		"NANCY",
		"LUNEVILL",
		"AVRICOUR",
		"DEUTSCHA",
		"STRASBOU",
		"BADENOOS",
		"SALZBURG",
		"ATTNANG",
		"WELS",
		"LINZ",
		"VIENNA",
		"POZSONY",
		"GALANTA",
		"POLICE"
	};

};

} // End of namespace LastExpress

#endif // LASTEXPRESS_SOUND_H
