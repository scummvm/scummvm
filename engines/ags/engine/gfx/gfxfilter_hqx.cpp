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

#include "gfx/bitmap.h"
#include "gfx/gfxfilter_hqx.h"
#include "gfx/hq2x3x.h"

namespace AGS
{
namespace Engine
{
namespace ALSW
{

using namespace Common;

const GfxFilterInfo HqxGfxFilter::FilterInfo = GfxFilterInfo("Hqx", "Hqx (High Quality)", 2, 3);

HqxGfxFilter::HqxGfxFilter()
    : _pfnHqx(nullptr)
    , _hqxScalingBuffer(nullptr)
{
}

HqxGfxFilter::~HqxGfxFilter()
{
    delete _hqxScalingBuffer;
}

const GfxFilterInfo &HqxGfxFilter::GetInfo() const
{
    return FilterInfo;
}

bool HqxGfxFilter::Initialize(const int color_depth, String &err_str)
{
    if (color_depth < 32)
    {
        err_str = "Only supports 32-bit colour games";
        return false;
    }
    return AllegroGfxFilter::Initialize(color_depth, err_str);
}

Bitmap* HqxGfxFilter::InitVirtualScreen(Bitmap *screen, const Size src_size, const Rect dst_rect)
{
    Bitmap *virtual_screen = AllegroGfxFilter::InitVirtualScreen(screen, src_size, dst_rect);

    // Choose used algorithm depending on minimal required integer scaling
    int min_scaling = Math::Min(dst_rect.GetWidth() / src_size.Width, dst_rect.GetHeight() / src_size.Height);
    min_scaling = Math::Clamp(min_scaling, 2, 3);
    if (min_scaling == 2)
        _pfnHqx = hq2x_32;
    else
        _pfnHqx = hq3x_32;
    _hqxScalingBuffer = BitmapHelper::CreateBitmap(src_size.Width * min_scaling, src_size.Height * min_scaling);

    InitLUTs();
    return virtual_screen;
}

Bitmap *HqxGfxFilter::ShutdownAndReturnRealScreen()
{
    Bitmap *real_screen = AllegroGfxFilter::ShutdownAndReturnRealScreen();
    delete _hqxScalingBuffer;
    _hqxScalingBuffer = nullptr;
    return real_screen;
}

Bitmap *HqxGfxFilter::PreRenderPass(Bitmap *toRender)
{
    _hqxScalingBuffer->Acquire();
    _pfnHqx(toRender->GetDataForWriting(), _hqxScalingBuffer->GetDataForWriting(),
        toRender->GetWidth(), toRender->GetHeight(), _hqxScalingBuffer->GetLineLength());
    _hqxScalingBuffer->Release();
    return _hqxScalingBuffer;
}

} // namespace ALSW
} // namespace Engine
} // namespace AGS
