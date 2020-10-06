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

#ifndef ICB_STAGE_DRAW_MODULE_H__
#define ICB_STAGE_DRAW_MODULE_H__

#include "engines/icb/gfx/psx_pxactor.h"

namespace ICB {

#define MAXIMUM_POTENTIAL_ON_SCREEN_ACTOR_QUANTITY 32
#define ACTOR_SHADE_LIMIT 0.3f

typedef struct {
	const char *anim_name;
	uint32 anim_hash;
	const char *palette_name;
	uint32 palette_hash;
	const char *base_name;
	uint32 base_hash;
	uint32 frame;
	bool8 *pInShade;
	psxActor psx_actor;
	uint16 r;
	uint16 g;
	uint16 b;
	_logic *log;
} SDactor;

void StageDrawPoly(SDactor *actors, uint32 actorQty);
void InitRevRenderDevice();
void DestoryRevRenderDevice();

} // End of namespace ICB

#endif // __STAGE_DRAW_MODULE_H__
