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
#include <engines/pink/objects/actions/action_sound.h>
#include <engines/pink/objects/actions/action_cel.h>
#include <graphics/managed_surface.h>
#include "graphics/palette.h"
#include "cel_decoder.h"

namespace Pink {
Director::Director(OSystem *system)
    : _system(system) {}

void Director::draw() {
    for (int i = 0; i < _sprites.size(); ++i) {
        CelDecoder *decoder = _sprites[i]->getDecoder();
        drawSprite(decoder);
    }
    _system->updateScreen();
}

void Director::drawSprite(CelDecoder *decoder) {
    const Graphics::Surface *surface;
    if (decoder->needsUpdate())
        surface = decoder->decodeNextFrame();
    else surface = decoder->getCurrentFrame();



    uint16 colourIndex = decoder->getTransparentColourIndex();
    if (colourIndex != 0) {
        Graphics::Surface *screen = _system->lockScreen();
        for (int y = 0; y < decoder->getHeight(); ++y) {
            for (int x = 0; x < decoder->getWidth(); ++x) {
                byte spritePixelColourIndex = *(byte*)surface->getBasePtr(x, y);
                if (spritePixelColourIndex != colourIndex && spritePixelColourIndex != 229) { // hack because sprite have wrong colour index
                    *(byte *) screen->getBasePtr(decoder->getX() + x, decoder->getY() + y) = spritePixelColourIndex;
                }
            }
        }
        _system->unlockScreen();
    }
    else _system->copyRectToScreen(surface->getPixels(), surface->pitch,
                                   decoder->getX(), decoder->getY(),
                                   surface->w, surface->h);

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

void Director::update() {
    for (int i = 0; i < _sounds.size(); ++i) {
        _sounds[i]->update();
    }
    for (int i = 0; i < _sprites.size(); ++i) {
        _sprites[i]->update();
    }
}

void Director::addSound(ActionSound *sound) {
    _sounds.push_back(sound);
}

void Director::removeSound(ActionSound *sound) {
    for (int i = 0; i < _sounds.size(); ++i) {
        if (_sounds[i] == sound)
            _sounds.remove_at(i);
    }
}

void Director::clear() {
    _sprites.clear();
}

}