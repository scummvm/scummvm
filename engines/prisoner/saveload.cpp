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
 * $URL: https://www.switchlink.se/svn/picture/script.cpp $
 * $Id: script.cpp 4 2008-08-08 14:21:30Z johndoe $
 *
 */

#include "common/events.h"
#include "common/keyboard.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"

#include "base/plugins.h"
#include "base/version.h"

#include "graphics/thumbnail.h"

#include "sound/mixer.h"

#include "prisoner/prisoner.h"
#include "prisoner/path.h"
#include "prisoner/resource.h"
#include "prisoner/resourcemgr.h"
#include "prisoner/screen.h"

namespace Prisoner {

#define PRISONER_SAVEGAME_VERSION 0 // 0 is dev version until in official SVN

void writeString(Common::WriteStream *out, const Common::String &str) {
	out->writeByte(str.size());
	out->write(str.c_str(), str.size());
}

Common::String readString(Common::ReadStream *stream) {
	Common::String str;
	int len = stream->readByte();
	while (len--)
		str += (char)stream->readByte();
	return str;
}

void writeResourceCacheSlotInfo(int16 resourceCacheSlot, ResourceManager *res, Common::WriteStream *out) {
	Common::String pakName;
	int16 pakSlot, resType;
	res->getSlotInfo(resourceCacheSlot, pakName, pakSlot, &resType);
	writeString(out, pakName);
	out->writeUint16LE(pakSlot);
	out->writeUint16LE(resType);
}

template<class T>
int16 loadResourceCacheSlotInfo(ResourceManager *res, Common::ReadStream *in) {
	Common::String pakName;
	int16 pakSlot, resType;
	pakName = readString(in);
	pakSlot = in->readUint16LE();
	resType = in->readUint16LE();
	return res->load<T>(pakName, pakSlot, resType);
}

void readResourceCacheSlotInfo(Common::ReadStream *in, Common::String &pakName, int16 &pakSlot) {
	pakName = readString(in);
	pakSlot = in->readUint16LE();
	in->readUint16LE(); // skip resType
}

void Zone::save(PrisonerEngine *vm, Common::WriteStream *out) {
	out->writeByte(used);
	if (used) {
		out->writeUint16LE(x1);
		out->writeUint16LE(y1);
		out->writeUint16LE(x2);
		out->writeUint16LE(y2);
		out->writeByte(type);
		out->writeUint16LE(actorIndex);
		out->writeUint16LE(mouseCursor);
		out->writeByte(hasText);
		out->writeUint16LE(textIndex);
		writeString(out, identifier);
		out->writeUint16LE(resourceCacheSlot);
		if (hasText) {
			writeResourceCacheSlotInfo(resourceCacheSlot, vm->_res, out);
		}
	}
}

void Zone::load(PrisonerEngine *vm, Common::ReadStream *in) {
	used = in->readByte();
	if (used) {
		x1 = in->readUint16LE();
		y1 = in->readUint16LE();
		x2 = in->readUint16LE();
		y2 = in->readUint16LE();
		type = in->readByte();
		actorIndex = in->readUint16LE();
		mouseCursor = in->readUint16LE();
		hasText = in->readByte();
		textIndex = in->readUint16LE();
		identifier = readString(in);
		resourceCacheSlot = in->readUint16LE();
		if (hasText) {
			Common::String pakName;
			int16 pakSlot;
			readResourceCacheSlotInfo(in, pakName, pakSlot);
			resourceCacheSlot = vm->loadTextResource(pakName, pakSlot);
		}
	}
}

void ZoneAction::save(PrisonerEngine *vm, Common::WriteStream *out) {
	out->writeUint16LE(used);
	if (used) {
		out->writeUint16LE(zoneActionIndex);
		out->writeUint16LE(zoneIndex);
		out->writeUint16LE(type);
		out->writeUint16LE(inventoryItemIndex);
		out->writeUint16LE(pathNodeIndex);
		out->writeUint16LE(scriptIndex1);
		out->writeUint16LE(scriptProgIndex);
		out->writeUint16LE(scriptIndex2);
		out->writeByte(0/*unk2*/);
		out->writeUint16LE(0/*unk3*/);
		out->writeUint16LE(0/*unk4*/);
		out->writeUint16LE(moduleIndex);
		out->writeUint16LE(sceneIndex);
	}
}

void ZoneAction::load(PrisonerEngine *vm, Common::ReadStream *in) {
	used = in->readUint16LE();
	if (used) {
		zoneActionIndex = in->readUint16LE();
		zoneIndex = in->readUint16LE();
		type = in->readUint16LE();
		inventoryItemIndex = in->readUint16LE();
		pathNodeIndex = in->readUint16LE();
		scriptIndex1 = in->readUint16LE();
		scriptProgIndex = in->readUint16LE();
		scriptIndex2 = in->readUint16LE();
		/*unk2 = */in->readByte();
		/*unk3 = */in->readUint16LE();
		/*unk4 = */in->readUint16LE();
		moduleIndex = in->readUint16LE();
		sceneIndex = in->readUint16LE();
	}
}

void ScriptProgram::save(PrisonerEngine *vm, Common::WriteStream *out) {
	out->writeUint16LE(resourceCacheSlot);
	if (resourceCacheSlot != -1) {
		writeResourceCacheSlotInfo(resourceCacheSlot, vm->_res, out);
		for (int16 i = 0; i < kMaxScripts; i++) {
			Script *script = &scripts[i];
			out->writeByte(script->status);
			if (script->status != kScriptStatusPaused) {
				out->writeUint32LE(script->ip - script->code);
				out->writeUint16LE(script->soundItemIndex);
				out->writeUint16LE(script->zoneIndex);
				out->writeByte(script->zoneEnterLeaveFlag);
				out->writeUint16LE(script->screenTextIndex);
				out->writeUint32LE(script->sleepCounter);
				out->writeUint16LE(script->syncScriptNumber);
				out->writeUint16LE(script->actorIndex);
				out->writeUint16LE(script->actorIndex2);
				out->writeUint16LE(script->altAnimationIndex);
				out->writeUint16LE(script->frameIndex);
				out->writeUint16LE(0/*script->unk3*/);
			}
		}
	}
}

void ScriptProgram::load(PrisonerEngine *vm, Common::ReadStream *in) {
	resourceCacheSlot = in->readUint16LE();
	if (resourceCacheSlot != -1) {
		resourceCacheSlot = loadResourceCacheSlotInfo<ScriptResource>(vm->_res, in);
		scriptResource = vm->_res->get<ScriptResource>(resourceCacheSlot);
		scriptCount = scriptResource->getCount() - 1;
		for (int16 i = 0; i < kMaxScripts; i++) {
			Script *script = &scripts[i];
			script->clear();
			script->status = in->readByte();
			if (script->status != kScriptStatusPaused) {
				uint32 codeOffs;
				codeOffs = in->readUint32LE();
				script->code = scriptResource->getScript(i);
				script->ip = script->code + codeOffs;
				script->soundItemIndex = in->readUint16LE();
				script->zoneIndex = in->readUint16LE();
				script->zoneEnterLeaveFlag = in->readByte();
				script->screenTextIndex = in->readUint16LE();
				script->sleepCounter = in->readUint32LE();
				script->syncScriptNumber = in->readUint16LE();
				script->actorIndex = in->readUint16LE();
				script->actorIndex2 = in->readUint16LE();
				script->altAnimationIndex = in->readUint16LE();
				script->frameIndex = in->readUint16LE();
				/*script->unk3 = */in->readUint16LE();
			}
		}
	}
}

void InventoryItem::save(PrisonerEngine *vm, Common::WriteStream *out) {
	out->writeUint16LE(resourceCacheSlot);
	if (resourceCacheSlot != -1) {
		out->writeUint16LE(id);
		out->writeByte(status);
		writeString(out, name);
		out->writeUint16LE(combinationIndex);
		writeResourceCacheSlotInfo(resourceCacheSlot, vm->_res, out);
	}
}

void InventoryItem::load(PrisonerEngine *vm, Common::ReadStream *in) {
	resourceCacheSlot = in->readUint16LE();
	if (resourceCacheSlot != -1) {
		id = in->readUint16LE();
		status = in->readByte();
		name = readString(in);
		combinationIndex = in->readUint16LE();
		resourceCacheSlot = loadResourceCacheSlotInfo<AnimationResource>(vm->_res, in);
	}
}

void InventoryItemCombination::save(PrisonerEngine *vm, Common::WriteStream *out) {
	out->writeByte(used);
	if (used) {
		out->writeUint16LE(inventoryItem1);
		out->writeUint16LE(inventoryItem2);
		out->writeUint16LE(scriptIndex);
	}
}

void InventoryItemCombination::load(PrisonerEngine *vm, Common::ReadStream *in) {
	used = in->readByte();
	if (used) {
		inventoryItem1 = in->readUint16LE();
		inventoryItem2 = in->readUint16LE();
		scriptIndex = in->readUint16LE();
	}
}

void ActorSprite::save(PrisonerEngine *vm, Common::WriteStream *out) {
	out->writeByte(used);
	if (used) {
		out->writeUint16LE(actorIndex);
		out->writeUint16LE(x);
		out->writeUint16LE(y);
		out->writeUint16LE(xsub);
		out->writeUint16LE(ysub);
		out->writeUint16LE(xadd);
		out->writeUint16LE(yadd);
		out->writeUint16LE(xoffs);
		out->writeUint16LE(yoffs);
		out->writeUint16LE(scale);
		out->writeUint16LE(boundsX1);
		out->writeUint16LE(boundsY1);
		out->writeUint16LE(boundsX2);
		out->writeUint16LE(boundsY2);
		out->writeUint16LE(zoneX1);
		out->writeUint16LE(zoneY1);
		out->writeUint16LE(zoneX2);
		out->writeUint16LE(zoneY2);
		out->writeByte(flag);
		out->writeUint16LE(elementIndex);
		out->writeUint16LE(prevFrameIndex);
		out->writeUint16LE(frameIndex);
		out->writeUint16LE(frameCount);
		out->writeUint16LE(ticks);
		out->writeUint16LE(frameListIndex);
		out->writeUint16LE(frameListCount);
	}
}

void ActorSprite::load(PrisonerEngine *vm, Common::ReadStream *in) {
	used = in->readByte();
	if (used) {
		actorIndex = in->readUint16LE();
		x = in->readUint16LE();
 		y = in->readUint16LE();
	 	xsub = in->readUint16LE();
		ysub = in->readUint16LE();
		xadd = in->readUint16LE();
		yadd = in->readUint16LE();
		xoffs = in->readUint16LE();
		yoffs = in->readUint16LE();
		scale = in->readUint16LE();
		boundsX1 = in->readUint16LE();
		boundsY1 = in->readUint16LE();
		boundsX2 = in->readUint16LE();
		boundsY2 = in->readUint16LE();
		zoneX1 = in->readUint16LE();
		zoneY1 = in->readUint16LE();
		zoneX2 = in->readUint16LE();
		zoneY2 = in->readUint16LE();
		flag = in->readByte();
		elementIndex = in->readUint16LE();
		prevFrameIndex = in->readUint16LE();
		frameIndex = in->readUint16LE();
		frameCount = in->readUint16LE();
		ticks = in->readUint16LE();
		frameListIndex = in->readUint16LE();
		frameListCount = in->readUint16LE();
	}
}

void Actor::save(PrisonerEngine *vm, Common::WriteStream *out) {
	out->writeUint16LE(resourceCacheSlot);
	if (resourceCacheSlot != -1) {
		writeResourceCacheSlotInfo(resourceCacheSlot, vm->_res, out);
		out->writeUint16LE(altAnimationIndex);
		out->writeUint16LE(status);
		out->writeUint16LE(pathWalkerIndex);
		out->writeUint16LE(pathResultIndex);
		out->writeUint16LE(pathResultCount);
		if (pathWalkerIndex >= 0) {
			PathResult *pathResult = pathWalker2;
			for (int j = 0; j < pathResultCount; j++) {
				out->writeUint16LE(pathResult->x);
				out->writeUint16LE(pathResult->y);
				out->writeUint16LE(pathResult->edgeIndex);
				out->writeUint16LE(pathResult->nodeIndex);
				out->writeUint16LE(pathResult->polyIndex);
				out->writeUint16LE(pathResult->direction);
				out->writeUint16LE(pathResult->scale);
				pathResult++;
			}
		}
		out->writeUint16LE(pathNodeIndex);
		out->writeUint16LE(pathEdgeIndex);
		out->writeUint16LE(pathPolyIndex);
		out->writeUint16LE(firstFrameListIndex);
		out->writeUint16LE(lastFrameListIndex);
		out->writeUint16LE(minTicks);
		out->writeUint16LE(maxTicks);
		out->writeUint32LE(ticks);
		out->writeByte(ticksFlag);
		writeString(out, pakName);
		out->writeUint16LE(pakSlot);
		out->writeUint16LE(frameListIndex);
		out->writeUint16LE(walkDestX);
		out->writeUint16LE(walkDestY);
		out->writeUint16LE(x2);
		out->writeUint16LE(y2);
		out->writeUint16LE(textFontNumber);
		out->writeUint16LE(fontOutlineColor);
		out->writeUint16LE(fontInkColor);
		//ActorSprite *actorSprite;
		out->writeUint16LE(x);
		out->writeUint16LE(y);
	}
}

void Actor::load(PrisonerEngine *vm, Common::ReadStream *in) {
	resourceCacheSlot = in->readUint16LE();
	if (resourceCacheSlot != -1) {
		resourceCacheSlot = loadResourceCacheSlotInfo<AnimationResource>(vm->_res, in);
		altAnimationIndex = in->readUint16LE();
		status = in->readUint16LE();
		pathWalkerIndex = in->readUint16LE();
		pathResultIndex = in->readUint16LE();
		pathWalker1 = NULL;
		pathWalker2 = NULL;
		pathResultCount = in->readUint16LE();
		// Assign pathwalker and load the path result data
		if (pathWalkerIndex >= 0) {
			// ----------------------------------------------------------
			// TODO: Own function
			pathWalkerIndex = vm->_pathWalkers.getFreeSlot();
			vm->_pathWalkers[pathWalkerIndex].used = 1;
			pathWalker1 = &vm->_pathWalkers[pathWalkerIndex].items[pathResultIndex];
			pathWalker2 = &vm->_pathWalkers[pathWalkerIndex].items[0];
			// ----------------------------------------------------------
			PathResult *pathResult = pathWalker2;
			for (int j = 0; j < pathResultCount; j++) {
				pathResult->x = in->readUint16LE();
				pathResult->y = in->readUint16LE();
				pathResult->edgeIndex = in->readUint16LE();
				pathResult->nodeIndex = in->readUint16LE();
				pathResult->polyIndex = in->readUint16LE();
				pathResult->direction = in->readUint16LE();
				pathResult->scale = in->readUint16LE();
				pathResult++;
			}
		}
		pathNodeIndex = in->readUint16LE();
		pathEdgeIndex = in->readUint16LE();
		pathPolyIndex = in->readUint16LE();
		firstFrameListIndex = in->readUint16LE();
		lastFrameListIndex = in->readUint16LE();
		minTicks = in->readUint16LE();
		maxTicks = in->readUint16LE();
		ticks = in->readUint32LE();
		ticksFlag = in->readByte();
		pakName = readString(in);
		pakSlot = in->readUint16LE();
		frameListIndex = in->readUint16LE();
		walkDestX = in->readUint16LE();
		walkDestY = in->readUint16LE();
		x2 = in->readUint16LE();
		y2 = in->readUint16LE();
		textFontNumber = in->readUint16LE();
		fontOutlineColor = in->readUint16LE();
		fontInkColor = in->readUint16LE();
		x = in->readUint16LE();
		y = in->readUint16LE();
	}
}

void AltActorAnimation::save(PrisonerEngine *vm, Common::WriteStream *out) {
	out->writeUint16LE(resourceCacheSlot);
	if (resourceCacheSlot != -1) {
		out->writeByte(value);
		writeResourceCacheSlotInfo(resourceCacheSlot, vm->_res, out);
	}
}

void AltActorAnimation::load(PrisonerEngine *vm, Common::ReadStream *in) {
	resourceCacheSlot = in->readUint16LE();
	if (resourceCacheSlot != -1) {
		value = in->readByte();
		debug("AltActorAnimation::load>");
		resourceCacheSlot = loadResourceCacheSlotInfo<AnimationResource>(vm->_res, in);
		debug("AltActorAnimation::load<");
	}
}

void SceneItem::save(PrisonerEngine *vm, Common::WriteStream *out) {
	out->writeUint16LE(actorIndex);
	if (actorIndex != -1)
		out->writeUint16LE(inventoryItemIndex);
}

void SceneItem::load(PrisonerEngine *vm, Common::ReadStream *in) {
	actorIndex = in->readUint16LE();
	if (actorIndex != -1)
		inventoryItemIndex = in->readUint16LE();
}

void ActorFrameSound::save(PrisonerEngine *vm, Common::WriteStream *out) {
	out->writeUint16LE(actorIndex);
	if (actorIndex != -1) {
		out->writeUint16LE(soundIndex);
		out->writeUint16LE(frameNum);
		out->writeUint16LE(unk1);
		out->writeUint16LE(volume);
		out->writeByte(unk2);
	}
}

void ActorFrameSound::load(PrisonerEngine *vm, Common::ReadStream *in) {
	actorIndex = in->readUint16LE();
	if (actorIndex != -1) {
		soundIndex = in->readUint16LE();
		frameNum = in->readUint16LE();
		unk1 = in->readUint16LE();
		volume = in->readUint16LE();
		unk2 = in->readByte();
	}
}

void PaletteTask::save(PrisonerEngine *vm, Common::WriteStream *out) {
	out->writeByte(active);
	out->writeUint16LE(value1);
	out->writeUint16LE(value2);
	out->writeUint16LE(value3);
	out->writeUint16LE(positionIncr);
}

void PaletteTask::load(PrisonerEngine *vm, Common::ReadStream *in) {
	active = in->readByte();
	value1 = in->readUint16LE();
	value2 = in->readUint16LE();
	value3 = in->readUint16LE();
	positionIncr = in->readUint16LE();
	updateTicks = 0;
}

void Dialog::save(PrisonerEngine *vm, Common::WriteStream *out) {
	out->writeByte(used);
	if (used) {
		writeString(out, pakName);
		out->writeUint16LE(pakSlot);
		for (uint i = 0; i < keywords.size(); i++) {
			out->writeByte(keywords[i]._used);
		}
	}
}

void Dialog::load(PrisonerEngine *vm, Common::ReadStream *in) {
	used = in->readByte();
	if (used) {
		pakName = readString(in);
		pakSlot = in->readUint16LE();
		vm->loadDialogKeywords(pakName, pakSlot, this, false);
		for (uint i = 0; i < keywords.size(); i++) {
			keywords[i]._used = in->readByte();
		}
	}
}

void FontColorDef::save(PrisonerEngine *vm, Common::WriteStream *out) {
	out->writeByte(inkColor);
	out->writeByte(outlineColor);
}

void FontColorDef::load(PrisonerEngine *vm, Common::ReadStream *in) {
	inkColor = in->readByte();
	outlineColor = in->readByte();
}

void ScreenText::save(PrisonerEngine *vm, Common::WriteStream *out) {
	out->writeByte(used);
	if (used) {
		out->writeByte(screenText);
		writeString(out, identifier);
		out->writeUint16LE(x0);
		out->writeUint16LE(y0);
		out->writeByte(screenCenter);
		out->writeUint16LE(resourceCacheSlot);
		out->writeUint16LE(actorIndex);
		out->writeUint32LE(finishedTime);
		out->writeUint16LE(speechPakSlot);
		out->writeUint16LE(chunkCount);
		out->writeUint16LE(chunkIndex);
		out->writeByte(inventoryActive);
		out->writeUint16LE(textIndex);
		out->writeUint16LE(fontIndex);
		out->writeUint16LE(fontOutlineColor);
		out->writeUint16LE(fontInkColor);
		out->writeUint16LE(x);
		out->writeUint16LE(y);
		out->writeUint16LE(lineCount);
		out->writeUint16LE(width);
		out->writeUint16LE(height);
		out->writeUint16LE(fontHeight);
		writeResourceCacheSlotInfo(resourceCacheSlot, vm->_res, out);
	}
}

void ScreenText::load(PrisonerEngine *vm, Common::ReadStream *in) {
	used = in->readByte();
	if (used) {
		screenText = in->readByte();
		identifier = readString(in);
		x0 = in->readUint16LE();
		y0 = in->readUint16LE();
		screenCenter = in->readByte();
		resourceCacheSlot = in->readUint16LE();
		actorIndex = in->readUint16LE();
		finishedTime = in->readUint32LE();
		speechPakSlot = in->readUint16LE() - 1; // Dec since the slot is already set to the next slot in-game
		chunkCount = in->readUint16LE();
		chunkIndex = in->readUint16LE();
		inventoryActive = in->readByte();
		textIndex = in->readUint16LE();
		fontIndex = in->readUint16LE();
		fontOutlineColor = in->readUint16LE();
		fontInkColor = in->readUint16LE();
		x = in->readUint16LE();
		y = in->readUint16LE();
		lineCount = in->readUint16LE();
		width = in->readUint16LE();
		height = in->readUint16LE();
		fontHeight = in->readUint16LE();
		resourceCacheSlot = loadResourceCacheSlotInfo<TextResource>(vm->_res, in);
	}
}

PrisonerEngine::kReadSaveHeaderError PrisonerEngine::readSaveHeader(Common::SeekableReadStream *in, bool loadThumbnail, SaveHeader &header) {

	header.version = in->readUint32LE();
	if (header.version != PRISONER_SAVEGAME_VERSION)
		return kRSHEInvalidVersion;

	byte descriptionLen = in->readByte();
	header.description = "";
	while (descriptionLen--)
		header.description += (char)in->readByte();

	if (loadThumbnail) {
		header.thumbnail = new Graphics::Surface();
		assert(header.thumbnail);
		if (!Graphics::loadThumbnail(*in, *header.thumbnail)) {
			delete header.thumbnail;
			header.thumbnail = 0;
		}
	} else {
		Graphics::skipThumbnail(*in);
	}

	// Not used yet, reserved for future usage
	header.gameID = in->readByte();
	header.flags = in->readUint32LE();

	return ((in->eos() || in->err()) ? kRSHEIoError : kRSHENoError);
}

void PrisonerEngine::savegame(const char *filename, const char *description) {

	Common::OutSaveFile *out;
	if (!(out = g_system->getSavefileManager()->openForSaving(filename))) {
		warning("Can't create file '%s', game not saved", filename);
		return;
	}

	out->writeUint32LE(PRISONER_SAVEGAME_VERSION);

	byte descriptionLen = strlen(description);
	out->writeByte(descriptionLen);
	out->write(description, descriptionLen);

	Graphics::saveThumbnail(*out);

	// Not used yet, reserved for future usage
	out->writeByte(0);
	out->writeUint32LE(0);

	// Module & Background
	out->writeUint16LE(_currModuleIndex);
	out->writeUint16LE(_currSceneIndex);
	out->writeUint16LE(_newModuleIndex);
	out->writeUint16LE(_newSceneIndex);
	out->writeUint16LE(_prevModuleIndex);
	out->writeUint16LE(_prevSceneIndex);
	out->writeUint16LE(_inventoryItemIndex2);
	out->writeUint16LE(_backgroundResourceCacheSlot);
	if (_backgroundResourceCacheSlot != -1) {
		writeResourceCacheSlotInfo(_backgroundResourceCacheSlot, _res, out);
	}
	out->writeUint16LE(_cameraX);
	out->writeUint16LE(_cameraY);
	out->writeByte(_backgroundCameraLocked);
	out->writeUint16LE(_cameraFollowsActorIndex);
	out->writeUint16LE(_backgroundObjectsResourceCacheSlot);
	if (_backgroundObjectsResourceCacheSlot != -1) {
		writeResourceCacheSlotInfo(_backgroundObjectsResourceCacheSlot, _res, out);
	}

	// Script variables
	for (uint i = 0; i < 250; i++)
		out->writeUint16LE(_globalScriptVars[i]);
	for (uint i = 0; i < 300; i++)
		out->writeUint16LE(_moduleScriptVars[i]);

	// Palette
	// TODO: Unknown palette struct "stru_761EC" seems unused
	out->writeByte(_scenePaletteOk);
	out->write(_scenePalette, 768);
	out->writeByte(_effectPaletteOk);
	out->write(_effectPalette, 768);

	// Palette tasks
	for (int i = 0; i < kMaxPaletteTasks; i++)
		_paletteTasks[i].save(this, out);

	// Path system
	_pathSystem->saveState(out);

	// Scene items
	_sceneItems.save(this, out);

	// Inventory item combinations
	_inventoryItemCombinations.save(this, out);

	// Inventory
	out->writeUint16LE(_inventoryItemCursor); // TODO: sg_inventoryItemCursor
	// TODO: "someArray" doesn't seem to be used
	out->writeUint16LE(_inventoryItemsCount);
	_inventoryItems.save(this, out);
	out->writeUint16LE(_inventoryItemSlotBaseIndex);
	out->writeUint16LE(_inventoryItemsResourceCacheIndex);
	if (_inventoryItemsResourceCacheIndex != -1) {
		writeResourceCacheSlotInfo(_inventoryItemsResourceCacheIndex, _res, out);
	}

	// Actor frame sounds
	out->writeUint16LE(_actorFrameSoundItemsCount);
	if (_actorFrameSoundItemsCount > 0) {
		_actorFrameSounds.save(this, out);
	}

	// Actors
	out->writeByte(_exitZoneActionFlag);
	out->writeUint16LE(_mainActorIndex);
	out->writeByte(_mainActorValid);
	out->writeUint16LE(_cameraDeltaX);
	out->writeUint16LE(_cameraDeltaY);

	// Actor sprites
	_actorSprites.save(this, out);
	// Actors
	_actors.save(this, out);
	// Alt actor animations
	_altActorAnimations.save(this, out);

	// Zones
	_zones.save(this, out);
	out->writeUint16LE(_zoneIndexAtMouse);

	// Main actor saved path/queued action
	out->writeByte(_actorPathFlag);
	out->writeUint16LE(_actorPathX);
	out->writeUint16LE(_actorPathY);
	_zoneActionItem.save(this, out);

	// Zone actions
	_queuedZoneAction.save(this, out);
	_zoneActions.save(this, out);
	out->writeUint16LE(_currZoneActionIndex);

	// LipSync
	out->writeUint16LE(_lipSyncX);
	out->writeUint16LE(_lipSyncY);
	out->writeUint16LE(_lipSyncScriptNumber);
	out->writeUint16LE(_lipSyncActorIndex);
	out->writeUint16LE(_lipSyncResourceCacheSlot);
	if (_lipSyncResourceCacheSlot != -1) {
		writeResourceCacheSlotInfo(_lipSyncResourceCacheSlot, _res, out);
		out->writeUint16LE(_lipSyncChannelStatus.size());
		for (uint i = 0; i < _lipSyncChannelStatus.size(); i++) {
			out->writeUint16LE(_lipSyncChannelStatus[i].ticks);
			out->writeUint16LE(_lipSyncChannelStatus[i].index);
		}
	}

	// Scripts
	out->writeUint16LE(_enterSceneScriptIndex);
	out->writeUint16LE(_enterSceneScriptProgramIndex);
	out->writeUint16LE(_leaveSceneScriptIndex);
	out->writeUint16LE(_leaveSceneScriptProgramIndex);
	_scriptPrograms[0].save(this, out);
	_scriptPrograms[1].save(this, out);

	// Dialog
	_dialogs.save(this, out);
	for (uint i = 0; i < 50; i++)
		out->writeUint16LE(_currDialogKeywordIndices[i]);
	out->writeUint16LE(_dialogActiveKeywordsCount);
	out->writeUint16LE(_currDialogIndex);
	out->writeUint16LE(_dialogMaxKeywordTextWidth);
	out->writeUint16LE(_dialogFontHeight);
	out->writeUint16LE(_selectedDialogKeywordIndex);
	out->writeByte(_dialogRunning);
	out->writeByte(_dialogFlag);

	// User input counter
	out->writeUint16LE(_userInputCounter);

	// Text colors
	_screenTextFontColor.save(this, out);
	_inventoryScreenTextFontColor.save(this, out);
	_inventoryFontColor.save(this, out);
	_zoneFontColor.save(this, out);
	_dialogFontColor.save(this, out);
	_dialogHoverFontColor.save(this, out);

	// Screen texts
	// Unused: g_currTextItemIndex
	_screenTexts.save(this, out);
	// Unused: word_61674
	out->writeByte(_screenTextShowing);
	out->writeByte(_screenTextHasSpeech);

	// TODO: Music
	out->writeByte(0);

	// TODO: Sound
	out->writeByte(0);

	// TODO...

	delete out;

}

void PrisonerEngine::loadgame(const char *filename) {

	Common::InSaveFile *in;
	if (!(in = g_system->getSavefileManager()->openForLoading(filename))) {
		warning("Can't open file '%s', game not loaded", filename);
		return;
	}

	SaveHeader header;

	kReadSaveHeaderError errorCode = readSaveHeader(in, false, header);

	if (errorCode != kRSHENoError) {
		warning("Error loading savegame '%s'", filename);
		delete in;
		return;
	}

	_updateDirtyRectsFlag = true;
	_needToUpdatePalette = false;
	_backgroundFlag = 4;

	_screen->clear();

	_currModuleIndex = 0;
	_newModuleIndex = 1;

	leaveScene();

	_newSceneIndex = -1;
	_newModuleIndex = -1;

	_loadingSavegame = true;

	clearActors();

	// Module & Background
	_currModuleIndex = in->readUint16LE();
	_currSceneIndex = in->readUint16LE();
	_newModuleIndex = in->readUint16LE();
	_newSceneIndex = in->readUint16LE();
	_prevModuleIndex = in->readUint16LE();
	_prevSceneIndex = in->readUint16LE();
	_inventoryItemIndex2 = in->readUint16LE();
	_backgroundResourceCacheSlot = in->readUint16LE();
	if (_backgroundResourceCacheSlot != -1) {
		Common::String pakName;
		int16 pakSlot;
		readResourceCacheSlotInfo(in, pakName, pakSlot);
		loadBackground(pakName, pakSlot);
	}
	_cameraX = in->readUint16LE();
	_cameraY = in->readUint16LE();
	_backgroundCameraLocked = in->readByte() != 0;
	_cameraFollowsActorIndex = in->readUint16LE();
	_backgroundObjectsResourceCacheSlot = in->readUint16LE();
	if (_backgroundObjectsResourceCacheSlot != -1) {
		Common::String pakName;
		int16 pakSlot;
		readResourceCacheSlotInfo(in, pakName, pakSlot);
		setBackgroundObjects(pakName, pakSlot);
	}

	// Script variables
	for (uint i = 0; i < 250; i++)
		_globalScriptVars[i] = in->readUint16LE();
	for (uint i = 0; i < 300; i++)
		_moduleScriptVars[i] = in->readUint16LE();

	// Palette
	// TODO: Unknown palette struct "stru_761EC" seems unused
	_scenePaletteOk = in->readByte() != 0;
	in->read(_scenePalette, 768);
	_effectPaletteOk = in->readByte() != 0;
	in->read(_effectPalette, 768);
	_screen->buildPaletteTransTable(_scenePalette, 0);
	_needToUpdatePalette = true;

	// Palette tasks
	for (int i = 0; i < kMaxPaletteTasks; i++) {
		_paletteTasks[i].load(this, in);
		// TODO: Start palette task
	}

	// Path system
	_pathSystem->loadState(in);

	// Scene items
	_sceneItems.load(this, in);

	// Inventory item combinations
	_inventoryItemCombinations.load(this, in);

	// Inventory
	_inventoryItemCursor = in->readUint16LE(); // TODO: sg_inventoryItemCursor
	// TODO: "someArray" doesn't seem to be used
	_inventoryItemsCount = in->readUint16LE();
	_inventoryItems.load(this, in);
	for (int inventoryItemIndex = 0; inventoryItemIndex < _inventoryItems.count(); inventoryItemIndex++) {
		InventoryItem *inventoryItem = &_inventoryItems[inventoryItemIndex];
		if (inventoryItem->resourceCacheSlot != -1) {
			loadInventoryItemText(inventoryItemIndex);
		}
	}
	_inventoryItemSlotBaseIndex = in->readUint16LE();
	_inventoryItemsResourceCacheIndex = in->readUint16LE();
	if (_inventoryItemsResourceCacheIndex != -1) {
		_inventoryItemsResourceCacheIndex = loadResourceCacheSlotInfo<AnimationResource>(_res, in);
	}

	// Actor frame sounds
	_actorFrameSoundItemsCount = in->readUint16LE();
	if (_actorFrameSoundItemsCount > 0) {
		_actorFrameSounds.load(this, in);
	}

	// Actors
	_exitZoneActionFlag = in->readByte();
	_mainActorIndex = in->readUint16LE();
	_mainActorValid = in->readByte() != 0;
	_cameraDeltaX = in->readUint16LE();
	_cameraDeltaY = in->readUint16LE();

	// Actor sprites
	_actorSprites.load(this, in);

	// Actors
	_actors.load(this, in);
	for (int16 actorIndex = 0; actorIndex < kMaxActors; actorIndex++) {
		Actor *actor = &_actors[actorIndex];
		if (actor->resourceCacheSlot != -1) {
			ActorSprite *actorSprite = &_actorSprites[actorIndex];
			if (actorSprite->prevFrameIndex != -1) {
				if (actorSprite->prevFrameIndex > 0) {
					actorSprite->prevFrameIndex--;
				} else {
					actorSprite->prevFrameIndex = actorSprite->frameCount - 1;
				}
			}
			if (actorSprite->frameIndex > 0) {
				actorSprite->frameIndex--;
			} else {
				actorSprite->frameIndex = actorSprite->frameCount - 1;
			}
			actor->actorSprite = actorSprite;
			actor->textFontNumber = _textFont;
		}
	}

	restoreActorSprites();
	buildActorSpriteDrawQueue();

	// Alt actor animations
	_altActorAnimations.load(this, in);

	// Zones
	_zones.load(this, in);
	_zoneIndexAtMouse = in->readUint16LE();

	// Main actor saved path/queued action
	_actorPathFlag = in->readByte();
	_actorPathX = in->readUint16LE();
	_actorPathY = in->readUint16LE();
	_zoneActionItem.load(this, in);

	// Zone actions
	_queuedZoneAction.load(this, in);
	_zoneActions.load(this, in);
	_currZoneActionIndex = in->readUint16LE();

	// LipSync
	_lipSyncX = in->readUint16LE();
	_lipSyncY = in->readUint16LE();
	_lipSyncScriptNumber = in->readUint16LE();
	_lipSyncActorIndex = in->readUint16LE();
	_lipSyncResourceCacheSlot = in->readUint16LE();
	if (_lipSyncResourceCacheSlot != -1) {
		Common::String pakName;
		int16 pakSlot;
		readResourceCacheSlotInfo(in, pakName, pakSlot);
		int16 count = in->readUint16LE();
		_lipSyncChannelStatusRestored = true;
		_lipSyncChannelStatus.clear();
		_lipSyncChannelStatus.reserve(count);
		for (int i = 0; i < count; i++) {
			_lipSyncChannelStatus.push_back(LipSyncChannelStatus());
			_lipSyncChannelStatus[i].ticks = in->readUint16LE();
			_lipSyncChannelStatus[i].index = in->readUint16LE();
		}
		startLipSync(pakName, pakSlot, _lipSyncScriptNumber, _lipSyncActorIndex, _lipSyncX, _lipSyncY);
	}

	// Scripts
	_enterSceneScriptIndex = in->readUint16LE();
	_enterSceneScriptProgramIndex = in->readUint16LE();
	_leaveSceneScriptIndex = in->readUint16LE();
	_leaveSceneScriptProgramIndex = in->readUint16LE();
	_scriptPrograms[0].load(this, in);
	_scriptPrograms[1].load(this, in);

	// Dialog
	_dialogs.load(this, in);
	for (uint i = 0; i < 50; i++)
		_currDialogKeywordIndices[i] = in->readUint16LE();
	_dialogActiveKeywordsCount = in->readUint16LE();
	_currDialogIndex = in->readUint16LE();
	_dialogMaxKeywordTextWidth = in->readUint16LE();
	_dialogFontHeight = in->readUint16LE();
	_selectedDialogKeywordIndex = in->readUint16LE();
	_dialogRunning = in->readByte() != 0;
	_dialogFlag = in->readByte() != 0;
	if (_dialogRunning)
		startDialog(_currDialogIndex);

	// User input counter
	_userInputCounter = in->readUint16LE();

	// Text colors
	_screenTextFontColor.load(this, in);
	_inventoryScreenTextFontColor.load(this, in);
	_inventoryFontColor.load(this, in);
	_zoneFontColor.load(this, in);
	_dialogFontColor.load(this, in);
	_dialogHoverFontColor.load(this, in);

	// Screen texts
	// Unused: g_currTextItemIndex
	_screenTexts.load(this, in);
	// Unused: word_61674
	_screenTextShowing = in->readByte() != 0;
	_screenTextHasSpeech = in->readByte() != 0;

	// TODO: Music
	in->readByte();

	// TODO: Sound
	in->readByte();

	// TODO: ...

	_inScene = true;
	initFrameTime();
	updateScreen(true, _cameraX + _mouseX, _cameraY + _mouseY);
	_backgroundFlag = 4;
	_updateDirtyRectsFlag = true;

	delete in;

}

Common::Error PrisonerEngine::loadGameState(int slot) {
	const char *fileName = getSavegameFilename(slot);
	loadgame(fileName);
	return Common::kNoError;
}

Common::Error PrisonerEngine::saveGameState(int slot, const char *description) {
	const char *fileName = getSavegameFilename(slot);
	savegame(fileName, description);
	return Common::kNoError;
}

const char *PrisonerEngine::getSavegameFilename(int num) {
	static Common::String filename;
	filename = getSavegameFilename(_targetName, num);
	return filename.c_str();
}

Common::String PrisonerEngine::getSavegameFilename(const Common::String &target, int num) {
	assert(num >= 0 && num <= 999);

	char extension[5];
	sprintf(extension, "%03d", num);

	return target + "." + extension;
}

} // End of namespace Picture
