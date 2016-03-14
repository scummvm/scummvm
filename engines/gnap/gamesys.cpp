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
 */

#include "gnap/gamesys.h"
#include "gnap/fontdata.h"
#include "graphics/fontman.h"
#include "graphics/font.h"
#include "graphics/decoders/bmp.h"

namespace Gnap {

void GfxItem::testUpdRect(const Common::Rect &updRect) {
	Common::Rect intersectingRect;
	if (!updFlag && prevFrame.spriteId != -1 &&
		updRectsCount < 20 && intersectRect(intersectingRect, prevFrame.rect, updRect))
		updRects[updRectsCount++] = intersectingRect;
}

// GameSys

GameSys::GameSys(GnapEngine *vm)
	: _vm(vm) {
	
	_newSpriteDrawItemsCount = 0;
	_removeSequenceItemsCount = 0;
	_removeSpriteDrawItemsCount = 0;
	_grabSpriteId = -1;
	_grabSpriteChanged = false;
	_reqRemoveSequenceItem = false;
	_removeSequenceItemSequenceId = -1;
	_removeSequenceItemValue = 0;
	_gfxItemsCount = 0;
	_animationsCount = 0;
	_backgroundImageValue3 = 0;
	_backgroundImageValue1 = 0;
	_backgroundImageValue4 = 0;
	_backgroundImageValue2 = 0;
	_backgroundImageError = true;
	_gameSysClock = 0;
	_lastUpdateClock = 0;
	_backgroundSurface = 0;
	_frontSurface = 0;
	for (int i = 0; i < kMaxAnimations; ++i) {
		_animations[i].sequenceId = -1;
		_animations[i].id = 0;
		_animations[i].status = 0;
	}
	_screenRect = Common::Rect(0, 0, 800, 600);
}

GameSys::~GameSys() {
}

void GameSys::insertSequence(int sequenceId, int a2, int sequenceId2, int a4, int flags, int totalDuration, int16 x, int16 y) {
	debug(0, "GameSys::insertSequence() [%08X, %d] -> [%08X, %d] (%d, %d)", sequenceId, a2, sequenceId2, a4, x, y);
	Sequence sequence;
	SequenceResource *sequenceResource = _vm->_sequenceCache->get(sequenceId);
	sequenceResource->_sequenceId = sequenceId;
	sequence._sequenceId = sequenceId;
	sequence._id = a2 != -1 ? a2 : sequenceResource->_field_8;
	sequence._sequenceId2 = sequenceId2 != (int32)0x80000000 ? sequenceId2 : sequenceResource->_sequenceId2;
	sequence._id2 = a4 != -1 ? a4 : sequenceResource->_field_10;
	sequence._flags = flags != -1 ? flags : sequenceResource->_flags;
	sequence._totalDuration = totalDuration != -1 ? totalDuration : sequenceResource->_totalDuration;
	sequence._x = (x < 10000 && x > -10000) ? x : sequenceResource->_xOffs;
	sequence._y = (y < 10000 && y > -10000) ? y : sequenceResource->_yOffs;
	_fatSequenceItems.push_back(sequence);
}

void GameSys::insertDirtyRect(const Common::Rect &rect) {
	_dirtyRects.push_back(rect);
}

void GameSys::removeSequence(int sequenceId, int id, bool resetFl) {
	//WaitForSingleObject(removeSequence2Mutex, INFINITE);
	if (_removeSequenceItemsCount < kMaxSequenceItems) {
		_removeSequenceItems[_removeSequenceItemsCount]._sequenceId = sequenceId;
		_removeSequenceItems[_removeSequenceItemsCount]._id = id;
		_removeSequenceItems[_removeSequenceItemsCount]._forceFrameReset = resetFl;
		++_removeSequenceItemsCount;
		//ResetEvent(removeSequenceItemsEvent);
		//ReleaseMutex(removeSequence2Mutex);
		//WaitForSingleObject(removeSequenceItemsEvent, INFINITE);
	}
}

void GameSys::invalidateGrabCursorSprite(int id, Common::Rect &rect, Graphics::Surface *surface1, Graphics::Surface *surface2) {
	//WaitForSingleObject(grabSpriteMutex, INFINITE);
	_grabSpriteId = id;
	_grabSpriteRect = rect;
	_grabSpriteSurface2 = surface2;
	_grabSpriteSurface1 = surface1;
	//ResetEvent(grabSpriteEvent);
	_grabSpriteChanged = true;
	//ReleaseMutex(grabSpriteMutex);
	//WaitForSingleObject(grabSpriteEvent, INFINITE);
}

void GameSys::requestClear2(bool resetFl) {
	_fatSequenceItems.clear();
	_seqItems.clear();
	for (int i = 0; i < _gfxItemsCount; ++i) {
		GfxItem *gfxItem = &_gfxItems[i];
		gfxItem->sequenceId = -1;
		gfxItem->animation = 0;
		gfxItem->soundValue = 0;
		if (resetFl) {
			gfxItem->currFrame.duration = 0;
			gfxItem->currFrame.spriteId = -1;
			gfxItem->currFrame.soundId = -1;
			gfxItem->currFrame.unkValue = -1;
			gfxItem->updFlag = true;
		} else {
			gfxItem->updFlag = false;
		}
  	}
	_lastUpdateClock = 0;
	_gameSysClock = 0;
}

void GameSys::requestClear1() {
	_gfxItemsCount = 0;
	_fatSequenceItems.clear();
	_seqItems.clear();
	_lastUpdateClock = 0;
	_gameSysClock = 0;
}

void GameSys::requestRemoveSequence(int sequenceId, int id) {
	//WaitForSingleObject(removeSequence2Mutex, INFINITE);
	_reqRemoveSequenceItem = true;
	_removeSequenceItemSequenceId = sequenceId;
	_removeSequenceItemValue = id;
	
	handleReqRemoveSequenceItem(); //CHECKME?
	
	//ResetEvent(reqClearEvent);
	//ReleaseMutex(removeSequence2Mutex);
	//WaitForSingleObject(reqClearEvent, INFINITE);
}

void GameSys::waitForUpdate() {
	//ResetEvent(updateEvent);
	//WaitForSingleObject(updateEvent, INFINITE);
}

int GameSys::isSequenceActive(int sequenceId, int id) {
	for (uint i = 0; i < _seqItems.size(); ++i)
		if (_seqItems[i]._sequenceId == sequenceId && _seqItems[i]._id == id)
			return true;
	return false;
}

void GameSys::setBackgroundSurface(Graphics::Surface *surface, int a4, int a5, int a6, int a7) {

	debug(1, "GameSys::setBackgroundSurface() Setting background image");
	
	_backgroundSurface = surface;
	if (!_backgroundSurface) {
		_backgroundImageError = true;
		return;
	}
	
	if (!_frontSurface || _frontSurface->w != surface->w || _frontSurface->h != surface->h) {
		debug(1, "GameSys::setBackgroundSurface() Creating background working surface");
		if (_frontSurface)
			_frontSurface->free();
		delete _frontSurface;
		_frontSurface = new Graphics::Surface();
		_frontSurface->create(surface->w, surface->h, surface->format);
	}

	memcpy(_frontSurface->getPixels(), surface->getPixels(), surface->pitch * surface->h);

	_vm->_system->copyRectToScreen(_frontSurface->getPixels(), _frontSurface->pitch, 0, 0, _frontSurface->w, _frontSurface->h);

	_backgroundImageError = false;
	_backgroundImageValue1 = a4;
	_backgroundImageValue3 = a6;
	_backgroundImageValue2 = a5;
	_backgroundImageValue4 = a7;
	_lastUpdateClock = 0;
	_gameSysClock = 0;

}

void GameSys::setScaleValues(int a1, int a2, int a3, int a4) {
	_backgroundImageValue1 = a1;
	_backgroundImageValue3 = a3;
	_backgroundImageValue2 = a2;
	_backgroundImageValue4 = a4;
}

void GameSys::insertSpriteDrawItem(Graphics::Surface *surface, int x, int y, int id) {
	if (surface && _newSpriteDrawItemsCount < kMaxSpriteDrawItems) {
		_newSpriteDrawItems[_newSpriteDrawItemsCount]._id = id;
		_newSpriteDrawItems[_newSpriteDrawItemsCount]._rect = Common::Rect(x, y, x + surface->w, y + surface->h);
		_newSpriteDrawItems[_newSpriteDrawItemsCount]._surface = surface;
		++_newSpriteDrawItemsCount;
	}
}

void GameSys::removeSpriteDrawItem(Graphics::Surface *surface, int id) {
	if (surface && _removeSpriteDrawItemsCount < kMaxSpriteDrawItems) {
		_removeSpriteDrawItems[_removeSpriteDrawItemsCount]._id = id;
		_removeSpriteDrawItems[_removeSpriteDrawItemsCount].surface = surface;
		++_removeSpriteDrawItemsCount;
	}
}

void GameSys::drawSpriteToBackground(int x, int y, int resourceId) {

	SpriteResource *spriteResource = _vm->_spriteCache->get(resourceId);
	uint32 *sourcePalette = spriteResource->_palette;
	byte *sourcePixels = spriteResource->_pixels;
	int spriteWidth = spriteResource->_width;
	int spriteHeight = spriteResource->_height;
	Common::Rect dstRect(0, 0, spriteWidth, spriteHeight);
	blitSprite32(_backgroundSurface, x, y, sourcePixels, spriteResource->_width, dstRect, sourcePalette, true);//spriteResource->_transparent != 0);
	_vm->_spriteCache->release(resourceId);

	// Add dirty rect so the modified background is redrawn
	insertDirtyRect(Common::Rect(x, y, x + spriteWidth, y + spriteHeight));
}

Graphics::Surface *GameSys::allocSurface(int width, int height) {
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(width, height, _backgroundSurface->format);
	surface->fillRect(Common::Rect(0, 0, surface->w, surface->h), 0xFFFF00FF);
	return surface;
}

Graphics::Surface *GameSys::createSurface(int resourceId) {
	debug(0, "GameSys::createSurface() resourceId: %08X", resourceId);
	
	SpriteResource *spriteResource = _vm->_spriteCache->get(resourceId);
	Graphics::Surface *surface = allocSurface(spriteResource->_width, spriteResource->_height);
	_vm->_spriteCache->release(resourceId);
	
	drawSpriteToSurface(surface, 0, 0, resourceId);

	return surface;
}

void GameSys::drawSpriteToSurface(Graphics::Surface *surface, int x, int y, int resourceId) {
	SpriteResource *spriteResource = _vm->_spriteCache->get(resourceId);
	uint32 *sourcePalette = spriteResource->_palette;
	byte *sourcePixels = spriteResource->_pixels;
	Common::Rect dstRect(0, 0, spriteResource->_width, spriteResource->_height);
	blitSprite32(surface, x, y, sourcePixels, spriteResource->_width, dstRect, sourcePalette, true);//spriteResource->_transparent != 0);
	_vm->_spriteCache->release(resourceId);
}

void GameSys::drawTextToSurface(Graphics::Surface *surface, int x, int y, byte r, byte g, byte b, const char *text) {

	// NOTE Not that nice but will have to do for now
	
	bool doDirty = false;

	if (!surface) {
		surface = _backgroundSurface;
		doDirty = true;
	}
	
	uint32 color = surface->format.RGBToColor(r, g, b);

	for (const char *cp = text; *cp != 0; ++cp) {
		byte c = *cp;
		if (c < 32 || c > 127)
			c = (byte)'_';
		c -= 32;
		int w = _dejaVuSans9ptCharDescriptors[c]._width;
		const byte *data = _dejaVuSans9ptCharBitmaps + _dejaVuSans9ptCharDescriptors[c]._offset;
		for (int xc = 0; xc < w; ++xc) {
			for (int yc = 15; yc >= 0; --yc) {
				byte *dst = (byte*)surface->getBasePtr(x + xc, y + yc);
				if (data[1 - (yc >> 3)] & (1 << (yc & 7)))
					WRITE_LE_UINT32(dst, color);
			}
			data += 2;
		}
		x += w + 1;
	}
	
	if (doDirty) {
		insertDirtyRect(Common::Rect(x, y, x + getTextWidth(text), y + 16));
	}
	
}

int GameSys::getTextWidth(const char *text) {
	int width = 0;
	for (const char *cp = text; *cp != 0; ++cp) {
		byte c = *cp;
		if (c < 32 || c > 127)
			c = (byte)'_';
		c -= 32;
		width += _dejaVuSans9ptCharDescriptors[c]._width + 1;
	}
	return width;
}

void GameSys::fillSurface(Graphics::Surface *surface, int x, int y, int width, int height, byte r, byte g, byte b) {
	Common::Rect rect(x, y, x + width, y + height);
	if (!surface) {
		_backgroundSurface->fillRect(rect, _backgroundSurface->format.RGBToColor(r, g, b)); 
		insertDirtyRect(rect);
	} else {
		surface->fillRect(rect, surface->format.RGBToColor(r, g, b)); 
	}
}

void GameSys::setAnimation(int sequenceId, int id, int animationIndex) {
	if (animationIndex < kMaxAnimations) {
		_animations[animationIndex].sequenceId = sequenceId;
		_animations[animationIndex].id = id;
		_animations[animationIndex].status = 0;
	}
}

int GameSys::getAnimationStatus(int animationIndex) {
	int result = -1;
	if (animationIndex < kMaxAnimations)
		result = _animations[animationIndex].status;
	return result;
}

int GameSys::getSpriteWidthById(int resourceId) {
	SpriteResource *spriteResource = _vm->_spriteCache->get(resourceId);
	const int width = spriteResource->_width;
	_vm->_spriteCache->release(resourceId);
	return width;
}

int GameSys::getSpriteHeightById(int resourceId) {
	SpriteResource *spriteResource = _vm->_spriteCache->get(resourceId);
	const int height = spriteResource->_height;
	_vm->_spriteCache->release(resourceId);
	return height;
}

Graphics::Surface *GameSys::loadBitmap(int resourceId) {
	debug("GameSys::loadBitmap() resourceId: %08X", resourceId);
	if (_vm->_dat->getResourceType(resourceId) != 1)
		return 0;
	byte *resourceData = _vm->_dat->loadResource(resourceId);
	uint32 resourceSize = _vm->_dat->getResourceSize(resourceId);
	Common::MemoryReadStream stream(resourceData, resourceSize, DisposeAfterUse::NO);
	Graphics::Surface *bmpSurface;
	Graphics::BitmapDecoder bmp;
	if (!bmp.loadStream(stream))
		error("GameSys::loadBitmap() Could not load bitmap resource %08X", resourceId);
	bmpSurface = bmp.getSurface()->convertTo(_vm->_system->getScreenFormat());
	delete[] resourceData;
	return bmpSurface;
}

void GameSys::drawBitmap(int resourceId) {
	Graphics::Surface *bmpSurface = loadBitmap(resourceId);
	if (!bmpSurface || !_backgroundSurface) {
		debug("GameSys::drawBitmap() Error loading the bitmap");
		return;
	}
	if (bmpSurface->format != _backgroundSurface->format ||
		bmpSurface->w != _backgroundSurface->w || bmpSurface->h != _backgroundSurface->h) {
		debug("GameSys::drawBitmap() Different bitmap properties than current background");
	} else {
		byte *src = (byte*)bmpSurface->getPixels();
		byte *dst = (byte*)_backgroundSurface->getPixels();
		const int pitch = bmpSurface->pitch;
		int height = bmpSurface->h;
		while (height--) {
			memcpy(dst, src, pitch);
			src += pitch;
			dst += pitch;
		}
	}
	bmpSurface->free();
	delete bmpSurface;	

	insertDirtyRect(Common::Rect(0, 0, 800, 600));
}

Sequence *GameSys::seqFind(int sequenceId, int id, int *outIndex) {
	for (uint i = 0; i < _seqItems.size(); ++i)
		if (_seqItems[i]._sequenceId == sequenceId && _seqItems[i]._id == id) {
			if (outIndex)
				*outIndex = i;
			return &_seqItems[i];
		}
	return 0;
}

int GameSys::seqLocateGfx(int sequenceId, int id, int *outGfxIndex) {
	for (int i = 0; i < _gfxItemsCount; ++i) {
		GfxItem *gfxItem = &_gfxItems[i];
		if (gfxItem->sequenceId == sequenceId && gfxItem->id == id) {
			if (outGfxIndex)
				*outGfxIndex = i;
			return gfxItem->sequenceId;
		}
		if (gfxItem->id > id) {
			if (outGfxIndex)
				*outGfxIndex = i;
			return 0;
		}
	}
	if (outGfxIndex)
		*outGfxIndex = _gfxItemsCount;
	return 0;
}

void GameSys::seqInsertGfx(int index, int duration) {
	Sequence *seqItem = &_seqItems[index];
	SequenceResource *sequenceResource = _vm->_sequenceCache->get(seqItem->_sequenceId);	

	if (sequenceResource->_animationsCount > 50 - _gfxItemsCount)
		return;

	int gfxIndex;
	seqLocateGfx(seqItem->_sequenceId, seqItem->_id, &gfxIndex);
	
	if (gfxIndex != _gfxItemsCount)
		memcpy(&_gfxItems[gfxIndex + sequenceResource->_animationsCount],	&_gfxItems[gfxIndex], sizeof(GfxItem) * (_gfxItemsCount - gfxIndex));
	_gfxItemsCount += sequenceResource->_animationsCount;
	
	for (int i = 0; i < sequenceResource->_animationsCount; ++i) {
		int totalDuration;
		GfxItem *gfxItem = &_gfxItems[i + gfxIndex];
		SequenceAnimation *animation = &sequenceResource->_animations[i];

		debug(0, "GameSys::seqInsertGfx() seqItem->sequenceId: %08X", seqItem->_sequenceId);

		gfxItem->sequenceId = seqItem->_sequenceId;
		gfxItem->id = seqItem->_id;
		gfxItem->animation = animation;
		gfxItem->currFrameNum = 0;
		gfxItem->flags = 0;
		gfxItem->delayTicks = seqItem->_totalDuration + animation->field_4;
		gfxItem->updFlag = false;
		gfxItem->updRectsCount = 0;
		gfxItem->prevFrame.duration = 0;
		gfxItem->prevFrame.spriteId = -1;
		gfxItem->prevFrame.soundId = -1;
		gfxItem->prevFrame.unkValue = -1;
		totalDuration = duration;
		if ((seqItem->_flags & 4) && totalDuration > 0) {
			gfxItem->prevFrame.duration = 1;
			if (gfxItem->delayTicks <= totalDuration)
				gfxItem->delayTicks = 0;
			else
				gfxItem->delayTicks -= totalDuration + 1;
			gfxItem->updFlag = false;
		} else if (gfxItem->delayTicks <= totalDuration) {
			int j;
			totalDuration -= gfxItem->delayTicks;
			gfxItem->delayTicks = 0;
			for (j = gfxItem->currFrameNum; j < animation->framesCount && animation->frames[j].duration <= totalDuration; ++j) {
				if (animation->frames[j].soundId != -1)
					_soundIds.push_back((gfxItem->sequenceId & 0xFFFF0000) | animation->frames[j].soundId);
				totalDuration -= animation->frames[j].duration;
			}
			if (animation->framesCount > j)
				gfxItem->currFrame = animation->frames[j++];
			else
				gfxItem->currFrame = animation->frames[j - 1];
			if (gfxItem->currFrame.spriteId != -1 && (seqItem->_x != 0 || seqItem->_y != 0))
				gfxItem->currFrame.rect.translate(seqItem->_x, seqItem->_y);
			// Update sprite scaling				
			if ((seqItem->_flags & 1) && gfxItem->currFrame.rect.bottom >= _backgroundImageValue1 && gfxItem->currFrame.rect.bottom <= _backgroundImageValue3) {
				int scaleValue = _backgroundImageValue2	+ (gfxItem->currFrame.rect.bottom - _backgroundImageValue1) *
					(_backgroundImageValue4 - _backgroundImageValue2) /
					(_backgroundImageValue3 - _backgroundImageValue1);
				gfxItem->currFrame.rect.top = gfxItem->currFrame.rect.bottom - scaleValue * (gfxItem->currFrame.rect.bottom - gfxItem->currFrame.rect.top) / 1000;
				gfxItem->currFrame.rect.right = scaleValue * (gfxItem->currFrame.rect.right - gfxItem->currFrame.rect.left) / 1000 + gfxItem->currFrame.rect.left;
				gfxItem->currFrame.isScaled = 1;
			}
			gfxItem->currFrame.duration -= totalDuration;
			if (gfxItem->currFrame.soundId != -1)
				_soundIds.push_back((gfxItem->sequenceId & 0xFFFF0000) | gfxItem->currFrame.soundId);
			gfxItem->currFrameNum = j;
			gfxItem->updFlag = true;
		} else {
			gfxItem->delayTicks -= totalDuration + 1;
			gfxItem->updFlag = false;
		}
	}

	for (int k = 0; k < kMaxAnimations; ++k) {
		if (_animations[k].sequenceId != -1 && _animations[k].sequenceId == seqItem->_sequenceId && _animations[k].id == seqItem->_id) {
			_animations[k].status = 1;
			break;
		}
	}
	
}

void GameSys::seqRemoveGfx(int sequenceId, int id) {
	int gfxIndex;
	if (seqLocateGfx(sequenceId, id, &gfxIndex)) {
		GfxItem *gfxItem = &_gfxItems[gfxIndex];
		while (gfxIndex < _gfxItemsCount && gfxItem->sequenceId == sequenceId && gfxItem->id == id) {
			if (gfxItem->prevFrame.spriteId == -1) {
				--_gfxItemsCount;
				if (gfxIndex != _gfxItemsCount)
					memcpy(&_gfxItems[gfxIndex], &_gfxItems[gfxIndex + 1], sizeof(GfxItem) * (_gfxItemsCount - gfxIndex));
			} else {
				gfxItem->sequenceId = -1;
				gfxItem->animation = 0;
				gfxItem->currFrame.duration = 0;
				gfxItem->currFrame.spriteId = -1;
				gfxItem->currFrame.soundId = -1;
				gfxItem->currFrame.unkValue = -1;
				gfxItem->updFlag = true;
				++gfxIndex;
				gfxItem = &_gfxItems[gfxIndex];
			}
		}
	}
}

bool GameSys::updateSequenceDuration(int sequenceId, int id, int *outDuration) {
	bool found = false;
	int duration = 0x7FFFFFFF;
	for (int i = 0; i < _gfxItemsCount; ++i) {
		GfxItem *gfxItem = &_gfxItems[i];
		if (gfxItem->sequenceId == sequenceId && gfxItem->id == id) {
			found = true;
			SequenceAnimation *animation = gfxItem->animation;
			if (animation) {
				if (gfxItem->currFrameNum < animation->framesCount)
					return false;
				if (gfxItem->updFlag) {
					if (gfxItem->currFrame.duration > 0)
						return false;
					if (-gfxItem->currFrame.duration < duration)
						duration = -gfxItem->currFrame.duration;
				} else {
					if (gfxItem->prevFrame.duration > 0)
						return false;
					if (-gfxItem->prevFrame.duration < duration)
						duration = -gfxItem->prevFrame.duration;
				}
			}
		}
	}
	if (found)
		*outDuration = duration;
	return found;
}

void GameSys::updateAnimationsStatus(int sequenceId, int id) {

	Animation *foundAnimation = 0;
	for (int animationIndex = 0; animationIndex < kMaxAnimations; ++animationIndex) {
		Animation *animation = &_animations[animationIndex];
		if (animation->sequenceId != -1 && animation->sequenceId == sequenceId && animation->id == id) {
			foundAnimation = animation;
			break;
		}
	}
	
	if (!foundAnimation)
		return;

	bool foundSequence = false;
	for (int i = 0; i < _gfxItemsCount; ++i) {
		GfxItem *gfxItem = &_gfxItems[i];
		SequenceAnimation *animation = gfxItem->animation;
		if (gfxItem->sequenceId == sequenceId && gfxItem->id == id && animation) {
			foundSequence = true;
			if (animation->framesCount > gfxItem->currFrameNum ||
				(gfxItem->updFlag && gfxItem->currFrame.duration > 1) ||
				gfxItem->prevFrame.duration > 1)
				foundSequence = false;
			break;
		}
	}

	if (foundSequence) {
		foundAnimation->sequenceId = -1;
		foundAnimation->status = 2;
	}

}

void GameSys::restoreBackgroundRect(const Common::Rect &rect) {
	Common::Rect clipRect;	
	if (!intersectRect(clipRect, rect, _screenRect))
		return;
	byte *src = (byte*)_backgroundSurface->getBasePtr(clipRect.left, clipRect.top);
	byte *dst = (byte*)_frontSurface->getBasePtr(clipRect.left, clipRect.top);
	const int bytes = _backgroundSurface->format.bytesPerPixel * clipRect.width();
	int height = clipRect.height();
	while (height--) {
		memcpy(dst, src, bytes);
		src += _backgroundSurface->pitch;
		dst += _frontSurface->pitch;
	}
}

void GameSys::blitSurface32(Graphics::Surface *destSurface, int x, int y, Graphics::Surface *sourceSurface,
	Common::Rect &sourceRect, bool transparent) {
	
	const int sourcePitch = sourceSurface->pitch;
	byte *dst = (byte*)destSurface->getBasePtr(x, y);
	byte *src = (byte*)sourceSurface->getBasePtr(sourceRect.left, sourceRect.top);
	int width = sourceRect.width();
	int height = sourceRect.height();
	while (height--) {
		byte *rsrc = src;		
		byte *rdst = dst;
		for (int xc = 0; xc < width; ++xc) {
			uint32 pixel = READ_LE_UINT32(rsrc);
			if (!transparent || pixel != 0xFFFF00FF)
				WRITE_LE_UINT32(rdst, pixel);
			rsrc += 4;
			rdst += 4;
		}
		dst += destSurface->pitch;
		src += sourcePitch;
	}
}

void GameSys::blitSprite32(Graphics::Surface *destSurface, int x, int y, byte *sourcePixels,
	int sourceWidth, Common::Rect &sourceRect, uint32 *sourcePalette, bool transparent) {
	
	const int sourcePitch = (sourceWidth + 3) & 0xFFFFFFFC;
	byte *dst = (byte*)destSurface->getBasePtr(x, y);
	byte *src = sourcePixels + sourceRect.left + sourcePitch * sourceRect.top;
	int width = sourceRect.width();
	int height = sourceRect.height();
	while (height--) {
		byte *rdst = dst;		
		for (int xc = 0; xc < width; ++xc) {
			byte srcPixel = src[xc];
			if (!transparent || srcPixel) {
				uint32 rgb = sourcePalette[srcPixel];
				rdst[0] = rgb & 0x000000FF;
				rdst[1] = (rgb & 0x0000FF00) >> 8;
				rdst[2] = (rgb & 0x00FF0000) >> 16;
			}
			rdst += 4;
		}
		dst += destSurface->pitch;
		src += sourcePitch;
	}
}

void GameSys::blitSpriteScaled32(Graphics::Surface *destSurface, Common::Rect &frameRect,
	Common::Rect &destRect, byte *sourcePixels, int sourceWidth, Common::Rect &sourceRect, uint32 *sourcePalette) {
	
	if (frameRect.height() <= 0 || frameRect.width() <= 0)
		return;

	const int ys = ((sourceRect.bottom - sourceRect.top - 1) << 16) / (frameRect.bottom - frameRect.top - 1);
	const int xs = ((sourceRect.right - sourceRect.left - 1) << 16) / (frameRect.right - frameRect.left - 1);
	const int destPitch = destSurface->pitch;
	const int sourcePitch = (sourceWidth + 3) & 0xFFFFFFFC;

	if (!frameRect.equals(destRect)) {
		byte *dst = (byte*)destSurface->getBasePtr(destRect.left, destRect.top);
		byte *src = sourcePixels + sourcePitch * sourceRect.top + sourceRect.left;
		const int height = destRect.bottom - destRect.top;
		const int width = destRect.right - destRect.left;
		int yi = ys * (destRect.top - frameRect.top);
		byte *hsrc = src + sourcePitch * ((yi + 0x8000) >> 16);
		for (int i = 0; i < height; ++i) {
			byte *wdst = dst;
			int xi = xs * (destRect.left - frameRect.left);
			byte *wsrc = hsrc + ((xi + 0x8000) >> 16);
			for (int j = 0; j < width; ++j) {
				byte srcPixel = *wsrc;
				if (srcPixel) {
					uint32 rgb = sourcePalette[srcPixel];
					wdst[0] = rgb & 0x000000FF;
					wdst[1] = (rgb & 0x0000FF00) >> 8;
					wdst[2] = (rgb & 0x00FF0000) >> 16;
				}
				wdst += 4;
				xi += xs;
				wsrc = hsrc + ((xi + 0x8000) >> 16);
			}
			dst += destPitch;
			yi += ys;
			hsrc = src + sourcePitch * ((yi + 0x8000) >> 16);
		}
	} else {
		byte *dst = (byte*)destSurface->getBasePtr(frameRect.left, frameRect.top);
		byte *src = sourcePixels + sourcePitch * sourceRect.top + sourceRect.left;
		const int height = frameRect.bottom - frameRect.top;
		const int width = frameRect.right - frameRect.left;
		byte *hsrc = sourcePixels + sourcePitch * sourceRect.top + sourceRect.left;
		int yi = 0;
		for (int i = 0; i < height; ++i) {
			byte *wdst = dst;
			byte *wsrc = hsrc;
			int xi = 0;
			for (int j = 0; j < width; ++j) {
				byte srcPixel = *wsrc;
				if (srcPixel) {
					uint32 rgb = sourcePalette[srcPixel];
					wdst[0] = rgb & 0x000000FF;
					wdst[1] = (rgb & 0x0000FF00) >> 8;
					wdst[2] = (rgb & 0x00FF0000) >> 16;
				}
				wdst += 4;
				xi += xs;
				wsrc = hsrc + ((xi + 0x8000) >> 16);
			}
			dst += destPitch;
			yi += ys;
			hsrc = src + sourcePitch * ((yi + 0x8000) >> 16);
		}
	}

}

void GameSys::seqDrawStaticFrame(Graphics::Surface *surface, SequenceFrame &frame, Common::Rect *subRect) {
	debug(1, "GameSys::seqDrawStaticFrame() rect: (%d, %d, %d, %d)",
		frame.rect.left, frame.rect.top, frame.rect.right, frame.rect.bottom);
	
	Common::Rect srcRect = subRect ? *subRect : frame.rect;
	Common::Rect clipRect;
	
	if (!intersectRect(clipRect, srcRect, _screenRect)) {
		debug(1, "GameSys::seqDrawStaticFrame() Surface not inside screen");
		return;
	}

	const int x = clipRect.left, y = clipRect.top;
	
	clipRect.translate(-frame.rect.left, -frame.rect.top);

	// TODO Save transparent flag somewhere
	blitSurface32(_frontSurface, x, y, surface, clipRect, true);

}

void GameSys::seqDrawSpriteFrame(SpriteResource *spriteResource, SequenceFrame &frame, Common::Rect *subRect) {
	debug(1, "GameSys::seqDrawSpriteFrame() spriteId: %04X; rect: (%d, %d, %d, %d)",
		frame.spriteId, frame.rect.left, frame.rect.top, frame.rect.right, frame.rect.bottom);
	
	Common::Rect srcRect = subRect ? *subRect : frame.rect;
	Common::Rect clipRect;
	
	if (!intersectRect(clipRect, srcRect, _screenRect)) {
		debug(1, "GameSys::seqDrawSpriteFrame() Sprite not inside screen");
		return;
	}

	uint32 *sourcePalette = spriteResource->_palette;
	byte *sourcePixels = spriteResource->_pixels;

	const int x = clipRect.left, y = clipRect.top;
	
	debug(1, "GameSys::seqDrawSpriteFrame() destX: %d; destY: %d; frame.isScaled: %d", x, y, frame.isScaled);

	// 32bit sprite drawing
	if (frame.isScaled) {
		Common::Rect sourceRect(0, 0, spriteResource->_width, spriteResource->_height);
		blitSpriteScaled32(_frontSurface,	frame.rect, clipRect, sourcePixels, spriteResource->_width, sourceRect, sourcePalette);
	} else {
		clipRect.translate(-frame.rect.left, -frame.rect.top);
		blitSprite32(_frontSurface, x, y, sourcePixels, spriteResource->_width, clipRect, sourcePalette, true);//spriteResource->_transparent != 0);
	}

}

void GameSys::drawSprites() {

	debug(1, "GameSys::drawSprites() _gfxItemsCount: %d", _gfxItemsCount);

	// TODO Split into multiple functions for clarity

	// Restore dirty background and collect rects to be redrawn for all sprites 
	// which aren't marked to be redrawn yet
	Common::Rect intersectingRect;
	for (uint i = 0; i < _dirtyRects.size(); ++i) {
		restoreBackgroundRect(_dirtyRects[i]);
		for (int j = 0; j < _gfxItemsCount; ++j)
			_gfxItems[j].testUpdRect(_dirtyRects[i]);
	}
	
	for (int k = 0; k < _gfxItemsCount; ++k) {
		GfxItem *gfxItem2 = &_gfxItems[k];

		if (!gfxItem2->updFlag)
			continue;

		if (gfxItem2->prevFrame.spriteId != -1) {
			int transparent = 0;
			if (gfxItem2->currFrame.spriteId != -1 && !(gfxItem2->flags & 2)) {
				if (gfxItem2->flags & 1) {
					transparent = 1;
				} else {
					int resourceId = (gfxItem2->sequenceId & 0xFFFF0000) | gfxItem2->currFrame.spriteId;
					SpriteResource *spriteResource = _vm->_spriteCache->get(resourceId);
					transparent = spriteResource->_transparent;
					_vm->_spriteCache->release(resourceId);
				}
			}
			if ((gfxItem2->flags & 8) || gfxItem2->currFrame.spriteId == -1 ||
				(!(gfxItem2->flags & 2) && (!gfxItem2->prevFrame.rect.equals(gfxItem2->currFrame.rect) || !transparent))) {
				restoreBackgroundRect(gfxItem2->prevFrame.rect);
				for (int l = 0; l < _gfxItemsCount; ++l)
					_gfxItems[l].testUpdRect(gfxItem2->prevFrame.rect);
			}
		}

		if (gfxItem2->currFrame.spriteId != -1) {
			int transparent = 0;
			if (gfxItem2->flags & 1) {
				transparent = 1;
			} else if (!(gfxItem2->flags & 2)) {
				int resourceId = (gfxItem2->sequenceId & 0xFFFF0000) | gfxItem2->currFrame.spriteId;
				SpriteResource *spriteResource = _vm->_spriteCache->get(resourceId);
				transparent = spriteResource->_transparent;
				_vm->_spriteCache->release(resourceId);
			}
			if (((gfxItem2->flags & 2) && !(gfxItem2->flags & 8)) ||
				gfxItem2->prevFrame.spriteId == -1 ||
				!gfxItem2->prevFrame.rect.equals(gfxItem2->currFrame.rect) || transparent) {
				for (int l = k; l < _gfxItemsCount; ++l)
					_gfxItems[l].testUpdRect(gfxItem2->currFrame.rect);
			}
		}

	}

	for (int m = 0; m < _gfxItemsCount; ++m) {
		GfxItem *gfxItem5 = &_gfxItems[m];
		
		debug(0, "DrawGfxItem(%d) updFlag: %d; currFrame.spriteId: %04X; updRectsCount: %d; flags: %04X; sequenceId: %08X",
			m, gfxItem5->updFlag, gfxItem5->currFrame.spriteId, gfxItem5->updRectsCount, gfxItem5->flags, gfxItem5->sequenceId);
		
		if (gfxItem5->updFlag) {
			if (gfxItem5->currFrame.spriteId != -1) {
				if (gfxItem5->flags & 1) {
					seqDrawStaticFrame(gfxItem5->surface, gfxItem5->currFrame, 0);
					//debug("seqDrawStaticFrame");
				} else if (gfxItem5->flags & 2) {
					// TODO seqDrawAviFrame(gfxItem5->currFrame.spriteId, &gfxItem5->currFrame, 0, gfxItem5->flags & 8);
				} else {
					int resourceId = (gfxItem5->sequenceId & 0xFFFF0000) | gfxItem5->currFrame.spriteId;
					SpriteResource *spriteResource = _vm->_spriteCache->get(resourceId);
					seqDrawSpriteFrame(spriteResource, gfxItem5->currFrame, 0);
					_vm->_spriteCache->release(resourceId);
				}
			}
		} else if (gfxItem5->updRectsCount > 0) {
			if (gfxItem5->flags & 1) {
				for (int n = 0; n < gfxItem5->updRectsCount; ++n)
					seqDrawStaticFrame(gfxItem5->surface, gfxItem5->prevFrame, &gfxItem5->updRects[n]);
			} else if (gfxItem5->flags & 2) {
				/* TODO
				spriteData4 = gfxItem5->prevFrame.spriteId;
				for (int n = 0; n < gfxItem5->updRectsCount; ++n)
					seqDrawAviFrame(spriteData4, &gfxItem5->prevFrame, &gfxItem5->updRects[n], gfxItem5->flags & 8);
				*/
			} else {
				int resourceId = (gfxItem5->sequenceId & 0xFFFF0000) | gfxItem5->prevFrame.spriteId;
				SpriteResource *spriteResource = _vm->_spriteCache->get(resourceId);
				for (int n = 0; n < gfxItem5->updRectsCount; ++n)
					seqDrawSpriteFrame(spriteResource, gfxItem5->prevFrame, &gfxItem5->updRects[n]);
				_vm->_spriteCache->release(resourceId);
			}
		}
	}

	debug(1, "GameSys::drawSprites() OK");

}

void GameSys::updateRect(const Common::Rect &r) {
	debug(1, "GameSys::updateRect() %d, %d, %d, %d [%d, %d]", r.left, r.top, r.right, r.bottom, r.width(), r.height());
	if (r.width() > 0 && r.height() > 0) {
		byte *pixels = (byte*)_frontSurface->getBasePtr(r.left, r.top);
		_vm->_system->copyRectToScreen(pixels, _frontSurface->pitch, r.left, r.top,
			r.width(), r.height());
	}
}

void GameSys::updateScreen() {

	debug(1, "GameSys::updateScreen()");

	for (uint i = 0; i < _dirtyRects.size(); ++i)
		updateRect(_dirtyRects[i]);

	if (_dirtyRects.size() > 0) {
		_dirtyRects.clear();
		_lastUpdateClock = 0;
		_gameSysClock = 0;
	}

	Common::Rect dstRect, srcRect, rcSrc2;

	for (int j = 0; j < _gfxItemsCount; ++j) {

		GfxItem *gfxItem = &_gfxItems[j];

		if (!gfxItem->updFlag)
			continue;

		if (gfxItem->prevFrame.spriteId == -1 ||
			!intersectRect(srcRect, _screenRect, gfxItem->prevFrame.rect)) {
			if (gfxItem->currFrame.spriteId != -1 && intersectRect(rcSrc2, _screenRect, gfxItem->currFrame.rect))
				updateRect(rcSrc2);
		} else if (gfxItem->currFrame.spriteId != -1 &&
			intersectRect(rcSrc2, _screenRect, gfxItem->currFrame.rect)) {
			updateRect(srcRect);
			updateRect(rcSrc2);
		}
		gfxItem->prevFrame = gfxItem->currFrame;		
	}
	
	updateRect(Common::Rect(0, 0, 800, 600));

	debug(1, "GameSys::updateScreen() OK");

}

void GameSys::handleReqRemoveSequenceItem() {
	if (_reqRemoveSequenceItem) {
		int gfxIndex2;
		_reqRemoveSequenceItem = false;
		if (seqFind(_removeSequenceItemSequenceId, _removeSequenceItemValue, &gfxIndex2))
			_seqItems.remove_at(gfxIndex2);
		if (seqLocateGfx(_removeSequenceItemSequenceId, _removeSequenceItemValue, &gfxIndex2)) {
			int gfxIndex2a = gfxIndex2;
			for (GfxItem *gfxItem = &_gfxItems[gfxIndex2a];
				gfxIndex2a < _gfxItemsCount && gfxItem->sequenceId == _removeSequenceItemSequenceId && gfxItem->id == _removeSequenceItemValue;
				gfxItem = &_gfxItems[gfxIndex2a])
				++gfxIndex2a;
			_gfxItemsCount -= gfxIndex2a - gfxIndex2;
			if (_gfxItemsCount != gfxIndex2)
				memcpy(&_gfxItems[gfxIndex2], &_gfxItems[gfxIndex2a], sizeof(GfxItem) * (_gfxItemsCount - gfxIndex2));
		}
	}
}

void GameSys::handleReqRemoveSequenceItems() {
	if (_removeSequenceItemsCount > 0) {
		for (int i = 0; i < _removeSequenceItemsCount; ++i) {
			int gfxIndex;
			if (seqFind(_removeSequenceItems[i]._sequenceId, _removeSequenceItems[i]._id, &gfxIndex))
				_seqItems.remove_at(gfxIndex);
			seqLocateGfx(_removeSequenceItems[i]._sequenceId, _removeSequenceItems[i]._id, &gfxIndex);
			for (GfxItem *gfxItem = &_gfxItems[gfxIndex];
				gfxIndex < _gfxItemsCount && gfxItem->sequenceId == _removeSequenceItems[i]._sequenceId && gfxItem->id == _removeSequenceItems[i]._id;
				gfxItem = &_gfxItems[gfxIndex]) {
				gfxItem->sequenceId = -1;
				gfxItem->animation = 0;
				gfxItem->soundValue = 0;
				if (_removeSequenceItems[i]._forceFrameReset) {
					gfxItem->currFrame.duration = 0;
					gfxItem->currFrame.spriteId = -1;
					gfxItem->currFrame.soundId = -1;
					gfxItem->currFrame.unkValue = -1;
					gfxItem->updFlag = true;
				} else {
					gfxItem->updFlag = false;
				}
				++gfxIndex;
			}
		}
		_removeSequenceItemsCount = 0;
	}
}

void GameSys::handleReqRemoveSpriteDrawItems() {
	if (_removeSpriteDrawItemsCount > 0) {
		for (int j = 0; j < _removeSpriteDrawItemsCount; ++j) {
			for (int i = 0; i < _gfxItemsCount; ++i) {
				GfxItem *gfxItem = &_gfxItems[i];
				if (gfxItem->sequenceId == -1 && !gfxItem->animation && (gfxItem->flags & 1) &&
					gfxItem->id == _removeSpriteDrawItems[j]._id && _removeSpriteDrawItems[j].surface == gfxItem->surface) {
					gfxItem->flags = 0;
					gfxItem->currFrame.duration = 0;
					gfxItem->currFrame.spriteId = -1;
					gfxItem->currFrame.soundId = -1;
					gfxItem->currFrame.unkValue = -1;
					gfxItem->updFlag = true;
				}
			}
		}
		_removeSpriteDrawItemsCount = 0;
	}
}

void GameSys::fatUpdateFrame() {

	debug(1, "GameSys::fatUpdateFrame()");
	
	int32 clockDelta = _gameSysClock - _lastUpdateClock;
	_lastUpdateClock = _gameSysClock;
	
	debug(1, "GameSys::fatUpdateFrame() clockDelta: %d", clockDelta);
	
	if (clockDelta <= 0)
		return;
	
	bool updFlag = true;
	int duration, currFrameNum;

	// NOTE Skipped avi code (reqAviStart)

	for (int i = 0; i < _gfxItemsCount; ++i) {
		GfxItem *gfxItem = &_gfxItems[i];
		SequenceAnimation *animation = gfxItem->animation;
		if ((gfxItem->sequenceId != -1 && animation) || (gfxItem->flags & 2) ||
			gfxItem->prevFrame.spriteId != -1 || gfxItem->prevFrame.duration > 0) {
			if (gfxItem->sequenceId == -1 || gfxItem->updFlag) {
				if ((gfxItem->flags & 2) && !gfxItem->updFlag) {
					// NOTE Skipped avi code
				}
			} else {
				Sequence *seqItem = seqFind(gfxItem->sequenceId, gfxItem->id, 0);
				if (!animation) {
					updFlag = false;
					gfxItem->sequenceId = -1;
					gfxItem->animation = 0;
					gfxItem->currFrame.duration = 0;
					gfxItem->currFrame.spriteId = -1;
					gfxItem->currFrame.soundId = -1;
					gfxItem->currFrame.unkValue = -1;
					gfxItem->updFlag = true;
				} else if (!seqItem) {
					updFlag = false;
					gfxItem->animation = 0;
					gfxItem->currFrame.duration = 0;
					gfxItem->currFrame.spriteId = -1;
					gfxItem->currFrame.soundId = -1;
					gfxItem->currFrame.unkValue = -1;
					gfxItem->updFlag = true;
				} else if ((seqItem->_flags & 4) && clockDelta > 1) {
					updFlag = false;
					if (gfxItem->delayTicks < clockDelta) {
						duration = clockDelta - gfxItem->delayTicks;
						gfxItem->delayTicks = 0;
						if (gfxItem->prevFrame.duration <= duration)
							gfxItem->prevFrame.duration = 1;
						else
							gfxItem->prevFrame.duration -= duration;
					} else {
						gfxItem->delayTicks -= clockDelta;
					}
					gfxItem->updFlag = false;
				} else if (gfxItem->delayTicks < clockDelta) {
					duration = clockDelta - gfxItem->delayTicks;
					gfxItem->delayTicks = 0;
					if (gfxItem->prevFrame.duration <= duration) {
						bool v20 = false;
						if (gfxItem->prevFrame.duration > 0) {
							duration -= gfxItem->prevFrame.duration;
							gfxItem->prevFrame.duration = -duration;
						} else {
							gfxItem->prevFrame.duration = 0;
							v20 = true;
						}
						currFrameNum = gfxItem->currFrameNum;
						if (animation->framesCount > currFrameNum) {
							updFlag = false;
							while (animation->framesCount > currFrameNum &&
								animation->frames[currFrameNum].duration <= duration) {
								if (animation->frames[currFrameNum].soundId != -1)
									_soundIds.push_back((gfxItem->sequenceId & 0xFFFF0000) | animation->frames[currFrameNum].soundId);
									duration -= animation->frames[currFrameNum].duration;
									++currFrameNum;
							}
							if (animation->framesCount > currFrameNum)
								gfxItem->currFrame = animation->frames[currFrameNum++];
							else
								gfxItem->currFrame = animation->frames[currFrameNum - 1];
							if (gfxItem->currFrame.spriteId != -1 && (seqItem->_x != 0 || seqItem->_y != 0))
								gfxItem->currFrame.rect.translate(seqItem->_x, seqItem->_y);
							// Update sprite scaling										
							if ((seqItem->_flags & 1) && gfxItem->currFrame.rect.bottom >= _backgroundImageValue1 && gfxItem->currFrame.rect.bottom <= _backgroundImageValue3) {
								int v17 = _backgroundImageValue2 + (gfxItem->currFrame.rect.bottom - _backgroundImageValue1) *
									(_backgroundImageValue4 - _backgroundImageValue2) /
									(_backgroundImageValue3 - _backgroundImageValue1);
								gfxItem->currFrame.rect.top = gfxItem->currFrame.rect.bottom - v17 * (gfxItem->currFrame.rect.bottom - gfxItem->currFrame.rect.top) / 1000;
								gfxItem->currFrame.rect.right = v17 * (gfxItem->currFrame.rect.right - gfxItem->currFrame.rect.left) / 1000 + gfxItem->currFrame.rect.left;
								gfxItem->currFrame.isScaled = 1;
							}
							gfxItem->currFrame.duration -= duration;
							if (gfxItem->currFrame.soundId != -1)
								_soundIds.push_back((gfxItem->sequenceId & 0xFFFF0000) | gfxItem->currFrame.soundId);
							gfxItem->currFrameNum = currFrameNum;
							gfxItem->updFlag = true;
						} else if (v20 && gfxItem->prevFrame.spriteId == -1) {
							--_gfxItemsCount;
							if (_gfxItemsCount != i)
								memcpy(&_gfxItems[i], &_gfxItems[i + 1], sizeof(GfxItem) * (_gfxItemsCount - i));
							--i;
						} else {
							gfxItem->updFlag = false;
						}
					} else {
						updFlag = false;
						gfxItem->prevFrame.duration -= duration;
						gfxItem->updFlag = false;
					}
				} else {
					updFlag = false;
					gfxItem->delayTicks -= clockDelta;
					gfxItem->updFlag = false;
				}
			}
		} else {
			--_gfxItemsCount;
			if (_gfxItemsCount != i)
				memcpy(&_gfxItems[i], &_gfxItems[i + 1], sizeof(GfxItem) * (_gfxItemsCount - i));
			--i;
		}
	}

	if (_newSpriteDrawItemsCount > 0) {
		debug(0, "_newSpriteDrawItemsCount: %d", _newSpriteDrawItemsCount);
		for (int k = 0; k < _newSpriteDrawItemsCount; ++k) {
			if (_gfxItemsCount < 50) {
				int insertIndex;
				seqLocateGfx(-1, _newSpriteDrawItems[k]._id, &insertIndex);
				if (_gfxItemsCount != insertIndex)
					memcpy(&_gfxItems[insertIndex + 1], &_gfxItems[insertIndex], sizeof(GfxItem) * (_gfxItemsCount - insertIndex));
				++_gfxItemsCount;
				GfxItem *gfxItem = &_gfxItems[insertIndex];
				gfxItem->sequenceId = -1;
				gfxItem->id = _newSpriteDrawItems[k]._id;
				gfxItem->animation = 0;
				gfxItem->currFrameNum = 0;
				gfxItem->flags = 1;
				gfxItem->delayTicks = 0;
				gfxItem->updFlag = true;
				gfxItem->updRectsCount = 0;
				gfxItem->surface = _newSpriteDrawItems[k]._surface;
				gfxItem->prevFrame.duration = 0;
				gfxItem->prevFrame.spriteId = -1;
				gfxItem->prevFrame.soundId = -1;
				gfxItem->prevFrame.unkValue = -1;
				gfxItem->currFrame.duration = 0;
				gfxItem->currFrame.isScaled = 0;
				gfxItem->currFrame.rect = _newSpriteDrawItems[k]._rect;
				gfxItem->currFrame.spriteId = _newSpriteDrawItems[k]._surface ? 0xCAFEBABE : -1;// TODO
				gfxItem->currFrame.soundId = -1;
				gfxItem->currFrame.unkValue = -1;
			}
		}
		_newSpriteDrawItemsCount = 0;
	}
	
	if (_grabSpriteChanged) {
		for (int i = 0; i < _gfxItemsCount; ++i) {
			GfxItem *gfxItem = &_gfxItems[i];
			if (gfxItem->sequenceId == -1 && !gfxItem->animation && (gfxItem->flags & 1) &&
				gfxItem->id == _grabSpriteId && gfxItem->surface == _grabSpriteSurface1) {
					gfxItem->currFrame.duration = 0;
					gfxItem->currFrame.isScaled = 0;
					gfxItem->currFrame.rect = _grabSpriteRect;
					gfxItem->currFrame.spriteId = _grabSpriteSurface2 ? 1 : -1;// TODO
					gfxItem->currFrame.soundId = -1;
					gfxItem->currFrame.unkValue = -1;
					gfxItem->updFlag = true;
					gfxItem->surface = _grabSpriteSurface2;
					break;
				}
		}
		_grabSpriteChanged = false;
	}
	
	// NOTE Skipped avi code (reqAviStart)

	debug(1, "GameSys::fatUpdateFrame() _fatSequenceItems.size(): %d", _fatSequenceItems.size());

	for (uint i = 0; i < _fatSequenceItems.size(); ++i) {
		Sequence *seqItem = &_fatSequenceItems[i];
		if (((seqItem->_flags & 8) || (seqItem->_flags & 0x20)) && seqItem->_sequenceId2 != -1) {
			duration = 0;
			if (((seqItem->_flags & 0x20) && seqLocateGfx(seqItem->_sequenceId2, seqItem->_id2, 0)) ||
				updateSequenceDuration(seqItem->_sequenceId2, seqItem->_id2, &duration)) {
				int index = -1;
				bool found = false;
				if (seqItem->_sequenceId2 == seqItem->_sequenceId	&& seqItem->_id == seqItem->_id2	&&
					seqFind(seqItem->_sequenceId, seqItem->_id, &index)) {
					_seqItems[index] = *seqItem;
					found = true;
				} else if (_seqItems.size() < 50) {
					index = _seqItems.size();
					_seqItems.push_back(*seqItem);
					found = true;
				}
				if (found) {
					updFlag = false;
					seqRemoveGfx(seqItem->_sequenceId2, seqItem->_id2);
					seqRemoveGfx(seqItem->_sequenceId, seqItem->_id);
					_fatSequenceItems.remove_at(i);
					--i;
					seqInsertGfx(index, duration);
				}
			}
		} else {
			updFlag = false;
			if (seqItem->_totalDuration < clockDelta) {
				int index;
				bool found = false;
				duration = clockDelta - seqItem->_totalDuration;
				seqItem->_totalDuration = 0;
				if (seqFind(seqItem->_sequenceId, seqItem->_id, &index)) {
					_seqItems[index] = *seqItem;
					found = true;
				} else if (_seqItems.size() < 50) {
					index = _seqItems.size();
					_seqItems.push_back(*seqItem);
					found = true;
				}
				if (found) {
					seqRemoveGfx(seqItem->_sequenceId, seqItem->_id);
					_fatSequenceItems.remove_at(i);
					--i;
					seqInsertGfx(index, duration - 1);
				}
			} else {
				seqItem->_totalDuration -= clockDelta;
			}
		}
	}

	debug(1, "GameSys::fatUpdateFrame() _seqItems.size(): %d", _seqItems.size());

	for (uint i = 0; i < _seqItems.size(); ++i) {
		Sequence *seqItem = &_seqItems[i];
		if (seqLocateGfx(seqItem->_sequenceId, seqItem->_id, 0)) {
			updateAnimationsStatus(seqItem->_sequenceId, seqItem->_id);
			if (seqItem->_flags & 2) {
				int gfxDuration;
				updFlag = false;
				if (updateSequenceDuration(seqItem->_sequenceId, seqItem->_id, &gfxDuration)) {
					seqRemoveGfx(seqItem->_sequenceId, seqItem->_id);
					seqInsertGfx(i, gfxDuration);
				}
			}
		} else {
			_seqItems.remove_at(i);
			--i;
		}
	}

}

void GameSys::fatUpdate() {

	debug(1, "GameSys::fatUpdate() _gfxItemsCount: %d", _gfxItemsCount);

	for (int i = 0; i < _gfxItemsCount; ++i) {
		_gfxItems[i].updFlag = false;
		_gfxItems[i].updRectsCount = 0;
	}
	
	handleReqRemoveSequenceItem();
	handleReqRemoveSequenceItems();
	handleReqRemoveSpriteDrawItems();
	// NOTE Skipped avi stuff (reqAviStop)
	
	fatUpdateFrame();

}

void GameSys::updatePlaySounds() {
	for (uint i = 0; i < _soundIds.size(); ++i)
		_vm->playSound(_soundIds[i], false);
	_soundIds.clear();
}

bool intersectRect(Common::Rect &intersectingRect, const Common::Rect &r1, const Common::Rect &r2) {
	if (r1.intersects(r2)) {
		intersectingRect = r1.findIntersectingRect(r2);
		return true;
	} else
		return false;
}

} // End of namespace Gnap
