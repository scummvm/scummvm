/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_PX_BITMAP_H_INCLUDED
#define ICB_PX_BITMAP_H_INCLUDED

#include "engines/icb/common/px_bitmap_pc.h"

namespace ICB {

typedef _pxPCBitmap _pxBitmap;
typedef _pxPCSprite _pxSprite;
#define PX_BITMAP_EXT PX_BITMAP_PC_EXT
#define PX_BITMAP_SCHEMA PC_BITMAP_SCHEMA
#define PX_BITMAP_ID PC_BITMAP_ID

} // End of namespace ICB

#endif // #ifndef _PX_BITMAP_H_INCLUDED
