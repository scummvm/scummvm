/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef COMMON_SAVEFILE_H
#define COMMON_SAVEFILE_H

#include <stdio.h>
#include <string.h>

class SaveFile {
public:
	virtual ~SaveFile() {}
	virtual int fread(void *buf, int size, int cnt) = 0;
	virtual int fwrite(void *buf, int size, int cnt) = 0;
#ifdef _WIN32_WCE
	//Should go away.  See scumm/saveload.cpp and scumm/imuse.cpp
	virtual int fseek(long offs, int whence) = 0;
	virtual int feof() = 0;
#endif
};

class StdioSaveFile : public SaveFile {
private:
	FILE *fh;
public:
	StdioSaveFile(const char *filename, const char *mode)
		{ fh = ::fopen(filename, mode); }
	~StdioSaveFile()
		{ if(fh) ::fclose(fh); }


	bool is_open() { return fh != NULL; }


	int fread(void *buf, int size, int cnt)
		{ return ::fread(buf, size, cnt, fh); }
	int fwrite(void *buf, int size, int cnt)
		{ return ::fwrite(buf, size, cnt, fh); }	
#ifdef _WIN32_WCE
	int fseek(long offs, int whence)
		{ return ::fseek(fh, offs, whence); }
	int feof()
		{ return ::feof(fh); }
#endif
};

class SaveFileManager {

public:
	virtual ~SaveFileManager() {}

	virtual SaveFile *open_savefile(const char *filename, bool saveOrLoad) {
		StdioSaveFile *sf = new StdioSaveFile(filename,
						      (saveOrLoad? "wb":"rb"));
		if (!sf->is_open()) {
			delete sf;
			sf = NULL;
		}
		return sf;
	}

	virtual void list_savefiles(const char * /* prefix */,  bool *marks, int num) {
		memset(marks, true, num * sizeof(bool));
	}

};

#endif
