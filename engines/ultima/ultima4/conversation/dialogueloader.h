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

#ifndef ULTIMA4_CONVERSATION_DIALOGUELOADER_H
#define ULTIMA4_CONVERSATION_DIALOGUELOADER_H

#include "common/hashmap.h"
#include "common/str.h"

namespace Ultima {
namespace Ultima4 {

class Dialogue;

/**
 * The generic dialogue loader interface.  Different dialogue
 * loaders should override the load method to load dialogues from
 * different sources (.tlk files, xml config elements, etc.).  They
 * must also register themselves with registerLoader for one or more
 * source types.  By convention, the source type of load() and
 * registerLoader() is an xu4-specific mime type.
 * The two main types used are application/x-u4tlk and text/x-u4cfg.
 */
class DialogueLoader {
public:
	virtual ~DialogueLoader() {}

	virtual Dialogue *load(void *source) = 0;
};

class DialogueLoaders {
private:
	Common::HashMap<Common::String, DialogueLoader *> _loaders;
public:
	static DialogueLoader *getLoader(const Common::String &mimeType);
public:
	DialogueLoaders();
	~DialogueLoaders();

	void registerLoader(DialogueLoader *loader, const Common::String &mimeType) {
		_loaders[mimeType] = loader;
	}

	DialogueLoader *operator[](const Common::String &mimeType) {
		return _loaders[mimeType];
	}
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
