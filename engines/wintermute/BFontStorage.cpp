/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "dcgf.h"
#include "BFontStorage.h"
#include "BGame.h"
#include "BFont.h"
#include "PlatformSDL.h"
#include "common/str.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_PERSISTENT(CBFontStorage, true)

//////////////////////////////////////////////////////////////////////////
CBFontStorage::CBFontStorage(CBGame *inGame): CBBase(inGame) {
	m_FTLibrary = NULL;
	InitFreeType();
}


//////////////////////////////////////////////////////////////////////////
CBFontStorage::~CBFontStorage() {
	Cleanup(true);
}


//////////////////////////////////////////////////////////////////////////
void CBFontStorage::InitFreeType() {
	FT_Error error = FT_Init_FreeType(&m_FTLibrary);
	if (error) {
		Game->LOG(0, "Error initializing FreeType library.");
	}
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBFontStorage::Cleanup(bool Warn) {
	int i;

	for (i = 0; i < m_Fonts.GetSize(); i++) {
		if (Warn) Game->LOG(0, "Removing orphan font '%s'", m_Fonts[i]->m_Filename);
		delete m_Fonts[i];
	}
	m_Fonts.RemoveAll();


	if (m_FTLibrary) FT_Done_FreeType(m_FTLibrary);


	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBFontStorage::InitLoop() {
	for (int i = 0; i < m_Fonts.GetSize(); i++) {
		m_Fonts[i]->InitLoop();
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
CBFont *CBFontStorage::AddFont(char *Filename) {
	if (!Filename) return NULL;

	for (int i = 0; i < m_Fonts.GetSize(); i++) {
		if (scumm_stricmp(m_Fonts[i]->m_Filename, Filename) == 0) {
			m_Fonts[i]->m_RefCount++;
			return m_Fonts[i];
		}
	}

	/*
	CBFont* font = new CBFont(Game);
	if(!font) return NULL;

	if(FAILED(font->LoadFile(Filename))){
	    delete font;
	    return NULL;
	}
	else {
	    font->m_RefCount = 1;
	    m_Fonts.Add(font);
	    return font;
	}
	*/
	CBFont *font = CBFont::CreateFromFile(Game, Filename);
	if (font) {
		font->m_RefCount = 1;
		m_Fonts.Add(font);
	}
	return font;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFontStorage::RemoveFont(CBFont *Font) {
	if (!Font) return E_FAIL;

	for (int i = 0; i < m_Fonts.GetSize(); i++) {
		if (m_Fonts[i] == Font) {
			m_Fonts[i]->m_RefCount--;
			if (m_Fonts[i]->m_RefCount <= 0) {
				delete m_Fonts[i];
				m_Fonts.RemoveAt(i);
			}
			break;
		}
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFontStorage::Persist(CBPersistMgr *PersistMgr) {

	if (!PersistMgr->m_Saving) Cleanup(false);

	PersistMgr->Transfer(TMEMBER(Game));
	m_Fonts.Persist(PersistMgr);

	if (!PersistMgr->m_Saving) InitFreeType();

	return S_OK;
}

} // end of namespace WinterMute
