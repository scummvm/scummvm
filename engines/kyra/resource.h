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

#ifndef KYRA_RESOURCE_H
#define KYRA_RESOURCE_H


#include "common/scummsys.h"
#include "common/str.h"
#include "common/file.h"
#include "common/list.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/stream.h"
#include "common/ptr.h"

#include "kyra/kyra_v1.h"
#include "kyra/kyra_hof.h"

namespace Kyra {

struct ResFileEntry {
	Common::String parent;
	mutable ResFileEntry *parentEntry;	// Cache to avoid lookup by string in the map
										// No smart invalidation is needed because the map is cleared globally
										// or expanded but no element is ever removed
	uint32 size;

	bool preload;
	bool mounted;
	bool prot;

	enum kType {
		kRaw = 0,
		kPak = 1,
		kInsMal = 2,
		kTlk = 3,
		kAutoDetect
	};
	kType type;
	uint32 offset;
};

struct CompFileEntry {
	uint32 size;
	uint8 *data;
};

typedef Common::HashMap<Common::String, ResFileEntry, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> ResFileMap;
typedef Common::HashMap<Common::String, CompFileEntry, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> CompFileMap;
class Resource;

class ResArchiveLoader {
public:
	struct File {
		File() : filename(), entry() {}
		File(const Common::String &f, const ResFileEntry &e) : filename(f), entry(e) {}

		bool operator ==(const Common::String &r) const {
			return filename.equalsIgnoreCase(r);
		}

		Common::String filename;
		ResFileEntry entry;
	};
	typedef Common::List<File> FileList;

	virtual ~ResArchiveLoader() {}

	virtual bool checkFilename(Common::String filename) const = 0;
	virtual bool isLoadable(const Common::String &filename, Common::SeekableReadStream &stream) const = 0;
	virtual bool loadFile(const Common::String &filename, Common::SeekableReadStream &stream, FileList &files) const = 0;
	// parameter 'archive' can be deleted by this method and it may not be deleted from the caller
	virtual Common::SeekableReadStream *loadFileFromArchive(const Common::String &file, Common::SeekableReadStream *archive, const ResFileEntry entry) const = 0;

	virtual ResFileEntry::kType getType() const = 0;
protected:
};

class CompArchiveLoader {
public:
	virtual ~CompArchiveLoader() {}

	virtual bool checkForFiles() const = 0;
	virtual bool loadFile(CompFileMap &loadTo) const = 0;
};

class Resource {
public:
	Resource(KyraEngine_v1 *vm);
	~Resource();

	bool reset();

	bool loadPakFile(const Common::String &filename);
	bool preloadProtected(const Common::String &filename);
	void unloadPakFile(const Common::String &filename);
	bool isInPakList(const Common::String &filename);

	bool loadFileList(const Common::String &filedata);
	bool loadFileList(const char * const *filelist, uint32 numFiles);
	// This unloads *all* pakfiles, even kyra.dat and protected ones
	void unloadAllPakFiles();

	bool exists(const char *file, bool errorOutOnFail=false);
	uint32 getFileSize(const char *file);
	uint8* fileData(const char *file, uint32 *size);
	Common::SeekableReadStream *getFileStream(const Common::String &file);

	bool loadFileToBuf(const char *file, void *buf, uint32 maxSize);
protected:
	void checkFile(const Common::String &file);
	bool isAccessible(const Common::String &file);
	bool isAccessible(const ResFileEntry *fileEntry);

	void detectFileTypes();
	void detectFileType(const Common::String &filename, ResFileEntry *fileEntry);

	void initializeLoaders();
	const ResArchiveLoader *getLoader(ResFileEntry::kType type) const;
	typedef Common::List<Common::SharedPtr<ResArchiveLoader> > LoaderList;
	typedef LoaderList::iterator LoaderIterator;
	typedef LoaderList::const_iterator CLoaderIterator;
	LoaderList _loaders;
	ResFileMap _map;

	ResFileEntry *getParentEntry(const ResFileEntry *entry) const;
	ResFileEntry *getParentEntry(const Common::String &filename) const;

	typedef Common::List<Common::SharedPtr<CompArchiveLoader> > CompLoaderList;
	typedef CompLoaderList::iterator CompLoaderIterator;
	typedef CompLoaderList::const_iterator CCompLoaderIterator;
	CompLoaderList _compLoaders;
	CompFileMap _compFiles;

	void tryLoadCompFiles();
	void clearCompFileList();

	KyraEngine_v1 *_vm;
};

enum kKyraResources {
	kLoadAll = -1,

	k1ForestSeq,
	k1KallakWritingSeq,
	k1KyrandiaLogoSeq,
	k1KallakMalcolmSeq,
	k1MalcolmTreeSeq,
	k1WestwoodLogoSeq,

	k1Demo1Seq,
	k1Demo2Seq,
	k1Demo3Seq,
	k1Demo4Seq,

	k1AmuleteAnimSeq,

	k1OutroReunionSeq,

	k1IntroCPSStrings,
	k1IntroCOLStrings,
	k1IntroWSAStrings,
	k1IntroStrings,

	k1OutroHomeString,

	k1RoomFilenames,
	k1RoomList,

	k1CharacterImageFilenames,

	k1ItemNames,
	k1TakenStrings,
	k1PlacedStrings,
	k1DroppedStrings,
	k1NoDropStrings,

	k1PutDownString,
	k1WaitAmuletString,
	k1BlackJewelString,
	k1PoisonGoneString,
	k1HealingTipString,
	k1WispJewelStrings,
	k1MagicJewelStrings,

	k1ThePoisonStrings,
	k1FluteStrings,

	k1FlaskFullString,
	k1FullFlaskString,

	k1VeryCleverString,
	k1NewGameString,

	k1DefaultShapes,
	k1Healing1Shapes,
	k1Healing2Shapes,
	k1PoisonDeathShapes,
	k1FluteShapes,
	k1Winter1Shapes,
	k1Winter2Shapes,
	k1Winter3Shapes,
	k1DrinkShapes,
	k1WispShapes,
	k1MagicAnimShapes,
	k1BranStoneShapes,

	k1PaletteList,

	k1GUIStrings,
	k1ConfigStrings,

	k1AudioTracks,
	k1AudioTracksIntro,

	k1TownsSFXwdTable,
	k1TownsSFXbtTable,
	k1TownsCDATable,
	k1CreditsStrings,

	k2SeqplayPakFiles,
	k2SeqplayCredits,
	k2SeqplayCreditsSpecial,
	k2SeqplayStrings,
	k2SeqplaySfxFiles,
	k2SeqplayTlkFiles,
	k2SeqplaySeqData,
	k2SeqplayIntroTracks,
	k2SeqplayFinaleTracks,
	k2SeqplayIntroCDA,
	k2SeqplayFinaleCDA,
	k2SeqplayShapeAnimData,

	k2IngamePakFiles,
	k2IngameSfxFiles,
	k2IngameSfxIndex,
	k2IngameTracks,
	k2IngameCDA,
	k2IngameTalkObjIndex,
	k2IngameTimJpStrings,
	k2IngameShapeAnimData,
	k2IngameTlkDemoStrings,

	k3MainMenuStrings,
	k3MusicFiles,
	k3ScoreTable,
	k3SfxFiles,
	k3SfxMap,
	k3ItemAnimData,
	k3ItemMagicTable,
	k3ItemStringMap,

	kMaxResIDs
};

struct Shape;
struct Room;

class StaticResource {
public:
	static const Common::String staticDataFilename() { return "kyra.dat"; }

	StaticResource(KyraEngine_v1 *vm) : _vm(vm), _resList(), _fileLoader(0), _builtIn(0), _filenameTable(0) {}
	~StaticResource() { deinit(); }

	static bool checkKyraDat();

	bool init();
	void deinit();

	const char * const*loadStrings(int id, int &strings);
	const uint8 *loadRawData(int id, int &size);
	const Shape *loadShapeTable(int id, int &entries);
	const Room *loadRoomTable(int id, int &entries);
	const uint8 * const*loadPaletteTable(int id, int &entries);
	const HofSeqData *loadHofSequenceData(int id, int &entries);
	const ItemAnimData_v1 *loadShapeAnimData_v1(int id, int &entries);
	const ItemAnimData_v2 *loadShapeAnimData_v2(int id, int &entries);

	// use '-1' to prefetch/unload all ids
	// prefetchId retruns false if only on of the resources
	// can't be loaded and it breaks then the first res
	// can't be loaded
	bool prefetchId(int id);
	void unloadId(int id);
private:
	void outputError(const Common::String &error);

	KyraEngine_v1 *_vm;

	struct FilenameTable;
	struct ResData;
	struct FileType;

	bool checkResList(int id, int &type, const void *&ptr, int &size);
	const void *checkForBuiltin(int id, int &type, int &size);
	const FilenameTable *searchFile(int id);
	const FileType *getFiletype(int type);
	const void *getData(int id, int requesttype, int &size);

	bool loadLanguageTable(const char *filename, void *&ptr, int &size);
	bool loadStringTable(const char *filename, void *&ptr, int &size);
	bool loadRawData(const char *filename, void *&ptr, int &size);
	bool loadShapeTable(const char *filename, void *&ptr, int &size);
	bool loadRoomTable(const char *filename, void *&ptr, int &size);
	bool loadPaletteTable(const char *filename, void *&ptr, int &size);
	bool loadHofSequenceData(const char *filename, void *&ptr, int &size);
	bool loadShapeAnimData_v1(const char *filename, void *&ptr, int &size);
	bool loadShapeAnimData_v2(const char *filename, void *&ptr, int &size);

	void freeRawData(void *&ptr, int &size);
	void freeStringTable(void *&ptr, int &size);
	void freeShapeTable(void *&ptr, int &size);
	void freeRoomTable(void *&ptr, int &size);
	void freePaletteTable(void *&ptr, int &size);
	void freeHofSequenceData(void *&ptr, int &size);
	void freeHofShapeAnimDataV1(void *&ptr, int &size);
	void freeHofShapeAnimDataV2(void *&ptr, int &size);

	const char *getFilename(const char *name);
	uint8 *getFile(const char *name, int &size);

	enum kResTypes {
		kLanguageList,
		kStringList,
		kRoomList,
		kShapeList,
		kRawData,
		kPaletteTable,

		k2SeqData,
		k2ShpAnimDataV1,
		k2ShpAnimDataV2
	};

	struct BuiltinRes {
		int id;
		int type;
		int size;
		const void *data;
	};

	struct FilenameTable {
		int id;
		int type;
		const char *filename;
	};

	struct FileType {
		int type;
		typedef bool (StaticResource::*LoadFunc)(const char *filename, void *&ptr, int &size);
		typedef void (StaticResource::*FreeFunc)(void *&ptr, int &size);

		LoadFunc load;
		FreeFunc free;
	};

	struct ResData {
		int id;
		int type;
		int size;
		void *data;
	};

	Common::List<ResData> _resList;

	const FileType *_fileLoader;
	const BuiltinRes *_builtIn;
	const FilenameTable *_filenameTable;
};

} // end of namespace Kyra

#endif




