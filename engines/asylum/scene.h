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
#include "asylum/worldstats.h"
#include "asylum/polygons.h"
#include "asylum/actionlist.h"
#include "asylum/text.h"
#include "asylum/cursor.h"
#include "asylum/speech.h"

#define SCENE_FILE_MASK "scn.%03d"
#define MUSIC_FILE_MASK "mus.%03d"

namespace Asylum {

class ActionList;
class Screen;
class Sound;
class Video;
class Cursor;
class Text;
class BlowUpPuzzle;
struct BarrierItem;
class WorldStats;
class Speech;

class SceneTitle {
public:
	SceneTitle(Scene *scene);
	~SceneTitle();

	void update(uint32 tick);
	bool loadingComplete() { return _done; }

private:
	Scene           *_scene;
	GraphicResource *_bg;
	GraphicResource *_progress;

	uint32 _start;
	uint32 _ticks;
	bool   _done;
	uint32 _spinnerFrame;
	uint32 _spinnerProgress;

}; // end of class SceneTitle


class Scene {
public:
	Scene(uint8 sceneIdx, AsylumEngine *engine);
	~Scene();

	void handleEvent(Common::Event *event, bool doUpdate);

	void enterScene();

	void activate() { _isActive = true;	}
	void deactivate() { _isActive = false; }
	bool isActive() { return _isActive;	}
	int  getSceneIndex() { return _sceneIdx; }

	Cursor* getCursor() { return _cursor; }
	ResourcePack* getResourcePack() { return _resPack; }
	ResourcePack* getMusicPack() { return _musPack; }
	GraphicResource* getGraphicResource(uint32 entry) {	return new GraphicResource(_resPack, entry); }
	BlowUpPuzzle* getBlowUpPuzzle() { return _blowUp;}
	void setBlowUpPuzzle(BlowUpPuzzle* puzzle) { _blowUp = puzzle; }
	void setScenePosition(int x, int y);

	AsylumEngine* vm() { return _vm; }
	WorldStats* worldstats() { return _ws; }
	Polygons* polygons() { return _polygons; }
	ActionList* actions() {	return _actions; }
    Speech* speech() { return _speech; }

    void getActorPosition(Actor *actor, Common::Point *pt);

    /**
     * Return the index of the current player actor
     */
    int getActorIndex() { return _playerActorIdx; }
    /**
     * Get a reference to an actor object from the
     * WorldStats actor list. Default parameter just
     * gets the instance associated with _playerActorIdx
     */
    Actor* getActor(int index = -1);
    /** .text:004072A0
     * Based on the value of param, replace the actor
     * graphic from the actor's grResTable
     *
     * TODO figure out exactly what param means
     */
    void updateActorDirection(int actorIndex, int param);
    /** .text:0040A2E0
     * If the actor index is invalid, load the default actor,
     * then call updateActorDirection with a param of 4
     */
    void updateActorDirectionDefault(int actorIndex);
    /** .text:00407260
     * Check the actor at actorIndex to see if the currently loaded
     * graphic resource matches the resource at grResTable[5]
     */
    bool defaultActorDirectionLoaded(int actorIndex, int grResTableIdx);
private:
	AsylumEngine *_vm;
	uint8	     _sceneIdx;
	WorldStats   *_ws;
	Polygons     *_polygons;
	ActionList   *_actions;
	SceneTitle   *_title;
    Speech       *_speech;

	Cursor			*_cursor;
	ResourcePack	*_resPack;
	ResourcePack	*_musPack;
	BlowUpPuzzle    *_blowUp;
	Common::Event   *_ev;
	GraphicResource *_bgResource;
	GraphicFrame	*_background;

	bool   _titleLoaded;
	bool   _walking;
	bool   _leftClick;
	bool   _rightButton;
	bool   _isActive;
	bool   _skipDrawScene;
	uint32 _playerActorIdx;

	/** .text:0040E460
	 * Initialize the current scene
	 */
	void initialize();

	void   update();
	int    updateScene();
	void   updateMouse();
	void   updateActor(uint32 actorIdx);
	/**
	 * TODO give more meaningful name
	 */
	void   updateActorSub01(Actor *act);
	void   updateBarriers();
	void   updateAmbientSounds();
	void   updateMusic();
	void   updateAdjustScreen();
	int    drawScene();
	void   drawActorsAndBarriers();
	int    queueActorUpdates();
	int    queueBarrierUpdates();
	bool   isBarrierVisible(BarrierItem *barrier);
	bool   isBarrierOnScreen(BarrierItem *barrier);
	uint32 getRandomResId(BarrierItem *barrier);	

	void copyToBackBufferClipped(Graphics::Surface *surface, int x, int y);

	void debugScreenScrolling(GraphicFrame *bg);
	void debugShowPolygons();
	void debugShowBarriers();
	void debugShowActors();
	void debugShowWalkRegion(PolyDefinitions *poly);

	// TODO: get rid of this
	void OLD_UPDATE();

	friend class SceneTitle;

}; // end of class Scene

} // end of namespace Asylum

#endif
