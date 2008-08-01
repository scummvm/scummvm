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
* $URL$
* $Id$
*
*/

#ifndef COMMON_ACTION
#define COMMON_ACTION

#include "common/events.h"
#include "common/func.h"
#include "common/list.h"
#include "common/str.h"

namespace Common {

struct HardwareKey;
class Keymap;


enum ActionType {
	kGenericActionType,

	// common actions
	kDirectionUpAction,
	kDirectionDownAction,
	kDirectionLeftAction,
	kDirectionRightAction,
	kLeftClickAction,
	kRightClickAction,
	kSaveAction,
	kMenuAction,
	kVirtualKeyboardAction,
	kRemapKeysAction,

	kActionTypeMax
};

enum ActionCategory {
	kGenericActionCategory,
	// classes of action - probably need to be slightly more specific than this
	kInGameAction,   // effects the actual gameplay
	kSystemAction,   //show a menu / change volume / etc

	kActionCategoryMax
};

struct Action {
	/** unique id used for saving/loading to config */
	int32 id;
	/** Human readable description */
	String description;
	/** Events to be sent when mapped key is pressed */
	List<Event> events;

	ActionCategory category;
	ActionType type;
	int priority;
	int group;
	int flags;

private:
	/** Hardware key that is mapped to this Action */
	const HardwareKey *_hwKey;
	Keymap *_parent;

public:
	Action(	String des = "", 
		ActionCategory cat = kGenericActionCategory,
		ActionType ty = kGenericActionType,
		int pr = 0, int gr = 0, int fl = 0 );

	void setParent(Keymap *parent);

	void mapKey(const HardwareKey *key);

	const HardwareKey *getMappedKey() const;
};

struct ActionPriorityComp : public BinaryFunction<Action, Action, bool> {
	bool operator()(const Action *x, const Action *y) const { 
		return x->priority > y->priority;
	}
	bool operator()(const Action &x, const Action &y) const { 
		return x.priority > y.priority;
	}
};

} // end of namespace Common

#endif
