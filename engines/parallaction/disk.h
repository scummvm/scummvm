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
 */

#ifndef PARALLACTION_DISK_H
#define PARALLACTION_DISK_H

#define PATH_LEN 200

#include "common/fs.h"

#include "common/file.h"
#include "graphics/surface.h"

namespace Parallaction {

class Table;
class Parallaction;
class Gfx;
class Script;
class Font;

struct Frames;
struct Cnv;
struct Sprites;
struct BackgroundInfo;

class Disk {

public:
	Disk() { }
	virtual ~Disk() { }

	virtual Common::String selectArchive(const Common::String &name) = 0;
	virtual void setLanguage(uint16 language) = 0;

	virtual Script* loadLocation(const char *name) = 0;
	virtual Script* loadScript(const char* name) = 0;
	virtual GfxObj* loadTalk(const char *name) = 0;
	virtual GfxObj* loadObjects(const char *name) = 0;
	virtual Frames* loadPointer(const char *name) = 0;
	virtual GfxObj* loadHead(const char* name) = 0;
	virtual Font* loadFont(const char* name) = 0;
	virtual GfxObj* loadStatic(const char* name) = 0;
	virtual Frames* loadFrames(const char* name) = 0;
	virtual void loadSlide(BackgroundInfo& info, const char *filename) = 0;
	virtual void loadScenery(BackgroundInfo& info, const char* background, const char* mask, const char* path) = 0;
	virtual Table* loadTable(const char* name) = 0;
	virtual Common::SeekableReadStream* loadMusic(const char* name) = 0;
	virtual Common::ReadStream* loadSound(const char* name) = 0;
};




#define MAX_ARCHIVE_ENTRIES			384

class Archive : public Common::SeekableReadStream {

protected:
	bool			_file;
	uint32			_fileOffset;
	uint32			_fileCursor;
	uint32			_fileEndOffset;
	Common::String	_archiveName;
	char			_archiveDir[MAX_ARCHIVE_ENTRIES][32];
	uint32			_archiveLenghts[MAX_ARCHIVE_ENTRIES];
	uint32			_archiveOffsets[MAX_ARCHIVE_ENTRIES];
	Common::File	_archive;
	uint32			_numFiles;

protected:
	void resetArchivedFile();

public:
	Archive();

	void open(const char* file);
	void close();
	Common::String name() const;
	bool openArchivedFile(const char *name);
	void closeArchivedFile();
	uint32 size() const;
	uint32 pos() const;
	bool eos() const;
	void seek(int32 offs, int whence = SEEK_SET);
	uint32 read(void *dataPtr, uint32 dataSize);
};

class Disk_ns : public Disk {

protected:
	Archive			_resArchive;
	Archive			_locArchive;
	char			_languageDir[3];
	Parallaction	*_vm;

protected:
	void errorFileNotFound(const char *s);

public:
	Disk_ns(Parallaction *vm);
	virtual ~Disk_ns();

	Common::String selectArchive(const Common::String &name);
	void setLanguage(uint16 language);
};

class DosDisk_ns : public Disk_ns {

private:
	void unpackBackground(Common::ReadStream *stream, byte *screen, byte *mask, byte *path);
	Cnv* loadExternalCnv(const char *filename);
	Cnv* loadCnv(const char *filename);
	Frames* loadExternalStaticCnv(const char *filename);
	void loadBackground(BackgroundInfo& info, const char *filename);
	void loadMaskAndPath(BackgroundInfo& info, const char *name);
	void parseDepths(BackgroundInfo &info, Common::SeekableReadStream &stream);
	void parseBackground(BackgroundInfo& info, Common::SeekableReadStream &stream);
	Font *createFont(const char *name, Cnv* cnv);

protected:
	Gfx	 *_gfx;

public:
	DosDisk_ns(Parallaction *vm);
	virtual ~DosDisk_ns();

	Script* loadLocation(const char *name);
	Script* loadScript(const char* name);
	GfxObj* loadTalk(const char *name);
	GfxObj* loadObjects(const char *name);
	Frames* loadPointer(const char *name);
	GfxObj* loadHead(const char* name);
	Font* loadFont(const char* name);
	GfxObj* loadStatic(const char* name);
	Frames* loadFrames(const char* name);
	void loadSlide(BackgroundInfo& info, const char *filename);
	void loadScenery(BackgroundInfo& info, const char* background, const char* mask, const char* path);
	Table* loadTable(const char* name);
	Common::SeekableReadStream* loadMusic(const char* name);
	Common::ReadStream* loadSound(const char* name);
};

class AmigaDisk_ns : public Disk_ns {

protected:
	Cnv* makeCnv(Common::SeekableReadStream &stream);
	Frames* makeStaticCnv(Common::SeekableReadStream &stream);
	void patchFrame(byte *dst, byte *dlta, uint16 bytesPerPlane, uint16 height);
	void unpackFrame(byte *dst, byte *src, uint16 planeSize);
	void unpackBitmap(byte *dst, byte *src, uint16 numFrames, uint16 bytesPerPlane, uint16 height);
	Common::SeekableReadStream *openArchivedFile(const char* name, bool errorOnFileNotFound = false);
	Font *createFont(const char *name, Common::SeekableReadStream &stream);
	void loadMask(BackgroundInfo& info, const char *name);
	void loadPath(BackgroundInfo& info, const char *name);
	void loadBackground(BackgroundInfo& info, const char *name);

public:
	AmigaDisk_ns(Parallaction *vm);
	virtual ~AmigaDisk_ns();

	Script* loadLocation(const char *name);
	Script* loadScript(const char* name);
	GfxObj* loadTalk(const char *name);
	GfxObj* loadObjects(const char *name);
	Frames* loadPointer(const char *name);
	GfxObj* loadHead(const char* name);
	Font* loadFont(const char* name);
	GfxObj* loadStatic(const char* name);
	Frames* loadFrames(const char* name);
	void loadSlide(BackgroundInfo& info, const char *filename);
	void loadScenery(BackgroundInfo& info, const char* background, const char* mask, const char* path);
	Table* loadTable(const char* name);
	Common::SeekableReadStream* loadMusic(const char* name);
	Common::ReadStream* loadSound(const char* name);
};


//	for the moment DosDisk_br subclasses Disk. When Amiga support will
//  be taken into consideration, it might be useful to add another level
//  like we did for Nippon Safes.
class DosDisk_br : public Disk {

protected:
	uint16			_language;

	Parallaction	*_vm;

	FilesystemNode	_baseDir;
	FilesystemNode	_partDir;

	FilesystemNode	_aniDir;
	FilesystemNode	_bkgDir;
	FilesystemNode	_mscDir;
	FilesystemNode	_mskDir;
	FilesystemNode	_pthDir;
	FilesystemNode	_rasDir;
	FilesystemNode	_scrDir;
	FilesystemNode	_sfxDir;
	FilesystemNode	_talDir;

protected:
	void errorFileNotFound(const FilesystemNode &dir, const Common::String &filename);
	Font *createFont(const char *name, Common::ReadStream &stream);
	Sprites*	createSprites(Common::ReadStream &stream);
	void loadBitmap(Common::SeekableReadStream &stream, Graphics::Surface &surf, byte *palette);
	GfxObj* createInventoryObjects(Common::SeekableReadStream &stream);

public:
	DosDisk_br(Parallaction *vm);
	virtual ~DosDisk_br();

	Common::String selectArchive(const Common::String &name);
	void setLanguage(uint16 language);
	Script* loadLocation(const char *name);
	Script* loadScript(const char* name);
	GfxObj* loadTalk(const char *name);
	GfxObj* loadObjects(const char *name);
	Frames* loadPointer(const char *name);
	GfxObj* loadHead(const char* name);
	Font* loadFont(const char* name);
	GfxObj* loadStatic(const char* name);
	Frames* loadFrames(const char* name);
	void loadSlide(BackgroundInfo& info, const char *filename);
	void loadScenery(BackgroundInfo& info, const char* name, const char* mask, const char* path);
	Table* loadTable(const char* name);
	Common::SeekableReadStream* loadMusic(const char* name);
	Common::ReadStream* loadSound(const char* name);
};

class DosDemo_br : public DosDisk_br {

public:
	DosDemo_br(Parallaction *vm);
	virtual ~DosDemo_br();

	Common::String selectArchive(const Common::String& name);

};

class AmigaDisk_br : public DosDisk_br {

protected:
	BackgroundInfo	_backgroundTemp;

	Sprites*	createSprites(Common::ReadStream &stream);
	Font *createFont(const char *name, Common::SeekableReadStream &stream);
	void loadMask(BackgroundInfo& info, Common::SeekableReadStream &stream);
	void loadBackground(BackgroundInfo& info, Common::SeekableReadStream &stream);

	FilesystemNode	_baseBkgDir;
	FilesystemNode	_fntDir;
	FilesystemNode	_commonAniDir;
	FilesystemNode	_commonBkgDir;
	FilesystemNode	_commonMscDir;
	FilesystemNode	_commonMskDir;
	FilesystemNode	_commonPthDir;
	FilesystemNode	_commonTalDir;

public:
	AmigaDisk_br(Parallaction *vm);
	virtual ~AmigaDisk_br();

	GfxObj* loadTalk(const char *name);
	Font* loadFont(const char* name);
	GfxObj* loadStatic(const char* name);
	Frames* loadFrames(const char* name);
	void loadSlide(BackgroundInfo& info, const char *filename);
	void loadScenery(BackgroundInfo& info, const char* name, const char* mask, const char* path);
	GfxObj* loadObjects(const char *name);
	Common::SeekableReadStream* loadMusic(const char* name);
	Common::ReadStream* loadSound(const char* name);
	Common::String selectArchive(const Common::String& name);

};

} // namespace Parallaction



#endif
