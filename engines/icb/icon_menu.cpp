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

#include "engines/icb/icon_menu.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/sound.h"
#include "engines/icb/res_man.h"
#include "engines/icb/remora.h"
#include "engines/icb/mission.h"

namespace ICB {

// Use globals as it reduces rdata storage on PSX
const char *global_nothing_selected = "NOTHING_SELECTED";

_icon_menu::_icon_menu() {
	m_eIconMenuGameState = INACTIVE;
	m_bValidSelection = FALSE8;
	m_nKeyLock = FALSE8;
	m_nHighlightCounter = 0;
	m_bHighlightVisible = FALSE8;
	m_bAllowEscape = TRUE8;
	m_bWiderThanScreen = FALSE8;
	m_nAddedMedipacks = 0;
	m_nAddedClips = 0;
	m_nAddedSymbol = 0;
	m_nAddedFlashCount = 0;

	strcpy(m_pcGlobalClusterFile, GLOBAL_CLUSTER_PATH);
	strcpy(m_pcIconCluster, ICON_CLUSTER_PATH);

	m_nGlobalClusterHash = NULL_HASH;
	m_nIconClusterHash = NULL_HASH;
	m_nSelectedIconHash = NULL_HASH;
	m_pcSelectedIconName = global_nothing_selected;
}

bool8 _icon_menu::CycleIconMenu(const _input &sKeyboardState) {
	bool8 nRetVal = TRUE8;

	static int lastInventoryPress = 0;
	int inventoryPress;

	// Cycle the transparency for the highlight, to pulse the icon on screen
	++m_nHighlightCounter;

	if (m_nHighlightCounter == ICON_MENU_HIGHLIGHT_SPEED) {
		m_nHighlightCounter = 0;
		m_bHighlightVisible = (bool8)!m_bHighlightVisible;
	}

	inventoryPress = sKeyboardState.IsButtonSet(__INVENTORY);

	// FIND GOBACK if there is one
	// found is -1 means none found
	int found = -1;
	int i;

	// loop through all the icons or until we find a goback
	i = 0;
	while ((i < m_pIconList->GetIconCount()) && (found == -1)) {
		// get the icon
		const uint32 hash = m_pIconList->GetIconHash(i);

		// look for goback or return
		if ((hash == HashString("return")) || (hash == HashString("goback")))
			found = i;

		i++;
	}

	// if there is a goback option and we are gholding down inventory key and we are not ucrrently on goback then scroll round until we
	// get there...
	// also only if we are not scrolling
	if ((found != -1) && (inventoryPress) && (m_nSelectedIcon != (uint)found) && (m_nScrollDirection == ICON_MENU_SCROLL_NONE)) {
		m_nSelectedIcon = found;
		m_pcSelectedIconName = const_cast<char *>(m_pIconList->GetIcon(m_nSelectedIcon));
		m_nSelectedIconHash = m_pIconList->GetIconHash(m_nSelectedIcon);
	}

	// See what keys are being pressed.

	// INVENTORY QUIT: we must not be in the remora, m_bAllowEscape must be true
	// key not locked, we are pressing inventory and we wern't last time...
	if ((!g_oRemora->IsActive()) && (m_bAllowEscape) && (!m_nKeyLock) && (inventoryPress) && (!lastInventoryPress)) {
		CloseDownIconMenu();

		// Return the player's state to what it was before the menu was activated.
		MS->player.Pop_control_mode();
		MS->player.Set_player_status(STOOD);

		// Lock the keypress so it can't be repeated.
		m_nKeyLock = TRUE8;

		// Tell the calling function this function doesn't need calling any more.
		nRetVal = FALSE8;

	}
	// REMORA QUIT: remora is active we just let go of inventory button, key not locked we have a return...
	else if ((g_oRemora->IsActive()) && (!m_nKeyLock) && (!inventoryPress) && (lastInventoryPress) && (found != -1)) {
		m_nLastSelection = found;
		m_bValidSelection = TRUE8;

		// Close down the menu.
		CloseDownIconMenu();

		// Lock the keypress so it can't be repeated.
		m_nKeyLock = TRUE8;

		// Tell the calling function this function doesn't need calling any more.
		nRetVal = FALSE8;

		lastInventoryPress = 0;
	}
	// CONVESATION QUIT: remora is not activem m_bAllowEscape is probabily true
	// no key lock, inventory was pressed and has now been released...
	// and we have a quit!
	else if ((!g_oRemora->IsActive()) && (!m_bAllowEscape) && (!m_nKeyLock) && (!inventoryPress) && (lastInventoryPress) && (found != -1)) {
		m_nLastSelection = found;
		m_bValidSelection = TRUE8;

		// Close down the menu.
		CloseDownIconMenu();

		// Lock the keypress so it can't be repeated.
		m_nKeyLock = TRUE8;

		// Tell the calling function this function doesn't need calling any more.
		nRetVal = FALSE8;

		lastInventoryPress = 0;
	} else if (!m_nKeyLock && sKeyboardState.IsButtonSet(__INTERACT)) {
		// Player is selecting the current icon.  Don't select it if it is the 'empty' icon
		if (m_pIconList->GetIconHash(m_nSelectedIcon) != HashString(ICON_LIST_EMPTY_ICON)) {
			m_nLastSelection = m_nSelectedIcon;
			m_bValidSelection = TRUE8;
		}

		if (!g_oRemora->IsActive()) {
			// Return the player's state to what it was before the menu was activated.
			MS->player.Pop_control_mode();
			MS->player.Set_player_status(STOOD);
		}
		// Close down the menu.
		CloseDownIconMenu();

		// Lock the keypress so it can't be repeated.
		m_nKeyLock = TRUE8;

		// Tell the calling function this function doesn't need calling any more.
		nRetVal = FALSE8;
	} else if (!m_nKeyLock && !sKeyboardState.IsButtonSet(__SIDESTEP) && (sKeyboardState.turn == __LEFT)) {
		// Move current selection left : if we are not scrolling & more than item in the list
		if ((m_nScrollDirection == ICON_MENU_SCROLL_NONE) && (m_pIconList->GetIconCount() > 1)) {

			if (m_nSelectedIcon == 0)
				m_nSelectedIcon = m_pIconList->GetIconCount() - 1;
			else
				--m_nSelectedIcon;

			// Set the name & hash value of the currently selected icon.
			// Set_string( m_pIconList->GetIcon( m_nSelectedIcon ), m_pcSelectedIconName, MAXLEN_ICON_NAME );
			m_pcSelectedIconName = const_cast<char *>(m_pIconList->GetIcon(m_nSelectedIcon));
			m_nSelectedIconHash = m_pIconList->GetIconHash(m_nSelectedIcon);

			// Lock the keypress so it can't be repeated.
			m_nKeyLock = TRUE8;

			// Tell the calling function this function does need calling again next cycle.
			nRetVal = TRUE8;

			// Hey hey we are scrolling to the right even though pressed LEFT
			m_nScrollDirection = ICON_MENU_SCROLL_RIGHT;
		}
	} else if (!m_nKeyLock && !sKeyboardState.IsButtonSet(__SIDESTEP) && sKeyboardState.turn == __RIGHT) {
		// Move current selection right : if we are not scrolling & more than item in the list
		if ((m_nScrollDirection == ICON_MENU_SCROLL_NONE) && (m_pIconList->GetIconCount() > 1)) {
			if (m_nSelectedIcon == (uint32)(m_pIconList->GetIconCount() - 1))
				m_nSelectedIcon = 0;
			else
				++m_nSelectedIcon;

			// Set the name & hash value of the currently selected icon.
			// Set_string( m_pIconList->GetIcon( m_nSelectedIcon ), m_pcSelectedIconName, MAXLEN_ICON_NAME );
			m_pcSelectedIconName = const_cast<char *>(m_pIconList->GetIcon(m_nSelectedIcon));
			m_nSelectedIconHash = m_pIconList->GetIconHash(m_nSelectedIcon);

			// Lock the keypress so it can't be repeated.
			m_nKeyLock = TRUE8;

			// Tell the calling function this function does need calling again next cycle.
			nRetVal = TRUE8;

			// Hey hey we are scrolling to the left even though pressed RIGHT
			m_nScrollDirection = ICON_MENU_SCROLL_LEFT;
		}
	}

	// Release the keylock if it is on and none of the keys that caused it to be set are still being pressed.
	if (m_nKeyLock && !sKeyboardState.IsButtonSet(__INVENTORY) && !sKeyboardState.IsButtonSet(__INTERACT)) {
		m_nKeyLock = FALSE8;
	}

	// update last press
	// only update if we are coming back, so as not to do the
	// muck up first time you do remora after inventory
	if (nRetVal) {
		lastInventoryPress = inventoryPress;
	}

	// Return a value to indicate if this function should be called again next cycle.
	return (nRetVal);
}

void _icon_menu::CycleHoldingLogic() {
	// Check if there is a current interact object.
	if (!MS->player.Fetch_player_interact_status()) {
		// No interact object, so drop whatever we're holding.
		ClearSelection();
	}
}

void _icon_menu::CycleAddingLogic() {
	// Increment the flash counter.  If not a state toggle point, simply return.
	if (m_nAddedFlashCount++ < ICON_MENU_ADDED_FLASHRATE)
		return;

	// Right, we are toggling the state of the flashing icons.  First reset the counter.
	m_nAddedFlashCount = 0;

	// Behaviour now depends on whether the icon is currently being displayed ot not.
	if (m_nAddedSymbol == 0) {
		// Symbol is currently off so we are turning it on.  We need to know whether to
		// flash a medipack symbol or an ammo clip symbol.
		if (m_nAddedMedipacks > 0) {
			// Turning on a medipack symbol.
			m_nAddedSymbol = 1;

			// Play a sound to go with it.
			RegisterSoundSpecial(defaultAddingMediSfx, addingMediDesc, 127, 0);
		} else if (m_nAddedClips > 0) {
			// Turning on a clips symbol.
			m_nAddedSymbol = 2;

			// Play a sound to go with it.
			RegisterSoundSpecial(defaultAddingClipSfx, addingClipDesc, 127, 0);
		} else if (m_bEmailArrived) {
			// Turning on an email-arrived symbol.
			m_nAddedSymbol = 3;

			// Play a sound to go with it.
			RegisterSoundSpecial(defaultEmailSfx, emailDesc, 127, 0);
		}
	} else {
		// See which symbol is active.
		switch (m_nAddedSymbol) {
		case 1:
			// Medipack symbol is currently being displayed.  Turn it off.
			--m_nAddedMedipacks;
			m_nAddedSymbol = 0;
			break;

		case 2:
			// Clips symbol is currently being displayed.  Turn it off.
			--m_nAddedClips;
			m_nAddedSymbol = 0;
			break;

		default:
			// This is a funny one.  First time in here will be because the symbol is 3, which
			// means an email-waiting symbol is being displayed.  To turn it off, we don't set
			// it to zero, however; instead we increment it, and then we keep incrementing it
			// until we hit the count that controls how long the symbol should be off before it
			// is flashed again.
			if (++m_nAddedSymbol == ICON_MENU_EMAIL_FLASHRATE)
				m_nAddedSymbol = 0;
		}
	}
}

void _icon_menu::PreloadIcon(const char *pcIconPath, const char *pcIconName) {
	uint32 nFullIconNameHash;

	// Make the full URL for the icon.
	char pcFullIconName[MAXLEN_URL];
	sprintf(pcFullIconName, "%s%s.%s", pcIconPath, pcIconName, PX_BITMAP_EXT);

#if defined(_PSX)
	const char *pcFullIconName = pcIconName;
#endif // #if defined(_PSX)

	// Open the icon resource.
	nFullIconNameHash = NULL_HASH;
	rs_icons->Res_async_open(pcFullIconName, nFullIconNameHash, m_pcIconCluster, m_nIconClusterHash);
}

const char *_icon_menu::GetLastSelection() {
	// Only return a selection if one has been made.
	if (m_bValidSelection) {
		if (m_pIconList->GetIconCount() > 0)
			return (m_pIconList->GetIcon(m_nLastSelection));
		else
			return (NULL);
	} else {
		return (NULL);
	}
}

uint32 _icon_menu::GetLastSelectionHash() const {
	// Only return a selection if one has been made.
	if (m_bValidSelection) {
		if (m_pIconList->GetIconCount() > 0)
			return (m_pIconList->GetIconHash(m_nLastSelection));
		else
			return (NULL_HASH);
	} else {
		return (NULL_HASH);
	}
}

void _icon_menu::CloseDownIconMenu() {
	// The Remora has to call this function when it quits, to make sure the icon menu disappears with it, but
	// it is up to the script writer to make sure that the Remora has an icon menu displayed; therefore, there is
	// the possibility that the Remora will quit and try to call this function when no icon menu is being
	// displayed.  We have to check for this here.
	if (m_eIconMenuGameState == INACTIVE)
		return;

	// Menu is active, so close it down.
	CloseDownIconMenuDisplay();
	m_eIconMenuGameState = INACTIVE;
}

bool8 _icon_menu::IsAdding() const {
	if ((m_nAddedMedipacks > 0) || (m_nAddedClips > 0) || m_bEmailArrived)
		return (TRUE8);
	else
		return (FALSE8);
}

#define ICON_MENU_SCROLLCYCLES_INCREMENT (ICON_X_SIZE / 4)
#define ICON_MENU_SCROLLCYCLES_MAX ICON_X_SIZE

// Scroll the icons smoothly left or right
// This returns the x-position to start drawing the icons from (nX)
// it also sets the first icon to start drawing (nIconIndex)
int _icon_menu::GetScrollingPosition(const int nInputX, uint32 &nIconIndex) {
	int nX = nInputX;

	// OK are we scrolling
	if (m_nScrollDirection != ICON_MENU_SCROLL_NONE) {
		if (m_nScrollCycles >= ICON_MENU_SCROLLCYCLES_MAX) {
			m_nScrollCycles = 0;
			m_nScrollDirection = ICON_MENU_SCROLL_NONE;
			m_nLastIconIndex = (uint8)nIconIndex;
		} else {
			if (m_nScrollDirection == ICON_MENU_SCROLL_RIGHT) {
				// scroll right
				nX += m_nScrollCycles;
				nX -= ICON_X_SIZE;
			} else {
				// scroll left : keep old icon index
				nX -= m_nScrollCycles;
				nIconIndex = m_nLastIconIndex;
			}

			m_nScrollCycles += ICON_MENU_SCROLLCYCLES_INCREMENT;
		}
	}

	return nX;
}

} // End of namespace ICB
