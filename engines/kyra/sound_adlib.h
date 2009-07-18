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
 * LGPL License
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef KYRA_SOUND_ADLIB_H
#define KYRA_SOUND_ADLIB_H

#include "kyra/sound.h"

#include "common/mutex.h"

namespace Kyra {
class AdlibDriver;

/**
 * AdLib implementation of the sound output device.
 *
 * It uses a special sound file format special to
 * Dune II, Kyrandia 1 and 2. While Dune II and
 * Kyrandia 1 are using exact the same format, the
 * one of Kyrandia 2 slightly differs.
 *
 * See AdlibDriver for more information.
 * @see AdlibDriver
 */
class SoundAdlibPC : public Sound {
public:
	SoundAdlibPC(KyraEngine_v1 *vm, Audio::Mixer *mixer);
	~SoundAdlibPC();

	kType getMusicType() const { return kAdlib; }

	bool init();
	void process();

	void loadSoundFile(uint file);
	void loadSoundFile(Common::String file);

	void playTrack(uint8 track);
	void haltTrack();
	bool isPlaying();

	void playSoundEffect(uint8 track);

	void beginFadeOut();
private:
	void internalLoadFile(Common::String file);

	void play(uint8 track);

	void unk1();
	void unk2();

	AdlibDriver *_driver;

	bool _v2;
	uint8 _trackEntries[500];
	uint8 *_soundDataPtr;
	int _sfxPlayingSound;

	Common::String _soundFileLoaded;

	uint8 _sfxPriority;
	uint8 _sfxFourthByteOfSong;

	int _numSoundTriggers;
	const int *_soundTriggers;

	static const int _kyra1NumSoundTriggers;
	static const int _kyra1SoundTriggers[];
};

} // end of namespace Kyra

#endif

