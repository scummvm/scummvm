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
#include "common/archive.h"

#include "kyra/kyra_v1.h"
#include "kyra/lol.h"
#include "kyra/kyra_hof.h"

namespace Kyra {

class Resource;

class ResArchiveLoader;

class Resource {
public:
	Resource(KyraEngine_v1 *vm);
	~Resource();

	bool reset();

	bool loadPakFile(Common::String filename);
	bool loadPakFile(Common::String name, Common::SharedPtr<Common::ArchiveMember> file);

	void unloadPakFile(Common::String filename, bool remFromCache = false);

	bool isInPakList(Common::String filename);

	bool isInCacheList(Common::String name);

	bool loadFileList(const Common::String &filedata);
	bool loadFileList(const char * const *filelist, uint32 numFiles);

	// This unloads *all* pakfiles, even kyra.dat and protected ones.
	// It does not remove files from cache though!
	void unloadAllPakFiles();

	void listFiles(const Common::String &pattern, Common::ArchiveMemberList &list);

	bool exists(const char *file, bool errorOutOnFail=false);
	uint32 getFileSize(const char *file);
	uint8* fileData(const char *file, uint32 *size);
	Common::SeekableReadStream *createReadStream(const Common::String &file);

	bool loadFileToBuf(const char *file, void *buf, uint32 maxSize);
protected:
	typedef Common::HashMap<Common::String, Common::Archive*, Common::CaseSensitiveString_Hash, Common::CaseSensitiveString_EqualTo> ArchiveMap;
	ArchiveMap _archiveCache;

	Common::SearchSet _files;
	Common::SearchSet _archiveFiles;
	Common::SearchSet _protectedFiles;

	Common::Archive *loadArchive(const Common::String &name, Common::SharedPtr<Common::ArchiveMember> member);
	Common::Archive *loadInstallerArchive(const Common::String &file, const Common::String &ext, const uint8 offset);

	bool loadProtectedFiles(const char * const * list);

	void initializeLoaders();

	typedef Common::List<Common::SharedPtr<ResArchiveLoader> > LoaderList;
	LoaderList _loaders;

	KyraEngine_v1 *_vm;
};

enum KyraResources {
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

	k1CreditsStrings,

	k1TownsSFXwdTable,
	k1TownsSFXbtTable,
	k1TownsCDATable,

	k1AmigaIntroSFXTable,
	k1AmigaGameSFXTable,

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

#ifdef ENABLE_LOL
	kLolIngamePakFiles,
	kLolCharacterDefs,
	kLolIngameSfxFiles,
	kLolIngameSfxIndex,
	kLolMusicTrackMap,
	kLolIngameGMSfxIndex,
	kLolIngameMT32SfxIndex,
	//kLolIngameADLSfxIndex,
	kLolSpellProperties,
	kLolGameShapeMap,
	kLolSceneItemOffs,
	kLolCharInvIndex,
	kLolCharInvDefs,
	kLolCharDefsMan,
	kLolCharDefsWoman,
	kLolCharDefsKieran,
	//kLolCharDefsUnk,
	kLolCharDefsAkshel,
	kLolExpRequirements,
	kLolMonsterModifiers,
	kLolMonsterShiftOffsets,
	kLolMonsterDirFlags,
	kLolMonsterScaleY,
	kLolMonsterScaleX,
	kLolMonsterScaleWH,
	kLolFlyingObjectShp,
	kLolInventoryDesc,

	kLolLevelShpList,
	kLolLevelDatList,
	kLolCompassDefs,
	kLolItemPrices,
	kLolStashSetup,

	kLolDscUnk1,
	kLolDscShapeIndex,
	kLolDscOvlMap,
	kLolDscScaleWidthData,
	kLolDscScaleHeightData,
	kLolDscX,
	kLolDscY,
	kLolDscTileIndex,
	kLolDscUnk2,
	kLolDscDoorShapeIndex,
	kLolDscDimData1,
	kLolDscDimData2,
	kLolDscBlockMap,
	kLolDscDimMap,
	kLolDscDoor1,
	kLolDscDoorScale,
	kLolDscDoor4,
	kLolDscDoorX,
	kLolDscDoorY,
	kLolDscOvlIndex,
	kLolDscBlockIndex,

	kLolScrollXTop,
	kLolScrollYTop,
	kLolScrollXBottom,
	kLolScrollYBottom,

	kLolButtonDefs,
	kLolButtonList1,
	kLolButtonList2,
	kLolButtonList3,
	kLolButtonList4,
	kLolButtonList5,
	kLolButtonList6,
	kLolButtonList7,
	kLolButtonList8,

	kLolLegendData,
	kLolMapCursorOvl,
	kLolMapStringId,
	//kLolMapPal,

	kLolSpellbookAnim,
	kLolSpellbookCoords,
	kLolHealShapeFrames,
	kLolLightningDefs,
	kLolFireballCoords,

	kLolHistory,
#endif // ENABLE_LOL

	kMaxResIDs
};

struct Shape;
struct Room;
struct AmigaSfxTable;

class StaticResource {
public:
	static const Common::String staticDataFilename() { return "KYRA.DAT"; }

	StaticResource(KyraEngine_v1 *vm) : _vm(vm), _resList(), _fileLoader(0), _builtIn(0), _filenameTable(0) {}
	~StaticResource() { deinit(); }

	bool loadStaticResourceFile();

	bool init();
	void deinit();

	const char * const *loadStrings(int id, int &strings);
	const uint8 *loadRawData(int id, int &size);
	const Shape *loadShapeTable(int id, int &entries);
	const AmigaSfxTable *loadAmigaSfxTable(int id, int &entries);
	const Room *loadRoomTable(int id, int &entries);
	const uint8 * const *loadPaletteTable(int id, int &entries);
	const HofSeqData *loadHofSequenceData(int id, int &entries);
	const ItemAnimData_v1 *loadShapeAnimData_v1(int id, int &entries);
	const ItemAnimData_v2 *loadShapeAnimData_v2(int id, int &entries);
#ifdef ENABLE_LOL
	const LoLCharacter *loadCharData(int id, int &entries);
	const SpellProperty *loadSpellData(int id, int &entries);
	const CompassDef *loadCompassData(int id, int &entries);
	const FlyingObjectShape *loadFlyingObjectData(int id, int &entries);
	const uint16 *loadRawDataBe16(int id, int &entries);
	const uint32 *loadRawDataBe32(int id, int &entries);
	const ButtonDef *loadButtonDefs(int id, int &entries);
#endif // ENABLE_LOL

	// use '-1' to prefetch/unload all ids
	// prefetchId retruns false if only on of the resources
	// can't be loaded and it breaks then the first res
	// can't be loaded
	bool prefetchId(int id);
	void unloadId(int id);
private:
	bool tryKyraDatLoad();

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
	bool loadAmigaSfxTable(const char *filename, void *&ptr, int &size);
	bool loadRoomTable(const char *filename, void *&ptr, int &size);
	bool loadPaletteTable(const char *filename, void *&ptr, int &size);
	bool loadHofSequenceData(const char *filename, void *&ptr, int &size);
	bool loadShapeAnimData_v1(const char *filename, void *&ptr, int &size);
	bool loadShapeAnimData_v2(const char *filename, void *&ptr, int &size);
#ifdef ENABLE_LOL
	bool loadCharData(const char *filename, void *&ptr, int &size);
	bool loadSpellData(const char *filename, void *&ptr, int &size);
	bool loadCompassData(const char *filename, void *&ptr, int &size);
	bool loadFlyingObjectData(const char *filename, void *&ptr, int &size);
	bool loadRawDataBe16(const char *filename, void *&ptr, int &size);
	bool loadRawDataBe32(const char *filename, void *&ptr, int &size);
	bool loadButtonDefs(const char *filename, void *&ptr, int &size);
#endif // ENABLE_LOL

	void freeRawData(void *&ptr, int &size);
	void freeStringTable(void *&ptr, int &size);
	void freeShapeTable(void *&ptr, int &size);
	void freeAmigaSfxTable(void *&ptr, int &size);
	void freeRoomTable(void *&ptr, int &size);
	void freePaletteTable(void *&ptr, int &size);
	void freeHofSequenceData(void *&ptr, int &size);
	void freeHofShapeAnimDataV1(void *&ptr, int &size);
	void freeHofShapeAnimDataV2(void *&ptr, int &size);
#ifdef ENABLE_LOL
	void freeCharData(void *&ptr, int &size);
	void freeSpellData(void *&ptr, int &size);
	void freeCompassData(void *&ptr, int &size);
	void freeFlyingObjectData(void *&ptr, int &size);
	void freeRawDataBe16(void *&ptr, int &size);
	void freeRawDataBe32(void *&ptr, int &size);
	void freeButtonDefs(void *&ptr, int &size);
#endif // ENABLE_LOL

	const char *getFilename(const char *name);
	Common::SeekableReadStream *getFile(const char *name);

	enum ResTypes {
		kLanguageList,
		kStringList,
		kRoomList,
		kShapeList,
		kRawData,
		kPaletteTable,
		kAmigaSfxTable,

		k2SeqData,
		k2ShpAnimDataV1,
		k2ShpAnimDataV2,

		kLolCharData,
		kLolSpellData,
		kLolCompassData,
		kLolFlightShpData,
		kLolRawDataBe16,
		kLolRawDataBe32,
		kLolButtonData
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

} // End of namespace Kyra

#endif




