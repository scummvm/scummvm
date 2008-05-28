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

// RSC Resource file management module

#include "saga/saga.h"

#include "saga/actor.h"
#include "saga/animation.h"
#include "saga/interface.h"
#include "saga/music.h"
#include "saga/rscfile.h"
#include "saga/scene.h"
#include "saga/sndres.h"

#include "common/advancedDetector.h"

namespace Saga {

struct MacResMap {
	int16 resAttr;
	int16 typeOffset;
	int16 nameOffset;
	int16 numTypes;
};

struct MacResource {
	int16 id;
	int16 nameOffset;
	byte attr;
	int32 dataOffset;
	byte name[255];
};

struct MacResType {
	uint32 id;
	int16 items;
	int16 maxItemId;
	int16 offset;
	MacResource *resources;
};


#define ID_MIDI     MKID_BE('Midi')

Resource::Resource(SagaEngine *vm): _vm(vm) {
	_contexts = NULL;
	_contextsCount = 0;
}

Resource::~Resource() {
	clearContexts();
}

bool Resource::loadSagaContext(ResourceContext *context, uint32 contextOffset, uint32 contextSize) {
	size_t i;
	bool result;
	byte tableInfo[RSC_TABLEINFO_SIZE];
	byte *tableBuffer;
	size_t tableSize;
	uint32 resourceTableOffset;
	ResourceData *resourceData;

	if (contextSize < RSC_MIN_FILESIZE) {
		return false;
	}

	context->file->seek(contextOffset + contextSize - RSC_TABLEINFO_SIZE);

	if (context->file->read(tableInfo, RSC_TABLEINFO_SIZE) != RSC_TABLEINFO_SIZE) {
		return false;
	}

	MemoryReadStreamEndian readS(tableInfo, RSC_TABLEINFO_SIZE, context->isBigEndian);

	resourceTableOffset = readS.readUint32();
	context->count = readS.readUint32();

	// Check for sane table offset
	if (resourceTableOffset != contextSize - RSC_TABLEINFO_SIZE - RSC_TABLEENTRY_SIZE * context->count) {
		return false;
	}

	// Load resource table
	tableSize = RSC_TABLEENTRY_SIZE * context->count;

	tableBuffer = (byte *)malloc(tableSize);

	context->file->seek(resourceTableOffset + contextOffset, SEEK_SET);

	result = (context->file->read(tableBuffer, tableSize) == tableSize);
	if (result) {
		context->table = (ResourceData *)calloc(context->count, sizeof(*context->table));

		MemoryReadStreamEndian readS1(tableBuffer, tableSize, context->isBigEndian);

		for (i = 0; i < context->count; i++) {
			resourceData = &context->table[i];
			resourceData->offset = contextOffset + readS1.readUint32();
			resourceData->size = readS1.readUint32();
			//sanity check
			if ((resourceData->offset > context->file->size()) || (resourceData->size > contextSize)) {
				result = false;
				break;
			}
		}
	}

	free(tableBuffer);
	return result;
}

bool Resource::loadMacContext(ResourceContext *context) {
	int32 macDataSize, macDataSizePad;
	int32 macResSize, macResSizePad;
	int32 macResOffset;

	uint32 macMapLength;
	uint32 macDataLength;
	uint32 macMapOffset;
	uint32 macDataOffset;

	MacResMap macResMap;
	MacResType *macResTypes;

	MacResType *macResType;
	MacResource *macResource;
	int i, j;
	byte macNameLen;
	bool notSagaContext = false;

	if (context->file->size() < RSC_MIN_FILESIZE + MAC_BINARY_HEADER_SIZE) {
		return false;
	}

	if (context->file->readByte() != 0) {
		return false;
	}
	context->file->readByte(); //MAX Name Len
	context->file->seek(74);
	if (context->file->readByte() != 0) {
		return false;
	}
	context->file->seek(82);
	if (context->file->readByte() != 0) {
		return false;
	}

	macDataSize = context->file->readSint32BE();
	macResSize = context->file->readSint32BE();
	macDataSizePad = (((macDataSize + 127) >> 7) << 7);
	macResSizePad = (((macResSize + 127) >> 7) << 7);

	macResOffset = MAC_BINARY_HEADER_SIZE + macDataSizePad;
	context->file->seek(macResOffset);

	macDataOffset = context->file->readUint32BE() + macResOffset;
	macMapOffset = context->file->readUint32BE() + macResOffset;
	macDataLength = context->file->readUint32BE();
	macMapLength = context->file->readUint32BE();

	if (macDataOffset >= context->file->size() || macMapOffset >= context->file->size() ||
		macDataLength + macMapLength > context->file->size()) {
			return false;
	}

	context->file->seek(macMapOffset + 22);

	macResMap.resAttr = context->file->readUint16BE();
	macResMap.typeOffset = context->file->readUint16BE();
	macResMap.nameOffset = context->file->readUint16BE();
	macResMap.numTypes = context->file->readUint16BE();
	macResMap.numTypes++;

	context->file->seek(macMapOffset + macResMap.typeOffset + 2);

	macResTypes = (MacResType *)calloc(macResMap.numTypes, sizeof(*macResTypes));

	for (i = macResMap.numTypes, macResType = macResTypes; i > 0; i--, macResType++) {
		macResType->id = context->file->readUint32BE();
		macResType->items = context->file->readUint16BE();
		macResType->offset = context->file->readUint16BE();
		macResType->items++;
		macResType->resources = (MacResource*)calloc(macResType->items, sizeof(*macResType->resources));
	}

	for (i = macResMap.numTypes, macResType = macResTypes; i > 0; i--, macResType++) {
		context->file->seek(macResType->offset + macMapOffset + macResMap.typeOffset);

		for (j = macResType->items, macResource = macResType->resources; j > 0; j--, macResource++) {
			macResource->id = context->file->readUint16BE();
			macResource->nameOffset = context->file->readUint16BE();
			macResource->dataOffset = context->file->readUint32BE();
			macResSize = context->file->readUint32BE();

			macResource->attr = macResource->dataOffset >> 24;
			macResource->dataOffset &= 0xFFFFFF;
			if (macResource->id > macResType->maxItemId) {
				macResType->maxItemId = macResource->id;
			}
		}

		for (j = macResType->items, macResource = macResType->resources; j > 0; j--, macResource++) {
			if (macResource->nameOffset != -1) {
				context->file->seek(macResource->nameOffset + macMapOffset + macResMap.nameOffset);
				macNameLen = context->file->readByte();
				context->file->read(macResource->name, macNameLen);
			}
		}
	}

//
	for (i = macResMap.numTypes, macResType = macResTypes; i > 0; i--, macResType++) {
		//getting offsets & sizes of midi
		if (((context->fileType & GAME_MUSICFILE_GM) > 0) && (macResType->id == ID_MIDI)) {

			context->count = macResType->maxItemId + 1;
			context->table = (ResourceData *)calloc(context->count, sizeof(*context->table));
			for (j = macResType->items, macResource = macResType->resources; j > 0; j--, macResource++) {
				context->file->seek(macDataOffset + macResource->dataOffset);
				context->table[macResource->id].size = context->file->readUint32BE();
				context->table[macResource->id].offset = context->file->pos();
			}
			notSagaContext = true;
			break;
		}
	}

//free
	for (i = 0; i < macResMap.numTypes; i++) {
		free(macResTypes[i].resources);
	}
	free(macResTypes);

	if ((!notSagaContext) && (!loadSagaContext(context, MAC_BINARY_HEADER_SIZE, macDataSize))) {
		return false;
	}

	return true;
}

bool Resource::loadContext(ResourceContext *context) {
	size_t i;
	int j;
	const GamePatchDescription *patchDescription;
	ResourceData *resourceData;
	uint16 subjectResourceType;
	ResourceContext *subjectContext;
	uint32 subjectResourceId;
	uint32 patchResourceId;
	ResourceData *subjectResourceData;
	byte *tableBuffer;
	size_t tableSize;
	bool isMacBinary;

	if (!context->file->open(context->fileName)) {
		return false;
	}

	context->isBigEndian = _vm->isBigEndian();

	if (context->fileType & GAME_SWAPENDIAN)
		context->isBigEndian = !context->isBigEndian;

	isMacBinary = (context->fileType & GAME_MACBINARY) > 0;
	context->fileType &= ~GAME_MACBINARY;

	if (isMacBinary) {
		if (!loadMacContext(context)) {
			return false;
		}
	} else {
		if (!loadSagaContext(context, 0, context->file->size())) {
			return false;
		}
	}

	//process internal patch files
	if (GAME_PATCHFILE & context->fileType) {
		subjectResourceType = ~GAME_PATCHFILE & context->fileType;
		subjectContext = getContext((GameFileTypes)subjectResourceType);
		if (subjectContext == NULL) {
			error("Resource::loadContext() Subject context not found");
		}
		loadResource(context, context->count - 1, tableBuffer, tableSize);

		MemoryReadStreamEndian readS2(tableBuffer, tableSize, context->isBigEndian);
		for (i = 0; i < tableSize / 8; i++) {
			subjectResourceId = readS2.readUint32();
			patchResourceId = readS2.readUint32();
			subjectResourceData = getResourceData(subjectContext, subjectResourceId);
			resourceData = getResourceData(context, patchResourceId);
			subjectResourceData->patchData = new PatchData(context->file);
			subjectResourceData->offset = resourceData->offset;
			subjectResourceData->size = resourceData->size;
		}
		free(tableBuffer);
	}

	//process external patch files
	for (j = 0; j < _vm->getPatchesCount(); j++) {
		patchDescription = &_vm->getPatchDescriptions()[j];
		if ((patchDescription->fileType & context->fileType) != 0) {
			if (patchDescription->resourceId < context->count) {
				resourceData = &context->table[patchDescription->resourceId];
				resourceData->patchData = new PatchData(patchDescription);
				if (resourceData->patchData->_patchFile->open(patchDescription->fileName)) {
					resourceData->offset = 0;
					resourceData->size = resourceData->patchData->_patchFile->size();
					// ITE uses several patch files which are loaded and then not needed
					// anymore (as they're in memory), so close them here. IHNM uses only
					// 1 patch file, which is reused, so don't close it
					if (_vm->getGameType() == GType_ITE)
						resourceData->patchData->_patchFile->close();
				} else {
					delete resourceData->patchData;
					resourceData->patchData = NULL;
				}
			}
		}
	}

	// Close the file if it's part of a series of files
	// This prevents having all voice files open in IHNM for no reason, as each chapter uses
	// a different voice file
	if (context->serial > 0)
		context->file->close();

	return true;
}

bool Resource::createContexts() {
	int i;
	ResourceContext *context;
	char musicFileName[256];
	char soundFileName[256];
	int soundFileIndex = 0;
	int voicesFileIndex = 0;
	bool digitalMusic = false;
	bool soundFileInArray = false;
	bool voicesFileInArray = false;
	bool multipleVoices = false;
	bool censoredVersion = false;
	uint16 voiceFileType = GAME_VOICEFILE;

	_vm->_voiceFilesExist = true;

	// If the Wyrmkeep credits file is found, set the Wyrmkeep version flag to true
	if (Common::File::exists("graphics/credit3n.dlt")) {
		_vm->_gf_wyrmkeep = true;
	}

	_contextsCount = 0;
	for (i = 0; _vm->getFilesDescriptions()[i].fileName; i++) {
		_contextsCount++;
		if (_vm->getFilesDescriptions()[i].fileType == GAME_SOUNDFILE)
			soundFileInArray = true;
		if (_vm->getFilesDescriptions()[i].fileType == GAME_VOICEFILE)
			voicesFileInArray = true;
	}

	if (!soundFileInArray) {
		if (_vm->getGameType() == GType_ITE) {
			// If the sound file is not specified in the detector table, add it here
			if (Common::File::exists("sounds.rsc") || Common::File::exists("sounds.cmp")) {
				_contextsCount++;
				soundFileIndex = _contextsCount - 1;
				if (Common::File::exists("sounds.rsc")) {
					sprintf(soundFileName, "sounds.rsc");
				} else {
					sprintf(soundFileName, "sounds.cmp");
					_vm->_gf_compressed_sounds = true;
				}
			} else if (Common::File::exists("soundsd.rsc") || Common::File::exists("soundsd.cmp")) {
				_contextsCount++;
				soundFileIndex = _contextsCount - 1;
				if (Common::File::exists("soundsd.rsc")) {
					sprintf(soundFileName, "soundsd.rsc");
				} else {
					sprintf(soundFileName, "soundsd.cmp");
					_vm->_gf_compressed_sounds = true;
				}
			} else {
				// No sound file found, don't add any file to the array
				soundFileInArray = true;
				// ITE floppy versions have both voices and sounds in voices.rsc
				voiceFileType = GAME_SOUNDFILE | GAME_VOICEFILE;
			}
		} else {
			// If the sound file is not specified in the detector table, add it here
			if (Common::File::exists("sfx.res") || Common::File::exists("sfx.cmp")) {
				_contextsCount++;
				soundFileIndex = _contextsCount - 1;
				if (Common::File::exists("sfx.res")) {
					sprintf(soundFileName, "sfx.res");
				} else {
					sprintf(soundFileName, "sfx.cmp");
					_vm->_gf_compressed_sounds = true;
				}
			} else {
				// No sound file found, don't add any file to the array
				soundFileInArray = true;
			}
		}
	}

	if (!voicesFileInArray) {
		if (_vm->getGameType() == GType_ITE) {
			// If the voices file is not specified in the detector table, add it here
			if (Common::File::exists("voices.rsc") || Common::File::exists("voices.cmp")) {
				_contextsCount++;
				voicesFileIndex = _contextsCount - 1;
				if (Common::File::exists("voices.rsc")) {
					sprintf(_voicesFileName[0], "voices.rsc");
				} else {
					sprintf(_voicesFileName[0], "voices.cmp");
					_vm->_gf_compressed_sounds = true;
				}
			} else if (Common::File::exists("voicesd.rsc") || Common::File::exists("voicesd.cmp")) {
				_contextsCount++;
				voicesFileIndex = _contextsCount - 1;
				if (Common::File::exists("voicesd.rsc")) {
					sprintf(_voicesFileName[0], "voicesd.rsc");
				} else {
					sprintf(_voicesFileName[0], "voicesd.cmp");
					_vm->_gf_compressed_sounds = true;
				}
			} else if (Common::File::exists("inherit the earth voices") ||
					   Common::File::exists("inherit the earth voices.cmp")) {
				_contextsCount++;
				voicesFileIndex = _contextsCount - 1;
				if (Common::File::exists("inherit the earth voices")) {
					sprintf(_voicesFileName[0], "inherit the earth voices");
				} else {
					sprintf(_voicesFileName[0], "inherit the earth voices.cmp");
					_vm->_gf_compressed_sounds = true;
				}

				// The resources in the Wyrmkeep combined Windows/Mac/Linux CD version are little endian, but
				// the voice file is big endian. If we got such a version with mixed files, mark this voice file
				// as big endian
				if (!_vm->isBigEndian())
					voiceFileType = GAME_VOICEFILE | GAME_SWAPENDIAN;	// This file is big endian
			} else {
				// No voice file found, don't add any file to the array
				voicesFileInArray = true;
				warning("No voice file found, voices will be disabled");
				_vm->_voicesEnabled = false;
				_vm->_subtitlesEnabled = true;
				_vm->_voiceFilesExist = false;
			}
		} else {
			// If the voices file is not specified in the detector table, add it here
			if (Common::File::exists("voicess.res") || Common::File::exists("voicess.cmp")) {
				_contextsCount++;
				voicesFileIndex = _contextsCount - 1;
				// IHNM has multiple voice files
				multipleVoices = true;
				// Note: it is assumed that the voices are always last in the list
				if (Common::File::exists("voices4.res") || Common::File::exists("voices4.cmp")) {
					_contextsCount += 6;	// voices1-voices6
				} else {
					// The German and French versions of IHNM don't have Nimdok's chapter, therefore the voices file
					// for that chapter is missing
					_contextsCount += 5;	// voices1-voices3, voices4-voices5
					censoredVersion = true;
				}
				if (Common::File::exists("voicess.res")) {
					sprintf(_voicesFileName[0], "voicess.res");
				} else {
					sprintf(_voicesFileName[0], "voicess.cmp");
					_vm->_gf_compressed_sounds = true;
				}
			} else if (Common::File::exists("voicesd.res") || Common::File::exists("voicesd.cmp")) {
				_contextsCount++;
				voicesFileIndex = _contextsCount - 1;
				if (Common::File::exists("voicesd.res")) {
					sprintf(_voicesFileName[0], "voicesd.res");
				} else {
					sprintf(_voicesFileName[0], "voicesd.cmp");
					_vm->_gf_compressed_sounds = true;
				}
			} else {
				// No voice file found, don't add any file to the array
				voicesFileInArray = true;

				if (_vm->getGameType() == GType_IHNM && _vm->isMacResources()) {
					// The Macintosh version of IHNM has no voices.res, and it has all
					// its voice files in subdirectories, so don't do anything here
				} else {
					warning("No voice file found, voices will be disabled");
					_vm->_voicesEnabled = false;
					_vm->_subtitlesEnabled = true;
					_vm->_voiceFilesExist = false;
				}
			}
		}
	}

	if (_vm->getGameType() == GType_ITE) {
		// Check for digital music in ITE
		if (Common::File::exists("music.rsc") || Common::File::exists("music.cmp")) {
			_contextsCount++;
			digitalMusic = true;
			if (Common::File::exists("music.cmp"))
				sprintf(musicFileName, "music.cmp");
			else
				sprintf(musicFileName, "music.rsc");
		} else if (Common::File::exists("musicd.rsc") || Common::File::exists("musicd.cmp")) {
			_contextsCount++;
			digitalMusic = true;
			if (Common::File::exists("musicd.cmp"))
				sprintf(musicFileName, "musicd.cmp");
			else
				sprintf(musicFileName, "musicd.rsc");
		} else {
			digitalMusic = false;
		}
	}

	_contexts = (ResourceContext*)calloc(_contextsCount, sizeof(*_contexts));

	for (i = 0; i < _contextsCount; i++) {
		context = &_contexts[i];
		context->file = new Common::File();
		context->serial = 0;

		// For ITE, add the digital music file and sfx file information here
		if (_vm->getGameType() == GType_ITE && digitalMusic && i == _contextsCount - 1) {
			context->fileName = musicFileName;
			context->fileType = GAME_MUSICFILE;
		} else if (!soundFileInArray && i == soundFileIndex) {
			context->fileName = soundFileName;
			context->fileType = GAME_SOUNDFILE;
		} else if (!voicesFileInArray && i == voicesFileIndex) {
			context->fileName = _voicesFileName[0];
			// can be GAME_VOICEFILE or GAME_SOUNDFILE | GAME_VOICEFILE or GAME_VOICEFILE | GAME_SWAPENDIAN
			context->fileType = voiceFileType;
		} else {
			if (!(!voicesFileInArray && multipleVoices && (i > voicesFileIndex))) {
				context->fileName = _vm->getFilesDescriptions()[i].fileName;
				context->fileType = _vm->getFilesDescriptions()[i].fileType;
			} else {
				int token = (censoredVersion && (i - voicesFileIndex >= 4)) ? 1 : 0;	// censored versions don't have voice4

				if (_vm->getFeatures() & GF_COMPRESSED_SOUNDS)
					sprintf(_voicesFileName[i - voicesFileIndex + token], "voices%i.cmp", i - voicesFileIndex + token);
				else
					sprintf(_voicesFileName[i - voicesFileIndex + token], "voices%i.res", i - voicesFileIndex + token);

				context->fileName = _voicesFileName[i - voicesFileIndex + token];
				context->fileType = GAME_VOICEFILE;

				// IHNM has several different voice files, so we need to allow
				// multiple resource contexts of the same type. We tell them
				// apart by assigning each of the duplicates a unique serial
				// number. The default behaviour when requesting a context will
				// be to look for serial number 0.
				context->serial = i - voicesFileIndex + token;
			}
		}

		if (!loadContext(context)) {
			return false;
		}
	}
	return true;
}

void Resource::clearContexts() {
	int i;
	size_t j;
	ResourceContext *context;
	if (_contexts == NULL) {
		return;
	}
	for (i = 0; i < _contextsCount; i++) {
		context = &_contexts[i];
		delete context->file;
		if (context->table != NULL) {
			for (j = 0; j < context->count; j++) {
				delete context->table[j].patchData;
			}
		}
		free(context->table);
	}
	free(_contexts);
	_contexts = NULL;
}

uint32 Resource::convertResourceId(uint32 resourceId) {

	if (_vm->getGameType() == GType_ITE && _vm->isMacResources()) {
		if (resourceId > 1537) {
			return resourceId - 2;
		} else {
			if (resourceId == 1535 || resourceId == 1536) {
				error("Wrong resource number %d for Mac ITE", resourceId);
			}
		}
	}

	return resourceId;
}

void Resource::loadResource(ResourceContext *context, uint32 resourceId, byte*&resourceBuffer, size_t &resourceSize) {
	Common::File *file;
	uint32 resourceOffset;
	ResourceData *resourceData;

	debug(8, "loadResource %d", resourceId);

	resourceData = getResourceData(context, resourceId);

	file = context->getFile(resourceData);

	resourceOffset = resourceData->offset;
	resourceSize = resourceData->size;

	resourceBuffer = (byte*)malloc(resourceSize);

	file->seek((long)resourceOffset, SEEK_SET);

	if (file->read(resourceBuffer, resourceSize) != resourceSize) {
		error("Resource::loadResource() failed to read");
	}

	// ITE uses several patch files which are loaded and then not needed
	// anymore (as they're in memory), so close them here. IHNM uses only
	// 1 patch file, which is reused, so don't close it
	if (resourceData->patchData != NULL && _vm->getGameType() == GType_ITE)
		file->close();
}

static int metaResourceTable[] = { 0, 326, 517, 677, 805, 968, 1165, 0, 1271 };
static int metaResourceTableDemo[] = { 0, 0, 0, 0, 0, 0, 0, 285, 0 };

void Resource::loadGlobalResources(int chapter, int actorsEntrance) {
	if (chapter < 0)
		chapter = (_vm->getGameId() != GID_IHNM_DEMO) ? 8 : 7;

	// TODO
	//if (module.voiceLUT)
	//	free module.voiceLUT;

	// TODO: close chapter context, or rather reassign it in our case

	ResourceContext *resourceContext;
	ResourceContext *soundContext;
	int i;

	resourceContext = _vm->_resource->getContext(GAME_RESOURCEFILE);
	if (resourceContext == NULL) {
		error("Resource::loadGlobalResources() resource context not found");
	}

	soundContext = _vm->_resource->getContext(GAME_SOUNDFILE);
	if (soundContext == NULL) {
		error("Resource::loadGlobalResources() sound context not found");
	}

	byte *resourcePointer;
	size_t resourceLength;

	if (_vm->getGameId() != GID_IHNM_DEMO) {
		_vm->_resource->loadResource(resourceContext, metaResourceTable[chapter],
									 resourcePointer, resourceLength);
	} else {
		_vm->_resource->loadResource(resourceContext, metaResourceTableDemo[chapter],
									 resourcePointer, resourceLength);
	}

	if (resourceLength == 0) {
		error("Resource::loadGlobalResources wrong metaResource");
	}

	MemoryReadStream metaS(resourcePointer, resourceLength);

	_metaResource.sceneIndex = metaS.readSint16LE();
	_metaResource.objectCount = metaS.readSint16LE();
	_metaResource.objectsStringsResourceID = metaS.readSint32LE();
	_metaResource.inventorySpritesID = metaS.readSint32LE();
	_metaResource.mainSpritesID = metaS.readSint32LE();
	_metaResource.objectsResourceID = metaS.readSint32LE();
	_metaResource.actorCount = metaS.readSint16LE();
	_metaResource.actorsStringsResourceID = metaS.readSint32LE();
	_metaResource.actorsResourceID = metaS.readSint32LE();
	_metaResource.protagFaceSpritesID = metaS.readSint32LE();
	_metaResource.field_22 = metaS.readSint32LE();
	_metaResource.field_26 = metaS.readSint16LE();
	_metaResource.protagStatesCount = metaS.readSint16LE();
	_metaResource.protagStatesResourceID = metaS.readSint32LE();
	_metaResource.cutawayListResourceID = metaS.readSint32LE();
	_metaResource.songTableID = metaS.readSint32LE();

	free(resourcePointer);

	_vm->_actor->loadActorList(actorsEntrance, _metaResource.actorCount,
						  _metaResource.actorsResourceID, _metaResource.protagStatesCount,
						  _metaResource.protagStatesResourceID);

	_vm->_actor->_protagonist->_sceneNumber = _metaResource.sceneIndex;

	_vm->_actor->_objectsStrings.freeMem();

	_vm->_resource->loadResource(resourceContext, _metaResource.objectsStringsResourceID, resourcePointer, resourceLength);
	_vm->loadStrings(_vm->_actor->_objectsStrings, resourcePointer, resourceLength);
	free(resourcePointer);

	if (chapter >= _vm->_sndRes->_fxTableIDsLen) {
		error("Chapter ID exceeds fxTableIDs length");
	}

	debug(0, "Going to read %d of %d", chapter, _vm->_sndRes->_fxTableIDs[chapter]);
	_vm->_resource->loadResource(soundContext, _vm->_sndRes->_fxTableIDs[chapter],
								 resourcePointer, resourceLength);

	if (resourceLength == 0) {
		error("Resource::loadGlobalResources Can't load sound effects for current track");
	}

	free(_vm->_sndRes->_fxTable);

	_vm->_sndRes->_fxTableLen = resourceLength / 4;
	_vm->_sndRes->_fxTable = (FxTable *)malloc(sizeof(FxTable) * _vm->_sndRes->_fxTableLen);

	MemoryReadStream fxS(resourcePointer, resourceLength);

	for (i = 0; i < _vm->_sndRes->_fxTableLen; i++) {
		_vm->_sndRes->_fxTable[i].res = fxS.readSint16LE();
		_vm->_sndRes->_fxTable[i].vol = fxS.readSint16LE();
	}
	free(resourcePointer);

	_vm->_interface->_defPortraits.freeMem();
	_vm->_sprite->loadList(_metaResource.protagFaceSpritesID, _vm->_interface->_defPortraits);

	_vm->_actor->_actorsStrings.freeMem();

	_vm->_resource->loadResource(resourceContext, _metaResource.actorsStringsResourceID, resourcePointer, resourceLength);
	_vm->loadStrings(_vm->_actor->_actorsStrings, resourcePointer, resourceLength);
	free(resourcePointer);

	_vm->_sprite->_inventorySprites.freeMem();
	_vm->_sprite->loadList(_metaResource.inventorySpritesID, _vm->_sprite->_inventorySprites);

	_vm->_sprite->_mainSprites.freeMem();
	_vm->_sprite->loadList(_metaResource.mainSpritesID, _vm->_sprite->_mainSprites);

	_vm->_actor->loadObjList(_metaResource.objectCount, _metaResource.objectsResourceID);

	_vm->_resource->loadResource(resourceContext, _metaResource.cutawayListResourceID, resourcePointer, resourceLength);

	if (resourceLength == 0) {
		error("Resource::loadGlobalResources Can't load cutaway list");
	}

	_vm->_anim->loadCutawayList(resourcePointer, resourceLength);

	if (_metaResource.songTableID > 0) {
		_vm->_resource->loadResource(resourceContext, _metaResource.songTableID, resourcePointer, resourceLength);

		if (chapter == 6) {
			int32 id = READ_LE_UINT32(&resourcePointer[actorsEntrance * 4]);
			free(resourcePointer);
			_vm->_resource->loadResource(resourceContext, id, resourcePointer, resourceLength);
		}

		if (resourceLength == 0) {
			error("Resource::loadGlobalResources Can't load songs list for current track");
		}

		free(_vm->_music->_songTable);

		_vm->_music->_songTableLen = resourceLength / 4;
		_vm->_music->_songTable = (int32 *)malloc(sizeof(int32) * _vm->_music->_songTableLen);

		MemoryReadStream songS(resourcePointer, resourceLength);

		for (i = 0; i < _vm->_music->_songTableLen; i++)
			_vm->_music->_songTable[i] = songS.readSint32LE();
		free(resourcePointer);
	} else {
		// The IHNM demo has a fixed music track and doesn't load a song table
		_vm->_music->setVolume(_vm->_musicVolume == 10 ? -1 : _vm->_musicVolume * 25, 1);
		_vm->_music->play(3, MUSIC_LOOP);
	}

	int voiceLUTResourceID = 0;

	_vm->_script->_globalVoiceLUT.freeMem();

	switch (chapter) {
	case 1:
		_vm->_sndRes->setVoiceBank(1);
		voiceLUTResourceID = 23;
		break;
	case 2:
		_vm->_sndRes->setVoiceBank(2);
		voiceLUTResourceID = 24;
		break;
	case 3:
		_vm->_sndRes->setVoiceBank(3);
		voiceLUTResourceID = 25;
		break;
	case 4:
		_vm->_sndRes->setVoiceBank(4);
		voiceLUTResourceID = 26;
		break;
	case 5:
		_vm->_sndRes->setVoiceBank(5);
		voiceLUTResourceID = 27;
		break;
	case 6:
		_vm->_sndRes->setVoiceBank(6);
		voiceLUTResourceID = 28;
		break;
	case 7:
		// IHNM demo
		_vm->_sndRes->setVoiceBank(0);
		voiceLUTResourceID = 17;
		break;
	case 8:
		_vm->_sndRes->setVoiceBank(0);
		voiceLUTResourceID = 22;
		break;
	}

	if (voiceLUTResourceID) {
		_vm->_resource->loadResource(resourceContext, voiceLUTResourceID, resourcePointer, resourceLength);
		_vm->_script->loadVoiceLUT(_vm->_script->_globalVoiceLUT, resourcePointer, resourceLength);
		free(resourcePointer);
	}

	_vm->_spiritualBarometer = 0;
	_vm->_scene->setChapterNumber(chapter);
}

} // End of namespace Saga
