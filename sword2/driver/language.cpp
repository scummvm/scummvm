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

#include "stdafx.h"
#include "sword2/driver/driver96.h"

namespace Sword2 {

// Language codes
#define ENGLISH		0x00
#define AMERICAN	0x01
#define GERMAN		0x02
#define FRENCH		0x03
#define SPANISH		0x04
#define ITIALIAN	0x05
#define JAPANESE	0x06
#define SLOVAK		0x07

uint8 languageVersion = ENGLISH;

static uint8 versionFromFile = 0;

/**
 * This function modifies the 'version' passed in to be the current language.
 * The first time this function is called, it gets the language from the
 * version.inf file distributed on the game CD.
 * @param version a pointer to the variable to store language information in
 * @return an RD error code if version.inf cannot be opened, or the version
 * cannot be obtained from it
 */

int32 GetLanguageVersion(uint8 *version) {
	if (versionFromFile) {
		*version = languageVersion;
	} else {
		versionFromFile = 1;
		languageVersion = AMERICAN;
		return RDERR_OPENVERSIONFILE;
	}
	return RD_OK;
}

/**
 * This function is useful for debugging. It sets the version to the one passed
 * in.
 */

int32 SetLanguageVersion(uint8 version) {
	languageVersion = version;
	return RD_OK;
}

} // End of namespace Sword2
