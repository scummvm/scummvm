/* Copyright (C) 1994-2003 Revolution Software Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

//=============================================================================
//
//	Filename	:	language.c
//	Created		:	20th August 1996
//	By			:	P.R.Porter
//
//	Summary		:	This module holds the functions which govern which language
//					version is current.
//
//
//	Version	Date		By		Description
//	------- ---------	---		-----------------------------------------------
//	1.0		16-Sep-96	PRP		Initial language version control functions.
//								Many of the functions are currently stubs, and
//								this will be addressed shortly.
//
//
//	Functions
//	---------
//
//	--------------------------------------------------------------------------
//
//	int32 GetLanguageVersion(uint8 *version)
//
//	This function modifies the 'version' passed in to be the current language.
//	The first time this function is called, it gets the language from the 
//	version.inf file distributed on the game CD.  It returns an RD error code
//	if this file cannot be opened, or the version cannot be obtained from it.
//
//	---------------------------------------------------------------------------
//	
//	int32 SetLanguageVersion(uint8 version)
//
//	This function is useful for debugging.  It sets the version to the one
//	passed in.
//
//	---------------------------------------------------------------------------
//	
//	int32 GetGameName(uint8 *name);
//
//	Fills the string pointed to by name with the title of the game, depending
//	upon what the current language version is.
//
//=============================================================================

#include "stdafx.h"
#include "driver96.h"

uint8 languageVersion = ENGLISH;

static uint8 versionFromFile = 0;

int32 GetLanguageVersion(uint8 *version) {
	if (versionFromFile) {
		*version = languageVersion;
	} else {
		versionFromFile = 1;
		languageVersion = AMERICAN;
		return(RDERR_OPENVERSIONFILE);
	}
	return RD_OK;
}


int32 SetLanguageVersion(uint8 version) {
	languageVersion = version;
	return RD_OK;
}

int32 GetGameName(uint8 *name) {
	uint8 version;
	int32 rv;

	rv = GetLanguageVersion(&version);

	switch (version) {
	case ENGLISH:
		strcpy((char *)name, "Broken Sword II");
		break;
	case AMERICAN:
		strcpy((char *)name, "Circle of Blood II");
		break;
	case GERMAN:
		strcpy((char *)name, "Baphomet's Fluch II");
		break;
	default:
		strcpy((char *)name, "Some game or other, part 86");
		return(RDERR_INVALIDVERSION);
	}

	return rv;
}
