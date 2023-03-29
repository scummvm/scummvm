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

#include "hpl1/engine/sound/SoundEntityData.h"

#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/engine/impl/tinyXML/tinyxml.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cSoundEntityData::cSoundEntityData(tString asName) : iResourceBase(asName, 0) {
	msMainSound = "";
	msStartSound = "";
	msStopSound = "";

	mbFadeStart = false;
	mbFadeStop = false;

	mfVolume = 1;
	mfMaxDistance = 0;
	mfMinDistance = 0;

	mbStream = false;
	mbLoop = false;
	mbUse3D = true;

	mfRandom = 1;
	mfInterval = 0;
}

//-----------------------------------------------------------------------

cSoundEntityData::~cSoundEntityData() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cSoundEntityData::CreateFromFile(const tString &asFile) {
	TiXmlDocument *pDoc = hplNew(TiXmlDocument, ());
	if (pDoc->LoadFile(asFile.c_str()) == false) {
		Error("Couldn't load '%s'!\n", asFile.c_str());
		hplDelete(pDoc);
		return false;
	}

	TiXmlElement *pRootElem = pDoc->FirstChildElement();

	////////////////////////////////////////////////
	// MAIN
	TiXmlElement *pMainElem = pRootElem->FirstChildElement("MAIN");
	if (pMainElem == NULL) {
		Error("Couldn't find MAIN element in '%s'!\n", asFile.c_str());
		hplDelete(pDoc);
		return false;
	}

	msMainSound = cString::ToString(pMainElem->Attribute("MainSound"), "");
	msStartSound = cString::ToString(pMainElem->Attribute("StartSound"), "");
	msStopSound = cString::ToString(pMainElem->Attribute("StopSound"), "");

	////////////////////////////////////////////////
	// PROPERTIES
	TiXmlElement *pPropElem = pRootElem->FirstChildElement("PROPERTIES");
	if (pPropElem == NULL) {
		Error("Couldn't find PROPERTIES element in '%s'!\n", asFile.c_str());
		hplDelete(pDoc);
		return false;
	}

	mbUse3D = cString::ToBool(pPropElem->Attribute("Use3D"), true);
	mbLoop = cString::ToBool(pPropElem->Attribute("Loop"), true);
	mbStream = cString::ToBool(pPropElem->Attribute("Stream"), true);

	mbBlockable = cString::ToBool(pPropElem->Attribute("Blockable"), false);
	mfBlockVolumeMul = cString::ToFloat(pPropElem->Attribute("BlockVolumeMul"), 0.6f);

	mfVolume = cString::ToFloat(pPropElem->Attribute("Volume"), 1);
	mfMaxDistance = cString::ToFloat(pPropElem->Attribute("MaxDistance"), 1);
	mfMinDistance = cString::ToFloat(pPropElem->Attribute("MinDistance"), 1);

	mbFadeStart = cString::ToBool(pPropElem->Attribute("FadeStart"), true);
	mbFadeStop = cString::ToBool(pPropElem->Attribute("FadeStop"), true);

	mfRandom = cString::ToFloat(pPropElem->Attribute("Random"), 1);
	mfInterval = cString::ToFloat(pPropElem->Attribute("Interval"), 0);

	mlPriority = cString::ToInt(pPropElem->Attribute("Priority"), 0);

	hplDelete(pDoc);

	return true;
}

//-----------------------------------------------------------------------

} // namespace hpl
