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
#include "mediastation/actors/cursor.h"
#include "mediastation/actors/diskimage.h"
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

void ImtGod::readControlCommands(Chunk &chunk) {
	ContextSectionType sectionType = kContextEndOfSection;
	do {
		sectionType = static_cast<ContextSectionType>(chunk.readTypedUint16());
		debugC(5, kDebugLoading, "%s: command 0x%x", __func__, static_cast<uint>(sectionType));
		if (sectionType != kContextEndOfSection) {
			readCommandFromStream(chunk, sectionType);
		}
	} while (sectionType != kContextEndOfSection);
}

void ImtGod::readCreateContextData(Chunk &chunk) {
	uint contextId = chunk.readTypedUint16();
	debugC(5, kDebugLoading, "%s: Context %d", __func__, contextId);
	Context *context = _loadedContexts.getValOrDefault(contextId);
	if (context == nullptr) {
		context = new Context();
		context->_id = contextId;
		_loadedContexts.setVal(contextId, context);
	}
}

void ImtGod::readDestroyContextData(Chunk &chunk) {
	uint contextId = chunk.readTypedUint16();
	debugC(5, kDebugLoading, "%s: Context %d", __func__, contextId);
	destroyContext(contextId);
}

void ImtGod::readDestroyActorData(Chunk &chunk) {
	uint actorId = chunk.readTypedUint16();
	debugC(5, kDebugLoading, "[%s] %s", g_engine->formatActorName(actorId).c_str(), __func__);
	destroyActor(actorId);
}

void ImtGod::readActorLoadComplete(Chunk &chunk) {
	uint actorId = chunk.readTypedUint16();
	Actor *actor = g_engine->getImtGod()->getActorById(actorId);
	debugC(5, kDebugLoading, "[%s] %s", actor->debugName(), __func__);
	actor->loadIsComplete();
}

void ImtGod::readCreateActorData(Chunk &chunk) {
	uint contextId = chunk.readTypedUint16();
	ActorType type = static_cast<ActorType>(chunk.readTypedUint16());
	uint id = chunk.readTypedUint16();
	debugC(5, kDebugLoading, "[%s] %s: type 0x%x", g_engine->formatActorName(id).c_str(), __func__, static_cast<uint>(type));

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

	case kActorTypeCursor:
		actor = new CursorActor();
		break;

	case kActorTypeDiskImage:
		actor = new DiskImageActor();
		break;

	default:
		error("%s: No class for actor type 0x%x", __func__, static_cast<uint>(type));
	}
	actor->setId(id);
	actor->setContextId(contextId);
	actor->initFromParameterStream(chunk);
	g_engine->getImtGod()->addConstructedActor(actor);
}

void ImtGod::readCreateVariableData(Chunk &chunk) {
	uint contextId = chunk.readTypedUint16();
	uint id = chunk.readTypedUint16();
	if (g_engine->getImtGod()->getVariable(id) != nullptr) {
		error("[%s] %s: Global variable already exists", g_engine->formatVariableName(id).c_str(), __func__);
	}

	ScriptValue *value = new ScriptValue(&chunk);
	Context *context = _loadedContexts.getValOrDefault(contextId);
	if (context == nullptr) {
		error("%s: Context %d does not exist or has not been loaded yet in this title", __func__, contextId);
	}

	context->_variables.setVal(id, value);
	debugC(5, kDebugLoading, "[%s] %s", g_engine->formatVariableName(id).c_str(), __func__);
}

void ImtGod::readHeaderSections(Subfile &subfile, Chunk &chunk) {
	do {
		debugC(5, kDebugLoading, "[%s] %s", g_engine->formatAssetNameForChannelIdent(chunk._id).c_str(), __func__);
		ChannelClient *actor = g_engine->getImtGod()->getChannelClientByChannelIdent(chunk._id);
		if (actor == nullptr) {
			error("%s: Client %s does not exist or has not been read yet in this title",
				__func__, g_engine->formatAssetNameForChannelIdent(chunk._id).c_str());
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

void ImtGod::readSetContextName(Chunk &chunk) {
	uint contextId = chunk.readTypedUint16();
	debugC(5, kDebugLoading, "%s: Context %d", __func__, contextId);
	Context *context = _loadedContexts.getValOrDefault(contextId);
	if (context == nullptr) {
		error("%s: Context %d does not exist or has not been loaded yet in this title", __func__, contextId);
	}
	context->_name = chunk.readTypedString();
}

void ImtGod::readCommandFromStream(Chunk &chunk, ContextSectionType sectionType) {
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
		readSetContextName(chunk);
		break;

	default:
		readUnrecognizedFromStream(chunk, static_cast<uint>(sectionType));
		break;
	}
}

} // End of namespace MediaStation
