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

#include "hpl1/engine/resources/ConfigFile.h"
#include "common/file.h"
#include "hpl1/debug.h"
#include "hpl1/engine/impl/tinyXML/tinyxml.h"
#include "hpl1/engine/resources/FileSearcher.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cConfigFile::cConfigFile(tWString asFile) {
	msFile = asFile;
	mpXmlDoc = hplNew(TiXmlDocument, ());
	// mpFileSearcher = apFileSearcher;
}

cConfigFile::~cConfigFile() {
	hplDelete(mpXmlDoc);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cConfigFile::Load() {
	Common::File cf;
	if (!cf.open(Common::String(msFile))) {
		Hpl1::logWarning(Hpl1::kDebugFilePath, "file %S could not be opened", msFile.c_str());
		return false;
	}
	return mpXmlDoc->LoadFile(cf);
}

//-----------------------------------------------------------------------

bool cConfigFile::Save() {
	return mpXmlDoc->SaveFile(Common::String(msFile).c_str());
}

//-----------------------------------------------------------------------

void cConfigFile::SetString(tString asLevel, tString asName, tString asVal) {
	TiXmlElement *pLevelElem = mpXmlDoc->FirstChildElement(asLevel.c_str());

	if (pLevelElem == NULL) {
		TiXmlElement *pNodeChild = hplNew(TiXmlElement, (asLevel.c_str()));
		pLevelElem = static_cast<TiXmlElement *>(mpXmlDoc->InsertEndChild(*pNodeChild));
		hplDelete(pNodeChild);
	}

	pLevelElem->SetAttribute(asName.c_str(), asVal.c_str());
}
//-----------------------------------------------------------------------

void cConfigFile::SetInt(tString asLevel, tString asName, int alVal) {
	char sBuffer[40];
	snprintf(sBuffer, 40, "%d", alVal);

	SetString(asLevel, asName, sBuffer);
}

//-----------------------------------------------------------------------

void cConfigFile::SetFloat(tString asLevel, tString asName, float afVal) {
	char sBuffer[40];
	snprintf(sBuffer, 40, "%f", afVal);

	SetString(asLevel, asName, sBuffer);
}

//-----------------------------------------------------------------------

void cConfigFile::SetBool(tString asLevel, tString asName, bool abVal) {
	SetString(asLevel, asName, abVal ? "true" : "false");
}

tString cConfigFile::GetString(tString asLevel, tString asName, tString asDefault) {
	const char *sVal = GetCharArray(asLevel, asName);
	if (sVal == NULL) {
		return asDefault;
	}

	return sVal;
}

int cConfigFile::GetInt(tString asLevel, tString asName, int alDefault) {
	const char *sVal = GetCharArray(asLevel, asName);
	if (sVal == NULL)
		return alDefault;

	return cString::ToInt(sVal, alDefault);
}

float cConfigFile::GetFloat(tString asLevel, tString asName, float afDefault) {
	const char *sVal = GetCharArray(asLevel, asName);
	if (sVal == NULL)
		return afDefault;

	return cString::ToFloat(sVal, afDefault);
}

bool cConfigFile::GetBool(tString asLevel, tString asName, bool abDefault) {
	const char *sVal = GetCharArray(asLevel, asName);
	if (sVal == NULL)
		return abDefault;

	return cString::ToBool(sVal, abDefault);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

const char *cConfigFile::GetCharArray(tString asLevel, tString asName) {
	TiXmlElement *pLevelElem = mpXmlDoc->FirstChildElement(asLevel.c_str());
	if (pLevelElem == NULL) {
		return NULL;
	}

	return pLevelElem->Attribute(asName.c_str());
}

//-----------------------------------------------------------------------
} // namespace hpl
