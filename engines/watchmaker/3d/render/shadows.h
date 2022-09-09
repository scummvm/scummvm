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

#ifndef WATCHMAKER_SHADOWS_H
#define WATCHMAKER_SHADOWS_H

#include "watchmaker/render.h"

namespace Watchmaker {

extern SHADOWBOX   *ShadowBoxesList[];
extern unsigned int gNumShadowBoxesList;
extern void *g_pddsShadowBuffer;
extern void *g_pddsShadowZBuffer;

//extern D3DSTENCILOP g_StencDecOp,g_StencIncOp;
bool RenderShadow(SHADOW *pShad, void *lpVBuf);
bool RenderShadowBox(SHADOWBOX *pSB, int VBO);
bool DrawShadow(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, WORD intens);
extern DWORD g_max_StencilVal;              // maximum value the stencil buffer will hold
extern bool RenderProjectiveShadow(SHADOW *pShad);
bool DrawProjectiveShadow(SHADOW *pShad);

} // End of namespace Watchmaker

#endif // WATCHMAKER_SHADOWS_H
