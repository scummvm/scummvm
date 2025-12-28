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
#include "mediastation/actors/camera.h"
#include "mediastation/actors/canvas.h"
#include "mediastation/actors/palette.h"
#include "mediastation/actors/image.h"
#include "mediastation/actors/path.h"
#include "mediastation/actors/sound.h"
#include "mediastation/actors/movie.h"
#include "mediastation/actors/sprite.h"
#include "mediastation/actors/stage.h"
#include "mediastation/actors/hotspot.h"
#include "mediastation/actors/timer.h"
#include "mediastation/actors/screen.h"
#include "mediastation/actors/font.h"
#include "mediastation/actors/text.h"

namespace MediaStation {

Context::~Context() {
	for (auto it = _variables.begin(); it != _variables.end(); ++it) {
		delete it->_value;
	}
	_variables.clear();
}

void MediaStationEngine::readControlCommands(Chunk &chunk) {
	ContextSectionType sectionType = kContextEndOfSection;
	do {
		sectionType = static_cast<ContextSectionType>(chunk.readTypedUint16());
		debugC(5, kDebugLoading, "%s: sectionType = 0x%x (@0x%llx)", __func__, static_cast<uint>(sectionType), static_cast<long long int>(chunk.pos()));
		if (sectionType != kContextEndOfSection) {
			readCommandFromStream(chunk, sectionType);
		}
	} while (sectionType != kContextEndOfSection);
}

void MediaStationEngine::readCreateContextData(Chunk &chunk) {
	uint contextId = chunk.readTypedUint16();
	debugC(5, kDebugLoading, "%s: Context %d", __func__, contextId);
	Context *context = _loadedContexts.getValOrDefault(contextId);
	if (context == nullptr) {
		context = new Context();
		context->_id = contextId;
		_loadedContexts.setVal(contextId, context);
	}
}

void MediaStationEngine::readDestroyContextData(Chunk &chunk) {
	uint contextId = chunk.readTypedUint16();
	debugC(5, kDebugLoading, "%s: Context %d", __func__, contextId);
	destroyContext(contextId);
}

void MediaStationEngine::readDestroyActorData(Chunk &chunk) {
	uint actorId = chunk.readTypedUint16();
	debugC(5, kDebugLoading, "%s: Actor %d", __func__, actorId);
	destroyActor(actorId);
}

void MediaStationEngine::readActorLoadComplete(Chunk &chunk) {
	uint actorId = chunk.readTypedUint16();
	debugC(5, kDebugLoading, "%s: Actor %d", __func__, actorId);
	Actor *actor = g_engine->getActorById(actorId);
	actor->loadIsComplete();
}

void MediaStationEngine::readCreateActorData(Chunk &chunk) {
	uint contextId = chunk.readTypedUint16();
	ActorType type = static_cast<ActorType>(chunk.readTypedUint16());
	uint id = chunk.readTypedUint16();
	debugC(5, kDebugLoading, "%s: Actor %d, type 0x%x", __func__, id, static_cast<uint>(type));

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

	case kActorTypeCamera:
		actor = new CameraActor();
		break;

	case kActorTypeStage:
		actor = new StageActor();
		break;

	case kActorTypeScreen:
		actor = new ScreenActor();
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
	g_engine->registerActor(actor);
}

void MediaStationEngine::readCreateVariableData(Chunk &chunk) {
	uint contextId = chunk.readTypedUint16();
	uint id = chunk.readTypedUint16();
	if (g_engine->getVariable(id) != nullptr) {
		error("%s: Global variable %d already exists", __func__, id);
	}

	ScriptValue *value = new ScriptValue(&chunk);
	Context *context = _loadedContexts.getValOrDefault(contextId);
	if (context == nullptr) {
		error("%s: Context %d does not exist or has not been loaded yet in this title", __func__, contextId);
	}

	context->_variables.setVal(id, value);
	debugC(5, kDebugScript, "%s: %d (type: %s)", __func__, id, scriptValueTypeToStr(value->getType()));
}

void MediaStationEngine::readHeaderSections(Subfile &subfile, Chunk &chunk) {
	do {
		ChannelClient *actor = g_engine->getChannelClientByChannelIdent(chunk._id);
		if (actor == nullptr) {
			error("%s: Client \"%s\" (0x%x) does not exist or has not been read yet in this title. (@0x%llx)", __func__, tag2str(chunk._id), chunk._id, static_cast<long long int>(chunk.pos()));
		}
		if (chunk.bytesRemaining() > 0) {
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

void MediaStationEngine::readContextNameData(Chunk &chunk) {
	uint contextId = chunk.readTypedUint16();
	debugC(5, kDebugLoading, "%s: Context %d", __func__, contextId);
	Context *context = _loadedContexts.getValOrDefault(contextId);
	if (context == nullptr) {
		error("%s: Context %d does not exist or has not been loaded yet in this title", __func__, contextId);
	}
	context->_name = chunk.readTypedString();
}

void MediaStationEngine::readCommandFromStream(Chunk &chunk, ContextSectionType sectionType) {
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
		readUnrecognizedFromStream(chunk, static_cast<uint>(sectionType));
		break;
	}
}

} // End of namespace MediaStation
