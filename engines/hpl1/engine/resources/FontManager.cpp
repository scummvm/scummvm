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

#include "hpl1/engine/resources/FontManager.h"
#include "hpl1/engine/graphics/Graphics.h"
#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/resources/ImageManager.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/engine/graphics/font_data.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cFontManager::cFontManager(cGraphics *apGraphics, cGui *apGui, cResources *apResources)
	: iResourceManager(apResources->GetFileSearcher(), apResources->GetLowLevel(),
					   apResources->GetLowLevelSystem()) {
	mpGraphics = apGraphics;
	mpResources = apResources;
	mpGui = apGui;
}

cFontManager::~cFontManager() {
	DestroyAll();
	Log(" Done with fonts\n");
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

FontData *cFontManager::CreateFontData(const tString &asName, int alSize, unsigned short alFirstChar,
									   unsigned short alLastChar) {
	tString sPath;
	FontData *pFont;
	tString asNewName = cString::ToLowerCase(asName);

	BeginLoad(asName);

	// asNewName = cString::SetFileExt(asName,"ttf");

	pFont = static_cast<FontData *>(this->FindLoadedResource(asNewName, sPath));

	if (pFont == NULL && sPath != "") {
		pFont = mpGraphics->GetLowLevel()->CreateFontData(asNewName);
		pFont->setUp(mpGraphics->GetDrawer(), mpLowLevelResources, mpGui);

		tString sExt = cString::ToLowerCase(cString::GetFileExt(asName));

		// True Type Font
		if (sExt == "ttf") {
			if (pFont->createFromFontFile(sPath, alSize, alFirstChar, alLastChar) == false) {
				hplDelete(pFont);
				EndLoad();
				return NULL;
			}
		}
		// Angel code font type
		else if (sExt == "fnt") {
			if (pFont->createFromBitmapFile(sPath) == false) {
				hplDelete(pFont);
				EndLoad();
				return NULL;
			}
		} else {
			Error("Font '%s' has an unkown extension!\n", asName.c_str());
			hplDelete(pFont);
			EndLoad();
			return NULL;
		}

		// mpResources->GetImageManager()->FlushAll();
		AddResource(pFont);
	}

	if (pFont)
		pFont->IncUserCount();
	else
		Error("Couldn't create font '%s'\n", asNewName.c_str());

	EndLoad();
	return pFont;
}

//-----------------------------------------------------------------------

iResourceBase *cFontManager::Create(const tString &asName) {
	return CreateFontData(asName, 16, 32, 255);
}

//-----------------------------------------------------------------------

void cFontManager::Unload(iResourceBase *apResource) {
}
//-----------------------------------------------------------------------

void cFontManager::Destroy(iResourceBase *apResource) {
	apResource->DecUserCount();

	if (apResource->HasUsers() == false) {
		RemoveResource(apResource);
		hplDelete(apResource);
	}
}

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
} // namespace hpl
