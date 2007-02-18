/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 Neil Millstone
 * Copyright (C) 2006 The ScummVM project
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
 */
 
#ifndef _GBAMPSAVE_H_
#define _GBAMPSAVE_H_

#include "stdafx.h"
#include "system.h"

#define SAVE_BUFFER_SIZE 100000

class GBAMPSaveFile : public Common::InSaveFile, public Common::OutSaveFile {
	FILE* handle;
	char buffer[SAVE_BUFFER_SIZE];
	int bufferPos;
	int saveSize;
	int flushed;

public:
	GBAMPSaveFile(char* name, bool saveOrLoad);
	~GBAMPSaveFile();
		
	virtual uint32 read(void *buf, uint32 size);
	virtual uint32 write(const void *buf, uint32 size);
	
	virtual bool eos() const;
	virtual void skip(uint32 bytes);

	virtual uint32 pos() const;
	virtual uint32 size() const;
	virtual void seek(int32 pos, int whence);
	
	void flushSaveBuffer();
	
	virtual bool isOpen() const {
		return handle != 0;
	}
};


class GBAMPSaveFileManager : public Common::SaveFileManager {

	
public:
	GBAMPSaveFileManager();
	~GBAMPSaveFileManager();
	
//	static GBAMPSaveFileManager* instance() { return instancePtr; }

	GBAMPSaveFile *openSavefile(const char *filename, bool saveOrLoad);
	
	virtual Common::OutSaveFile* openForSaving(const char* filename) { return openSavefile(filename, true); }
	virtual Common::InSaveFile* openForLoading(const char* filename) { return openSavefile(filename, false); }
	
	
	void listSavefiles(const char *prefix, bool *marks, int num);
	
	void deleteFile(char* name);
	void listFiles();
};


#endif
