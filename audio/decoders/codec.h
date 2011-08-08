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

#ifndef AUDIO_DECODERS_CODEC_H
#define AUDIO_DECODERS_CODEC_H

namespace Common {
	class SeekableReadStream;
}

namespace Audio {

class AudioStream;

// FIXME: Why is this class called "Codec" -- it is not meant to encode,
// only to decode, isn't it? So "Decoder" seems like a better name. 
// Or "StreamDecoder"
// But if people insist on using Codecs, then this diretory should
// be renamed from "audio/decoders" to "audio/codecs".
class Codec {
public:
// WTF: A "generic" interface without any documentation or explanation
// is almost worse than useless. 
// This class should at least say
// - what uses it (apparently right now only by QuickTimeAudioDecoder)
// - pointer to 1-2 example implementations
// - what is the only method in it supposed to do?
//   e.g. who owns the input stream, who the output stream, how much
//   data is supposed to be decoded etc.

	Codec() {}
	virtual ~Codec() {}

// FIXME: The corresponding method Video::Codec::decodeImage takes a pointer
// to a stream, this takes a reference. Unify!
	virtual AudioStream *decodeFrame(Common::SeekableReadStream &data) = 0;
};

} // End of namespace Audio

#endif
