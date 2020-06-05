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

#ifndef PETKA_SOUND_H
#define PETKA_SOUND_H

#include "audio/mixer.h"

#include "common/hash-str.h"
#include "common/ptr.h"

namespace Common {
class SeekableReadStream;
}

namespace Petka {

class Sound {
public:
	Sound(Common::SeekableReadStream *stream, Audio::Mixer::SoundType type);
	~Sound();

	Audio::Mixer::SoundType type();

	void play(bool isLoop = false);
	void stop();

	void pause(bool p);

	bool isPlaying();

	void setBalance(uint16 x, uint16 width);

private:
	Common::ScopedPtr<Common::SeekableReadStream> _stream;
	Audio::Mixer::SoundType _type;
	Audio::SoundHandle _handle;
};

class PetkaEngine;

class SoundMgr {
public:
	SoundMgr(PetkaEngine &vm) : _vm(vm) {}

	Sound *addSound(const Common::String &name, Audio::Mixer::SoundType type);
	Sound *findSound(const Common::String &name) const;

	void removeSound(const Common::String &name);
	void removeSoundsWithType(Audio::Mixer::SoundType type);
	void removeAll();

private:
	typedef Common::HashMap<Common::String, Common::ScopedPtr<Sound>, Common::CaseSensitiveString_Hash> SoundsMap;
	PetkaEngine &_vm;
	SoundsMap _sounds;
};

} // End of namespace Petka

#endif
