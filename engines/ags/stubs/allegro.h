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

#ifndef AGS_STUBS_ALLEGRO_H
#define AGS_STUBS_ALLEGRO_H

#include "ags/stubs/allegro/alconfig.h"
#include "ags/stubs/allegro/base.h"
#include "ags/stubs/allegro/color.h"
#include "ags/stubs/allegro/config.h"
#include "ags/stubs/allegro/digi.h"
#include "ags/stubs/allegro/error.h"
#include "ags/stubs/allegro/file.h"
#include "ags/stubs/allegro/fixed.h"
#include "ags/stubs/allegro/gfx.h"
#include "ags/stubs/allegro/keyboard.h"
#include "ags/stubs/allegro/midi.h"
#include "ags/stubs/allegro/mouse.h"
#include "ags/stubs/allegro/sound.h"
#include "ags/stubs/allegro/system.h"
#include "ags/stubs/allegro/unicode.h"

namespace AGS3 {

extern int install_allegro();
extern void allegro_exit();

} // namespace AGS3

#endif
