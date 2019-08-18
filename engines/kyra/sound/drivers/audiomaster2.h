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

#ifndef KYRA_SOUND_AUDIOMASTER2_H
#define KYRA_SOUND_AUDIOMASTER2_H

namespace Audio {
	class Mixer;
}

namespace Common {
	class SeekableReadStream;
	class String;
}

namespace Kyra {

class AudioMaster2Internal;

class AudioMaster2 {
public:
	AudioMaster2(Audio::Mixer *mixer);
	~AudioMaster2();

	bool init();
	bool loadRessourceFile(Common::SeekableReadStream *data);

	bool startSound(const Common::String &name);
	bool stopSound(const Common::String &name);

	void flushResource(const Common::String &name);
	void flushAllResources();

	void fadeOut(int delay);
	bool isFading();

	int getPlayDuration();

	void setMusicVolume(int volume);
	void setSoundEffectVolume(int volume);

private:
	AudioMaster2Internal *_am2i;
};

} // End of namespace Kyra

#endif

#endif
