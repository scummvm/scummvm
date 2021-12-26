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
// Base class for graphic filter which provides virtual screen scaling
//
//=============================================================================

#ifndef AGS_ENGINE_GFX_SCALING_GFX_FILTER_H
#define AGS_ENGINE_GFX_SCALING_GFX_FILTER_H

#include "ags/engine/gfx/gfxfilter.h"
#include "ags/shared/util/scaling.h"

namespace AGS3 {
namespace AGS {
namespace Engine {

using AGS::Shared::PlaneScaling;

class ScalingGfxFilter : public IGfxFilter {
public:
	bool Initialize(const int color_depth, String &err_str) override;
	void UnInitialize() override;
	Rect SetTranslation(const Size src_size, const Rect dst_rect) override;
	Rect GetDestination() const override;

protected:
	Rect            _dstRect;
	PlaneScaling    _scaling;
};

} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
