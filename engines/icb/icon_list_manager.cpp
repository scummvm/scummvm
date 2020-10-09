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

#include "engines/icb/icon_list_manager.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/mission.h"

namespace ICB {

const char *global_icon_list_remora = "icon_list_remora";
const char *global_icon_list_inventory = "icon_list_inventory";
const char *global_icon_list_arms = "icon_list_arms";

_icon_list_manager::_icon_list_manager() {
	// The icon lists for the inventory and Remora would get created automatically, but I do it here just so
	// I know they always get the same slots, which will be useful in debugging.
	m_pListOfLists[0].SetListName(ICON_LIST_INVENTORY);
	m_pListOfLists[0].SetScope(GAME_WIDE);

	m_pListOfLists[1].SetListName(ICON_LIST_REMORA);
	m_pListOfLists[1].SetScope(SESSION_WIDE);
}

void _icon_list_manager::ActivateIconMenu(const char *pcListName, bool8 bAllowEscape, bool8 bDrawStatusIcons) {
	uint32 i;
	int32 nListIndex;
	_icon_menu_duplicates sDuplicates;
	uint32 nHash;
	int32 nInitialSelected;
	uint32 nNumClips, nNumMedi;

	nListIndex = FindListByName(pcListName);

	// The player object now 'owns' the count of medipacks and clips.  To find out how many to draw,
	// we need to get these counts here and add them to the inventory.  Not in the Remora though.
	if (bDrawStatusIcons) {
		// Get number of clips and medipacks from the player object.
		nNumClips = MS->player.GetNoAmmoClips();
		nNumMedi = MS->player.GetNoMediPacks();

		// Set this number in the icon menu.
		m_pListOfLists[nListIndex].SetAbsoluteIconCount(ARMS_AMMO_NAME, nNumClips);
		m_pListOfLists[nListIndex].SetAbsoluteIconCount(ARMS_HEALTH_NAME, nNumMedi);

		// Find the medipacks position.
		nInitialSelected = m_pListOfLists[nListIndex].GetIconPosition(ARMS_HEALTH_NAME);

		// If we didn't find it, just set the 0 icon to be displayed.
		if (nInitialSelected == -1)
			nInitialSelected = 0;
	} else {
		// Just select first icon in the Remora.
		nInitialSelected = 0;

		// No clips or medipacks in the Remora.
		m_pListOfLists[nListIndex].RemoveIcon(ARMS_AMMO_NAME, TRUE8);
		m_pListOfLists[nListIndex].RemoveIcon(ARMS_HEALTH_NAME, TRUE8);
	}

	// We've found the list.  If there are no icons in it, add the 'empty' one.
	if (m_pListOfLists[nListIndex].GetIconCount() == 0) {
		nHash = HashString(ICON_LIST_EMPTY_ICON);
		m_pListOfLists[nListIndex].AddIcon(ICON_LIST_EMPTY_ICON, nHash);
	}

	// Build the information structure for the list.
	sDuplicates.s_nNumItems = m_pListOfLists[nListIndex].GetIconCount();
	for (i = 0; i < sDuplicates.s_nNumItems; ++i)
		sDuplicates.s_pnItemCounts[i] = m_pListOfLists[nListIndex].GetDuplicateCount(i);

	// Activate the menu.
	g_oIconMenu->Activate(&m_pListOfLists[nListIndex], sDuplicates, bAllowEscape, nInitialSelected);
}

void _icon_list_manager::ResetToScopeLevel(IconListScope eScope) {
	uint32 i;

	// Go through the lists, removing them if they are out of scope.
	for (i = 0; i < ICON_LIST_MANAGER_MAX_LISTS; ++i) {
		// Watch out for deleted ones.
		if (strcmp(m_pListOfLists[i].GetListName(), ICON_LIST_DELETED_PLACEHOLDER) && (m_pListOfLists[i].GetScope() <= eScope)) {
			// This list needs killing.
			m_pListOfLists[i].SetListName(ICON_LIST_DELETED_PLACEHOLDER);
		}
	}
}

void _icon_list_manager::AddIconToList(const char *pcListName, const char *pcIconName, const uint32 nHash) {
	uint32 i;
	int32 nListIndex;

	// See if the list already exists.
	nListIndex = FindListByName(pcListName);

	// If it doesn't, we need to create a new one, reusing an empty slot if there is one.
	if (nListIndex == ICON_LIST_NOT_FOUND) {
		// Look for the next available slot.
		i = 0;
		while ((i < ICON_LIST_MANAGER_MAX_LISTS) && strcmp(m_pListOfLists[i].GetListName(), ICON_LIST_DELETED_PLACEHOLDER) &&
		       (strlen(m_pListOfLists[i].GetListName()) > 0)) {
			++i;
		}

		// Check if we have run out of list slots.
		if (i == ICON_LIST_MANAGER_MAX_LISTS)
			Fatal_error("Run out of list slots adding an icon to list %s", pcListName);

		// Whichever one of the other two loop conditions failed, we are now pointing at the slot for the new list.  Because
		// it is a new list, we must set its name.
		m_pListOfLists[i].Reset();
		m_pListOfLists[i].SetListName(pcListName);
		m_pListOfLists[i].AddIcon(pcIconName, nHash);
	} else {
		// List was found, so just add the icon to it.
		m_pListOfLists[nListIndex].AddIcon(pcIconName, nHash);
	}
}

uint32 _icon_list_manager::GetList(const char *pcListName, char *pNames[ICON_LIST_MAX_ICONS], uint32 *pnHashes, uint32 *pnCounts) const {
	uint32 i;
	int32 nListIndex;
	uint32 nNumIconsInList;

	// See if the list already exists.
	nListIndex = FindListByName(pcListName);

	// If we failed to find the list, we'll treat this as an error.
	if (nListIndex == ICON_LIST_NOT_FOUND)
		Fatal_error("List %s not found in _icon_list_manager::GetList()", pcListName);

	// Get the number of icons in the list.
	nNumIconsInList = m_pListOfLists[nListIndex].GetIconCount();

	// If there are none, then there is nothing to do.
	if (nNumIconsInList == 0)
		return (0);

	// Loop for each icon in the list.
	for (i = 0; i < nNumIconsInList; ++i) {
		pNames[i] = const_cast<char *>(m_pListOfLists[nListIndex].GetIcon(i));
		pnHashes[i] = m_pListOfLists[nListIndex].GetIconHash(i);
		pnCounts[i] = m_pListOfLists[nListIndex].GetDuplicateCount(i);
	}

	// And return the count.
	return (nNumIconsInList);
}

uint32 _icon_list_manager::GetItemCount(const char *pcListName) const {
	int32 nListIndex;

	nListIndex = FindListByName(pcListName);

	if (nListIndex == ICON_LIST_NOT_FOUND)
		Fatal_error("_icon_list_manager::GetItemCount(): list %s not found.", pcListName);

	return (m_pListOfLists[nListIndex].GetIconCount());
}

bool8 _icon_list_manager::Carrying(const char *pcListName, const char *pcItem) const {
	int32 nListIndex;

	nListIndex = FindListByName(pcListName);

	if (nListIndex == ICON_LIST_NOT_FOUND)
		Fatal_error("_icon_list_manager::Carrying(): list %s not found.", pcListName);

	if (m_pListOfLists[nListIndex].GetDuplicateCount(pcItem) > 0)
		return (TRUE8);
	else
		return (FALSE8);
}

uint32 _icon_list_manager::CarryingHowMany(const char *pcListName, const char *pcItem) const {
	int32 nListIndex;

	nListIndex = FindListByName(pcListName);

	if (nListIndex == ICON_LIST_NOT_FOUND)
		Fatal_error("_icon_list_manager::CarryingHowMany(): list %s not found.", pcListName);

	return (m_pListOfLists[nListIndex].GetDuplicateCount(pcItem));
}

bool8 _icon_list_manager::Holding(const char *pcItem) const {
	uint32 nHoldingHash, nHash;

	// Get the last selection.
	nHoldingHash = g_oIconMenu->GetLastSelectionHash();

	// Compute hash of item we're looking for.
	nHash = HashString(pcItem);

	// It is valid and in range, so check if the name of that list item matches the one we're checking.
	if (nHash != nHoldingHash)
		return (FALSE8);
	else
		return (TRUE8);
}

void _icon_list_manager::RemoveIconFromList(const char *pcListName, const char *pcIconName) {
	int32 nListIndex;

	nListIndex = FindListByName(pcListName);

	if (nListIndex == ICON_LIST_NOT_FOUND)
		Fatal_error("_icon_list_manager::RemoveIconFromList(): list %s not found.", pcListName);

	m_pListOfLists[nListIndex].RemoveIcon(pcIconName, FALSE8);
}

void _icon_list_manager::SetListScope(const char *pcListName, IconListScope eScope) {
	int32 nListIndex;

	nListIndex = FindListByName(pcListName);

	if (nListIndex == ICON_LIST_NOT_FOUND)
		Fatal_error("_icon_list_manager::SetScope(): list %s not found.", pcListName);

	m_pListOfLists[nListIndex].SetScope(eScope);
}

void _icon_list_manager::DestroyList(const char *pcListName) {
	int32 nListIndex;

	nListIndex = FindListByName(pcListName);

	if (nListIndex == ICON_LIST_NOT_FOUND)
		Fatal_error("_icon_list_manager::DestroyList(): list %s not found.", pcListName);

	m_pListOfLists[nListIndex].SetListName(ICON_LIST_DELETED_PLACEHOLDER);
}

void _icon_list_manager::ResetList(const char *pcListName) {
	int32 nListIndex;

	nListIndex = FindListByName(pcListName);

	if (nListIndex == ICON_LIST_NOT_FOUND)
		return;

	m_pListOfLists[nListIndex].Reset();
}

void _icon_list_manager::AddIconToList(const char *pcListName, const char *pcIconName) {
	const uint32 nHash = HashString(pcIconName);

	AddIconToList(pcListName, pcIconName, nHash);
}

int32 _icon_list_manager::FindListByName(const char *pcListName) const {
	uint32 i;

	// Look for the list.
	i = 0;
	while ((i < ICON_LIST_MANAGER_MAX_LISTS) && strcmp(m_pListOfLists[i].GetListName(), pcListName))
		++i;

	// See which loop condition failed.
	if (i < ICON_LIST_MANAGER_MAX_LISTS)
		return (i);
	else
		return (ICON_LIST_NOT_FOUND);
}

} // End of namespace ICB
