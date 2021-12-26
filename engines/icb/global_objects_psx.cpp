/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/global_objects_psx.h"
#include "engines/icb/animation_mega_set.h"
#include "engines/icb/common/px_clu_api.h"

namespace ICB {

// The texture manager for the actor textures
TextureManager *tman;

// The 3 directional lights and the ambient light
psxLight Lights[4];

// Camera and animation structures
psxCamera *g_camera;

// Actor structure
psxActor *g_av_actor;

// Lighting structure and coordinates, colour components
PSXLamp *g_av_Light;

#if CD_BUILD == 0
// Colour of the z-fragments outlines
int32 zfragRed = 255;
int32 zfragGreen = 0;
int32 zfragBlue = 0;

// Colour of the z-layers outlines
int32 zoverRed = 255;
int32 zoverGreen = 0;
int32 zoverBlue = 255;
#endif // #if CD_BUILD == 0

// Colour of the actors bounding box
int32 bboxRed = 0;
int32 bboxGreen = 255;
int32 bboxBlue = 0;

// Colour of the actors screen outline
int32 slineRed = 0;
int32 slineGreen = 0;
int32 slineBlue = 255;

// Colour of the wire-frame man
int32 wfrmRed = 100;
int32 wfrmGreen = 50;
int32 wfrmBlue = 20;

// Drawing options
int32 _normalScale = 8;
int32 _useWindingOrder = 1;
int32 _removeBackface = 1;
int32 _drawWfrm = 0;
int32 _drawTxture = 1;
int32 _drawLit = 1;
int32 _drawGouraud = 1;
int32 _drawPolys = 1;
int32 _drawNormals = 0;

int32 _drawActor = 1;
int32 _drawSline = 0;
int32 _drawShadowSline = 0;
int32 _drawBbox = 0;
int32 _drawShadowBbox = 0;
int32 _drawSolidBbox = 0;
int32 _drawZfragEdge = 0;
int32 _drawZLayerEdge = 0;
int32 _drawZfragImg = 1;
int32 _drawText = 0;
int32 _drawCompass = 0;
int32 _drawBG = 1;
int32 _drawProps = 1;
int32 _drawSemiTrans = 1;

int32 _drawDebugMesh = 0;

} // End of namespace ICB
