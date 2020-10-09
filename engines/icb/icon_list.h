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

#ifndef ICB_ICON_LIST_H_INCLUDED
#define ICB_ICON_LIST_H_INCLUDED

// Include headers needed by this class.
#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/common/px_string.h"
#include "engines/icb/common/px_assert.h"
#include "engines/icb/common/px_clu_api.h"
#include "engines/icb/p4.h"
#include "engines/icb/debug.h"
#include "engines/icb/string_vest.h"

namespace ICB {

extern const char *global_deleted_list;

// This is used as a placeholder for deleted lists to save reallocating.
#define ICON_LIST_DELETED_PLACEHOLDER global_deleted_list

// This defines the available values for the scope of these lists.
enum IconListScope { CURRENT_LOGIC = 0, SESSION_WIDE, MISSION_WIDE, GAME_WIDE };

// This defines the maximum number of icons that can be in the menu.
#define ICON_LIST_MAX_ICONS 16 // This defines an array size, so must be a multiple of 4!

// And this defines the maximum number that can be displayed at one time.
#define ICON_LIST_MAX_DISPLAYED 10

// This defines how many of each icon are allowed (only meaningful when the list is handling duplicates).
#define ICON_MAX_DUPLICATE_COUNT 99

// This means there is no current selection.
#define ICON_LIST_NO_SELECTION (-1)

// This defines the name of the 'empty' icon.

extern const char *iconListEmptyIcon;
#define ICON_LIST_EMPTY_ICON iconListEmptyIcon

// Class to manage the lists of icons used in the game.  the inventory, Remora and speech system all use these lists.
class _icon_list {
public:
	// Default constructor and destructor.
	_icon_list();
	~_icon_list() {}

	// Alternative constructor, which takes the list name.
	inline _icon_list(const char *pcListName);

	// Copy constructor and assignment operator.
	inline _icon_list(const _icon_list &oX);
	inline const _icon_list &operator=(const _icon_list &oOpB);

	// This resets the list to its newly-created state.
	void Reset();

	// Gets and sets.
	const char *GetListName() const { return (m_pcListName); }
	void SetListName(const char *pcListName) { m_pcListName = pcListName; }

	uint8 GetIconCount() const { return (m_nItemCount); }

	IconListScope GetScope() const { return (m_eScope); }
	void SetScope(IconListScope eScope) { m_eScope = eScope; }

	inline const char *GetIcon(uint32 nIndex) const;
	inline uint32 GetIconHash(uint32 nIndex) const;
	uint8 GetDuplicateCount(const char *pcIconName) const;
	uint8 GetDuplicateCount(uint32 nIndex) const;

	void SetAbsoluteIconCount(const char *pcIconName, uint32 nCount);

	// This returns the index position of an icon in the list.
	int32 GetIconPosition(const char *pcIconName) const;

	// These add and remove items from the list.
	void AddIcon(const char *pcIconName, const uint32 nIconNameHash);
	void RemoveIcon(const char *pcIconName, bool8 bForceRemove);

private:
	const char *m_pcListName;                                  // The name of the list.
	IconListScope m_eScope;                                    // Scope of the list (determines when it is allowed to be destroyed).
	uint32 m_pnIconListHash[ICON_LIST_MAX_ICONS];              // The hash number of items currently in the inventory.
	uint8 m_pnDuplicateCount[ICON_LIST_MAX_ICONS];             // The list of items currently in the inventory.
	char m_ppcIconList[ICON_LIST_MAX_ICONS][MAXLEN_ICON_NAME]; // The list of items currently in the inventory.
	uint8 m_nItemCount;                                        // My own count of the number of items currently in the inventory.
	bool8 m_bAllowDuplicates;                                  // If true, a count is incremented when a duplicate is added.
	uint8 m_nPad1;
	uint8 m_nPad2;

	// Private functions used only in this class.
	void Clone(const _icon_list &oSource);
};

inline _icon_list::_icon_list(const _icon_list &oX) { Clone(oX); }

inline const _icon_list &_icon_list::operator=(const _icon_list &oOpB) {
	Clone(oOpB);

	return (*this);
}

inline void _icon_list::Reset() {
	memset((uint8 *)m_pnIconListHash, 0, ICON_LIST_MAX_ICONS * sizeof(uint32));
	memset((uint8 *)m_pnDuplicateCount, 0, ICON_LIST_MAX_ICONS * sizeof(uint8));
	memset((uint8 *)m_ppcIconList, 0, ICON_LIST_MAX_ICONS * MAXLEN_ICON_NAME * sizeof(char));
	m_nItemCount = 0;
}

inline const char *_icon_list::GetIcon(uint32 nIndex) const { return (m_ppcIconList[nIndex]); }

inline uint32 _icon_list::GetIconHash(uint32 nIndex) const {
	if (nIndex >= m_nItemCount) {
		Fatal_error("_icon_list::GetIconHash( %d ) called - list has %d items", nIndex, m_nItemCount);
	}

	return (m_pnIconListHash[nIndex]);
}

} // End of namespace ICB

#endif // #if !defined( ICON_LIST_H_INCLUDED )
