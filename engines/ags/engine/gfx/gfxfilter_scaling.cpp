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

#include "gfx/gfxfilter_scaling.h"

namespace AGS
{
namespace Engine
{

bool ScalingGfxFilter::Initialize(const int color_depth, String &err_str)
{
    // succeed by default
    return true;
}

void ScalingGfxFilter::UnInitialize()
{
    // do nothing by default
}

Rect ScalingGfxFilter::SetTranslation(const Size src_size, const Rect dst_rect)
{
    // do not restrict scaling by default
    _dstRect = dst_rect;
    _scaling.Init(src_size, dst_rect);
    return _dstRect;
}

Rect ScalingGfxFilter::GetDestination() const
{
    return _dstRect;
}

} // namespace Engine
} // namespace AGS
