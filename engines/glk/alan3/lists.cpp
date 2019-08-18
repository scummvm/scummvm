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

#include "glk/alan3/lists.h"
#include "glk/alan3/syserr.h"

namespace Glk {
namespace Alan3 {

void initArray(void *array) {
	implementationOfSetEndOfArray((Aword *)array);
}

/* How to know we are at end of a table or array, first Aword == EOD */
void implementationOfSetEndOfArray(Aword *adr) {
	*adr = EOD;
}


bool implementationOfIsEndOfList(Aword *adr) {
	return *adr == EOD;
}

int lengthOfArrayImplementation(void *array_of_any_type, int element_size_in_bytes) {
	int length;
	int element_size = element_size_in_bytes / sizeof(Aword);
	Aword *array = (Aword *)array_of_any_type;
	if (array == NULL)
		syserr("Taking length of NULL array");
	for (length = 0; !isEndOfArray(&array[length * element_size]); length++)
		;
	return length;
}

void addElementImplementation(void *array_of_any_type, void *element, int element_size_in_bytes) {
	Aword *array = (Aword *)array_of_any_type;
	int length = lengthOfArray(array);
	int element_size_in_words = element_size_in_bytes / sizeof(Aword);
	memcpy(&array[length * element_size_in_words], element, element_size_in_bytes);
	setEndOfArray(&array[(length + 1)*element_size_in_words]);
}

} // End of namespace Alan3
} // End of namespace Glk
