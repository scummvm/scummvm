/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names are
 * too numerous to list here. Please refer to the COPYRIGHT
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

#ifndef SHERLOCK_SCALPEL_SCALPEL_3DO_AUDIO_H
#define SHERLOCK_SCALPEL_SCALPEL_3DO_AUDIO_H

#include "common/hashmap.h"
#include "common/str.h"
#include "common/types.h"

namespace Sherlock {
namespace Scalpel {

/**
 * Get the 3DO audio durations map (filename without extension -> duration in ms).
 * Loads from sherlock_scalpel_3do_audio.dat on first call.
 * Returns empty map if file is missing or unreadable.
 */
const Common::HashMap<Common::String, uint32> &get3doAudioDurations();

} // namespace Scalpel
} // namespace Sherlock

#endif
