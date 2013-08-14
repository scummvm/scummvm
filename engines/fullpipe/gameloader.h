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
 */

#ifndef FULLPIPE_GAMELOADER_H
#define FULLPIPE_GAMELOADER_H

#include "fullpipe/objects.h"
#include "fullpipe/inventory.h"
#include "fullpipe/messages.h"

namespace Fullpipe {

class SceneTag;
class CMctlCompound;
class CInputController;
class CInteractionController;

class CGameLoader : public CObject {
 public:
	CGameLoader();
	virtual ~CGameLoader();

	virtual bool load(MfcArchive &file);
	bool loadScene(int sceneId);
	bool gotoScene(int sceneId, int entranceId);
	bool preloadScene(int sceneId, int entranceId);

	void updateSystems(int counterdiff);

	int getSceneTagBySceneId(int sceneId, SceneTag **st);
	void applyPicAniInfos(Scene *sc, PicAniInfo **picAniInfo, int picAniInfoCount);

	GameProject *_gameProject;
	CInteractionController *_interactionController;
	CInputController *_inputController;
	CInventory2 _inventory;
	Sc2Array _sc2array;
	void *_sceneSwitcher;
	void *_preloadCallback;
	void *_readSavegameCallback;
	int16 _field_F8;
	int16 _field_FA;
	PreloadItems _preloadItems;
	CGameVar *_gameVar;
	char *_gameName;
	ExCommand _exCommand;
	int _updateCounter;
	int _preloadSceneId;
	int _preloadEntranceId;
};

CInventory2 *getGameLoaderInventory();
CInteractionController *getGameLoaderInteractionController();
CMctlCompound *getSc2MctlCompoundBySceneId(int16 sceneId);

} // End of namespace Fullpipe

#endif /* FULLPIPE_GAMELOADER_H */
