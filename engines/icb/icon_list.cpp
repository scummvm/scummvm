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

#include "engines/icb/p4.h"
#include "engines/icb/icon_list.h"
#include "engines/icb/debug.h"

namespace ICB {

// Don't use #defines for common strings as it wastes rdata storage (on PSX at least)
const char *global_deleted_list = "DELETED_LIST";
const char *iconListEmptyIcon = "empty";

_icon_list::_icon_list() {
	m_pcListName = const_cast<char *>(ICON_LIST_DELETED_PLACEHOLDER);
	m_eScope = CURRENT_LOGIC;
	m_nItemCount = 0;
	m_bAllowDuplicates = TRUE8;
	m_nPad1 = 0;
	m_nPad2 = 0;

	// Note, a hash of 0 means unknown HASH value i.e. NULL_HASH = 0.
	_ASSERT(NULL_HASH == 0);
	memset((uint8 *)m_pnIconListHash, 0, ICON_LIST_MAX_ICONS * sizeof(uint32));
	memset((uint8 *)m_pnDuplicateCount, 0, ICON_LIST_MAX_ICONS * sizeof(uint8));
	memset((uint8 *)m_ppcIconList, 0, ICON_LIST_MAX_ICONS * MAXLEN_ICON_NAME * sizeof(char));
}

void _icon_list::Clone(const _icon_list &oSource) {
	uint32 i;

	m_pcListName = oSource.m_pcListName;

	m_eScope = oSource.m_eScope;
	m_nItemCount = oSource.m_nItemCount;
	m_bAllowDuplicates = oSource.m_bAllowDuplicates;

	for (i = 0; i < m_nItemCount; ++i) {
		Set_string(const_cast<char *>(oSource.m_ppcIconList[i]), m_ppcIconList[i], MAXLEN_ICON_NAME);
		m_pnIconListHash[i] = oSource.m_pnIconListHash[i];
		m_pnDuplicateCount[i] = oSource.m_pnDuplicateCount[i];
	}
}

uint8 _icon_list::GetDuplicateCount(const char *pcIconName) const {
	uint32 i = 0;

	if (strlen(pcIconName) == 0)
		Fatal_error("Empty icon name passed into _icon_list::GetDuplicateCount()");

	uint32 nIconNameHash = HashString(pcIconName);

	for (i = 0; i < m_nItemCount; ++i) {
		if (m_pnIconListHash[i] == nIconNameHash) {
			if (m_bAllowDuplicates)
				return (m_pnDuplicateCount[i]);
			else
				return (1);
		}
	}

	// We fell off the end of the list, which means the icon is not in the list.
	return (0);
}

void _icon_list::SetAbsoluteIconCount(const char *pcIconName, uint32 nCount) {
	uint32 i;
	uint32 nHash;

	// If the list does not allow duplicates, this function is disallowed.
	if (!m_bAllowDuplicates)
		Fatal_error("SetAbsoluteIconCount() called for list %s which doesn't allow duplicates", m_pcListName);

	// Also, count must be in range.
	if (nCount > ICON_MAX_DUPLICATE_COUNT)
		Fatal_error("Attempt to set %d of icon %s in list %s (maximum=%d)", nCount, pcIconName, m_pcListName, ICON_MAX_DUPLICATE_COUNT);

	// Work out the hash of the icon name.
	nHash = HashString(pcIconName);

	// Check if it is in the list already.
	i = 0;
	while ((i < m_nItemCount) && (m_pnIconListHash[i] != nHash))
		++i;

	// Did we find it?
	if (i == m_nItemCount) {
		// No, it wasn't in the list.  Can't add a new one if the list is full.
		if (m_nItemCount == ICON_LIST_MAX_ICONS)
			Fatal_error("_icon_list::SetAbsoluteIconCount(): Icon list %s full adding %s", m_pcListName, pcIconName);

// Add a fresh entry.
		Set_string(const_cast<char *>(pcIconName), m_ppcIconList[i], MAXLEN_ICON_NAME);

		m_pnIconListHash[i] = nHash;
		m_pnDuplicateCount[i] = (uint8)nCount;
		++m_nItemCount;
	} else {
		// Icon is already in list.
		m_pnDuplicateCount[i] = (uint8)nCount;
	}
}

void _icon_list::AddIcon(const char *pcIconName, const uint32 nIconNameHash) {
	uint32 i;

	// If the 'empty' icon is the only one in the list, remove it before adding the new one.
	if ((m_nItemCount == 1) && (strcmp(m_ppcIconList[0], ICON_LIST_EMPTY_ICON) == 0))
		--m_nItemCount;

	// Check if it is in the list already.
	i = 0;
	while ((i < m_nItemCount) && (m_pnIconListHash[i] != nIconNameHash))
		++i;

	// Did we find it?
	if (i == m_nItemCount) {
		// No, it wasn't in the list.
		if (m_nItemCount == ICON_LIST_MAX_ICONS)
			Fatal_error("_icon_list::AddItem(): Icon list %s full", m_pcListName);

// Add a fresh entry.
		Set_string(const_cast<char *>(pcIconName), m_ppcIconList[i], MAXLEN_ICON_NAME);
		m_pnIconListHash[i] = nIconNameHash;
		m_pnDuplicateCount[i] = 1;
		++m_nItemCount;
	} else {
		// Yes, it is already in the list, so behaviour depends on whether or not duplicates are allowed.  Nothing to do if
		// duplicates are not allowed.
		if (m_bAllowDuplicates) {
			// If the name of the icon is 'return' or 'goback' we don't add a duplicate.
			if (strcmp(pcIconName, "return") && strcmp(pcIconName, "goback")) {
				if (m_pnDuplicateCount[i] < ICON_MAX_DUPLICATE_COUNT)
					++m_pnDuplicateCount[i];
			}
		}
	}
}

void _icon_list::RemoveIcon(const char *pcIconName, bool8 bForceRemove) {
	uint32 i, j;
	uint32 nIconNameHash = HashString(pcIconName);

	// Find it in the list.
	i = 0;
	while ((i < m_nItemCount) && (m_pnIconListHash[i] != nIconNameHash))
		++i;

	// Don't do anything if it isn't in the list.  And don't do anything if the count is already at zero.
	if ((i < m_nItemCount) && (m_pnDuplicateCount[i] > 0)) {
		// Decrement the duplicate count.
		if (bForceRemove)
			m_pnDuplicateCount[i] = 0;
		else
			--m_pnDuplicateCount[i];

		// We can remove the item if the count hits zero.  This is the same whether or not duplicates are allowed.
		if (m_pnDuplicateCount[i] == 0) {
			// Might seem inefficient to remove holes from the array, rather than just mark them deleted, but it is
			// swings and roundabouts: other code ends up less efficient if I don't remove holes, and it's not like
			// we're dealing with thousands of array elements.
			for (j = i + 1; j < m_nItemCount; ++j) {
				strcpy(m_ppcIconList[j - 1], m_ppcIconList[j]);
				m_pnIconListHash[j - 1] = m_pnIconListHash[j];
				m_pnDuplicateCount[j - 1] = m_pnDuplicateCount[j];
			}

			// Don't actually need to delete the item; reducing the count takes care of it.
			--m_nItemCount;
		}
	}
}

uint8 _icon_list::GetDuplicateCount(uint32 nIndex) const {
	if (m_bAllowDuplicates)
		return (m_pnDuplicateCount[nIndex]);
	else
		return ((m_pnDuplicateCount[nIndex] == 0) ? (uint8)0 : (uint8)1);
}

int32 _icon_list::GetIconPosition(const char *pcIconName) const {
	uint32 i;
	uint32 nIconNameHash;

	// Get hash we are looking for.
	nIconNameHash = HashString(pcIconName);

	for (i = 0; i < m_nItemCount; ++i) {
		if (m_pnIconListHash[i] == nIconNameHash)
			return (i);
	}

	// Not found.
	return (-1);
}

} // End of namespace ICB
