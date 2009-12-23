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

enum HitType {
	kHitNone       = -1,
	kHitActionArea = 2,
	kHitBarrier    = 3,
	kHitActor      = 4
};

class SceneTitle {
public:
	SceneTitle(Scene *scene);
	~SceneTitle();

	void update(int32 tick);
	bool loadingComplete() { return _done; }

private:
	Scene *_scene;

	GraphicResource *_bg;
	GraphicResource *_progress;

	int32 _start;
	int32 _ticks;
	bool  _done;
	uint32 _spinnerFrame;
	int32 _spinnerProgress;
	bool  _showMouseState;

}; // end of class SceneTitle


class Scene {
public:
	Scene(uint8 sceneIdx, AsylumEngine *engine);
	~Scene();

	/** .text:0040E460
	 * Initialize the current scene
	 */
	void initialize();

	void handleEvent(Common::Event *event, bool doUpdate);

	void enterScene();

	void activate() { _isActive = true; }
	void deactivate() { _isActive = false; }
	bool isActive() { return _isActive; }
	int  getSceneIndex() { return _sceneIdx; }

	Cursor* getCursor() { return _cursor; }
	ResourcePack* getResourcePack() { return _resPack; }
	ResourcePack* getMusicPack() { return _musPack; }
	GraphicResource* getGraphicResource(int32 entry) { return new GraphicResource(_resPack, entry); }
	BlowUpPuzzle* getBlowUpPuzzle() { return _blowUp;}
	void setBlowUpPuzzle(BlowUpPuzzle* puzzle) { _blowUp = puzzle; }
	void setScenePosition(int x, int y);

	AsylumEngine* vm() { return _vm; }
	WorldStats* worldstats() { return _ws; }
	Polygons* polygons() { return _polygons; }
	ActionList* actions() { return _actions; }
	Speech* speech() { return _speech; }

	/** .text:0040A1B0
	 * Get the supplied actor's position relative to the
	 * current scene's xLeft and yTop
	 */
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
	/** .text:00407A00
	 * TODO
	 */
	void setActorDirection(Actor *act, int direction);

	/** .text:004094c0
	 * Determine the amount to increase the supplied sound
	 * sample's volume based on the actor's position
	 */
	int32 calculateVolumeAdjustment(AmbientSoundItem *snd, Actor *act);

protected:
	/** .text:0040EA50
	 * Run various hit tests and return the index,
	 * and a reference to the located type
	 */
	int32 hitTest(int32 x, int32 y, HitType &type);
	/** .text:0040F010
	 * TODO
	 */
	int32 hitTestActionArea(int32 x, int32 y);
	/** .text:0040E7F0
	 * Check if the mouse cursor is currently intersecting
	 * the currently active actor
	 */
	bool hitTestActor(int16 x, int16 y);
	/** .text:004341E0
	 * Check if the mouse cursor is currently intersecting
	 * a graphic resource at the supplied coordinates
	 */
	bool hitTestPixel(int32 grResId, int32 frame, int16 x, int16 y, bool flipped);
	/** .text:0040E8A0
	 * Checks if the supplied coordinates are inside an action area, barrier or
	 * actor, and returns -1 if nothing was found, or the type of hit if found
	 */
	int32 hitTestScene(int16 x, int16 y, HitType &type);
	/** .text:00408980
	 * Determine if the supplied point intersects
	 * an action area's active region
	 */
	int32 findActionArea(int32 pointX, int32 pointY);
	/** .text:0040EAA0
	 * Check if a barrier exist at the supplied coordinates.
	 * If so, return it's index within the barriers array, if not,
	 * return -1
	 */
	int32 hitTestBarrier(int32 x, int32 y);
private:
	AsylumEngine  *_vm;
	Common::Event *_ev;

	uint8 _sceneIdx;
	int32 _playerActorIdx;
	bool  _titleLoaded;
	bool  _walking;
	bool  _leftClick;
	bool  _rightButton;
	bool  _isActive;
	bool  _skipDrawScene;

	WorldStats   *_ws;
	Polygons     *_polygons;
	ActionList   *_actions;
	SceneTitle   *_title;
	Speech       *_speech;
	Cursor       *_cursor;
	ResourcePack *_resPack;
	ResourcePack *_musPack;
	BlowUpPuzzle *_blowUp;

	GraphicResource *_bgResource;
	GraphicFrame    *_background;

	void update();
	void startMusic();

	/** .text:0040B5B0
	 * Loop through the various update blocks (actors,
	 * barriers, mouse, music, sfx, screenPosition), then
	 * process the current action script
	 */
	int updateScene();
	/** .text:0040D190
	 * TODO add description
	 */
	void updateMouse();
	/** .text:0040D580
	 * TODO
	 */
	void handleMouseUpdate(int direction, Common::Rect rect);
	/** .text:0040B740
	 * TODO add description
	 */
	void updateActor(int32 actorIdx);
	/**
	 * TODO give more meaningful name
	 */
	void updateActorSub01(Actor *act);
	/** .text:0040CBD0
	 * TODO add description
	 */
	void updateBarriers();
	/** .text:00409BA0
	 * TODO add description
	 */
	void updateAmbientSounds();
	/** .text:00409EF0
	 * TODO add description
	 */
	void updateMusic();
	/** .text:0040DAE0
	 * TODO add description
	 */
	void updateAdjustScreen();
	int drawScene();
	/** .text:0040A3C0
	 * TODO add description
	 */
	void drawActorsAndBarriers();
	int queueActorUpdates();
	int queueBarrierUpdates();
	bool isBarrierVisible(BarrierItem *barrier);
	bool isBarrierOnScreen(BarrierItem *barrier);
	int32 getRandomResId(BarrierItem *barrier);

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
