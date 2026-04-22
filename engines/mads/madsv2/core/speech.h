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

#ifndef MADS_CORE_SPEECH_H
#define MADS_CORE_SPEECH_H

#include "audio/audiostream.h"
#include "common/stream.h"
#include "mads/madsv2/core/general.h"

namespace MADS {
namespace MADSV2 {

extern bool speech_system_active;
extern bool speech_on;
extern int global_speech_ready;

extern void speech_init();
extern void speech_shutdown();
extern Audio::AudioStream *speech_load(const char *resName, int id, bool useMainMemory = true);
extern void speech_play(const char *resName, int id);
extern void speech_all_off();
extern void speech_sample_rate(int rate);
extern void speech_go();
extern void global_speech_load(int id);
extern void global_speech_go(int id);
extern void global_speech(int id);

} // namespace MADSV2
} // namespace MADS

#endif
