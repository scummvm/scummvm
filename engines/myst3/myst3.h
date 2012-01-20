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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

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

namespace Myst3 {

// Engine Debug Flags
enum {
	kDebugVariable = (1 << 0),
	kDebugSaveLoad = (1 << 1),
	kDebugNode     = (1 << 2),
	kDebugScript   = (1 << 3)
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
struct NodeData;

typedef Common::SharedPtr<NodeData> NodePtr;

class Myst3Engine : public Engine {

protected:
	// Engine APIs
	virtual Common::Error run();
	virtual GUI::Debugger *getDebugger() { return (GUI::Debugger *)_console; }
public:
	GameState *_state;
	Scene *_scene;
	Cursor *_cursor;
	Inventory *_inventory;
	Renderer *_gfx;
	Menu *_menu;
	Database *_db;
	Sound *_sound;
	
	Common::RandomSource *_rnd;

	// Used by the projectors on J'nanin, see puzzle #14
	Graphics::Surface *_projectorBackground;

	Myst3Engine(OSystem *syst, int gameFlags);
	virtual ~Myst3Engine();

	Common::Error loadGameState(int slot);

	const DirectorySubEntry *getFileDescription(const char* room, uint16 index, uint16 face, DirectorySubEntry::ResourceType type);
	Graphics::Surface *loadTexture(uint16 id);

	void goToNode(uint16 nodeID, uint transition);
	void loadNode(uint16 nodeID, uint32 roomID = 0, uint32 ageID = 0);
	void loadNodeCubeFaces(uint16 nodeID);
	void loadNodeFrame(uint16 nodeID);
	void loadNodeMenu(uint16 nodeID);
	int16 openDialog(uint16 id);

	void runNodeInitScripts();
	void runNodeBackgroundScripts();
	void runScriptsFromNode(uint16 nodeID, uint32 roomID = 0, uint32 ageID = 0);

	void loadMovie(uint16 id, uint16 condition, bool resetCond, bool loop);
	void playSimpleMovie(uint16 id);
	void removeMovie(uint16 id);
	void setMovieLooping(uint16 id, bool loop);

	void addSpotItem(uint16 id, uint16 condition, bool fade);
	void addMenuSpotItem(uint16 id, uint16 condition, const Common::Rect &rect);
	void addSunSpot(uint16 pitch, uint16 heading, uint16 intensity,
			uint16 color, uint16 var, bool varControlledIntensity, uint16 radius);

	void setMenuAction(uint16 action) { _menuAction = action; }
	void setShouldQuit() { _shouldQuit = true; }

	void processInput(bool lookOnly);
	void drawFrame();
private:
	OSystem *_system;
	Console *_console;
	
	Node *_node;

	Common::Array<Archive *> _archivesCommon;
	Archive *_archiveNode;

	Script *_scriptEngine;

	Common::Array<ScriptedMovie *> _movies;
	Common::Array<Drawable *> _drawables;

	bool _shouldQuit;
	uint16 _menuAction;

	Common::Array<HotSpot *> listHoveredHotspots(NodePtr nodeData);
	void updateCursor();

	void addArchive(const Common::String &file, bool mandatory);

	friend class Console;
};

} // end of namespace Myst3

#endif
