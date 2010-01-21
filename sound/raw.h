/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "common/sys.h"
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
	FLAG_STEREO = 1 << 3,

	/** loop the audio */
	FLAG_LOOP = 1 << 6
};


/**
 * Creates a audio stream, which plays the given raw data.
 *
 * @param ptr Data
 * @param len Length of the data (in bytes!)
 * @param rate The sample rate of the data.
 * @param flags Flags combination.
 * @see Mixer::RawFlags
 * @return The new SeekableAudioStream (or 0 on failure).
 */
SeekableAudioStream *makeRawMemoryStream(const byte *ptr, uint32 len,
		DisposeAfterUse::Flag autofreeBuffer,
		int rate, byte flags);

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
AudioStream *makeRawMemoryStream(const byte *ptr, uint32 len,
		DisposeAfterUse::Flag autofreeBuffer,
		int rate, byte flags,
		uint loopStart, uint loopEnd);


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
 * @param len Length of the data (in bytes!)
 * @param flags Flags combination.
 * @see Mixer::RawFlags
 * @param disposeStream Whether the "stream" object should be destroyed after playback.
 * @return The new SeekableAudioStream (or 0 on failure).
 */
SeekableAudioStream *makeRawDiskStream(Common::SeekableReadStream *stream,
		RawDiskStreamAudioBlock *block, int numBlocks,
		int rate, byte flags,
		DisposeAfterUse::Flag disposeStream);

/**
 * NOTE:
 * This API is considered deprecated.
 *
 * Factory function for a Raw Disk Stream.  This can stream raw PCM
 * audio data from disk. The function takes an pointer to an array of
 * RawDiskStreamAudioBlock which defines the start position and length of
 * each block of uncompressed audio in the stream.
 */
AudioStream *makeRawDiskStream(Common::SeekableReadStream *stream,
		RawDiskStreamAudioBlock *block, int numBlocks,
		int rate, byte flags,
		DisposeAfterUse::Flag disposeStream,
		uint loopStart, uint loopEnd);


} // End of namespace Audio

#endif
