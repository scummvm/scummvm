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
#ifndef SCUMMVM_SCENE_H
#define SCUMMVM_SCENE_H

#include "common/rect.h"
#include "common/system.h"

namespace Dragons {

class DragonsEngine;
class ActorManager;
class Background;
class DragonRMS;
class BackgroundResourceLoader;
class DragonINIResource;
class BigfileArchive;
class Screen;
class ScriptOpcodes;
struct DragonINI;

class Scene {
public:
	Common::Point _camera;
	int16 data_80063392;

private:
	DragonsEngine *_vm;
	Screen *_screen;
	ActorManager *_actorManager;
	Background *_stage;
	BigfileArchive *_bigfileArchive;
	DragonRMS *_dragonRMS;
	DragonINIResource *_dragonINIResource;
	BackgroundResourceLoader *_backgroundLoader;
	ScriptOpcodes *_scriptOpcodes;

	int16 _currentSceneId;

public:
	Scene(DragonsEngine *vm, Screen *screen, ScriptOpcodes *scriptOpcodes, BigfileArchive *bigfileArchive, ActorManager *actorManager, DragonRMS *_dragonRMS, DragonINIResource *_dragonINIResource);

	void loadScene(uint32 sceneId, uint32 cameraPointId);
	int16 getPriorityAtPosition(Common::Point pos);
	void draw();
	bool contains(DragonINI *ini);
	byte *getPalette();
	uint16 getSceneId();
	void setSceneId(int16 newSceneId);
	Common::Point getPoint(uint32 pointIndex);
	uint16 getStageWidth();
	uint16 getStageHeight();
	void loadImageOverlay(uint16 iptId);
	void removeImageOverlay(uint16 iptId);
private:
	void loadSceneData(uint32 sceneId, uint32 cameraPointId);

};

} // End of namespace Dragons

#endif //SCUMMVM_SCENE_H
