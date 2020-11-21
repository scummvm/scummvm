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
// Standard 3D-accelerated filter
//
//=============================================================================

#ifndef AGS_ENGINE_GFX_D3DGFXFILTER_H
#define AGS_ENGINE_GFX_D3DGFXFILTER_H

#include "gfx/gfxfilter_scaling.h"

namespace AGS {
namespace Engine {
namespace D3D {

class D3DGfxFilter : public ScalingGfxFilter {
public:
	const GfxFilterInfo &GetInfo() const override;

	virtual void SetSamplerStateForStandardSprite(void *direct3ddevice9);
	virtual bool NeedToColourEdgeLines();

	static const GfxFilterInfo FilterInfo;
};

} // namespace D3D
} // namespace Engine
} // namespace AGS

#endif
