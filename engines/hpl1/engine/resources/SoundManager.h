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

#ifndef HPL_SOUND_MANAGER_H
#define HPL_SOUND_MANAGER_H

#include "hpl1/engine/resources/ResourceManager.h"

namespace hpl {

class cSound;
class cResources;
class iSoundData;

class cSoundManager : public iResourceManager {
public:
	cSoundManager(cSound *apSound, cResources *apResources);
	~cSoundManager();

	iResourceBase *Create(const tString &asName);
	iSoundData *CreateSoundData(const tString &asName, bool abStream, bool abLoopStream = false);

	void Destroy(iResourceBase *apResource);
	void Unload(iResourceBase *apResource);

	void DestroyAll();

private:
	cSound *mpSound;
	cResources *mpResources;

	tStringList mlstFileFormats;

	iSoundData *FindData(const tString &asName, tString &asFilePath);
};

} // namespace hpl

#endif // HPL_SOUND_MANAGER_H
