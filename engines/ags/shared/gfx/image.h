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

#ifndef AGS_SHARED_GFX_IMAGE_H
#define AGS_SHARED_GFX_IMAGE_H

#include "ags/lib/allegro.h"

namespace AGS3 {

BITMAP *load_bitmap(const char *filename, color *pal);
BITMAP *load_bmp(const char *filename, color *pal);
BITMAP *load_bmp_pf(PACKFILE *f, color *pal);
BITMAP *load_lbm(const char *filename, color *pal);
BITMAP *load_pcx(const char *filename, color *pal);
BITMAP *load_pcx_pf(PACKFILE *f, color *pal);
BITMAP *load_tga(const char *filename, color *pal);
BITMAP *load_tga_pf(PACKFILE *f, color *pal);

int save_bitmap(Common::WriteStream &out, BITMAP *bmp, const RGB *pal);

} // namespace AGS3

#endif
