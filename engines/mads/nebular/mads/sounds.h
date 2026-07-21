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

#ifndef MADS_NEBULAR_MADS_SOUNDS_H
#define MADS_NEBULAR_MADS_SOUNDS_H

#include "common/scummsys.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

enum {
	/* Shared */
	N_AllFade       =  1,
	N_MusicOff      =  2,
	N_MusicFade     =  3,
	N_NoiseFade     =  5,
	N_IsAnySoundOn  =  8,

	N_TitleScreen   = 24
};

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS

#endif
