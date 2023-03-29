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

#ifndef HPL_AI_NODE_GENERATOR_H
#define HPL_AI_NODE_GENERATOR_H

#include "hpl1/engine/game/GameTypes.h"
#include "hpl1/engine/system/SystemTypes.h"

#include "hpl1/engine/physics/PhysicsWorld.h"
#include "hpl1/engine/scene/World3D.h"

namespace hpl {

class cWorld3D;

class cCollideRayCallback : public iPhysicsRayCallback {
public:
	virtual ~cCollideRayCallback() {}
	bool OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams);

	bool mbIntersected;
	cVector3f mvPos;
	float mfDist;
};

//-------------------------------

class cAINodeGeneratorParams {
public:
	cAINodeGeneratorParams();

	tString msNodeType;

	float mfHeightFromGround;
	float mfMinWallDist;

	cVector3f mvMinPos;
	cVector3f mvMaxPos;

	float mfGridSize;
};

//-------------------------------

class cAINodeGenerator : public iPhysicsRayCallback {
public:
	cAINodeGenerator();
	virtual ~cAINodeGenerator();

	void Generate(cWorld3D *apWorld, cAINodeGeneratorParams *apParams);

private:
	bool OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams);

	void SaveToFile();
	void LoadFromFile();

	cAINodeGeneratorParams *mpParams;
	cWorld3D *mpWorld;
	tTempAiNodeList *mpNodeList;
	cCollideRayCallback _rayCallback;
};

} // namespace hpl

#endif // HPL_AI_NODE_GENERATOR_H
