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

class Table;
class Parallaction;
class Gfx;
class Script;

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

	uint32			_numFiles;

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

class Disk {

protected:
	Archive			_resArchive;
	Archive			_locArchive;
	char		  	_languageDir[3];
	Parallaction 	*_vm;

protected:
	void errorFileNotFound(const char *s);

public:
	Disk(Parallaction *vm);
	virtual ~Disk();

	void selectArchive(const char *name);
	void setLanguage(uint16 language);

	virtual Script* loadLocation(const char *name) = 0;
	virtual Script* loadScript(const char* name) = 0;
	virtual Cnv* loadTalk(const char *name) = 0;
	virtual Cnv* loadObjects(const char *name) = 0;
	virtual StaticCnv* loadPointer() = 0;
	virtual StaticCnv* loadHead(const char* name) = 0;
	virtual Cnv* loadFont(const char* name) = 0;
	virtual StaticCnv* loadStatic(const char* name) = 0;
	virtual Cnv* loadFrames(const char* name) = 0;
	virtual void loadSlide(const char *filename) = 0;
	virtual void loadScenery(const char* background, const char* mask) = 0;
	virtual Table* loadTable(const char* name) = 0;

};

class DosDisk : public Disk {

private:
	void unpackBackground(Common::ReadStream *stream, byte *screen, byte *mask, byte *path);
	Cnv* loadExternalCnv(const char *filename);
	Cnv* loadCnv(const char *filename);
	StaticCnv *loadExternalStaticCnv(const char *filename);
	void loadBackground(const char *filename);
	void loadMaskAndPath(const char *name);
	void parseDepths(Common::SeekableReadStream &stream);
	void parseBackground(Common::SeekableReadStream &stream);

protected:
	Gfx	 *_gfx;

public:
	DosDisk(Parallaction *vm);
	virtual ~DosDisk();

	Script* loadLocation(const char *name);
	Script* loadScript(const char* name);
	Cnv* loadTalk(const char *name);
	Cnv* loadObjects(const char *name);
	StaticCnv* loadPointer();
	StaticCnv* loadHead(const char* name);
	Cnv* loadFont(const char* name);
	StaticCnv* loadStatic(const char* name);
	Cnv* loadFrames(const char* name);
	void loadSlide(const char *filename);
	void loadScenery(const char* background, const char* mask);
	Table* loadTable(const char* name);
};

class AmigaDisk : public Disk {

protected:
	Cnv* makeCnv(Common::SeekableReadStream &stream);
	StaticCnv* makeStaticCnv(Common::SeekableReadStream &stream);
	void unpackBitmap(byte *dst, byte *src, uint16 numFrames, uint16 planeSize);
	Common::SeekableReadStream *openArchivedFile(const char* name, bool errorOnFileNotFound = false);

public:
	AmigaDisk(Parallaction *vm);
	virtual ~AmigaDisk();

	Script* loadLocation(const char *name);
	Script* loadScript(const char* name);
	Cnv* loadTalk(const char *name);
	Cnv* loadObjects(const char *name);
	StaticCnv* loadPointer();
	StaticCnv* loadHead(const char* name);
	Cnv* loadFont(const char* name);
	StaticCnv* loadStatic(const char* name);
	Cnv* loadFrames(const char* name);
	void loadSlide(const char *filename);
	void loadScenery(const char* background, const char* mask);
	Table* loadTable(const char* name);
};

} // namespace Parallaction



#endif
