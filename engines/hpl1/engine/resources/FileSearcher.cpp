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
#include "hpl1/engine/resources/FileSearcher.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/resources/LowLevelResources.h"
#include "hpl1/engine/system/LowLevelSystem.h"


namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cFileSearcher::cFileSearcher(iLowLevelResources *apLowLevelResources)
	{
		mpLowLevelResources = apLowLevelResources;
	}

	//-----------------------------------------------------------------------

	cFileSearcher::~cFileSearcher()
	{
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cFileSearcher::AddDirectory(tString asPath, tString asMask)
	{
		tWStringList lstFileNames;
		//Make the path with only "/" and lower case.
		asPath = cString::ToLowerCase(cString::ReplaceCharTo(asPath,"\\","/"));

		tStringSetIt it = m_setLoadedDirs.find(asPath);
		//If the path is not allready added, add it!
		if(it==m_setLoadedDirs.end())
		{
			m_setLoadedDirs.insert(asPath);

			mpLowLevelResources->FindFilesInDir(lstFileNames,cString::To16Char(asPath),
												cString::To16Char(asMask));

			for(tWStringListIt it = lstFileNames.begin();it!=lstFileNames.end();it++)
			{
				tString sFile = cString::To8Char(*it);
				m_mapFiles.insert(tFilePathMap::value_type(
													cString::ToLowerCase(sFile),
													cString::SetFilePath(sFile,asPath)));
			}
		}
	}

	void cFileSearcher::ClearDirectories()
	{
		m_mapFiles.clear();
		m_setLoadedDirs.clear();
	}

	//-----------------------------------------------------------------------

	tString cFileSearcher::GetFilePath(tString asName)
	{
		tFilePathMapIt it = m_mapFiles.find(cString::ToLowerCase(asName));
		if(it == m_mapFiles.end())return "";

		return it->second;
	}

	//-----------------------------------------------------------------------

}
