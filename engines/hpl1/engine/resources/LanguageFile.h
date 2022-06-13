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
#ifndef HPL_LANGUAGE_FILE_H
#define HPL_LANGUAGE_FILE_H

#include <map>
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

	class cResources;

	//--------------------------------

	class cLanguageEntry
	{
	public:
		tWString mwsText;
	};

	typedef std::map<tString, cLanguageEntry*> tLanguageEntryMap;
	typedef tLanguageEntryMap::iterator tLanguageEntryMapIt;

	//--------------------------------

	class cLanguageCategory
	{
	public:
		~cLanguageCategory(){
			STLMapDeleteAll(m_mapEntries);
		}

		tLanguageEntryMap m_mapEntries;
	};

	typedef std::map<tString, cLanguageCategory*> tLanguageCategoryMap;
	typedef tLanguageCategoryMap::iterator tLanguageCategoryMapIt;

	//--------------------------------

	class cLanguageFile
	{
	public:
		cLanguageFile(cResources *apResources);
		~cLanguageFile();

		bool LoadFromFile(const tString asFile);

		const tWString& Translate(const tString& asCat, const tString& asName);

	private:
		tLanguageCategoryMap m_mapCategories;
		tWString mwsEmpty;

		cResources *mpResources;
	};

};
#endif // HPL_LANGUAGE_FILE_H
