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

#include "gfx/gfxfilter_scaling.h"

namespace AGS {
namespace Engine {

bool ScalingGfxFilter::Initialize(const int color_depth, String &err_str) {
	// succeed by default
	return true;
}

void ScalingGfxFilter::UnInitialize() {
	// do nothing by default
}

Rect ScalingGfxFilter::SetTranslation(const Size src_size, const Rect dst_rect) {
	// do not restrict scaling by default
	_dstRect = dst_rect;
	_scaling.Init(src_size, dst_rect);
	return _dstRect;
}

Rect ScalingGfxFilter::GetDestination() const {
	return _dstRect;
}

} // namespace Engine
} // namespace AGS
