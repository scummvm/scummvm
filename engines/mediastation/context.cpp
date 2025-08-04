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
#include "mediastation/mediascript/function.h"
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

	// Read headers and actors in the first subfile.
	Subfile subfile = getNextSubfile();
	Chunk chunk = subfile.nextChunk();
	readHeaderSections(subfile, chunk);

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
	for (auto it = _actors.begin(); it != _actors.end(); ++it) {
		delete it->_value;
	}
	_actors.clear();
	// The same actor pointers are in here, so don't delete again.
	_actorsByChunkReference.clear();

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

ScriptValue *Context::getVariable(uint variableId) {
	return _variables.getValOrDefault(variableId);
}

void Context::readControlCommands(Chunk &chunk) {
	ContextSectionType command = kEndOfContextData;
	do {
		command = static_cast<ContextSectionType>(chunk.readTypedUint16());
		if (command != kEndOfContextData) {
			readCommandFromStream(command, chunk);
		}
	} while (command != kEndOfContextData);
}

void Context::readCreateContextData(Chunk &chunk) {
	// The original had contexts created from the base engine class,
	// but things are currently structured a bit differently, so this
	// is a no-op for now.
	_id = chunk.readTypedUint16();
}

void Context::readDestroyContextData(Chunk &chunk) {
	uint contextId = chunk.readTypedUint16();
	g_engine->releaseContext(contextId);
}

void Context::readDestroyActorData(Chunk &chunk) {
	uint actorId = chunk.readTypedUint16();
	g_engine->destroyActor(actorId);
}

void Context::readActorLoadComplete(Chunk &chunk) {
	uint actorId = chunk.readTypedUint16();
	Actor *actor = g_engine->getActorById(actorId);
	actor->loadIsComplete();
}

void Context::readCreateActorData(Chunk &chunk) {
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
}

void Context::readCreateVariableData(Chunk &chunk) {
	uint contextId = chunk.readTypedUint16();
	if (contextId != _id) {
		warning("%s: Repeated context ID didn't match: %d != %d", __func__, contextId, _id);
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

void Context::readHeaderSections(Subfile &subfile, Chunk &chunk) {
	do {
		if (chunk._id == MKTAG('i', 'g', 'o', 'd')) {
			StreamType streamType = static_cast<StreamType>(chunk.readTypedUint16());
			if (streamType != kControlCommandsStream) {
				error("%s: Expected header chunk, got %s (@0x%llx)", __func__, tag2str(chunk._id), static_cast<long long int>(chunk.pos()));
			}

			readControlCommands(chunk);
		} else {
			Actor *actor = getActorByChunkReference(chunk._id);
			if (actor == nullptr) {
				// We should only need to look in the global scope when there is an
				// install cache (INSTALL.CXT).
				actor = g_engine->getActorByChunkReference(chunk._id);
				if (actor == nullptr) {
					error("%s: Actor for chunk \"%s\" (0x%x) does not exist or has not been read yet in this title. (@0x%llx)", __func__, tag2str(chunk._id), chunk._id, static_cast<long long int>(chunk.pos()));
				}
			}
			actor->readChunk(chunk);
		}

		if (chunk.bytesRemaining() != 0) {
			warning("%s: %d bytes remaining at end of chunk", __func__, chunk.bytesRemaining());
		}

		if (!subfile.atEnd()) {
			chunk = subfile.nextChunk();
		}
	} while (!subfile.atEnd());
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

void Context::readContextNameData(Chunk &chunk) {
	uint contextId = chunk.readTypedUint16();
	if (contextId != _id) {
		warning("%s: Repeated context ID didn't match: %d != %d", __func__, contextId, _id);
	}

	_name = chunk.readTypedString();
}

void Context::readCommandFromStream(ContextSectionType sectionType, Chunk &chunk) {
	debugC(5, kDebugLoading, "%s: %d", __func__, static_cast<uint>(sectionType));
	switch (sectionType) {
	case kContextCreateData:
		readCreateContextData(chunk);
		break;

	case kContextDestroyData:
		readDestroyContextData(chunk);
		break;

	case kContextCreateActorData:
		readCreateActorData(chunk);
		break;

	case kContextDestroyActorData:
		readDestroyActorData(chunk);
		break;

	case kContextActorLoadComplete:
		readActorLoadComplete(chunk);
		break;

	case kContextCreateVariableData:
		readCreateVariableData(chunk);
		break;

	case kContextNameData:
		readContextNameData(chunk);
		break;

	default:
		g_engine->readUnrecognizedFromStream(chunk, static_cast<uint>(sectionType));
	}
}

} // End of namespace MediaStation
