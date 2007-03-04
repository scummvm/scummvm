/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
 */

#ifndef PARALLACTION_DISK_H
#define PARALLACTION_DISK_H

#include "parallaction/defs.h"
#include "common/file.h"

namespace Parallaction {

//------------------------------------------------------
//		ARCHIVE MANAGEMENT
//------------------------------------------------------


#define MAX_ARCHIVE_ENTRIES 		384

#define DIRECTORY_OFFSET_IN_FILE	0x4000

class Archive : public Common::SeekableReadStream {

protected:

	bool   			_file;
	uint32			_fileOffset;
	uint32			_fileCursor;
	uint32			_fileEndOffset;

	char 			_archiveDir[MAX_ARCHIVE_ENTRIES][32];
	uint32			_archiveLenghts[MAX_ARCHIVE_ENTRIES];
	uint32			_archiveOffsets[MAX_ARCHIVE_ENTRIES];

	Common::File 	_archive;

protected:
	void resetArchivedFile();

public:
	Archive();

	void open(const char *file);
	void close();

	bool openArchivedFile(const char *name);
	void closeArchivedFile();

	uint32 size() const;
	uint32 pos() const;
	bool eos() const;
	void seek(int32 offs, int whence = SEEK_SET);

	uint32 read(void *dataPtr, uint32 dataSize);
};

void loadLocation(const char *name, char* script);
void loadScript(const char* name, char *script);
void loadTalk(const char *name, Cnv *cnv);
void loadObjects(const char *name, Cnv *cnv);
void loadPointer(StaticCnv* cnv);
void loadHead(const char* name, StaticCnv* cnv);
void loadFont(const char* name, Cnv* cnv);
void loadStatic(const char* name, StaticCnv* cnv);
void loadFrames(const char* name, Cnv* cnv);
void loadSlide(const char *filename);
void loadScenery(const char *filename);
void loadBackground(const char* background, const char* mask);

} // namespace Parallaction



#endif
