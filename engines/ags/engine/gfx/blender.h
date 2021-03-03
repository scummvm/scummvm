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

//=============================================================================
//
// AGS specific color blending routines for transparency and tinting effects
//
//=============================================================================

#ifndef AGS_ENGINE_GFX_BLENDER_H
#define AGS_ENGINE_GFX_BLENDER_H

namespace AGS3 {

// Customizable alpha blender that uses the supplied alpha value as src alpha,
// and preserves destination's alpha channel (if there was one);
void set_my_trans_blender(int r, int g, int b, int a);

// Additive alpha blender plain copies src over, applying a summ of src and
// dst alpha values.
void set_additive_alpha_blender();
// Opaque alpha blender plain copies src over, applying opaque alpha value.
void set_opaque_alpha_blender();
// Sets argb2argb for 32-bit mode, and provides appropriate funcs for blending 32-bit onto 15/16/24-bit destination
void set_argb2any_blender();

} // namespace AGS3

#endif
