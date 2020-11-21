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
// Graphics driver factory interface
//
// Graphics factory is supposed to be singleton. Factory implementation must
// guarantee that it may be created and destroyed any number of times during
// program execution.
//
//=============================================================================

#ifndef __AGS_EE_GFX__GFXDRIVERFACTORY_H
#define __AGS_EE_GFX__GFXDRIVERFACTORY_H

#include <memory>
#include "util/string.h"
#include "util/string_types.h"

namespace AGS
{
namespace Engine
{

using Common::String;
using Common::StringV;
class IGraphicsDriver;
class IGfxFilter;
struct GfxFilterInfo;
typedef std::shared_ptr<IGfxFilter> PGfxFilter;


class IGfxDriverFactory
{
public:
    virtual ~IGfxDriverFactory() = default;

    // Shutdown graphics factory and deallocate any resources it owns;
    // graphics factory will be unusable after calling this function.
    virtual void                 Shutdown() = 0;
    // Get graphics driver associated with this factory; creates one if
    // it does not exist.
    virtual IGraphicsDriver *    GetDriver() = 0;
    // Destroy graphics driver associated with this factory; does nothing
    // if one was not created yet, 
    virtual void                 DestroyDriver() = 0;

    // Get number of supported filters
    virtual size_t               GetFilterCount() const = 0;
    // Get filter description
    virtual const GfxFilterInfo *GetFilterInfo(size_t index) const = 0;
    // Get ID of the default filter
    virtual String               GetDefaultFilterID() const = 0;

    // Assign specified filter to graphics driver
    virtual PGfxFilter           SetFilter(const String &id, String &filter_error) = 0;
};

// Query the available graphics factory names
void GetGfxDriverFactoryNames(StringV &ids);
// Acquire the graphics factory singleton object by its id
IGfxDriverFactory *GetGfxDriverFactory(const String id);

} // namespace Engine
} // namespace AGS

#endif // __AGS_EE_GFX__GFXDRIVERFACTORY_H
