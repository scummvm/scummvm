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

#ifndef HPL_PHYSICS_MATERIAL_NEWTON_H
#define HPL_PHYSICS_MATERIAL_NEWTON_H

#include "hpl1/engine/libraries/newton/Newton.h"

#include "hpl1/engine/physics/PhysicsMaterial.h"

namespace hpl {

class iPhysicsBody;
class cPhysicsContactData;

class cPhysicsMaterialNewton : public iPhysicsMaterial {
public:
	cPhysicsMaterialNewton(const tString &asName, iPhysicsWorld *apWorld, int alMatId = -1);
	~cPhysicsMaterialNewton();

	void SetElasticity(float afElasticity);
	float GetElasticity() const;
	void SetStaticFriction(float afElasticity);
	float GetStaticFriction() const;
	void SetKineticFriction(float afElasticity);
	float GetKineticFriction() const;

	void SetFrictionCombMode(ePhysicsMaterialCombMode aMode);
	ePhysicsMaterialCombMode GetFrictionCombMode() const;
	void SetElasticityCombMode(ePhysicsMaterialCombMode aMode);
	ePhysicsMaterialCombMode GetElasticityCombMode() const;

	void UpdateMaterials();

	int GetId() { return mlMaterialId; }

private:
	float Combine(ePhysicsMaterialCombMode aMode, float afX, float afY);

	static int BeginContactCallback(const NewtonMaterial *material,
									const NewtonBody *apBody1, const NewtonBody *apBody2, int);
	static void ProcessContactCallback(const NewtonJoint *joint, float, int);

	NewtonWorld *mpNewtonWorld;

	int mlMaterialId;

	ePhysicsMaterialCombMode mFrictionMode;
	ePhysicsMaterialCombMode mElasticityMode;

	float mfElasticity;
	float mfStaticFriction;
	float mfKineticFriction;
};

} // namespace hpl

#endif // HPL_PHYSICS_MATERIAL_NEWTON_H
