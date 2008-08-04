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
 * $URL$
 * $Id$
 *
 * String resource managment routines
 */

#include "tinsel/dw.h"
#include "tinsel/sound.h"
#include "tinsel/strres.h"
#include "common/file.h"
#include "common/endian.h"

namespace Tinsel {

#ifdef DEBUG
// Diagnostic number
int newestString;
#endif

// buffer for resource strings
static uint8 *textBuffer = 0;

// language resource string filenames
static const char *languageFiles[] = {
	"english.txt",
	"french.txt",
	"german.txt",
	"italian.txt",
	"spanish.txt"
};

// Set if we're handling 2-byte characters.
bool bMultiByte = false;

/**
 * Called to load a resource file for a different language
 * @param newLang			The new language
 */
void ChangeLanguage(LANGUAGE newLang) {
	Common::File f;
	uint32 textLen = 0;	// length of buffer

	if (textBuffer) {
		// free the previous buffer
		free(textBuffer);
		textBuffer = NULL;
	}

	// Try and open the specified language file. If it fails, and the language
	// isn't English, try falling back on opening 'english.txt' - some foreign
	// language versions reused it rather than their proper filename
	if (!f.open(languageFiles[newLang])) {
		if ((newLang == TXT_ENGLISH) || !f.open(languageFiles[TXT_ENGLISH]))
			error("Cannot find file %s", languageFiles[newLang]);
	}

	// Check whether the file is compressed or not -  for compressed files the 
	// first long is the filelength and for uncompressed files it is the chunk 
	// identifier
	textLen = f.readUint32LE();
	if (f.ioFailed())
		error("File %s is corrupt", languageFiles[newLang]);

	if (textLen == CHUNK_STRING || textLen == CHUNK_MBSTRING) {
		// the file is uncompressed

		bMultiByte = (textLen == CHUNK_MBSTRING);

		// get length of uncompressed file
		textLen = f.size();
		f.seek(0, SEEK_SET);	// Set to beginning of file

		if (textBuffer == NULL) {
			// allocate a text buffer for the strings
			textBuffer = (uint8 *)malloc(textLen);

			// make sure memory allocated
			assert(textBuffer);
		}

		// load data
		if (f.read(textBuffer, textLen) != textLen)
			// file must be corrupt if we get to here
			error("File %s is corrupt", languageFiles[newLang]);

		// close the file
		f.close();
	} else {	// the file must be compressed
		error("Compression handling has been removed!");
	}
}

/**
 * Loads a string resource identified by id.
 * @param id			identifier of string to be loaded
 * @param pBuffer		points to buffer that receives the string
 * @param bufferMax		maximum number of chars to be copied to the buffer
 */
int LoadStringRes(int id, char *pBuffer, int bufferMax) {
#ifdef DEBUG
	// For diagnostics
	newestString = id;
#endif

	// base of string resource table
	uint8 *pText = textBuffer;

	// index into text resource file
	uint32 index = 0;

	// number of chunks to skip
	int chunkSkip = id / STRINGS_PER_CHUNK;

	// number of strings to skip when in the correct chunk
	int strSkip = id % STRINGS_PER_CHUNK;

	// length of string
	int len;

	// skip to the correct chunk
	while (chunkSkip-- != 0) {
		// make sure chunk id is correct
		assert(READ_LE_UINT32(pText + index) == CHUNK_STRING || READ_LE_UINT32(pText + index) == CHUNK_MBSTRING);

		if (READ_LE_UINT32(pText + index + sizeof(uint32)) == 0) {
			// TEMPORARY DIRTY BODGE
			strcpy(pBuffer, "!! HIGH STRING !!");

			// string does not exist
			return 0;
		}

		// get index to next chunk
		index = READ_LE_UINT32(pText + index + sizeof(uint32));
	}

	// skip over chunk id and offset
	index += (2 * sizeof(uint32));

	// pointer to strings
	pText = pText + index;

	// skip to the correct string
	while (strSkip-- != 0) {
		// skip to next string
		pText += *pText + 1;
	}

	// get length of string
	len = *pText;

	if (len) {
		// the string exists

		// copy the string to the buffer
		if (len < bufferMax) {
			memcpy(pBuffer, pText + 1, len);

			// null terminate
			pBuffer[len] = 0;

			// number of chars copied
			return len + 1;
		} else {
			memcpy(pBuffer, pText + 1, bufferMax - 1);

			// null terminate
			pBuffer[bufferMax - 1] = 0;

			// number of chars copied
			return bufferMax;
		}
	}

	// TEMPORARY DIRTY BODGE
	strcpy(pBuffer, "!! NULL STRING !!");

	// string does not exist
	return 0;
}

void FreeTextBuffer() {
	if (textBuffer) {
		free(textBuffer);
		textBuffer = NULL;
	}
}

} // end of namespace Tinsel
