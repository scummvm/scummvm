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
// Dummy OpenGL filter; does nothing useful at the moment
//
//=============================================================================

#ifndef __AGS_EE_GFX__OGLGFXFILTER_H
#define __AGS_EE_GFX__OGLGFXFILTER_H

#include "gfx/gfxfilter_scaling.h"

namespace AGS
{
namespace Engine
{
namespace OGL
{

class OGLGfxFilter : public ScalingGfxFilter
{
public:
    const GfxFilterInfo &GetInfo() const override;

    virtual bool UseLinearFiltering() const;
    virtual void SetFilteringForStandardSprite();

    static const GfxFilterInfo FilterInfo;
};

} // namespace D3D
} // namespace Engine
} // namespace AGS

#endif // __AGS_EE_GFX__OGLGFXFILTER_H
