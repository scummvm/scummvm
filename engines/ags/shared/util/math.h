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
// Helper math functions
//
//=============================================================================
#ifndef __AGS_CN_UTIL__MATH_H
#define __AGS_CN_UTIL__MATH_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace AGS
{
namespace Common
{

namespace Math
{
    template <class T>
    inline const T &Max(const T &a, const T &b)
    {
        return a < b ? b : a;
    }

    template <class T>
    inline const T &Min(const T &a, const T &b)
    {
        return a < b ? a : b;
    }

    template <class T>
    inline const T &Clamp(const T &val, const T &floor, const T &ceil)
    {
        return Max<T>(floor, Min<T>(val, ceil));
    }

    template <class T>
    inline void ClampLength(T &from, T &length, const T &floor, const T &height)
    {
        if (from < floor)
        {
            length -= floor - from;
            from = floor;
        }
        else if (from >= floor + height)
        {
            from = floor + height;
            length = 0;
        }

        length = Max<T>(length, 0);
        length = Min<T>(length, height - from);
    }

    // Get a measure of how value A is greater than value B;
    // if A is smaller than or equal to B, returns 0.
    template <class T>
    inline T Surplus(const T &larger, const T &smaller)
    {
        return larger > smaller ? larger - smaller : 0;
    }

    inline float RadiansToDegrees(float rads)
    {
        return rads * (float)(180.0 / M_PI);
    }

    inline float DegreesToRadians(float deg)
    {
        return deg * (float)(M_PI / 180.0);
    }
} // namespace Math

} // namespace Common
} // namespace AGS

#endif // __AGS_CN_UTIL__MATH_H
