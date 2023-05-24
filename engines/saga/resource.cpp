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

// RSC Resource file management module

#include "saga/saga.h"

#include "saga/actor.h"
#include "saga/animation.h"
#include "saga/interface.h"
#include "saga/music.h"
#include "saga/resource.h"
#include "saga/scene.h"
#include "saga/sndres.h"

#include "engines/advancedDetector.h"
#include "common/compression/powerpacker.h"

namespace Saga {

// Patch files. Files not found will be ignored
static const GamePatchDescription ITEPatch_Files[] = {
	{       "cave.mid", GAME_RESOURCEFILE,    9},
	{      "intro.mid", GAME_RESOURCEFILE,   10},
	{   "fvillage.mid", GAME_RESOURCEFILE,   11},
	{    "elkhall.mid", GAME_RESOURCEFILE,   12},
	{      "mouse.mid", GAME_RESOURCEFILE,   13},
	{   "darkclaw.mid", GAME_RESOURCEFILE,   14},
	{   "birdchrp.mid", GAME_RESOURCEFILE,   15},
	{   "orbtempl.mid", GAME_RESOURCEFILE,   16},
	{     "spooky.mid", GAME_RESOURCEFILE,   17},
	{    "catfest.mid", GAME_RESOURCEFILE,   18},
	{ "elkfanfare.mid", GAME_RESOURCEFILE,   19},
	{     "bcexpl.mid", GAME_RESOURCEFILE,   20},
	{   "boargtnt.mid", GAME_RESOURCEFILE,   21},
	{   "boarking.mid", GAME_RESOURCEFILE,   22},
	{   "explorea.mid", GAME_RESOURCEFILE,   23},
	{   "exploreb.mid", GAME_RESOURCEFILE,   24},
	{   "explorec.mid", GAME_RESOURCEFILE,   25},
	{   "sunstatm.mid", GAME_RESOURCEFILE,   26},
	{   "nitstrlm.mid", GAME_RESOURCEFILE,   27},
	{   "humruinm.mid", GAME_RESOURCEFILE,   28},
	{   "damexplm.mid", GAME_RESOURCEFILE,   29},
	{     "tychom.mid", GAME_RESOURCEFILE,   30},
	{     "kitten.mid", GAME_RESOURCEFILE,   31},
	{      "sweet.mid", GAME_RESOURCEFILE,   32},
	{   "brutalmt.mid", GAME_RESOURCEFILE,   33},
	{     "shiala.mid", GAME_RESOURCEFILE,   34},

	{       "wyrm.pak", GAME_RESOURCEFILE, 1529},
	{      "wyrm1.dlt", GAME_RESOURCEFILE, 1530},
	{      "wyrm2.dlt", GAME_RESOURCEFILE, 1531},
	{      "wyrm3.dlt", GAME_RESOURCEFILE, 1532},
	{      "wyrm4.dlt", GAME_RESOURCEFILE, 1533},
	{   "credit3n.dlt", GAME_RESOURCEFILE, 1796}, // PC
	{   "credit3m.dlt", GAME_RESOURCEFILE, 1796}, // Macintosh
	{   "credit4n.dlt", GAME_RESOURCEFILE, 1797}, // PC
	{   "credit4m.dlt", GAME_RESOURCEFILE, 1797}, // Macintosh
	{       "p2_a.voc", GAME_VOICEFILE,       4},
	{       "p2_a.iaf", GAME_VOICEFILE,       4},
	{             NULL,              0,       0}
};

static const GamePatchDescription ITEMacPatch_Files[] = {
	{       "wyrm.pak", GAME_RESOURCEFILE, 1529},
	{      "wyrm1.dlt", GAME_RESOURCEFILE, 1530},
	{      "wyrm2.dlt", GAME_RESOURCEFILE, 1531},
	{      "wyrm3.dlt", GAME_RESOURCEFILE, 1532},
	{      "wyrm4.dlt", GAME_RESOURCEFILE, 1533},
	{   "credit3m.dlt", GAME_RESOURCEFILE, 1796},
	{   "credit4m.dlt", GAME_RESOURCEFILE, 1797},
	{       "p2_a.iaf", GAME_VOICEFILE,       4},
	{             NULL,              0,       0}
};

static const GamePatchDescription *PatchLists[PATCHLIST_MAX] = {
	/* PATCHLIST_NONE */    nullptr,
	/* PATCHLIST_ITE */     ITEPatch_Files,
	/* PATCHLIST_ITE_MAC */ ITEMacPatch_Files
};

struct ITEAmigaIndex {
	uint32 fileOffset;
	uint32 numEntries;
};

struct ITEAmigaEXEDescriptor {
	ITEAmigaIndex voiceIndex;
	ITEAmigaIndex soundIndex;
};

bool ResourceContext::loadResIteAmigaSound(SagaEngine *_vm, int type) {
	Common::String exeName;

	for (const ADGameFileDescription *gameFileDescription = _vm->getFilesDescriptions();
		gameFileDescription->fileName; gameFileDescription++) {
		if (Common::String(gameFileDescription->fileName).hasSuffix(".exe"))
			exeName = gameFileDescription->fileName;
	}

	if (exeName.empty())
		return false;

	// Right now German and English ECS version have same offsets to
	// offset tables, no need to distinguish them
	static const ITEAmigaEXEDescriptor ecsDesc = {
		{ 0x56a8, 3730 },
		{ 0x90f0,   44 },
	};

	// Right now German and English ECS version have same offsets to
	// offset tables, no need to distinguish them
	static const ITEAmigaEXEDescriptor agaDesc = {
		{ 0x53a8, 3730 },
		{ 0x8df0,   44 },
	};
 
	const ITEAmigaEXEDescriptor *exedesc = _vm->isECS() ? &ecsDesc : &agaDesc;
	const ITEAmigaIndex& amigaIdx = _fileType & GAME_VOICEFILE ? exedesc->voiceIndex : exedesc->soundIndex;

	if (amigaIdx.numEntries <= 1)
		return false;

	_table.resize(amigaIdx.numEntries - 1);

	Common::File f;

	if(!f.open(exeName.c_str()))
		return false;

	f.seek(amigaIdx.fileOffset);

	for (uint32 i = 0; i < amigaIdx.numEntries - 1; i++) {
		ResourceData *resourceData = &_table[i];
		resourceData->offset = f.readUint32BE();
		resourceData->diskNum = -1;
	}

	uint32 lastEntry = f.readUint32BE();

	for (uint32 i = 0; i < amigaIdx.numEntries - 2; i++) {
		_table[i].size = _table[i + 1].offset - _table[i].offset;
	}

	_table[amigaIdx.numEntries - 2].size = lastEntry - _table[amigaIdx.numEntries - 2].offset;

	return true;
}

bool ResourceContext::loadResIteAmiga(SagaEngine *_vm, int type, bool isFloppy) {
	if (_fileType & (GAME_VOICEFILE | GAME_SOUNDFILE))
		return loadResIteAmigaSound(_vm, type);
	_file->seek(0);
	uint16 resourceCount = _file->readUint16BE();
	uint16 scriptCount = _file->readUint16BE();
	uint32 count = (type &  GAME_SCRIPTFILE) ? scriptCount : resourceCount;
	uint32 extraOffset = isFloppy ? 1024 : 0;

	if (type &  GAME_SCRIPTFILE)
		_file->seek(resourceCount * 10, SEEK_CUR);

	_table.resize(count);

	for (uint32 i = 0; i < count; i++) {
		ResourceData *resourceData = &_table[i];
		resourceData->offset = _file->readUint32BE() + extraOffset;
		resourceData->size = _file->readUint32BE();
		resourceData->diskNum = _file->readUint16BE();
	}

	return true;
}

bool ResourceContext::loadResV1() {
	size_t i;
	bool result;
	byte tableInfo[RSC_TABLEINFO_SIZE];
	ByteArray tableBuffer;
	uint32 count;
	uint32 resourceTableOffset;
	ResourceData *resourceData;

	if (_fileSize < RSC_MIN_FILESIZE) {
		warning("ResourceContext::loadResV1(): Incorrect contextSize: %d < %d", (int) _fileSize, RSC_MIN_FILESIZE);
		return false;
	}

	_file->seek(-RSC_TABLEINFO_SIZE, SEEK_END);

	if (_file->read(tableInfo, RSC_TABLEINFO_SIZE) != RSC_TABLEINFO_SIZE) {
		warning("ResourceContext::loadResV1(): Incorrect table size: %d for %s", RSC_TABLEINFO_SIZE, _fileName);
		return false;
	}

	Common::MemoryReadStreamEndian readS(tableInfo, RSC_TABLEINFO_SIZE, _isBigEndian);

	resourceTableOffset = readS.readUint32();
	count = readS.readUint32();

	// Check for sane table offset
	if (resourceTableOffset != _fileSize - RSC_TABLEINFO_SIZE - RSC_TABLEENTRY_SIZE * count) {
		warning("ResourceContext::loadResV1(): Incorrect tables offset: %d != %d for %s, endian is %d",
			resourceTableOffset, (int)_fileSize - RSC_TABLEINFO_SIZE - RSC_TABLEENTRY_SIZE * count,
			_fileName, _isBigEndian);
		return false;
	}

	// Load resource table
	tableBuffer.resize(RSC_TABLEENTRY_SIZE * count);

	_file->seek(resourceTableOffset, SEEK_SET);

	result = (_file->read(tableBuffer.getBuffer(), tableBuffer.size()) == tableBuffer.size());
	if (result) {
		_table.resize(count);

		Common::MemoryReadStreamEndian readS1(tableBuffer.getBuffer(), tableBuffer.size(), _isBigEndian);

		for (i = 0; i < count; i++) {
			resourceData = &_table[i];
			resourceData->offset = readS1.readUint32();
			resourceData->size = readS1.readUint32();
			// Sanity check
			if ((resourceData->offset > (uint)_fileSize) || (resourceData->size > (uint)_fileSize)) {
				result = false;
				break;
			}
		}
	}

	return result;
}

bool ResourceContext::load(SagaEngine *vm, Resource *resource) {
	if (_fileName == nullptr) // IHNM special case
		return true;

	_file.reset(Common::MacResManager::openFileOrDataFork(_fileName));
	if (!_file)
		return false;

	_fileSize = _file->size();
	_isBigEndian = vm->isBigEndian();

	if (_fileType & GAME_SWAPENDIAN)
		_isBigEndian = !_isBigEndian;

	if ((_fileType & (GAME_MACBINARY | GAME_MUSICFILE_GM)) == (GAME_MACBINARY | GAME_MUSICFILE_GM)) {
		_macRes.reset(new Common::MacResManager());
		if (!_macRes->open(_fileName))
			return false;
		// Unpacking MacBinary packed MIDI files happens on-demand
		return true;
	}


	if (!loadRes(vm, _fileType))
		return false;

	GamePatchList index = vm->getPatchList();
	if (index < PATCHLIST_MAX && index > PATCHLIST_NONE)
		processPatches(resource, PatchLists[index]);

	// Close the file if it's part of a series of files.
	// This prevents having all voice files open in IHNM for no reason, as each chapter uses
	// a different voice file.
	if (_serial > 0)
		closeFile();

	return true;
}

Resource::Resource(SagaEngine *vm): _vm(vm) {
}

Resource::~Resource() {
	clearContexts();
}

void Resource::addContext(const char *fileName, uint16 fileType, bool isCompressed, int serial) {
	ResourceContext *context;
	context = createContext();
	context->_fileName = fileName;
	context->_fileType = fileType;
	context->_isCompressed = isCompressed;
	context->_serial = serial;
	_contexts.push_back(context);
}

bool Resource::createContexts() {
	bool soundFileInArray = false;
	bool voiceFileInArray = false;

	_vm->_voiceFilesExist = true;

	struct SoundFileInfo {
		int gameId;
		char fileName[40];
		bool isCompressed;
		uint16 voiceFileAddType;
	};

	for (const ADGameFileDescription *gameFileDescription = _vm->getFilesDescriptions();
		gameFileDescription->fileName; gameFileDescription++) {
		if (gameFileDescription->fileType > 0)
			addContext(gameFileDescription->fileName, gameFileDescription->fileType);
		if ((gameFileDescription->fileType & GAME_RESOURCEFILE) && _vm->getPlatform() == Common::kPlatformAmiga && _vm->getGameId() == GID_ITE)
			addContext(gameFileDescription->fileName, (gameFileDescription->fileType & ~GAME_RESOURCEFILE) | GAME_SCRIPTFILE | GAME_SWAPENDIAN);
		if ((gameFileDescription->fileType & GAME_RESOURCEFILE) && _vm->getPlatform() == Common::kPlatformAmiga && _vm->getGameId() == GID_ITE)
			addContext(gameFileDescription->fileName, (gameFileDescription->fileType & ~GAME_RESOURCEFILE) | GAME_MUSICFILE_FM);
		if (gameFileDescription->fileType == GAME_SOUNDFILE) {
			soundFileInArray = true;
		}
		if (gameFileDescription->fileType == GAME_VOICEFILE) {
			voiceFileInArray = true;
		}
	}

	//// Detect and add SFX files ////////////////////////////////////////////////
	SoundFileInfo sfxFiles[] = {
		{	GID_ITE,	"sounds.rsc",		false,	0	},
		{	GID_ITE,	"sounds.cmp",		true,	0	},
		{	GID_ITE,	"soundsd.rsc",		false,	0	},
		{	GID_ITE,	"soundsd.cmp",		true,	0	},
#ifdef ENABLE_IHNM
		{	GID_IHNM,	"sfx.res",			false,	0	},
		{	GID_IHNM,	"sfx.cmp",			true,	0	},
#endif
		{	-1,			"",				false,	0	}
	};

	_soundFileName[0] = 0;
	if (!soundFileInArray) {
		for (SoundFileInfo *curSoundFile = sfxFiles; (curSoundFile->gameId != -1); curSoundFile++) {
			if (curSoundFile->gameId != _vm->getGameId()) continue;
			if (!Common::File::exists(curSoundFile->fileName)) continue;
			Common::strcpy_s(_soundFileName, curSoundFile->fileName);
			uint32 flags = GAME_SOUNDFILE;

			if (_vm->getFeatures() & GF_SOME_MAC_RESOURCES)
				flags |= GAME_SWAPENDIAN;
			addContext(_soundFileName, flags, curSoundFile->isCompressed);
			break;
		}
	}

	//// Detect and add voice files /////////////////////////////////////////////
	SoundFileInfo voiceFiles[] = {
		{	GID_ITE,	"voices.rsc",					false	,	(uint16)((_soundFileName[0] == 0) ? GAME_SOUNDFILE : 0)},
		{	GID_ITE,	"voices.cmp",					true	,	(uint16)((_soundFileName[0] == 0) ? GAME_SOUNDFILE : 0)},
		{	GID_ITE,	"voicesd.rsc",					false	,	(uint16)((_soundFileName[0] == 0) ? GAME_SOUNDFILE : 0)},
		{	GID_ITE,	"voicesd.cmp",					true	,	(uint16)((_soundFileName[0] == 0) ? GAME_SOUNDFILE : 0)},
		// The resources in the Wyrmkeep combined Windows/Mac/Linux CD version are little endian, but
		// the voice file is big endian. If we got such a version with mixed files, mark this voice file
		// as big endian
		{	GID_ITE,	"inherit the earth voices",		false	,	(uint16)(_vm->isBigEndian() ? 0 : GAME_SWAPENDIAN)},
		{	GID_ITE,	"inherit the earth voices.cmp",	true	,	(uint16)(_vm->isBigEndian() ? 0 : GAME_SWAPENDIAN)},
		{	GID_ITE,	"ite voices",				false	,	GAME_MACBINARY},
#ifdef ENABLE_IHNM
		{	GID_IHNM,	"voicess.res",					false	,	0},
		{	GID_IHNM,	"voicess.cmp",					true	,	0},
		{	GID_IHNM,	"voicesd.res",					false	,	0},
		{	GID_IHNM,	"voicesd.cmp",					true	,	0},
#endif
		{	-1,			"",							false	,	0}
	};

	// Detect and add voice files
	_voicesFileName[0][0] = 0;
	if (!voiceFileInArray) {
		for (SoundFileInfo *curSoundFile = voiceFiles; (curSoundFile->gameId != -1); curSoundFile++) {
			if (curSoundFile->gameId != _vm->getGameId()) continue;
			bool exists = curSoundFile->voiceFileAddType & GAME_MACBINARY ? Common::MacResManager::exists(curSoundFile->fileName) : Common::File::exists(curSoundFile->fileName);
			if (!exists) continue;

			Common::strcpy_s(_voicesFileName[0], curSoundFile->fileName);
			addContext(_voicesFileName[0], GAME_VOICEFILE | curSoundFile->voiceFileAddType, curSoundFile->isCompressed);

			// Special cases
			if (!scumm_stricmp(curSoundFile->fileName, "voicess.res") ||
			    !scumm_stricmp(curSoundFile->fileName, "voicess.cmp")) {
				// IHNM has multiple voice files
				for (size_t i = 1; i <= 6; i++) { // voices1-voices6
					Common::sprintf_s(_voicesFileName[i], "voices%i.%s", (uint)i, curSoundFile->isCompressed ? "cmp" : "res");
					if (i == 4) {
						// The German and French versions of IHNM don't have Nimdok's chapter,
						// therefore the voices file for that chapter is missing
						if (!Common::File::exists(_voicesFileName[i])) {
							continue;
						}
					}
					addContext(_voicesFileName[i], GAME_VOICEFILE, curSoundFile->isCompressed, i);
				}
			}
			break;
		}
	}

	if (!voiceFileInArray && _voicesFileName[0][0] == 0) {
#ifdef ENABLE_IHNM
		if (_vm->getGameId() == GID_IHNM && _vm->isMacResources()) {
			// The Macintosh version of IHNM has no voices.res, and it has all
			// its voice files in subdirectories, so don't do anything here
			_contexts.push_back(new VoiceResourceContext_RES());
		} else {
#endif
			warning("No voice file found, voices will be disabled");
			_vm->_voicesEnabled = false;
			_vm->_subtitlesEnabled = true;
			_vm->_voiceFilesExist = false;
#ifdef ENABLE_IHNM
		}
#endif
	}

	//// Detect and add music files /////////////////////////////////////////
	SoundFileInfo musicFiles[] = {
		{	GID_ITE,	"music.rsc",	false,	0	},
		{	GID_ITE,	"music.cmp",	true,	0	},
		{	GID_ITE,	"musicd.rsc",	false,	0	},
		{	GID_ITE,	"musicd.cmp",	true,	0	},
		{	-1,			"",			false	,	0}
	};

	// Check for digital music in ITE

	for (SoundFileInfo *curSoundFile = musicFiles; (curSoundFile->gameId != -1); curSoundFile++) {
		if (curSoundFile->gameId != _vm->getGameId()) continue;
		if (!Common::File::exists(curSoundFile->fileName)) continue;
		Common::strcpy_s(_musicFileName, curSoundFile->fileName);
		uint32 flags = GAME_DIGITALMUSICFILE;

		if (_vm->getFeatures() & GF_SOME_MAC_RESOURCES)
			flags |= GAME_SWAPENDIAN;
		addContext(_musicFileName, flags, curSoundFile->isCompressed);
		break;
	}

	for (ResourceContextList::iterator i = _contexts.begin(); i != _contexts.end(); ++i) {
		if (!(*i)->load(_vm, this)) {
			warning("Cannot load context %s", (*i)->_fileName);
			return false;
		}
	}
	return true;
}

void Resource::clearContexts() {
	ResourceContextList::iterator i = _contexts.begin();
	while (i != _contexts.end()) {
		ResourceContext * context = *i;
		i = _contexts.erase(i);
		delete context;
	}
}

#define ID_MIDI     MKTAG('M','i','d','i')

void Resource::loadResource(ResourceContext *context, uint32 resourceId, ByteArray &resourceBuffer) {
	if ((context->_fileType & (GAME_MACBINARY | GAME_MUSICFILE_GM)) == (GAME_MACBINARY | GAME_MUSICFILE_GM) && context->_macRes) {
		Common::SeekableReadStream *s = context->_macRes->getResource(ID_MIDI, resourceId);
		if (!s)
			return;
		resourceBuffer.resize(s->size());
		s->read(resourceBuffer.getBuffer(), s->size());

		delete s;
		
		return;
	}

	ResourceData *resourceData = context->getResourceData(resourceId);
	Common::SeekableReadStream *file = nullptr;
	uint32 resourceOffset = resourceData->offset;
	Common::File actualFile;

	if (resourceData->diskNum == -1)
		file = context->getFile(resourceData);
	else {
		Common::String fileName = context->_fileName;
		int sz = fileName.size();
		while(sz > 0 && fileName[sz - 1] != '.')
			sz--;
		if (sz > 0)
			sz--;
		if (_vm->getFeatures() & GF_ITE_FLOPPY)
			fileName = Common::String::format("%s%02d.adf", fileName.substr(0, sz).c_str(), resourceData->diskNum + 1);
		else
			fileName = Common::String::format("%s.%03d", fileName.substr(0, sz).c_str(), resourceData->diskNum);
		if (!actualFile.open(fileName))
			error("Resource::loadResource() failed to open %s", fileName.c_str());
		file = &actualFile;
	}

	debug(8, "loadResource %d 0x%X:0x%X", resourceId, resourceOffset, uint(resourceData->size));
	resourceBuffer.resize(resourceData->size);

	file->seek((long)resourceOffset, SEEK_SET);

	if (file->read(resourceBuffer.getBuffer(), resourceBuffer.size()) != resourceBuffer.size()) {
		error("Resource::loadResource() failed to read");
	}

	if (_vm->getPlatform() == Common::Platform::kPlatformAmiga &&
	    resourceBuffer.size() >= 16 && READ_BE_UINT32(resourceBuffer.getBuffer()) == MKTAG('H', 'E', 'A', 'D')
	    && READ_BE_UINT32(resourceBuffer.getBuffer() + 12) == MKTAG('P', 'A', 'C', 'K')) {
		uint32 unpackedLen = READ_BE_UINT32(resourceBuffer.getBuffer() + 4);
		uint32 packedLen = READ_BE_UINT32(resourceBuffer.getBuffer() + 8);
		uint32 actualUncompressedLen = 0;
		if (packedLen != resourceBuffer.size() - 20) {
			warning("Compressed size mismatch in resource %d: %d vs %d", resourceId, packedLen, resourceBuffer.size() - 20);
		}
		byte *uncompressed = Common::PowerPackerStream::unpackBuffer(resourceBuffer.getBuffer() + 12, packedLen + 8, actualUncompressedLen);
		if (uncompressed == nullptr || unpackedLen != actualUncompressedLen) {
			warning("Uncompressed size mismatch in resource %d: %d vs %d", resourceId, unpackedLen, actualUncompressedLen);
		}

		if (context->fileType() & GAME_MUSICFILE_FM) {
			byte b = 0;
			for (uint32 i = 0; i < unpackedLen; i++) {
				b += uncompressed[i];
				uncompressed[i] = b;
			}
		}

		// TODO: Use move semantics
		resourceBuffer = ByteArray(uncompressed, actualUncompressedLen);
		delete[] uncompressed;
	}
}

ResourceContext *Resource::getContext(uint16 fileType, int serial) {
	for (ResourceContextList::const_iterator i = _contexts.begin(); i != _contexts.end(); ++i) {
		ResourceContext * context = *i;
		if ((context->fileType() & fileType) && (context->serial() == serial)) {
			return context;
		}
	}
	return nullptr;
}

} // End of namespace Saga
