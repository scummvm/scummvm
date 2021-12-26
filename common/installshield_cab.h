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

#ifndef COMMON_INSTALLSHIELD_CAB_H
#define COMMON_INSTALLSHIELD_CAB_H

#include "common/types.h"

namespace Common {

/**
 * @defgroup common_installshield InstallShield
 * @ingroup common
 *
 * @brief API for managing the InstallShield.
 *
 *
 * @{
 */

class Archive;
class SeekableReadStream;

/**
 * This factory method creates an Archive instance corresponding to the content
 * of the single- or multi-file InstallShield cabinet with the given base name
 *
 * May return 0 in case of a failure.
 * 
 * @param baseName The base filename, e.g. the "data" in "data1.cab"
 */
Archive *makeInstallShieldArchive(const Common::String &baseName);

/** @} */

} // End of namespace Common

#endif
