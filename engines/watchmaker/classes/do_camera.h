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

class CameraMan {
	// Constants:
	static const int MAX_CAMERA_STEPS = 500;

	t3dCAMERA FirstPersonCamera, *DestCamera, *LastCamera, CameraCarrello;
	t3dCAMERA CameraStep[MAX_CAMERA_STEPS], AnimCamera;
	int16 CurCameraSubStep = 0, CurCameraStep = 0, NumCameraSteps = 0;
	t3dV3F OldCameraTarget, OldPlayerDir, FirstPersonTarget;
	t3dV3F SourceBlend, TargetBlend;


	uint8 bForceDirectCamera = false, bCameraCarrello = false;

	uint8 t3dCurCameraIndex = 255;
	uint8 t3dLastCameraIndex = 255;

	void NextCameraStep(WGame &game);
	void HandleCameraCarrello(t3dBODY *croom);
public:
	t3dV3F HeadAngles;
	t3dF32 CamAngleX, CamAngleY;

	void resetAngle();
	void resetLastCameraIndex() { t3dLastCameraIndex = 255; }
	uint8 getCurCameraIndex() { return t3dCurCameraIndex; }

	void MoveHeadAngles(t3dF32 diffx, t3dF32 diffy);
	void GetCameraTarget(Init &init, t3dV3F *Target);
	t3dCAMERA *PickCamera(t3dBODY *b, unsigned char in);
	void doCamera(WGame &game);
	void GetRealCharPos(Init &init, t3dV3F *Target, int32 oc, uint8 bn);
	void ResetCameraTarget();
	void ResetCameraSource();
	void ProcessCamera(WGame &game);
	uint8 GetCameraIndexUnderPlayer(int32 pl);
	void StartAnimCamera(WGame &game);
	uint8 ClipGolfCameraMove(t3dV3F *NewT, t3dV3F *OldT, t3dV3F *Source);
};


} // End of namespace Watchmaker

#endif // WATCHMAKER_DO_CAMERA_H
