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

#ifndef HPL_GPU_PROGRAM_H
#define HPL_GPU_PROGRAM_H

#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/resources/ResourceBase.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

class iTexture;

enum eGpuProgramType {
	eGpuProgramType_Vertex,
	eGpuProgramType_Fragment,
	eGpuProgramType_LastEnum
};

enum eGpuProgramMatrix {
	eGpuProgramMatrix_View,
	eGpuProgramMatrix_Projection,
	eGpuProgramMatrix_Texture,
	eGpuProgramMatrix_ViewProjection,
	eGpuProgramMatrix_LastEnum
};

enum eGpuProgramMatrixOp {
	eGpuProgramMatrixOp_Identity,
	eGpuProgramMatrixOp_Inverse,
	eGpuProgramMatrixOp_Transpose,
	eGpuProgramMatrixOp_InverseTranspose,
	eGpuProgramMatrixOp_LastEnum
};

class iGpuProgram : public iResourceBase {
public:
	iGpuProgram(const tString &asName) : iResourceBase(asName, 0) {
	}
	virtual ~iGpuProgram() {}

	static void SetLogDebugInformation(bool abX) { mbDebugInfo = abX; }

	/**
	 * Bind the program to the GPU
	 */
	virtual void Bind() = 0;
	/**
	 * Unbind the program to the GPU
	 */
	virtual void UnBind() = 0;

	virtual bool SetFloat(const tString &asName, float afX) = 0;

	bool SetVec2f(const tString &asName, const cVector2f avVec) {
		return SetVec2f(asName, avVec.x, avVec.y);
	}
	virtual bool SetVec2f(const tString &asName, float afX, float afY) = 0;

	bool SetVec3f(const tString &asName, const cVector3f &avVec) {
		return SetVec3f(asName, avVec.x, avVec.y, avVec.z);
	}
	bool SetColor3f(const tString &asName, const cColor &aCol) {
		return SetVec3f(asName, aCol.r, aCol.g, aCol.b);
	}
	virtual bool SetVec3f(const tString &asName, float afX, float afY, float afZ) = 0;

	bool SetColor4f(const tString &asName, const cColor &aCol) {
		return SetVec4f(asName, aCol.r, aCol.g, aCol.b, aCol.a);
	}
	virtual bool SetVec4f(const tString &asName, float afX, float afY, float afZ, float afW) = 0;

	virtual bool SetMatrixf(const tString &asName, const cMatrixf &mMtx) = 0;
	virtual bool SetMatrixf(const tString &asName, eGpuProgramMatrix mType,
							eGpuProgramMatrixOp mOp) = 0;

protected:
	static bool mbDebugInfo;
};

} // namespace hpl

#endif // HPL_GPU_PROGRAM_H
