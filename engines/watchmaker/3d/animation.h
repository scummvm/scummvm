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

#ifndef WATCHMAKER_ANIMATION_H
#define WATCHMAKER_ANIMATION_H

#include "watchmaker/t3d.h"
#include "watchmaker/3d/t3d_body.h"
#include "watchmaker/work_dirs.h"

namespace Watchmaker {

uint8 GetLightDirection(t3dV3F *dest, uint8 pos);
unsigned char GetLightPosition(t3dV3F *dest, unsigned char pos);
uint8 GetFullLightDirection(t3dV3F *dest, uint8 pos);

void FixupAnim(t3dMESH *mesh, unsigned char pos, const char *room);
t3dBODY *LoadShadowMeshes(WGame &game, const char *pname, t3dBODY *Body);
int8 t3dLoadAnimation(WGame &game, const char *s, t3dMESH *mesh, uint16 Flag);
t3dCHARACTER *t3dLoadCharacter(WGame &game, const char *pname, t3dCHARACTER *b, uint16 num);
void ReleasePreloadedAnims();
uint8 CompareLightPosition(char *roomname, uint8 pos1, t3dV3F *pos2, t3dF32 acceptable_dist);

} // End of namespace Watchmaker

#endif // WATCHMAKER_ANIMATION_H
