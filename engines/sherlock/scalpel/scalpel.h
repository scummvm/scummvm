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

#ifndef SHERLOCK_SCALPEL_H
#define SHERLOCK_SCALPEL_H

#include "sherlock/sherlock.h"
#include "sherlock/scalpel/scalpel_darts.h"

namespace Sherlock {

namespace Scalpel {

extern uint BUTTON_TOP;
extern uint BUTTON_MIDDLE;
extern uint BUTTON_BOTTOM;
extern uint COMMAND_FOREGROUND;
extern uint COMMAND_HIGHLIGHTED;
extern uint COMMAND_NULL;
extern uint INFO_FOREGROUND;
extern uint INFO_BACKGROUND;
extern uint INV_FOREGROUND;
extern uint INV_BACKGROUND;
extern uint PEN_COLOR;
extern uint INFO_BLACK;
extern uint BORDER_COLOR;
extern uint COMMAND_BACKGROUND;
extern uint BUTTON_BACKGROUND;
extern uint TALK_FOREGROUND;
extern uint TALK_NULL;

class ScalpelEngine : public SherlockEngine {
private:
	Darts *_darts;
	int _mapResult;

	/**
	 * Initialize graphics mode
	 */
	void setupGraphics();

	/**
	 * Show the 3DO splash screen
	 */
	bool show3DOSplash();

	/**
	 * Show the starting city cutscene which shows the game title
	 */
	bool showCityCutscene();
	bool showCityCutscene3DO();

	/**
	 * Show the back alley where the initial murder takes place
	 */
	bool showAlleyCutscene();
	bool showAlleyCutscene3DO();

	/**
	 * Show the Baker Street outside cutscene
	 */
	bool showStreetCutscene();
	bool showStreetCutscene3DO();

	/**
	 * Show Holmes and Watson at the breakfast table, lestrade's note, and then the scrolling credits
	 */
	bool showOfficeCutscene();
	bool showOfficeCutscene3DO();

	/**
	 * Show the game credits
	 */
	bool scrollCredits();

	/**
	 * Load the default inventory for the game, which includes both the initial active inventory,
	 * as well as special pending inventory items which can appear automatically in the player's
	 * inventory once given required flags are set
	 */
	void loadInventory();

	/**
	 * Transition to show an image
	 */
	void showLBV(const Common::String &filename);
protected:
	/**
	 * Game initialization
	 */
	void initialize() override;

	/**
	 * Show the opening sequence
	 */
	void showOpening() override;

	/**
	 * Starting a scene within the game
	 */
	void startScene() override;
public:
	ScalpelEngine(OSystem *syst, const SherlockGameDescription *gameDesc);
	~ScalpelEngine() override;

	/**
	 * Takes care of clearing the mirror in scene 12 (mansion drawing room), in case anything drew over it
	 */
	void eraseBrumwellMirror();

	/**
	 * Takes care of drawing Holme's reflection onto the mirror in scene 12 (mansion drawing room)
	 */
	void doBrumwellMirror();

	/**
	 * This clears the mirror in scene 12 (mansion drawing room) in case anything messed draw over it
	 */
	void flushBrumwellMirror();

	/**
	 * Show the ScummVM restore savegame dialog
	 */
	void showScummVMSaveDialog();

	/**
	 * Show the ScummVM restore savegame dialog
	 */
	void showScummVMRestoreDialog();

	/**
	 * Play back a 3do movie
	 */
	bool play3doMovie(const Common::String &filename, const Common::Point &pos, bool isPortrait = false);
};

} // End of namespace Scalpel

} // End of namespace Sherlock

#endif
