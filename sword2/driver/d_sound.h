/* Copyright (C) 1994-2004 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef D_SOUND_H
#define D_SOUND_H

#include "common/file.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Sword2 {

class MusicInputStream;

// Max number of sound fx
#define MAXFX 16
#define MAXMUS 2

enum {
	kCLUMode = 1,
	kMP3Mode,
	kVorbisMode,
	kFlacMode
};

extern void sword2_sound_handler(void *refCon);

struct WavInfo {
	uint8 channels;
	uint16 rate;
	uint32 samples;
	uint8 *data;
};

struct FxHandle {
	int32 _id;
	bool _paused;
	int8 _volume;
	PlayingSoundHandle _handle;
};

class Sound : public AudioStream {
private:
	Sword2Engine *_vm;

	OSystem::MutexRef _mutex;

	int32 _panTable[33];
	bool _soundOn;

	static int32 _musicVolTable[17];
	MusicInputStream *_music[MAXMUS];
	int16 *_mixBuffer;
	int _mixBufferLen;

	bool _musicPaused;
	bool _musicMuted;
	uint8 _musicVol;

	PlayingSoundHandle _soundHandleSpeech;
	bool _speechPaused;
	bool _speechMuted;
	uint8 _speechVol;

	FxHandle _fx[MAXFX];
	bool _fxPaused;
	bool _fxMuted;
	uint8 _fxVol;

	int32 getFxIndex(int32 id);
	void stopFxHandle(int i);

public:
	Sound(Sword2Engine *vm);
	~Sound();

	// AudioStream API

	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const;
	bool endOfData() const;
	int getRate() const;

	// End of AudioStream API

	void buildPanTable(bool reverse);

	bool getWavInfo(uint8 *data, WavInfo *wavInfo);

	void muteMusic(bool mute);
	bool isMusicMute(void);
	void setMusicVolume(uint8 vol);
	uint8 getMusicVolume(void);
	void pauseMusic(void);
	void unpauseMusic(void);
	void stopMusic(void);
	void waitForLeadOut(void);
	int32 streamCompMusic(uint32 musicId, bool looping);
	int32 musicTimeRemaining(void);

	void muteSpeech(bool mute);
	bool isSpeechMute(void);
	void setSpeechVolume(uint8 vol);
	uint8 getSpeechVolume(void);
	void pauseSpeech(void);
	void unpauseSpeech(void);
	int32 stopSpeech(void);
	int32 getSpeechStatus(void);
	int32 amISpeaking(void);
	uint32 preFetchCompSpeech(uint32 speechid, uint16 **buf);
	int32 playCompSpeech(uint32 speechid, uint8 vol, int8 pan);

	void muteFx(bool mute);
	bool isFxMute(void);
	uint8 getFxVolume(void);
	void setFxVolume(uint8 vol);
	int32 setFxIdVolumePan(int32 id, uint8 vol, int8 pan);
	int32 setFxIdVolume(int32 id, uint8 vol);
	void pauseFx(void);
	void pauseFxForSequence(void);
	void unpauseFx(void);
	bool isFxPlaying(int32 id);
	int32 playFx(int32 id, uint8 *data, uint8 vol, int8 pan, uint8 type);
	int32 stopFx(int32 id);
	void clearAllFx(void);
};

} // End of namespace Sword2

#endif
