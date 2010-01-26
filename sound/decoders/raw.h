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

#ifndef SOUND_RAW_H
#define SOUND_RAW_H

#include "common/scummsys.h"
#include "common/types.h"


namespace Common { class SeekableReadStream; }


namespace Audio {

class AudioStream;
class SeekableAudioStream;

/**
 * Various flags which can be bit-ORed and then passed to
 * makeRawMemoryStream and some other AudioStream factories
 * to control their behavior.
 *
 * Engine authors are advised not to rely on a certain value or
 * order of these flags (in particular, do not store them verbatim
 * in savestates).
 */
enum RawFlags {
	/** unsigned samples (default: signed) */
	FLAG_UNSIGNED = 1 << 0,

	/** sound is 16 bits wide (default: 8bit) */
	FLAG_16BITS = 1 << 1,

	/** samples are little endian (default: big endian) */
	FLAG_LITTLE_ENDIAN = 1 << 2,

	/** sound is in stereo (default: mono) */
	FLAG_STEREO = 1 << 3
};


/**
 * Creates a audio stream, which plays the given raw data.
 *
 * The data pointer is assumed to have been allocated with malloc().
 * In particular, if autofreeBuffer is set to DisposeAfterUse::YES,
 * then this buffer will be deallocated using free(). So do not
 * use a buffer allocated with new[]!
 *
 * @param ptr 	pointer to a buffer containing audio data
 * @param len	length of the buffer in bytes
 * @param rate	sample rate of the data
 * @param flags	audio format flags combination
 * @see Mixer::RawFlags
 * @param autofreeBuffer	whether the data buffer should be destroyed after playback
 * @return The new SeekableAudioStream (or 0 on failure).
 */
SeekableAudioStream *makeRawMemoryStream(const byte *ptr, uint32 len,
		int rate, byte flags,
		DisposeAfterUse::Flag autofreeBuffer = DisposeAfterUse::YES
		);

/**
 * NOTE:
 * This API is considered deprecated.
 *
 * Factory function for a raw PCM AudioStream, which will simply treat all
 * data in the buffer described by ptr and len as raw sample data in the
 * specified format. It will then simply pass this data directly to the mixer,
 * after converting it to the sample format used by the mixer (i.e. 16 bit
 * signed native endian). Optionally supports (infinite) looping of a portion
 * of the data.
 */
AudioStream *makeRawMemoryStream_OLD(const byte *ptr, uint32 len,
		int rate, byte flags,
		uint loopStart, uint loopEnd,
		DisposeAfterUse::Flag autofreeBuffer = DisposeAfterUse::YES
		);


/**
 * Struct used to define the audio data to be played by a RawDiskStream.
 */
struct RawDiskStreamAudioBlock {
	int32 pos;		///< Position in stream of the block
	int32 len;		///< Length of the block (in samples)
};

/**
 * Creates a audio stream, which plays from given stream.
 *
 * @param stream Stream to play from
 * @param block Pointer to an RawDiskStreamAudioBlock array
 * @see RawDiskStreamAudioBlock
 * @param numBlocks Number of blocks.
 * @param rate The rate
 * @param flags Flags combination.
 * @see Mixer::RawFlags
 * @param disposeStream Whether the "stream" object should be destroyed after playback.
 * @return The new SeekableAudioStream (or 0 on failure).
 */
SeekableAudioStream *makeRawDiskStream(Common::SeekableReadStream *stream,
		RawDiskStreamAudioBlock *block, int numBlocks,
		int rate, byte flags,
		DisposeAfterUse::Flag disposeStream);


} // End of namespace Audio

#endif
