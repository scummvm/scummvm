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
 * ARJ decompressor used in engines:
 * - drascula
 */

#ifndef COMMON_UNARJ_H
#define COMMON_UNARJ_H

#include "common/str.h"

namespace Common {

/**
 * @defgroup common_unarj ARJ decompressor
 * @ingroup common
 *
 * @brief API related to ARJ archive files.
 *
 * @{
 */

class Archive;

/**
 * This factory method creates an Archive instance corresponding to the content
 * of the ARJ compressed file with the given name.
 *
 * May return 0 in case of a failure.
 */
Archive *makeArjArchive(const String &name, bool flattenTree = false);

/**
 * Similar to previous but for multi-volume archives
 */
Archive *makeArjArchive(const Array<String> &names, bool flattenTree = false);

/** @} */

} // End of namespace Common

#endif
