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

#include "graphics/managed_surface.h"
#include "graphics/palette.h"

#include "pink/cel_decoder.h"
#include "pink/director.h"
#include "pink/objects/actions/action_sound.h"
#include "pink/objects/actions/action_cel.h"

namespace Pink {
Director::Director(OSystem *system)
    : _system(system), showBounds(1) {}

void Director::draw() {
    _system->fillScreen(0);
    for (uint i = 0; i < _sprites.size(); ++i) {
        drawSprite(_sprites[i]);
    }
    _system->updateScreen();
}

void Director::drawSprite(ActionCEL *sprite) {
    CelDecoder *decoder = sprite->getDecoder();
    const Graphics::Surface *surface;
    if (decoder->needsUpdate()) {

        surface = decoder->decodeNextFrame();
    }
    else surface = decoder->getCurrentFrame();

    int h = surface->h;
    if (surface->h + decoder->getY() > 480)
        h = 480 - decoder->getY();
    int w = surface->w;
    if (surface->w + decoder->getX() > 640)
        w = 640 - decoder->getX();

    if (!showBounds) {
        Graphics::Surface *screen = _system->lockScreen();

        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                uint16 spritePixelColourIndex = *(byte*)surface->getBasePtr(x, y);
                if (spritePixelColourIndex != decoder->getTransparentColourIndex()) {
                    *(byte *) screen->getBasePtr(decoder->getX() + x, decoder->getY() + y) = spritePixelColourIndex;
                }
            }
        }
        _system->unlockScreen();
    }
    else _system->copyRectToScreen(surface->getPixels(), surface->pitch,
                                   decoder->getX(), decoder->getY(),
                                   w, h);

}


void Director::addSprite(ActionCEL *sprite) {
    _sprites.push_back(sprite);
    int i;
    for (i = _sprites.size() - 1; i > 0 ; --i) {
        if (sprite->getZ() < _sprites[i - 1]->getZ()){
            _sprites[i] = _sprites[i - 1];
        } else break;
    }
    _sprites[i] = sprite;
}

void Director::removeSprite(ActionCEL *sprite) {
    for (uint i = 0; i < _sprites.size(); ++i) {
        if (sprite == _sprites[i]) {
            _sprites.remove_at(i);
            break;
        }
    }
}

void Director::setPallette(const byte *pallete) {
    _system->getPaletteManager()->setPalette(pallete, 0, 256);
}

void Director::update() {
    for (uint i = 0; i < _sounds.size(); ++i) {
        _sounds[i]->update();
    }
    for (uint i = 0; i < _sprites.size(); ++i) {
        _sprites[i]->update();
    }
}

void Director::addSound(ActionSound *sound) {
    _sounds.push_back(sound);
}

void Director::removeSound(ActionSound *sound) {
    for (uint i = 0; i < _sounds.size(); ++i) {
        if (_sounds[i] == sound)
            _sounds.remove_at(i);
    }
}

void Director::clear() {
    _sprites.clear();
}

Actor *Director::getActorByPoint(Common::Point point) {
    for (int i = _sprites.size() - 1; i >= 0; --i) {
        CelDecoder *decoder = _sprites[i]->getDecoder();
        const Graphics::Surface *frame = decoder->getCurrentFrame();
        Common::Rect &rect = decoder->getRectangle();
        if (rect.contains(point) &&
            *(byte*)frame->getBasePtr(point.x - rect.left, point.y - rect.top)
            != decoder->getTransparentColourIndex())
            return _sprites[i]->getActor();
    }

    return nullptr;
}

}
