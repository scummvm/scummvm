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

#ifndef PLATFORM_SDL_RISCOS_UTILS_H
#define PLATFORM_SDL_RISCOS_UTILS_H

#include "common/str.h"

// Helper functions
namespace RISCOS_Utils {

/**
 * Converts a Unix style path to a RISC OS style path.
 *
 * @param str Unix style path to convert.
 * @return RISC OS style path.
 */
Common::String toRISCOS(Common::String path);

/**
 * Converts a RISC OS style path to a Unix style path.
 *
 * @param str RISC OS style path to convert.
 * @return Unix style path.
 */
Common::String toUnix(Common::String path);

}

#endif
