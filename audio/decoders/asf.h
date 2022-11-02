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

// Based on xoreos' ASF code

#ifndef AUDIO_DECODERS_ASF_H
#define AUDIO_DECODERS_ASF_H

namespace Audio {

/**
 * Try to load a ASF from the given seekable stream and create a RewindableAudioStream
 * from that data.
 *
 * @param stream          The SeekableReadStream from which to read the ASF data.
 * @param disposeAfterUse Whether to delete the stream after use.
 *
 * @return A new SeekableAudioStream, or 0, if an error occurred.
 */

SeekableAudioStream *makeASFStream(
	Common::SeekableReadStream *stream,
	DisposeAfterUse::Flag disposeAfterUse);

} // End of namespace Audio

#endif // AUDIO_DECODERS_ASF_H
