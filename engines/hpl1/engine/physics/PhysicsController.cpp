/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 *
 * HPL1 Engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HPL1 Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HPL1 Engine.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "hpl1/engine/physics/PhysicsController.h"

#include "hpl1/engine/physics/PhysicsWorld.h"
#include "hpl1/engine/physics/PhysicsBody.h"
#include "hpl1/engine/physics/PhysicsJoint.h"

#include "hpl1/engine/math/Math.h"

#include "hpl1/engine/system/LowLevelSystem.h"

namespace hpl {

	bool iPhysicsController::mbUseInputMatrixFix = false;

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	iPhysicsController::iPhysicsController(const tString &asName, iPhysicsWorld *apWorld)
	{
		msName = asName;

		mpWorld = apWorld;

		mpBody = NULL;
		mpJoint = NULL;

		mbActive = false;

		mPidController.SetErrorNum(10);

		mbMulMassWithOutput = false;

		mfMaxOutput = 0;

		mbLogInfo = false;

		mbPaused = false;
	}

	//-----------------------------------------------------------------------

	iPhysicsController::~iPhysicsController()
	{

	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void iPhysicsController::SetPidIntegralSize(int alSize)
	{
		mPidController.SetErrorNum(alSize);
	}

	//-----------------------------------------------------------------------


	void iPhysicsController::Update(float afTimeStep)
	{
		if(mbActive==false || mbPaused) return;
		if(mpBody==NULL) return;

		cVector3f vInput = GetInputValue(mInputType);
		//Get the local input.
		if(	mbUseInputMatrixFix == false ||
			(mInputType != ePhysicsControllerInput_JointAngle && mInputType != ePhysicsControllerInput_JointDist) )
		{
			vInput = cMath::MatrixMul(cMath::MatrixInverse(mpBody->GetLocalMatrix().GetRotation()),vInput);
		}


		float fValue = GetAxisValue(mInputAxis, vInput);
		float fError = mfDestValue - fValue;

		float fOutput = GetOutputValue(fError,fValue,afTimeStep);

		if(mfMaxOutput>0){
			if(fOutput>0)
				fOutput = cMath::Min(fOutput, mfMaxOutput);
			else
				fOutput = cMath::Max(fOutput, -mfMaxOutput);
		}

		if(mbLogInfo)
			Log("%s | Input: %f Dest: %f Error: %f OutPut: %f\n",msName.c_str(),fValue,mfDestValue,fError,fOutput);

		AddOutputValue(mOutputType,mOutputAxis,fOutput);

		////////////////////////////////////////
		//Check if dest vale is reached
		if(mEndType == ePhysicsControllerEnd_OnDest && mpJoint)
		{
			if(std::abs(fValue - mfDestValue) < kEpsilonf)
			{
				mbActive = false;
				iPhysicsController *pNext = mpJoint->GetController(msNextController);
				if(pNext) pNext->SetActive(true);
			}
		}
	}

	//-----------------------------------------------------------------------

	void iPhysicsController::SetActive(bool abX)
	{
		if(abX == mbActive) return;

		mPidController.Reset();

		mbActive = abX;
	}

	//////////////////////////////////////////////////////////////////////////
	// PROTECTED METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cVector3f iPhysicsController::GetInputValue(ePhysicsControllerInput aInput)
	{
		switch(aInput)
		{
		case ePhysicsControllerInput_AngularSpeed: return mpBody ? mpBody->GetAngularVelocity() : 0;
		case ePhysicsControllerInput_LinearSpeed: return mpBody ? mpBody->GetLinearVelocity() : 0;
		case ePhysicsControllerInput_JointAngle: return mpJoint ? mpJoint->GetAngle() : 0;
		case ePhysicsControllerInput_JointDist: return mpJoint ? mpJoint->GetDistance() : 0;
		}
		return 0;
	}

	//-----------------------------------------------------------------------

	float iPhysicsController::GetOutputValue(float afError,float afInput, float afTimeStep)
	{
		if(mType == ePhysicsControllerType_Pid)
		{
			mPidController.p = mfA;
			mPidController.i = mfB;
			mPidController.d = mfC;

			return mPidController.Output(afError, afTimeStep);
		}
		else
		{
			return afError *mfA - afInput *mfB;
		}

		return 0;
	}

	//-----------------------------------------------------------------------

	void iPhysicsController::AddOutputValue(ePhysicsControllerOutput aOutput,
									ePhysicsControllerAxis aAxis,
									float afVal)
	{
		cVector3f vVec(0,0,0);

		switch(aAxis)
		{
		case ePhysicsControllerAxis_X: vVec.x = afVal; break;
		case ePhysicsControllerAxis_Y: vVec.y = afVal; break;
		case ePhysicsControllerAxis_Z: vVec.z = afVal; break;
		}

		if(mbMulMassWithOutput) vVec = vVec * mpBody->GetMass();

		//Set the output to body space
		vVec = cMath::MatrixMul(mpBody->GetLocalMatrix().GetRotation(), vVec);


		switch(aOutput)
		{
		case ePhysicsControllerOutput_Torque: mpBody->AddTorque(vVec); break;
		case ePhysicsControllerOutput_Force: mpBody->AddForce(vVec); break;
		}
	}

	//-----------------------------------------------------------------------

	float iPhysicsController::GetAxisValue(ePhysicsControllerAxis aAxis, const cVector3f &avVec)
	{
		switch(aAxis)
		{
		case ePhysicsControllerAxis_X: return avVec.x;
		case ePhysicsControllerAxis_Y: return avVec.y;
		case ePhysicsControllerAxis_Z: return avVec.z;
		}
		return 0;
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// SAVE OBJECT STUFF
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	kBeginSerializeBase(cSaveData_iPhysicsController)
	kSerializeVar(msName, eSerializeType_String)

	kSerializeVar(mlBodyId, eSerializeType_Int32)
	kSerializeVar(mlJointId, eSerializeType_Int32)

	kSerializeVar(mfA, eSerializeType_Float32)
	kSerializeVar(mfB, eSerializeType_Float32)
	kSerializeVar(mfC, eSerializeType_Float32)

	kSerializeVar(mfDestValue, eSerializeType_Float32)
	kSerializeVar(mfMaxOutput, eSerializeType_Float32)

	kSerializeVar(mbMulMassWithOutput, eSerializeType_Bool)

	kSerializeVar(mType, eSerializeType_Int32)

	kSerializeVar(mInputType, eSerializeType_Int32)
	kSerializeVar(mInputAxis, eSerializeType_Int32)

	kSerializeVar(mOutputType, eSerializeType_Int32)
	kSerializeVar(mOutputAxis, eSerializeType_Int32)

	kSerializeVar(mEndType, eSerializeType_Int32)

	kSerializeVar(msNextController, eSerializeType_String)

	kSerializeVar(mbActive, eSerializeType_Bool)
	kSerializeVar(mbPaused, eSerializeType_Bool)
	kEndSerialize()

	//-----------------------------------------------------------------------

	iSaveData* iPhysicsController::CreateSaveData()
	{
		return hplNew( cSaveData_iPhysicsController, () );
	}

	//-----------------------------------------------------------------------

	void iPhysicsController::SaveToSaveData(iSaveData *apSaveData)
	{
		kSaveData_SaveToBegin(iPhysicsController);

		kSaveData_SaveTo(msName);

		kSaveData_SaveTo(mfA);
		kSaveData_SaveTo(mfB);
		kSaveData_SaveTo(mfC);
		kSaveData_SaveTo(mfDestValue);
		kSaveData_SaveTo(mfMaxOutput);

		kSaveData_SaveTo(mbMulMassWithOutput);

		kSaveData_SaveTo(mType);
		kSaveData_SaveTo(mInputType);
		kSaveData_SaveTo(mInputAxis);
		kSaveData_SaveTo(mOutputType);
		kSaveData_SaveTo(mOutputAxis);
		kSaveData_SaveTo(mEndType);

		kSaveData_SaveTo(msNextController);

		kSaveData_SaveTo(mbActive);
		kSaveData_SaveTo(mbPaused);

		kSaveData_SaveObject(mpBody,mlBodyId);
		kSaveData_SaveObject(mpJoint,mlJointId);
	}

	//-----------------------------------------------------------------------

	void iPhysicsController::LoadFromSaveData(iSaveData *apSaveData)
	{
		kSaveData_LoadFromBegin(iPhysicsController);

		kSaveData_LoadFrom(msName);

		kSaveData_LoadFrom(mfA);
		kSaveData_LoadFrom(mfB);
		kSaveData_LoadFrom(mfC);
		kSaveData_LoadFrom(mfDestValue);
		kSaveData_LoadFrom(mfMaxOutput);

		kSaveData_LoadFrom(mbMulMassWithOutput);

		mType = (ePhysicsControllerType)pData->mType;
		mInputType = (ePhysicsControllerInput)pData->mInputType;
		mInputAxis = (ePhysicsControllerAxis)pData->mInputAxis;
		mOutputType = (ePhysicsControllerOutput)pData->mOutputType;
		mOutputAxis = (ePhysicsControllerAxis)pData->mOutputAxis;
		mEndType = (ePhysicsControllerEnd)pData->mEndType;

		kSaveData_LoadFrom(msNextController);

		kSaveData_LoadFrom(mbActive);
		kSaveData_LoadFrom(mbPaused);
	}

	//-----------------------------------------------------------------------

	void iPhysicsController::SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame)
	{
		kSaveData_SetupBegin(iPhysicsController);

		kSaveData_LoadObject(mpBody,mlBodyId,iPhysicsBody*);
		kSaveData_LoadObject(mpJoint,mlJointId, iPhysicsJoint*);
	}

	//-----------------------------------------------------------------------


}
