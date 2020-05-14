/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef MYST3_ENGINE_H
#define MYST3_ENGINE_H

#include "engines/engine.h"

#include "common/array.h"
#include "common/ptr.h"
#include "common/system.h"
#include "common/random.h"

#include "engines/myst3/archive.h"

namespace Graphics {
struct Surface;
}

namespace Common {
struct Event;
}

namespace Myst3 {

// Engine Debug Flags
enum {
	kDebugVariable = (1 << 0),
	kDebugSaveLoad = (1 << 1),
	kDebugNode     = (1 << 2),
	kDebugScript   = (1 << 3)
};

enum TransitionType {
	kTransitionFade = 1,
	kTransitionNone,
	kTransitionZip,
	kTransitionLeftToRight,
	kTransitionRightToLeft
};

class Archive;
class Console;
class Drawable;
class GameState;
class HotSpot;
class Cursor;
class Inventory;
class Database;
class Scene;
class Script;
class SpotItemFace;
class SunSpot;
class Renderer;
class Menu;
class Node;
class Sound;
class Ambient;
class ScriptedMovie;
class ShakeEffect;
class RotationEffect;
class Transition;
class FrameLimiter;
struct NodeData;
struct Myst3GameDescription;

typedef Common::SharedPtr<NodeData> NodePtr;

class Myst3Engine : public Engine {

protected:
	// Engine APIs
	virtual Common::Error run() override;
	virtual void syncSoundSettings() override;
	virtual void pauseEngineIntern(bool pause) override;

public:
	GameState *_state;
	Scene *_scene;
	Cursor *_cursor;
	Inventory *_inventory;
	Renderer *_gfx;
	Menu *_menu;
	Database *_db;
	Sound *_sound;
	Ambient *_ambient;
	
	Common::RandomSource *_rnd;

	// Used by the projectors on J'nanin, see puzzle #14
	Graphics::Surface *_projectorBackground;

	Myst3Engine(OSystem *syst, const Myst3GameDescription *version);
	virtual ~Myst3Engine();

	bool hasFeature(EngineFeature f) const override;
	Common::Platform getPlatform() const;
	Common::Language getGameLanguage() const;
	uint32 getGameLocalizationType() const;
	bool isTextLanguageEnglish() const;
	bool isWideScreenModEnabled() const;

	bool canSaveGameStateCurrently() override;
	bool canLoadGameStateCurrently() override;
	Common::Error loadGameState(int slot) override;
	Common::Error loadGameState(Common::String fileName, TransitionType transition);
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	Common::Error saveGameState(const Common::String &desc, const Graphics::Surface *thumbnail, bool isAutosave);

	ResourceDescription getFileDescription(const Common::String &room, uint32 index, uint16 face,
	                                            Archive::ResourceType type);
	ResourceDescriptionArray listFilesMatching(const Common::String &room, uint32 index, uint16 face,
	                                        Archive::ResourceType type);

	Graphics::Surface *loadTexture(uint16 id);
	static Graphics::Surface *decodeJpeg(const ResourceDescription *jpegDesc);

	void goToNode(uint16 nodeID, TransitionType transition);
	void loadNode(uint16 nodeID, uint32 roomID = 0, uint32 ageID = 0);
	void unloadNode();
	void loadNodeCubeFaces(uint16 nodeID);
	void loadNodeFrame(uint16 nodeID);
	void loadNodeMenu(uint16 nodeID);

	void setupTransition();
	void drawTransition(TransitionType transitionType);

	void dragItem(uint16 statusVar, uint16 movie, uint16 frame, uint16 hoverFrame, uint16 itemVar);
	void dragSymbol(uint16 var, uint16 id);
	int16 openDialog(uint16 id);

	void runNodeInitScripts();
	void runNodeBackgroundScripts();
	void runScriptsFromNode(uint16 nodeID, uint32 roomID = 0, uint32 ageID = 0);
	void runBackgroundSoundScriptsFromNode(uint16 nodeID, uint32 roomID = 0, uint32 ageID = 0);
	void runAmbientScripts(uint32 node);

	void loadMovie(uint16 id, uint16 condition, bool resetCond, bool loop);
	void playMovieGoToNode(uint16 movie, uint16 node);
	void playMovieFullFrame(uint16 movie);
	void playSimpleMovie(uint16 id, bool fullframe = false, bool refreshAmbientSounds = false);
	void removeMovie(uint16 id);
	void setMovieLooping(uint16 id, bool loop);

	void addSpotItem(uint16 id, int16 condition, bool fade);
	SpotItemFace *addMenuSpotItem(uint16 id, int16 condition, const Common::Rect &rect);
	void loadNodeSubtitles(uint32 id);

	void addSunSpot(uint16 pitch, uint16 heading, uint16 intensity,
			uint16 color, uint16 var, bool varControlledIntensity, uint16 radius);
	SunSpot computeSunspotsIntensity(float pitch, float heading);

	void setMenuAction(uint16 action) { _menuAction = action; }

	void animateDirectionChange(float pitch, float heading, uint16 scriptTicks);
	void getMovieLookAt(uint16 id, bool start, float &pitch, float &heading);

	void drawFrame(bool noSwap = false);

	void processInput(bool interactive);
	void processEventForKeyboardState(const Common::Event &event);
	void processEventForGamepad(const Common::Event &event);
	void updateInputState();

	bool inputValidatePressed();
	bool inputEscapePressed();
	bool inputSpacePressed();
	bool inputTilePressed();

	void settingsInitDefaults();
	void settingsLoadToVars();
	void settingsApplyFromVars();

private:
	OSystem *_system;
	const Myst3GameDescription *_gameDescription;

	Node *_node;

	Common::Array<Archive *> _archivesCommon;
	Archive *_archiveNode;

	Script *_scriptEngine;

	Common::Array<ScriptedMovie *> _movies;
	Common::Array<SunSpot *> _sunspots;
	Common::Array<Drawable *> _drawables;

	uint16 _menuAction;

	// Used by Amateria's magnetic rings
	ShakeEffect *_shakeEffect;
	// Used by Voltaic's spinning gears
	RotationEffect *_rotationEffect;

	FrameLimiter *_frameLimiter;
	Transition *_transition;

	bool _inputSpacePressed;
	bool _inputEnterPressed;
	bool _inputEscapePressed;
	bool _inputEscapePressedNotConsumed;
	bool _inputTildePressed;

	bool _interactive;

	uint32 _backgroundSoundScriptLastRoomId;
	uint32 _backgroundSoundScriptLastAgeId;

	/**
	 * When the widescreen mode is active, the user can manually hide
	 * the inventory by clicking on an unused inventory space.
	 * This allows interacting with the scene portion that is below
	 * the inventory.
	 */
	bool _inventoryManualHide;

	HotSpot *getHoveredHotspot(NodePtr nodeData, uint16 var = 0);
	void updateCursor();

	bool checkDatafiles();

	bool addArchive(const Common::String &file, bool mandatory);
	void openArchives();
	void closeArchives();

	bool isInventoryVisible();

	void interactWithHoveredElement();

	friend class Console;
};

} // end of namespace Myst3

#endif
