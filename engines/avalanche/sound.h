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

#ifndef AVALANCHE_SOUND_H
#define AVALANCHE_SOUND_H

#include "audio/mixer.h"

namespace Audio {
class PCSpeaker;
}

namespace Avalanche {

class SoundHandler {
public:
	bool _soundFl;

	SoundHandler(AvalancheEngine *vm);
	~SoundHandler();

	void toggleSound();
	void playNote(int freq, int length);
	void click();
	void blip();
	void syncVolume();
	void stopSound();

private:
	AvalancheEngine *_vm;
	Audio::PCSpeaker *_speakerStream;
	Audio::SoundHandle _speakerHandle;
};

} // End of namespace Avalanche

#endif // AVALANCHE_SOUND_H
