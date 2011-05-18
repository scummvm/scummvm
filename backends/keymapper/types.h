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

#ifndef KEYMAPPER_TYPES_H
#define KEYMAPPER_TYPES_H

#include "common/scummsys.h"

#ifdef ENABLE_KEYMAPPER

namespace Common {

enum KeyType {
	kGenericKeyType,
	kDirUpKeyType,
	kDirDownKeyType,
	kDirLeftKeyType,
	kDirRightKeyType,
	kActionKeyType,
	kTriggerLeftKeyType,
	kTriggerRightKeyType,
	kStartKeyType,
	kSelectKeyType,
	/*  ...  */

	kKeyTypeMax
};

enum ActionType {
	kGenericActionType,

	// common actions
	kDirUpActionType,
	kDirDownActionType,
	kDirLeftActionType,
	kDirRightActionType,
	kLeftClickActionType,
	kRightClickActionType,
	kSaveActionType,
	kMenuActionType,
	kQuitActionType,
	kVirtualKeyboardActionType,
	kKeyRemapActionType,
	kVolumeUpActionType,
	kVolumeDownActionType,


	kActionTypeMax
};

} // End of namespace Common

#endif // #ifdef ENABLE_KEYMAPPER

#endif // #ifndef KEYMAPPER_TYPES_H
