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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PARALLACTION_DISK_H
#define PARALLACTION_DISK_H

#define PATH_LEN 200

#include "common/archive.h"
#include "common/str.h"

namespace Common {
class FSDirectory;
class ReadStream;
class SeekableReadStream;
}

namespace Graphics {
struct Surface;
}

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
class GfxObj;
struct MaskBuffer;
struct PathBuffer;

class Disk {

public:
	Disk() { }
	virtual ~Disk() { }

	virtual void init() { }

	virtual Common::String selectArchive(const Common::String &name) = 0;
	virtual void setLanguage(uint16 language) = 0;

	virtual Script* loadLocation(const char *name) = 0;
	virtual Script* loadScript(const char* name) = 0;
	virtual GfxObj* loadTalk(const char *name) = 0;
	virtual GfxObj* loadObjects(const char *name, uint8 part = 0) = 0;
	virtual Frames* loadPointer(const char *name) = 0;
	virtual GfxObj* loadHead(const char* name) = 0;
	virtual Font* loadFont(const char* name) = 0;
	virtual GfxObj* loadStatic(const char* name) = 0;
	virtual Frames* loadFrames(const char* name) = 0;
	virtual void loadSlide(BackgroundInfo& info, const char *filename) = 0;
	virtual void loadScenery(BackgroundInfo& info, const char* background, const char* mask, const char* path) = 0;
	virtual Table* loadTable(const char* name) = 0;
	virtual Common::SeekableReadStream* loadMusic(const char* name) = 0;
	virtual Common::SeekableReadStream* loadSound(const char* name) = 0;
	virtual MaskBuffer *loadMask(const char *name, uint32 w, uint32 h) { return 0; }
	virtual PathBuffer *loadPath(const char *name, uint32 w, uint32 h) { return 0; }
};

class Disk_ns : public Disk {

protected:
	Parallaction		*_vm;

	Common::SearchSet	_sset;

	Common::String		_resArchiveName;
	Common::String		_language;
	Common::SeekableReadStream *openFile(const char *filename);
	virtual Common::SeekableReadStream *tryOpenFile(const char *filename) { return 0; }
	void errorFileNotFound(const char *filename);

	void addArchive(const Common::String& name, int priority);

	virtual void decodeCnv(byte *data, uint16 numFrames, uint16 width, uint16 height, Common::SeekableReadStream *stream) = 0;
	Cnv *makeCnv(Common::SeekableReadStream *stream);

public:
	Disk_ns(Parallaction *vm);
	~Disk_ns() override;

	Common::String selectArchive(const Common::String &name) override;
	void setLanguage(uint16 language) override;

	Script* loadLocation(const char *name) override;
	Script* loadScript(const char* name) override;
};

class DosDisk_ns : public Disk_ns {

private:
	void unpackBackground(Common::ReadStream *stream, byte *screen, byte *mask, byte *path);
	Cnv* loadCnv(const char *filename);
	void loadBackground(BackgroundInfo& info, const char *filename);
	void createMaskAndPathBuffers(BackgroundInfo &info);
	void parseDepths(BackgroundInfo &info, Common::SeekableReadStream &stream);
	Font *createFont(const char *name, Cnv* cnv);

protected:
	Gfx	 *_gfx;
	Common::SeekableReadStream *tryOpenFile(const char* name) override;
	void decodeCnv(byte *data, uint16 numFrames, uint16 width, uint16 height, Common::SeekableReadStream *stream) override;

public:
	DosDisk_ns(Parallaction *vm);
	~DosDisk_ns() override;

	void init() override;

	GfxObj* loadTalk(const char *name) override;
	GfxObj* loadObjects(const char *name, uint8 part = 0) override;
	Frames* loadPointer(const char *name) override;
	GfxObj* loadHead(const char* name) override;
	Font* loadFont(const char* name) override;
	GfxObj* loadStatic(const char* name) override;
	Frames* loadFrames(const char* name) override;
	void loadSlide(BackgroundInfo& info, const char *filename) override;
	void loadScenery(BackgroundInfo& info, const char* background, const char* mask, const char* path) override;
	Table* loadTable(const char* name) override;
	Common::SeekableReadStream* loadMusic(const char* name) override;
	Common::SeekableReadStream* loadSound(const char* name) override;
};

class AmigaDisk_ns : public Disk_ns {

protected:
	void patchFrame(byte *dst, byte *dlta, uint16 bytesPerPlane, uint16 height);
	void unpackFrame(byte *dst, byte *src, uint16 planeSize);
	void unpackBitmap(byte *dst, byte *src, uint16 numFrames, uint16 bytesPerPlane, uint16 height);
	Common::SeekableReadStream *tryOpenFile(const char* name) override;
	Font *createFont(const char *name, Common::SeekableReadStream &stream);
	void loadMask_internal(BackgroundInfo& info, const char *name);
	void loadPath_internal(BackgroundInfo& info, const char *name);
	void loadBackground(BackgroundInfo& info, const char *name);
	void buildMask(byte* buf);

	void decodeCnv(byte *data, uint16 numFrames, uint16 width, uint16 height, Common::SeekableReadStream *stream) override;

public:
	AmigaDisk_ns(Parallaction *vm);
	~AmigaDisk_ns() override;

	void init() override;

	GfxObj* loadTalk(const char *name) override;
	GfxObj* loadObjects(const char *name, uint8 part = 0) override;
	Frames* loadPointer(const char *name) override;
	GfxObj* loadHead(const char* name) override;
	Font* loadFont(const char* name) override;
	GfxObj* loadStatic(const char* name) override;
	Frames* loadFrames(const char* name) override;
	void loadSlide(BackgroundInfo& info, const char *filename) override;
	void loadScenery(BackgroundInfo& info, const char* background, const char* mask, const char* path) override;
	Table* loadTable(const char* name) override;
	Common::SeekableReadStream* loadMusic(const char* name) override;
	Common::SeekableReadStream* loadSound(const char* name) override;
};


class Disk_br : public Disk {

	Common::SeekableReadStream *openFile_internal(bool errorOnNotFound, const Common::String &name, const Common::String &ext);

protected:
	Parallaction	*_vm;
	Common::SearchSet	_sset;
	Common::FSDirectory *_baseDir;

	uint16			_language;
	Common::String		_currentPart;

	Common::SeekableReadStream *tryOpenFile(const Common::String &name, const Common::String &ext = Common::String());
	Common::SeekableReadStream *openFile(const Common::String &name, const Common::String &ext = Common::String());
	void errorFileNotFound(const Common::String &filename);

public:
	Disk_br(Parallaction *vm);
	~Disk_br() override;
};

//	for the moment DosDisk_br subclasses Disk. When Amiga support will
//  be taken into consideration, it might be useful to add another level
//  like we did for Nippon Safes.
class DosDisk_br : public Disk_br {

protected:
	Font *createFont(const char *name, Common::ReadStream &stream);
	Sprites*	createSprites(Common::ReadStream *stream);
	void loadBitmap(Common::SeekableReadStream &stream, Graphics::Surface &surf, byte *palette);
	GfxObj* createInventoryObjects(Common::SeekableReadStream &stream);

public:
	DosDisk_br(Parallaction *vm);

	void init() override;

	Common::String selectArchive(const Common::String &name) override;
	void setLanguage(uint16 language) override;
	Script* loadLocation(const char *name) override;
	Script* loadScript(const char* name) override;
	GfxObj* loadTalk(const char *name) override;
	GfxObj* loadObjects(const char *name, uint8 part = 0) override;
	Frames* loadPointer(const char *name) override;
	GfxObj* loadHead(const char* name) override;
	Font* loadFont(const char* name) override;
	GfxObj* loadStatic(const char* name) override;
	Frames* loadFrames(const char* name) override;
	void loadSlide(BackgroundInfo& info, const char *filename) override;
	void loadScenery(BackgroundInfo& info, const char* name, const char* mask, const char* path) override;
	Table* loadTable(const char* name) override;
	Common::SeekableReadStream* loadMusic(const char* name) override;
	Common::SeekableReadStream* loadSound(const char* name) override;
	MaskBuffer *loadMask(const char *name, uint32 w, uint32 h) override;
	PathBuffer *loadPath(const char *name, uint32 w, uint32 h) override;
};

class DosDemoDisk_br : public DosDisk_br {

public:
	DosDemoDisk_br(Parallaction *vm);

	void init() override;

	Common::String selectArchive(const Common::String& name) override;
};

class AmigaDisk_br : public DosDisk_br {

protected:
	Sprites*	createSprites(Common::ReadStream *stream);
	Font *createFont(const char *name, Common::SeekableReadStream &stream);
	void loadBackground(BackgroundInfo& info, const char *filename);
	void adjustForPalette(Graphics::Surface &surf, int transparentColor = -1);

public:
	AmigaDisk_br(Parallaction *vm);

	void init() override;

	Common::String selectArchive(const Common::String& name) override;
	GfxObj* loadTalk(const char *name) override;
	Font* loadFont(const char* name) override;
	GfxObj* loadStatic(const char* name) override;
	Frames* loadFrames(const char* name) override;
	void loadSlide(BackgroundInfo& info, const char *filename) override;
	void loadScenery(BackgroundInfo& info, const char* name, const char* mask, const char* path) override;
	GfxObj* loadObjects(const char *name, uint8 part = 0) override;
	Common::SeekableReadStream* loadMusic(const char* name) override;
	Common::SeekableReadStream* loadSound(const char* name) override;
	MaskBuffer *loadMask(const char *name, uint32 w, uint32 h) override;
};

} // namespace Parallaction



#endif
