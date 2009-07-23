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

#ifndef GOB_SAVE_SAVELOAD_H
#define GOB_SAVE_SAVELOAD_H

#include "gob/save/savefile.h"
#include "gob/save/savehandler.h"

namespace Gob {

class GobEngine;

/** A system for saving and loading. */
class SaveLoad {
public:
	/** How to handle the specific save. */
	enum SaveMode {
		kSaveModeNone,   //!< Don't handle it
		kSaveModeIgnore, //!< Ignore it
		kSaveModeExists, //!< Just claim it exists
		kSaveModeSave    //!< A normal save
	};

	/** The constructor.
	 *
	 *  @param targetName The game's target name. Used as a base for the save names.
	 */
	SaveLoad(GobEngine *vm);
	virtual ~SaveLoad();

	/** "foo\bar\quux.bla" => "quux.bla". */
	static const char *stripPath(const char *fileName);

	/** Returns how to handle that file. */
	virtual SaveMode getSaveMode(const char *fileName) const;

	/** Returns the file's (virtual) size. */
	int32 getSize(const char *fileName);
	/** Loads size bytes from offset into the variables starting with dataVar. */
	bool load(const char *fileName, int16 dataVar, int32 size, int32 offset);
	/** Saves size bytes from the variables starting with data dataVar at offset. */
	bool save(const char *fileName, int16 dataVar, int32 size, int32 offset);

protected:
	GobEngine *_vm;

	virtual SaveHandler *getHandler(const char *fileName) const;
	virtual const char *getDescription(const char *fileName) const;
};

/** Save/Load class for Gobliins 2, Ween: The Prophecy and Bargon Attack. */
class SaveLoad_v2 : public SaveLoad {
public:
	static const uint32 kSlotCount = 15;
	static const uint32 kSlotNameLength = 40;

	SaveLoad_v2(GobEngine *vm, const char *targetName);
	virtual ~SaveLoad_v2();

	SaveMode getSaveMode(const char *fileName) const;

protected:
	struct SaveFile {
		const char *sourceName;
		SaveMode mode;
		SaveHandler *handler;
		const char *description;
	};

	/** Handles the save slots. */
	class GameHandler : public SaveHandler {
	public:
		GameHandler(GobEngine *vm, const char *target);
		~GameHandler();

		int32 getSize();
		bool load(int16 dataVar, int32 size, int32 offset);
		bool save(int16 dataVar, int32 size, int32 offset);

	private:
		/** Slot file construction. */
		class File : public SlotFileIndexed {
		public:
			File(GobEngine *vm, const char *base);
			~File();

			int getSlot(int32 offset) const;
			int getSlotRemainder(int32 offset) const;
		};

		/** The index. kSlotCount * kSlotNameLength bytes. */
		byte _index[600];
		bool _hasIndex;

		File *_slotFile;

		void buildIndex(byte *buffer) const;
	};

	static SaveFile _saveFiles[];

	GameHandler *_gameHandler;
	NotesHandler *_notesHandler;
	TempSpriteHandler *_tempSpriteHandler;

	SaveHandler *getHandler(const char *fileName) const;
	const char *getDescription(const char *fileName) const;

	const SaveFile *getSaveFile(const char *fileName) const;
	SaveFile *getSaveFile(const char *fileName);
};

/** Save/Load class for Goblins 3 and Lost in Time. */
class SaveLoad_v3 : public SaveLoad {
public:
	static const uint32 kSlotCount = 30;
	static const uint32 kSlotNameLength = 40;

	enum ScreenshotType {
		kScreenshotTypeGob3, //!< Goblins 3 type screenshot
		kScreenshotTypeLost  //!< Lost in Time type screenshot
	};

	SaveLoad_v3(GobEngine *vm, const char *targetName, ScreenshotType sShotType);
	virtual ~SaveLoad_v3();

	SaveMode getSaveMode(const char *fileName) const;

protected:
	struct SaveFile {
		const char *sourceName;
		SaveMode mode;
		SaveHandler *handler;
		const char *description;
	};

	class ScreenshotHandler;

	/** Handles the save slots. */
	class GameHandler : public SaveHandler {
	friend class SaveLoad_v3::ScreenshotHandler;
	public:

		GameHandler(GobEngine *vm, const char *target, bool usesScreenshots);
		~GameHandler();

		int32 getSize();
		bool load(int16 dataVar, int32 size, int32 offset);
		bool save(int16 dataVar, int32 size, int32 offset);

		bool saveScreenshot(int slot, const SavePartSprite *screenshot);
		bool loadScreenshot(int slot, SavePartSprite *screenshot);

		/** Slot file construction. */
		class File : public SlotFileIndexed {
		public:
			File(GobEngine *vm, const char *base);
			File(const File &file);
			~File();

			int getSlot(int32 offset) const;
			int getSlotRemainder(int32 offset) const;
		};
	private:

		File *_slotFile;

		bool _usesScreenshots;

		bool _firstSize;

		/** Global properties. */
		byte _props[500];
		/** Index. kSlotCount * kSlotNameLength bytes. */
		byte _index[1200];
		bool _hasIndex;

		SaveReader *_reader;
		SaveWriter *_writer;

		void buildIndex(byte *buffer) const;

		bool createReader(int slot);
		bool createWriter(int slot);

	};

	/** Handles the screenshots. */
	class ScreenshotHandler : public TempSpriteHandler {
	public:
		ScreenshotHandler(GobEngine *vm, GameHandler *gameHandler, ScreenshotType sShotType);
		~ScreenshotHandler();

		int32 getSize();
		bool load(int16 dataVar, int32 size, int32 offset);
		bool save(int16 dataVar, int32 size, int32 offset);

	private:
		/** Slot file construction. */
		class File : public SaveLoad_v3::GameHandler::File {
		public:
			File(const SaveLoad_v3::GameHandler::File &file,
					uint32 shotSize, uint32 shotIndexSize);
			~File();

			int getSlot(int32 offset) const;
			int getSlotRemainder(int32 offset) const;

			void buildIndex(byte *buffer) const;

		protected:
			uint32 _shotSize;
			uint32 _shotIndexSize;
		};

		File *_file;
		GameHandler *_gameHandler;
		ScreenshotType _sShotType;

		uint32 _shotSize;
		int32 _shotIndexSize;
		byte _index[80];
	};

	static SaveFile _saveFiles[];

	ScreenshotType _sShotType;

	GameHandler *_gameHandler;
	NotesHandler *_notesHandler;
	TempSpriteHandler *_tempSpriteHandler;
	ScreenshotHandler *_screenshotHandler;

	SaveHandler *getHandler(const char *fileName) const;
	const char *getDescription(const char *fileName) const;

	const SaveFile *getSaveFile(const char *fileName) const;
	SaveFile *getSaveFile(const char *fileName);
};

/** Save/Load class for Woodruff. */
class SaveLoad_v4 : public SaveLoad {
public:
	static const uint32 kSlotCount = 10;
	static const uint32 kSlotNameLength = 40;

	SaveLoad_v4(GobEngine *vm, const char *targetName);
	virtual ~SaveLoad_v4();

	SaveMode getSaveMode(const char *fileName) const;

protected:
	struct SaveFile {
		const char *sourceName;
		SaveMode mode;
		SaveHandler *handler;
		const char *description;
	};

	class ScreenPropsHandler;

	/** Handles the save slots. */
	class GameHandler : public SaveHandler {
	friend class SaveLoad_v4::ScreenPropsHandler;
	public:
		GameHandler(GobEngine *vm, const char *target);
		~GameHandler();

		int getLastSlot() const;

		int32 getSize();
		bool load(int16 dataVar, int32 size, int32 offset);
		bool save(int16 dataVar, int32 size, int32 offset);

		bool saveScreenProps(int slot, const byte *props);
		bool loadScreenProps(int slot, byte *props);

		/** Slot file construction. */
		class File : public SlotFileIndexed {
		public:
			File(GobEngine *vm, const char *base);
			File(const File &file);
			~File();

			int getSlot(int32 offset) const;
			int getSlotRemainder(int32 offset) const;
		};

	private:
		bool _firstSize;

		byte _props[500];
		/** The index. kSlotCount * kSlotNameLength bytes + 800 bytes 0. */
		byte _index[1200];
		bool _hasIndex;

		File *_slotFile;

		int _lastSlot;

		SaveReader *_reader;
		SaveWriter *_writer;

		void buildIndex(byte *buffer) const;

		bool createReader(int slot);
		bool createWriter(int slot);
	};

	class CurScreenPropsHandler : public SaveHandler {
	friend class SaveLoad_v4::ScreenPropsHandler;
	public:
		CurScreenPropsHandler(GobEngine *vm);
		~CurScreenPropsHandler();

		int32 getSize();
		bool load(int16 dataVar, int32 size, int32 offset);
		bool save(int16 dataVar, int32 size, int32 offset);

	private:
		byte *_props;
	};

	class ScreenPropsHandler : public SaveHandler {
	public:
		ScreenPropsHandler(GobEngine *vm, uint32 slot,
				CurScreenPropsHandler *curProps, GameHandler *gameHandler);
		~ScreenPropsHandler();

		int32 getSize();
		bool load(int16 dataVar, int32 size, int32 offset);
		bool save(int16 dataVar, int32 size, int32 offset);

	private:
		class File : public SaveLoad_v4::GameHandler::File {
		public:
			File(const SaveLoad_v4::GameHandler::File &file, uint32 slot);
			~File();

			int getSlot(int32 offset) const;
			int getSlotRemainder(int32 offset) const;

		private:
			uint32 _slot;
		};

		uint32 _slot;
		CurScreenPropsHandler *_curProps;
		GameHandler *_gameHandler;

		File *_file;
	};

	static SaveFile _saveFiles[];

	GameHandler *_gameHandler;
	CurScreenPropsHandler *_curProps;
	ScreenPropsHandler *_props[10];

	SaveHandler *getHandler(const char *fileName) const;
	const char *getDescription(const char *fileName) const;

	const SaveFile *getSaveFile(const char *fileName) const;
	SaveFile *getSaveFile(const char *fileName);
};

/** Save/Load class for Urban Runner. */
class SaveLoad_v6 : public SaveLoad {
public:
	static const uint32 kSlotCount = 60;
	static const uint32 kSlotNameLength = 40;

	SaveLoad_v6(GobEngine *vm, const char *targetName);
	virtual ~SaveLoad_v6();

	SaveMode getSaveMode(const char *fileName) const;

protected:
	struct SaveFile {
		const char *sourceName;
		SaveMode mode;
		SaveHandler *handler;
		const char *description;
	};

	/** Handles the save slots. */
	class GameHandler : public SaveHandler {
	public:
		GameHandler(GobEngine *vm, const char *target);
		~GameHandler();

		int32 getSize();
		bool load(int16 dataVar, int32 size, int32 offset);
		bool save(int16 dataVar, int32 size, int32 offset);

	private:
		/** Slot file construction. */
		class File : public SlotFileIndexed {
		public:
			File(GobEngine *vm, const char *base);
			~File();

			int getSlot(int32 offset) const;
			int getSlotRemainder(int32 offset) const;
		};

		byte _props[500];
		/** The index. 500 bytes properties + kSlotCount * kSlotNameLength bytes. */
		byte _index[2400];

		File *_slotFile;

		void buildIndex(byte *buffer) const;

		void refreshProps();
	};

	static SaveFile _saveFiles[];

	GameHandler *_gameHandler;

	SaveHandler *getHandler(const char *fileName) const;
	const char *getDescription(const char *fileName) const;

	const SaveFile *getSaveFile(const char *fileName) const;
	SaveFile *getSaveFile(const char *fileName);
};

/** Save/Load class for Playtoons. */
/** Only used for the moment to check file presence */

class SaveLoad_Playtoons : public SaveLoad {
public:
	SaveLoad_Playtoons(GobEngine *vm);
	virtual ~SaveLoad_Playtoons();

protected:
	struct SaveFile {
		const char *sourceName;
		SaveMode mode;
		SaveHandler *handler;
		const char *description;
	};

	static SaveFile _saveFiles[];

	SaveMode getSaveMode(const char *fileName) const;

	const SaveFile *getSaveFile(const char *fileName) const;
	
	SaveFile *getSaveFile(const char *fileName);

};

} // End of namespace Gob

#endif // GOB_SAVE_SAVELOAD_H
