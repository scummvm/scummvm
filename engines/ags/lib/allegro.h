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

#include "ags/lib/allegro/alconfig.h"
#include "ags/lib/allegro/base.h"
#include "ags/lib/allegro/color.h"
#include "ags/lib/allegro/config.h"
#include "ags/lib/allegro/digi.h"
#include "ags/lib/allegro/error.h"
#include "ags/lib/allegro/file.h"
#include "ags/lib/allegro/fixed.h"
#include "ags/lib/allegro/gfx.h"
#include "ags/lib/allegro/keyboard.h"
#include "ags/lib/allegro/midi.h"
#include "ags/lib/allegro/mouse.h"
#include "ags/lib/allegro/sound.h"
#include "ags/lib/allegro/system.h"
#include "ags/lib/allegro/unicode.h"

namespace AGS3 {

extern int install_allegro();
extern void allegro_exit();

} // namespace AGS3

#endif
