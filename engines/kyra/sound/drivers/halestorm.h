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

#ifndef KYRA_SOUND_HALESTORM_H
#define KYRA_SOUND_HALESTORM_H

#include "common/scummsys.h"

namespace Audio {
	class Mixer;
}

namespace Kyra {

class HSSoundSystem;
class SoundMacRes;

class HalestormDriver {
public:
	HalestormDriver(SoundMacRes *res, Audio::Mixer *mixer);
	~HalestormDriver();

	enum InterpolationMode {
		kNone = 0,
		kSimple = 1,
		kTable = 2
	};

	// The original driver would generate 8-bit sound, which makes
	// sense, since most of the hardware had support for only that.
	// However, the internal quality of the intermediate pcm stream
	// (before the downsampling to 8-bit in the final pass) is a bit
	// higher (up to 12 bits, depending on the channel use). I have
	// added an "output16bit" option which will output the unmodified
	// intermediate data (but converting it from unsigned to signed).
	bool init(bool hiQuality, InterpolationMode imode, bool output16bit);

	void registerSamples(const uint16 *resList, bool registerOnly);
	void releaseSamples();
	int changeSystemVoices(int numChanMusicTotal, int numChanMusicPoly, int numChanSfx);

	void startSoundEffect(int id, int rate = 0);
	void enqueueSoundEffect(int id, int rate, int note);
	void stopSoundEffect(int id);
	void stopAllSoundEffects();

	enum HSCommands {
		kSongPlayOnce			= 0,
		kSongPlayLoop			= 1,
		kSongAbort				= 2,
		kSongIsPlaying			= 3,
		kSongFadeOut			= 10,
		kSongFadeIn				= 11,
		kSongFadeGetState		= 12,
		kSongFadeReset			= 13,
		kSetRateAndIntrplMode	= 14
	};

	int doCommand(int cmd, ...);

	void setMusicVolume(int volume);
	void setSoundEffectVolume(int volume);

private:
	HSSoundSystem *_hs;
};

} // End of namespace Kyra

#endif
