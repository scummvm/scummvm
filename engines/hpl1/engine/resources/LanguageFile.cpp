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

#include "hpl1/engine/resources/LanguageFile.h"
#include "hpl1/engine/impl/tinyXML/tinyxml.h"
#include "hpl1/engine/resources/FileSearcher.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/engine/resources/FileSearcher.h"
#include "hpl1/engine/resources/Resources.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLanguageFile::cLanguageFile(cResources *apResources) {
	mpResources = apResources;
}

//-----------------------------------------------------------------------

cLanguageFile::~cLanguageFile() {
	STLMapDeleteAll(m_mapCategories);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cLanguageFile::LoadFromFile(const tString asFile) {
	TiXmlDocument *pDoc = hplNew(TiXmlDocument, (asFile.c_str()));
	if (pDoc->LoadFile() == false) {
		hplDelete(pDoc);
		Error("Couldn't find language file '%s'\n", asFile.c_str());
		return false;
	}

	TiXmlElement *pRootElem = pDoc->FirstChildElement();

	///////////////////////////
	// Iterate the resources
	TiXmlElement *pResourceElem = pRootElem->FirstChildElement("RESOURCES");
	if (pResourceElem) {
		TiXmlElement *pDirElem = pResourceElem->FirstChildElement("Directory");
		for (; pDirElem != NULL; pDirElem = pDirElem->NextSiblingElement("Directory")) {
			tString sPath = pDirElem->Attribute("Path");
			mpResources->AddResourceDir(sPath);
		}
	} else {
		Warning("No resources element found in '%s'\n", asFile.c_str());
	}

	///////////////////////////
	// Iterate the categories
	TiXmlElement *pCatElem = pRootElem->FirstChildElement("CATEGORY");
	for (; pCatElem != NULL; pCatElem = pCatElem->NextSiblingElement("CATEGORY")) {
		cLanguageCategory *pCategory = hplNew(cLanguageCategory, ());
		tString sCatName = pCatElem->Attribute("Name");

		m_mapCategories.insert(tLanguageCategoryMap::value_type(sCatName, pCategory));

		///////////////////////////
		// Iterate the entries
		TiXmlElement *pEntryElem = pCatElem->FirstChildElement("Entry");
		for (; pEntryElem != NULL; pEntryElem = pEntryElem->NextSiblingElement("Entry")) {
			cLanguageEntry *pEntry = hplNew(cLanguageEntry, ());
			tString sEntryName = pEntryElem->Attribute("Name");

			if (pEntryElem->FirstChild() == NULL) {
				pEntry->mwsText = _W("");
			} else {
				TiXmlText *pTextNode = pEntryElem->FirstChild()->ToText();
				if (pTextNode) {
					// pEntry->msText = pTextNode->Value();
					// pEntry->msText = cString::ReplaceStringTo(pEntry->msText,"[br]","\n");

					tString sString = pTextNode->Value();
					pEntry->mwsText = _W("");

					// if(sCatName == "TEST") Log("String: '%s' %d\n",sString.c_str(),sString.size());

					for (size_t i = 0; i < sString.size(); ++i) {
						unsigned char c = sString[i];
						if (c == '[') {
							bool bFoundCommand = true;
							tString sCommand = "";
							int lCount = 1;

							while (sString[i + lCount] != ']' && i + lCount < sString.size() && lCount < 16) {
								sCommand += sString[i + lCount];
								lCount++;
							}

							if (sCommand == "br") {
								pEntry->mwsText += _W('\n');
							} else if (sCommand[0] == 'u') {
								int lNum = cString::ToInt(sCommand.substr(1).c_str(), 0);
								pEntry->mwsText += (wchar_t)lNum;
							} else {
								bFoundCommand = false;
							}

							// Go forward or add [ to string
							if (bFoundCommand) {
								i += lCount;
							} else {
								pEntry->mwsText += sString[i];
							}
						}
						// Decode UTF-8!
						else if (c >= 128) {
							unsigned char c2 = sString[i + 1];

							int lNum = c & 0x1f; // c AND 0001 1111
							lNum = lNum << 6;
							lNum = lNum | (c2 & 0x3f); // c AND 0011 1111

							pEntry->mwsText += (wchar_t)lNum;
							++i;
							// Log(" %d: (%x %x) -> %d\n",i,c,c2,(wchar_t)lNum);
						} else {
							// if(sCatName == "TEST") Log(" %d: %c | %d\n",i,c,c);
							pEntry->mwsText += c;
							// if(sCatName == "TEST") Log(" '%s'\n",cString::To8Char(pEntry->mwsText).c_str());
						}
					}
				}
			}

			// if(sE == "Motion blur:") Log("After String: '%s'\n",cString::To8Char(pEntry->mwsText).c_str());

			auto ret = pCategory->m_mapEntries.insert(tLanguageEntryMap::value_type(sEntryName, pEntry));
			if (ret.second == false) {
				Warning("Language entry '%s' in category '%s' already exists!\n", sEntryName.c_str(), sCatName.c_str());
				hplDelete(pEntry);
			}
		}
	}

	hplDelete(pDoc);

	/*{
		tString sRawFile = cString::SetFileExt(asFile,"")+"_raw_text.txt";
		Log("Saving raw text '%s'\n", sRawFile.c_str());
		FILE *pFile = fopen(sRawFile.c_str(),"w+");
		if(pFile==NULL) Error("Could not open file: '%s'\n", sRawFile.c_str());

		tLanguageCategoryMapIt CatIt = m_mapCategories.begin();
		for(; CatIt != m_mapCategories.end(); ++CatIt)
		{
			cLanguageCategory *pCategory = CatIt->second;
			tLanguageEntryMapIt EntryIt = pCategory->m_mapEntries.begin();
			for(; EntryIt != pCategory->m_mapEntries.end(); ++EntryIt)
			{
				cLanguageEntry *pEntry = EntryIt->second;
				fputs(cString::To8Char(pEntry->mwsText + _W("\n\n")).c_str(), pFile);
			}
		}


		fclose(pFile);
	}*/

	return true;
}

//-----------------------------------------------------------------------

const tWString &cLanguageFile::Translate(const tString &asCat, const tString &asName) {
	tLanguageCategoryMapIt CatIt = m_mapCategories.find(asCat);
	if (CatIt == m_mapCategories.end()) {
		Warning("Could not find language file category '%s'\n", asCat.c_str());
		return mwsEmpty;
	}

	cLanguageCategory *pCategory = CatIt->second;
	tLanguageEntryMapIt EntryIt = pCategory->m_mapEntries.find(asName);
	if (EntryIt == pCategory->m_mapEntries.end()) {
		Warning("Could not find language file entry '%s'\n", asName.c_str());
		return mwsEmpty;
	}

	cLanguageEntry *pEntry = EntryIt->second;

	return pEntry->mwsText;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
} // namespace hpl
