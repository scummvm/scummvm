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
// Graphics filter interface
//
//=============================================================================

#ifndef AGS_ENGINE_GFX_GFXFILTER_H
#define AGS_ENGINE_GFX_GFXFILTER_H

#include "ags/lib/std/memory.h"
#include "ags/shared/util/geometry.h"
#include "ags/shared/util/string.h"

namespace AGS3 {
namespace AGS {
namespace Engine {

using Shared::String;

struct GfxFilterInfo {
	String   Id;
	String   Name;
	int      MinScale;
	int      MaxScale;

	GfxFilterInfo() {
	}
	GfxFilterInfo(String id, String name, int min_scale = 0, int max_scale = 0)
		: Id(id)
		, Name(name)
		, MinScale(min_scale)
		, MaxScale(max_scale) {
	}
};

class IGfxFilter {
public:
	virtual ~IGfxFilter() {}

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
} // namespace AGS3

#endif
