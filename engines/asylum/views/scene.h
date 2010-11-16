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

#ifndef ASYLUM_SCENE_H
#define ASYLUM_SCENE_H

#include "asylum/shared.h"

#include "common/array.h"
#include "common/events.h"
#include "common/rational.h"
#include "graphics/surface.h"

#define SCENE_FILE_MASK "scn.%03d"
#define MUSIC_FILE_MASK "mus.%03d"

namespace Asylum {

class ActionList;
class Actor;
class AsylumEngine;
class BlowUpPuzzle;
class Cursor;
class GraphicResource;
class Polygons;
class ResourcePack;
class SceneTitle;
class Screen;
class Special;
class Speech;
class Sound;
class Text;
class Video;
class WorldStats;

struct ActionArea;
struct AmbientSoundItem;
struct GraphicFrame;
struct ObjectItem;
struct PolyDefinitions;

enum HitType {
	kHitNone       = -1,
	kHitActionArea = 2,
	kHitObject    = 3,
	kHitActor      = 4
};

class Scene {
public:
	Scene(AsylumEngine *engine);
	~Scene();

	/**
	 * Enter a scene
	 *
	 * @param packId Package id for the scene.
	 */
	void enter(ResourcePackId packId);

	/**
	 * Load the scene data
	 * @param packId Package id for the scene.
	 */
	void load(ResourcePackId packId);


	void handleEvent(Common::Event *event, bool doUpdate);

	void activate();
	void deactivate() { _isActive = false; }
	bool isActive() { return _isActive; }
	ResourcePackId  getPackId() { return _packId; }

	//BlowUpPuzzle* getBlowUpPuzzle() { return _blowUp;}
	//void setBlowUpPuzzle(BlowUpPuzzle* puzzle) { _blowUp = puzzle; }
	void setScenePosition(int x, int y);

	AsylumEngine* vm() { return _vm; }

	ActionList  *actions()    { return _actions; }
	Polygons    *polygons()   { return _polygons; }
	Special     *special()    { return _special; }
	Speech      *speech()     { return _speech; }
	WorldStats  *worldstats() { return _ws; }

	/** .text:0040A1B0
	 * Get the supplied actor's position relative to the
	 * current scene's xLeft and yTop
	 */
	void getActorPosition(Actor *actor, Common::Point *pt);
	/**
	 * Return the index of the current player actor
	 */
	ActorIndex getPlayerIndex() { return _playerActorIdx; }

	void setPlayerActorIndex(ActorIndex index) { _playerActorIdx = index; }
	void changePlayer(ActorIndex index);

	/**
	 * Get a reference to an actor object from the
	 * WorldStats actor list. Default parameter just
	 * gets the instance associated with _playerActorIdx
	 */
	Actor* getActor(ActorIndex index = kActorInvalid);

	/** .text:00407260
	 * Check the actor at actorIndex to see if the currently loaded
	 * graphic resource matches the resource at grResTable[5]
	 */
	bool defaultActorDirectionLoaded(int actorIndex, int grResTableIdx);

	/** .text:004094c0
	 * Determine the amount to increase the supplied sound
	 * sample's volume based on the actor's position
	 */
	int32 calculateVolumeAdjustment(AmbientSoundItem *snd, Actor *act);


	ActorDirection getGlobalDirection() { return _globalDirection; }

	bool updateSceneCoordinates(int32 targetX, int32 targetY, int32 val, bool checkSceneCoords = false, int32 *param = NULL);

	void adjustCoordinates(int32 x, int32 y, Common::Point *point);

	int processActor(int *x, int *param);

	void updatePalette(int32 param);
	void makeGreyPalette();

	// Shared methods
	void resetActor0();

	/** .text:00408980
	 * Determine if the supplied point intersects
	 * an action area's active region
	 */
	int32 findActionArea(const Common::Point pt);

	int32 isInActionArea(const Common::Point &pt, ActionArea *area);

protected:
	/** .text:0040EA50
	 * Run various hit tests and return the index,
	 * and a reference to the located type
	 */
	int32 hitTest(const Common::Point pt, HitType &type);
	/** .text:0040F010
	 * TODO
	 */
	int32 hitTestActionArea(const Common::Point pt);
	/** .text:0040E7F0
	 * Check if the mouse cursor is currently intersecting
	 * the currently active actor
	 */
	bool hitTestActor(const Common::Point pt);
	/** .text:004341E0
	 * Check if the mouse cursor is currently intersecting
	 * a graphic resource at the supplied coordinates
	 */
	bool hitTestPixel(ResourceId resourceId, int32 frame, int16 x, int16 y, bool flipped);
	/** .text:0040E8A0
	 * Checks if the supplied coordinates are inside an action area, object or
	 * actor, and returns -1 if nothing was found, or the type of hit if found
	 */
	ResourceId hitTestScene(const Common::Point pt, HitType &type);

	/** .text:0040EAA0
	 * Check if a object exist at the supplied coordinates.
	 * If so, return it's index within the objects array, if not,
	 * return -1
	 */
	int32 hitTestObject(const Common::Point pt);

private:
	AsylumEngine  *_vm;
	Common::Event *_ev;

	ResourcePackId _packId;
	int32 _playerActorIdx;
	bool  _titleLoaded;
	bool  _walking;
	bool  _leftClick;
	bool  _rightButton;
	bool  _isActive;
	ActorDirection _globalDirection;

	ActionList   *_actions;
	//BlowUpPuzzle *_blowUp;
	Special      *_special;
	Speech       *_speech;
	SceneTitle   *_title;
	Polygons     *_polygons;
	WorldStats   *_ws;

	GraphicResource *_bgResource;
	GraphicFrame    *_background;

	struct UpdateItem {
		ActorIndex index;
		int32 priority;
	};

	Common::Array<UpdateItem> _updateList;

	/** .text:0040B5B0
	 * Loop through the various update blocks (actors,
	 * objects, mouse, music, sfx, screenPosition), then
	 * process the current action script
	 */
	bool update();
	/** .text:0040D190
	 * TODO add description
	 */
	void updateMouse();
	/** .text:0040D580
	 * TODO
	 */
	void handleMouseUpdate(int direction, Common::Rect rect);

	void updateActors();

	/** .text:0040CBD0
	 * TODO add description
	 */
	void updateObjects();
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

	void updateScreen();



	void playIntroSpeech();


	//////////////////////////////////////////////////////////////////////////
	// Scene drawing
	//////////////////////////////////////////////////////////////////////////
	int drawScene();

	void buildUpdateList();
	void processUpdateList();
	static bool updateListCompare(const UpdateItem &item1, const UpdateItem &item2);

	void checkVisibleActorsPriority();
	void adjustActorPriority(ActorIndex index);

	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////
	bool pointIntersectsRect(Common::Point point, Common::Rect rect);

	//////////////////////////////////////////////////////////////////////////
	// Scene debugging
	//////////////////////////////////////////////////////////////////////////
	void copyToBackBufferClipped(Graphics::Surface *surface, int x, int y);

	void debugScreenScrolling(GraphicFrame *bg);
	void debugShowPolygons();
	void debugShowObjects();
	void debugShowActors();
	void debugShowWalkRegion(PolyDefinitions *poly);

	friend class SceneTitle;

};

} // end of namespace Asylum

#endif
