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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BBVS_GAMEMODULE_H
#define BBVS_GAMEMODULE_H

#include "common/array.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/rect.h"
#include "common/str.h"

namespace Bbvs {

const int kInventoryItemCount = 42;
const int kInventoryItemSpriteCount = 2 * kInventoryItemCount;
const int kDialogItemSpriteCount = 26;
const int kGuiSpriteCount = 21;
const int kCameraInitsCount = 4;

struct Condition {
	byte cond;
	byte value1;
	int16 value2;
};

struct Conditions {
	Condition conditions[8];
};

struct ActionResult {
	byte kind;
	byte value1;
	int16 value2;
};

struct ActionResults {
	ActionResult actionResults[8];
};

struct ActionCommand {
	uint16 cmd;
	int16 sceneObjectIndex;
	uint32 timeStamp;
	Common::Point walkDest;
	int32 param;
};

class ActionCommands : public Common::Array<ActionCommand> {
};

struct Action {
	Conditions conditions;
	ActionResults results;
	ActionCommands actionCommands;
};

struct InventoryItemInfo {
	int16 xOffs, yOffs;
	int16 width, height;
};

struct CameraInit {
	Common::Point cameraPos;
	byte cameraLinks[8];
	Common::Rect rects[8];
};

struct SceneObjectDef {
	char name[20];
	int animIndices[16];
	int walkSpeed;
};

struct SceneObjectInit {
	Conditions conditions;
	int sceneObjectIndex;
	int animIndex;
	int x, y;
};

struct BgObject {
	char name[20];
	Common::Rect rect;
};

struct Animation {
	int frameCount;
	int *frameSpriteIndices;
	int16 *frameTicks;
	Common::Rect *frameRects1;
	Common::Rect *frameRects2;
	Animation()
		: frameCount(0), frameSpriteIndices(0), frameTicks(0), frameRects1(0), frameRects2(0) {
	}
	~Animation() {
		delete[] frameSpriteIndices;
		delete[] frameTicks;
		delete[] frameRects1;
		delete[] frameRects2;
	}
};

struct SceneExit {
	Common::Rect rect;
	int newModuleNum;
};

struct SceneSound {
	Conditions conditions;
	uint soundNum;
};

class GameModule {
public:
	GameModule();
	~GameModule();

	void load(const char *filename);

	int getFieldC();
	int getButtheadObjectIndex();

	int getGuiSpriteIndex(int index);
	int getInventoryItemSpriteIndex(int index);
	int getDialogItemSpriteIndex(int index);

	int getActionsCount();
	Action *getAction(int index);

	InventoryItemInfo *getInventoryItemInfo(int index);

	CameraInit *getCameraInit(int cameraNum);

	int getSceneExitsCount();
	SceneExit *getSceneExit(int index);

	int getWalkRectsCount();
	Common::Rect *getWalkRects();

	int getSceneObjectDefsCount();
	SceneObjectDef *getSceneObjectDef(int index);

	int getSceneObjectInitsCount();
	SceneObjectInit *getSceneObjectInit(int index);

	int getBgObjectsCount();
	BgObject *getBgObject(int index);

	int getBgSpritesCount();
	int getBgSpriteIndex(int index);
	int getBgSpritePriority(int index);

	int getSceneSoundsCount();
	SceneSound *getSceneSound(int index);
	uint getSceneSoundIndex(uint soundNum);

	uint getPreloadSoundsCount();
	uint getPreloadSound(uint index);

	Animation *getAnimation(int index);

protected:

	int _bgSpriteCount;
	int *_bgSpriteIndices;
	int16 *_bgSpritePriorities;

	CameraInit _cameraInits[kCameraInitsCount];

	int _walkRectsCount;
	Common::Rect *_walkRects;

	int _sceneExitsCount;
	SceneExit *_sceneExits;

	int _bgObjectsCount;
	BgObject *_bgObjects;

	int _animationsCount;
	Animation *_animations;

	int _sceneObjectDefsCount;
	SceneObjectDef *_sceneObjectDefs;

	int _sceneObjectInitsCount;
	SceneObjectInit *_sceneObjectInits;

	int _actionsCount;
	Action *_actions;

	int _sceneSoundsCount;
	SceneSound *_sceneSounds;

	uint _preloadSoundsCount;
	uint *_preloadSounds;

	int _guiSpriteIndices[kGuiSpriteCount];
	int _inventoryItemSpriteIndices[kInventoryItemSpriteCount];
	InventoryItemInfo _inventoryItemInfos[kInventoryItemCount];
	int _dialogItemSpriteIndices[kDialogItemSpriteCount];

	int _fieldC;
	int _buttheadObjectIndex;

	Common::Point readPoint(Common::SeekableReadStream &s);
	Common::Rect readRect(Common::SeekableReadStream &s);
	Conditions readConditions(Common::SeekableReadStream &s);

	void unload();

	void loadBgSprites(Common::SeekableReadStream &s);
	void loadCameraInits(Common::SeekableReadStream &s);
	void loadWalkRects(Common::SeekableReadStream &s);
	void loadSceneExits(Common::SeekableReadStream &s);
	void loadBgObjects(Common::SeekableReadStream &s);
	void loadAnimations(Common::SeekableReadStream &s);
	void loadSceneObjectDefs(Common::SeekableReadStream &s);
	void loadSceneObjectInits(Common::SeekableReadStream &s);
	void loadActions(Common::SeekableReadStream &s);
	void loadGuiSpriteIndices(Common::SeekableReadStream &s);
	void loadInventoryItemSpriteIndices(Common::SeekableReadStream &s);
	void loadInventoryItemInfos(Common::SeekableReadStream &s);
	void loadDialogItemSpriteIndices(Common::SeekableReadStream &s);
	void loadSceneSounds(Common::SeekableReadStream &s);
	void loadPreloadSounds(Common::SeekableReadStream &s);

};

} // End of namespace Bbvs

#endif // BBVS_GAMEMODULE_H
