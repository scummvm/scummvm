/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_SOUND_LOWLEVEL_H
#define ICB_SOUND_LOWLEVEL_H

#include "engines/icb/common/px_string.h"

namespace ICB {

void SetupSndEngine();

int32 GetSamplePitch(const char *sampleName, bool8 isInSession);

void StartSample(int32 ch, const char *sampleName, bool8 isInSession, int32 looping);
void StopSample(int32 ch);
void SetChannelVolumeAndPan(int32 ch, int32 volume, int32 pan);
void SetChannelPitch(int32 ch, int32 pitch);

void LoadSessionSounds(const char *cluster);
void LoadMissionSounds(const char *cluster);

// Prototype for useful cluster managing function
bool8 DoesClusterContainFile(pxString clustername, uint32 hash_to_find, uint32 &fileoffset, uint32 &filesize);
bool8 IsSpeechPlaying(void);

// returns length of sound in 12ths of second
int32 PreloadSpeech(uint32 hash); // does nothing on psx
int32 SayLineOfSpeech(uint32 speechHash);
void StopSpeechPlayback(void);
void CancelSpeechPlayback(void);

} // End of namespace ICB

#endif // SOUND_LOWLEVEL_H
