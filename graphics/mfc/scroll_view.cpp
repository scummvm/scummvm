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

#include "common/textconsole.h"
#include "graphics/mfc/afxwin.h"

namespace Graphics {
namespace MFC {

IMPLEMENT_DYNAMIC(CScrollView, CView)
BEGIN_MESSAGE_MAP(CScrollView, CView)
END_MESSAGE_MAP()

const SIZE CScrollView::sizeNull = { 0, 0 };

void CScrollView::SetScrollSizes(int nMapMode, SIZE sizeTotal,
                                 const SIZE &sizePage, const SIZE &sizeLine) {
	error("TODO: CScrollView::SetScrollSizes");
}

} // namespace MFC
} // namespace Graphics
