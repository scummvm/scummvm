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
// Helper struct for scaling coordinates
//
// TODO: rewrite into proper Transform class.
// Maybe implement as real matrix and/or floats if that is better/runs faster.
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_SCALING_H
#define AGS_SHARED_UTIL_SCALING_H

#include "ags/shared/core/types.h"
#include "ags/shared/util/geometry.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

class AxisScaling {
public:
	AxisScaling()
		: _scale(kUnit)
		, _unscale(kUnit)
		, _srcOffset(0)
		, _dstOffset(0) {
	}

	bool IsIdentity() const {
		return _scale == kUnit && _srcOffset == 0 && _dstOffset == 0;
	}

	bool IsTranslateOnly() const {
		return _scale == kUnit;
	}

	void Init(const int32_t src_length, const int32_t dst_length, const int32_t src_offset = 0, const int32_t dst_offset = 0) {
		_scale = kUnit;
		_unscale = kUnit;
		_srcOffset = src_offset;
		_dstOffset = dst_offset;

		if (src_length != 0) {
			int32_t scale = (dst_length << kShift) / src_length;
			if (scale != 0) {
				_scale = scale;
				_unscale = scale;
				int32_t scaled_val = ScaleDistance(src_length);
				if (scaled_val < dst_length)
					_scale++;
			}
		}
	}

	void SetSrcOffset(int32_t x) {
		_srcOffset = x;
	}

	void SetDstOffset(int32_t x) {
		_dstOffset = x;
	}

	inline int32_t GetSrcOffset() const {
		return _srcOffset;
	}

	inline int32_t ScalePt(int32_t x) const {
		return (((x - _srcOffset) * _scale) >> kShift) + _dstOffset;
	}
	inline int32_t ScaleDistance(int32_t x) const {
		return ((x * _scale) >> kShift);
	}
	inline int32_t UnScalePt(int32_t x) const {
		return ((x - _dstOffset) << kShift) / _unscale + _srcOffset;
	}
	inline int32_t UnScaleDistance(int32_t x) const {
		return (x << kShift) / _unscale;
	}

private:
	int32_t _scale;
	int32_t _unscale;
	int32_t _srcOffset;
	int32_t _dstOffset;
};

struct PlaneScaling {
	AxisScaling X;
	AxisScaling Y;

	bool IsIdentity() const {
		return X.IsIdentity() && Y.IsIdentity();
	}

	bool IsTranslateOnly() const {
		return X.IsTranslateOnly() && Y.IsTranslateOnly();
	}

	void Init(const Size &src_size, const Rect &dst_rect) {
		X.Init(src_size.Width, dst_rect.GetWidth(), 0, dst_rect.Left);
		Y.Init(src_size.Height, dst_rect.GetHeight(), 0, dst_rect.Top);
	}

	void Init(const Rect &src_rect, const Rect &dst_rect) {
		X.Init(src_rect.GetWidth(), dst_rect.GetWidth(), src_rect.Left, dst_rect.Left);
		Y.Init(src_rect.GetHeight(), dst_rect.GetHeight(), src_rect.Top, dst_rect.Top);
	}

	void SetSrcOffsets(int x, int y) {
		X.SetSrcOffset(x);
		Y.SetSrcOffset(y);
	}

	void SetDestOffsets(int x, int y) {
		X.SetDstOffset(x);
		Y.SetDstOffset(y);
	}

	inline Point Scale(const Point &p) const {
		return Point(X.ScalePt(p.X), Y.ScalePt(p.Y));
	}

	inline Rect ScaleRange(const Rect &r) const {
		return RectWH(X.ScalePt(r.Left), Y.ScalePt(r.Top), X.ScaleDistance(r.GetWidth()), Y.ScaleDistance(r.GetHeight()));
	}

	inline Point UnScale(const Point &p) const {
		return Point(X.UnScalePt(p.X), Y.UnScalePt(p.Y));
	}

	inline Rect UnScaleRange(const Rect &r) const {
		return RectWH(X.UnScalePt(r.Left), Y.UnScalePt(r.Top), X.UnScaleDistance(r.GetWidth()), Y.UnScaleDistance(r.GetHeight()));
	}
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
