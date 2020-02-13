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

#ifndef NUVIE_SOUND_MIXER_RANDOM_COLLECTION_AUDIO_STREAM_H
#define NUVIE_SOUND_MIXER_RANDOM_COLLECTION_AUDIO_STREAM_H

#include "ultima/shared/std/containers.h"
#include "audio/audiostream.h"

namespace Ultima {
namespace Nuvie {
namespace U6Audio {

class RandomCollectionAudioStream : public Audio::AudioStream {
public:
	/**
	 * Mark this stream as finished. That is, signal that no further data
	 * will be queued to it. Only after this has been done can this
	 * stream ever 'end'.
	 */
	virtual void finish() = 0;
};

/**
 * Factory function for an QueuingAudioStream.
 */
RandomCollectionAudioStream *makeRandomCollectionAudioStream(int rate, bool stereo, Std::vector<Audio::RewindableAudioStream *>streams, DisposeAfterUse::Flag disposeAfterUse);

} // End of namespace U6Audio
} // End of namespace Nuvie
} // End of namespace Ultima

#endif
