/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//=============================================================================
//
// Base implementation of IGfxDriverFactory
//
// GfxDriverFactoryBase is a template implementation of basic driver factory
// functionality, such as creating and destruction of graphics driver, and
// managing graphic filters.
//
//=============================================================================

#ifndef AGS_ENGINE_GFX_GFX_DRIVER_FACTORY_BASE_H
#define AGS_ENGINE_GFX_GFX_DRIVER_FACTORY_BASE_H

#include "ags/lib/std/vector.h"
#include "ags/engine/gfx/gfx_driver_factory.h"
#include "ags/engine/gfx/gfxfilter.h"

namespace AGS3 {
namespace AGS {
namespace Engine {

template <class TGfxDriverClass, class TGfxFilterClass>
class GfxDriverFactoryBase : public IGfxDriverFactory {
protected:
	~GfxDriverFactoryBase() override {
		delete _driver;
	}

public:
	void Shutdown() override {
		delete this;
	}

	IGraphicsDriver *GetDriver() override {
		if (!_driver)
			_driver = EnsureDriverCreated();
		return _driver;
	}

	void DestroyDriver() override {
		delete _driver;
		_driver = nullptr;
	}

	PGfxFilter SetFilter(const String &id, String &filter_error) override {
		TGfxDriverClass *driver = EnsureDriverCreated();
		if (!driver) {
			filter_error = "Graphics driver was not created";
			return PGfxFilter();
		}

		const int color_depth = driver->GetDisplayMode().ColorDepth;
		if (color_depth == 0) {
			filter_error = "Graphics mode is not set";
			return PGfxFilter();
		}

		std::shared_ptr<TGfxFilterClass> filter(CreateFilter(id));
		if (!filter) {
			filter_error = "Filter does not exist";
			return PGfxFilter();
		}

		if (!filter->Initialize(color_depth, filter_error)) {
			return PGfxFilter();
		}

		driver->SetGraphicsFilter(filter);
		return filter;
	}

protected:
	GfxDriverFactoryBase()
		: _driver(nullptr) {
	}

	virtual TGfxDriverClass *EnsureDriverCreated() = 0;
	virtual TGfxFilterClass *CreateFilter(const String &id) = 0;

	TGfxDriverClass *_driver;
};

} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
