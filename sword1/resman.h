/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef RESMAN_H
#define RESMAN_H

#include "memman.h"
#include "file.h"
#include "sworddefs.h"

namespace Sword1 {

#define MAX_LABEL_SIZE (31+1)

struct Grp {
	uint32 noRes;
	MemHandle *resHandle;
	uint32 *offset;
	uint32 *length;
};

struct Clu {
	char label[MAX_LABEL_SIZE];
	uint32 noGrp;
	Grp **grp;
};

struct Prj {
	uint32 noClu;
	Clu **clu;
};

class ResMan {
public:
	ResMan(const char *resFile, MemMan *pMemoMan);
	~ResMan(void);
	void flush(void);
	void resClose(uint32 id);
	void resOpen(uint32 id);
	void *fetchRes(uint32 id);
	void dumpRes(uint32 id);
	void *openFetchRes(uint32 id);
	void *cptResOpen(uint32 id);
	Header *lockScript(uint32 scrID);
	void unlockScript(uint32 scrID);
	FrameHeader *fetchFrame(void *resourceData, uint32 frameNo);
private:
	uint32 filesInGroup(uint32 id);
	uint32 resLength(uint32 id);
	MemHandle *resHandle(uint32 id);
	uint32 resOffset(uint32 id);
	void openCptResourceBigEndian(uint32 id);
	void openScriptResourceBigEndian(uint32 id);

	File *openClusterFile(uint32 id);
	void loadCluDescript(const char *fileName);
	void freeCluDescript(void);
	Prj _prj;
	MemMan *_memMan;
	static const uint32 _scriptList[TOTAL_SECTIONS];	//a table of resource tags
	static uint32 _srIdList[29];
};

} // End of namespace Sword1 

#endif //RESMAN_H
