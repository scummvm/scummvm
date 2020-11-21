/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

//=============================================================================
//
// Graphics driver factory interface
//
// Graphics factory is supposed to be singleton. Factory implementation must
// guarantee that it may be created and destroyed any number of times during
// program execution.
//
//=============================================================================

#ifndef AGS_ENGINE_GFX_GFXDRIVERFACTORY_H
#define AGS_ENGINE_GFX_GFXDRIVERFACTORY_H

#include <memory>
#include "util/string.h"
#include "util/string_types.h"

namespace AGS {
namespace Engine {

using Common::String;
using Common::StringV;
class IGraphicsDriver;
class IGfxFilter;
struct GfxFilterInfo;
typedef std::shared_ptr<IGfxFilter> PGfxFilter;


class IGfxDriverFactory {
public:
	virtual ~IGfxDriverFactory() = default;

	// Shutdown graphics factory and deallocate any resources it owns;
	// graphics factory will be unusable after calling this function.
	virtual void                 Shutdown() = 0;
	// Get graphics driver associated with this factory; creates one if
	// it does not exist.
	virtual IGraphicsDriver     *GetDriver() = 0;
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

#endif
