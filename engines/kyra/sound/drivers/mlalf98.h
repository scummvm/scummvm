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

#ifdef ENABLE_EOB

#ifndef KYRA_SOUND_MLALF98_H
#define KYRA_SOUND_MLALF98_H

#include "common/scummsys.h"
#include "common/array.h"

namespace Common {
	class SeekableReadStream;
}

namespace Audio {
	class Mixer;
}

namespace Kyra {

class MLALF98Internal;

class MLALF98 {
public:
	enum EmuType {
		kType9801_26 = 1,
		kType9801_86 = 2
	};

	struct ADPCMData {
		ADPCMData() : smpStart(0), smpEnd(0), unk4(0), unk5(0), volume(0), unk7(0) {}
		uint16 smpStart;
		uint16 smpEnd;
		uint8 unk4;
		uint8 unk5;
		uint8 volume;
		uint8 unk7;
	};

	typedef Common::Array<ADPCMData> ADPCMDataArray;

public:
	MLALF98(Audio::Mixer *mixer, EmuType emuType);
	~MLALF98();

	// The caller has to dispose of the stream. The stream can be discarded
	// immediately after calling the respective loader function.
	void loadMusicData(Common::SeekableReadStream *data);
	void loadSoundEffectData(Common::SeekableReadStream *data);
	void loadExtData(ADPCMDataArray &data);

	void startMusic(int track);
	void fadeOutMusic();
	void startSoundEffect(int track);

	void allChannelsOff();
	void resetExtUnit();

	void setMusicVolume(int volume);
	void setSoundEffectVolume(int volume);

private:
	MLALF98Internal *_drv;
};

} // End of namespace Kyra

#endif

#endif
