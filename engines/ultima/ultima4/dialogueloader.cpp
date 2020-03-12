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

#include "ultima/ultima4/debug.h"
#include "ultima/ultima4/conversation.h"
#include "ultima/ultima4/dialogueloader.h"

namespace Ultima {
namespace Ultima4 {

Std::map<Common::String, DialogueLoader *> *DialogueLoader::_loaderMap = NULL;

DialogueLoader *DialogueLoader::getLoader(const Common::String &mimeType) {
    ASSERT(_loaderMap != NULL, "DialogueLoader::getLoader loaderMap not initialized");
    if (_loaderMap->find(mimeType) == _loaderMap->end())
        return NULL;
    return (*_loaderMap)[mimeType];
}

DialogueLoader *DialogueLoader::registerLoader(DialogueLoader *loader, const Common::String &mimeType) {
    if (_loaderMap == NULL) {
        _loaderMap = new Std::map<Common::String, DialogueLoader *>;
    }
    (*_loaderMap)[mimeType] = loader;
    return loader;
}

} // End of namespace Ultima4
} // End of namespace Ultima
