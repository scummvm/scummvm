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

#include <stdio.h>
#include <stdlib.h>

#include <engine.h>
#include <vocabulary.h>

int main(int argc, char** argv) {
	int res;
	int sizes[1000];
	int altsizes[1000];
	int count, *classes;
	loadResources(SCI_VERSION_AUTODETECT, 1);

	for (res = 0; res < 1000; res++) {
		resource_t* r;
		int i = 0;

		sizes[res] = -1;

		if ((r = findResource(sci_script, res)) == 0) continue;
		sizes[res] = 0;
		altsizes[res] = 0;
		i += 2;
		i = getInt16(r->data + i);

		while (i < r->length - 2) {
			switch (getInt16(r->data + i)) {
			case 1:
			case 6:
				sizes[res]++;
				break;
			default:
				altsizes[res]++;
			}
			i += getInt16(r->data + i + 2);
		}
		fflush(stdout);
	}

	for (res = 0; res < 1000; res++) if (sizes[res] != -1) printf("%03d %d\n", res, sizes[res]);
	printf("\n");

	classes = vocabulary_get_classes(&count);
	for (res = 0; res < count; res++) {
		printf("%03d %d (%d)\n", classes[res],
		       sizes[classes[res]]--, altsizes[classes[res]]);
	}

	freeResources();
	return 0;
}
