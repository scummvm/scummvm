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

#ifndef ADRIFT_OS_GLK_H
#define ADRIFT_OS_GLK_H

#include "common/scummsys.h"
#include "glk/adrift/scare.h"

namespace Glk {
namespace Adrift {

extern sc_game gsc_game;

extern bool adrift_startup_code(Common::SeekableReadStream *gameFile);
extern void adrift_main();

} // End of namespace Adrift
} // End of namespace Glk

#endif
