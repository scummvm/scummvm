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

#ifndef HPL_CGPROGRAM_H
#define HPL_CGPROGRAM_H

//#include <windows.h>
#if 0
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include <GL/GLee.h>
#endif

#include "temp-types.h"

#include "hpl1/engine/graphics/GPUProgram.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

class cCGProgram : public iGpuProgram {
public:
	cCGProgram(tString asName, CGcontext aContext, eGpuProgramType aType);
	~cCGProgram();

	bool Reload();
	void Unload();
	void Destroy();

	tString GetProgramName() { return msName; }

	bool CreateFromFile(const tString &asFile, const tString &asEntry = "main");

	void Bind();
	void UnBind();

	bool SetFloat(const tString &asName, float afX);
	bool SetVec2f(const tString &asName, float afX, float afY);
	bool SetVec3f(const tString &asName, float afX, float afY, float afZ);
	bool SetVec4f(const tString &asName, float afX, float afY, float afZ, float afW);

	bool SetMatrixf(const tString &asName, const cMatrixf &mMtx);
	bool SetMatrixf(const tString &asName, eGpuProgramMatrix mType,
					eGpuProgramMatrixOp mOp);

	bool SetTexture(const tString &asName, iTexture *apTexture, bool abAutoDisable = true);
	bool SetTextureToUnit(int alUnit, iTexture *apTexture);

	/// CG SPECIFIC //////////////////////

	CGprogram GetProgram() { return mProgram; }
	CGprofile GetProfile() { return mProfile; }

	static void SetVProfile(tString asProfile) {
		msForceVP = asProfile;
	}
	static void SetFProfile(tString asProfile) {
		msForceFP = asProfile;
	}
	static tString &GetVProfile() { return msForceVP; }
	static tString &GetFProfile() { return msForceFP; }

protected:
	CGcontext mContext;

	tString msName;
	tString msFile;
	tString msEntry;
	CGprogram mProgram;
	CGprofile mProfile;

	CGparameter mvTexUnitParam[MAX_TEXTUREUNITS];

	CGparameter GetParam(const tString &asName, CGtype aType);

	static tString msForceFP;
	static tString msForceVP;
};
};     // namespace hpl
#endif // HPL_CGPROGRAM_H
