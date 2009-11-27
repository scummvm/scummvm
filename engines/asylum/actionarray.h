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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_ACTIONARRAY_H_
#define ASYLUM_ACTIONARRAY_H_

#include "asylum/asylum.h"
#include "common/array.h"

namespace Asylum {

typedef struct ActionItem {
	int actionListIndex;
	int actionListItemIndex;
	int actorIndex;
	int field_C;
	int field_10;

} ActionItem;

typedef struct ActionStruct {
	ActionItem entries[10];
	int     count;
	int     field_CC;

} ActionStruct;

struct ActionDefinitions;

class ActionArray {
public:
	ActionArray(AsylumEngine *engine);
	virtual ~ActionArray();

	/** .text:00401020
	 * Reset the _actionArray entries to their
	 * default values
	 */
	void reset();

	/** .text:00401050
	 * Initialize the script element at actionIndex to
	 * the actor at actorIndex
	 *
	 * FIXME passing in a reference to the command at actionIndex
	 * to do a quick update.
	 */
	void initItem(ActionDefinitions *command, int actionIndex, int actorIndex);

	void setActionFlag(bool value) { _actionFlag = value; }

private:
	bool         _actionFlag;
	ActionStruct _items;
	AsylumEngine *_vm;

}; // end of class ActionArray

} // end of namespace Asylum

#endif /* ASYLUM_ACTIONARRAY_H_ */
