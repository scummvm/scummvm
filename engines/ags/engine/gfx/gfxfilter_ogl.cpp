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

#if AGS_PLATFORM_OS_WINDOWS || AGS_PLATFORM_OS_ANDROID || AGS_PLATFORM_OS_IOS || AGS_PLATFORM_OS_LINUX

#include "gfx/gfxfilter_ogl.h"
#include "ogl_headers.h"

namespace AGS
{
namespace Engine
{
namespace OGL
{

const GfxFilterInfo OGLGfxFilter::FilterInfo = GfxFilterInfo("StdScale", "Nearest-neighbour");

bool OGLGfxFilter::UseLinearFiltering() const
{
    return false;
}

void OGLGfxFilter::SetFilteringForStandardSprite()
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

const GfxFilterInfo &OGLGfxFilter::GetInfo() const
{
    return FilterInfo;
}

} // namespace OGL
} // namespace Engine
} // namespace AGS

#endif // only on Windows, Android and iOS
