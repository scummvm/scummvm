/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_ICONMENU_H_INCLUDED
#define ICB_ICONMENU_H_INCLUDED

// Include headers needed by this class.
#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/common/px_string.h"
#include "engines/icb/common/px_assert.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_bitmap.h"
#include "engines/icb/player.h"
#include "engines/icb/icon_list.h"
#include "engines/icb/icon_menu_pc.h"

namespace ICB {

// Use globals as it reduces rdata storage on PSX
extern const char *global_nothing_selected;

#define ICON_MENU_LOG "icon_menu_log.txt"

// This defines the maximum number of icons that can be in the menu.
#define ICON_MENU_MAX_ICONS 15 // NB: CHECK PADDING IMPLICATIONS FOR PSX BEFORE CHANGING THIS!

// This is the standard number of frames in each icon.
#define ICON_FRAME_COUNT 2

// Controls how fast the highlight flashes.
#define ICON_MENU_HIGHLIGHT_SPEED 5

// This controls the flash rate of the added medipacks and ammo clips.
#define ICON_MENU_ADDED_FLASHRATE 4

// This controls the flashrate of the email-waiting symbol.
#define ICON_MENU_EMAIL_FLASHRATE 24

// These define the names of the off-screen indicator arrows.
#define ICON_MENU_OFF_SCREEN_LEFT "left_arrow"
#define ICON_MENU_OFF_SCREEN_RIGHT "right_arrow"

// These are names for the ammo and healthcare stuff.
#define ARMS_GUN_NAME "gun"
#define ARMS_AMMO_NAME "ammo"
#define ARMS_HEALTH_NAME "health_pack"
#define EMAIL_ARRIVED_NAME "mail"

// Used for telling which way we are scrolling / if we are scrolling
#define ICON_MENU_SCROLL_NONE 0
#define ICON_MENU_SCROLL_LEFT 1
#define ICON_MENU_SCROLL_RIGHT 2

// struct _icon_menu_duplicates
// This structure must be passed into the icon menu upon activation, whereupon the information in it
// will be used to add a count to the number of each item currently in the menu.
struct _icon_menu_duplicates {
	uint8 s_nNumItems;
	uint8 s_pnItemCounts[ICON_MENU_MAX_ICONS];

	// Initialisation.
	_icon_menu_duplicates() { s_nNumItems = 0; }
};

// This class presents a set of icons to the user and runs the selection interface.  It is packaged as a separate
// class because it is used by the Remora, the speech system and the inventory.
class _icon_menu {
public:
	// Default constructor and destructor.
	_icon_menu();
	~_icon_menu() {}

	// If the menu is up, call this to process its logic and draw the menu.
	bool8 CycleIconMenu(const _input &sKeyboardState);

	// Draw the special armed inventory
	void DrawArmedMenu(const int nBullets, const int maxBullets, const int nClips, const int maxClips);

	// And this draws it each cycle.
	void DrawIconMenu();

	// Call this to close the menu down.
	void CloseDownIconMenu();

	// This is the main function for running a menu selection.
	void Activate(const _icon_list *pIconList, const _icon_menu_duplicates &sDuplicates, bool8 bAllowEscape, uint32 nSelected);

	// A funstion to recreate/load the icon surfaces after task switching
	void ReActivate();

	// This reports whether or not the icon menu is active.
	bool8 IsActive() const { return ((m_eIconMenuGameState == ACTIVE) ? TRUE8 : FALSE8); }

	// This says whether or not there is currently a valid selection (i.e. player is 'holding' something.)
	bool8 IsHolding() const { return (m_bValidSelection); }

	// This draws the current selection for as long as it is still 'held'.
	void CycleHoldingLogic();

	// These handle the logic and drawing for adding medipacks and clips.
	void SetAddingClipsCount(uint32 nNumClips);
	void SetAddingMedipacksCount(uint32 nNumMedipacks);
	bool8 IsAdding() const;
	void CycleAddingLogic();
	void DrawAdding();

	// An icon needs to flash when an email arrives.  This mechanism borrows the logic from the medipacks and clips above.
	void SetEmailArrived();
	void ClearEmailArrived() { m_bEmailArrived = FALSE8; }

	// This reports the last menu selection if one is available.
	const char *GetLastSelection();
	uint32 GetLastSelectionHash() const;

	// This clears the selection.
	void ClearSelection() { m_bValidSelection = FALSE8; }

	uint32 GetTransparencyKey() const { return (m_nTransparentKey); }
	void SetTransparencyColourKey();

	// These are used for PSX smoothing.  (Won't do any harm on the PC.)
	void PreloadIcon(const char *pcIconPath, const char *pcIconName);

      private:
	int GetScrollingPosition(const int nInputX, uint32 &nIconIndex);

	enum IconMenuGameState { INACTIVE = 0, ACTIVE };

	IconMenuGameState m_eIconMenuGameState; // Current state of the icon menu object.
	const _icon_list *m_pIconList;          // Pointer to the list of icons to run the selection for.
	uint32 m_nLastSelection;                // Stores the last selection made by the chooser.
	uint32 m_nSelectedIcon;                 // Stores the icon currently centred in the screen.
	uint32 m_nHighlightCounter;             // Used to control frequency of highlight flashing.
	const char *m_pcSelectedIconName;       // Name of the currently selected icon.
	uint32 m_nSelectedIconHash;             // Hash of the currently selected icon.
	uint32 m_nIconClusterHash;              // Icon cluster hash value.
	_icon_menu_duplicates m_sDuplicates;    // Count of any duplicate items.
	uint32 m_nGlobalClusterHash;            // Hash value for the global cluster.

	uint32 m_pnIconSurfaceIDs[ICON_MENU_MAX_ICONS];   // IDs of surfaces where non-highlighted icons are stored.
	uint32 m_pnHiLiteSurfaceIDs[ICON_MENU_MAX_ICONS]; // IDs of surfaces for storing highlighted versions.
	uint32 m_nLeftArrowID;                            // The left-off screen arrow.
	uint32 m_nRightArrowID;                           // Highlighted version of it.
	uint32 m_nLeftArrowHiLiteID;                      // The right-off screen arrow.
	uint32 m_nRightArrowHiLiteID;                     // Highlighted version of it.
	uint32 m_nAddedClipsSurface;                      // Surface for the flashing of added clips.
	uint32 m_nAddedMedipacksSurface;                  // Surface for the flashing of added medipacks.
	uint32 m_nEmailArrivedSurface;                    // Surface for the email-arrived icon.
	uint32 m_nMenuY;                                  // The y height to draw the menu at
	uint32 m_nTransparentKey;                         // Use this colour for transparency in blitting.
	char m_pcGlobalClusterFile[MAXLEN_CLUSTER_URL];   // The global cluster file.
	char m_pcIconCluster[MAXLEN_CLUSTER_URL];         // Icon cluster name.

	bool8 m_bValidSelection;   // Gets set true when a selection is made and false when it is read.
	bool8 m_nKeyLock;          // Stops key-repeat.
	bool8 m_bHighlightVisible; // Whether or not the highlight is visible on the selected icon.
	bool8 m_bAllowEscape;      // Allows backstep to escape from the menu if true.

	bool8 m_bWiderThanScreen; // Set to true when the icons won't all fit on the screen.
	uint8 m_nScrollCycles;    // Count of how many game cycles we have been scrolling
	uint8 m_nLastIconIndex;   // the last selected icon index used for scrolling
	uint8 m_nScrollDirection; // which direction we are scrolling

	uint8 m_nAddedMedipacks;  // Gets set to cause flashing of added medipacks.
	uint8 m_nAddedClips;      // Gets set to cause flashing of added clips.
	uint8 m_nAddedSymbol;     // Indicates which 'adding' icon to display.
	uint8 m_nAddedFlashCount; // Used to control the flashing of added medipacs and clips.

	bool8 m_bEmailArrived;      // Used to flag the arrival of an email.
	uint8 m_nMaxIconsDisplayed; // Holds current maximum icons that can be displayed.
	uint8 m_bPad2;
	uint8 m_bPad3;

	// Here I block the use of the default '='.
	_icon_menu(const _icon_menu &) {}

	void operator=(const _icon_menu &) {}

	// Private functions used only in this class.
	void CloseDownIconMenuDisplay();

	void SetUpOffScreenArrows();
	void SetupAdding(const char *pcIconName, uint32 &nSurfaceID);
	void CloseDownAdding();
};

extern _icon_menu *g_oIconMenu;
}

#endif // #if !defined( ICONMENU_H_INCLUDED )
