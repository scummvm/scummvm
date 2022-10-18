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

#ifndef AGS_ENGINE_MEDIA_AUDIO_AUDIODEFINES_H
#define AGS_ENGINE_MEDIA_AUDIO_AUDIODEFINES_H

#define MUS_MIDI 1
#define MUS_MP3  2
#define MUS_WAVE 3
#define MUS_MOD  4
#define MUS_OGG  5

 // Max channels that are distributed among game's audio types
#define MAX_GAME_CHANNELS         16
#define MAX_SOUND_CHANNELS         8
#define SPECIAL_CROSSFADE_CHANNEL (MAX_GAME_CHANNELS)
// Total number of channels: game chans + utility chans
#define TOTAL_AUDIO_CHANNELS      (MAX_GAME_CHANNELS + 1)
// Number of game channels reserved for speech voice-over
#define NUM_SPEECH_CHANS          1
// Legacy channel numbers
#define MAX_GAME_CHANNELS_v320    8
#define TOTAL_AUDIO_CHANNELS_v320 (MAX_GAME_CHANNELS_v320 + 1)

#define SCHAN_SPEECH  0
#define SCHAN_AMBIENT 1
#define SCHAN_MUSIC   2
#define SCHAN_NORMAL  3
#define AUDIOTYPE_LEGACY_AMBIENT_SOUND 1
#define AUDIOTYPE_LEGACY_MUSIC 2
#define AUDIOTYPE_LEGACY_SOUND 3

#endif
