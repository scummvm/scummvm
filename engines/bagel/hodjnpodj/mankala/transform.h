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

#ifndef HODJNPODJ_MANKALA_TRANSFORM_H
#define HODJNPODJ_MANKALA_TRANSFORM_H

#include "bagel/afxwin.h"

namespace Bagel {
namespace HodjNPodj {
namespace Mankala {

typedef struct polar_tag {
	double mag;
	double angle;
} polar;

// CONSTANTS (borrowed from BORLANDC math.h include file

#if 0
/* Constants rounded for 21 decimals. */
#define M_E         2.71828182845904523536
#define M_LOG2E     1.44269504088896340736
#define M_LOG10E    0.434294481903251827651
#define M_LN2       0.693147180559945309417
#define M_LN10      2.30258509299404568402
#define M_PI        3.14159265358979323846
#define M_PI_2      1.57079632679489661923
#define M_PI_4      0.785398163397448309616
#define M_1_PI      0.318309886183790671538
#define M_2_PI      0.636619772367581343076
#define M_1_SQRTPI  0.564189583547756286948
#define M_2_SQRTPI  1.12837916709551257390
#define M_SQRT2     1.41421356237309504880
#define M_SQRT_2    0.707106781186547524401
#endif

#define MAG(z)  (sqrt(((z).x)*((z).x)+((z).y)*((z).y)))     //note z is of type complex
#define ANG(z)  (atan((z).y/(z).x))

BOOL dft(double* output, const int* input, int Length);
BOOL graph(HWND hWndParent, HGLOBAL hglbX, HGLOBAL hglbY, int len);

LRESULT GraphWndProc(HWND hWnd, UINT  msg, WPARAM wParam, LPARAM lParam);
BOOL EnumChildWndProc(HWND hWndOwner, LPARAM lParam);

} // namespace Mankala
} // namespace HodjNPodj
} // namespace Bagel

#endif
