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

#include "hpl1/engine/resources/FileSearcher.h"
#include "hpl1/engine/resources/low_level_resources.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"
#include "hpl1/hpl1.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cFileSearcher::cFileSearcher(LowLevelResources *apLowLevelResources) {
	mpLowLevelResources = apLowLevelResources;
}

//-----------------------------------------------------------------------

cFileSearcher::~cFileSearcher() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cFileSearcher::AddDirectory(tString asPath, tString asMask) {
	tStringList lstFileNames;
	// Make the path with only "/" and lower case.
	asPath = cString::ToLowerCase(cString::ReplaceCharTo(asPath, "\\", "/"));

	tStringSetIt it = m_setLoadedDirs.find(asPath);
	// If the path is not allready added, add it!
	if (it == m_setLoadedDirs.end()) {
		m_setLoadedDirs.insert(asPath);

		mpLowLevelResources->findFilesInDir(lstFileNames, asPath, asMask);
		for (const auto &f : lstFileNames) {
			m_mapFiles.insert(tFilePathMap::value_type(
				cString::ToLowerCase(f),
				cString::SetFilePath(f, asPath)));
		}
	}
}

void cFileSearcher::ClearDirectories() {
	m_mapFiles.clear();
	m_setLoadedDirs.clear();
}

//-----------------------------------------------------------------------

tString cFileSearcher::GetFilePath(tString asName) {
	tFilePathMapIt it = m_mapFiles.find(cString::ToLowerCase(asName));
	if (it == m_mapFiles.end())
		return "";

	return it->second;
}

//-----------------------------------------------------------------------

} // namespace hpl
