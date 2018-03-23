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

#include "director.h"
#include <engines/pink/objects/actions/action_cel.h>
#include "graphics/surface.h"
#include "graphics/palette.h"
#include "cel_decoder.h"

namespace Pink {
Director::Director(OSystem *system)
    : _system(system) {}

void Director::draw() {
    bool needUpdate = 0;
    for (int i = 0; i < _sprites.size(); ++i) {
        CelDecoder *decoder = _sprites[i]->getDecoder();
        if (decoder->needsUpdate()) {
            const Graphics::Surface *surface = decoder->decodeNextFrame();
            _system->copyRectToScreen(surface->getPixels(), surface->pitch,
                                      decoder->getX(), decoder->getY(),
                                      surface->w, surface->h);
            needUpdate = 1;
        }
    }
    if (needUpdate)
        _system->updateScreen();
}

void Director::addSprite(ActionCEL *sprite) {
    _sprites.push_back(sprite); //TODO impl sorting
}

void Director::removeSprite(ActionCEL *sprite) {
    for (int i = 0; i < _sprites.size(); ++i) {
        if (sprite == _sprites[i]) {
            _sprites.remove_at(i);
            break;
        }
    }
}

void Director::setPallette(const byte *pallete) {
    _system->getPaletteManager()->setPalette(pallete, 0, 256);
}

}