/** @file renderer.h
	@brief
	This file contains 3d models render routines

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

#ifndef RENDERER_H
#define RENDERER_H

#include "sys.h"

int32 isUsingOrhoProjection;

int16 projPosXScreen; // fullRedrawVar1
int16 projPosYScreen; // fullRedrawVar2
int16 projPosZScreen; // fullRedrawVar3
int16 projPosX;
int16 projPosY;
int16 projPosZ;

int32 orthoProjX; // setSomethingVar1
int32 orthoProjY; // setSomethingVar2
int32 orthoProjZ; // setSomethingVar2

int32 destX;
int32 destY;
int32 destZ;

int16 *shadeAngleTab3; // tab3


int16 polyRenderType; //FillVertic_AType;
int32 numOfVertex;
int16 vertexCoordinates[193];
int16 *pRenderV1;

void setLightVector(int32 angleX, int32 angleY, int32 angleZ);

int32 computePolygons();
void renderPolygons(int32 ecx, int32 edi);

void prepareIsoModel(uint8 *bodyPtr); // loadGfxSub

int32 projectPositionOnScreen(int32 cX, int32 cY, int32 cZ);
void setCameraPosition(int32 X, int32 Y, int32 cX, int32 cY, int32 cZ);
void setCameraAngle(int32 transPosX, int32 transPosY, int32 transPosZ, int32 rotPosX, int32 rotPosY, int32 rotPosZ, int32 param6);
void setBaseTranslation(int32 X, int32 Y, int32 Z);
void setBaseRotation(int32 X, int32 Y, int32 Z);
void setOrthoProjection(int32 X, int32 Y, int32 Z);

int32 renderIsoModel(int32 X, int32 Y, int32 Z, int32 angleX, int32 angleY, int32 angleZ, uint8 *bodyPtr);

void copyActorInternAnim(uint8 *bodyPtrSrc, uint8 *bodyPtrDest);

void renderBehaviourModel(int32 boxLeft, int32 boxTop, int32 boxRight, int32 boxBottom, int32 Y, int32 angle, uint8 *entityPtr);

void renderInventoryItem(int32 X, int32 Y, uint8* itemBodyPtr, int32 angle, int32 param);

#endif
