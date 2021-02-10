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

#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "trecision/nl/lib/addtype.h"
#include "trecision/trecision.h"

#include "common/str.h"
#include "common/textconsole.h"

namespace Trecision {

#define MAXFHANDLE		10
#define MAXFILES		500

static FILE				*hFile;
static FILEENTRY		pFE[MAXFILES];
static char				*pBase;
static int				dwFECnt;
static FILEHANDLE		lpFH[MAXFHANDLE];
static int				dwFHCnt;
static int				lFileEnd;

void CheckFileInCD(const char *name);
void CloseSys(const char *str);
void FastFileFinish();
void AnimFileFinish();
void SpeechFileFinish();

/* -----------------19/01/98 17.12-------------------
 * Compare
 *
 * bsearch comparison routine
 * --------------------------------------------------*/
int Compare(const void *p1, const void *p2) {
	LPFILEENTRY p1c = (LPFILEENTRY)p1, p2c = (LPFILEENTRY)p2;
	return (scumm_stricmp((p1c)->name, (p2c)->name));
}

/* -----------------19/01/98 17.13-------------------
 * FastFileInit
 *
 * Initialize for fast file access. The master file and maximum number
 * of open "files" are specified.
 * --------------------------------------------------*/
char FastFileInit(const char *fname) {
	FastFileFinish();

//	get a file handle array - just do it
	dwFHCnt = MAXFHANDLE - 1;

	hFile = fopen(fname, "rb");

	if (hFile == NULL) {
		warning("FastFileInit: CreateFile open error %s", fname);
		hFile = NULL;
		FastFileFinish();
		CloseSys(g_vm->_sysText[1]);
		return false;
	}

//	get initial data from the memory mapped file
	fread(&dwFECnt, 4, 1, hFile);
	fread(pFE, sizeof(FILEENTRY), dwFECnt, hFile);
	lFileEnd = pFE[dwFECnt - 1].offset;

	return true;

}

/* -----------------19/01/98 17.14-------------------
 * FastFileFinish
 *
 * Clean up resources
 * --------------------------------------------------*/
void FastFileFinish(void) {
	if (hFile != NULL) {
		fclose(hFile);
		hFile = NULL;
	}
	dwFHCnt = 0;
	pBase = NULL;
	dwFECnt = 0;

}

/* -----------------19/01/98 17.15-------------------
 * FastFileOpen
 *
 * Search the directory for the file, and return a file handle if found.
 * --------------------------------------------------*/
LPFILEHANDLE FastFileOpen(const char *name) {
	FILEENTRY fe;
	LPFILEENTRY pfe;

	if (pFE == NULL) {
		warning("FastFileOpen: not initialized");
		return NULL;
	}
	if (name == NULL || name[0] == 0) {
		warning("FastFileOpen: invalid name");
		return NULL;
	}

	strcpy(fe.name, name);
	pfe = (LPFILEENTRY)bsearch(&fe, pFE, dwFECnt, sizeof(FILEENTRY), Compare);
	if (pfe == NULL) {
		CheckFileInCD(name);
		pfe = (LPFILEENTRY)bsearch(&fe, pFE, dwFECnt, sizeof(FILEENTRY), Compare);
	}
	if (pfe != NULL) {
		for (int i = 0; i < dwFHCnt; i++) {
			if (!lpFH[i].inuse) {
				lpFH[i].inuse = true;
				lpFH[i].pos = pfe->offset;
				lpFH[i].size = (pfe + 1)->offset - pfe->offset;
				lpFH[i].pfe = pfe;
				fseek(hFile, lpFH[i].pos, SEEK_SET);
				return &lpFH[i];
			}
		}
		warning("FastFileOpen: Out of file handles");
	} else {
		warning("FastFileOpen: File %s not found", name);
		CloseSys(g_vm->_sysText[1]);
	}

	return NULL;

}

/* -----------------19/01/98 17.16-------------------
 * FastFileClose
 *
 * Mark a fast file handle as closed
 * --------------------------------------------------*/
char FastFileClose(LPFILEHANDLE pfh) {
	if (pfh == NULL || pfh->inuse != true) {
		warning("FastFileClose: invalid handle");
		return false;
	}
	pfh->inuse = false;
	return true;

}

/* -----------------19/01/98 17.17-------------------
 * FastFileRead
 *
 * read from a fast file (memcpy!)
 * --------------------------------------------------*/
int FastFileRead(LPFILEHANDLE pfh, void *ptr, int size) {
	if (pfh == NULL || pfh->inuse != true) {
		warning("FastFileRead: invalid handle");
		return false;
	}
	if (size < 0) {
		warning("FastFileRead: invalid size");
		return false;
	}
	if ((pfh->pos + size) > ((pfh->pfe) + 1)->offset) {
		warning("FastFileRead: read past end of file");
		return false;
	}
	fread(ptr, 1, size, hFile);
	pfh->pos += size;
	return size;

}

/* -----------------19/01/98 17.17-------------------
 * FastFileLen
 *
 * Get the current length in a fast file
 * --------------------------------------------------*/
int FastFileLen(LPFILEHANDLE pfh) {
	LPFILEENTRY pfe;

	if (pfh == NULL || pfh->inuse != true) {
		warning("FastFileTell: invalid handle");
		return -1;
	}
	pfe = pfh->pfe;
	return (pfe + 1)->offset - pfe->offset;

}

// BigFile
#define MAXSMACK	3
extern unsigned char _curSmackBuffer;
FILE *aFile[MAXSMACK] = {NULL, NULL, NULL}, *FmvFile = NULL;
FILEENTRY AFE[1000];
int AFNum;

/* -----------------19/01/98 17.13-------------------
 * AnimFileInit
 * --------------------------------------------------*/
char AnimFileInit(const char *fname) {
	AnimFileFinish();

	for (int a = 0; a < MAXSMACK; a++) {
		aFile[a] = fopen(fname, "rb");

		if (aFile[a] == NULL) {
			warning("AnimFileInit: CreateFile open error %s", fname);
			aFile[a] = NULL;
			AnimFileFinish();
			CloseSys(g_vm->_sysText[1]);
			return false;
		}
	}

	fread(&AFNum, 4, 1, aFile[0]);
	fread(AFE, sizeof(FILEENTRY), AFNum, aFile[0]);

	return true;
}

/* -----------------19/01/98 17.14-------------------
 * AnimFileFinish
 * --------------------------------------------------*/
void AnimFileFinish(void) {
	for (int a = 0; a < MAXSMACK; a++) {
		if (aFile[a] != NULL) {
			fclose(aFile[a]);
			aFile[a] = NULL;
		}
	}
	AFNum = 0;
}

/* -----------------19/01/98 17.15-------------------
 * AnimFileOpen
 * --------------------------------------------------*/
int AnimFileOpen(const char *name) {
	FILEENTRY fe;
	LPFILEENTRY pfe;

	if (name == NULL || name[0] == 0) {
		warning("AnimFileOpen: invalid name");
		return NULL;
	}

	strcpy(fe.name, name);
	pfe = (LPFILEENTRY)bsearch(&fe, AFE, AFNum, sizeof(FILEENTRY), Compare);
	if (pfe == NULL) {
		CheckFileInCD(name);
		pfe = (LPFILEENTRY)bsearch(&fe, AFE, AFNum, sizeof(FILEENTRY), Compare);
	}

	if (pfe != NULL) {
		fseek(aFile[_curSmackBuffer], pfe->offset, SEEK_SET);
		return (int)aFile[_curSmackBuffer];
	}

	warning("AnimFileOpen: File %s not found", name);
	CloseSys(g_vm->_sysText[1]);
	return NULL;
}

/* -----------------19/01/98 17.15-------------------
 * FmvFileOpen
 * --------------------------------------------------*/
int FmvFileOpen(const char *name) {
	extern char UStr[];

	if (FmvFile != NULL) fclose(FmvFile);
	FmvFile = NULL;

	if (name == NULL || name[0] == 0) {
		warning("FmvFileOpen: invalid name");
		return NULL;
	}

	sprintf(UStr, "%sFMV\\%s", g_vm->_gamePath, name);
	FmvFile = fopen(UStr, "rb");
	if (FmvFile != NULL)
		return (int)FmvFile;

	warning("Fmv file %s not found!", name);
	CloseSys(g_vm->_sysText[1]);
	return -1;
}


// SpeechFile
FILE *sFile = NULL;
FILEENTRY SFE[2000];
int SFNum;

/* -----------------04/08/98 11.33-------------------
 * SpeechFileInit
 * --------------------------------------------------*/
char SpeechFileInit(const char *fname) {
	SpeechFileFinish();

	sFile = fopen(fname, "rb");

	if (sFile == NULL) {
		warning("SpeechFileInit: CreateFile open error %s", fname);
		sFile = NULL;
		SpeechFileFinish();
		CloseSys(g_vm->_sysText[1]);
		return false;
	}

	fread(&SFNum, 4, 1, sFile);
	fread(SFE, sizeof(FILEENTRY), SFNum, sFile);

	return true;
}

/* -----------------04/08/98 11.33-------------------
 * SpeechFileFinish
 * --------------------------------------------------*/
void SpeechFileFinish(void) {
	if (sFile != NULL) {
		fclose(sFile);
		sFile = NULL;
	}
	SFNum = 0;
}

/* -----------------04/08/98 11.34-------------------
 * SpeechFileLen
 * --------------------------------------------------*/
int SpeechFileLen(const char *name) {
	FILEENTRY fe;
	LPFILEENTRY pfe;

	if (name == NULL || name[0] == 0) {
		warning("SpeechFileOpen: invalid name");
		return NULL;
	}

	strcpy(fe.name, name);
	pfe = (LPFILEENTRY)bsearch(&fe, SFE, SFNum, sizeof(FILEENTRY), Compare);
	if (pfe != NULL)
		return ((pfe + 1)->offset - pfe->offset);

	warning("SpeechFileOpen: File %s not found", name);
	CloseSys(g_vm->_sysText[1]);
	return NULL;
}

/* -----------------04/08/98 11.12-------------------
 * SpeechFileRead
 * --------------------------------------------------*/
int SpeechFileRead(const char *name, unsigned char *buf) {
	FILEENTRY fe;
	LPFILEENTRY pfe;

	if (name == NULL || name[0] == 0 || buf == NULL) {
		warning("SpeechFileOpen: invalid name");
		return NULL;
	}

	strcpy(fe.name, name);
	pfe = (LPFILEENTRY)bsearch(&fe, SFE, SFNum, sizeof(FILEENTRY), Compare);
	if (pfe == NULL) {
		CheckFileInCD(name);
		pfe = (LPFILEENTRY)bsearch(&fe, SFE, SFNum, sizeof(FILEENTRY), Compare);
	}

	if (pfe != NULL) {
		fseek(sFile, pfe->offset, SEEK_SET);
		return fread(buf, 1, ((pfe + 1)->offset - pfe->offset), sFile);
	}

	warning("SpeechFileOpen: File %s not found", name);
	CloseSys(g_vm->_sysText[1]);
	return NULL;
}

} // End of namespace Trecision
