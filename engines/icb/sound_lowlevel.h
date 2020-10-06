/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_SOUND_LOWLEVEL_H
#define ICB_SOUND_LOWLEVEL_H

#include "engines/icb/common/px_string.h"

namespace ICB {

void SetupSndEngine();

int32 GetSamplePitch(const cstr sampleName, bool8 isInSession);

void StartSample(int32 ch, const cstr sampleName, bool8 isInSession, int looping);
void StopSample(int32 ch);
void SetChannelVolumeAndPan(int32 ch, int32 volume, int32 pan);
void SetChannelPitch(int32 ch, int32 pitch);

void LoadSessionSounds(const cstr cluster);
void LoadMissionSounds(const cstr cluster);

// Prototype for useful cluster managing function
bool8 DoesClusterContainFile(pxString clustername, uint32 hash_to_find, uint32 &fileoffset, uint32 &filesize);
bool8 IsSpeechPlaying(void);

// returns length of sound in 12ths of second
int PreloadSpeech(uint32 hash); // does nothing on psx
int SayLineOfSpeech(uint32 speechHash);
void StopSpeechPlayback(void);
void CancelSpeechPlayback(void);

} // End of namespace ICB

#endif // SOUND_LOWLEVEL_H
