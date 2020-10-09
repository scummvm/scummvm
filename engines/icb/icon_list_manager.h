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

#ifndef ICB_ICON_LIST_MANAGER_H_INCLUDED
#define ICB_ICON_LIST_MANAGER_H_INCLUDED

#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/common/px_string.h"
#include "engines/icb/string_vest.h"
#include "engines/icb/p4.h"
#include "engines/icb/icon_menu.h"
#include "engines/icb/icon_list.h"

namespace ICB {

#define ICON_LIST_MANAGER_LOG "icon_list_manager_log.txt"

extern const char *global_icon_list_remora;
extern const char *global_icon_list_inventory;
extern const char *global_icon_list_arms;

// These are predefined lists : make them global to reduce rdata usage
#define ICON_LIST_REMORA global_icon_list_remora
#define ICON_LIST_INVENTORY global_icon_list_inventory

// This indicates a list cannot be found.
#define ICON_LIST_NOT_FOUND (-1)

// The maximum number of lists allowed.
#define ICON_LIST_MANAGER_MAX_LISTS 8

// Class to manage the lists of icons used in the game.  the inventory, Remora and speech system all use these lists.
class _icon_list_manager {
public:
	// Default constructor and destructor.
	_icon_list_manager();
	~_icon_list_manager() {}

	// This resets lists that have a scope less than the argument passed in.
	void ResetToScopeLevel(IconListScope eScope);

	// Call this to activate the menu selection for a list of icons.  Note that the chooser does not have a
	// deactivate or cycle-logic function because once activated, all its functionality (including shutting down)
	// is handled by class _icon_menu.
	void ActivateIconMenu(const char *pcListName, bool8 bAllowEscape, bool8 bDrawStatusIcons);

	// This reports whether or not the inventory is active.
	bool8 IsActive() const { return (g_oIconMenu->IsActive()); }

	// This returns the contents of a list to the engine (for save and restore etc.).
	uint32 GetList(const char *pcListName, char *pNames[ICON_LIST_MAX_ICONS], uint32 *pnHashes, uint32 *pnCounts) const;

	// The same logic as the adding of ammo etc is borrowed for when an email arrives.
	void SetEmailArrived() { g_oIconMenu->SetEmailArrived(); }

	// Call this every cycle to cycle the inventory's logic when it is active.
	void CycleInventoryLogic(const _input &sKeyboardState) const { g_oIconMenu->CycleIconMenu(sKeyboardState); }

	// These cycle the logic for a currently-held item.
	bool8 IsHolding() const { return (g_oIconMenu->IsHolding()); }
	void CycleHoldingLogic() const { g_oIconMenu->CycleHoldingLogic(); }

	// This is for internal engine use only, as it returns a string, which cannot go back to script.
	inline const char *HoldingWhat() const;

	// These have direct script counterparts.
	void AddIconToList(const char *pcListName, const char *pcIconName);
	void AddIconToList(const char *pcListName, const char *pcIconName, const uint32 nHash);
	void RemoveIconFromList(const char *pcListName, const char *pcIconName);
	uint32 GetItemCount(const char *pcListName) const;
	bool8 Carrying(const char *pcListName, const char *pcItem) const;
	uint32 CarryingHowMany(const char *pcListName, const char *pcItem) const;
	bool8 ItemHeld() const { return (g_oIconMenu->IsHolding()); }
	bool8 Holding(const char *pcItem) const;
	void Drop() { g_oIconMenu->ClearSelection(); }
	void SetListScope(const char *pcListName, IconListScope eScope);
	void DestroyList(const char *pcListName);
	void ResetList(const char *pcListName);

private:
	_icon_list m_pListOfLists[ICON_LIST_MANAGER_MAX_LISTS]; // The list of icon lists.

	// Here I block the use of the default '='.
	_icon_list_manager(const _icon_list_manager &) {}

	void operator=(const _icon_list_manager &) {}

	// Private functions used only within this class.
	int32 FindListByName(const char *pcListName) const;
};

inline const char *_icon_list_manager::HoldingWhat() const { return (g_oIconMenu->GetLastSelection()); }

extern _icon_list_manager *g_oIconListManager;

} // End of namespace ICB

#endif // #if !defined( ICON_LIST_MANAGER_H_INCLUDED )
