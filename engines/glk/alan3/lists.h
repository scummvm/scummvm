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

#ifndef GLK_ALAN3_LISTS
#define GLK_ALAN3_LISTS

/* Various utility functions for handling lists and arrays */

#include "glk/alan3/acode.h"
#include "glk/alan3/types.h"

namespace Glk {
namespace Alan3 {

extern void initArray(void *array);

#define isEndOfArray(x) implementationOfIsEndOfList((Aword *) (x))
extern bool implementationOfIsEndOfList(Aword *adr);

#define setEndOfArray(x) implementationOfSetEndOfArray((Aword *) (x))
extern void implementationOfSetEndOfArray(Aword *adr);

#define lengthOfArray(array) lengthOfArrayImplementation((array), sizeof(*(array)))
extern int lengthOfArrayImplementation(void *array, int element_size_in_bytes);

#define addElement(array, element) addElementImplementation((array), (&element), sizeof(element))
extern void addElementImplementation(void *array_of_any_type, void *element_of_any_size, int element_size_in_bytes);
#endif

} // End of namespace Alan3
} // End of namespace Glk
