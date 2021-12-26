/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ASYLUM_VIEWS_SCENE_H
#define ASYLUM_VIEWS_SCENE_H

#include "common/array.h"
#include "common/events.h"
#include "common/rational.h"

#include "graphics/surface.h"

#include "asylum/system/screen.h"

#include "asylum/eventhandler.h"
#include "asylum/shared.h"

#define SCENE_FILE_MASK "scn.%03d"
#define MUSIC_FILE_MASK "mus.%03d"

// If defined, will show the scene update times on the debugger output
//#define DEBUG_SCENE_TIMES

namespace Asylum {

class Actor;
class AsylumEngine;
class Puzzle;
class Cursor;
class GraphicResource;
class Polygons;
class Polygon;
class ResourcePack;
class SceneTitle;
class Screen;
class Special;
class Speech;
class Sound;
class Text;
class VideoPlayer;
class WorldStats;

struct ActionArea;
struct AmbientSoundItem;
struct GraphicFrame;
struct ObjectItem;

enum HitType {
	kHitNone       = -1,
	kHitActionArea = 2,
	kHitObject    = 3,
	kHitActor      = 4
};

enum ActionAreaType {
	kActionAreaType1 = 1,
	kActionAreaType2 = 2
};

class Scene : public EventHandler {
public:
	Scene(AsylumEngine *engine);
	virtual ~Scene();

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
	 * Enter the scene after a loaded game
	 */
	void enterLoad();

	/**
	 * Handle events
	 *
	 * @param ev The event
	 *
	 * @return true if it succeeds, false if it fails.
	 */
	bool handleEvent(const AsylumEvent &ev);

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
	Actor *getActor(ActorIndex index = kActorInvalid);

	/**
	 * Change player actor
	 *
	 * @param index new index for the player actor
	 */
	void changePlayer(ActorIndex index);

	/**
	 * Update player position when changing current player
	 *
	 * @param index Zero-based index of the actor
	 */
	void changePlayerUpdate(ActorIndex index);

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
	 * Updates the screen
	 *
	 * @return true if it succeeds, false if it fails.
	 */
	bool updateScreen();

	/**
	 * Updates the ambient sounds.
	 */
	void updateAmbientSounds();

	/**
	 * Rain drawing function for chapter 5.
	 */
	void drawRain();

	/**
	 * 	Determine if the supplied point intersects an action area's active region.
	 *
	 * @param	type	 	The type.
	 * @param	pt		 	The point.
	 * @param	highlight	(optional) whether to highlight the polygons as they are checked.
	 *
	 * @return	The found action area.
	 */
	int32 findActionArea(ActionAreaType type, const Common::Point &pt, bool highlight = false);

	/**
	 * Check if rectangles intersect.
	 *
	 * @param x  The x coordinate.
	 * @param y  The y coordinate.
	 * @param x1 The first x value.
	 * @param y1 The first y value.
	 * @param x2 The second x value.
	 * @param y2 The second y value.
	 * @param x3 The third int32.
	 * @param y3 The third int32.
	 *
	 * @return true if it succeeds, false if it fails.
	 */
	bool rectIntersect(int32 x, int32 y, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3) const;

	Polygons    *polygons()   { return _polygons; }
	WorldStats  *worldstats() { return _ws; }
	uint32 getFrameCounter() { return _frameCounter; }

	const byte *getSavedPalette() { return _savedPalette; }
	const Graphics::Surface &getSavedScreen() { return _savedScreen; }

private:
	AsylumEngine  *_vm;

	ResourcePackId _packId;

	Polygons     *_polygons;
	WorldStats   *_ws;

	struct UpdateItem {
		ActorIndex index;
		int32 priority;
	};

	// Music volume
	int32 _musicVolume;

	Common::Array<UpdateItem> _updateList;
	uint32 _frameCounter;

	Graphics::Surface _savedScreen;
	byte _savedPalette[PALETTE_SIZE];

	//////////////////////////////////////////////////////////////////////////
	// Message handling
	void activate();
	bool init();
	bool update();
	bool action(AsylumAction a);
	bool key(const AsylumEvent &evt);
	bool clickDown(const AsylumEvent &evt);

	//////////////////////////////////////////////////////////////////////////
	// Scene update
	//////////////////////////////////////////////////////////////////////////

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
	void updateCursor(ActorDirection direction, const Common::Rect &rect);

	//////////////////////////////////////////////////////////////////////////
	// Scene drawing
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Draw the loading screen
	 */
	void preload();

	/**
	 * Draw the scene
	 *
	 * @return true if it succeeds, false if it fails.
	 */
	bool drawScene();

	/**
	 * Builds the update list.
	 */
	void buildUpdateList();

	/**
	 * Process the update list.
	 */
	void processUpdateList();

	/**
	 * Compare two items priority on the update list
	 *
	 * @param item1 The first item.
	 * @param item2 The second item.
	 *
	 * @return true item1 priority is superior to item2 priority, false otherwise
	 */
	static bool updateListCompare(const UpdateItem &item1, const UpdateItem &item2);

	/**
	 * Check visible actors priority.
	 */
	void checkVisibleActorsPriority();

	/**
	 * Adjust actor priority.
	 *
	 * @param index Zero-based index of the actor
	 */
	void adjustActorPriority(ActorIndex index);

	int32 _chapter5RainFrameIndex;

	//////////////////////////////////////////////////////////////////////////
	// HitTest
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Run various hit tests and return the index, and a reference to the located type.
	 *
	 * @param [in,out] type The type.
	 *
	 * @return The index
	 */
	int32 hitTest(HitType &type);

	/**
	 * Checks if the supplied coordinates are inside an action area, object or actor, and returns -1 if nothing was found, or the type of hit if
	 * found.
	 *
	 * @param [in,out] type The type.
	 *
	 * @return The Index
	 */
	int32 hitTestScene(HitType &type);

	/**
	 * Check if the mouse cursor is currently intersecting an action area
	 *
	 * @return the index
	 */
	int32 hitTestActionArea();

	/**
	 * Check if the mouse cursor is currently intersecting the currently active actor.
	 *
	 * @return The actor index
	 */
	ActorIndex hitTestActor();

	/**
	 * Check if the mouse cursor is currently intersecting the player.
	 *
	 * @return true if it succeeds, false if it fails.
	 */
	bool hitTestPlayer();

	/**
	 * Check if a object exist at the supplied coordinates. If so, return it's index within the objects array, if not, return -1.
	 *
	 * @return the object index
	 */
	int32 hitTestObject();

	/**
	 * Check if the mouse cursor is currently intersecting a graphic resource at the supplied coordinates.
	 *
	 * @param resourceId Identifier for the resource.
	 * @param frame 	 The frame.
	 * @param x 		 The x coordinate.
	 * @param y 		 The y coordinate.
	 * @param flipped    true to flipped.
	 *
	 * @return true if it succeeds, false if it fails.
	 */
	bool hitTestPixel(ResourceId resourceId, uint32 frame, int16 x, int16 y, bool flipped);

	//////////////////////////////////////////////////////////////////////////
	// Hit actions
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Handle hit
	 *
	 * @param index The index
	 * @param type  The type.
	 */
	void handleHit(int32 index, HitType type);

	void clickInventory();

	void hitAreaChapter2(int32 id);
	bool _isCTRLPressed;
	int32 _hitAreaChapter7Counter;
	void hitAreaChapter7(int32 id);
	void hitAreaChapter11(int32 id);

	void hitActorChapter2(ActorIndex index);
	void hitActorChapter11(ActorIndex index);

	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Play intro speech.
	 */
	void playIntroSpeech();

	/**
	 * Stop speech.
	 */
	void stopSpeech();

	/**
	 * Play specific speech.
	 *
	 * @param code The key code.
	 *
	 * @return true if it succeeds, false if it fails.
	 */
	bool speak(Common::KeyCode code);

	/**
	 * Check if a point lies below the rectangle's top-left to bottom-right diagonal.
	 *
	 * @param point The point.
	 * @param rect  The rectangle.
	 *
	 * @return true if below, false if above.
	 */
	bool pointBelowLine(const Common::Point &point, const Common::Rect &rect) const;

	/**
	 * Adjust coordinates.
	 *
	 * @param point The point.
	 */
	void adjustCoordinates(Common::Point *point);

	//////////////////////////////////////////////////////////////////////////
	// Scene debugging
	//////////////////////////////////////////////////////////////////////////
	void debugShowActors();
	void debugShowObjects();
	void debugShowPolygons();
	void debugShowPolygon(uint32 index, uint32 color = 0xFF);
	void debugHighlightPolygon(uint32 index);
	void debugShowSceneRects();
	void debugScreenScrolling();
	void debugShowWalkRegion(Polygon *poly);

	friend class SceneTitle;
};

} // end of namespace Asylum

#endif // ASYLUM_VIEWS_SCENE_H
