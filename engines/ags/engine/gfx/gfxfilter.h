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
// Graphics filter interface
//
//=============================================================================

#ifndef __AGS_EE_GFX__GFXFILTER_H
#define __AGS_EE_GFX__GFXFILTER_H

#include <memory>
#include "util/geometry.h"
#include "util/string.h"

namespace AGS
{
namespace Engine
{

using Common::String;

struct GfxFilterInfo
{
    String   Id;
    String   Name;
    int      MinScale;
    int      MaxScale;

    GfxFilterInfo()
    {}
    GfxFilterInfo(String id, String name, int min_scale = 0, int max_scale = 0)
        : Id(id)
        , Name(name)
        , MinScale(min_scale)
        , MaxScale(max_scale)
    {}
};

class IGfxFilter
{
public:
    virtual ~IGfxFilter() = default;

    virtual const GfxFilterInfo &GetInfo() const = 0;

    // Init filter for the specified color depth
    virtual bool Initialize(const int color_depth, String &err_str) = 0;
    virtual void UnInitialize() = 0;
    // Try to set rendering translation; returns actual supported destination rect
    virtual Rect SetTranslation(const Size src_size, const Rect dst_rect) = 0;
    // Get defined destination rect for this filter
    virtual Rect GetDestination() const = 0;
};

typedef std::shared_ptr<IGfxFilter> PGfxFilter;

} // namespace Engine
} // namespace AGS

#endif // __AGS_EE_GFX__GFXFILTER_H
