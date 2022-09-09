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

#ifndef WATCHMAKER_LL_SOUND_H
#define WATCHMAKER_LL_SOUND_H

#include "watchmaker/types.h"

namespace Watchmaker {

typedef float sF32;
typedef unsigned int sU32;
typedef signed int sS32;
typedef unsigned short sU16;

#define SOUND_NAME_LEN      100

#define SOUND_SPEECH    32

struct sV3F {
	sF32 x, y, z;
};

struct sEnvironment {
	sS32 lIndex;                                            // room index (for retrieval)
	sS32 lRoom;                                             // room effect level at low frequencies
	sS32 lRoomHF;                                           // room effect high-frequency level re. low frequency level
	sF32 flRoomRolloffFactor;                               // like DS3D flRolloffFactor but for room effect
	sF32 flDecayTime;                                       // reverberation decay time at low frequencies
	sF32 flDecayHFRatio;                                    // high-frequency to low-frequency decay time ratio
	sS32 lReflections;                                      // early reflections level relative to room effect
	sF32 flReflectionsDelay;                                // initial reflection delay time
	sS32 lReverb;                                           // late reverberation level relative to room effect
	sF32 flReverbDelay;                                     // late reverberation delay time relative to initial reflection
	sU32 dwEnvironment;                                     // sets all listener properties ****
	sF32 flEnvironmentSize;                                 // environment size in meters
	sF32 flEnvironmentDiffusion;                            // environment diffusion
	sF32 flAirAbsorptionHF;                                 // change in level per meter at 5 kHz
	sU32 dwFlags;                                           // modifies the behavior of properties
};

struct sSound {
	char name[SOUND_NAME_LEN];
	sS32 lIndex;
	sU32 dwLooped;
	sF32 flMaxDistance;                                     // servono fl
	sF32 flMinDistance;                                     // servono fl
	sV3F v3flPosition;                                      // meshlink
	sV3F v3flConeOrientation;                               // angolo con (0,0,-1)
	sU32 dwConeInsideAngle;                                 // servono int
	sU32 dwConeOutsideAngle;                                // servono int
	sS32 dwConeOutsideVolume;                               // servono int (negativo)
	sU32 dwFlags;                                           // flags
};

struct sListener {
	sF32 flDistanceFactor;                                  // non serve
	sV3F v3flFrontOrientation;                              // vettore front camera
	sV3F v3flTopOrientation;                                // vettore alto camera
	sV3F v3flPosition;                                      // quella della telecamera
	sF32 flRolloff;                                         // non serve
};

bool sSetListener(sListener *NewListener);
bool sStartSound(sSound *CurSound, bool Reload);
bool sStopSound(int32 index);
bool sStopAllSounds();
bool sIsPlaying(sS32 lIndex);

bool mInitMusicSystem();
bool mCloseMusicSystem();
bool mLoadMusic(const char *FileName);
bool mPlayMusic(const char *FileName);
bool mStopMusic();
bool mRestoreMixerVolume();

bool mSetAllVolume(unsigned char Volume);
bool sSetAllSoundsVolume(unsigned char Vol);
bool sSetAllSpeechVolume(unsigned char Vol);
bool sStartSoundDiffuse(sSound *CurSound);

} // End of namespace Watchmaker

#endif // WATCHMAKER_LL_SOUND_H
