/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SKYSOUND_H
#define SKYSOUND_H

#include "sound/mixer.h"
#include "sky/disk.h"
#include "base/engine.h"

enum {
	SOUND_CH0    = 0,
	SOUND_CH1    = 1,
	SOUND_BG     = 2,
	SOUND_VOICE  = 3,
	SOUND_SPEECH = 4
};

struct SfxQueue {
	uint8 count, fxNo, chan, vol;
};

#define MAX_QUEUED_FX 4

class SkySound {
protected:

public:

	SoundMixer *_mixer;
	PlayingSoundHandle _voiceHandle;
	PlayingSoundHandle _effectHandle;
	PlayingSoundHandle _bgSoundHandle;
	PlayingSoundHandle _ingameSound0, _ingameSound1, _ingameSpeech;

	uint16 _saveSounds[2];

protected:

	int playSound(uint32 id, byte *sound, uint32 size, PlayingSoundHandle *handle);

public:
	SkySound(SoundMixer *mixer, SkyDisk *pDisk, uint8 pVolume);
	~SkySound(void);
	int playVoice(byte *sound, uint32 size);
	int playBgSound(byte *sound, uint32 size);

	void loadSection(uint8 pSection);
	void playSound(uint16 sound, uint16 volume, uint8 channel);
	void fnStartFx(uint32 sound, uint8 channel);
	bool startSpeech(uint16 textNum);
	bool speechFinished(void) { return _ingameSpeech == 0; };
	void fnPauseFx(void) { _mixer->pauseAll(true); };
	void fnUnPauseFx(void) { _mixer->pauseAll(false); };
	void fnStopFx(void);
	void stopSpeech(void);
	void checkFxQueue(void);
	void restoreSfx(void);
	uint8 _soundsTotal;

private:
	SkyDisk *_skyDisk;
	uint16 _sfxBaseOfs;
	uint8 *_soundData;
	uint8 *_sampleRates, *_sfxInfo;
	uint8 _mainSfxVolume;

	static uint16 _speechConvertTable[8];
	static SfxQueue _sfxQueue[MAX_QUEUED_FX];
};

#endif
