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

#include "ultima/ultima4/conversation/conversation.h"
#include "ultima/ultima4/conversation/dialogueloader.h"
#include "ultima/ultima4/conversation/dialogueloader_hw.h"
#include "ultima/ultima4/conversation/dialogueloader_lb.h"
#include "ultima/ultima4/conversation/dialogueloader_tlk.h"

namespace Ultima {
namespace Ultima4 {

static DialogueLoaders *g_loaders;

DialogueLoaders::DialogueLoaders() {
	g_loaders = this;
	registerLoader(new U4HWDialogueLoader, "application/x-u4hwtlk");
	registerLoader(new U4LBDialogueLoader, "application/x-u4lbtlk");
	registerLoader(new U4TlkDialogueLoader(), "application/x-u4tlk");
}

DialogueLoaders::~DialogueLoaders() {
	for (Common::HashMap<Common::String, DialogueLoader *>::iterator it = _loaders.begin();
			it != _loaders.end(); ++it)
		delete it->_value;
	g_loaders = nullptr;
}

DialogueLoader *DialogueLoaders::getLoader(const Common::String &mimeType) {
	return (*g_loaders)[mimeType];
}

} // End of namespace Ultima4
} // End of namespace Ultima
