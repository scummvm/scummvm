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

#ifndef AGS_SHARED_AC_AUDIOCLIPTYPE_H
#define AGS_SHARED_AC_AUDIOCLIPTYPE_H

namespace AGS3 {

// Forward declaration
namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

#define AUDIO_CLIP_TYPE_SOUND 1
struct AudioClipType {
	int id;
	int reservedChannels;
	int volume_reduction_while_speech_playing;
	int crossfadeSpeed;
	int reservedForFuture;

	void ReadFromFile(Common::Stream *in);
	void WriteToFile(Common::Stream *out);
	void ReadFromSavegame(Common::Stream *in);
	void WriteToSavegame(Common::Stream *out) const;
};

} // namespace AGS3

#endif
