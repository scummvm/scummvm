/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2002-2005 The ScummVM Team
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

#include "stdafx.h"
#include "common/scummsys.h"
#include "common/stream.h"


class SaveFile : public Common::ReadStream, public Common::WriteStream {
public:
	virtual ~SaveFile() {}

	virtual bool isOpen() const = 0;
};

class SaveFileManager {

public:
	virtual ~SaveFileManager() {}

	/**
	 * Open the file with name filename in the given directory for saving or loading.
	 * @param filename	the filename
	 * @param directory	the directory
	 * @param saveOrLoad	true for saving, false for loading
	 * @return pointer to a SaveFile object
	 */
	virtual SaveFile *openSavefile(const char *filename, bool saveOrLoad) = 0;
	virtual void listSavefiles(const char * /* prefix */, bool *marks, int num) = 0;

	/** Get the path to the save game directory. */
	virtual const char *getSavePath() const;
};

class DefaultSaveFileManager : public SaveFileManager {
public:
	virtual SaveFile *openSavefile(const char *filename, bool saveOrLoad);
	virtual void listSavefiles(const char * /* prefix */, bool *marks, int num);

protected:
	virtual SaveFile *makeSaveFile(const char *filename, bool saveOrLoad);
};

#endif
