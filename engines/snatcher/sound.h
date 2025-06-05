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

#ifndef SNATCHER_SOUND_H
#define SNATCHER_SOUND_H

#include "common/platform.h"

namespace Audio {
class Mixer;
}

namespace Snatcher {

class SoundDevice;
class FIO;

class SoundEngine {
public:
	SoundEngine(FIO *fio, Common::Platform platform, int soundOptions);
	~SoundEngine();

	bool init(Audio::Mixer *mixer);

	void cdaPlay(int track);
	void cdaStop();
	bool cdaIsPlaying() const;
	uint32 cdaGetTime() const;

	struct FMStatus {
		FMStatus() : music(0), sfx(0), sync(0), blocked(0) {}
		uint8 music;
		uint8 sfx;
		uint8 sync;
		uint8 blocked;
	};

	void fmSendCommand(int cmd, int restoreVolume, int trackType = 0);
	const FMStatus &fmGetStatus() const;
	void fmBlock(int block) { _fmStatus.blocked = block; }

	struct PCMStatus {
		PCMStatus() : statusBits(0), blocked(0) {}
		uint8 statusBits;
		uint8 blocked;
	};

	void pcmSendCommand(int cmd, int arg);
	void pcmInitSound(int sndId);
	const PCMStatus &pcmGetStatus() const;
	void pcmBlock(int block) { _pcmStatus.blocked = block; }

	void pause(bool toggle);
	void update();

	void setUnkCond(bool enable);

	void setMusicVolume(int vol);
	void setSoundEffectVolume(int vol);

private:
	FMStatus _fmStatus;
	PCMStatus _pcmStatus;
	SoundDevice *_dev;
};

} // End of namespace Snatcher

#endif // SNATCHER_SOUND_H
