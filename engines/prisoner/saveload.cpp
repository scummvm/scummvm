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

#include "graphics/paletteman.h"
#include "graphics/scaler.h"
#include "graphics/thumbnail.h"

#include "audio/mixer.h"

#include "prisoner/prisoner.h"
#include "prisoner/path.h"
#include "prisoner/resource.h"
#include "prisoner/resourcemgr.h"
#include "prisoner/screen.h"


namespace Prisoner {

// Version history:
// 0 - Development format
// 1 - Common::Serializer, save date and play time in the header, music playing and loop state
//     Version 0 saves have the same layout without these fields and still load.
static const Common::Serializer::Version kSavegameVersion = 1;

// Strings are stored as a length byte followed by the characters
static void syncString(Common::Serializer &s, Common::String &str) {
	char buffer[256];
	byte length = MIN<uint>(str.size(), 255);
	s.syncAsByte(length);
	if (s.isSaving())
		memcpy(buffer, str.c_str(), length);
	s.syncBytes((byte *)buffer, length);
	if (s.isLoading())
		str = Common::String(buffer, length);
}

// Syncs the pak name, slot and type of a loaded resource. When loading, the caller
// loads the resource from them.
static void syncResourceInfo(PrisonerEngine *vm, Common::Serializer &s, int16 resourceCacheSlot,
	Common::String &pakName, int16 &pakSlot, int16 &resType) {
	if (s.isSaving())
		vm->_res->getSlotInfo(resourceCacheSlot, pakName, pakSlot, &resType);
	syncString(s, pakName);
	s.syncAsUint16LE(pakSlot);
	s.syncAsUint16LE(resType);
}

// Syncs a loaded resource. When loading, the resource is loaded again and
// resourceCacheSlot is set to its new slot.
template<class T>
static void syncResourceCacheSlot(PrisonerEngine *vm, Common::Serializer &s, int16 &resourceCacheSlot) {
	Common::String pakName;
	int16 pakSlot, resType;
	syncResourceInfo(vm, s, resourceCacheSlot, pakName, pakSlot, resType);
	if (s.isLoading())
		resourceCacheSlot = vm->_res->load<T>(pakName, pakSlot, resType);
}

void Zone::sync(PrisonerEngine *vm, Common::Serializer &s) {
	s.syncAsByte(used);
	if (used) {
		s.syncAsUint16LE(x1);
		s.syncAsUint16LE(y1);
		s.syncAsUint16LE(x2);
		s.syncAsUint16LE(y2);
		s.syncAsByte(type);
		s.syncAsUint16LE(actorIndex);
		s.syncAsUint16LE(mouseCursor);
		s.syncAsByte(hasText);
		s.syncAsUint16LE(textIndex);
		syncString(s, identifier);
		s.syncAsUint16LE(resourceCacheSlot);
		if (hasText) {
			Common::String pakName;
			int16 pakSlot, resType;
			syncResourceInfo(vm, s, resourceCacheSlot, pakName, pakSlot, resType);
			if (s.isLoading())
				resourceCacheSlot = vm->loadTextResource(pakName, pakSlot);
		}
	}
}

void ZoneAction::sync(PrisonerEngine *vm, Common::Serializer &s) {
	s.syncAsUint16LE(used);
	if (used) {
		// Values that were saved but aren't used
		byte unk2 = 0;
		uint16 unk3 = 0, unk4 = 0;
		s.syncAsUint16LE(zoneActionIndex);
		s.syncAsUint16LE(zoneIndex);
		s.syncAsUint16LE(type);
		s.syncAsUint16LE(inventoryItemIndex);
		s.syncAsUint16LE(pathNodeIndex);
		s.syncAsUint16LE(scriptIndex1);
		s.syncAsUint16LE(scriptProgIndex);
		s.syncAsUint16LE(scriptIndex2);
		s.syncAsByte(unk2);
		s.syncAsUint16LE(unk3);
		s.syncAsUint16LE(unk4);
		s.syncAsUint16LE(moduleIndex);
		s.syncAsUint16LE(sceneIndex);
	}
}

void ScriptProgram::sync(PrisonerEngine *vm, Common::Serializer &s) {
	s.syncAsUint16LE(resourceCacheSlot);
	if (resourceCacheSlot != -1) {
		syncResourceCacheSlot<ScriptResource>(vm, s, resourceCacheSlot);
		if (s.isLoading()) {
			scriptResource = vm->_res->get<ScriptResource>(resourceCacheSlot);
			scriptCount = scriptResource->getCount() - 1;
		}
		for (int16 i = 0; i < kMaxScripts; i++) {
			Script *script = &scripts[i];
			if (s.isLoading())
				script->clear();
			s.syncAsByte(script->status);
			if (script->status != kScriptStatusPaused) {
				uint32 codeOffs = 0;
				uint16 unk3 = 0;
				if (s.isSaving())
					codeOffs = script->ip - script->code;
				s.syncAsUint32LE(codeOffs);
				if (s.isLoading()) {
					script->code = scriptResource->getScript(i);
					script->ip = script->code + codeOffs;
				}
				s.syncAsUint16LE(script->soundIndex);
				s.syncAsUint16LE(script->zoneIndex);
				s.syncAsByte(script->zoneEnterLeaveFlag);
				s.syncAsUint16LE(script->screenTextIndex);
				s.syncAsUint32LE(script->sleepCounter);
				s.syncAsUint16LE(script->syncScriptNumber);
				s.syncAsUint16LE(script->actorIndex);
				s.syncAsUint16LE(script->actorIndex2);
				s.syncAsUint16LE(script->altAnimationIndex);
				s.syncAsUint16LE(script->frameIndex);
				s.syncAsUint16LE(unk3);
			}
		}
	}
}

void InventoryItem::sync(PrisonerEngine *vm, Common::Serializer &s) {
	s.syncAsUint16LE(resourceCacheSlot);
	if (resourceCacheSlot != -1) {
		s.syncAsUint16LE(id);
		s.syncAsByte(status);
		syncString(s, name);
		s.syncAsUint16LE(combinationIndex);
		syncResourceCacheSlot<AnimationResource>(vm, s, resourceCacheSlot);
	}
}

void InventoryItemCombination::sync(PrisonerEngine *vm, Common::Serializer &s) {
	s.syncAsByte(used);
	if (used) {
		s.syncAsUint16LE(inventoryItem1);
		s.syncAsUint16LE(inventoryItem2);
		s.syncAsUint16LE(scriptIndex);
	}
}

void ActorSprite::sync(PrisonerEngine *vm, Common::Serializer &s) {
	s.syncAsByte(used);
	if (used) {
		s.syncAsUint16LE(actorIndex);
		s.syncAsUint16LE(x);
		s.syncAsUint16LE(y);
		s.syncAsUint16LE(xsub);
		s.syncAsUint16LE(ysub);
		s.syncAsUint16LE(xadd);
		s.syncAsUint16LE(yadd);
		s.syncAsUint16LE(xoffs);
		s.syncAsUint16LE(yoffs);
		s.syncAsUint16LE(scale);
		s.syncAsUint16LE(boundsX1);
		s.syncAsUint16LE(boundsY1);
		s.syncAsUint16LE(boundsX2);
		s.syncAsUint16LE(boundsY2);
		s.syncAsUint16LE(zoneX1);
		s.syncAsUint16LE(zoneY1);
		s.syncAsUint16LE(zoneX2);
		s.syncAsUint16LE(zoneY2);
		s.syncAsByte(flag);
		s.syncAsUint16LE(elementIndex);
		s.syncAsUint16LE(prevFrameIndex);
		s.syncAsUint16LE(frameIndex);
		s.syncAsUint16LE(frameCount);
		s.syncAsUint16LE(ticks);
		s.syncAsUint16LE(frameListIndex);
		s.syncAsUint16LE(frameListCount);
	}
}

void Actor::sync(PrisonerEngine *vm, Common::Serializer &s) {
	s.syncAsUint16LE(resourceCacheSlot);
	if (resourceCacheSlot != -1) {
		syncResourceCacheSlot<AnimationResource>(vm, s, resourceCacheSlot);
		s.syncAsUint16LE(altAnimationIndex);
		s.syncAsUint16LE(status);
		s.syncAsUint16LE(pathWalkerIndex);
		s.syncAsUint16LE(pathResultIndex);
		if (s.isLoading()) {
			pathWalker1 = nullptr;
			pathWalker2 = nullptr;
		}
		s.syncAsUint16LE(pathResultCount);
		if (pathWalkerIndex >= 0) {
			if (s.isLoading()) {
				// Assign a new path walker for the path result data
				pathWalkerIndex = vm->_pathWalkers.getFreeSlot();
				vm->_pathWalkers[pathWalkerIndex].used = 1;
				pathWalker1 = &vm->_pathWalkers[pathWalkerIndex].items[pathResultIndex];
				pathWalker2 = &vm->_pathWalkers[pathWalkerIndex].items[0];
			}
			PathResult *pathResult = pathWalker2;
			for (int j = 0; j < pathResultCount; j++) {
				s.syncAsUint16LE(pathResult->x);
				s.syncAsUint16LE(pathResult->y);
				s.syncAsUint16LE(pathResult->edgeIndex);
				s.syncAsUint16LE(pathResult->nodeIndex);
				s.syncAsUint16LE(pathResult->polyIndex);
				s.syncAsUint16LE(pathResult->direction);
				s.syncAsUint16LE(pathResult->scale);
				pathResult++;
			}
		}
		s.syncAsUint16LE(pathNodeIndex);
		s.syncAsUint16LE(pathEdgeIndex);
		s.syncAsUint16LE(pathPolyIndex);
		s.syncAsUint16LE(firstFrameListIndex);
		s.syncAsUint16LE(lastFrameListIndex);
		s.syncAsUint16LE(minTicks);
		s.syncAsUint16LE(maxTicks);
		s.syncAsUint32LE(ticks);
		s.syncAsByte(ticksFlag);
		syncString(s, pakName);
		s.syncAsUint16LE(pakSlot);
		s.syncAsUint16LE(frameListIndex);
		s.syncAsUint16LE(walkDestX);
		s.syncAsUint16LE(walkDestY);
		s.syncAsUint16LE(x2);
		s.syncAsUint16LE(y2);
		s.syncAsUint16LE(textFontNumber);
		s.syncAsUint16LE(fontOutlineColor);
		s.syncAsUint16LE(fontInkColor);
		s.syncAsUint16LE(x);
		s.syncAsUint16LE(y);
	}
}

void AltActorAnimation::sync(PrisonerEngine *vm, Common::Serializer &s) {
	s.syncAsUint16LE(resourceCacheSlot);
	if (resourceCacheSlot != -1) {
		s.syncAsByte(value);
		syncResourceCacheSlot<AnimationResource>(vm, s, resourceCacheSlot);
	}
}

void SceneItem::sync(PrisonerEngine *vm, Common::Serializer &s) {
	s.syncAsUint16LE(actorIndex);
	if (actorIndex != -1)
		s.syncAsUint16LE(inventoryItemIndex);
}

void ActorFrameSound::sync(PrisonerEngine *vm, Common::Serializer &s) {
	s.syncAsUint16LE(actorIndex);
	if (actorIndex != -1) {
		s.syncAsUint16LE(soundIndex);
		s.syncAsUint16LE(frameListIndex);
		s.syncAsUint16LE(frameIndex);
		s.syncAsUint16LE(volume);
		s.syncAsByte(flag);
	}
}

void PaletteTask::sync(PrisonerEngine *vm, Common::Serializer &s) {
	s.syncAsByte(active);
	s.syncAsUint16LE(value1);
	s.syncAsUint16LE(value2);
	s.syncAsUint16LE(value3);
	s.syncAsUint16LE(positionIncr);
	if (s.isLoading())
		updateTicks = 0;
}

void Dialog::sync(PrisonerEngine *vm, Common::Serializer &s) {
	s.syncAsByte(used);
	if (used) {
		syncString(s, pakName);
		s.syncAsUint16LE(pakSlot);
		if (s.isLoading())
			vm->loadDialogKeywords(pakName, pakSlot, this, false);
		for (uint i = 0; i < keywords.size(); i++)
			s.syncAsByte(keywords[i]._used);
	}
}

void FontColorDef::sync(PrisonerEngine *vm, Common::Serializer &s) {
	s.syncAsByte(inkColor);
	s.syncAsByte(outlineColor);
}

void ScreenText::sync(PrisonerEngine *vm, Common::Serializer &s) {
	s.syncAsByte(used);
	if (used) {
		s.syncAsByte(screenText);
		syncString(s, identifier);
		s.syncAsUint16LE(x0);
		s.syncAsUint16LE(y0);
		s.syncAsByte(screenCenter);
		s.syncAsUint16LE(resourceCacheSlot);
		s.syncAsUint16LE(actorIndex);
		s.syncAsUint32LE(finishedTime);
		s.syncAsUint16LE(speechPakSlot);
		// The slot already points to the next speech in the game, play the current one again
		if (s.isLoading())
			speechPakSlot--;
		s.syncAsUint16LE(chunkCount);
		s.syncAsUint16LE(chunkIndex);
		s.syncAsByte(inventoryActive);
		s.syncAsUint16LE(textIndex);
		s.syncAsUint16LE(fontIndex);
		s.syncAsUint16LE(fontOutlineColor);
		s.syncAsUint16LE(fontInkColor);
		s.syncAsUint16LE(x);
		s.syncAsUint16LE(y);
		s.syncAsUint16LE(lineCount);
		s.syncAsUint16LE(width);
		s.syncAsUint16LE(height);
		s.syncAsUint16LE(fontHeight);
		syncResourceCacheSlot<TextResource>(vm, s, resourceCacheSlot);
	}
}

PrisonerEngine::kReadSaveHeaderError PrisonerEngine::readSaveHeader(Common::SeekableReadStream *in, bool loadThumbnail, SaveHeader &header) {
	Common::Serializer s(in, nullptr);

	header.thumbnail = nullptr;
	header.saveDate = 0;
	header.saveTime = 0;
	header.playTime = 0;

	if (!s.syncVersion(kSavegameVersion))
		return kRSHEInvalidVersion;
	header.version = s.getVersion();

	syncString(s, header.description);

	if (!Graphics::loadThumbnail(*in, header.thumbnail, !loadThumbnail))
		return kRSHEIoError;

	// Not used yet, reserved for future usage
	s.syncAsByte(header.gameID);
	s.syncAsUint32LE(header.flags);

	s.syncAsUint32LE(header.saveDate, 1);
	s.syncAsUint16LE(header.saveTime, 1);
	s.syncAsUint32LE(header.playTime, 1);

	return (in->eos() || in->err()) ? kRSHEIoError : kRSHENoError;
}

Common::Error PrisonerEngine::savegame(const char *filename, const char *description) {
	Common::OutSaveFile *out = g_system->getSavefileManager()->openForSaving(filename);
	if (!out) {
		warning("Can't create file '%s', game not saved", filename);
		return Common::Error(Common::kCreatingFileFailed, filename);
	}

	Common::Serializer s(nullptr, out);
	s.syncVersion(kSavegameVersion);

	Common::String descriptionStr = description;
	syncString(s, descriptionStr);

	if (_menuBackgroundSaved) {
		// Saving from the main menu: take the thumbnail from the game screen behind the menu
		byte palette[256 * 3];
		g_system->getPaletteManager()->grabPalette(palette, 0, 256);
		Graphics::Surface thumbnail;
		::createThumbnail(&thumbnail, (const byte *)_menuBackgroundSurface->getPixels(),
			_menuBackgroundSurface->w, _menuBackgroundSurface->h, palette);
		Graphics::saveThumbnail(*out, thumbnail);
		thumbnail.free();
	} else {
		Graphics::saveThumbnail(*out);
	}

	// Not used yet, reserved for future usage
	byte gameID = 0;
	uint32 flags = 0;
	s.syncAsByte(gameID);
	s.syncAsUint32LE(flags);

	TimeDate now;
	g_system->getTimeAndDate(now);
	uint32 saveDate = ((now.tm_mday & 0xFF) << 24) | (((now.tm_mon + 1) & 0xFF) << 16) | ((now.tm_year + 1900) & 0xFFFF);
	uint16 saveTime = ((now.tm_hour & 0xFF) << 8) | (now.tm_min & 0xFF);
	uint32 playTime = getTotalPlayTime();
	s.syncAsUint32LE(saveDate);
	s.syncAsUint16LE(saveTime);
	s.syncAsUint32LE(playTime);

	syncGameState(s);

	out->finalize();
	bool failed = out->err();
	delete out;

	if (failed) {
		warning("Error writing file '%s', game not saved", filename);
		return Common::Error(Common::kWritingFailed, filename);
	}
	return Common::kNoError;
}

Common::Error PrisonerEngine::loadgame(const char *filename) {
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename);
	if (!in) {
		warning("Can't open file '%s', game not loaded", filename);
		return Common::Error(Common::kReadingFailed, filename);
	}

	SaveHeader header;
	kReadSaveHeaderError errorCode = readSaveHeader(in, false, header);
	if (errorCode != kRSHENoError) {
		warning("Error loading savegame '%s'", filename);
		delete in;
		if (errorCode == kRSHEInvalidVersion)
			return Common::Error(Common::kReadingFailed, "This saved game was made with a newer version of the engine");
		return Common::Error(Common::kReadingFailed, filename);
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

	Common::Serializer s(in, nullptr);
	s.setVersion(header.version);
	syncGameState(s);

	bool failed = in->err();
	delete in;

	if (header.version >= 1)
		setTotalPlayTime(header.playTime);

	_inScene = true;
	initFrameTime();
	updateScreen(true, _cameraX + _mouseX, _cameraY + _mouseY);
	_backgroundFlag = 4;
	_updateDirtyRectsFlag = true;
	_mainMenuRequested = false;
	_inGame = true;
	_menuMouseCursorActive = false;
	_menuMouseCursor = -1;
	_moduleScriptCalled = false;

	if (failed) {
		warning("Error reading savegame '%s'", filename);
		return Common::Error(Common::kReadingFailed, filename);
	}
	return Common::kNoError;
}

// Syncs everything after the header. When loading, the state is restored in the same
// order the data is read, since later parts depend on resources loaded earlier.
void PrisonerEngine::syncGameState(Common::Serializer &s) {
	// Module & Background
	s.syncAsUint16LE(_currModuleIndex);
	s.syncAsUint16LE(_currSceneIndex);
	s.syncAsUint16LE(_newModuleIndex);
	s.syncAsUint16LE(_newSceneIndex);
	s.syncAsUint16LE(_prevModuleIndex);
	s.syncAsUint16LE(_prevSceneIndex);
	s.syncAsUint16LE(_inventoryItemIndex2);
	s.syncAsUint16LE(_backgroundResourceCacheSlot);
	if (_backgroundResourceCacheSlot != -1) {
		Common::String pakName;
		int16 pakSlot, resType;
		syncResourceInfo(this, s, _backgroundResourceCacheSlot, pakName, pakSlot, resType);
		if (s.isLoading())
			loadBackground(pakName, pakSlot);
	}
	s.syncAsUint16LE(_cameraX);
	s.syncAsUint16LE(_cameraY);
	s.syncAsByte(_backgroundCameraLocked);
	s.syncAsUint16LE(_cameraFollowsActorIndex);
	s.syncAsUint16LE(_backgroundObjectsResourceCacheSlot);
	if (_backgroundObjectsResourceCacheSlot != -1) {
		Common::String pakName;
		int16 pakSlot, resType;
		syncResourceInfo(this, s, _backgroundObjectsResourceCacheSlot, pakName, pakSlot, resType);
		if (s.isLoading())
			setBackgroundObjects(pakName, pakSlot);
	}

	// Script variables
	for (uint i = 0; i < 250; i++)
		s.syncAsUint16LE(_globalScriptVars[i]);
	for (uint i = 0; i < 300; i++)
		s.syncAsUint16LE(_moduleScriptVars[i]);

	// Palette
	// TODO: Unknown palette struct "stru_761EC" seems unused
	s.syncAsByte(_scenePaletteOk);
	s.syncBytes(_scenePalette, 768);
	s.syncAsByte(_effectPaletteOk);
	s.syncBytes(_effectPalette, 768);
	if (s.isLoading()) {
		_screen->buildPaletteTransTable(_scenePalette, 0);
		_needToUpdatePalette = true;
	}

	// Palette tasks
	for (int i = 0; i < kMaxPaletteTasks; i++)
		_paletteTasks[i].sync(this, s);
	if (s.isLoading())
		restartPaletteTasks();

	// Path system
	_pathSystem->syncState(s);

	// Scene items
	_sceneItems.sync(this, s);

	// Inventory item combinations
	_inventoryItemCombinations.sync(this, s);

	// Inventory
	s.syncAsUint16LE(_inventoryItemCursor); // TODO: sg_inventoryItemCursor
	// TODO: "someArray" doesn't seem to be used
	s.syncAsUint16LE(_inventoryItemsCount);
	_inventoryItems.sync(this, s);
	if (s.isLoading()) {
		for (int inventoryItemIndex = 0; inventoryItemIndex < _inventoryItems.count(); inventoryItemIndex++) {
			if (_inventoryItems[inventoryItemIndex].resourceCacheSlot != -1)
				loadInventoryItemText(inventoryItemIndex);
		}
	}
	s.syncAsUint16LE(_inventoryItemSlotBaseIndex);
	s.syncAsUint16LE(_inventoryItemsResourceCacheIndex);
	if (_inventoryItemsResourceCacheIndex != -1)
		syncResourceCacheSlot<AnimationResource>(this, s, _inventoryItemsResourceCacheIndex);

	// Actor frame sounds
	s.syncAsUint16LE(_actorFrameSoundItemsCount);
	if (_actorFrameSoundItemsCount > 0)
		_actorFrameSounds.sync(this, s);

	// Actors
	s.syncAsByte(_exitZoneActionFlag);
	s.syncAsUint16LE(_mainActorIndex);
	s.syncAsByte(_mainActorValid);
	s.syncAsUint16LE(_cameraDeltaX);
	s.syncAsUint16LE(_cameraDeltaY);

	// Actor sprites
	_actorSprites.sync(this, s);

	// Actors
	_actors.sync(this, s);
	if (s.isLoading()) {
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
	}

	// Alt actor animations
	_altActorAnimations.sync(this, s);

	// Zones
	_zones.sync(this, s);
	s.syncAsUint16LE(_zoneIndexAtMouse);

	// Main actor saved path/queued action
	s.syncAsByte(_actorPathFlag);
	s.syncAsUint16LE(_actorPathX);
	s.syncAsUint16LE(_actorPathY);
	_zoneActionItem.sync(this, s);

	// Zone actions
	_queuedZoneAction.sync(this, s);
	_zoneActions.sync(this, s);
	s.syncAsUint16LE(_currZoneActionIndex);

	// LipSync
	s.syncAsUint16LE(_lipSyncX);
	s.syncAsUint16LE(_lipSyncY);
	s.syncAsUint16LE(_lipSyncScriptNumber);
	s.syncAsUint16LE(_lipSyncActorIndex);
	s.syncAsUint16LE(_lipSyncResourceCacheSlot);
	if (_lipSyncResourceCacheSlot != -1) {
		Common::String pakName;
		int16 pakSlot, resType;
		syncResourceInfo(this, s, _lipSyncResourceCacheSlot, pakName, pakSlot, resType);
		uint16 count = _lipSyncChannelStatus.size();
		s.syncAsUint16LE(count);
		if (s.isLoading()) {
			_lipSyncChannelStatusRestored = true;
			_lipSyncChannelStatus.clear();
			_lipSyncChannelStatus.resize(count);
		}
		for (uint i = 0; i < count; i++) {
			s.syncAsUint16LE(_lipSyncChannelStatus[i].ticks);
			s.syncAsUint16LE(_lipSyncChannelStatus[i].index);
		}
		if (s.isLoading())
			startLipSync(pakName, pakSlot, _lipSyncScriptNumber, _lipSyncActorIndex, _lipSyncX, _lipSyncY);
	}

	// Scripts
	s.syncAsUint16LE(_enterSceneScriptIndex);
	s.syncAsUint16LE(_enterSceneScriptProgramIndex);
	s.syncAsUint16LE(_leaveSceneScriptIndex);
	s.syncAsUint16LE(_leaveSceneScriptProgramIndex);
	_scriptPrograms[0].sync(this, s);
	_scriptPrograms[1].sync(this, s);

	// Dialog
	_dialogs.sync(this, s);
	for (uint i = 0; i < 50; i++)
		s.syncAsUint16LE(_currDialogKeywordIndices[i]);
	s.syncAsUint16LE(_dialogActiveKeywordsCount);
	s.syncAsUint16LE(_currDialogIndex);
	s.syncAsUint16LE(_dialogMaxKeywordTextWidth);
	s.syncAsUint16LE(_dialogFontHeight);
	s.syncAsUint16LE(_selectedDialogKeywordIndex);
	s.syncAsByte(_dialogRunning);
	s.syncAsByte(_dialogFlag);
	if (s.isLoading() && _dialogRunning)
		startDialog(_currDialogIndex);

	// User input locking
	s.syncAsUint16LE(_lockUserInputRefCounter);

	// Text colors
	_screenTextFontColor.sync(this, s);
	_inventoryScreenTextFontColor.sync(this, s);
	_inventoryFontColor.sync(this, s);
	_zoneFontColor.sync(this, s);
	_dialogFontColor.sync(this, s);
	_dialogHoverFontColor.sync(this, s);

	// Screen texts
	// Unused: g_currTextItemIndex
	_screenTexts.sync(this, s);
	// Unused: word_61674
	s.syncAsByte(_screenTextShowing);
	s.syncAsByte(_screenTextHasSpeech);

	// Music
	for (int16 musicIndex = 0; musicIndex < kMaxMusics; musicIndex++) {
		MusicSlot &musicSlot = _musics[musicIndex];
		s.syncAsUint16LE(musicSlot.resourceCacheSlot);
		if (musicSlot.resourceCacheSlot != -1) {
			Common::String pakName;
			int16 pakSlot, resType;
			syncResourceInfo(this, s, musicSlot.resourceCacheSlot, pakName, pakSlot, resType);
			if (s.isLoading()) {
				// Digital music is loaded from the "W" paks, MIDI from the "M" paks
				musicSlot.isDigital = pakName.hasPrefix("W");
				if (musicSlot.isDigital)
					musicSlot.resourceCacheSlot = _res->load<SoundResource>(pakName, pakSlot, 12);
				else
					musicSlot.resourceCacheSlot = _res->load<MidiResource>(pakName, pakSlot, 13);
				// Version 0 saves don't know whether the music was playing, restart it
				musicSlot.isPlaying = true;
				musicSlot.loop = false;
				musicSlot.loopCount = 0;
			}
			s.syncAsByte(musicSlot.volumeFlag);
			s.syncAsUint16LE(musicSlot.volume);
			s.syncAsByte(musicSlot.shouldResume);
			s.syncAsByte(musicSlot.moduleWide);
			s.syncAsByte(musicSlot.isPlaying, 1);
			s.syncAsByte(musicSlot.loop, 1);
			s.syncAsUint16LE(musicSlot.loopCount, 1);
			if (s.isLoading() && musicSlot.isPlaying) {
				setMusicVolume(musicIndex, musicSlot.volume);
				playMusic(musicIndex, musicSlot.loop, musicSlot.loopCount);
			}
		}
	}

	// Sound
	for (int16 soundIndex = 0; soundIndex < kMaxSounds; soundIndex++) {
		SoundSlot &soundSlot = _sounds[soundIndex];
		s.syncAsUint16LE(soundSlot.resourceCacheSlot);
		if (soundSlot.resourceCacheSlot != -1) {
			syncResourceCacheSlot<SoundResource>(this, s, soundSlot.resourceCacheSlot);
			s.syncAsByte(soundSlot.volumeFlag);
			s.syncAsUint16LE(soundSlot.volume);
			s.syncAsByte(soundSlot.shouldResume);
			s.syncAsByte(soundSlot.moduleWide);
			if (s.isLoading() && soundSlot.shouldResume) {
				// Restart the endlessly looping sound
				setSoundVolume(soundIndex, soundSlot.volume);
				playLoopingSound(soundIndex, 0);
			}
		}
	}
}

// Saving needs a running game in a stable state: a scene is loaded and no scene change,
// video, inventory screen or confirmation box is in progress. The ScummVM menu can still be
// opened in all of these (and ScummVM's periodic autosave asks here too).
bool PrisonerEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	return _inGame && _inScene && _currModuleIndex != -1 && _currSceneIndex != -1 &&
		_newModuleIndex == -1 && _newSceneIndex == -1 &&
		!_isPlayingMux && !_needToPlayMux && !_screensaverRunning &&
		!_inventoryInputRunning && !_isDialogMenuShowing;
}

// Loading also works from the main menu before a game has started
bool PrisonerEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return !_isPlayingMux && !_needToPlayMux && !_screensaverRunning &&
		!_inventoryInputRunning && !_isDialogMenuShowing;
}

Common::Error PrisonerEngine::loadGameState(int slot) {
	return loadgame(getSavegameFilename(slot));
}

Common::Error PrisonerEngine::saveGameState(int slot, const Common::String &description, bool isAutosave) {
	return savegame(getSavegameFilename(slot), description.c_str());
}

const char *PrisonerEngine::getSavegameFilename(int num) {
	static Common::String filename;
	filename = getSavegameFilename(_targetName, num);
	return filename.c_str();
}

Common::String PrisonerEngine::getSavegameFilename(const Common::String &target, int num) {
	assert(num >= 0 && num <= 999);

	char extension[5];
	snprintf(extension, 5, "%03d", num);

	return target + "." + extension;
}

} // End of namespace Prisoner
