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
#include "saga/resource.h"
#include "saga/scene.h"
#include "saga/sndres.h"

#include "engines/advancedDetector.h"

namespace Saga {

Resource::Resource(SagaEngine *vm): _vm(vm) {
	_contexts = NULL;
	_contextsCount = 0;
}

Resource::~Resource() {
	clearContexts();
}

bool Resource::loadResContext_v1(ResourceContext *context, uint32 contextOffset, uint32 contextSize) {
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
			if ((resourceData->offset > (uint)context->fileSize) || (resourceData->size > contextSize)) {
				result = false;
				break;
			}
		}
	}

	free(tableBuffer);
	return result;
}

bool Resource::loadContext(ResourceContext *context) {
	size_t i;
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

	context->fileSize = context->file->size();
	context->isBigEndian = _vm->isBigEndian();

	if (context->fileType & GAME_SWAPENDIAN)
		context->isBigEndian = !context->isBigEndian;

	isMacBinary = (context->fileType & GAME_MACBINARY) > 0;
	context->fileType &= ~GAME_MACBINARY;

	if (!isMacBinary) {
		if (!loadResContext(context, 0, context->fileSize)) {
			return false;
		}
	} else {
		if (!loadMacContext(context)) {
			return false;
		}
	}

	//process internal patch files
	if (context->fileType & GAME_PATCHFILE) {
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
			subjectResourceData = subjectContext->getResourceData(subjectResourceId);
			resourceData = context->getResourceData(patchResourceId);
			subjectResourceData->patchData = new PatchData(context->file);
			subjectResourceData->offset = resourceData->offset;
			subjectResourceData->size = resourceData->size;
		}
		free(tableBuffer);
	}

	//process external patch files
	for (patchDescription = _vm->getPatchDescriptions(); patchDescription && patchDescription->fileName; ++patchDescription) {
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
					if (_vm->getGameId() == GID_ITE)
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
	int soundFileIndex = 0;
	int voicesFileIndex = 0;
	bool digitalMusic = false;
	bool soundFileInArray = false;
	bool multipleVoices = false;
	bool censoredVersion = false;
	bool compressedSounds = false;
	bool compressedMusic = false;
	uint16 voiceFileType = GAME_VOICEFILE;
	bool fileFound = false;
	int maxFile = 0;

	_vm->_voiceFilesExist = true;

	struct SoundFileInfo {
		char fileName[40];
		bool isCompressed;
	};

	SoundFileInfo *curSoundfiles = 0;

	// If the Wyrmkeep credits file is found, set the Wyrmkeep version flag to true
	if (Common::File::exists("graphics/credit3n.dlt")) {
		_vm->_gf_wyrmkeep = true;
	}

	_contextsCount = 0;
	for (i = 0; _vm->getFilesDescriptions()[i].fileName; i++) {
		_contextsCount++;
		if (_vm->getFilesDescriptions()[i].fileType == GAME_SOUNDFILE)
			soundFileInArray = true;
	}

	//// Detect and add SFX files ////////////////////////////////////////////////
	SoundFileInfo sfxFilesITE[] = {
		{	"sounds.rsc",		false	},
		{	"sounds.cmp",		true	},
		{	"soundsd.rsc",		false	},
		{	"soundsd.cmp",		true	}
	};

#ifdef ENABLE_IHNM
	SoundFileInfo sfxFilesIHNM[] = {
		{	"sfx.res",			false	},
		{	"sfx.cmp",			true	}
	};
#endif

#ifdef ENABLE_SAGA2
	SoundFileInfo sfxFilesFTA2[] = {
		{	"ftasound.hrs",		false	}
	};

	SoundFileInfo sfxFilesDino[] = {
		{	"dinosnd.hrs",		false	},
	};
#endif

	if (!soundFileInArray) {
		// If the sound file is not specified in the detector table, add it here
		fileFound = false;

		switch (_vm->getGameId()) {
			case GID_ITE:
				curSoundfiles = sfxFilesITE;
				maxFile = 4;
				break;
#ifdef ENABLE_IHNM
			case GID_IHNM:
				curSoundfiles = sfxFilesIHNM;
				maxFile = 2;
				break;
#endif
#ifdef ENABLE_SAGA2
			case GID_DINO:
				curSoundfiles = sfxFilesDino;
				maxFile = 1;
				break;
			case GID_FTA2:
				curSoundfiles = sfxFilesFTA2;
				maxFile = 1;
				break;
#endif
		}

		for (i = 0; i < maxFile; i++) {
			if (Common::File::exists(curSoundfiles[i].fileName)) {
				_contextsCount++;
				soundFileIndex = _contextsCount - 1;
				strcpy(_soundFileName, curSoundfiles[i].fileName);
				compressedSounds = curSoundfiles[i].isCompressed;
				fileFound = true;
				break;
			}
		}

		if (!fileFound) {
			// No sound file found, don't add any file to the array
			soundFileInArray = true;
			if (_vm->getGameId() == GID_ITE) {
				// ITE floppy versions have both voices and sounds in voices.rsc
				voiceFileType = GAME_SOUNDFILE | GAME_VOICEFILE;
			}
		}
	}

	//// Detect and add voice files /////////////////////////////////////////////
	SoundFileInfo voiceFilesITE[] = {
		{	"voices.rsc",					false	},
		{	"voices.cmp",					true	},
		{	"voicesd.rsc",					false	},
		{	"voicesd.cmp",					true	},
		{	"inherit the earth voices",		false	},
		{	"inherit the earth voices.cmp",	true	},
		{	"ite voices.bin",				false	}
	};

#ifdef ENABLE_IHNM
	SoundFileInfo voiceFilesIHNM[] = {
		{	"voicess.res",					false	},
		{	"voicess.cmp",					true	},
		{	"voicesd.res",					false	},
		{	"voicesd.cmp",					true	},
	};
#endif

#ifdef ENABLE_SAGA2
	SoundFileInfo voiceFilesFTA2[] = {
		{	"ftavoice.hrs",					false	},
	};
#endif

	// Detect and add voice files
	fileFound = false;

	switch (_vm->getGameId()) {
		case GID_ITE:
			curSoundfiles = voiceFilesITE;
			maxFile = 7;
			break;
#ifdef ENABLE_IHNM
		case GID_IHNM:
			curSoundfiles = voiceFilesIHNM;
			maxFile = 4;
			break;
#endif
#ifdef ENABLE_SAGA2
		/*
		case GID_DINO:
			// TODO
			curSoundfiles = NULL;
			maxFile = 0;
			break;
		*/
		case GID_FTA2:
			curSoundfiles = voiceFilesFTA2;
			maxFile = 1;
			break;
#endif
	}

	for (i = 0; i < maxFile; i++) {
		if (Common::File::exists(curSoundfiles[i].fileName)) {
			_contextsCount++;
			voicesFileIndex = _contextsCount - 1;
			strcpy(_voicesFileName[0], curSoundfiles[i].fileName);
			compressedSounds = curSoundfiles[i].isCompressed;
			fileFound = true;

			// Special cases
			if (!scumm_stricmp(curSoundfiles[i].fileName, "inherit the earth voices") ||
				!scumm_stricmp(curSoundfiles[i].fileName, "inherit the earth voices.cmp")) {
				// The resources in the Wyrmkeep combined Windows/Mac/Linux CD version are little endian, but
				// the voice file is big endian. If we got such a version with mixed files, mark this voice file
				// as big endian
				if (!_vm->isBigEndian())
					voiceFileType = GAME_VOICEFILE | GAME_SWAPENDIAN;	// This file is big endian
			}

			if (!scumm_stricmp(curSoundfiles[i].fileName, "ite voices.bin")) {
				voiceFileType = GAME_VOICEFILE | GAME_MACBINARY;
			}

			if (!scumm_stricmp(curSoundfiles[i].fileName, "voicess.res") ||
				!scumm_stricmp(curSoundfiles[i].fileName, "voicess.cmp")) {
				// IHNM has multiple voice files
				multipleVoices = true;
				// Note: it is assumed that the voice files are always last in the list
				if (Common::File::exists("voices4.res") || Common::File::exists("voices4.cmp")) {
					_contextsCount += 6;	// voices1-voices6
				} else {
					// The German and French versions of IHNM don't have Nimdok's chapter,
					// therefore the voices file for that chapter is missing
					_contextsCount += 5;	// voices1-voices3, voices4-voices5
					censoredVersion = true;
				}
			}

			break;
		}
	}

	if (!fileFound) {
		if (_vm->getGameId() == GID_IHNM && _vm->isMacResources()) {
			// The Macintosh version of IHNM has no voices.res, and it has all
			// its voice files in subdirectories, so don't do anything here
		} else {
			warning("No voice file found, voices will be disabled");
			_vm->_voicesEnabled = false;
			_vm->_subtitlesEnabled = true;
			_vm->_voiceFilesExist = false;
		}
	}

	//// Detect and add ITE music files /////////////////////////////////////////
	SoundFileInfo musicFilesITE[] = {
		{	"music.rsc",	false	},
		{	"music.cmp",	true	},
		{	"musicd.rsc",	false	},
		{	"musicd.cmp",	true	},
	};

	// Check for digital music in ITE
	if (_vm->getGameId() == GID_ITE) {
		fileFound = false;

		for (i = 0; i < 4; i++) {
			if (Common::File::exists(musicFilesITE[i].fileName)) {
				_contextsCount++;
				digitalMusic = true;
				compressedMusic = musicFilesITE[i].isCompressed;
				fileFound = true;
				strcpy(_musicFileName, musicFilesITE[i].fileName);
				break;
			}
		}

		if (!fileFound) {
			// No sound file found, don't add any file to the array
			digitalMusic = false;
		}
	}

	_contexts = (ResourceContext*)calloc(_contextsCount, sizeof(*_contexts));

	for (i = 0; i < _contextsCount; i++) {
		context = &_contexts[i];
		context->file = new Common::File();
		context->serial = 0;

		// For ITE, add the digital music file and sfx file information here
		if (_vm->getGameId() == GID_ITE && digitalMusic && i == _contextsCount - 1) {
			context->fileName = _musicFileName;
			context->fileType = GAME_DIGITALMUSICFILE;
			context->isCompressed = compressedMusic;
		} else if (!soundFileInArray && i == soundFileIndex) {
			context->fileName = _soundFileName;
			context->fileType = GAME_SOUNDFILE;
			context->isCompressed = compressedSounds;
		} else if (_vm->_voiceFilesExist && i == voicesFileIndex && !(_vm->getGameId() == GID_IHNM && _vm->isMacResources())) {
			context->fileName = _voicesFileName[0];
			// can be GAME_VOICEFILE or GAME_SOUNDFILE | GAME_VOICEFILE or GAME_VOICEFILE | GAME_SWAPENDIAN
			context->fileType = voiceFileType;
			context->isCompressed = compressedSounds;
		} else {
			if (!(_vm->_voiceFilesExist && multipleVoices && (i > voicesFileIndex))) {
				context->fileName = _vm->getFilesDescriptions()[i].fileName;
				context->fileType = _vm->getFilesDescriptions()[i].fileType;
				context->isCompressed = compressedSounds;
			} else {
				int token = (censoredVersion && (i - voicesFileIndex >= 4)) ? 1 : 0;	// censored versions don't have voice4

				if (compressedSounds)
					sprintf(_voicesFileName[i - voicesFileIndex + token], "voices%i.cmp", i - voicesFileIndex + token);
				else
					sprintf(_voicesFileName[i - voicesFileIndex + token], "voices%i.res", i - voicesFileIndex + token);

				context->fileName = _voicesFileName[i - voicesFileIndex + token];
				context->fileType = GAME_VOICEFILE;
				context->isCompressed = compressedSounds;

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
		if (_vm->isSaga2()) {
			free(context->categories);
		}
		free(context->table);
	}
	free(_contexts);
	_contexts = NULL;
}

void Resource::loadResource(ResourceContext *context, uint32 resourceId, byte*&resourceBuffer, size_t &resourceSize) {
	Common::File *file;
	uint32 resourceOffset;
	ResourceData *resourceData;

	debug(8, "loadResource %d", resourceId);

	resourceData = context->getResourceData(resourceId);

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
	if (resourceData->patchData != NULL && _vm->getGameId() == GID_ITE)
		file->close();
}

} // End of namespace Saga
