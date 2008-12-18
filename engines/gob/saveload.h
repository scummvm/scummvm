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

#ifndef GOB_SAVELOAD_H
#define GOB_SAVELOAD_H

#include "common/array.h"
#include "common/stream.h"

#include "gob/video.h"
#include "gob/variables.h"

namespace Gob {

class TempSprite {
public:
	TempSprite();
	~TempSprite();

	bool getProperties(int16 dataVar, int32 size, int32 offset,
			int &index, bool &palette) const;

	int32 getSize() const;

	bool saveSprite(const SurfaceDesc &surfDesc);
	bool savePalette(const Video::Color *palette);
	bool loadSprite(SurfaceDesc &surfDesc);
	bool loadPalette(Video::Color *palette);

	bool toBuffer(byte *buffer, int32 size, bool palette) const;
	bool fromBuffer(const byte *buffer, int32 size, bool palette);
	
private:
	byte *_sprite;
	int16 _width;
	int16 _height;
	int32 _size;
	Video::Color _palette[256];

	int getSpriteIndex(int32 size) const;
	bool getSpritePalette(int32 size) const;
};

class PlainSave {
public:
	PlainSave(Endianness endianness);
	~PlainSave();

	bool save(int16 dataVar, int32 size, int32 offset, const char *name,
			const Variables *variables);
	bool load(int16 dataVar, int32 size, int32 offset, const char *name,
			Variables *variables);

	bool save(int16 dataVar, int32 size, int32 offset, const char *name,
			const byte *variables, const byte *variableSizes) const;
	bool load(int16 dataVar, int32 size, int32 offset, const char *name,
			byte *variables, byte *variableSizes) const;

private:
	Endianness _endianness;
};

class StagedSave {
public:
	StagedSave(Endianness endianness);
	~StagedSave();

	void addStage(int32 size, bool endianed = true);

	bool save(int16 dataVar, int32 size, int32 offset, const char *name,
			const Variables *variables);
	bool load(int16 dataVar, int32 size, int32 offset, const char *name,
			Variables *variables);

	bool save(int16 dataVar, int32 size, int32 offset, const char *name,
			const byte *variables, const byte *variableSizes);
	bool load(int16 dataVar, int32 size, int32 offset, const char *name,
			byte *variables, byte *variableSizes);

private:
	struct Stage {
		byte *bufVar;
		byte *bufVarSizes;
		int32 size;
		int32 offset;
		bool endianed;

		Stage(int32 s = 0, int32 off = 0, bool end = true) :
			bufVar(0), bufVarSizes(0), size(s), offset(off), endianed(end) {}
	};

	enum Mode {
		kModeNone,
		kModeSave,
		kModeLoad
	};

	Endianness _endianness;

	Common::Array<Stage> _stages;
	enum Mode _mode;
	char *_name;

	bool _loaded;

	int findStage(int16 dataVar, int32 size, int32 offset) const;
	bool allSaved() const;

	uint32 getSize() const;

	void clear();
	void assertMode(Mode mode, const char *name);

	bool write() const;
	bool read();
};

class PagedBuffer {
public:
	PagedBuffer(uint32 pageSize = 1024);
	~PagedBuffer();

	bool empty() const;
	uint32 getSize() const;

	void clear();

	bool write(const byte *buffer, uint32 size, uint32 offset);
	bool read(byte *buffer, uint32 size, uint32 offset) const;

	uint32 writeToStream(Common::WriteStream &out) const;
	uint32 readFromStream(Common::ReadStream &in);

private:
	uint32 _size;
	uint32 _pageSize;
	Common::Array<byte *> _pages;

	void grow(uint32 size, uint32 offset);
};

class SaveLoad {
public:
	enum SaveMode {
		kSaveModeNone,
		kSaveModeIgnore,
		kSaveModeExists,
		kSaveModeSave
	};

	SaveLoad(GobEngine *vm, const char *targetName);
	virtual ~SaveLoad();

	virtual SaveMode getSaveMode(const char *fileName);

	int32 getSize(const char *fileName);
	bool load(const char *fileName, int16 dataVar, int32 size, int32 offset);
	bool save(const char *fileName, int16 dataVar, int32 size, int32 offset);

	char *setCurrentSlot(char *destName, int slot);
	void buildIndex(byte *buffer, char *name, int n, int32 size, int32 offset = 0);

	static const char *stripPath(const char *fileName);

	static bool fromEndian(byte *buf, const byte *sizes, uint32 count, Endianness endianness);
	static bool toEndian(byte *buf, const byte *sizes, uint32 count, Endianness endianness);
	static uint32 read(Common::ReadStream &in,
			byte *buf, byte *sizes, uint32 count);
	static uint32 write(Common::WriteStream &out,
			const byte *buf, const byte *sizes, uint32 count);

	static bool loadDataEndian(Common::ReadStream &in,
			int16 dataVar, uint32 size,
			byte *variables, byte *variableSizes, Endianness endianness);
	static bool saveDataEndian(Common::WriteStream &out,
			int16 dataVar, uint32 size,
			const byte *variables, const byte *variableSizes, Endianness endianness);

protected:
	GobEngine *_vm;

	char *_targetName;

	virtual int getSaveType(const char *fileName);

	virtual int32 getSizeVersioned(int type);
	virtual bool loadVersioned(int type, int16 dataVar, int32 size, int32 offset);
	virtual bool saveVersioned(int type, int16 dataVar, int32 size, int32 offset);
};

class SaveLoad_v2 : public SaveLoad {
public:
	enum SaveType {
		kSaveGame,
		kSaveTempSprite,
		kSaveNotes
	};

	SaveLoad_v2(GobEngine *vm, const char *targetName);
	virtual ~SaveLoad_v2();

	virtual SaveMode getSaveMode(const char *fileName);

protected:
	struct SaveFile {
		const char *sourceName;
		char *destName;
		SaveMode mode;
		SaveType type;
	};

	static SaveFile _saveFiles[];

	int32 _varSize;

	TempSprite _tmpSprite;
	PlainSave *_notes;
	StagedSave *_save;

	byte _indexBuffer[600];
	bool _hasIndex;

	virtual int getSaveType(const char *fileName);

	virtual int32 getSizeVersioned(int type);
	virtual bool loadVersioned(int type, int16 dataVar, int32 size, int32 offset);
	virtual bool saveVersioned(int type, int16 dataVar, int32 size, int32 offset);

	int getSlot(int32 offset) const;
	int getSlotRemainder(int32 offset) const;

	int32 getSizeGame(SaveFile &saveFile);
	int32 getSizeTempSprite(SaveFile &saveFile);
	int32 getSizeNotes(SaveFile &saveFile);

	bool loadGame(SaveFile &saveFile, int16 dataVar, int32 size, int32 offset);
	bool loadTempSprite(SaveFile &saveFile, int16 dataVar, int32 size, int32 offset);
	bool loadNotes(SaveFile &saveFile, int16 dataVar, int32 size, int32 offset);

	bool saveGame(SaveFile &saveFile, int16 dataVar, int32 size, int32 offset);
	bool saveTempSprite(SaveFile &saveFile, int16 dataVar, int32 size, int32 offset);
	bool saveNotes(SaveFile &saveFile, int16 dataVar, int32 size, int32 offset);

	void assertInited();
};

enum SaveType {
	kSaveNone = -1,
	kSaveGame,
	kSaveTempSprite,
	kSaveNotes,
	kSaveScreenshot,
	kSaveIgnore
};

class SaveLoad_v3 : public SaveLoad {
public:
	enum SaveType {
		kSaveNone,
		kSaveGame,
		kSaveTempSprite,
		kSaveNotes,
		kSaveScreenshot
	};

	SaveLoad_v3(GobEngine *vm, const char *targetName,
			uint32 screenshotSize = 19968,
			int32 indexOffset = 40, int32 screenshotOffset = 80);
	virtual ~SaveLoad_v3();

	virtual SaveMode getSaveMode(const char *fileName);

protected:
	struct SaveFile {
		const char *sourceName;
		char *destName;
		SaveMode mode;
		SaveType type;
		int slot;
	};

	bool _useScreenshots;
	bool _firstSizeGame;

	uint32 _screenshotSize;
	int32 _indexOffset;
	int32 _screenshotOffset;

	static SaveFile _saveFiles[];

	int32 _varSize;

	TempSprite _screenshot;
	TempSprite _tmpSprite;
	PlainSave *_notes;
	StagedSave *_save;

	byte _propBuffer[1000];
	byte _indexBuffer[1200];
	bool _hasIndex;

	virtual int getSaveType(const char *fileName);

	virtual int32 getSizeVersioned(int type);
	virtual bool loadVersioned(int type, int16 dataVar, int32 size, int32 offset);
	virtual bool saveVersioned(int type, int16 dataVar, int32 size, int32 offset);

	int getSlot(int32 offset) const;
	int getSlotRemainder(int32 offset) const;

	int32 getSizeGame(SaveFile &saveFile);
	int32 getSizeTempSprite(SaveFile &saveFile);
	int32 getSizeNotes(SaveFile &saveFile);
	int32 getSizeScreenshot(SaveFile &saveFile);

	bool loadGame(SaveFile &saveFile, int16 dataVar, int32 size, int32 offset);
	bool loadTempSprite(SaveFile &saveFile, int16 dataVar, int32 size, int32 offset);
	bool loadNotes(SaveFile &saveFile, int16 dataVar, int32 size, int32 offset);
	bool loadScreenshot(SaveFile &saveFile, int16 dataVar, int32 size, int32 offset);

	bool saveGame(SaveFile &saveFile, int16 dataVar, int32 size, int32 offset);
	bool saveTempSprite(SaveFile &saveFile, int16 dataVar, int32 size, int32 offset);
	bool saveNotes(SaveFile &saveFile, int16 dataVar, int32 size, int32 offset);
	bool saveScreenshot(SaveFile &saveFile, int16 dataVar, int32 size, int32 offset);

	void assertInited();

	void buildScreenshotIndex(byte *buffer, char *name, int n);
};

class SaveLoad_v4 : public SaveLoad {
public:
	enum SaveType {
		kSaveNone,
		kSaveScreenProps,
		kSaveGame,
		kSaveGameScreenProps
	};

	bool _firstSizeGame;

	SaveLoad_v4(GobEngine *vm, const char *targetName);
	virtual ~SaveLoad_v4();

	virtual SaveMode getSaveMode(const char *fileName);

protected:
	struct SaveFile {
		const char *sourceName;
		char *destName;
		SaveMode mode;
		SaveType type;
	};

	static SaveFile _saveFiles[];

	int32 _varSize;

	StagedSave *_save;

	byte _propBuffer[1000];
	byte _indexBuffer[1200];
	bool _hasIndex;

	byte *_screenProps;

	virtual int getSaveType(const char *fileName);

	virtual int32 getSizeVersioned(int type);
	virtual bool loadVersioned(int type, int16 dataVar, int32 size, int32 offset);
	virtual bool saveVersioned(int type, int16 dataVar, int32 size, int32 offset);

	int getSlot(int32 offset) const;
	int getSlotRemainder(int32 offset) const;

	int32 getSizeScreenProps(SaveFile &saveFile);
	int32 getSizeGame(SaveFile &saveFile);
	int32 getSizeGameScreenProps(SaveFile &saveFile);

	bool loadScreenProps(SaveFile &saveFile, int16 dataVar, int32 size, int32 offset);
	bool loadGame(SaveFile &saveFile, int16 dataVar, int32 size, int32 offset);
	bool loadGameScreenProps(SaveFile &saveFile, int16 dataVar, int32 size, int32 offset);

	bool saveScreenProps(SaveFile &saveFile, int16 dataVar, int32 size, int32 offset);
	bool saveGame(SaveFile &saveFile, int16 dataVar, int32 size, int32 offset);
	bool saveGameScreenProps(SaveFile &saveFile, int16 dataVar, int32 size, int32 offset);

	void assertInited();
};

class SaveLoad_v6 : public SaveLoad {
public:
	enum SaveType {
		kSaveNone,
		kSaveGame,
		kSaveNoCD
	};

	SaveLoad_v6(GobEngine *vm, const char *targetName);
	virtual ~SaveLoad_v6();

	virtual SaveMode getSaveMode(const char *fileName);

protected:
	struct SaveFile {
		const char *sourceName;
		char *destName;
		SaveMode mode;
		SaveType type;
	};

	static SaveFile _saveFiles[];

	int32 _varSize;

	StagedSave *_save;

	byte _indexBuffer[2900];
	bool _hasIndex;

	virtual int getSaveType(const char *fileName);

	virtual int32 getSizeVersioned(int type);
	virtual bool loadVersioned(int type, int16 dataVar, int32 size, int32 offset);
	virtual bool saveVersioned(int type, int16 dataVar, int32 size, int32 offset);

	int getSlot(int32 offset) const;
	int getSlotRemainder(int32 offset) const;

	int32 getSizeGame(SaveFile &saveFile);

	bool loadGame(SaveFile &saveFile, int16 dataVar, int32 size, int32 offset);

	bool saveGame(SaveFile &saveFile, int16 dataVar, int32 size, int32 offset);

	void assertInited();

	void refreshIndex();
};

} // End of namespace Gob

#endif // GOB_SAVELOAD_H
