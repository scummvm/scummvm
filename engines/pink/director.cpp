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
#include "pink/objects/actors/actor.h"

namespace Pink {
Director::Director()
	: _surface(640, 480) {
	_wndManager.setScreen(&_surface);
}

void Director::update() {
	for (uint i = 0; i < _sounds.size(); ++i) {
		_sounds[i]->update();
	}

	for (uint i = 0; i < _sprites.size(); ++i) {
		if (_sprites[i]->needsUpdate())
			_sprites[i]->update();
	}

	draw();
}

void Director::addSprite(ActionCEL *sprite) {
	_sprites.push_back(sprite);
	int i;
	for (i = _sprites.size() - 1; i > 0 ; --i) {
		if (sprite->getZ() < _sprites[i - 1]->getZ())
			_sprites[i] = _sprites[i - 1];
		else
			break;
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
	_dirtyRects.push_back(sprite->getBounds());
}

void Director::removeSound(ActionSound *sound) {
	for (uint i = 0; i < _sounds.size(); ++i) {
		if (_sounds[i] == sound)
			_sounds.remove_at(i);
	}
}

void Director::clear() {
	_dirtyRects.push_back(Common::Rect(0, 0, 640, 480));
	_sprites.resize(0);
}

void Director::pause(bool pause) {
	for (uint i = 0; i < _sprites.size() ; ++i) {
		_sprites[i]->pause(pause);
	}
}

void Director::saveStage() {
	_savedSprites = _sprites;
	_sprites.clear();
}

void Director::loadStage() {
	assert(_sprites.empty());
	_sprites = _savedSprites;
	_savedSprites.clear();
}

Actor *Director::getActorByPoint(const Common::Point point) {
	for (int i = _sprites.size() - 1; i >= 0; --i) {
		if (_sprites[i]->getActor()->isCursor())
			continue;
		CelDecoder *decoder = _sprites[i]->getDecoder();
		const Graphics::Surface *frame = decoder->getCurrentFrame();
		const Common::Rect &rect = _sprites[i]->getBounds();
		if (rect.contains(point)) {
			byte spritePixel = *(const byte *)frame->getBasePtr(point.x - rect.left, point.y - rect.top);
			if (spritePixel != decoder->getTransparentColourIndex())
				return _sprites[i]->getActor();
		}
	}

	return nullptr;
}

void Director::draw() {
	if (!_dirtyRects.empty()) {
		mergeDirtyRects();

		for (uint i = 0; i < _dirtyRects.size(); ++i) {
			drawRect(_dirtyRects[i]);
		}

		_dirtyRects.resize(0);
		_surface.update();
	}
	else
		g_system->updateScreen();
}

void Director::mergeDirtyRects() {
	Common::Array<Common::Rect>::iterator rOuter, rInner;
	for (rOuter = _dirtyRects.begin(); rOuter != _dirtyRects.end(); ++rOuter) {
		rInner = rOuter;
		while (++rInner != _dirtyRects.end()) {
			if ((*rOuter).intersects(*rInner)) {
				// These two rectangles overlap, so merge them
				rOuter->extend(*rInner);

				// remove the inner rect from the list
				_dirtyRects.erase(rInner);

				// move back to beginning of list
				rInner = rOuter;
			}
		}
	}
}

void Director::addDirtyRect(const Common::Rect &rect) {
	_dirtyRects.push_back(rect);
}

void Director::addDirtyRects(ActionCEL *sprite) {
	const Common::Rect spriteRect = sprite->getBounds();
	const Common::List<Common::Rect> *dirtyRects = sprite->getDecoder()->getDirtyRects();
	if (dirtyRects->size() > 100) {
		_dirtyRects.push_back(spriteRect);
	} else {
		for (Common::List<Common::Rect>::const_iterator it = dirtyRects->begin(); it != dirtyRects->end(); ++it) {
			Common::Rect dirtyRect = *it;
			dirtyRect.translate(spriteRect.left, spriteRect.top);
			_dirtyRects.push_back(dirtyRect);
		}
	}
	sprite->getDecoder()->clearDirtyRects();
}

void Director::drawRect(const Common::Rect &rect) {
	_surface.fillRect(rect, 0);
	for (uint i = 0; i < _sprites.size(); ++i) {
		const Common::Rect &spriteRect = _sprites[i]->getBounds();
		Common::Rect interRect = rect.findIntersectingRect(spriteRect);
		if (interRect.isEmpty())
			continue;

		Common::Rect srcRect(interRect);
		srcRect.translate(-spriteRect.left, -spriteRect.top);
		_surface.transBlitFrom(*_sprites[i]->getDecoder()->getCurrentFrame(), srcRect, interRect, _sprites[i]->getDecoder()->getTransparentColourIndex());
	}
}

}
