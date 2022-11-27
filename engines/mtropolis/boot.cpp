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

#include "common/crc.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/memstream.h"
#include "common/stuffit.h"
#include "common/winexe.h"
#include "common/zlib.h"

#include "graphics/maccursor.h"
#include "graphics/wincursor.h"

#include "mtropolis/boot.h"
#include "mtropolis/detection.h"
#include "mtropolis/runtime.h"
#include "mtropolis/subtitles.h"

#include "mtropolis/plugin/mti.h"
#include "mtropolis/plugin/obsidian.h"
#include "mtropolis/plugin/spqr.h"
#include "mtropolis/plugin/standard.h"
#include "mtropolis/plugins.h"

namespace MTropolis {

namespace Boot {

class GameDataHandler;

struct ManifestSubtitlesDef {
	const char *linesTablePath;
	const char *speakerTablePath;
	const char *assetMappingTablePath;
	const char *modifierMappingTablePath;
};

enum ManifestFileType {
	MTFT_AUTO = 0,       // Automatic, determine based on extension or file type
	MTFT_PLAYER = 1,     // mTropolis Player program
	MTFT_EXTENSION = 2,  // Extension (only use this if the extension contains cursors, otherwise use MTFT_SPECIAL if it has something else useful, or exclude it if not)
	MTFT_MAIN = 3,       // Main segment
	MTFT_ADDITIONAL = 4, // Additional segment
	MTFT_VIDEO = 5,      // External video file
	MTFT_SPECIAL = 6,    // Some other kind of file, or something that might be incorrectly detected as a different type of file (e.g. installers)
};

struct ManifestFile {
	const char *fileName;
	ManifestFileType fileType;
};

struct Game {
	MTropolisGameBootID bootID;
	const ManifestFile *manifest;
	const char **directories;
	const ManifestSubtitlesDef *subtitlesDef;
	GameDataHandler *(*gameDataFactory)(const Boot::Game &game, const MTropolisGameDescription &desc);
};

template<class T>
struct GameDataHandlerFactory {
	static GameDataHandler *create(const Boot::Game &game, const MTropolisGameDescription &desc) {
		return new T(game, desc);
	}
};

struct FileIdentification {
	union Tag {
		uint32 value;
		char debug[4];
	};

	FileIdentification();

	Common::String fileName;
	ManifestFileType category;

	Tag macType;
	Tag macCreator;
	Common::SharedPtr<Common::MacResManager> resMan;
	Common::SharedPtr<Common::SeekableReadStream> stream;
};

FileIdentification::FileIdentification() : category(MTFT_AUTO) {
	macType.value = 0;
	macCreator.value = 0;
}

static void initResManForFile(FileIdentification &f) {
	if (!f.resMan) {
		f.resMan.reset(new Common::MacResManager());
		if (!f.resMan->open(f.fileName))
			error("Failed to open resources of file '%s'", f.fileName.c_str());
	}
}

class GameDataHandler {
public:
	GameDataHandler(const Boot::Game &game, const MTropolisGameDescription &desc);
	virtual ~GameDataHandler();

	virtual void unpackAdditionalFiles(Common::Array<Common::SharedPtr<ProjectPersistentResource> > &persistentResources, Common::Array<FileIdentification> &files);
	virtual void categorizeSpecialFiles(Common::Array<FileIdentification> &files);
	virtual void addPlugIns(ProjectDescription &projectDesc, const Common::Array<FileIdentification> &files);
};

GameDataHandler::GameDataHandler(const Boot::Game &game, const MTropolisGameDescription &desc) {
}

GameDataHandler::~GameDataHandler() {
}

void GameDataHandler::unpackAdditionalFiles(Common::Array<Common::SharedPtr<ProjectPersistentResource> > &persistentResources, Common::Array<FileIdentification> &files) {
}

void GameDataHandler::categorizeSpecialFiles(Common::Array<FileIdentification> &files) {
}

void GameDataHandler::addPlugIns(ProjectDescription &projectDesc, const Common::Array<FileIdentification> &files) {
	Common::SharedPtr<MTropolis::PlugIn> standardPlugIn = PlugIns::createStandard();
	projectDesc.addPlugIn(standardPlugIn);
}

template<class T>
class PersistentResource : public ProjectPersistentResource {
public:
	explicit PersistentResource(const Common::SharedPtr<T> &item);
	const Common::SharedPtr<T> &getItem();

	static Common::SharedPtr<ProjectPersistentResource> wrap(const Common::SharedPtr<T> &item);

private:
	Common::SharedPtr<T> _item;
};

template<class T>
PersistentResource<T>::PersistentResource(const Common::SharedPtr<T> &item) : _item(item) {
}

template<class T>
const Common::SharedPtr<T> &PersistentResource<T>::getItem() {
	return _item;
}

template<class T>
Common::SharedPtr<ProjectPersistentResource> PersistentResource<T>::wrap(const Common::SharedPtr<T> &item) {
	return Common::SharedPtr<ProjectPersistentResource>(new PersistentResource<T>(item));
}

class ObsidianGameDataHandler : public GameDataHandler {
public:
	ObsidianGameDataHandler(const Game &game, const MTropolisGameDescription &gameDesc);

	void unpackAdditionalFiles(Common::Array<Common::SharedPtr<ProjectPersistentResource> > &persistentResources, Common::Array<FileIdentification> &files) override;
	void categorizeSpecialFiles(Common::Array<FileIdentification> &files) override;
	void addPlugIns(ProjectDescription &projectDesc, const Common::Array<FileIdentification> &files) override;

private:
	bool _isMac;
	bool _isRetail;
	bool _isEnglish;

	void unpackMacRetailInstaller(Common::Array<Common::SharedPtr<ProjectPersistentResource> > &persistentResources, Common::Array<FileIdentification> &files);
	Common::SharedPtr<Obsidian::WordGameData> loadWinWordGameData();
	Common::SharedPtr<Obsidian::WordGameData> loadMacWordGameData();

	Common::SharedPtr<Common::Archive> _installerArchive;
};

ObsidianGameDataHandler::ObsidianGameDataHandler(const Game &game, const MTropolisGameDescription &gameDesc) : GameDataHandler(game, gameDesc) {
	_isMac = (gameDesc.desc.platform == Common::kPlatformMacintosh);
	_isEnglish = (gameDesc.desc.language == Common::EN_ANY);
	_isRetail = ((gameDesc.desc.flags & ADGF_DEMO) == 0);
}

struct MacInstallerUnpackRequest {
	const char *fileName;
	uint32 type;
	uint32 creator;
};

struct MacVISE3InstallerUnpackRequest {
	const char *fileName;
	bool extractData;
	bool extractResources;
	ManifestFileType fileType;
};

void ObsidianGameDataHandler::unpackAdditionalFiles(Common::Array<Common::SharedPtr<ProjectPersistentResource> > &persistentResources, Common::Array<FileIdentification> &files) {
	if (_isMac && _isRetail)
		unpackMacRetailInstaller(persistentResources, files);
}

void ObsidianGameDataHandler::unpackMacRetailInstaller(Common::Array<Common::SharedPtr<ProjectPersistentResource> > &persistentResources, Common::Array<FileIdentification> &files) {
	const MacInstallerUnpackRequest requests[] = {
		{"Obsidian", MKTAG('A', 'P', 'P', 'L'), MKTAG('M', 'f', 'P', 'l')},
		{"Basic.rPP", MKTAG('M', 'F', 'X', 'O'), MKTAG('M', 'f', 'P', 'l')},
		{"mCursors.cPP", MKTAG('M', 'F', 'c', 'r'), MKTAG('M', 'f', 'P', 'l')},
		{"Obsidian.cPP", MKTAG('M', 'F', 'c', 'r'), MKTAG('M', 'f', 'M', 'f')},
		{"RSGKit.rPP", MKTAG('M', 'F', 'c', 'o'), MKTAG('M', 'f', 'M', 'f')},
	};

	Common::SharedPtr<Common::MacResManager> installerResMan(new Common::MacResManager());
	persistentResources.push_back(PersistentResource<Common::MacResManager>::wrap(installerResMan));

	if (!installerResMan->open("Obsidian Installer"))
		error("Failed to open Obsidian Installer");

	if (!installerResMan->hasDataFork())
		error("Obsidian Installer has no data fork");

	Common::SeekableReadStream *installerDataForkStream = installerResMan->getDataFork();

	// Not counted/persisted because the StuffIt archive owns the stream.  It will also delete it if createStuffItArchive fails.
	_installerArchive.reset(Common::createStuffItArchive(installerDataForkStream));
	installerDataForkStream = nullptr;

	persistentResources.push_back(PersistentResource<Common::Archive>::wrap(_installerArchive));

	if (!_installerArchive) {

		error("Failed to open Obsidian Installer archive");
	}

	debug(1, "Unpacking resource files...");

	for (const MacInstallerUnpackRequest &request : requests) {
		Common::SharedPtr<Common::MacResManager> resMan(new Common::MacResManager());

		if (!resMan->open(request.fileName, *_installerArchive))
			error("Failed to open file '%s' from installer package", request.fileName);

		FileIdentification ident;
		ident.fileName = request.fileName;
		ident.macCreator.value = request.creator;
		ident.macType.value = request.type;
		ident.resMan = resMan;
		ident.category = MTFT_AUTO;
		files.push_back(ident);
	}

	{
		debug(1, "Unpacking startup segment...");

		Common::SharedPtr<Common::SeekableReadStream> startupStream(_installerArchive->createReadStreamForMember("Obsidian Data 1"));

		FileIdentification ident;
		ident.fileName = "Obsidian Data 1";
		ident.macCreator.value = MKTAG('M', 'f', 'P', 'l');
		ident.macType.value = MKTAG('M', 'F', 'm', 'm');
		ident.category = MTFT_AUTO;
		ident.stream = startupStream;
		files.push_back(ident);
	}
}

void ObsidianGameDataHandler::categorizeSpecialFiles(Common::Array<FileIdentification> &files) {
	// Flag the installer as Special so it doesn't get detected as the player due to being an application
	// Flag RSGKit as Special so it doesn't get fed to the cursor loader
	for (FileIdentification &file : files) {
		if (file.fileName == "Obsidian Installer" || file.fileName == "RSGKit.rPP" || file.fileName == "RSGKit.r95")
			file.category = MTFT_SPECIAL;
	}
}

void ObsidianGameDataHandler::addPlugIns(ProjectDescription &projectDesc, const Common::Array<FileIdentification> &files) {
	Common::SharedPtr<Obsidian::WordGameData> wgData;

	if (_isRetail && _isEnglish) {
		if (_isMac)
			wgData = loadMacWordGameData();
		else
			wgData = loadWinWordGameData();
	}

	Common::SharedPtr<Obsidian::ObsidianPlugIn> obsidianPlugIn(new Obsidian::ObsidianPlugIn(wgData));
	projectDesc.addPlugIn(obsidianPlugIn);

	Common::SharedPtr<MTropolis::PlugIn> standardPlugIn = PlugIns::createStandard();
	static_cast<Standard::StandardPlugIn *>(standardPlugIn.get())->getHacks().allowGarbledListModData = true;
	projectDesc.addPlugIn(standardPlugIn);
}

Common::SharedPtr<Obsidian::WordGameData> ObsidianGameDataHandler::loadMacWordGameData() {
	Common::ArchiveMemberPtr rsgKit = _installerArchive->getMember(Common::Path("RSGKit.rPP"));
	if (!rsgKit)
		error("Couldn't find word game file in installer archive");

	Common::SharedPtr<Obsidian::WordGameData> wgData(new Obsidian::WordGameData());

	Common::SharedPtr<Common::SeekableReadStream> stream(rsgKit->createReadStream());
	if (!stream)
		error("Failed to open word game file");

	Obsidian::WordGameLoadBucket buckets[] = {
		{0, 0},             // 0 letters
		{0xD7C8, 0xD7CC},   // 1 letter
		{0xD7CC, 0xD84D},   // 2 letter
		{0xD84D, 0xE25D},   // 3 letter
		{0x1008C, 0x12AA8}, // 4 letter
		{0x14C58, 0x19614}, // 5 letter
		{0x1C73C, 0x230C1}, // 6 letter
		{0x26D10, 0x2EB98}, // 7 letter
		{0x32ADC, 0x3AA0E}, // 8 letter
		{0x3E298, 0x45B88}, // 9 letter
		{0x48BE8, 0x4E0D0}, // 10 letter
		{0x4FFB0, 0x53460}, // 11 letter
		{0x545F0, 0x56434}, // 12 letter
		{0x56D84, 0x57CF0}, // 13 letter
		{0x58158, 0x58833}, // 14 letter
		{0x58A08, 0x58CD8}, // 15 letter
		{0x58D8C, 0x58EAD}, // 16 letter
		{0x58EF4, 0x58F72}, // 17 letter
		{0x58F90, 0x58FDC}, // 18 letter
		{0, 0},             // 19 letter
		{0x58FEC, 0x59001}, // 20 letter
		{0x59008, 0x59034}, // 21 letter
		{0x5903C, 0x59053}, // 22 letter
	};

	if (!wgData->load(stream.get(), buckets, 23, 1, false))
		error("Failed to load word game data");

	return wgData;
}

Common::SharedPtr<Obsidian::WordGameData> ObsidianGameDataHandler::loadWinWordGameData() {
	Common::File f;
	if (!f.open("RSGKit.r95")) {
		error("Couldn't open word game data file");
		return nullptr;
	}

	Common::SharedPtr<Obsidian::WordGameData> wgData(new Obsidian::WordGameData());

	Obsidian::WordGameLoadBucket buckets[] = {
		{0, 0},             // 0 letters
		{0x63D54, 0x63D5C}, // 1 letter
		{0x63BF8, 0x63CA4}, // 2 letter
		{0x627D8, 0x631E8}, // 3 letter
		{0x5C2C8, 0x60628}, // 4 letter
		{0x52F4C, 0x5919C}, // 5 letter
		{0x47A64, 0x4F2FC}, // 6 letter
		{0x3BC98, 0x43B20}, // 7 letter
		{0x2DA78, 0x38410}, // 8 letter
		{0x218F8, 0x2AA18}, // 9 letter
		{0x19D78, 0x1FA18}, // 10 letter
		{0x15738, 0x18BE8}, // 11 letter
		{0x128A8, 0x14DE8}, // 12 letter
		{0x1129C, 0x1243C}, // 13 letter
		{0x10974, 0x110C4}, // 14 letter
		{0x105EC, 0x108BC}, // 15 letter
		{0x10454, 0x105A8}, // 16 letter
		{0x103A8, 0x10434}, // 17 letter
		{0x10348, 0x10398}, // 18 letter
		{0, 0},             // 19 letter
		{0x10328, 0x10340}, // 20 letter
		{0x102EC, 0x1031C}, // 21 letter
		{0x102D0, 0x102E8}, // 22 letter
	};

	if (!wgData->load(&f, buckets, 23, 4, true)) {
		error("Failed to load word game data file");
		return nullptr;
	}

	return wgData;
}

class MTIGameDataHandler : public GameDataHandler {
public:
	MTIGameDataHandler(const Game &game, const MTropolisGameDescription &gameDesc);

	void addPlugIns(ProjectDescription &projectDesc, const Common::Array<FileIdentification> &files) override;
};

MTIGameDataHandler::MTIGameDataHandler(const Game &game, const MTropolisGameDescription &gameDesc) : GameDataHandler(game, gameDesc) {
}

void MTIGameDataHandler::addPlugIns(ProjectDescription &projectDesc, const Common::Array<FileIdentification> &files) {
	Common::SharedPtr<MTropolis::PlugIn> mtiPlugIn(PlugIns::createMTI());
	projectDesc.addPlugIn(mtiPlugIn);

	Common::SharedPtr<MTropolis::PlugIn> standardPlugIn = PlugIns::createStandard();
	static_cast<Standard::StandardPlugIn *>(standardPlugIn.get())->getHacks().allowGarbledListModData = true;
	projectDesc.addPlugIn(standardPlugIn);
}

class SPQRGameDataHandler : public GameDataHandler {
public:
	SPQRGameDataHandler(const Game &game, const MTropolisGameDescription &gameDesc);

	void addPlugIns(ProjectDescription &projectDesc, const Common::Array<FileIdentification> &files) override;
	void unpackAdditionalFiles(Common::Array<Common::SharedPtr<ProjectPersistentResource> > &persistentResources, Common::Array<FileIdentification> &files) override;

private:
	struct VISE3FileDesc {
		VISE3FileDesc();

		char type[4];
		char creator[4];
		uint32 compressedDataSize;
		uint32 uncompressedDataSize;
		uint32 compressedResSize;
		uint32 uncompressedResSize;
		uint32 positionInArchive;

		Common::String fileName;
	};

	class VISE3ArchiveMember : public Common::ArchiveMember {
	public:
		VISE3ArchiveMember(Common::SeekableReadStream *archiveStream, const VISE3FileDesc *fileDesc, bool isResFork);

		Common::SeekableReadStream *createReadStream() const override;
		Common::String getName() const override;

	private:
		Common::SeekableReadStream *_archiveStream;
		const VISE3FileDesc *_fileDesc;
		bool _isResFork;
	};

	class VISE3Archive : public Common::Archive {
	public:
		explicit VISE3Archive(Common::SeekableReadStream *archiveStream);

		const VISE3FileDesc *getFileDesc(const Common::Path &path) const;

		bool hasFile(const Common::Path &path) const override;
		int listMembers(Common::ArchiveMemberList &list) const override;
		const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
		Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;

	private:
		bool getFileDescIndex(const Common::Path &path, uint &anOutIndex, bool &anOutIsResFork) const;

		Common::SeekableReadStream *_archiveStream;
		Common::Array<VISE3FileDesc> _fileDescs;
	};

	bool _isMac;
};

SPQRGameDataHandler::VISE3ArchiveMember::VISE3ArchiveMember(Common::SeekableReadStream *archiveStream, const SPQRGameDataHandler::VISE3FileDesc *fileDesc, bool isResFork) : _archiveStream(archiveStream), _fileDesc(fileDesc), _isResFork(isResFork) {
}

Common::SeekableReadStream *SPQRGameDataHandler::VISE3ArchiveMember::createReadStream() const {
	static const uint8 vl3DeobfuscationTable[] = {
		0x6a, 0xb7, 0x36, 0xec, 0x15, 0xd9, 0xc8, 0x73, 0xe8, 0x38, 0x9a, 0xdf, 0x21, 0x25, 0xd0, 0xcc,
		0xfd, 0xdc, 0x16, 0xd7, 0xe3, 0x43, 0x05, 0xc5, 0x8f, 0x48, 0xda, 0xf2, 0x3f, 0x10, 0x23, 0x6c,
		0x77, 0x7c, 0xf9, 0xa0, 0xa3, 0xe9, 0xed, 0x46, 0x8b, 0xd8, 0xac, 0x54, 0xce, 0x2d, 0x19, 0x5e,
		0x6d, 0x7d, 0x87, 0x5d, 0xfa, 0x5b, 0x9b, 0xe0, 0xc7, 0xee, 0x9f, 0x52, 0xa9, 0xb9, 0x0a, 0xd1,
		0xfe, 0x78, 0x76, 0x4a, 0x3d, 0x44, 0x5a, 0x96, 0x90, 0x1f, 0x26, 0x9d, 0x58, 0x1b, 0x8e, 0x57,
		0x59, 0xc3, 0x0b, 0x6b, 0xfc, 0x1d, 0xe6, 0xa2, 0x7f, 0x92, 0x4f, 0x40, 0xb4, 0x06, 0x72, 0x4d,
		0xf4, 0x34, 0xaa, 0xd2, 0x49, 0xad, 0xef, 0x22, 0x1a, 0xb5, 0xba, 0xbf, 0x29, 0x68, 0x89, 0x93,
		0x3e, 0x32, 0x04, 0xf5, 0xde, 0xe1, 0x6f, 0xfb, 0x67, 0xe4, 0x7e, 0x08, 0xaf, 0xf0, 0xab, 0x41,
		0x82, 0xea, 0x50, 0x0f, 0x2a, 0xc6, 0x35, 0xb3, 0xa8, 0xca, 0xe5, 0x4c, 0x45, 0x8a, 0x97, 0xae,
		0xd6, 0x66, 0x27, 0x53, 0xc9, 0x1c, 0x3c, 0x03, 0x99, 0xc1, 0x09, 0x2e, 0x69, 0x37, 0x8d, 0x2f,
		0x60, 0xc2, 0xa6, 0x18, 0x4e, 0x7a, 0xb8, 0xcf, 0xa7, 0x3a, 0x17, 0xd5, 0x9e, 0xf1, 0x84, 0x51,
		0x0d, 0xa4, 0x64, 0xc4, 0x1e, 0xb1, 0x30, 0x98, 0xbb, 0x79, 0x01, 0xf6, 0x62, 0x0e, 0xb2, 0x63,
		0x91, 0xcb, 0xff, 0x80, 0x71, 0xe7, 0xd4, 0x00, 0xdb, 0x75, 0x2c, 0xbd, 0x39, 0x33, 0x94, 0xbc,
		0x8c, 0x3b, 0xb6, 0x20, 0x85, 0x24, 0x88, 0x2b, 0x70, 0x83, 0x6e, 0x7b, 0x9c, 0xbe, 0x14, 0x47,
		0x65, 0x4b, 0x56, 0x81, 0xf8, 0x12, 0x11, 0x28, 0xeb, 0x55, 0x74, 0xa1, 0x31, 0xf7, 0xb0, 0x13,
		0x86, 0xdd, 0x5f, 0x42, 0xd3, 0x02, 0x61, 0x95, 0x0c, 0x5c, 0xa5, 0xcd, 0xc0, 0x07, 0xe2, 0xf3,
	};

	uint32 uncompressedSize = _isResFork ? _fileDesc->uncompressedResSize : _fileDesc->uncompressedDataSize;
	uint32 compressedSize = _isResFork ? _fileDesc->compressedResSize : _fileDesc->compressedDataSize;
	uint32 filePosition = _fileDesc->positionInArchive;

	if (_isResFork)
		filePosition += _fileDesc->compressedDataSize;

	if (uncompressedSize == 0)
		return nullptr;

	Common::Array<byte> compressedData;
	compressedData.resize(compressedSize);

	_archiveStream->seek(filePosition, SEEK_SET);
	if (_archiveStream->read(&compressedData[0], compressedSize) != compressedSize)
		return nullptr;

	// Undo byte swapping
	for (uint i = 1; i < compressedSize; i += 2) {
		byte temp = compressedData[i];
		compressedData[i] = compressedData[i - 1];
		compressedData[i - 1] = temp;
	}

	// Undo obfuscation
	for (byte &b : compressedData)
		b = vl3DeobfuscationTable[b];

	byte *decompressedData = static_cast<byte *>(malloc(uncompressedSize));
	if (!decompressedData)
		return nullptr;

	if (!Common::inflateZlibHeaderless(decompressedData, uncompressedSize, &compressedData[0], compressedSize)) {
		free(decompressedData);
		return nullptr;
	}

	return new Common::MemoryReadStream(decompressedData, uncompressedSize, DisposeAfterUse::YES);
}

Common::String SPQRGameDataHandler::VISE3ArchiveMember::getName() const {
	if (_isResFork)
		return _fileDesc->fileName + ".rsrc";
	else
		return _fileDesc->fileName;
}

SPQRGameDataHandler::VISE3FileDesc::VISE3FileDesc() : type{ 0, 0, 0, 0 }, creator{ 0, 0, 0, 0 }, compressedDataSize(0), uncompressedDataSize(0), compressedResSize(0), uncompressedResSize(0), positionInArchive(0) {
}

SPQRGameDataHandler::VISE3Archive::VISE3Archive(Common::SeekableReadStream *archiveStream) : _archiveStream(archiveStream) {
	uint8 vl3Header[44];
	if (archiveStream->read(vl3Header, 44) != 44 || memcmp(vl3Header, "SVCT", 4))
		error("Failed to read VISE 3 header");

	uint32 catalogPosition = READ_BE_UINT32(vl3Header + 36);

	if (!archiveStream->seek(catalogPosition))
		error("Failed to seek to VISE 3 catalog");

	uint8 vl3Catalog[20];
	if (archiveStream->read(vl3Catalog, 20) != 20 || memcmp(vl3Catalog, "CVCT", 4))
		error("Failed to read VISE 3 catalog");

	uint16 numEntries = READ_BE_UINT16(vl3Catalog + 16);

	for (uint16 i = 0; i < numEntries; i++) {
		uint8 entryMagic[4];
		if (archiveStream->read(entryMagic, 4) != 4 || memcmp(entryMagic + 1, "VCT", 3))
			error("Failed to read VISE 3 catalog item");

		if (entryMagic[0] == 'D') {
			uint8 directoryData[78];
			if (archiveStream->read(directoryData, 78) != 78)
				error("Failed to read VISE 3 directory");

			uint8 nameLength = directoryData[76];
			archiveStream->seek(nameLength, SEEK_CUR);
		} else if (entryMagic[0] == 'F') {
			uint8 fileData[120];
			if (archiveStream->read(fileData, 120) != 120)
				error("Failed to read VISE 3 file");

			VISE3FileDesc desc;
			memcpy(desc.type, fileData + 40, 4);
			memcpy(desc.creator, fileData + 44, 4);
			desc.compressedDataSize = READ_BE_UINT32(fileData + 64);
			desc.uncompressedDataSize = READ_BE_UINT32(fileData + 68);
			desc.compressedResSize = READ_BE_UINT32(fileData + 72);
			desc.uncompressedResSize = READ_BE_UINT32(fileData + 76);
			desc.positionInArchive = READ_BE_UINT32(fileData + 96);

			uint8 nameLength = fileData[118];

			if (nameLength > 0) {
				char fileNameChars[256];
				if (archiveStream->read(fileNameChars, nameLength) != nameLength)
					error("Failed to read VISE 3 file name");
				desc.fileName = Common::String(fileNameChars, nameLength);
			}

			_fileDescs.push_back(desc);
		} else {
			error("Unknown VISE 3 catalog entry item type");
		}
	}
}

const SPQRGameDataHandler::VISE3FileDesc *SPQRGameDataHandler::VISE3Archive::getFileDesc(const Common::Path &path) const {
	Common::String convertedPath = path.toString(':');
	for (const VISE3FileDesc &desc : _fileDescs) {
		if (desc.fileName == convertedPath)
			return &desc;
	}

	return nullptr;
}

bool SPQRGameDataHandler::VISE3Archive::hasFile(const Common::Path &path) const {
	uint index = 0;
	bool isResFork = false;
	return getFileDescIndex(path, index, isResFork);
}

int SPQRGameDataHandler::VISE3Archive::listMembers(Common::ArchiveMemberList &list) const {
	int numMembers = 0;
	for (uint fileIndex = 0; fileIndex < _fileDescs.size(); fileIndex++) {
		const VISE3FileDesc &desc = _fileDescs[fileIndex];

		if (desc.uncompressedDataSize) {
			list.push_back(Common::ArchiveMemberPtr(new VISE3ArchiveMember(_archiveStream, &desc, false)));
			numMembers++;
		}
		if (desc.uncompressedResSize) {
			list.push_back(Common::ArchiveMemberPtr(new VISE3ArchiveMember(_archiveStream, &desc, true)));
			numMembers++;
		}
	}
	return numMembers;
}

const Common::ArchiveMemberPtr SPQRGameDataHandler::VISE3Archive::getMember(const Common::Path &path) const {
	uint descIndex = 0;
	bool isResFork = false;
	if (!getFileDescIndex(path, descIndex, isResFork))
		return nullptr;

	return Common::ArchiveMemberPtr(new VISE3ArchiveMember(_archiveStream, &_fileDescs[descIndex], isResFork));
}

Common::SeekableReadStream *SPQRGameDataHandler::VISE3Archive::createReadStreamForMember(const Common::Path &path) const {
	Common::ArchiveMemberPtr archiveMember = getMember(path);
	if (!archiveMember)
		return nullptr;

	return archiveMember->createReadStream();
}

bool SPQRGameDataHandler::VISE3Archive::getFileDescIndex(const Common::Path &path, uint &anOutIndex, bool &anOutIsResFork) const {
	Common::String convertedPath = path.toString(':');
	bool isResFork = false;
	if (convertedPath.hasSuffix(".rsrc")) {
		isResFork = true;
		convertedPath = convertedPath.substr(0, convertedPath.size() - 5);
	}

	for (uint descIndex = 0; descIndex < _fileDescs.size(); descIndex++) {
		const VISE3FileDesc &desc = _fileDescs[descIndex];

		if (desc.fileName == convertedPath) {
			if ((isResFork ? desc.uncompressedResSize : desc.uncompressedDataSize) == 0)
				return false;

			anOutIsResFork = isResFork;
			anOutIndex = descIndex;
			return true;
		}
	}

	return false;
}

SPQRGameDataHandler::SPQRGameDataHandler(const Game &game, const MTropolisGameDescription &gameDesc) : GameDataHandler(game, gameDesc), _isMac(gameDesc.desc.platform == Common::kPlatformMacintosh) {
}

void SPQRGameDataHandler::addPlugIns(ProjectDescription &projectDesc, const Common::Array<FileIdentification> &files) {
	Common::SharedPtr<MTropolis::PlugIn> standardPlugIn = PlugIns::createStandard();
	projectDesc.addPlugIn(standardPlugIn);

	Common::SharedPtr<MTropolis::PlugIn> spqrPlugIn = PlugIns::createSPQR();
	projectDesc.addPlugIn(spqrPlugIn);
}

void SPQRGameDataHandler::unpackAdditionalFiles(Common::Array<Common::SharedPtr<ProjectPersistentResource> > &persistentResources, Common::Array<FileIdentification> &files) {
	if (_isMac) {
		const MacVISE3InstallerUnpackRequest unpackRequests[] = {
			{"Basic.rPP", false, true, MTFT_EXTENSION},
			{"Extras.rPP", false, true, MTFT_EXTENSION},
			{"mCursors.cPP", false, true, MTFT_EXTENSION},
			{"SPQR PPC Start", false, true, MTFT_PLAYER},
			{"Data File SPQR", true, false, MTFT_MAIN},
		};

		Common::SharedPtr<Common::MacResManager> installerResMan(new Common::MacResManager());

		if (!installerResMan->open("Install.vct"))
			error("Failed to open SPQR installer");

		if (!installerResMan->hasDataFork())
			error("SPQR installer has no data fork");

		Common::SharedPtr<Common::SeekableReadStream> installerDataForkStream(installerResMan->getDataFork());

		VISE3Archive archive(installerDataForkStream.get());

		debug(1, "Unpacking files...");

		for (const MacVISE3InstallerUnpackRequest &request : unpackRequests) {
			const VISE3FileDesc *fileDesc = archive.getFileDesc(request.fileName);

			if (!fileDesc)
				error("Couldn't find file '%s' in VISE 3 archive", request.fileName);

			FileIdentification ident;
			ident.fileName = fileDesc->fileName;
			ident.macCreator.value = MKTAG(fileDesc->creator[0], fileDesc->creator[1], fileDesc->creator[2], fileDesc->creator[3]);
			ident.macType.value = MKTAG(fileDesc->type[0], fileDesc->type[1], fileDesc->type[2], fileDesc->type[3]);
			ident.category = request.fileType;

			if (request.extractResources) {
				Common::SharedPtr<Common::MacResManager> resMan(new Common::MacResManager());
				if (!resMan->open(request.fileName, archive))
					error("Failed to open Mac res manager for file '%s'", request.fileName);

				ident.resMan = resMan;
			}

			if (request.extractData)
				ident.stream.reset(archive.createReadStreamForMember(request.fileName));

			files.push_back(ident);
		}
	}
}

class STTGSGameDataHandler : public GameDataHandler {
public:
	STTGSGameDataHandler(const Game &game, const MTropolisGameDescription &gameDesc);

	void addPlugIns(ProjectDescription &projectDesc, const Common::Array<FileIdentification> &files) override;
};

STTGSGameDataHandler::STTGSGameDataHandler(const Game &game, const MTropolisGameDescription &gameDesc) : GameDataHandler(game, gameDesc) {
}

void STTGSGameDataHandler::addPlugIns(ProjectDescription &projectDesc, const Common::Array<FileIdentification> &files) {
	Common::SharedPtr<MTropolis::PlugIn> standardPlugIn = PlugIns::createStandard();
	static_cast<Standard::StandardPlugIn *>(standardPlugIn.get())->getHacks().allowGarbledListModData = true;
	projectDesc.addPlugIn(standardPlugIn);
}

static bool getMacTypesForMacBinary(const char *fileName, uint32 &outType, uint32 &outCreator) {
	Common::SharedPtr<Common::SeekableReadStream> stream(SearchMan.createReadStreamForMember(fileName));

	if (!stream)
		return false;

	byte mbHeader[MBI_INFOHDR];
	if (stream->read(mbHeader, MBI_INFOHDR) != MBI_INFOHDR)
		return false;

	if (mbHeader[0] != 0 || mbHeader[74] != 0)
		return false;

	Common::CRC_BINHEX crc;
	uint16 checkSum = crc.crcFast(mbHeader, 124);

	if (checkSum != READ_BE_UINT16(&mbHeader[124]))
		return false;

	outType = MKTAG(mbHeader[65], mbHeader[66], mbHeader[67], mbHeader[68]);
	outCreator = MKTAG(mbHeader[69], mbHeader[70], mbHeader[71], mbHeader[72]);

	return true;
}

static uint32 getWinFileEndingPseudoTag(const Common::String &fileName) {
	byte bytes[4] = {0, 0, 0, 0};
	size_t numInserts = 4;
	if (fileName.size() < 4)
		numInserts = fileName.size();

	for (size_t i = 0; i < numInserts; i++)
		bytes[i] = static_cast<byte>(invariantToLower(fileName[fileName.size() - numInserts + i]));

	return MKTAG(bytes[0], bytes[1], bytes[2], bytes[3]);
}

static bool getMacTypesForFile(const char *fileName, uint32 &outType, uint32 &outCreator) {
	if (getMacTypesForMacBinary(fileName, outType, outCreator))
		return true;

	return false;
}

static bool fileSortCompare(const FileIdentification &a, const FileIdentification &b) {
	// If file names are mismatched then we want the first one to be shorter
	if (a.fileName.size() > b.fileName.size())
		return !fileSortCompare(b, a);

	size_t aSize = a.fileName.size();
	for (size_t i = 0; i < aSize; i++) {
		char ac = invariantToLower(a.fileName[i]);
		char bc = invariantToLower(b.fileName[i]);

		if (ac < bc)
			return true;
		if (bc < ac)
			return false;
	}

	return aSize < b.fileName.size();
}

static void loadCursorsMac(FileIdentification &f, CursorGraphicCollection &cursorGraphics) {
	initResManForFile(f);

	const uint32 bwType = MKTAG('C', 'U', 'R', 'S');
	const uint32 colorType = MKTAG('c', 'r', 's', 'r');

	Common::MacResIDArray bwIDs = f.resMan->getResIDArray(bwType);
	Common::MacResIDArray colorIDs = f.resMan->getResIDArray(colorType);

	Common::MacResIDArray bwOnlyIDs;
	for (Common::MacResIDArray::const_iterator bwIt = bwIDs.begin(), bwItEnd = bwIDs.end(); bwIt != bwItEnd; ++bwIt) {
		bool hasColor = false;
		for (Common::MacResIDArray::const_iterator colorIt = colorIDs.begin(), colorItEnd = colorIDs.end(); colorIt != colorItEnd; ++colorIt) {
			if ((*colorIt) == (*bwIt)) {
				hasColor = true;
				break;
			}
		}

		if (!hasColor)
			bwOnlyIDs.push_back(*bwIt);
	}

	int numCursorsLoaded = 0;
	for (int cti = 0; cti < 2; cti++) {
		const uint32 resType = (cti == 0) ? bwType : colorType;
		const bool isBW = (cti == 0);
		const Common::MacResIDArray &resArray = (cti == 0) ? bwOnlyIDs : colorIDs;

		for (size_t i = 0; i < resArray.size(); i++) {
			Common::SharedPtr<Common::SeekableReadStream> resData(f.resMan->getResource(resType, resArray[i]));
			if (!resData) {
				warning("Failed to open cursor resource");
				return;
			}

			Common::SharedPtr<Graphics::MacCursor> cursor(new Graphics::MacCursor());
			// Some CURS resources are 72 bytes instead of the expected 68, make sure they load as the correct format
			if (!cursor->readFromStream(*resData, isBW, 0xff, isBW)) {
				warning("Failed to load cursor resource");
				return;
			}

			cursorGraphics.addMacCursor(resArray[i], cursor);
			numCursorsLoaded++;
		}
	}

	if (numCursorsLoaded == 0) {
		// If an extension is in detection, it should either have cursors or be categorized as Special if it has some other use.
		warning("Expected to find cursors in '%s' but there were none.", f.fileName.c_str());
	}
}

static bool loadCursorsWin(FileIdentification &f, CursorGraphicCollection &cursorGraphics) {
	Common::SharedPtr<Common::SeekableReadStream> stream = f.stream;

	if (!stream) {
		Common::SharedPtr<Common::File> file(new Common::File());
		if (!file->open(f.fileName))
			error("Failed to open file '%s'", f.fileName.c_str());

		stream = file;
	}

	Common::SharedPtr<Common::WinResources> winRes(Common::WinResources::createFromEXE(stream.get()));
	if (!winRes) {
		warning("Couldn't load resources from PE file");
		return false;
	}

	int numCursorGroupsLoaded = 0;
	Common::Array<Common::WinResourceID> cursorGroupIDs = winRes->getIDList(Common::kWinGroupCursor);
	for (Common::Array<Common::WinResourceID>::const_iterator it = cursorGroupIDs.begin(), itEnd = cursorGroupIDs.end(); it != itEnd; ++it) {
		const Common::WinResourceID &id = *it;

		Common::SharedPtr<Graphics::WinCursorGroup> cursorGroup(Graphics::WinCursorGroup::createCursorGroup(winRes.get(), *it));
		if (!winRes) {
			warning("Couldn't load cursor group");
			return false;
		}

		if (cursorGroup->cursors.size() == 0) {
			// Empty?
			continue;
		}

		cursorGraphics.addWinCursorGroup(id.getID(), cursorGroup);
		numCursorGroupsLoaded++;
	}

	if (numCursorGroupsLoaded == 0) {
		// If an extension is in detection, it should either have cursors or be categorized as Special if it has some other use.
		warning("Expected to find cursors in '%s' but there were none.", f.fileName.c_str());
	}

	return true;
}

namespace Games {

const ManifestFile obsidianRetailMacEnFiles[] = {
	{"Obsidian Installer", MTFT_SPECIAL},
	{"Obsidian Data 2", MTFT_ADDITIONAL},
	{"Obsidian Data 3", MTFT_ADDITIONAL},
	{"Obsidian Data 4", MTFT_ADDITIONAL},
	{"Obsidian Data 5", MTFT_ADDITIONAL},
	{"Obsidian Data 6", MTFT_ADDITIONAL},
	{nullptr, MTFT_AUTO}
};

const ManifestFile obsidianDemoMacEnFiles[] = {
	{"Obsidian Demo", MTFT_PLAYER},
	{"Basic.rPP", MTFT_EXTENSION},
	{"Experimental.rPP", MTFT_EXTENSION},
	{"Extras.rPP", MTFT_EXTENSION},
	{"mCursors.cPP", MTFT_EXTENSION},
	{"mNet.rPP", MTFT_EXTENSION},
	{"Obsidian.cPP", MTFT_EXTENSION},
	{"RSGKit.rPP", MTFT_SPECIAL},
	{"Obs Demo Large w Sega", MTFT_MAIN},
	{nullptr, MTFT_AUTO}
};

const ManifestFile obsidianRetailWinEnFiles[] = {
	{"Obsidian.exe", MTFT_PLAYER},
	{"Obsidian.c95", MTFT_EXTENSION},
	{"MCURSORS.C95", MTFT_EXTENSION},
	{"RSGKit.r95", MTFT_SPECIAL},
	{"Obsidian Data 1.MPL", MTFT_MAIN},
	{"Obsidian Data 2.MPX", MTFT_ADDITIONAL},
	{"Obsidian Data 3.MPX", MTFT_ADDITIONAL},
	{"Obsidian Data 4.MPX", MTFT_ADDITIONAL},
	{"Obsidian Data 5.MPX", MTFT_ADDITIONAL},
	{"Obsidian Data 6.MPX", MTFT_ADDITIONAL},
	{nullptr, MTFT_AUTO}
};

const ManifestFile obsidianDemoWinEnFiles1[] = {
	{"OBSIDIAN.EXE", MTFT_PLAYER},
	{"OBSIDIAN.R95", MTFT_EXTENSION},
	{"TEXTWORK.R95", MTFT_EXTENSION},
	{"EXPRMNTL.R95", MTFT_EXTENSION},
	{"MCURSORS.C95", MTFT_EXTENSION},
	{"OBSIDIAN DEMO DATA.MPL", MTFT_MAIN},
	{nullptr, MTFT_AUTO}
};

const ManifestFile obsidianDemoWinEnFiles2[] = {
	{"OBSIDIAN.EXE", MTFT_PLAYER},
	{"OBSIDIAN.R95", MTFT_EXTENSION},
	{"TEXTWORK.R95", MTFT_EXTENSION},
	{"EXPRMNTL.R95", MTFT_EXTENSION},
	{"MCURSORS.C95", MTFT_EXTENSION},
	{"OBSIDI~1.MPL", MTFT_MAIN},
	{nullptr, MTFT_AUTO}
};

const ManifestFile obsidianDemoWinEnFiles3[] = {
	{"OBSIDIAN DEMO.EXE", MTFT_PLAYER},
	{"OBSIDIAN1.R95", MTFT_EXTENSION},
	{"OBSIDIAN2.R95", MTFT_EXTENSION},
	{"OBSIDIAN3.R95", MTFT_EXTENSION},
	{"OBSIDIAN4.C95", MTFT_EXTENSION},
	{"OBSIDIAN DEMO DATA.MPL", MTFT_MAIN},
	{nullptr, MTFT_AUTO}
};

const ManifestFile obsidianDemoWinEnFiles4[] = {
	{"OBSIDIAN.EXE", MTFT_PLAYER},
	{"OBSIDIAN.R95", MTFT_EXTENSION},
	{"TEXTWORK.R95", MTFT_EXTENSION},
	{"EXPRMNTL.R95", MTFT_EXTENSION},
	{"MCURSORS.C95", MTFT_EXTENSION},
	{"OBSIDIAN.MPL", MTFT_MAIN},
	{nullptr, MTFT_AUTO}
};

const ManifestFile obsidianDemoWinEnFiles5[] = {
	{"OBSIDI~1.EXE", MTFT_PLAYER},
	{"OBSIDIAN.R95", MTFT_EXTENSION},
	{"TEXTWORK.R95", MTFT_EXTENSION},
	{"EXPRMNTL.R95", MTFT_EXTENSION},
	{"MCURSORS.C95", MTFT_EXTENSION},
	{"OBSIDI~1.MPL", MTFT_MAIN},
	{nullptr, MTFT_AUTO}
};

const ManifestFile obsidianDemoWinEnFiles6[] = {
	{"OBSIDIAN.EXE", MTFT_PLAYER},
	{"OBSIDIAN.R95", MTFT_EXTENSION},
	{"TEXTWORK.R95", MTFT_EXTENSION},
	{"EXPRMNTL.R95", MTFT_EXTENSION},
	{"MCURSORS.C95", MTFT_EXTENSION},
	{"OBSIDIAN DEMO DATA.MPL", MTFT_MAIN},
	{nullptr, MTFT_AUTO}
};

const ManifestFile obsidianDemoWinEnFiles7[] = {
	{"OBSIDIAN DEMO.EXE", MTFT_PLAYER},
	{"OBSIDIAN1.R95", MTFT_EXTENSION},
	{"OBSIDIAN2.R95", MTFT_EXTENSION},
	{"OBSIDIAN3.R95", MTFT_EXTENSION},
	{"OBSIDIAN4.C95", MTFT_EXTENSION},
	{"OBSIDIAN DEMO DATA.MPL", MTFT_MAIN},
	{nullptr, MTFT_AUTO}
};



const ManifestFile obsidianRetailWinDeFiles[] = {
	{"Obsidian.exe", MTFT_PLAYER},
	{"Obsidian.c95", MTFT_EXTENSION},
	{"MCURSORS.C95", MTFT_EXTENSION},
	{"Obsidian Data 1.MPL", MTFT_MAIN},
	{"Obsidian Data 2.MPX", MTFT_ADDITIONAL},
	{"Obsidian Data 3.MPX", MTFT_ADDITIONAL},
	{"Obsidian Data 4.MPX", MTFT_ADDITIONAL},
	{"Obsidian Data 5.MPX", MTFT_ADDITIONAL},
	{"Obsidian Data 6.MPX", MTFT_ADDITIONAL},
	{nullptr, MTFT_AUTO}
};

const ManifestFile obsidianRetailWinItFiles[] = {
	{"Obsidian.exe", MTFT_PLAYER},
	{"Obsidian.c95", MTFT_EXTENSION},
	{"MCURSORS.C95", MTFT_EXTENSION},
	{"RSGKit.r95", MTFT_SPECIAL},
	{"Obsidian Data 1.MPL", MTFT_MAIN},
	{"Obsidian Data 2.MPX", MTFT_ADDITIONAL},
	{"Obsidian Data 3.MPX", MTFT_ADDITIONAL},
	{"Obsidian Data 4.MPX", MTFT_ADDITIONAL},
	{"Obsidian Data 5.MPX", MTFT_ADDITIONAL},
	{"Obsidian Data 6.MPX", MTFT_ADDITIONAL},
	{nullptr, MTFT_AUTO}
};

const char *obsidianRetailWinDirectories[] = {
	"Obsidian",
	"Obsidian/RESOURCE",
	"RESOURCE",
	nullptr
};

const ManifestSubtitlesDef obsidianRetailEnSubtitlesDef = {
	"subtitles_lines_obsidian_en.csv",
	"subtitles_speakers_obsidian_en.csv",
	"subtitles_asset_mapping_obsidian_en.csv",

	// Modifier mapping is the same for both Mac and Win retail, since the MIDI GUIDs are all identical.
	"subtitles_modifier_mapping_obsidian_en.csv"
};

const ManifestFile mtiRetailWinFiles[] = {
	{"MTPLAY32.EXE", MTFT_PLAYER},
	{"GROUP3.R95", MTFT_EXTENSION},
	{"MTIKIT.R95", MTFT_EXTENSION},
	{"MTI1.MPL", MTFT_MAIN},
	{"MTI2.MPX", MTFT_ADDITIONAL},
	{"MTI3.MPX", MTFT_ADDITIONAL},
	{"MTI4.MPX", MTFT_ADDITIONAL},
	{"1.AVI", MTFT_VIDEO},
	{"2.AVI", MTFT_VIDEO},
	{"3.AVI", MTFT_VIDEO},
	{"4.AVI", MTFT_VIDEO},
	{"5.AVI", MTFT_VIDEO},
	{"6.AVI", MTFT_VIDEO},
	{"7.AVI", MTFT_VIDEO},
	{"8.AVI", MTFT_VIDEO},
	{"9.AVI", MTFT_VIDEO},
	{"10.AVI", MTFT_VIDEO},
	{nullptr, MTFT_AUTO}};

const ManifestFile mtiDemoWinFiles[] = {
	{"MTIWIN95.EXE", MTFT_PLAYER},
	{"GROUP3.R95", MTFT_EXTENSION},
	{"MTIKIT.R95", MTFT_EXTENSION},
	{"MUP_DATA.MPL", MTFT_MAIN},
	{nullptr, MTFT_AUTO}
};

const char *mtiRetailWinDirectories[] = {
	"MTPLAY32",
	"MTPLAY32/RESOURCE",
	"VIDEO",
	nullptr
};

const ManifestFile spqrRetailWinEnFiles[] = {
	{"SPQR32.EXE", MTFT_PLAYER},
	{"MCURSORS.C95", MTFT_EXTENSION},
	{"SPQR.MPL", MTFT_MAIN},
	{"S_6842.MPX", MTFT_ADDITIONAL},
	{nullptr, MTFT_AUTO}
};

const char *spqrRetailWinDirectories[] = {
	"RESOURCE",
	nullptr
};

const ManifestFile spqrRetailMacEnFiles[] = {
	{"Install.vct", MTFT_SPECIAL},
	{"S_6772", MTFT_ADDITIONAL},
	{nullptr, MTFT_AUTO}
};

const char *spqrRetailMacDirectories[] = {
	"GAME",
	nullptr
};

const ManifestFile sttgsDemoWinFiles[] = {
	{"MTPLAY95.EXE", MTFT_PLAYER},
	{"Trektriv.mpl", MTFT_MAIN},
	{nullptr, MTFT_AUTO}
};

const Game games[] = {
	// Obsidian - Retail - Macintosh - English
	{
		MTBOOT_OBSIDIAN_RETAIL_MAC_EN,
		obsidianRetailMacEnFiles,
		nullptr,
		&obsidianRetailEnSubtitlesDef,
		GameDataHandlerFactory<ObsidianGameDataHandler>::create
	},
	// Obsidian - Retail - Windows - English
	{
		MTBOOT_OBSIDIAN_RETAIL_WIN_EN,
		obsidianRetailWinEnFiles,
		obsidianRetailWinDirectories,
		&obsidianRetailEnSubtitlesDef,
		GameDataHandlerFactory<ObsidianGameDataHandler>::create
	},
	// Obsidian - Retail - Windows - German
	{
		MTBOOT_OBSIDIAN_RETAIL_WIN_DE,
		obsidianRetailWinDeFiles,
		obsidianRetailWinDirectories,
		nullptr,
		GameDataHandlerFactory<ObsidianGameDataHandler>::create
	},
	// Obsidian - Retail - Windows - Italian
	{
		MTBOOT_OBSIDIAN_RETAIL_WIN_IT,
		obsidianRetailWinItFiles,
		obsidianRetailWinDirectories,
		nullptr,
		GameDataHandlerFactory<ObsidianGameDataHandler>::create
	},
	// Obsidian - Demo - Macintosh - English
	{
		MTBOOT_OBSIDIAN_DEMO_MAC_EN,
		obsidianDemoMacEnFiles,
		nullptr,
		nullptr,
		GameDataHandlerFactory<ObsidianGameDataHandler>::create
	},
	// Obsidian - Demo - Windows - English - Variant 1
	{
		MTBOOT_OBSIDIAN_DEMO_WIN_EN_1,
		obsidianDemoWinEnFiles1,
		nullptr,
		nullptr,
		GameDataHandlerFactory<ObsidianGameDataHandler>::create
	},
	// Obsidian - Demo - Windows - English - Variant 2
	{
		MTBOOT_OBSIDIAN_DEMO_WIN_EN_2,
		obsidianDemoWinEnFiles2,
		nullptr,
		nullptr,
		GameDataHandlerFactory<ObsidianGameDataHandler>::create
	},
	// Obsidian - Demo - Windows - English - Variant 3
	{
		MTBOOT_OBSIDIAN_DEMO_WIN_EN_3,
		obsidianDemoWinEnFiles3,
		nullptr,
		nullptr,
		GameDataHandlerFactory<ObsidianGameDataHandler>::create
	},
	// Obsidian - Demo - Windows - English - Variant 4
	{
		MTBOOT_OBSIDIAN_DEMO_WIN_EN_4,
		obsidianDemoWinEnFiles4,
		nullptr,
		nullptr,
		GameDataHandlerFactory<ObsidianGameDataHandler>::create
	},
	// Obsidian - Demo - Windows - English - Variant 5
	{
		MTBOOT_OBSIDIAN_DEMO_WIN_EN_5,
		obsidianDemoWinEnFiles5,
		nullptr,
		nullptr,
		GameDataHandlerFactory<ObsidianGameDataHandler>::create
	},
	// Obsidian - Demo - Windows - English - Variant 6
	{
		MTBOOT_OBSIDIAN_DEMO_WIN_EN_6,
		obsidianDemoWinEnFiles6,
		nullptr,
		nullptr,
		GameDataHandlerFactory<ObsidianGameDataHandler>::create
	},
	// Obsidian - Demo - Windows - English - Variant 7
	{
		MTBOOT_OBSIDIAN_DEMO_WIN_EN_7,
		obsidianDemoWinEnFiles7,
		nullptr,
		nullptr,
		GameDataHandlerFactory<ObsidianGameDataHandler>::create
	},
	// Muppet Treasure Island - Retail - Windows - Multiple languages
	{
		MTBOOT_MTI_RETAIL_WIN,
		mtiRetailWinFiles,
		mtiRetailWinDirectories,
		nullptr,
		GameDataHandlerFactory<MTIGameDataHandler>::create
	},
	// Muppet Treasure Island - Demo - Windows
	{
		MTBOOT_MTI_DEMO_WIN,
		mtiDemoWinFiles,
		nullptr,
		nullptr,
		GameDataHandlerFactory<MTIGameDataHandler>::create
	},
	// SPQR: The Empire's Darkest Hour - Retail - Windows - English
	{
		MTBOOT_SPQR_RETAIL_WIN,
		spqrRetailWinEnFiles,
		spqrRetailWinDirectories,
		nullptr,
		GameDataHandlerFactory<SPQRGameDataHandler>::create
	},
	// SPQR: The Empire's Darkest Hour - Retail - Macintosh - English
	{
		MTBOOT_SPQR_RETAIL_MAC,
		spqrRetailMacEnFiles,
		spqrRetailMacDirectories,
		nullptr,
		GameDataHandlerFactory<SPQRGameDataHandler>::create
	},
	// Star Trek: The Game Show - Demo - Windows
	{
		MTBOOT_STTGS_DEMO_WIN,
		sttgsDemoWinFiles,
		nullptr,
		nullptr,
		GameDataHandlerFactory<STTGSGameDataHandler>::create
	},
};

} // End of namespace Games

} // End of namespace Boot

Common::SharedPtr<ProjectDescription> bootProject(const MTropolisGameDescription &gameDesc) {
	Common::SharedPtr<ProjectDescription> desc;

	Common::Array<Common::SharedPtr<ProjectPersistentResource>> persistentResources;

	Common::SharedPtr<Boot::GameDataHandler> gameDataHandler;

	Common::SharedPtr<SubtitleAssetMappingTable> subsAssetMappingTable;
	Common::SharedPtr<SubtitleModifierMappingTable> subsModifierMappingTable;
	Common::SharedPtr<SubtitleSpeakerTable> subsSpeakerTable;
	Common::SharedPtr<SubtitleLineTable> subsLineTable;

	Common::String speakerTablePath;
	Common::String linesTablePath;
	Common::String assetMappingTablePath;
	Common::String modifierMappingTablePath;

	const Boot::Game *bootGame = nullptr;
	for (const Boot::Game &bootGameCandidate : Boot::Games::games) {
		if (bootGameCandidate.bootID == gameDesc.bootID) {
			// Multiple manifests should not have the same manifest ID!
			assert(!bootGame);
			bootGame = &bootGameCandidate;
		}
	}

	if (!bootGame)
		error("Couldn't boot mTropolis game, don't have a file manifest for manifest ID %i", static_cast<int>(gameDesc.bootID));

	if (bootGame->gameDataFactory)
		gameDataHandler.reset(bootGame->gameDataFactory(*bootGame, gameDesc));
	else
		gameDataHandler.reset(new Boot::GameDataHandler(*bootGame, gameDesc));


	if (bootGame->subtitlesDef) {
		linesTablePath = bootGame->subtitlesDef->linesTablePath;
		speakerTablePath = bootGame->subtitlesDef->speakerTablePath;
		assetMappingTablePath = bootGame->subtitlesDef->assetMappingTablePath;
		modifierMappingTablePath = bootGame->subtitlesDef->modifierMappingTablePath;
	}

	if (gameDesc.desc.platform == Common::kPlatformMacintosh) {
		Common::Array<Boot::FileIdentification> macFiles;

		debug(1, "Attempting to boot Macintosh game...");

		const Boot::ManifestFile *fileDesc = bootGame->manifest;
		while (fileDesc->fileName) {
			const char *fileName = fileDesc->fileName;

			Boot::FileIdentification ident;
			ident.fileName = fileName;
			ident.category = static_cast<Boot::ManifestFileType>(fileDesc->fileType);
			ident.macType.value = 0;
			ident.macCreator.value = 0;
			if (ident.category == Boot::MTFT_AUTO && !Boot::getMacTypesForFile(fileName, ident.macType.value, ident.macCreator.value))
				error("Couldn't determine Mac file type code for file '%s'", fileName);

			macFiles.push_back(ident);

			fileDesc++;
		}

		gameDataHandler->unpackAdditionalFiles(persistentResources, macFiles);
		gameDataHandler->categorizeSpecialFiles(macFiles);

		Common::sort(macFiles.begin(), macFiles.end(), Boot::fileSortCompare);

		// File types changed in mTropolis 2.0 in a way that MFmx and MFxm have different meaning than mTropolis 1.0.
		// So, we need to detect what variety of files we have available:
		// MT1 Mac: MFmm[+MFmx]
		// MT2 Mac: MFmm[+MFxm]
		// MT2 Cross: MFmx[+MFxx]
		bool haveAnyMFmm = false;
		bool haveAnyMFmx = false;
		//bool haveAnyMFxx = false; // Unused
		bool haveAnyMFxm = false;

		for (Boot::FileIdentification &macFile : macFiles) {
			if (macFile.category == Boot::MTFT_AUTO) {
				switch (macFile.macType.value) {
				case MKTAG('M', 'F', 'm', 'm'):
					haveAnyMFmm = true;
					break;
				case MKTAG('M', 'F', 'm', 'x'):
					haveAnyMFmx = true;
					break;
				case MKTAG('M', 'F', 'x', 'm'):
					haveAnyMFxm = true;
					break;
				case MKTAG('M', 'F', 'x', 'x'):
					//haveAnyMFxx = true; // Unused
					break;
				default:
					break;
				};
			}
		}

		bool isMT2CrossPlatform = (haveAnyMFmx && !haveAnyMFmm);
		if (isMT2CrossPlatform && haveAnyMFxm)
			error("Unexpected combination of player file types");

		// Identify unknown files
		for (Boot::FileIdentification &macFile : macFiles) {
			if (macFile.category == Boot::MTFT_AUTO) {
				switch (macFile.macType.value) {
				case MKTAG('M', 'F', 'm', 'm'):
					macFile.category = Boot::MTFT_MAIN;
					break;
				case MKTAG('M', 'F', 'm', 'x'):
					macFile.category = isMT2CrossPlatform ? Boot::MTFT_MAIN : Boot::MTFT_ADDITIONAL;
					break;
				case MKTAG('M', 'F', 'x', 'm'):
				case MKTAG('M', 'F', 'x', 'x'):
					macFile.category = Boot::MTFT_ADDITIONAL;
					break;
				case MKTAG('A', 'P', 'P', 'L'):
					macFile.category = Boot::MTFT_PLAYER;
					break;
				case MKTAG('M', 'F', 'c', 'o'):
				case MKTAG('M', 'F', 'c', 'r'):
				case MKTAG('M', 'F', 'X', 'O'):
					macFile.category = Boot::MTFT_EXTENSION;
					break;
				default:
					error("Failed to categorize input file '%s'", macFile.fileName.c_str());
					break;
				};
			}
		}

		Boot::FileIdentification *mainSegmentFile = nullptr;
		Common::Array<Boot::FileIdentification *> segmentFiles;

		int addlSegments = 0;

		// Bin segments
		for (Boot::FileIdentification &macFile : macFiles) {
			switch (macFile.category) {
			case Boot::MTFT_PLAYER:
				// Case handled below after cursor loading
				break;
			case Boot::MTFT_EXTENSION:
				// Case handled below after cursor loading
				break;
			case Boot::MTFT_MAIN:
				mainSegmentFile = &macFile;
				break;
			case Boot::MTFT_ADDITIONAL: {
					addlSegments++;
					int segmentID = addlSegments + 1;

					size_t segmentIndex = static_cast<size_t>(segmentID - 1);
					while (segmentFiles.size() <= segmentIndex)
						segmentFiles.push_back(nullptr);
					segmentFiles[segmentIndex] = &macFile;
				} break;
			case Boot::MTFT_VIDEO:
				break;
			case Boot::MTFT_SPECIAL:
				break;
			case Boot::MTFT_AUTO:
				break;
			}
		}

		if (segmentFiles.size() > 0)
			segmentFiles[0] = mainSegmentFile;
		else
			segmentFiles.push_back(mainSegmentFile);

		// Load cursors
		Common::SharedPtr<CursorGraphicCollection> cursorGraphics(new CursorGraphicCollection());

		for (Boot::FileIdentification &macFile : macFiles) {
			if (macFile.category == Boot::MTFT_PLAYER)
				Boot::loadCursorsMac(macFile, *cursorGraphics);
		}

		for (Boot::FileIdentification &macFile : macFiles) {
			if (macFile.category == Boot::MTFT_EXTENSION)
				Boot::loadCursorsMac(macFile, *cursorGraphics);
		}

		// Create the project description
		desc.reset(new ProjectDescription(isMT2CrossPlatform ? KProjectPlatformCrossPlatform : kProjectPlatformMacintosh));

		for (Boot::FileIdentification *segmentFile : segmentFiles) {
			if (!segmentFile)
				error("Missing segment file");

			Common::SharedPtr<Common::SeekableReadStream> dataFork;

			if (segmentFile->stream)
				dataFork = segmentFile->stream;
			else {
				Boot::initResManForFile(*segmentFile);
				dataFork.reset(segmentFile->resMan->getDataFork());
				if (!dataFork)
					error("Segment file '%s' has no data fork", segmentFile->fileName.c_str());

				persistentResources.push_back(Boot::PersistentResource<Common::MacResManager>::wrap(segmentFile->resMan));
			}

			persistentResources.push_back(Boot::PersistentResource<Common::SeekableReadStream>::wrap(dataFork));

			desc->addSegment(0, dataFork.get());
		}

		gameDataHandler->addPlugIns(*desc, macFiles);

		desc->setCursorGraphics(cursorGraphics);
	} else if (gameDesc.desc.platform == Common::kPlatformWindows) {
		Common::Array<Boot::FileIdentification> winFiles;

		debug(1, "Attempting to boot Windows game...");

		const Boot::ManifestFile *fileDesc = bootGame->manifest;
		while (fileDesc->fileName) {
			const char *fileName = fileDesc->fileName;

			Boot::FileIdentification ident;
			ident.fileName = fileName;
			ident.category = fileDesc->fileType;
			ident.macType.value = 0;
			ident.macCreator.value = 0;
			winFiles.push_back(ident);

			fileDesc++;
		}

		gameDataHandler->unpackAdditionalFiles(persistentResources, winFiles);
		gameDataHandler->categorizeSpecialFiles(winFiles);

		Common::sort(winFiles.begin(), winFiles.end(), Boot::fileSortCompare);

		bool isCrossPlatform = false;
		bool isWindows = false;
		bool isMT1 = false;
		bool isMT2 = false;

		// Identify unknown files
		for (Boot::FileIdentification &winFile : winFiles) {
			if (winFile.category == Boot::MTFT_AUTO) {
				switch (Boot::getWinFileEndingPseudoTag(winFile.fileName)) {
				case MKTAG('.', 'm', 'p', 'l'):
					winFile.category = Boot::MTFT_MAIN;
					isWindows = true;
					isMT1 = true;
					if (isMT2)
						error("Unexpected mix of file platforms");
					break;
				case MKTAG('.', 'm', 'p', 'x'):
					winFile.category = Boot::MTFT_ADDITIONAL;
					isWindows = true;
					isMT1 = true;
					if (isMT2)
						error("Unexpected mix of file platforms");
					break;

				case MKTAG('.', 'm', 'f', 'w'):
					winFile.category = Boot::MTFT_MAIN;
					if (isMT1 || isCrossPlatform)
						error("Unexpected mix of file platforms");
					isWindows = true;
					isMT2 = true;
					break;

				case MKTAG('.', 'm', 'x', 'w'):
					winFile.category = Boot::MTFT_ADDITIONAL;
					if (isMT1 || isCrossPlatform)
						error("Unexpected mix of file platforms");
					isWindows = true;
					isMT2 = true;
					break;

				case MKTAG('.', 'm', 'f', 'x'):
					winFile.category = Boot::MTFT_MAIN;
					if (isWindows)
						error("Unexpected mix of file platforms");
					isCrossPlatform = true;
					isMT2 = true;
					break;

				case MKTAG('.', 'm', 'x', 'x'):
					winFile.category = Boot::MTFT_ADDITIONAL;
					if (isWindows)
						error("Unexpected mix of file platforms");
					isCrossPlatform = true;
					isMT2 = true;
					break;

				case MKTAG('.', 'c', '9', '5'):
				case MKTAG('.', 'e', '9', '5'):
				case MKTAG('.', 'r', '9', '5'):
				case MKTAG('.', 'x', '9', '5'):
					winFile.category = Boot::MTFT_EXTENSION;
					break;

				case MKTAG('.', 'e', 'x', 'e'):
					winFile.category = Boot::MTFT_PLAYER;
					break;

				default:
					error("Failed to categorize input file '%s'", winFile.fileName.c_str());
					break;
				};
			}
		}

		Boot::FileIdentification *mainSegmentFile = nullptr;
		Common::Array<Boot::FileIdentification *> segmentFiles;

		int addlSegments = 0;

		// Bin segments
		for (Boot::FileIdentification &winFile : winFiles) {
			switch (winFile.category) {
			case Boot::MTFT_PLAYER:
				// Case handled below after cursor loading
				break;
			case Boot::MTFT_EXTENSION:
				// Case handled below after cursor loading
				break;
			case Boot::MTFT_MAIN:
				mainSegmentFile = &winFile;
				break;
			case Boot::MTFT_ADDITIONAL: {
				addlSegments++;
				int segmentID = addlSegments + 1;

				size_t segmentIndex = static_cast<size_t>(segmentID - 1);
				while (segmentFiles.size() <= segmentIndex)
					segmentFiles.push_back(nullptr);
				segmentFiles[segmentIndex] = &winFile;
			} break;
			case Boot::MTFT_VIDEO:
				break;
			case Boot::MTFT_SPECIAL:
				break;
			case Boot::MTFT_AUTO:
				break;
			}
		}

		if (segmentFiles.size() > 0)
			segmentFiles[0] = mainSegmentFile;
		else
			segmentFiles.push_back(mainSegmentFile);

		// Load cursors
		Common::SharedPtr<CursorGraphicCollection> cursorGraphics(new CursorGraphicCollection());

		for (Boot::FileIdentification &winFile : winFiles) {
			if (winFile.category == Boot::MTFT_PLAYER)
				Boot::loadCursorsWin(winFile, *cursorGraphics);
		}

		for (Boot::FileIdentification &winFile : winFiles) {
			if (winFile.category == Boot::MTFT_EXTENSION)
				Boot::loadCursorsWin(winFile, *cursorGraphics);
		}

		// Create the project description
		desc.reset(new ProjectDescription(isCrossPlatform ? KProjectPlatformCrossPlatform : kProjectPlatformWindows));

		for (Boot::FileIdentification *segmentFile : segmentFiles) {
			if (!segmentFile)
				error("Missing segment file");

			if (segmentFile->stream) {
				persistentResources.push_back(Boot::PersistentResource<Common::SeekableReadStream>::wrap(segmentFile->stream));
				desc->addSegment(0, segmentFile->stream.get());
			} else {
				desc->addSegment(0, segmentFile->fileName.c_str());
			}
		}

		gameDataHandler->addPlugIns(*desc, winFiles);

		desc->setCursorGraphics(cursorGraphics);
	}

	Common::SharedPtr<ProjectResources> resources(new ProjectResources());
	resources->persistentResources = persistentResources;

	desc->setResources(resources);

	if (assetMappingTablePath.size() > 0 && linesTablePath.size() > 0) {
		subsAssetMappingTable.reset(new SubtitleAssetMappingTable());
		subsModifierMappingTable.reset(new SubtitleModifierMappingTable());
		subsSpeakerTable.reset(new SubtitleSpeakerTable());
		subsLineTable.reset(new SubtitleLineTable());

		Common::ErrorCode assetMappingError = subsAssetMappingTable->load(assetMappingTablePath);
		Common::ErrorCode modifierMappingError = subsModifierMappingTable->load(modifierMappingTablePath);
		Common::ErrorCode speakerError = subsSpeakerTable->load(speakerTablePath);
		Common::ErrorCode linesError = speakerError;

		if (speakerError == Common::kNoError)
			linesError = subsLineTable->load(linesTablePath, *subsSpeakerTable);

		if (assetMappingError != Common::kNoError || modifierMappingError != Common::kNoError || linesError != Common::kNoError) {
			// If all sub files are missing, then the user hasn't installed them
			if (assetMappingError != Common::kPathDoesNotExist || modifierMappingError != Common::kPathDoesNotExist || linesError != Common::kPathDoesNotExist) {
				warning("Failed to load subtitles data");
			}

			subsAssetMappingTable.reset();
			subsModifierMappingTable.reset();
			subsLineTable.reset();
			subsSpeakerTable.reset();
		}
	}

	SubtitleTables subTables;
	subTables.assetMapping = subsAssetMappingTable;
	subTables.lines = subsLineTable;
	subTables.modifierMapping = subsModifierMappingTable;
	subTables.speakers = subsSpeakerTable;

	desc->getSubtitles(subTables);

	return desc;
}

void bootAddSearchPaths(const Common::FSNode &gameDataDir, const MTropolisGameDescription &gameDesc) {

	const Boot::Game *bootGame = nullptr;
	for (const Boot::Game &bootGameCandidate : Boot::Games::games) {
		if (bootGameCandidate.bootID == gameDesc.bootID) {
			// Multiple manifests should not have the same manifest ID!
			assert(!bootGame);
			bootGame = &bootGameCandidate;
		}
	}

	if (!bootGame)
		error("Couldn't boot mTropolis game, don't have a file manifest for manifest ID %i", static_cast<int>(gameDesc.bootID));

	if (!bootGame->directories)
		return;

	size_t index = 0;
	while (bootGame->directories[index]) {
		const char *directoryPath = bootGame->directories[index++];

		SearchMan.addSubDirectoryMatching(gameDataDir, directoryPath);
	}
}

} // End of namespace MTropolis
