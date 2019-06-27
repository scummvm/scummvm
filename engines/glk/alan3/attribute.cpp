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

#include "glk/alan3/attribute.h"
#include "glk/alan3/syserr.h"
#include "glk/alan3/current.h"
#include "glk/alan3/lists.h"

namespace Glk {
namespace Alan3 {

/*----------------------------------------------------------------------*/
static AttributeEntry *findAttribute(AttributeEntry *attributeTable, int attributeCode) {
	AttributeEntry *attribute = attributeTable;
	while (attribute->code != attributeCode) {
		attribute++;
		if (isEndOfArray(attribute))
			syserr("Attribute not found.");
	}
	return attribute;
}


/*======================================================================*/
Aptr getAttribute(AttributeEntry *attributeTable, int attributeCode) {
	AttributeEntry *attribute = findAttribute(attributeTable, attributeCode);

	return attribute->value;
}


/*======================================================================*/
void setAttribute(AttributeEntry *attributeTable, int attributeCode, Aptr newValue) {
	AttributeEntry *attribute = findAttribute(attributeTable, attributeCode);

	attribute->value = newValue;
	gameStateChanged = TRUE;
}

} // End of namespace Alan3
} // End of namespace Glk
