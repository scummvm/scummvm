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

#ifndef HPL_GPU_PROGRAM_MANAGER_H
#define HPL_GPU_PROGRAM_MANAGER_H

#include "hpl1/engine/resources/ResourceManager.h"

#include "hpl1/engine/graphics/GPUProgram.h"

namespace hpl {

class iLowLevelGraphics;

class cGpuProgramManager : public iResourceManager {
public:
	cGpuProgramManager(cFileSearcher *apFileSearcher, iLowLevelGraphics *apLowLevelGraphics,
					   LowLevelResources *apLowLevelResources, LowLevelSystem *apLowLevelSystem);
	~cGpuProgramManager();

	iResourceBase *Create(const tString &asName);
	/**
	 * Creates a new GPU program
	 * \param asName name of the program
	 * \param asEntry the entry point of the program (usually "main")
	 * \param aType type of the program
	 * \return
	 */
	iGpuProgram *CreateProgram(const tString &vertex, const tString &fragment, eGpuProgramType aType = eGpuProgramType_LastEnum);

	void Destroy(iResourceBase *apResource);
	void Unload(iResourceBase *apResource);

private:
	iLowLevelGraphics *mpLowLevelGraphics;
};

} // namespace hpl

#endif // HPL_GPU_PROGRAM_MANAGER_H
