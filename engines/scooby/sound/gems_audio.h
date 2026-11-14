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

#ifndef SCOOBY_GEMS_AUDIO_H
#define SCOOBY_GEMS_AUDIO_H

#include "common/scummsys.h"

// Entry points for the 68000-side GEMS calls made by the recovered game code. Each forwards to the
// engine's GemsDriver and does nothing when no driver is available.

namespace Scooby {

// Ghidra RunStartupSequence 0x000008E2-0x000008FF: gemsInit with the cartridge's four GEMS banks.
void gemsInit();

// Ghidra gemsStartSong (0x00009762), the game's PlayAudioCommand: starts sequence 'song' (low byte).
void gemsStartSong(int song);

// Ghidra gemsStopSong (0x00009776), the game's StopAudioPlayback: stops sequence 'song' (low byte).
void gemsStopSong(int song);

// Ghidra gemsStopAll (0x001F6F52), the game's InitializeAudioDriver: stops every sequence and voice.
void gemsStopAll();

// Ghidra gemsPauseAll (0x001F6F2A).
void gemsPauseAll();

// Ghidra gemsResumeAll (0x001F6F3E).
void gemsResumeAll();

} // namespace Scooby

#endif // SCOOBY_GEMS_AUDIO_H
