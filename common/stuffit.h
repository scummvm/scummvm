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

/**
 * @file
 * StuffIt decompressor used in engines:
 * - grim
 * - groovie
 * - kyra
 */

#ifndef COMMON_STUFFIT_H
#define COMMON_STUFFIT_H

namespace Common {

/**
 * @defgroup common_stuffit StuffIt decompressor
 * @ingroup common
 *
 * @brief API related to StuffIt archive files.
 *
 * @{
 */

class Archive;
class String;
class SeekableReadStream;

/**
 * This factory method creates an Archive instance corresponding to the content
 * of the StuffIt compressed file with the given name.
 *
 * May return 0 in case of a failure.
 */
Archive *createStuffItArchive(const String &fileName);
Archive *createStuffItArchive(SeekableReadStream *stream);

/** @} */

} // End of namespace Common

#endif
