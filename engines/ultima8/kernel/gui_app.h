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

#ifndef ULTIMA8_KERNEL_GUIAPP_H
#define ULTIMA8_KERNEL_GUIAPP_H

#include "ultima8/std/containers.h"
#include "ultima8/usecode/intrinsics.h"
#include "ultima8/misc/args.h"

#include "ultima8/kernel/core_app.h"
#include "ultima8/kernel/mouse.h"
#include "ultima8/kernel/hid_keys.h"
#include "ultima8/misc/console.h"
#include "ultima8/misc/p_dynamic_cast.h"
#include "common/events.h"

namespace Ultima8 {

class Kernel;
class UCMachine;
class Game;
class Gump;
class ConsoleGump;
class GameMapGump;
class ScalerGump;
class InverterGump;
class RenderSurface;
class PaletteManager;
class GameData;
class World;
class ObjectManager;
class FontManager;
class HIDManager;
class AvatarMoverProcess;
class IDataSource;
class ODataSource;
struct Texture;

namespace Pentagram {
class AudioMixer;

const unsigned int savegame_version = 5;
}

// Hack alert
struct HWMouseCursor;
#if defined(WIN32) && defined(I_AM_COLOURLESS_EXPERIMENTING_WITH_HW_CURSORS)
#include <windows.h>
#endif

class GUIApp : public CoreApp {
public:
	ENABLE_RUNTIME_CLASSTYPE()

	GUIApp(int argc, const char *const *argv);
	virtual ~GUIApp();

	static GUIApp *get_instance() {
		return p_dynamic_cast<GUIApp *>(application);
	}

	void startup();
	void shutdown();

	void startupGame();
	void startupPentagramMenu();
	void shutdownGame(bool reloading = true);
	void changeGame(Pentagram::istring newgame);

	// When in the Pentagram Menu, load minimal amount of data for the specific game
	// Used to enable access to the games gumps and shapes
	void menuInitMinimal(Pentagram::istring game);

	void changeVideoMode(int width, int height, int fullscreen = -1); // -1 = no change, -2 = fullscreen toggle
	RenderSurface *getScreen() {
		return screen;
	}

	virtual void run();
	virtual void handleEvent(const Common::Event &event);

	virtual void paint();
	virtual bool isPainting() {
		return painting;
	}


	INTRINSIC(I_getCurrentTimerTick);
	INTRINSIC(I_setAvatarInStasis);
	INTRINSIC(I_getAvatarInStasis);
	INTRINSIC(I_getTimeInGameHours);
	INTRINSIC(I_getTimeInMinutes);
	INTRINSIC(I_getTimeInSeconds);
	INTRINSIC(I_setTimeInGameHours);
	INTRINSIC(I_avatarCanCheat);
	INTRINSIC(I_makeAvatarACheater);
	INTRINSIC(I_closeItemGumps);

	void setAvatarInStasis(bool stat) {
		avatarInStasis = stat;
	}
	bool isAvatarInStasis() const {
		return avatarInStasis;
	}
	void toggleAvatarInStasis() {
		avatarInStasis = !avatarInStasis;
	}
	bool isPaintEditorItems() const {
		return paintEditorItems;
	}
	void togglePaintEditorItems() {
		paintEditorItems = !paintEditorItems;
	}
	bool isShowTouchingItems() const {
		return showTouching;
	}
	void toggleShowTouchingItems() {
		showTouching = !showTouching;
	}

	uint32 getGameTimeInSeconds();

	GameMapGump *getGameMapGump() {
		return gameMapGump;
	}
	ConsoleGump *getConsoleGump() {
		return consoleGump;
	}
	Gump *getDesktopGump() {
		return desktopGump;
	}
	Gump *getGump(uint16 gumpid);

	//! add a gump to the right core gump (such as desktopgump)
	//! \param gump the gump to be added
	void addGump(Gump *gump);

	AvatarMoverProcess *getAvatarMoverProcess() {
		return avatarMoverProcess;
	}

	//! save a game
	//! \param filename the file to save to
	//! \return true if succesful
	bool saveGame(std::string filename, std::string desc,
	              bool ignore_modals = false);

	//! load a game
	//! \param filename the savegame to load
	//! \return true if succesful.
	bool loadGame(std::string filename);

	//! start a new game
	//! \return true if succesful.
	bool newGame(const std::string &savegame);

	//! get mouse cursor length. 0 = short, 1 = medium, 2 = long
	int getMouseLength(int mx, int my);

	//! get mouse cursor direction. 0 = up, 1 = up-right, 2 = right, etc...
	int getMouseDirection(int mx, int my);

	//! get current mouse cursor location
	void getMouseCoords(int &mx, int &my) {
		mx = mouseX;
		my = mouseY;
	}

	//! set current mouse cursor location
	void setMouseCoords(int mx, int my);

	bool isMouseDown(MouseButton button);

	enum MouseCursor {
		MOUSE_NORMAL = 0,
		MOUSE_NONE = 1,
		MOUSE_TARGET = 2,
		MOUSE_PENTAGRAM = 3,
		MOUSE_HAND = 4,
		MOUSE_QUILL = 5,
		MOUSE_MAGGLASS = 6,
		MOUSE_CROSS = 7,
		MOUSE_POINTER = 8  //!< Pentagrams default pointer
	};

	//! set the current mouse cursor
	void setMouseCursor(MouseCursor cursor);

	//! flash the red cross mouse cursor for a brief while
	void flashCrossCursor();

	//! push the current mouse cursor to the stack
	void pushMouseCursor();

	//! pop the last mouse cursor from the stack
	void popMouseCursor();

	//! Enter gump text mode (aka SDL Unicode keyhandling)
	void enterTextMode(Gump *);

	//! Leave gump text mode (aka SDL Unicode keyhandling)
	void leaveTextMode(Gump *);

	//! Display an error message box
	//! \param message The message to display on the box
	//! \param exit_to_menu If true, then exit to the Pentagram menu then display the message
	void Error(std::string message, std::string title = std::string(), bool exit_to_menu = false);

protected:
	virtual void DeclareArgs();

private:
	uint32 save_count;

	//! write savegame info (time, ..., game-specifics)
	void writeSaveInfo(ODataSource *ods);

	//! save CoreApp/GUIApp data
	void save(ODataSource *ods);

	//! load CoreApp/GUIApp data
	bool load(IDataSource *ids, uint32 version);

	//! reset engine (including World, UCMachine, a.o.)
	void resetEngine();

	//! create core gumps (DesktopGump, GameMapGump, ConsoleGump, ...)
	void setupCoreGumps();

	//! Does a Full reset of the Engine (including shutting down Video)
	void fullReset();

	// full system
	Game *game;
	Pentagram::istring change_gamename;
	std::string error_message;
	std::string error_title;

	Kernel *kernel;
	ObjectManager *objectmanager;
	HIDManager *hidmanager;
	UCMachine *ucmachine;
	RenderSurface *screen;
	bool fullscreen;
	PaletteManager *palettemanager;
	GameData *gamedata;
	World *world;
	FontManager *fontmanager;

	Gump            *desktopGump;
	ConsoleGump     *consoleGump;
	GameMapGump     *gameMapGump;
	ScalerGump      *scalerGump;
	InverterGump    *inverterGump;

	AvatarMoverProcess *avatarMoverProcess;

	// called depending upon command line arguments
	void GraphicSysInit(); // starts/restarts the graphics subsystem
	bool LoadConsoleFont(std::string confontini); // loads the console font

	void handleDelayedEvents();

	// Various dependancy flags
	bool runSDLInit;

	// Timing stuff
	int32 lerpFactor;       //!< Interpolation factor for this frame (0-256)
	bool inBetweenFrame;    //!< Set true if we are doing an inbetween frame

	bool frameSkip;         //!< Set to true to enable frame skipping (default false)
	bool frameLimit;        //!< Set to true to enable frame limiting (default true)
	bool interpolate;       //!< Set to true to enable interpolation (default true)
	int32 animationRate;    //!< The animation rate. Affects all processes! (default 100)

	// Sort of Camera Related Stuff, move somewhere else

	bool avatarInStasis;    //!< If this is set to true, Avatar can't move,
	//!< nor can Avatar start more usecode
	bool paintEditorItems;  //!< If true, paint items with the SI_EDITOR flag

	bool painting;          //!< Set true when painting

	bool showTouching;          //!< If true, highlight items touching Avatar

	int mouseX, mouseY;

	Texture *defMouse;      //!< Default Pentagram mouse for when there is no GameData

	//! get the current mouse frame
	int getMouseFrame();
	std::stack<MouseCursor> cursors;
	uint32 flashingcursor; // time (g_system->getMillis) mouse started flashing, or 0

	// Hack alert
	HWMouseCursor           *hwcursors; // Shape frames converted into system specific format.

#if defined(WIN32) && defined(I_AM_COLOURLESS_EXPERIMENTING_WITH_HW_CURSORS)
	void                    CreateHWCursors();
	static LRESULT CALLBACK myWindowProc(HWND, UINT, WPARAM, LPARAM);

#endif

	static void conAutoPaint(void);

	// mouse input state
	MButton mouseButton[MOUSE_LAST];

	uint16 mouseOverGump;

	enum DraggingState {
		DRAG_NOT = 0,
		DRAG_OK = 1,
		DRAG_INVALID = 2,
		DRAG_TEMPFAIL = 3
	} dragging;
	ObjId dragging_objid;
	uint16 dragging_item_startgump;
	uint16 dragging_item_lastgump;

	int dragging_offsetX, dragging_offsetY;
	unsigned int inversion;

	uint32 lastDown[HID_LAST];
	uint8 down[HID_LAST];
public:
	void setDraggingOffset(int x, int y) {
		dragging_offsetX = x;
		dragging_offsetY = y;
	}
	void getDraggingOffset(int &x, int &y) {
		x = dragging_offsetX;
		y = dragging_offsetY;
	}

	unsigned int getInversion() const {
		return inversion;
	}
	void setInversion(unsigned int i) {
		inversion = i & 0xFFFF;
	}
	bool isInverted() {
		return (inversion >= 0x4000 && inversion < 0xC000);
	}

private:
	void startDragging(int mx, int my);
	void moveDragging(int mx, int my);
	void stopDragging(int mx, int my);

	int32 timeOffset;
	bool has_cheated;
	bool cheats_enabled;

public:
	bool areCheatsEnabled() const {
		return cheats_enabled;
	}
	void setCheatMode(bool enabled) {
		cheats_enabled = enabled;
	}
	bool hasCheated() const {
		return has_cheated;
	}
	void makeCheater() {
		has_cheated = true;
	}

private:

	std::list<ObjId>    textmodes;      //!< Gumps that want text mode

	// Load and save games from arbitrary filenames from the console
	static void         ConCmd_saveGame(const Console::ArgvType &argv);         //!< "GUIApp::saveGame <filename>" console command
	static void         ConCmd_loadGame(const Console::ArgvType &argv);         //!< "GUIApp::loadGame <filename>" console command
	static void         ConCmd_newGame(const Console::ArgvType &argv);          //!< "GUIApp::newGame" console command

	static void         ConCmd_quit(const Console::ArgvType &argv);             //!< "quit" console command

	static void         ConCmd_changeGame(const Console::ArgvType &argv);       //!< "GuiApp::changeGame" console command
	static void         ConCmd_listGames(const Console::ArgvType &argv);            //!< "GuiApp::listGames" console command

	static void         ConCmd_setVideoMode(const Console::ArgvType &argv);     //!< "GuiApp::setVideoMode" console command
	static void         ConCmd_toggleFullscreen(const Console::ArgvType &argv); //!< "GuiApp::toggleFullscreen" console command

	// This should be a console variable once they are implemented
	bool                drawRenderStats;
	static void         ConCmd_drawRenderStats(const Console::ArgvType &argv);  //!< "GUIApp::drawRenderStats" console command

	static void         ConCmd_engineStats(const Console::ArgvType &argv);  //!< "GUIApp::engineStats" console command

	static void         ConCmd_toggleAvatarInStasis(const Console::ArgvType &argv); //!< "GUIApp::toggleAvatarInStasis" console command
	static void         ConCmd_togglePaintEditorItems(const Console::ArgvType &argv);   //!< "GUIApp::togglePaintEditorItems" console command
	static void         ConCmd_toggleShowTouchingItems(const Console::ArgvType &argv);  //!< "GUIApp::toggleShowTouchingItems" console command

	static void         ConCmd_closeItemGumps(const Console::ArgvType &argv);   //!< "GUIApp::closeItemGumps" console command

	static void         ConCmd_toggleCheatMode(const Console::ArgvType &argv);  //!< "Cheat::toggle" console command

	static void         ConCmd_memberVar(const Console::ArgvType &argv);    //!< "GuiApp::memberVar <member> [newvalue] [updateini]" console command

	bool                ttfoverrides;

	// Audio Mixer
	Pentagram::AudioMixer *audiomixer;
};

} // End of namespace Ultima8

#endif
