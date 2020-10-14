/** @file debug.scene.cpp
	@brief
	This file contains scenario debug routines

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "debug.scene.h"
#include "scene.h"
#include "grid.h"
#include "lbaengine.h"
#include "redraw.h"
#include "interface.h"
#include "renderer.h"

int32 showingZones = 0;
int32 typeZones = 127; // all zones on as default

void drawBoundingBoxProjectPoints(ScenePoint* pPoint3d, ScenePoint* pPoint3dProjected) {
	projectPositionOnScreen(pPoint3d->X, pPoint3d->Y, pPoint3d->Z);

	pPoint3dProjected->X = projPosX;
	pPoint3dProjected->Y = projPosY;
	pPoint3dProjected->Z = projPosZ;

	if (renderLeft > projPosX)
		renderLeft = projPosX;

	if (renderRight < projPosX)
		renderRight = projPosX;

	if (renderTop > projPosY)
		renderTop = projPosY;

	if (renderBottom < projPosY)
		renderBottom = projPosY;
}

int32 checkZoneType(int32 type) {
	switch (type) {
	case 0:
		if (typeZones & 0x01)
			return 1;
		break;
	case 1:
		if (typeZones & 0x02)
			return 1;
		break;
	case 2:
		if (typeZones & 0x04)
			return 1;
		break;
	case 3:
		if (typeZones & 0x08)
			return 1;
		break;
	case 4:
		if (typeZones & 0x10)
			return 1;
		break;
	case 5:
		if (typeZones & 0x20)
			return 1;
		break;
	case 6:
		if (typeZones & 0x40)
			return 1;
		break;
	default:
		break;
	}

	return 0;
}

void displayZones(int16 pKey) {
	if (showingZones == 1) {
		int z;
		ZoneStruct *zonePtr = sceneZones;
		for (z = 0; z < sceneNumZones; z++) {
			zonePtr = &sceneZones[z];

			if (checkZoneType(zonePtr->type)) {
				ScenePoint frontBottomLeftPoint;
				ScenePoint frontBottomRightPoint;

				ScenePoint frontTopLeftPoint;
				ScenePoint frontTopRightPoint;

				ScenePoint backBottomLeftPoint;
				ScenePoint backBottomRightPoint;

				ScenePoint backTopLeftPoint;
				ScenePoint backTopRightPoint;

				ScenePoint frontBottomLeftPoint2D;
				ScenePoint frontBottomRightPoint2D;

				ScenePoint frontTopLeftPoint2D;
				ScenePoint frontTopRightPoint2D;

				ScenePoint backBottomLeftPoint2D;
				ScenePoint backBottomRightPoint2D;

				ScenePoint backTopLeftPoint2D;
				ScenePoint backTopRightPoint2D;

				uint8 color;

				// compute the points in 3D

				frontBottomLeftPoint.X = zonePtr->bottomLeft.X - cameraX;
				frontBottomLeftPoint.Y = zonePtr->bottomLeft.Y - cameraY;
				frontBottomLeftPoint.Z = zonePtr->topRight.Z - cameraZ;

				frontBottomRightPoint.X = zonePtr->topRight.X - cameraX;
				frontBottomRightPoint.Y = zonePtr->bottomLeft.Y - cameraY;
				frontBottomRightPoint.Z = zonePtr->topRight.Z - cameraZ;

				frontTopLeftPoint.X = zonePtr->bottomLeft.X - cameraX;
				frontTopLeftPoint.Y = zonePtr->topRight.Y - cameraY;
				frontTopLeftPoint.Z = zonePtr->topRight.Z - cameraZ;

				frontTopRightPoint.X = zonePtr->topRight.X - cameraX;
				frontTopRightPoint.Y = zonePtr->topRight.Y - cameraY;
				frontTopRightPoint.Z = zonePtr->topRight.Z - cameraZ;

				backBottomLeftPoint.X = zonePtr->bottomLeft.X - cameraX;
				backBottomLeftPoint.Y = zonePtr->bottomLeft.Y - cameraY;
				backBottomLeftPoint.Z = zonePtr->bottomLeft.Z - cameraZ;

				backBottomRightPoint.X = zonePtr->topRight.X - cameraX;
				backBottomRightPoint.Y = zonePtr->bottomLeft.Y - cameraY;
				backBottomRightPoint.Z = zonePtr->bottomLeft.Z - cameraZ;

				backTopLeftPoint.X = zonePtr->bottomLeft.X - cameraX;
				backTopLeftPoint.Y = zonePtr->topRight.Y - cameraY;
				backTopLeftPoint.Z = zonePtr->bottomLeft.Z - cameraZ;

				backTopRightPoint.X = zonePtr->topRight.X - cameraX;
				backTopRightPoint.Y = zonePtr->topRight.Y - cameraY;
				backTopRightPoint.Z = zonePtr->bottomLeft.Z - cameraZ;

				// project all points

				drawBoundingBoxProjectPoints(&frontBottomLeftPoint,    &frontBottomLeftPoint2D);
				drawBoundingBoxProjectPoints(&frontBottomRightPoint,   &frontBottomRightPoint2D);
				drawBoundingBoxProjectPoints(&frontTopLeftPoint,       &frontTopLeftPoint2D);
				drawBoundingBoxProjectPoints(&frontTopRightPoint,      &frontTopRightPoint2D);
				drawBoundingBoxProjectPoints(&backBottomLeftPoint,     &backBottomLeftPoint2D);
				drawBoundingBoxProjectPoints(&backBottomRightPoint,    &backBottomRightPoint2D);
				drawBoundingBoxProjectPoints(&backTopLeftPoint,        &backTopLeftPoint2D);
				drawBoundingBoxProjectPoints(&backTopRightPoint,       &backTopRightPoint2D);

				// draw all lines

				color = 15 * 3 + zonePtr->type * 16;

				// draw front part
				drawLine(frontBottomLeftPoint2D.X, frontBottomLeftPoint2D.Y, frontTopLeftPoint2D.X, frontTopLeftPoint2D.Y, color);
				drawLine(frontTopLeftPoint2D.X, frontTopLeftPoint2D.Y, frontTopRightPoint2D.X, frontTopRightPoint2D.Y, color);
				drawLine(frontTopRightPoint2D.X, frontTopRightPoint2D.Y, frontBottomRightPoint2D.X, frontBottomRightPoint2D.Y, color);
				drawLine(frontBottomRightPoint2D.X, frontBottomRightPoint2D.Y, frontBottomLeftPoint2D.X, frontBottomLeftPoint2D.Y, color);

				// draw top part
				drawLine(frontTopLeftPoint2D.X, frontTopLeftPoint2D.Y, backTopLeftPoint2D.X, backTopLeftPoint2D.Y, color);
				drawLine(backTopLeftPoint2D.X, backTopLeftPoint2D.Y, backTopRightPoint2D.X, backTopRightPoint2D.Y, color);
				drawLine(backTopRightPoint2D.X, backTopRightPoint2D.Y, frontTopRightPoint2D.X, frontTopRightPoint2D.Y, color);
				drawLine(frontTopRightPoint2D.X, frontTopRightPoint2D.Y, frontTopLeftPoint2D.X, frontTopLeftPoint2D.Y, color);

				// draw back part
				drawLine(backBottomLeftPoint2D.X, backBottomLeftPoint2D.Y, backTopLeftPoint2D.X, backTopLeftPoint2D.Y, color);
				drawLine(backTopLeftPoint2D.X, backTopLeftPoint2D.Y, backTopRightPoint2D.X, backTopRightPoint2D.Y, color);
				drawLine(backTopRightPoint2D.X, backTopRightPoint2D.Y, backBottomRightPoint2D.X, backBottomRightPoint2D.Y, color);
				drawLine(backBottomRightPoint2D.X, backBottomRightPoint2D.Y, backBottomLeftPoint2D.X, backBottomLeftPoint2D.Y, color);

				// draw bottom part
				drawLine(frontBottomLeftPoint2D.X, frontBottomLeftPoint2D.Y, backBottomLeftPoint2D.X, backBottomLeftPoint2D.Y, color);
				drawLine(backBottomLeftPoint2D.X, backBottomLeftPoint2D.Y, backBottomRightPoint2D.X, backBottomRightPoint2D.Y, color);
				drawLine(backBottomRightPoint2D.X, backBottomRightPoint2D.Y, frontBottomRightPoint2D.X, frontBottomRightPoint2D.Y, color);
				drawLine(frontBottomRightPoint2D.X, frontBottomRightPoint2D.Y, frontBottomLeftPoint2D.X, frontBottomLeftPoint2D.Y, color);
			}
		}
	}
}
