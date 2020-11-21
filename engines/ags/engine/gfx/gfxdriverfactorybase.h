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
// Base implementation of IGfxDriverFactory
//
// GfxDriverFactoryBase is a template implementation of basic driver factory
// functionality, such as creating and destruction of graphics driver, and
// managing graphic filters.
//
//=============================================================================

#ifndef __AGS_EE_GFX__GFXDRIVERFACTORYBASE_H
#define __AGS_EE_GFX__GFXDRIVERFACTORYBASE_H

#include <vector>
#include "gfx/gfxdriverfactory.h"
#include "gfx/gfxfilter.h"

namespace AGS
{
namespace Engine
{

template <class TGfxDriverClass, class TGfxFilterClass>
class GfxDriverFactoryBase : public IGfxDriverFactory
{
protected:
    ~GfxDriverFactoryBase() override
    {
        delete _driver;
    }

public:
    void Shutdown() override
    {
        delete this;
    }

    IGraphicsDriver *GetDriver() override
    {
        if (!_driver)
            _driver = EnsureDriverCreated();
        return _driver;
    }

    void DestroyDriver() override
    {
        delete _driver;
        _driver = nullptr;
    }

    PGfxFilter SetFilter(const String &id, String &filter_error) override
    {
        TGfxDriverClass *driver = EnsureDriverCreated();
        if (!driver)
        {
            filter_error = "Graphics driver was not created";
            return PGfxFilter();
        }

        const int color_depth = driver->GetDisplayMode().ColorDepth;
        if (color_depth == 0)
        {
            filter_error = "Graphics mode is not set";
            return PGfxFilter();
        }

        std::shared_ptr<TGfxFilterClass> filter(CreateFilter(id));
        if (!filter)
        {
            filter_error = "Filter does not exist";
            return PGfxFilter();
        }

        if (!filter->Initialize(color_depth, filter_error))
        {
            return PGfxFilter();
        }

        driver->SetGraphicsFilter(filter);
        return filter;
    }

protected:
    GfxDriverFactoryBase()
        : _driver(nullptr)
    {
    }

    virtual TGfxDriverClass *EnsureDriverCreated() = 0;
    virtual TGfxFilterClass *CreateFilter(const String &id) = 0;

    TGfxDriverClass         *_driver;
};

} // namespace Engine
} // namespace AGS

#endif // __AGS_EE_GFX__GFXDRIVERFACTORYBASE_H
