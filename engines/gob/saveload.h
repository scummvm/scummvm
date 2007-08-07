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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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

#include "common/stream.h"

#include "gob/video.h"

namespace Gob {

enum SaveType {
	kSaveNone = -1,
	kSaveGame,
	kSaveTempSprite,
	kSaveNotes,
	kSaveScreenshot,
	kSaveIgnore
};

class SaveLoad {
public:
	int32 getSize(SaveType type);
	bool load(SaveType type, int16 dataVar, int32 size, int32 offset);
	bool save(SaveType type, int16 dataVar, int32 size, int32 offset);

	virtual SaveType getSaveType(const char *fileName) = 0;

	SaveLoad(GobEngine *vm, const char *targetName);
	virtual ~SaveLoad();

protected:
	int _curSlot;
	char **_saveFiles;

	int _stagesCount;
	byte **_buffer;

	// While using the notepad or changing the font, the original executable
	// temporarily dumps Draw::_backSurface to a file. Since that's not really
	// a nice thing to do, we work around it.
	SurfaceDesc *_tempSprite;
	Video::Color _tempPal[256];
	int32 _tempSpriteSize;

	GobEngine *_vm;

	int getSpriteIndex(int32 size) {
		if (size < -1000)
			size += 1000;

		return -size - 1;
	}
	bool getSpritePalette(int32 size) {
		return size < -1000;
	}

	const char *setCurSlot(int slot);
	bool fromEndian(byte *buf, const byte *sizes, uint32 count);
	bool toEndian(byte *buf, const byte *sizes, uint32 count);
	uint32 read(Common::ReadStream &in, byte *buf,
			byte *sizes, uint32 count);
	uint32 write(Common::WriteStream &out, byte *buf,
			byte *sizes, uint32 count);

	bool loadDataEndian(Common::ReadStream &in, int16 dataVar, uint32 size);
	bool saveDataEndian(Common::WriteStream &out, int16 dataVar, uint32 size);

	bool loadTempSprite(uint32 index, bool palette);
	bool saveTempSprite(uint32 index, bool palette);
	bool loadSprite(Common::ReadStream &in, int32 size);
	bool saveSprite(Common::WriteStream &out, int32 size);

	int32 getSizeTempSprite();
	bool loadTempSprite(int16 dataVar, int32 size, int32 offset);
	bool saveTempSprite(int16 dataVar, int32 size, int32 offset);

	virtual uint32 getSaveGameSize() = 0;

	virtual int32 getSizeGame() = 0;
	virtual int32 getSizeNotes() = 0;
	virtual int32 getSizeScreenshot() = 0;
	virtual bool loadGame(int16 dataVar, int32 size, int32 offset) = 0;
	virtual bool loadNotes(int16 dataVar, int32 size, int32 offset) = 0;
	virtual bool loadScreenshot(int16 dataVar, int32 size, int32 offset) = 0;
	virtual bool saveGame(int16 dataVar, int32 size, int32 offset) = 0;
	virtual bool saveNotes(int16 dataVar, int32 size, int32 offset) = 0;
	virtual bool saveScreenshot(int16 dataVar, int32 size, int32 offset) = 0;
};

class SaveLoad_v2 : public SaveLoad {
public:
	virtual SaveType getSaveType(const char *fileName);

	SaveLoad_v2(GobEngine *vm, const char *targetName);
	virtual ~SaveLoad_v2();

protected:
	virtual uint32 getSaveGameSize();

	virtual int32 getSizeGame();
	virtual int32 getSizeNotes();
	virtual int32 getSizeScreenshot();
	virtual bool loadGame(int16 dataVar, int32 size, int32 offset);
	virtual bool loadNotes(int16 dataVar, int32 size, int32 offset);
	virtual bool loadScreenshot(int16 dataVar, int32 size, int32 offset);
	virtual bool saveGame(int16 dataVar, int32 size, int32 offset);
	virtual bool saveNotes(int16 dataVar, int32 size, int32 offset);
	virtual bool saveScreenshot(int16 dataVar, int32 size, int32 offset);
};

class SaveLoad_v3 : public SaveLoad_v2 {
public:
	virtual SaveType getSaveType(const char *fileName);

	SaveLoad_v3(GobEngine *vm, const char *targetName, uint32 screenshotSize = 19968,
			int32 indexOffset = 40, int32 screenshotOffset = 80);
	virtual ~SaveLoad_v3() {}

protected:
	bool _useScreenshots;
	bool _firstSizeGame;
	int8 _saveSlot;

	uint32 _screenshotSize;
	int32 _indexOffset;
	int32 _screenshotOffset;

	virtual uint32 getSaveGameSize();

	virtual int32 getSizeGame();
	virtual int32 getSizeScreenshot();
	virtual bool loadGame(int16 dataVar, int32 size, int32 offset);
	virtual bool loadScreenshot(int16 dataVar, int32 size, int32 offset);
	virtual bool saveGame(int16 dataVar, int32 size, int32 offset);
	virtual bool saveNotes(int16 dataVar, int32 size, int32 offset);
	virtual bool saveScreenshot(int16 dataVar, int32 size, int32 offset);

	bool saveGame(int32 screenshotSize);
};

} // End of namespace Gob

#endif // GOB_SAVELOAD_H
