
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

#ifndef M4_PLATFORM_SOUND_DIGI_H
#define M4_PLATFORM_SOUND_DIGI_H

#include "common/str-array.h"
#include "m4/m4_types.h"

namespace M4 {
namespace Sound {

class Digi {
private:
	Common::StringArray _names;

private:
	void unload(const Common::String &name);

public:
	/**
	 * Preload a digi sample into memory buffer for play back later.
	 */
	void preload(const Common::String &name, int roomNum);

	void preload_sounds(const char **names);
	void unload_sounds();
	void task();

	// digi_play and digi_play_loop play a particular sound file in a given channel,
	// at a particular volume. The room_num parameter tells us what directory the sound
	// is stored in (all sounds are AIFFs). Trigger is an integer that is fed into
	// kernel_dispatch_trigger when the sound has finished playing
	// If the sound has been preloaded it will be played from memory, otherwise it will
	// be streamed from disk

	int32 play(const char *name, int32 channel, int32 vol, int32 trigger, int32 room_num = -1);
	int32 play_loop(const char *name, int32 channel, int32 vol, int32 trigger, int32 room_num = -1);
	void stop(int slot);
	void flush_mem();

	void read_another_chunk();
};

} // namespace Sound

extern void digi_preload(const Common::String &name, int roomNum);
extern int32 digi_play(const char *name, int32 channel, int32 vol, int32 trigger, int32 room_num = -1);
extern int32 digi_play_loop(const char *name, int32 channel, int32 vol, int32 trigger, int32 room_num = -1);
extern void digi_read_another_chunk();
extern void digi_stop(int slot);

//extern void digi_preload_stream_breaks(const seriesStreamBreak *list);
//extern void digi_unload_stream_breaks(const seriesStreamBreak *list);

} // namespace M4

#endif
