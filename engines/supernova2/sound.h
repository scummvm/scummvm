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

#ifndef SUPERNOVA2_SOUND_H
#define SUPERNOVA2_SOUND_H

#include "audio/mixer.h"

namespace Supernova2 {

class Supernova2Engine;
class ResourceManager;

enum AudioId {
	kAudioIntroDing,
	kAudioSuccess,
	kAudioTaxiOpen,
	kAudioTaxiLeaving,
	kAudioTaxiArriving,
	kAudioKiosk,
	kAudioStage1,
	kAudioStage2,
	kAudioAppearance1,
	kAudioAppearance2,
	kAudioAppearance3,
	kAudioElevatorBell,
	kAudioElevator1,
	kAudioShip1,
	kAudioShip2,
	kAudioShip3,
	kAudioShipDeath,
	kAudioDeath,
	kAudioCaught,
	kAudioNumSamples
};

enum MusicId {
	kMusicIntro = 52,
	kMusicMadMonkeys = 56,
	kMusicOutro = 55
};

class Sound {
public:

public:
	Sound(Audio::Mixer *mixer, ResourceManager *resMan);

	void play(AudioId index);
	void play(MusicId index);
	void playSiren();
	void setVolume(int volume);
	int getVolume();
	void stop();
	bool isPlaying();
private:
	Audio::Mixer *_mixer;
	ResourceManager *_resMan;
	Audio::SoundHandle _soundHandle;
};

}

#endif
