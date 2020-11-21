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
// Helper struct for scaling coordinates
//
// TODO: rewrite into proper Transform class.
// Maybe implement as real matrix and/or floats if that is better/runs faster.
//
//=============================================================================
#ifndef __AGS_EE_UTIL__SCALING_H
#define __AGS_EE_UTIL__SCALING_H

#include "core/types.h"
#include "util/geometry.h"

namespace AGS
{
namespace Engine
{

class AxisScaling
{
public:
    AxisScaling()
        : _scale(kUnit)
        , _unscale(kUnit)
        , _srcOffset(0)
        , _dstOffset(0)
    {
    }

    bool IsIdentity() const
    {
        return _scale == kUnit && _srcOffset == 0 && _dstOffset == 0;
    }

    bool IsTranslateOnly() const
    {
        return _scale == kUnit;
    }

    void Init(const int32_t src_length, const int32_t dst_length, const int32_t src_offset = 0, const int32_t dst_offset = 0)
    {
        _scale = kUnit;
        _unscale = kUnit;
        _srcOffset = src_offset;
        _dstOffset = dst_offset;

        if (src_length != 0)
        {
            int32_t scale = (dst_length << kShift) / src_length;
            if (scale != 0)
            {
                _scale = scale;
                _unscale = scale;
                int32_t scaled_val = ScaleDistance(src_length);
                if (scaled_val < dst_length)
                    _scale++;
            }
        }
    }

    void SetSrcOffset(int32_t x)
    {
        _srcOffset = x;
    }

    void SetDstOffset(int32_t x)
    {
        _dstOffset = x;
    }

    inline int32_t GetSrcOffset() const { return _srcOffset; }

    inline int32_t ScalePt(int32_t x) const
    {
        return (((x - _srcOffset) * _scale) >> kShift) + _dstOffset;
    }
    inline int32_t ScaleDistance(int32_t x) const
    {
        return ((x * _scale) >> kShift);
    }
    inline int32_t UnScalePt(int32_t x) const
    {
        return ((x - _dstOffset) << kShift) / _unscale + _srcOffset;
    }
    inline int32_t UnScaleDistance(int32_t x) const
    {
        return (x << kShift) / _unscale;
    }

private:
    int32_t _scale;
    int32_t _unscale;
    int32_t _srcOffset;
    int32_t _dstOffset;
};

struct PlaneScaling
{
    AxisScaling X;
    AxisScaling Y;

    bool IsIdentity() const
    {
        return X.IsIdentity() && Y.IsIdentity();
    }

    bool IsTranslateOnly() const
    {
        return X.IsTranslateOnly() && Y.IsTranslateOnly();
    }

    void Init(const Size &src_size, const Rect &dst_rect)
    {
        X.Init(src_size.Width, dst_rect.GetWidth(), 0, dst_rect.Left);
        Y.Init(src_size.Height, dst_rect.GetHeight(), 0, dst_rect.Top);
    }

    void Init(const Rect &src_rect, const Rect &dst_rect)
    {
        X.Init(src_rect.GetWidth(), dst_rect.GetWidth(), src_rect.Left, dst_rect.Left);
        Y.Init(src_rect.GetHeight(), dst_rect.GetHeight(), src_rect.Top, dst_rect.Top);
    }

    void SetSrcOffsets(int x, int y)
    {
        X.SetSrcOffset(x);
        Y.SetSrcOffset(y);
    }

    void SetDestOffsets(int x, int y)
    {
        X.SetDstOffset(x);
        Y.SetDstOffset(y);
    }

    inline Point Scale(const Point p) const
    {
        return Point(X.ScalePt(p.X), Y.ScalePt(p.Y));
    }

    inline Rect ScaleRange(const Rect r) const
    {
        return RectWH(X.ScalePt(r.Left), Y.ScalePt(r.Top), X.ScaleDistance(r.GetWidth()), Y.ScaleDistance(r.GetHeight()));
    }

    inline Point UnScale(const Point p) const
    {
        return Point(X.UnScalePt(p.X), Y.UnScalePt(p.Y));
    }

    inline Rect UnScaleRange(const Rect r) const
    {
        return RectWH(X.UnScalePt(r.Left), Y.UnScalePt(r.Top), X.UnScaleDistance(r.GetWidth()), Y.UnScaleDistance(r.GetHeight()));
    }
};

} // namespace Engine
} // namespace AGS

#endif // __AGS_EE_UTIL__SCALING_H
