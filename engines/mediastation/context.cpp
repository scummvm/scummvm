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

#include "mediastation/mediastation.h"
#include "mediastation/context.h"
#include "mediastation/debugchannels.h"

#include "mediastation/bitmap.h"
#include "mediastation/mediascript/collection.h"
#include "mediastation/actors/canvas.h"
#include "mediastation/actors/palette.h"
#include "mediastation/actors/image.h"
#include "mediastation/actors/path.h"
#include "mediastation/actors/sound.h"
#include "mediastation/actors/movie.h"
#include "mediastation/actors/sprite.h"
#include "mediastation/actors/hotspot.h"
#include "mediastation/actors/timer.h"
#include "mediastation/actors/screen.h"
#include "mediastation/actors/font.h"
#include "mediastation/actors/text.h"

namespace MediaStation {

Context::Context(const Common::Path &path) : Datafile(path) {
	uint32 signature = _handle->readUint32BE();
	if (signature != MKTAG('I', 'I', '\0', '\0')) {
		error("%s: Wrong signature for file %s: 0x%08x", __func__, _name.c_str(), signature);
	}

	_unk1 = _handle->readUint32LE();
	_subfileCount = _handle->readUint32LE();
	_fileSize = _handle->readUint32LE();
	debugC(5, kDebugLoading, "Context::Context(): _unk1 = 0x%x", _unk1);

	Subfile subfile = getNextSubfile();
	Chunk chunk = subfile.nextChunk();

	if (g_engine->isFirstGenerationEngine()) {
		readOldStyleHeaderSections(subfile, chunk);
	} else {
		readNewStyleHeaderSections(subfile, chunk);
	}

	chunk = subfile._currentChunk;
	while (!subfile.atEnd()) {
		readActorInFirstSubfile(chunk);
		if (!subfile.atEnd()) {
			chunk = subfile.nextChunk();
		}
	}

	// Read actors in the rest of the subfiles.
	for (uint i = 1; i < _subfileCount; i++) {
		subfile = getNextSubfile();
		readActorFromLaterSubfile(subfile);
	}

	// Some sprites and images don't have any image data themselves, they just
	// reference the same image data in another actor. So we need to check for
	// these and create the appropriate references.
	for (auto it = _actors.begin(); it != _actors.end(); ++it) {
		Actor *actor = it->_value;
		uint referencedActorId = actor->_actorReference;
		if (referencedActorId != 0) {
			switch (actor->type()) {
			case kActorTypeImage: {
				ImageActor *image = static_cast<ImageActor *>(actor);
				ImageActor *referencedImage = static_cast<ImageActor *>(getActorById(referencedActorId));
				if (referencedImage == nullptr) {
					error("%s: Actor %d references non-existent actor %d", __func__, actor->id(), referencedActorId);
				}
				image->_bitmap = referencedImage->_bitmap;
				break;
			}

			case kActorTypeSprite: {
				SpriteMovieActor *sprite = static_cast<SpriteMovieActor *>(actor);
				SpriteMovieActor *referencedSprite = static_cast<SpriteMovieActor *>(getActorById(referencedActorId));
				if (referencedSprite == nullptr) {
					error("%s: Actor %d references non-existent actor %d", __func__, actor->id(), referencedActorId);
				}
				sprite->_frames = referencedSprite->_frames;
				sprite->_clips = referencedSprite->_clips;
				break;
			}

			default:
				error("%s: Actor type %d referenced, but reference not implemented yet", __func__, actor->type());
			}
		}
	}
}

Context::~Context() {
	delete _palette;
	_palette = nullptr;

	for (auto it = _actors.begin(); it != _actors.end(); ++it) {
		delete it->_value;
	}
	_actors.clear();
	// The same actor pointers are in here, so don't delete again.
	_actorsByChunkReference.clear();

	for (auto it = _functions.begin(); it != _functions.end(); ++it) {
		delete it->_value;
	}
	_functions.clear();

	for (auto it = _variables.begin(); it != _variables.end(); ++it) {
		delete it->_value;
	}
	_variables.clear();
}

Actor *Context::getActorById(uint actorId) {
	return _actors.getValOrDefault(actorId);
}

Actor *Context::getActorByChunkReference(uint chunkReference) {
	return _actorsByChunkReference.getValOrDefault(chunkReference);
}

Function *Context::getFunctionById(uint functionId) {
	return _functions.getValOrDefault(functionId);
}

ScriptValue *Context::getVariable(uint variableId) {
	return _variables.getValOrDefault(variableId);
}

void Context::readCreateContextData(Chunk &chunk) {
	_fileNumber = chunk.readTypedUint16();

	ContextParametersSectionType sectionType = static_cast<ContextParametersSectionType>(chunk.readTypedUint16());
	while (sectionType != kContextParametersEmptySection) {
		debugC(5, kDebugLoading, "ContextParameters::ContextParameters: sectionType = 0x%x (@0x%llx)", static_cast<uint>(sectionType), static_cast<long long int>(chunk.pos()));
		switch (sectionType) {
		case kContextParametersName: {
			uint repeatedFileNumber = chunk.readTypedUint16();
			if (repeatedFileNumber != _fileNumber) {
				warning("%s: Repeated file number didn't match: %d != %d", __func__, repeatedFileNumber, _fileNumber);
			}
			_contextName = chunk.readTypedString();

			uint endingFlag = chunk.readTypedUint16();
			if (endingFlag != 0) {
				warning("%s: Got non-zero ending flag 0x%x", __func__, endingFlag);
			}
			break;
		}

		case kContextParametersFileNumber: {
			error("%s: Section type FILE_NUMBER not implemented yet", __func__);
			break;
		}

		case kContextParametersVariable: {
			readCreateVariableData(chunk);
			break;
		}

		case kContextParametersBytecode: {
			Function *function = new Function(chunk);
			_functions.setVal(function->_id, function);
			break;
		}

		default:
			error("%s: Unknown section type 0x%x", __func__, static_cast<uint>(sectionType));
		}

		sectionType = static_cast<ContextParametersSectionType>(chunk.readTypedUint16());
	}
}

Actor *Context::readCreateActorData(Chunk &chunk) {
	uint contextId = chunk.readTypedUint16();
	ActorType type = static_cast<ActorType>(chunk.readTypedUint16());
	uint id = chunk.readTypedUint16();
	debugC(4, kDebugLoading, "_type = 0x%x, _id = 0x%x", static_cast<uint>(type), id);

	Actor *actor = nullptr;
	switch (type) {
	case kActorTypeImage:
		actor = new ImageActor();
		break;

	case kActorTypeMovie:
		actor = new StreamMovieActor();
		break;

	case kActorTypeSound:
		actor = new SoundActor();
		break;

	case kActorTypePalette:
		actor = new PaletteActor();
		break;

	case kActorTypePath:
		actor = new PathActor();
		break;

	case kActorTypeTimer:
		actor = new TimerActor();
		break;

	case kActorTypeHotspot:
		actor = new HotspotActor();
		break;

	case kActorTypeSprite:
		actor = new SpriteMovieActor();
		break;

	case kActorTypeCanvas:
		actor = new CanvasActor();
		break;

	case kActorTypeScreen:
		actor = new ScreenActor();
		_screenActor = static_cast<ScreenActor *>(actor);
		break;

	case kActorTypeFont:
		actor = new FontActor();
		break;

	case kActorTypeText:
		actor = new TextActor();
		break;

	default:
		error("%s: No class for actor type 0x%x (@0x%llx)", __func__, static_cast<uint>(type), static_cast<long long int>(chunk.pos()));
	}
	actor->setId(id);
	actor->setContextId(contextId);
	actor->initFromParameterStream(chunk);
	return actor;
}

void Context::readCreateVariableData(Chunk &chunk) {
	uint repeatedFileNumber = chunk.readTypedUint16();
	if (repeatedFileNumber != _fileNumber) {
		warning("%s: Repeated file number didn't match: %d != %d", __func__, repeatedFileNumber, _fileNumber);
	}

	uint id = chunk.readTypedUint16();
	if (g_engine->getVariable(id) != nullptr) {
		error("%s: Global variable %d already exists", __func__, id);
	}

	ScriptValue *value = new ScriptValue(&chunk);
	_variables.setVal(id, value);
	debugC(5, kDebugScript, "Created global variable %d (type: %s)",
		id, scriptValueTypeToStr(value->getType()));
}

void Context::readOldStyleHeaderSections(Subfile &subfile, Chunk &chunk) {
	error("%s: Not implemented yet", __func__);
}

void Context::readNewStyleHeaderSections(Subfile &subfile, Chunk &chunk) {
	bool moreSectionsToRead = (chunk._id == MKTAG('i', 'g', 'o', 'd'));
	if (!moreSectionsToRead) {
		warning("%s: Got no header sections (@0x%llx)", __func__, static_cast<long long int>(chunk.pos()));
	}

	while (moreSectionsToRead) {
		// Verify this chunk is a header.
		// TODO: What are the situations when it's not?
		uint16 sectionType = chunk.readTypedUint16();
		debugC(5, kDebugLoading, "Context::readNewStyleHeaderSections(): sectionType = 0x%x (@0x%llx)", static_cast<uint>(sectionType), static_cast<long long int>(chunk.pos()));
		bool chunkIsHeader = (sectionType == 0x000d);
		if (!chunkIsHeader) {
			error("%s: Expected header chunk, got %s (@0x%llx)", __func__, tag2str(chunk._id), static_cast<long long int>(chunk.pos()));
		}

		// Read this header section.
		moreSectionsToRead = readHeaderSection(chunk);
		if (subfile.atEnd()) {
			break;
		} else {
			debugC(5, kDebugLoading, "\nContext::readNewStyleHeaderSections(): Getting next chunk (@0x%llx)", static_cast<long long int>(chunk.pos()));
			chunk = subfile.nextChunk();
			moreSectionsToRead = (chunk._id == MKTAG('i', 'g', 'o', 'd'));
		}
	}
	debugC(5, kDebugLoading, "Context::readNewStyleHeaderSections(): Finished reading sections (@0x%llx)", static_cast<long long int>(chunk.pos()));
}

void Context::readActorInFirstSubfile(Chunk &chunk) {
	if (chunk._id == MKTAG('i', 'g', 'o', 'd')) {
		warning("%s: Skippping \"igod\" actor link chunk", __func__);
		chunk.skip(chunk.bytesRemaining());
		return;
	}

	// TODO: Make sure this is not an actor link.
	Actor *actor = getActorByChunkReference(chunk._id);
	if (actor == nullptr) {
		// We should only need to look in the global scope when there is an
		// install cache (INSTALL.CXT).
		actor = g_engine->getActorByChunkReference(chunk._id);
		if (actor == nullptr) {
			error("%s: Actor for chunk \"%s\" (0x%x) does not exist or has not been read yet in this title. (@0x%llx)", __func__, tag2str(chunk._id), chunk._id, static_cast<long long int>(chunk.pos()));
		}
	}
	debugC(5, kDebugLoading, "\nContext::readActorInFirstSubfile(): Got actor with chunk ID %s in first subfile (type: 0x%x) (@0x%llx)", tag2str(chunk._id), static_cast<uint>(actor->type()), static_cast<long long int>(chunk.pos()));
	actor->readChunk(chunk);
}

void Context::readActorFromLaterSubfile(Subfile &subfile) {
	Chunk chunk = subfile.nextChunk();
	Actor *actor = getActorByChunkReference(chunk._id);
	if (actor == nullptr) {
		// We should only need to look in the global scope when there is an
		// install cache (INSTALL.CXT).
		actor = g_engine->getActorByChunkReference(chunk._id);
		if (actor == nullptr) {
			error("%s: Actor for chunk \"%s\" (0x%x) does not exist or has not been read yet in this title. (@0x%llx)", __func__, tag2str(chunk._id), chunk._id, static_cast<long long int>(chunk.pos()));
		}
	}
	debugC(5, kDebugLoading, "\nContext::readActorFromLaterSubfile(): Got actor with chunk ID %s in later subfile (type: 0x%x) (@0x%llx)", tag2str(chunk._id), actor->type(), static_cast<long long int>(chunk.pos()));
	actor->readSubfile(subfile, chunk);
}

bool Context::readHeaderSection(Chunk &chunk) {
	uint16 sectionType = chunk.readTypedUint16();
	debugC(5, kDebugLoading, "Context::readHeaderSection(): sectionType = 0x%x (@0x%llx)", static_cast<uint>(sectionType), static_cast<long long int>(chunk.pos()));
	switch (sectionType) {
	case kContextParametersSection: {
		readCreateContextData(chunk);
		break;
	}

	case kContextActorLinkSection: {
		warning("%s: ACTOR_LINK not implemented yet", __func__);
		chunk.skip(chunk.bytesRemaining());
		break;
	}

	case kContextPaletteSection: {
		if (_palette != nullptr) {
			error("%s: Got multiple palettes (@0x%llx)", __func__, static_cast<long long int>(chunk.pos()));
		}

		byte *buffer = new byte[Graphics::PALETTE_SIZE];
		chunk.read(buffer, Graphics::PALETTE_SIZE);
		_palette = new Graphics::Palette(buffer, Graphics::PALETTE_COUNT, DisposeAfterUse::YES);
		debugC(5, kDebugLoading, "Context::readHeaderSection(): Read palette");

		// This is likely just an ending flag that we expect to be zero.
		uint endingFlag = chunk.readTypedUint16();
		if (endingFlag != 0) {
			warning("%s: Got non-zero ending flag 0x%x", __func__, endingFlag);
		}
		break;
	}

	case kContextActorHeaderSection: {
		Actor *actor = readCreateActorData(chunk);
		_actors.setVal(actor->id(), actor);
		g_engine->registerActor(actor);
		if (actor->_chunkReference != 0) {
			debugC(5, kDebugLoading, "Context::readHeaderSection(): Storing actor with chunk ID \"%s\" (0x%x)", tag2str(actor->_chunkReference), actor->_chunkReference);
			_actorsByChunkReference.setVal(actor->_chunkReference, actor);
		}

		if (actor->type() == kActorTypeMovie) {
			StreamMovieActor *movie = static_cast<StreamMovieActor *>(actor);
			if (movie->_audioChunkReference != 0) {
				_actorsByChunkReference.setVal(movie->_audioChunkReference, actor);
			}
			if (movie->_animationChunkReference != 0) {
				_actorsByChunkReference.setVal(movie->_animationChunkReference, actor);
			}
		}
		// TODO: This datum only appears sometimes.
		uint unk2 = chunk.readTypedUint16();
		debugC(5, kDebugLoading, "Context::readHeaderSection(): Got unknown value at end of actor header section 0x%x", unk2);
		break;
	}

	case kContextFunctionSection: {
		Function *function = new Function(chunk);
		_functions.setVal(function->_id, function);
		if (!g_engine->isFirstGenerationEngine()) {
			uint endingFlag = chunk.readTypedUint16();
			if (endingFlag != 0) {
				warning("%s: Got non-zero ending flag 0x%x in function section", __func__, endingFlag);
			}
		}
		break;
	}

	case kContextUnkAtEndSection: {
		int unk1 = chunk.readTypedUint16();
		int unk2 = chunk.readTypedUint16();
		debugC(5, kDebugLoading, "Context::readHeaderSection(): unk1 = %d, unk2 = %d", unk1, unk2);
		return false;
	}

	case kContextEmptySection: {
		error("%s: EMPTY Not implemented yet", __func__);
		break;
	}

	case kContextPoohSection: {
		error("%s: POOH Not implemented yet", __func__);
		break;
	}

	default:
		error("%s: Unknown section type 0x%x (@0x%llx)", __func__, static_cast<uint>(sectionType), static_cast<long long int>(chunk.pos()));
	}

	return true;
}

} // End of namespace MediaStation
