/* Copyright (C) 2003 The ScummVM project
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
#include "driver96.h"
#include "../sword2.h"

uint32 timeGetTime(void) {
	return g_bs2->_syst->get_msecs();
}

void VirtualUnlock(uint8 *free_memman, uint32 total_free_memory) {
	warning("stub VirtualUnlock");
}

void GlobalMemoryStatus(MEMORYSTATUS *memo) {
	warning("stub GlobalMemoryStatus");
	memo->dwTotalPhys = 16000*1024; // hard code 16mb for now
}

void SetFileAttributes(char *file, uint32 atrib) {
	warning("stub SetFileAttributes");
}

void DeleteFile(char *file) {
	warning("stub DeleteFile");
}

void GetCurrentDirectory(uint32 max, char* path) {
	warning("stub GetCurrentDirectory");
}

int32 GetVolumeInformation(char *cdPath, char *sCDName, uint32 maxPath, uint8 *, DWORD *dwMaxCompLength, DWORD *dwFSFlags, uint8 *, uint32 a) {
	warning("stub GetVolumeInformation %s", cdPath);
	strcpy(sCDName, CD1_LABEL);
	return 1;
}

// FIXME wrap different platform specific mkdir calls and actually do something
void _mkdir(const char *pathname) {
	warning("stub _mkdir %s", pathname);
}

void GetModuleFileName(void *module, char *destStr, uint32 maxLen) {
	warning("stub GetModuleFileName");
}

