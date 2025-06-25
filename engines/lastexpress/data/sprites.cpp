/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "lastexpress/lastexpress.h"
#include "sprites.h"

namespace LastExpress {

SpriteManager::SpriteManager(LastExpressEngine *engine) {
	_engine = engine;
}

void SpriteManager::drawCycle() {
	if (_drawSequencesFlag) {
		Extent extent = Extent(0, 480, 0, 0, 0, 0);
		Sprite *queue = _frameQueue;

		int oldLeft = 640;
		int oldRight = 0;
		
		if (_coordinatesAreSet) {
			if (_eraseRect.right >= _eraseRect.left &&
				_eraseRect.right < 640 &&
				_eraseRect.top <= _eraseRect.bottom &&
				_eraseRect.bottom < 480) {

				memcpy(&extent, &_eraseRect, sizeof(extent));

				oldRight = extent.right;
				oldLeft = extent.left;

				if (_engine->getGraphicsManager()->acquireSurface()) {
					_engine->getGraphicsManager()->copy(
						_engine->getGraphicsManager()->_backBuffer,
						(PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels(),
						_eraseRect.left,
						_eraseRect.top,
						_eraseRect.right - _eraseRect.left + 1,
						_eraseRect.bottom - _eraseRect.top + 1);
					_engine->getGraphicsManager()->unlockSurface();
				}
			}

			resetEraseQueue();
		}

		for (; queue; queue = queue->nextSprite) {
			if (queue->copyScreenAndRedrawFlag == 1)
				_engine->getGraphicsManager()->eraseSprite(queue->eraseMask);
		}

		for (Sprite *i = _frameQueue; i; i = i->nextSprite) {
			if (i->rect.left < oldLeft)
				oldLeft = i->rect.left;

			if (i->rect.top < extent.top)
				extent.top = i->rect.top;

			if (i->rect.right > oldRight)
				oldRight = i->rect.right;

			if (i->rect.bottom > extent.bottom)
				extent.bottom = i->rect.bottom;

			if (_engine->getGraphicsManager()->acquireSurface()) {
				if (i->compType) {
					if (i->compType == 1) {
						switch (i->compBits) {
						case 3:
							_engine->getGraphicsManager()->bitBltWax8(i, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
							break;
						case 4:
							_engine->getGraphicsManager()->bitBltWax16(i, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
							break;
						case 5:
							_engine->getGraphicsManager()->bitBltWax32(i, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
							break;
						case 7:
							_engine->getGraphicsManager()->bitBltWax128(i, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
							break;
						default:
							break;
						}
					} else if (i->compType == 2) {
						switch (i->compBits) {
						case 3:
							_engine->getGraphicsManager()->bitBltWane8(i, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
							break;
						case 4:
							_engine->getGraphicsManager()->bitBltWane16(i, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
							break;
						case 5:
							_engine->getGraphicsManager()->bitBltWane32(i, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
							break;
						case 7:
							_engine->getGraphicsManager()->bitBltWane128(i, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
							break;
						default:
							break;
						}
					}
				} else {
					switch (i->compBits) {
					case 3:
						_engine->getGraphicsManager()->bitBltSprite8(i, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
						break;
					case 4:
						_engine->getGraphicsManager()->bitBltSprite16(i, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
						break;
					case 5:
						_engine->getGraphicsManager()->bitBltSprite32(i, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
						break;
					case 7:
						_engine->getGraphicsManager()->bitBltSprite128(i, (PixMap *)_engine->getGraphicsManager()->_screenSurface.getPixels());
						break;
					default:
						break;
					}
				}

				_engine->getGraphicsManager()->unlockSurface();
			}
		}

		if (oldLeft != 640)
			_engine->getGraphicsManager()->burstBox(oldLeft, extent.top, oldRight - oldLeft + 1, extent.bottom - extent.top + 1);

		_drawSequencesFlag = false;
	}
}

void SpriteManager::drawCycleSimple(PixMap *pixels) {
	for (Sprite *i = _frameQueue; i; i = i->nextSprite) {
		if (i->compType != 2 && i->compType != 3) {
			switch (i->compBits) {
			case 3:
				_engine->getGraphicsManager()->bitBltSprite8(i, pixels);
				break;
			case 4:
				_engine->getGraphicsManager()->bitBltSprite16(i, pixels);
				break;
			case 5:
				_engine->getGraphicsManager()->bitBltSprite32(i, pixels);
				break;
			case 7:
				_engine->getGraphicsManager()->bitBltSprite128(i, pixels);
				break;
			default:
				continue;
			}
		}
	}
}

void SpriteManager::queueErase(Sprite *sprite) {
	if (sprite && sprite->compType != 3) {
		_coordinatesAreSet = true;

		if (_eraseRect.left > sprite->rect.left)
			_eraseRect.left = sprite->rect.left;

		if (_eraseRect.top > sprite->rect.top)
			_eraseRect.top = sprite->rect.top;

		if (_eraseRect.right < sprite->rect.right)
			_eraseRect.right = sprite->rect.right;

		if (_eraseRect.bottom < sprite->rect.bottom)
			_eraseRect.bottom = sprite->rect.bottom;
	}
}

void SpriteManager::resetEraseQueue() {
	_eraseRect.left = 640;
	_eraseRect.top = 480;
	_coordinatesAreSet = false;
	_eraseRect.right = 0;
	_eraseRect.bottom = 0;
}

void SpriteManager::killSpriteQueue() {
	_drawSequencesFlag = true;
	_frameQueue = nullptr;
}

void SpriteManager::touchSpriteQueue() {
	_drawSequencesFlag = true;
}

void SpriteManager::drawSprite(Sprite *sprite) {
	if (sprite) {
		Sprite *queue = _frameQueue;

		while (queue) {
			if (queue == sprite)
				return;

			queue = queue->nextSprite;
		}

		_drawSequencesFlag = true;

		if (_frameQueue) {
			bool insertedInQueue = false;

			if (sprite->hotspotPriority <= _frameQueue->hotspotPriority) {
				queue = _frameQueue;

				for (Sprite *i = _frameQueue->nextSprite; !insertedInQueue && i; i = i->nextSprite) {
					if (sprite->hotspotPriority > i->hotspotPriority) {
						queue->nextSprite = sprite;
						sprite->nextSprite = i;
						insertedInQueue = true;
					}

					queue = i;
				}

				if (!insertedInQueue) {
					queue->nextSprite = sprite;
					sprite->nextSprite = nullptr;
				}

			} else {
				sprite->nextSprite = _frameQueue;
				_frameQueue = sprite;
			}
		} else {
			_frameQueue = sprite;
			sprite->nextSprite = nullptr;
		}
	}
}

void SpriteManager::removeSprite(Sprite *sprite) {
	Sprite *queue = _frameQueue;
	Sprite *spriteToRemove = nullptr;

	if (sprite) {
		while (queue != sprite && queue) {
			spriteToRemove = queue;
			queue = queue->nextSprite;
		}

		if (queue) {
			if (spriteToRemove) {
				spriteToRemove->nextSprite = queue->nextSprite;
			} else {
				_frameQueue = queue->nextSprite;
			}

			_drawSequencesFlag = true;
		}
	}
}

void SpriteManager::destroySprite(Sprite **sprites, bool redrawFlag) {
	if (sprites && *sprites) {
		queueErase(*sprites);
		removeSprite(*sprites);

		if (redrawFlag)
			drawCycle();

		*sprites = nullptr;
	}
}

} // End of namespace LastExpress
