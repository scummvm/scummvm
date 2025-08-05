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
#ifndef TOT__SOUND_H__
#define TOT__SOUND_H__

#include "util.h"

namespace Tot {

void initSound();
void setSfxVolume(byte voll, byte volr);
void setMidiVolume(byte voll, byte volr);
void fadeOutMusic(byte voll, byte volr);
void fadeInMusic(byte voll, byte volr);
void setMasterVolume(byte voll, byte volr);
void loadVoc(Common::String vocFile, int32 startPos, uint vocSize);
void autoPlayVoc(Common::String vocFile, int32 startPos, uint vocSize);
void pitavocmem();
void playVoc(Common::String vocFile, int32 startPos, uint vocSize);
void stopVoc();
void playMidiFile(Common::String nomfich, bool loop);
void pitamidmem();
void loopactivo(bool mid_loop);
} // End of namespace Tot

#endif
