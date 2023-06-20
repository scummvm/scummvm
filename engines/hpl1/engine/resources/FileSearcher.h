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

#ifndef HPL_FILESEARCHER_H
#define HPL_FILESEARCHER_H

#include "hpl1/engine/resources/ResourcesTypes.h"
#include "hpl1/engine/system/SystemTypes.h"
#include "hpl1/std/map.h"
#include "common/multimap.h"

namespace hpl {

class LowLevelResources;

typedef Common::MultiMap<tString, tString> tFilePathMap;
typedef tFilePathMap::iterator tFilePathMapIt;

class cFileSearcher {
public:
	cFileSearcher(LowLevelResources *apLowLevelResources);
	~cFileSearcher();

	/**
	 * Adds a directory that will be searched when looking for files.
	 * \param asMask What files that should be searched for, for example: "*.jpeg".
	 * \param asPath The path to the directory.
	 */
	void AddDirectory(tString asPath, tString asMask);

	/**
	 * Clears all directories
	 */
	void ClearDirectories();

	/**
	 * Gets a file pointer and searches through all added resources.
	 * \param asName Name of the file.
	 * \return Path to the file. "" if file is not found.
	 */
	tString GetFilePath(tString asName);

private:
	tFilePathMap m_mapFiles;
	tStringSet m_setLoadedDirs;

	LowLevelResources *mpLowLevelResources;
};

} // namespace hpl

#endif // HPL_FILESEARCHER_H
