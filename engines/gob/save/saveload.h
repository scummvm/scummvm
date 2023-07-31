/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
		kSaveModeNone,   ///< Don't handle it
		kSaveModeIgnore, ///< Ignore it
		kSaveModeExists, ///< Just claim it exists
		kSaveModeSave    ///< A normal save
	};

	/** The constructor.
	 *
	 *  @param targetName The game's target name. Used as a base for the save names.
	 */
	SaveLoad(GobEngine *vm);
	virtual ~SaveLoad();

	/** "foo\bar\quux.bla" => "quux.bla". */
	static const char *stripPath(const char *fileName, char separator = '\\');

	/** Changes all file separator characters (/,:,\) in path to newSeparator */
	static Common::String replacePathSeparators(const char *path, char newSeparator);

	/** Returns how to handle that file. */
	virtual SaveMode getSaveMode(const char *fileName) const;

	/** Returns all files known by the saving system matching a pattern. */
	virtual Common::List<Common::String> getFilesMatchingPattern(const char *pattern) const;

	/** Returns the file's (virtual) size. */
	int32 getSize(const char *fileName);
	/** Loads size bytes from offset into the variables starting with dataVar. */
	bool load(const char *fileName, int16 dataVar, int32 size, int32 offset);
	bool loadToRaw(const char *fileName, byte *ptr, int32 size, int32 offset);
	/** Saves size bytes from the variables starting with data dataVar at offset. */
	bool save(const char *fileName, int16 dataVar, int32 size, int32 offset);
	bool saveFromRaw(const char *fileName, byte *ptr, int32 size, int32 offset);
	bool copySaveGame(const char *fileNameSrc, const char *fileNameDest);

	/** Deletes the file. */
	bool deleteFile(const char *fileName);

protected:
	GobEngine *_vm;

	virtual SaveHandler *getHandler(const char *fileName) const;
	virtual const char *getDescription(const char *fileName) const;
};

/** Save/Load class for Geisha. */
class SaveLoad_Geisha : public SaveLoad {
public:
	SaveLoad_Geisha(GobEngine *vm, const char *targetName);
	~SaveLoad_Geisha() override;

	SaveMode getSaveMode(const char *fileName) const override;

protected:
	static const uint32 kSlotCount =  7;
	static const uint32 kSlotSize  = 44;

	static const uint32 kSaveFileSize = kSlotCount * kSlotSize;

	struct SaveFile {
		const char *sourceName;
		SaveMode mode;
		SaveHandler *handler;
		const char *description;
	};

	/** Handles the save slots. */
	class GameHandler : public SaveHandler {
	public:
		GameHandler(GobEngine *vm, const Common::String &target);
		~GameHandler() override;

		int32 getSize() override;
		bool load(int16 dataVar, int32 size, int32 offset) override;
		bool save(int16 dataVar, int32 size, int32 offset) override;

	private:
		/** Slot file construction. */
		class File : public SlotFileIndexed {
		public:
			File(GobEngine *vm, const Common::String &base);
			~File() override;

			int getSlot(int32 offset) const override;
			int getSlotRemainder(int32 offset) const override;
		};

		File _file;
	};

	static SaveFile _saveFiles[];

	SaveHandler *getHandler(const char *fileName) const override;
	const char *getDescription(const char *fileName) const override;

	const SaveFile *getSaveFile(const char *fileName) const;
	SaveFile *getSaveFile(const char *fileName);
};

/** Save/Load class for Gobliins 2, Ween: The Prophecy and Bargon Attack. */
class SaveLoad_v2 : public SaveLoad {
public:
	static const uint32 kSlotCount = 15;
	static const uint32 kSlotNameLength = 40;

	/** The index. kSlotCount * kSlotNameLength bytes. */
	static const uint32 kIndexSize = kSlotCount * kSlotNameLength;

	SaveLoad_v2(GobEngine *vm, const char *targetName);
	~SaveLoad_v2() override;

	SaveMode getSaveMode(const char *fileName) const override;

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
		~GameHandler() override;

		int32 getSize() override;
		bool load(int16 dataVar, int32 size, int32 offset) override;
		bool save(int16 dataVar, int32 size, int32 offset) override;

	private:
		/** Slot file construction. */
		class File : public SlotFileIndexed {
		public:
			File(GobEngine *vm, const char *base);
			~File() override;

			int getSlot(int32 offset) const override;
			int getSlotRemainder(int32 offset) const override;
		};

		byte _index[kIndexSize];
		bool _hasIndex;

		File *_slotFile;

		void buildIndex(byte *buffer) const;
	};

	static SaveFile _saveFiles[];

	GameHandler *_gameHandler;
	NotesHandler *_notesHandler;
	TempSpriteHandler *_tempSpriteHandler;

	SaveHandler *getHandler(const char *fileName) const override;
	const char *getDescription(const char *fileName) const override;

	const SaveFile *getSaveFile(const char *fileName) const;
	SaveFile *getSaveFile(const char *fileName);
};

/** Save/Load class for Fascination. */
class SaveLoad_Fascination : public SaveLoad {
public:
	static const uint32 kSlotCount = 15;
	static const uint32 kSlotNameLength = 40;

	/** The index. kSlotCount * kSlotNameLength bytes. */
	static const uint32 kIndexSize = kSlotCount * kSlotNameLength;

	SaveLoad_Fascination(GobEngine *vm, const char *targetName);
	~SaveLoad_Fascination() override;

	SaveMode getSaveMode(const char *fileName) const override;

protected:
	struct SaveFile {
		const char *sourceName;
		SaveMode mode;
		int slot;
		SaveHandler *handler;
		const char *description;
	};

	/** Handles the save slots. */
	class GameHandler : public SaveHandler {
	public:
		GameHandler(GobEngine *vm, const char *target, int slot, byte *index, bool *hasIndex);
		~GameHandler() override;

		int32 getSize() override;
		bool load(int16 dataVar, int32 size, int32 offset) override;
		bool save(int16 dataVar, int32 size, int32 offset) override;

	private:
		/** Slot file construction. */
		class File : public SlotFileIndexed {
		public:
			File(GobEngine *vm, const char *base);
			~File() override;

			int getSlot(int32 offset) const override;
			int getSlotRemainder(int32 offset) const override;
		};

		byte *_index;
		bool *_hasIndex;
		int _slot;

		File *_slotFile;

		void buildIndex(byte *buffer) const;
	};

	byte _index[kIndexSize];
	bool _hasIndex;

	static SaveFile _saveFiles[];

	SaveHandler *getHandler(const char *fileName) const override;
	const char *getDescription(const char *fileName) const override;

	const SaveFile *getSaveFile(const char *fileName) const;
	SaveFile *getSaveFile(const char *fileName);
};

/** Save/Load class for Adibou 1 */
class SaveLoad_Adibou1 : public SaveLoad {
public:
	static const int32 kAdibou1NbrOfDrawings = 8;
	SaveLoad_Adibou1(GobEngine *vm, const char *targetName);
	~SaveLoad_Adibou1() override;

	SaveMode getSaveMode(const char *fileName) const override;

protected:
	struct SaveFile {
		const char *sourceName;
		SaveMode mode;
		SaveHandler *handler;
		const char *description;
	};

	class SpriteHandler : public TempSpriteHandler {
	public:
		SpriteHandler(GobEngine *vm, const Common::String &target, const Common::String &ext);
		~SpriteHandler() override;

		int32 getSize() override;
		bool load(int16 dataVar, int32 size, int32 offset) override;
		bool save(int16 dataVar, int32 size, int32 offset) override;

	private:
		class File : public SlotFileStatic {
		public:
			File(GobEngine *vm, const Common::String &base, const Common::String &ext);
			~File() override;
		};

		File _file;
	};


	class DrawingWithThumbnailHandler : public TempSpriteHandler {
	public:
		DrawingWithThumbnailHandler(GobEngine *vm,
								   const Common::String &target,
								   const Common::String &ext);
		~DrawingWithThumbnailHandler() override;

		int32 getSize() override;
		bool load(int16 dataVar, int32 size, int32 offset) override;
		bool save(int16 dataVar, int32 size, int32 offset) override;

	private:
		class File : public SlotFileStatic {
		public:
			File(GobEngine *vm, const Common::String &base, const Common::String &ext);
			~File() override;
		};

		File _file;
		SaveWriter *_writer;
		SaveReader *_reader;
	};

	class GameFileHandler : public SaveHandler {
	public:
		GameFileHandler(GobEngine *vm, const Common::String &target, const Common::String &ext);
		~GameFileHandler() override;

		int32 getSize() override;
		bool load(int16 dataVar, int32 size, int32 offset) override;
		bool save(int16 dataVar, int32 size, int32 offset) override;
		bool deleteFile() override;

	private:
		// Save from raw pointer if ptrRaw != nullptr, else save from game variables
		bool save(const byte *ptrRaw, int16 dataVar, int32 size, int32 offset);

		class File : public SlotFileStatic {
		public:
			File(GobEngine *vm, const Common::String &base, const Common::String &ext);
			~File() override;
		};

		File _file;
	};

	static SaveFile _saveFiles[];

	GameFileHandler *_bouHandler;
	GameFileHandler *_constructionHandler;
	SpriteHandler   *_drawingHandler;
	TempSpriteHandler *_menuHandler;
	DrawingWithThumbnailHandler *_drawingWithThumbnailHandler[kAdibou1NbrOfDrawings];

	SaveHandler *getHandler(const char *fileName) const override;
	const char *getDescription(const char *fileName) const override;

	const SaveFile *getSaveFile(const char *fileName) const;
	SaveFile *getSaveFile(const char *fileName);
};

/** Save/Load class for Goblins 3 and Lost in Time. */
class SaveLoad_v3 : public SaveLoad {
public:
	static const uint32 kSlotCount = 30;
	static const uint32 kSlotNameLength = 40;

	static const uint32 kPropsSize = 500;
	/** Index. kSlotCount * kSlotNameLength bytes. */
	static const uint32 kIndexSize = kSlotCount * kSlotNameLength;

	enum ScreenshotType {
		kScreenshotTypeGob3, ///< Goblins 3 type screenshot
		kScreenshotTypeLost  ///< Lost in Time type screenshot
	};

	SaveLoad_v3(GobEngine *vm, const char *targetName, ScreenshotType sShotType);
	~SaveLoad_v3() override;

	SaveMode getSaveMode(const char *fileName) const override;

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
		~GameHandler() override;

		int32 getSize() override;
		bool load(int16 dataVar, int32 size, int32 offset) override;
		bool save(int16 dataVar, int32 size, int32 offset) override;

		bool saveScreenshot(int slot, const SavePartSprite *screenshot);
		bool loadScreenshot(int slot, SavePartSprite *screenshot);

		/** Slot file construction. */
		class File : public SlotFileIndexed {
		public:
			File(GobEngine *vm, const char *base);
			File(const File &file);
			~File() override;

			int getSlot(int32 offset) const override;
			int getSlotRemainder(int32 offset) const override;
		};
	private:

		File *_slotFile;

		bool _usesScreenshots;

		bool _firstSize;

		/** Global properties. */
		byte _props[kPropsSize];
		byte _index[kIndexSize];
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
		~ScreenshotHandler() override;

		int32 getSize() override;
		bool load(int16 dataVar, int32 size, int32 offset) override;
		bool save(int16 dataVar, int32 size, int32 offset) override;

	private:
		/** Slot file construction. */
		class File : public SaveLoad_v3::GameHandler::File {
		public:
			File(const SaveLoad_v3::GameHandler::File &file,
					uint32 shotSize, uint32 shotIndexSize);
			~File() override;

			int getSlot(int32 offset) const override;
			int getSlotRemainder(int32 offset) const override;

			void buildScreenshotIndex(byte *buffer) const;

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

	SaveHandler *getHandler(const char *fileName) const override;
	const char *getDescription(const char *fileName) const override;

	const SaveFile *getSaveFile(const char *fileName) const;
	SaveFile *getSaveFile(const char *fileName);
};

/** Save/Load class for Inca 2. */
class SaveLoad_Inca2 : public SaveLoad {
public:
	static const uint32 kSlotCount =  40;
	static const uint32 kPropsSize = 500;

	SaveLoad_Inca2(GobEngine *vm, const char *targetName);
	~SaveLoad_Inca2() override;

	SaveMode getSaveMode(const char *fileName) const override;

protected:
	struct SaveFile {
		const char *sourceName;
		SaveMode mode;
		SaveHandler *handler;
		const char *description;
	};

	/** Handles the voice language info file. */
	class VoiceHandler : public SaveHandler {
	public:
		VoiceHandler(GobEngine *vm);
		~VoiceHandler() override;

		int32 getSize() override;
		bool load(int16 dataVar, int32 size, int32 offset) override;
		bool save(int16 dataVar, int32 size, int32 offset) override;
	};

	class ScreenshotHandler;

	/** Handles the save slots. */
	class GameHandler : public SaveHandler {
	friend class SaveLoad_Inca2::ScreenshotHandler;
	public:

		GameHandler(GobEngine *vm, const char *target);
		~GameHandler() override;

		int32 getSize() override;
		bool load(int16 dataVar, int32 size, int32 offset) override;
		bool save(int16 dataVar, int32 size, int32 offset) override;

		bool saveScreenshot(int slot, const SavePartSprite *screenshot);
		bool loadScreenshot(int slot, SavePartSprite *screenshot);

		/** Slot file construction. */
		class File : public SlotFileIndexed {
		public:
			File(GobEngine *vm, const char *base);
			File(const File &file);
			~File() override;

			int getSlot(int32 offset) const override;
			int getSlotRemainder(int32 offset) const override;
		};

	private:

		File *_slotFile;

		byte _props[kPropsSize];

		SaveReader *_reader;
		SaveWriter *_writer;

		void buildIndex();

		bool createReader(int slot);
		bool createWriter(int slot);
	};

	/** Handles the screenshots. */
	class ScreenshotHandler : public TempSpriteHandler {
	public:
		ScreenshotHandler(GobEngine *vm, GameHandler *gameHandler);
		~ScreenshotHandler() override;

		int32 getSize() override;
		bool load(int16 dataVar, int32 size, int32 offset) override;
		bool save(int16 dataVar, int32 size, int32 offset) override;

	private:
		/** Slot file construction. */
		class File : public SaveLoad_Inca2::GameHandler::File {
		public:
			File(const SaveLoad_Inca2::GameHandler::File &file);
			~File() override;

			int getSlot(int32 offset) const override;
			int getSlotRemainder(int32 offset) const override;

			void buildScreenshotIndex(byte *buffer) const;
		};

		File *_file;
		GameHandler *_gameHandler;

		byte _index[80];
	};

	static SaveFile _saveFiles[];

	VoiceHandler      *_voiceHandler;
	TempSpriteHandler *_tempSpriteHandler;
	GameHandler       *_gameHandler;
	ScreenshotHandler *_screenshotHandler;

	SaveHandler *getHandler(const char *fileName) const override;
	const char *getDescription(const char *fileName) const override;

	const SaveFile *getSaveFile(const char *fileName) const;
	SaveFile *getSaveFile(const char *fileName);
};

/** Save/Load class for Woodruff. */
class SaveLoad_v4 : public SaveLoad {
public:
	static const uint32 kSlotCount = 10;
	static const uint32 kSlotNameLength = 40;

	static const uint32 kPropsSize = 500;
	/** Index. kSlotCount * kSlotNameLength bytes + 800 bytes 0. */
	static const uint32 kIndexSize = (kSlotCount * kSlotNameLength) + 800;

	SaveLoad_v4(GobEngine *vm, const char *targetName);
	~SaveLoad_v4() override;

	SaveMode getSaveMode(const char *fileName) const override;

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
		~GameHandler() override;

		int getLastSlot() const;

		int32 getSize() override;
		bool load(int16 dataVar, int32 size, int32 offset) override;
		bool save(int16 dataVar, int32 size, int32 offset) override;

		bool saveScreenProps(int slot, const byte *props);
		bool loadScreenProps(int slot, byte *props);

		/** Slot file construction. */
		class File : public SlotFileIndexed {
		public:
			File(GobEngine *vm, const char *base);
			File(const File &file);
			~File() override;

			int getSlot(int32 offset) const override;
			int getSlotRemainder(int32 offset) const override;
		};

	private:
		bool _firstSize;

		byte _props[kPropsSize];
		byte _index[kIndexSize];
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
		~CurScreenPropsHandler() override;

		int32 getSize() override;
		bool load(int16 dataVar, int32 size, int32 offset) override;
		bool save(int16 dataVar, int32 size, int32 offset) override;

	private:
		byte *_props;
	};

	class ScreenPropsHandler : public SaveHandler {
	public:
		ScreenPropsHandler(GobEngine *vm, uint32 slot,
				CurScreenPropsHandler *curProps, GameHandler *gameHandler);
		~ScreenPropsHandler() override;

		int32 getSize() override;
		bool load(int16 dataVar, int32 size, int32 offset) override;
		bool save(int16 dataVar, int32 size, int32 offset) override;

	private:
		class File : public SaveLoad_v4::GameHandler::File {
		public:
			File(const SaveLoad_v4::GameHandler::File &file, uint32 slot);
			~File() override;

			int getSlot(int32 offset) const override;
			int getSlotRemainder(int32 offset) const override;

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

	SaveHandler *getHandler(const char *fileName) const override;
	const char *getDescription(const char *fileName) const override;

	const SaveFile *getSaveFile(const char *fileName) const;
	SaveFile *getSaveFile(const char *fileName);
};

/** Save/Load class for Urban Runner. */
class SaveLoad_v6 : public SaveLoad {
public:
	static const uint32 kSlotCount = 60;
	static const uint32 kSlotNameLength = 40;

	static const uint32 kPropsSize = 500;
	/** Index. kSlotCount * kSlotNameLength bytes. */
	static const uint32 kIndexSize = kSlotCount * kSlotNameLength;

	SaveLoad_v6(GobEngine *vm, const char *targetName);
	~SaveLoad_v6() override;

	SaveMode getSaveMode(const char *fileName) const override;

protected:
	struct SaveFile {
		const char *sourceName;
		SaveMode mode;
		SaveHandler *handler;
		const char *description;
	};

	/** Handles the temp sprites. */
	class SpriteHandler : public TempSpriteHandler {
	public:
		SpriteHandler(GobEngine *vm);
		~SpriteHandler() override;

		bool set(SaveReader *reader, uint32 part);
		bool get(SaveWriter *writer, uint32 part);
	};

	/** Handles the save slots. */
	class GameHandler : public SaveHandler {
	public:
		GameHandler(GobEngine *vm, const char *target, SpriteHandler &spriteHandler);
		~GameHandler() override;

		int32 getSize() override;
		bool load(int16 dataVar, int32 size, int32 offset) override;
		bool save(int16 dataVar, int32 size, int32 offset) override;

		uint8 getExtraID(int slot);
		bool loadExtra(int slot, uint8 id, int16 dataVar, int32 size, int32 offset);
		bool saveExtra(int slot, uint8 id, int16 dataVar, int32 size, int32 offset);

	private:
		/** Slot file construction. */
		class File : public SlotFileIndexed {
		public:
			File(GobEngine *vm, const char *base);
			~File() override;

			int getSlot(int32 offset) const override;
			int getSlotRemainder(int32 offset) const override;
		};

		SpriteHandler *_spriteHandler;

		byte _props[kPropsSize];
		byte _index[kIndexSize];

		File *_slotFile;

		SaveReader *_reader;
		SaveWriter *_writer;

		bool _hasExtra;

		void buildIndex(byte *buffer) const;

		void refreshProps();

		bool createReader(int slot);
		bool createWriter(int slot);
	};

	/** Handles the autosave. */
	class AutoHandler : public SaveHandler {
	public:
		AutoHandler(GobEngine *vm, const Common::String &target);
		~AutoHandler() override;

		int32 getSize() override;
		bool load(int16 dataVar, int32 size, int32 offset) override;
		bool save(int16 dataVar, int32 size, int32 offset) override;

	private:
		class File : public SlotFileStatic {
		public:
			File(GobEngine *vm, const Common::String &base);
			~File() override;
		};

		File _file;
	};

	/** Handles the autosave temp sprite. */
	class AutoSpriteHandler : public TempSpriteHandler {
	public:
		AutoSpriteHandler(GobEngine *vm, const Common::String &target);
		~AutoSpriteHandler() override;

		int32 getSize() override;
		bool load(int16 dataVar, int32 size, int32 offset) override;
		bool save(int16 dataVar, int32 size, int32 offset) override;

	private:
		class File : public SlotFileStatic {
		public:
			File(GobEngine *vm, const Common::String &base);
			~File() override;
		};

		File _file;
	};

	/** Handles the temporary saves. */
	class TempHandler : public SaveHandler {
	public:
		TempHandler(GobEngine *vm);
		~TempHandler() override;

		int32 getSize() override;
		bool load(int16 dataVar, int32 size, int32 offset) override;
		bool save(int16 dataVar, int32 size, int32 offset) override;
		bool deleteFile() override;

	private:
		bool _empty;

		uint32 _size;
		byte *_data;
	};

	class ExtraHandler : public SaveHandler {
	public:
		ExtraHandler(GobEngine *vm, GameHandler &game, uint8 id, int slot);
		~ExtraHandler() override;

		int32 getSize() override;
		bool load(int16 dataVar, int32 size, int32 offset) override;
		bool save(int16 dataVar, int32 size, int32 offset) override;

	private:
		uint8 _id;
		int   _slot;

		GameHandler *_game;
	};

	static SaveFile _saveFiles[];

	SpriteHandler     *_spriteHandler;
	GameHandler       *_gameHandler;
	AutoHandler       *_autoHandler;
	AutoSpriteHandler *_autoSpriteHandler;
	TempHandler       *_tmpHandler[2];
	ExtraHandler      *_extraHandler[120];

	SaveHandler *getHandler(const char *fileName) const override;
	const char *getDescription(const char *fileName) const override;

	const SaveFile *getSaveFile(const char *fileName) const;
	SaveFile *getSaveFile(const char *fileName);
};

/** Save/Load class for Playtoons. */
class SaveLoad_Playtoons : public SaveLoad {
public:
	static const uint32 kSlotCount = 60;
	static const uint32 kSlotNameLength = 40;

	static const uint32 kPropsSize = 3921;
	/** Index. kSlotCount * kSlotNameLength bytes. */
	static const uint32 kIndexSize = kSlotCount * kSlotNameLength;

	SaveLoad_Playtoons(GobEngine *vm, const char *targetName);
	~SaveLoad_Playtoons() override;

	SaveMode getSaveMode(const char *fileName) const override;

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
		~GameHandler() override;

		int32 getSize() override;
		bool load(int16 dataVar, int32 size, int32 offset) override;
		bool save(int16 dataVar, int32 size, int32 offset) override;

	private:
		/** Slot file construction. */
		class File : public SlotFileIndexed {
		public:
			File(GobEngine *vm, const char *base);
			~File() override;

			int getSlot(int32 offset) const override;
			int getSlotRemainder(int32 offset) const override;
		};

		byte _props[kPropsSize];
		byte _index[kIndexSize];

		File *_slotFile;
		TempSpriteHandler *_tempSpriteHandler;

		void buildIndex(byte *buffer) const;
	};

	static SaveFile _saveFiles[];

	GameHandler *_gameHandler;

	SaveHandler *getHandler(const char *fileName) const override;
	const char *getDescription(const char *fileName) const override;

	const SaveFile *getSaveFile(const char *fileName) const;
	SaveFile *getSaveFile(const char *fileName);
};

/** Save/Load class for Adibou 2/Adi. */
class SaveLoad_v7: public SaveLoad {
public:
	static const uint32 kChildrenCount = 16;
	static const uint32 kAdibou2NbrOfApplications = 5;
	static const uint32 kAdibou2NbrOfSavedDrawings = 12;
	static const uint32 kAdibou2NbrOfConstructionGameFiles = 3;
	SaveLoad_v7(GobEngine *vm, const char *targetName);
	~SaveLoad_v7() override;

	SaveMode getSaveMode(const char *fileName) const override;
	Common::List<Common::String> getFilesMatchingPattern(const char *pattern) const override;

protected:
	struct SaveFile {
		const char *sourceName;
		SaveMode mode;
		SaveHandler *handler;
		const char *description;
	};

	class SpriteHandler : public TempSpriteHandler {
	public:
		SpriteHandler(GobEngine *vm, const Common::String &target, const Common::String &ext);
		~SpriteHandler() override;

		int32 getSize() override;
		bool load(int16 dataVar, int32 size, int32 offset) override;
		bool save(int16 dataVar, int32 size, int32 offset) override;
		bool loadToRaw(byte *ptr, int32 size, int32 offset) override;
		bool saveFromRaw(const byte* ptr, int32 size, int32 offset) override;
		bool deleteFile() override;

	private:
		class File : public SlotFileStatic {
		public:
			File(GobEngine *vm, const Common::String &base, const Common::String &ext);
			~File() override;
		};

		File _file;
	};

	class DrawingOnFloppyDiskHandler : public TempSpriteHandler {
	public:
		DrawingOnFloppyDiskHandler(GobEngine *vm,
								   SaveReader *reader,
								   SaveWriter *writer,
								   bool isThumbnail,
								   uint32 chunkSize = 1);
		~DrawingOnFloppyDiskHandler() override;

		int32 getSize() override;
		bool load(int16 dataVar, int32 size, int32 offset) override;
		bool save(int16 dataVar, int32 size, int32 offset) override;
		bool deleteFile() override;

	private:
		class File : public SlotFileStatic {
		public:
			File(GobEngine *vm, const Common::String &base, const Common::String &ext);
			~File() override;
		};

		//File _file;
		SaveWriter *_writer;
		SaveReader *_reader;
		bool _isThumbnail;
		int32 _chunkSize;
	};

	class GameFileHandler : public SaveHandler {
	public:
		GameFileHandler(GobEngine *vm, const Common::String &target, const Common::String &ext);
		~GameFileHandler() override;

		int32 getSize() override;
		bool load(int16 dataVar, int32 size, int32 offset) override;
		bool save(int16 dataVar, int32 size, int32 offset) override;
		bool loadToRaw(byte *ptr, int32 size, int32 offset) override;
		bool saveFromRaw(const byte *ptr, int32 size, int32 offset) override;
		bool deleteFile() override;

	private:
		// Save from raw pointer if ptrRaw != nullptr, else save from game variables
		bool save(const byte *ptrRaw, int16 dataVar, int32 size, int32 offset);

		class File : public SlotFileStatic {
		public:
			File(GobEngine *vm, const Common::String &base, const Common::String &ext);
			~File() override;
		};

		File _file;
	};


	static SaveFile _saveFiles[];

	SpriteHandler               *_faceHandler[kChildrenCount];
	FakeFileHandler             *_childrenHandler;
	FakeFileHandler             *_debilHandler[4];
	GameFileHandler             *_configHandler;
	GameFileHandler             *_adibou2EnvHandler[kChildrenCount];
	SpriteHandler               *_adibou2WeatherHandler[kChildrenCount];
	GameFileHandler             *_adibou2BreakoutGameProgressHandler[kChildrenCount];
	FakeFileHandler             *_adibou2ConstructionGameTempFileHandler[kAdibou2NbrOfConstructionGameFiles];
	GameFileHandler             *_adibou2ConstructionGameProgressHandler[kChildrenCount][kAdibou2NbrOfConstructionGameFiles];
	GameFileHandler             *_adibou2AppProgressHandler[kChildrenCount][kAdibou2NbrOfApplications];
	GameFileHandler             *_adibou2MemoHandler[kChildrenCount];
	GameFileHandler             *_adibou2DiploHandler[kChildrenCount];
	GameFileHandler             *_adibou2AppliHandler[kAdibou2NbrOfApplications];
	GameFileHandler             *_adibou2CriteHandler[kAdibou2NbrOfApplications];
	GameFileHandler             *_adibou2ExoHandler[kAdibou2NbrOfApplications];
	GameFileHandler             *_adibou2ApplicationsInfoHandler;
	FakeFileHandler             *_adibou2RetourHandler;
	FakeFileHandler             *_adibou2LanceHandler;
	TempSpriteHandler           *_adibou2AppIcoHandler[kAdibou2NbrOfApplications];
	SpriteHandler               *_adibou2DrawingHandler[kChildrenCount][kAdibou2NbrOfSavedDrawings];
	SpriteHandler               *_adibou2DrawingThumbnailHandler[kChildrenCount][kAdibou2NbrOfSavedDrawings];
	GameFileHandler             *_adibou2DrawingPaletteHandler[kChildrenCount];
	SpriteHandler               *_adibou2DrawingMailboxHandler[kChildrenCount];
	SpriteHandler               *_adibou2CakePhotoHandler[kChildrenCount];
	SpriteHandler               *_adibou2FlowerPhotoHandler[kChildrenCount];
	SpriteHandler               *_adibou2FlowerInVaseHandler[kChildrenCount];
	SpriteHandler               *_adibou2FadedFlowerInVaseHandler[kChildrenCount];
	SpriteHandler               *_adibou2PosterHandler[kChildrenCount];
	DrawingOnFloppyDiskHandler  *_adibou2DrawingOnFloppyDiskHandler;
	DrawingOnFloppyDiskHandler  *_adibou2DrawingThumbnailOnFloppyDiskHandler;
	FakeFileHandler             *_adibou2TestDobHandler;
	FakeFileHandler             *_adibou2ExerciseListHandler;

	FakeFileHandler             *_adi4BaseHandler[2];
	FakeFileHandler             *_adi4EcoleHandler[11];

	SaveHandler *getHandler(const char *fileName) const override;
	const char *getDescription(const char *fileName) const override;

	const SaveFile *getSaveFile(const char *fileName) const;
};

} // End of namespace Gob

#endif // GOB_SAVE_SAVELOAD_H
