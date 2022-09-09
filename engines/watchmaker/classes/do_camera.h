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

#ifndef WATCHMAKER_DO_CAMERA_H
#define WATCHMAKER_DO_CAMERA_H

#include "watchmaker/t3d.h"
#include "watchmaker/globvar.h"
#include "watchmaker/game.h"

namespace Watchmaker {

extern t3dV3F HeadAngles;
extern t3dF32 CamAngleX, CamAngleY;

void GetCameraTarget(Init &init, t3dV3F *Target);
t3dCAMERA *PickCamera(t3dBODY *b, unsigned char in);;
void doCamera(WGame &game);
void GetRealCharPos(Init &init, t3dV3F *Target, int32 oc, uint8 bn);
void ResetCameraTarget(void);
void ResetCameraSource(void);
void ProcessCamera(WGame &game);
uint8 GetCameraIndexUnderPlayer(int32 pl);
void StartAnimCamera(WGame &game);
uint8 ClipGolfCameraMove(t3dV3F *NewT, t3dV3F *OldT, t3dV3F *Source);

} // End of namespace Watchmaker

#endif // WATCHMAKER_DO_CAMERA_H
