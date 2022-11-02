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

#ifndef AGS_LIB_ALLEGRO_H
#define AGS_LIB_ALLEGRO_H

#define ALLEGRO_H

#include "ags/lib/allegro/alconfig.h"
#include "ags/lib/allegro/base.h"
#include "ags/lib/allegro/color.h"
#include "ags/lib/allegro/config.h"
#include "ags/lib/allegro/draw.h"
#include "ags/lib/allegro/error.h"
#include "ags/lib/allegro/file.h"
#include "ags/lib/allegro/fixed.h"
#include "ags/lib/allegro/fmaths.h"
#include "ags/lib/allegro/gfx.h"
#include "ags/lib/allegro/system.h"
#include "ags/lib/allegro/unicode.h"

namespace AGS3 {

inline int install_allegro() {
	return 0;
}

inline void allegro_exit() {}

} // namespace AGS3

#endif
