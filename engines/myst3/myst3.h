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

#include "common/system.h"
#include "common/random.h"

#include "engines/myst3/archive.h"
#include "engines/myst3/movie.h"
#include "engines/myst3/node.h"
#include "engines/myst3/scene.h"

namespace Graphics {
struct Surface;
}

namespace Common {
struct Event;
}

namespace Myst3 {

enum GameVersionFlags {
	kFlagNone      = 0,
	kFlagVersion10 = (1 << 0), // v1.0
	kFlagDVD       = (1 << 1)  // DVD version
};

typedef uint32 SafeDiskKey[4];

struct ExecutableVersion {
	const char *description;
	int flags;
	const char *executable;
	uint32 baseOffset;
	uint32 ageTableOffset;
	uint32 nodeInitScriptOffset;
	uint32 soundNamesOffset;
	uint32 ambientCuesOffset;
	const SafeDiskKey *safeDiskKey;
};

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

class Console;
class GameState;
class HotSpot;
class Cursor;
class Inventory;
class Database;
class Script;
class Renderer;
class Menu;
class Sound;
class Ambient;
class ShakeEffect;
class RotationEffect;
class Transition;
struct NodeData;
struct Myst3GameDescription;

typedef Common::SharedPtr<NodeData> NodePtr;

class Myst3Engine : public Engine {

protected:
	// Engine APIs
	virtual Common::Error run() override;
	virtual void syncSoundSettings() override;
	virtual GUI::Debugger *getDebugger() override { return (GUI::Debugger *)_console; }
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
	Common::Language getDefaultLanguage() const;
	bool isMonolingual() const;
	const ExecutableVersion *getExecutableVersion() const;

	bool canLoadGameStateCurrently() override;
	Common::Error loadGameState(int slot) override;
	Common::Error loadGameState(Common::String fileName, TransitionType transition);

	const DirectorySubEntry *getFileDescription(const char* room, uint32 index, uint16 face, DirectorySubEntry::ResourceType type);
	Graphics::Surface *loadTexture(uint16 id);
	static Graphics::Surface *decodeJpeg(const DirectorySubEntry *jpegDesc);

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
	void playSimpleMovie(uint16 id, bool fullframe = false);
	void removeMovie(uint16 id);
	void setMovieLooping(uint16 id, bool loop);

	void addSpotItem(uint16 id, uint16 condition, bool fade);
	SpotItemFace *addMenuSpotItem(uint16 id, uint16 condition, const Common::Rect &rect);
	void loadNodeSubtitles(uint32 id);

	void addSunSpot(uint16 pitch, uint16 heading, uint16 intensity,
			uint16 color, uint16 var, bool varControlledIntensity, uint16 radius);
	SunSpot computeSunspotsIntensity(float pitch, float heading);

	void setMenuAction(uint16 action) { _menuAction = action; }

	void animateDirectionChange(float pitch, float heading, uint16 speed);
	void getMovieLookAt(uint16 id, bool start, float &pitch, float &heading);

	void processInput(bool lookOnly);
	void drawFrame(bool noSwap = false);

	bool inputValidatePressed();
	bool inputEscapePressed();
	bool inputSpacePressed();
	bool inputTilePressed();

	void settingsInitDefaults();
	void settingsLoadToVars();
	void settingsApplyFromVars();
private:
	OSystem *_system;
	Console *_console;
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

	Transition *_transition;

	bool _inputSpacePressed;
	bool _inputEnterPressed;
	bool _inputEscapePressed;
	bool _inputTildePressed;

	uint32 _backgroundSoundScriptLastRoomId;

	HotSpot *getHoveredHotspot(NodePtr nodeData, uint16 var = 0);
	void updateCursor();

	bool checkDatafiles();

	bool addArchive(const Common::String &file, bool mandatory);
	void openArchives();
	void closeArchives();

	bool isInventoryVisible();

	void interactWithHoveredElement(bool lookOnly);
	void processEventForGamepad(const Common::Event &event);

	friend class Console;
};

} // end of namespace Myst3

#endif
