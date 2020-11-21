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
// High quality x2 scaling filter
//
//=============================================================================

#ifndef __AGS_EE_GFX__HQ2XGFXFILTER_H
#define __AGS_EE_GFX__HQ2XGFXFILTER_H

#include "gfx/gfxfilter_allegro.h"

namespace AGS
{
namespace Engine
{
namespace ALSW
{

class HqxGfxFilter : public AllegroGfxFilter
{
public:
    HqxGfxFilter();
    ~HqxGfxFilter() override;

    const GfxFilterInfo &GetInfo() const override;

    bool Initialize(const int color_depth, String &err_str) override;
    Bitmap *InitVirtualScreen(Bitmap *screen, const Size src_size, const Rect dst_rect) override;
    Bitmap *ShutdownAndReturnRealScreen() override;

    static const GfxFilterInfo FilterInfo;

protected:
    Bitmap *PreRenderPass(Bitmap *toRender) override;

    typedef void (*PfnHqx)(unsigned char *in, unsigned char *out, int src_w, int src_h, int bpl);

    PfnHqx  _pfnHqx;
    Bitmap *_hqxScalingBuffer;
};

} // namespace ALSW
} // namespace Engine
} // namespace AGS

#endif // __AGS_EE_GFX__HQ2XGFXFILTER_H
