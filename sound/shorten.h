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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// The code in this file is currently only used in SAGA2 (in the
// SAGA engine), so if that engine isn't enabled, we will skip
// compiling it. If you plan to use this code in another engine,
// you will have to add the proper define check here.
// Also please add the define check at the comment after the
// matching #endif further down this file.
#if defined(ENABLE_SAGA2)

#ifndef SOUND_SHORTEN_H
#define SOUND_SHORTEN_H

#include "common/scummsys.h"

namespace Common { class ReadStream; }

namespace Audio {

class AudioStream;

/**
 * Try to load a Shorten file from the given stream. Returns true if
 * successful. In that case, the stream's seek position will be set to the
 * start of the audio data, and size, rate and flags contain information
 * necessary for playback.
 */
extern byte *loadShortenFromStream(Common::ReadStream &stream, int &size, int &rate, byte &flags);

/**
 * Try to load a Shorten file from the given stream and create an AudioStream
 * from that data.
 *
 * This function uses loadShortenFromStream() internally.
 */
AudioStream *makeShortenStream(Common::ReadStream &stream);

} // End of namespace Audio

#endif

#endif // defined(ENABLE_SAGA2)


