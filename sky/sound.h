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

#include "sound/mixer.h"
#include "sky/disk.h"
#include "common/engine.h"

class SkySound {
protected:

public:

	SoundMixer *_mixer;
	PlayingSoundHandle _voiceHandle;
	PlayingSoundHandle _effectHandle;
	PlayingSoundHandle _bgSoundHandle;
	PlayingSoundHandle _ingameSound;

protected:

	int playSound(byte *sound, uint32 size, PlayingSoundHandle *handle);

public:
	SkySound(SoundMixer *mixer, SkyDisk *pDisk);
	~SkySound(void);
	int playVoice(byte *sound, uint32 size);
	int playBgSound(byte *sound, uint32 size);

	void loadSection(uint8 pSection);
	void playSound(uint16 sound, uint16 volume);

private:
	SkyDisk *_skyDisk;
	uint8 _soundsTotal;
	uint16 _sfxBaseOfs;
    uint8 *_soundData;
	uint8 *_sampleRates, *_sfxInfo;
};

