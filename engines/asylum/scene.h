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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_SCENE_H_
#define ASYLUM_SCENE_H_

#include "common/events.h"

#include "asylum/asylum.h"
#include "asylum/respack.h"
#include "asylum/graphics.h"
#include "asylum/text.h"
#include "asylum/sceneres.h"

namespace Asylum {

class Screen;
class Sound;
class Video;
class Text;
class SceneResource;
class WorldStats;
class BlowUpPuzzle;
struct ActionDefinitions;
struct PolyDefinitions;
struct BarrierItem;

//uint32 playerTypeTable[16] = {0, 0, 0, 0, 1, 0, 2, 0, 3, 0, 1, 1, 3, 0, 0, 0};

class Scene {
public:
	Scene(uint8 sceneIdx);
	~Scene();

	void handleEvent(Common::Event *event, bool doUpdate);

	void enterScene();
	void activate() { _isActive = true; }
	void deactivate() { _isActive = false; }
	bool isActive() { return _isActive; }
	int  getSceneIndex() { return _sceneIdx; }
	
	ActionDefinitions* getDefaultActionList();
	ActionDefinitions* getActionList(int actionListIndex);

	void setActorPosition(int actorIndex, int x, int y);
	void setActorAction(int actorIndex, int action);
	void actorVisible(int actorIndex, bool visible);
	bool actorVisible(int actorIndex);
	void setScenePosition(int x, int y);

	SceneResource*   getResources() { return _sceneResource; }
	ResourcePack*	 getResourcePack() { return _resPack; }
    ResourcePack*	 getMusicPack() { return _musPack; }
    ResourcePack*	 getSpeechPack() { return _speechPack; }
	GraphicResource* getGraphicResource(uint32 entry) { return new GraphicResource(_resPack, entry); }
    BlowUpPuzzle*    getBlowUpPuzzle() { return _blowUp; }
    void             setBlowUpPuzzle(BlowUpPuzzle* puzzle) { _blowUp = puzzle; }

private:
	void copyToBackBufferClipped(Graphics::Surface *surface, int x, int y);

	uint8	        _sceneIdx;
	SceneResource	*_sceneResource;
    ResourcePack	*_resPack;
	ResourcePack	*_speechPack;
	ResourcePack	*_musPack;
    BlowUpPuzzle    *_blowUp;
	Common::Event   *_ev;
	Text			*_text;
	GraphicResource *_bgResource;
	GraphicResource *_cursorResource;
	GraphicFrame	*_background;

	bool	_walking;
	uint32	_mouseX;
	uint32	_mouseY;
	int32	_startX;
	int32	_startY;
	bool	_leftClick;
	bool	_rightButton;
	uint32	_curMouseCursor;
	int32	_cursorStep;
	bool	_isActive;
    bool	_skipDrawScene;
    uint32  _playerActorIdx;

	void   update();
    int    updateScene();
    void   updateActor(uint32 actorIdx);
    void   updateBarriers(WorldStats *worldStats);
    void   updateAmbientSounds();
    void   updateMusic();
    void   updateAdjustScreen();
    int    drawScene();
    int    drawBarriers();

    int   isActorVisible(ActorItem *actor);

    bool   isBarrierVisible(BarrierItem *barrier);
    bool   isBarrierOnScreen(BarrierItem *barrier);
    uint32 getRandomResId(BarrierItem *barrier);

	/**
	 * Check whether the cursor resource needs to be changed, and
	 * if so, make that change
	 */
	void updateCursor();

	/**
	 * Update the cursor to visually indicate that it is over a
	 * clickable region (by running its associated animation)
	 */
	void animateCursor();

	void debugScreenScrolling(GraphicFrame *bg);
	void debugShowPolygons();
	void debugShowBarriers();
	void debugShowWalkRegion(PolyDefinitions *poly);

    // TODO: get rid of this
    void OLD_UPDATE(WorldStats *worldStats);
    
}; // end of class Scene

} // end of namespace Asylum

#endif
