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

#include "core/platform.h"
#include "stdio.h"
#include "gfx/gfxfilter_aad3d.h"

#if AGS_PLATFORM_OS_WINDOWS
#include <d3d9.h>
#endif

namespace AGS
{
namespace Engine
{
namespace D3D
{

const GfxFilterInfo AAD3DGfxFilter::FilterInfo = GfxFilterInfo("Linear", "Linear interpolation");

const GfxFilterInfo &AAD3DGfxFilter::GetInfo() const
{
    return FilterInfo;
}

void AAD3DGfxFilter::SetSamplerStateForStandardSprite(void *direct3ddevice9)
{
#if AGS_PLATFORM_OS_WINDOWS
    IDirect3DDevice9* d3d9 = ((IDirect3DDevice9*)direct3ddevice9);
    d3d9->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    d3d9->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
#endif
}

bool AAD3DGfxFilter::NeedToColourEdgeLines()
{
    return true;
}

} // namespace D3D
} // namespace Engine
} // namespace AGS
