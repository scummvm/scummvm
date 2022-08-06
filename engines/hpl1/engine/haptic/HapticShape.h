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

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_HAPTIC_SHAPE_H
#define HPL_HAPTIC_SHAPE_H

#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/haptic/HapticTypes.h"
#include "hpl1/engine/math/MathTypes.h"

namespace hpl {

class iHapticSurface;
class iLowLevelGraphics;
class iPhysicsBody;
class cSubMeshEntity;

class iHapticShape {
public:
	iHapticShape(const tString &asName, eHapticShapeType aType) : mpSurface(NULL), mType(aType), msName(asName),
																  mpBody(NULL), mpSubMeshEntity(NULL), mlTransformCount(-1) {}
	virtual ~iHapticShape() {}

	eHapticShapeType GetType() { return mType; }
	const tString &GetName() { return msName; }

	virtual void SetEnabled(bool abX) = 0;
	virtual bool GetEnabled() = 0;

	virtual void SetTransform(const cMatrixf &a_mtxTransform) = 0;
	virtual cMatrixf GetTransform() = 0;

	virtual cVector3f GetAppliedForce() = 0;

	virtual void SetSurface(iHapticSurface *apSurface) = 0;

	virtual void RenderDebug(iLowLevelGraphics *apLowLevel, const cColor &aColor) = 0;

	iHapticSurface *GetSurface() const { return mpSurface; }

	cVector3f GetSize() const { return mvSize; }

	iPhysicsBody *GetBody() const { return mpBody; }
	void SetBody(iPhysicsBody *apBody) { mpBody = apBody; }

	cSubMeshEntity *GetSubMeshEntity() const { return mpSubMeshEntity; }
	void SetSubMeshEntity(cSubMeshEntity *apSubMeshEntity) { mpSubMeshEntity = apSubMeshEntity; }

	int GetTransformCount() const { return mlTransformCount; }
	void SetTransformCount(int alX) { mlTransformCount = alX; }

protected:
	tString msName;
	iHapticSurface *mpSurface;
	eHapticShapeType mType;
	cVector3f mvSize;

	iPhysicsBody *mpBody;
	cSubMeshEntity *mpSubMeshEntity;
	int mlTransformCount;
};

}     // namespace hpl

#endif // HPL_HAPTIC_SHAPE_H
