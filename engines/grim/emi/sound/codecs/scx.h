/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 */

#ifndef GRIM_SCX_H
#define GRIM_SCX_H

namespace Common {
	class SeekableReadStream;
}

namespace Grim {

/**
 * Takes an input stream containing SCX sound data and creates
 * an RewindableAudioStream from that.
 *
 * @param stream            the SeekableReadStream from which to read the SCX data
 * @param disposeAfterUse   whether to delete the stream after use
 * @return   a new RewindableAudioStream, or NULL, if an error occurred
 */
Audio::RewindableAudioStream *makeSCXStream(
	Common::SeekableReadStream *stream,
	DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);

} // End of namespace Grim

#endif
