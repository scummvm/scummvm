//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

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
