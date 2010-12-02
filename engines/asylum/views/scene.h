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

#include "asylum/eventhandler.h"
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

class Scene : public EventHandler {
public:
	Scene(AsylumEngine *engine);
	~Scene();

	/**
	 * Load the scene data
	 *
	 * @param packId Package id for the scene.
	 */
	void load(ResourcePackId packId);

	/**
	 * Enter a scene
	 *
	 * @param packId Package id for the scene.
	 */
	void enter(ResourcePackId packId);

	/**
	 * Handle events
	 *
	 * @param ev The event
	 *
	 * @return true if it succeeds, false if it fails.
	 */
	bool handleEvent(const AsylumEvent &ev);

	/**
	 * Return the index of the player actor
	 */
	ActorIndex getPlayerIndex() { return _playerActorIdx; }

	/**
	 * Sets the player actor index.
	 *
	 * @param index index of the player actor
	 */
	void setPlayerActorIndex(ActorIndex index) { _playerActorIdx = index; }

	/**
	 * Gets the current scene pack identifier.
	 *
	 * @return The pack identifier.
	 */
	ResourcePackId  getPackId() { return _packId; }

	/**
	 * Get a reference to an actor object from the
	 * WorldStats actor list. Default parameter just
	 * gets the instance associated with _playerActorIdx
	 */
	Actor* getActor(ActorIndex index = kActorInvalid);

	/**
	 * Change player actor
	 *
	 * @param index new index for the player actor
	 */
	void changePlayer(ActorIndex index);

	/**
	 * Updates the scene coordinates.
	 *
	 * @param targetX 		   Target x coordinate.
	 * @param targetY 		   Target y coordinate.
	 * @param val 			   The value.
	 * @param checkSceneCoords true to check scene coordinates.
	 * @param [in,out] param   If non-null, the parameter.
	 *
	 * @return true if it succeeds, false if it fails.
	 */
	bool updateSceneCoordinates(int32 targetX, int32 targetY, int32 val, bool checkSceneCoords = false, int32 *param = NULL);

	/**
	 * Determine if the supplied point intersects an action area's active region.
	 *
	 * @param pt The point.
	 *
	 * @return The found action area.
	 */
	int32 findActionArea(const Common::Point pt);

	/**
	 * Determine if the supplied point is in the action area
	 *
	 * @param pt 			The point.
	 * @param [in,out] area If non-null, the area.
	 *
	 * @return
	 */
	bool isInActionArea(const Common::Point &pt, ActionArea *area);

	ActionList  *actions()    { return _actions; }
	Polygons    *polygons()   { return _polygons; }
	Special     *special()    { return _special; }
	Speech      *speech()     { return _speech; }
	WorldStats  *worldstats() { return _ws; }

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

	ResourcePackId _packId;
	int32 _playerActorIdx;
	bool  _walking;


	ActorDirection _globalDirection;

	ActionList   *_actions;
	Special      *_special;
	Speech       *_speech;
	Polygons     *_polygons;
	WorldStats   *_ws;

	struct UpdateItem {
		ActorIndex index;
		int32 priority;
	};

	Common::Array<UpdateItem> _updateList;

	//////////////////////////////////////////////////////////////////////////
	// Message handling
	void activate();
	bool init();
	bool update();
	bool key(const AsylumEvent &evt);
	bool clickDown(const AsylumEvent &evt);
	bool clickUp(const AsylumEvent &evt);

	//////////////////////////////////////////////////////////////////////////
	// Scene update
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Updates the screen
	 *
	 * @return true if it succeeds, false if it fails.
	 */
	bool updateScreen();

	/**
	 * Loop through the various update blocks (actors, objects, mouse, music, sfx, coordinates), then process the current action script.
	 *
	 * @return true if the script is done executing, false otherwise
	 */
	bool updateScene();

	/**
	 * Updates the mouse.
	 */
	void updateMouse();

	/**
	 * Updates the actors.
	 */
	void updateActors();

	/**
	 * Updates the objects.
	 */
	void updateObjects();

	/**
	 * Updates the ambient sounds.
	 */
	void updateAmbientSounds();

	/**
	 * Updates the music.
	 */
	void updateMusic();

	/**
	 * Updates the screen
	 *
	 *  - update coordinates or allow scrolling if the proper debug option is set
	 */
	void updateAdjustScreen();

	/**
	 * Updates the screen coordinates.
	 */
	void updateCoordinates();

	/**
	 * Update cursor
	 *
	 * @param direction The direction.
	 * @param rect 		The rectangle.
	 */
	void updateCursor(int direction, Common::Rect rect);

	//////////////////////////////////////////////////////////////////////////
	// Scene drawing
	//////////////////////////////////////////////////////////////////////////
	void preload(); // Draw the loading screen
	bool drawScene();

	void buildUpdateList();
	void processUpdateList();
	static bool updateListCompare(const UpdateItem &item1, const UpdateItem &item2);

	void checkVisibleActorsPriority();
	void adjustActorPriority(ActorIndex index);

	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////
	void playIntroSpeech();

	bool pointIntersectsRect(Common::Point point, Common::Rect rect);

	//////////////////////////////////////////////////////////////////////////
	// Scene debugging
	//////////////////////////////////////////////////////////////////////////
	void debugScreenScrolling();
	void debugShowPolygons();
	void debugShowObjects();
	void debugShowActors();
	void debugShowWalkRegion(PolyDefinitions *poly);

	friend class SceneTitle;
};

} // end of namespace Asylum

#endif
