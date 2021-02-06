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
//
// Standard 3D-accelerated filter
//
//=============================================================================

#ifndef __AGS_EE_GFX__D3DGFXFILTER_H
#define __AGS_EE_GFX__D3DGFXFILTER_H

#include "gfx/gfxfilter_scaling.h"

namespace AGS
{
namespace Engine
{
namespace D3D
{

class D3DGfxFilter : public ScalingGfxFilter
{
public:
    const GfxFilterInfo &GetInfo() const override;

    virtual void SetSamplerStateForStandardSprite(void *direct3ddevice9);
    virtual bool NeedToColourEdgeLines();

    static const GfxFilterInfo FilterInfo;
};

} // namespace D3D
} // namespace Engine
} // namespace AGS

#endif // __AGS_EE_GFX__D3DGFXFILTER_H
