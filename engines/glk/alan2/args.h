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

#ifndef GLK_ALAN2_ARGS
#define GLK_ALAN2_ARGS

/* Handles the various startup methods on all machines.
 *
 * Main function args() will set up global variable advnam and the flags,
 * the terminal will also be set up and connected.
 */

#include "glk/alan2/types.h"

namespace Glk {
namespace Alan2 {

#define PROGNAME "alan2"

extern void args(int argc, char *argv[]);

} // End of namespace Alan2
} // End of namespace Glk

#endif
