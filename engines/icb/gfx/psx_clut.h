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

#ifndef ICB_PSX_CLUT_H
#define ICB_PSX_CLUT_H

namespace ICB {

// Where the actor cluts go
#define CHARACTER_CLUT_X (0)
#define CHARACTER_CLUT_Y (240)

// Where the actor alternate cluts go
#define CHARACTER_ALTERNATE_CLUT_X (512)
#define CHARACTER_ALTERNATE_CLUT_Y (500)

// Where the background goes
#define BACKGROUND_CLUT_X (0)
#define BACKGROUND_CLUT_Y (496)

// Where the icons put their CLUT's
#define ICONS_CLUT_X (0)
#define ICONS_CLUT_Y (497)

// Where the text CLUT lives
#define TEXT_CLUT_X (0)
#define TEXT_CLUT_Y (498)

// Where the layer Z-map CLUT lives
#define LAYERS_ZMAP_CLUT_X (0)
#define LAYERS_ZMAP_CLUT_Y (499)

// Where the solid layers CLUT lives
#define LAYERS_SOLID_CLUT_X (0)
#define LAYERS_SOLID_CLUT_Y (500)

// Where the solid layers CLUT lives
#define LAYERS_SOLIDFG_CLUT_X (0)
#define LAYERS_SOLIDFG_CLUT_Y (501)

// Where the semi-trans additive CLUT lives
#define LAYERS_SEMIADD_CLUT_X (0)
#define LAYERS_SEMIADD_CLUT_Y (502)

// Where the semi-trans average CLUT lives
#define LAYERS_SEMIAVG_CLUT_X (0)
#define LAYERS_SEMIAVG_CLUT_Y (503)

// Where the remora sprites CLUT lives
#define REMORA_SPRITE_CLUT_X (0)
#define REMORA_SPRITE_CLUT_Y (504)

// Where the props put their CLUT's
#define PROPS_CLUT_X (256)
#define PROPS_CLUT_Y (240)
#define PROPS_CLUT_MAX_Y (255)

// Where the layer CLUT's live
#define LAYER_CLUT_X (512 + 128)
#define LAYER_CLUT_MAX_Y (511)

// Where the PSX debug font lives
#define FNTLOAD_X (768)
#define FNTLOAD_Y (256)

// Where the text characters get loaded into
#define TEXT_FONT_X (512)
#define TEXT_FONT_Y (240)
#define TEXT_FONT_MAX_X (1024)
#define TEXT_FONT_MAX_Y (256)

} // End of namespace ICB

#endif // #ifndef PSX_CLUT_H
