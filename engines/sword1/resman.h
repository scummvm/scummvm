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

#ifndef SWORD1_RESMAN_H
#define SWORD1_RESMAN_H

#include "sword1/memman.h"
#include "sword1/swordres.h"
#include "common/file.h"
#include "sword1/sworddefs.h"
#include "common/endian.h"
#include "common/mutex.h"

namespace Sword1 {

#define MAX_LABEL_SIZE (31+1)

#if defined(__PSP__)
#define MAX_OPEN_CLUS 4 // the PSP can't have more than 8 files open simultaneously
						// since we also need filehandles for music and sometimes savegames
						// set the maximum number of open clusters to 4.
#else
#define MAX_OPEN_CLUS 8 // don't open more than 8 files at once
#endif

struct Grp {
	uint32 noRes;
	MemHandle *resHandle;
	uint32 *offset;
	uint32 *length;
};

struct Clu {
	uint32 refCount;
	Common::File *file;
	char label[MAX_LABEL_SIZE];
	uint32 noGrp;
	Grp *grp;
	Clu *nextOpen;
};

struct Prj {
	uint32 noClu;
	Clu *clu;
};

class ResMan {
public:
	ResMan(const char *fileName, bool isMacFile, bool isKorean);
	~ResMan();
	void flush();
	void resClose(uint32 id);
	void resOpen(uint32 id);
	void *fetchRes(uint32 id);
	void dumpRes(uint32 id);
	void *openFetchRes(uint32 id);
	void *cptResOpen(uint32 id);
	Header *lockScript(uint32 scrID);
	void unlockScript(uint32 scrID);
	FrameHeader *fetchFrame(void *resourceData, uint32 frameNo);

	uint16 getUint16(uint16 value) {
		return (_isBigEndian) ? FROM_BE_16(value) : FROM_LE_16(value);
	}
	uint32 getUint32(uint32 value) {
		return (_isBigEndian) ? FROM_BE_32(value) : FROM_LE_32(value);
	}
	uint16 getLEUint16(uint16 value) {
		return FROM_LE_16(value);
	}
	uint32 getLEUint32(uint32 value) {
		return FROM_LE_32(value);
	}
	uint16 readUint16(const void *ptr) {
		return (_isBigEndian) ? READ_BE_UINT16(ptr) : READ_LE_UINT16(ptr);
	}
	uint32 readUint32(const void *ptr) {
		return (_isBigEndian) ? READ_BE_UINT32(ptr) : READ_LE_UINT32(ptr);
	}
	uint32 readLEUint32(const void *ptr) {
		return READ_LE_UINT32(ptr);
	}
	uint16 toUint16(uint16 value) {
		return (_isBigEndian) ? TO_BE_16(value) : TO_LE_16(value);
	}
	uint32 toUint32(uint32 value) {
		return (_isBigEndian) ? TO_BE_32(value) : TO_LE_32(value);
	}

	uint32 getDeathFontId();

private:
	uint32     resLength(uint32 id);
	MemHandle *resHandle(uint32 id);
	uint32     resOffset(uint32 id);
	Common::File      *resFile(uint32 id);

	void openCptResourceBigEndian(uint32 id);
	void openScriptResourceBigEndian(uint32 id);
	void openCptResourceLittleEndian(uint32 id);
	void openScriptResourceLittleEndian(uint32 id);

	void loadCluDescript(const char *fileName);
	void freeCluDescript();
	Prj _prj;
	MemMan *_memMan;
	static const uint32 _scriptList[TOTAL_SECTIONS];    //a table of resource tags
	Clu *_openCluStart, *_openCluEnd;
	int  _openClus;
	bool _isBigEndian;
	bool _isKorTrs = false;

	Common::Mutex _resourceAccessMutex;

	uint32 _srIdList[29] = {
		// the file numbers differ for the control panel file IDs, so we need this array
		OTHER_SR_FONT,    // SR_FONT
		0x04050000,       // SR_BUTTON
		OTHER_SR_REDFONT, // SR_REDFONT
		0x04050001,       // SR_PALETTE
		0x04050002,       // SR_PANEL_ENGLISH
		0x04050003,       // SR_PANEL_FRENCH
		0x04050004,       // SR_PANEL_GERMAN
		0x04050005,       // SR_PANEL_ITALIAN
		0x04050006,       // SR_PANEL_SPANISH
		0x04050007,       // SR_PANEL_AMERICAN
		0x04050008,       // SR_TEXT_BUTTON
		0x04050009,       // SR_SPEED
		0x0405000A,       // SR_SCROLL1
		0x0405000B,       // SR_SCROLL2
		0x0405000C,       // SR_CONFIRM
		0x0405000D,       // SR_VOLUME
		0x0405000E,       // SR_VLIGHT
		0x0405000F,       // SR_VKNOB
		0x04050010,       // SR_WINDOW
		0x04050011,       // SR_SLAB1
		0x04050012,       // SR_SLAB2
		0x04050013,       // SR_SLAB3
		0x04050014,       // SR_SLAB4
		0x04050015,       // SR_BUTUF
		0x04050016,       // SR_BUTUS
		0x04050017,       // SR_BUTDS
		0x04050018,       // SR_BUTDF
		0x04050019,       // SR_DEATHPANEL
		SR_DEATHFONT,
	};
};

} // End of namespace Sword1

#endif //RESMAN_H
