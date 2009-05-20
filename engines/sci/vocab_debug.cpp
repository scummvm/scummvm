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

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/resource.h"

namespace Sci {

#if 0
int *vocabulary_get_classes(ResourceManager *resmgr, int* count) {
	Resource* r;
	int *c;
	unsigned int i;

	if ((r = resmgr->findResource(kResourceTypeVocab, 996, 0)) == NULL)
		return 0;

	c = (int *)malloc(sizeof(int) * r->size / 2);
	for (i = 2; i < r->size; i += 4) {
		c[i/4] = READ_LE_UINT16(r->data + i);
	}
	*count = r->size / 4;

	return c;
}

int vocabulary_get_class_count(ResourceManager *resmgr) {
	Resource* r;

	if ((r = resmgr->findResource(kResourceTypeVocab, 996, 0)) == 0)
		return 0;

	return r->size / 4;
}
#endif

bool vocabulary_get_snames(ResourceManager *resmgr, bool isOldSci0, Common::StringList &selectorNames) {
	int count;

	Resource *r = resmgr->findResource(kResourceTypeVocab, 997, 0);

	if (!r) // No such resource?
		return false;

	count = READ_LE_UINT16(r->data) + 1; // Counter is slightly off

	for (int i = 0; i < count; i++) {
		int offset = READ_LE_UINT16(r->data + 2 + i * 2);
		int len = READ_LE_UINT16(r->data + offset);

		Common::String tmp((const char *)r->data + offset + 2, len);
		selectorNames.push_back(tmp);

		// Early SCI versions used the LSB in the selector ID as a read/write
		// toggle. To compensate for that, we add every selector name twice.
		if (isOldSci0)
			selectorNames.push_back(tmp);
	}

	return true;
}

int vocabulary_lookup_sname(const Common::StringList &selectorNames, const char *sname) {
	for (uint pos = 0; pos < selectorNames.size(); ++pos) {
		if (selectorNames[pos] == sname)
			return pos;
	}

	return -1;
}

void vocabulary_get_opcodes(ResourceManager *resmgr, Common::Array<opcode> &o) {
	int count, i = 0;
	Resource* r = resmgr->findResource(kResourceTypeVocab, VOCAB_RESOURCE_OPCODES, 0);

	o.clear();

	// if the resource couldn't be loaded, leave
	if (r == NULL) {
		warning("unable to load vocab.%03d", VOCAB_RESOURCE_OPCODES);
		return;
	}

	count = READ_LE_UINT16(r->data);

	o.resize(256);
	for (i = 0; i < count; i++) {
		int offset = READ_LE_UINT16(r->data + 2 + i * 2);
		int len = READ_LE_UINT16(r->data + offset) - 2;
		o[i].type = READ_LE_UINT16(r->data + offset + 2);
		o[i].number = i;
		o[i].name = Common::String((char *)r->data + offset + 4, len);
#if 1 //def VOCABULARY_DEBUG
		printf("Opcode %02X: %s, %d\n", i, o[i].name.c_str(), o[i].type);
#endif
	}
	for (i = count; i < 256; i++) {
		o[i].type = 0;
		o[i].number = i;
		o[i].name = "undefined";
	}
}

} // End of namespace Sci
