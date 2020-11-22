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

#include <string.h>
#include <stdlib.h>
#include "ac/common.h"
#include "ac/tree_map.h"

namespace AGS3 {

TreeMap::TreeMap() {
	left = nullptr;
	right = nullptr;
	text = nullptr;
	translation = nullptr;
}

char *TreeMap::findValue(const char *key) {
	if (text == nullptr)
		return nullptr;

	if (strcmp(key, text) == 0)
		return translation;
	//debug_script_warn("Compare: '%s' with '%s'", key, text);

	if (strcmp(key, text) < 0) {
		if (left == nullptr)
			return nullptr;
		return left->findValue(key);
	} else {
		if (right == nullptr)
			return nullptr;
		return right->findValue(key);
	}
}

void TreeMap::addText(const char *ntx, char *trans) {
	if ((ntx == nullptr) || (ntx[0] == 0) ||
		((text != nullptr) && (strcmp(ntx, text) == 0)))
		// don't add if it's an empty string or if it's already here
		return;

	if (text == nullptr) {
		text = (char *)malloc(strlen(ntx) + 1);
		translation = (char *)malloc(strlen(trans) + 1);
		if (translation == nullptr)
			quit("load_translation: out of memory");
		strcpy(text, ntx);
		strcpy(translation, trans);
	} else if (strcmp(ntx, text) < 0) {
		// Earlier in alphabet, add to left
		if (left == nullptr)
			left = new TreeMap();

		left->addText(ntx, trans);
	} else if (strcmp(ntx, text) > 0) {
		// Later in alphabet, add to right
		if (right == nullptr)
			right = new TreeMap();

		right->addText(ntx, trans);
	}
}

void TreeMap::clear() {
	if (left) {
		left->clear();
		delete left;
	}
	if (right) {
		right->clear();
		delete right;
	}
	if (text)
		free(text);
	if (translation)
		free(translation);
	left = nullptr;
	right = nullptr;
	text = nullptr;
	translation = nullptr;
}

TreeMap::~TreeMap() {
	clear();
}

} // namespace AGS3
