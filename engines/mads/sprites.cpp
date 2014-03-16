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

#include "common/scummsys.h"
#include "engines/util.h"
#include "graphics/palette.h"
#include "mads/mads.h"
#include "mads/screen.h"
#include "mads/msurface.h"
#include "mads/sprites.h"

namespace MADS {

enum {
	kEndOfLine   = 0,
	kEndOfSprite = 1,
	kMarker = 2
};

#define TRANSPARENT_COLOR_INDEX 0xFF

class DepthEntry {
public:
	int depth;
	int index;

	DepthEntry(int depthAmt, int indexVal) { depth = depthAmt; index = indexVal; }
};

bool sortHelper(const DepthEntry &entry1, const DepthEntry &entry2) {
	return entry1.depth < entry2.depth;
}

typedef Common::List<DepthEntry> DepthList;

/*------------------------------------------------------------------------*/

MSprite::MSprite(): MSurface() {
}

MSprite::MSprite(Common::SeekableReadStream *source, const Common::Array<RGB6> &palette,
		const Common::Rect &bounds): 
		MSurface(bounds.width(), bounds.height()), 
		_offset(Common::Point(bounds.left, bounds.top)) {
	// Load the sprite data
	loadSprite(source, palette);
}

MSprite::~MSprite() {
}

void MSprite::loadSprite(Common::SeekableReadStream *source,
		const Common::Array<RGB6> &palette) {
	byte *outp, *lineStart;
	bool newLine = false;

	outp = getData();
	lineStart = getData();
	int spriteSize = this->w * this->h;
	byte transIndex = getTransparencyIndex();
	Common::fill(outp, outp + spriteSize, transIndex);

	for (;;) {
		byte cmd1, cmd2, count, pixel;

		if (newLine) {
			outp = lineStart + getWidth();
			lineStart = outp;
			newLine = false;
		}

		cmd1 = source->readByte();

		if (cmd1 == 0xFC)
			break;
		else if (cmd1 == 0xFF)
			newLine = true;
		else if (cmd1 == 0xFD) {
			while (!newLine) {
				count = source->readByte();
				if (count == 0xFF) {
					newLine = true;
				} else {
					pixel = source->readByte();
					while (count--)
						*outp++ = (pixel == 0xFD) ? getTransparencyIndex() : pixel;
				}
			}
		} else {
			while (!newLine) {
				cmd2 = source->readByte();
				if (cmd2 == 0xFF) {
					newLine = true;
				} else if (cmd2 == 0xFE) {
					count = source->readByte();
					pixel = source->readByte();
					while (count--)
						*outp++ = (pixel == 0xFD) ? getTransparencyIndex() : pixel;
				} else {
					*outp++ = (cmd2 == 0xFD) ? getTransparencyIndex() : cmd2;
				}
			}
		}
	}

	// Do a final iteration over the sprite to convert it's pixels to
	// the final positions in the main palette
	for (outp = getData(); spriteSize > 0; --spriteSize, ++outp) {
		if (*outp != transIndex)
			*outp = palette[*outp]._palIndex;
	}
}

byte MSprite::getTransparencyIndex() const {
	return TRANSPARENT_COLOR_INDEX;
}

/*------------------------------------------------------------------------*/

MADSEngine *SpriteSlot::_vm = nullptr;

SpriteSlot::SpriteSlot() {
	_spriteType = ST_NONE;
	_seqIndex = 0;
	_spritesIndex = 0;
	_frameNumber = 0;
	_depth = 0;
	_scale = 0;
}

SpriteSlot::SpriteSlot(SpriteType type, int seqIndex) {
	_spriteType = type;
	_seqIndex = seqIndex;
	_spritesIndex = 0;
	_frameNumber = 0;
	_depth = 0;
	_scale = 0;
}

bool SpriteSlot::operator==(const SpriteSlotSubset &other) const {
	return (_spritesIndex == other._spritesIndex) && (_frameNumber == other._frameNumber) &&
		(_position == other._position) && (_depth == other._depth) && 
		(_scale == other._scale);
}

void SpriteSlot::copy(const SpriteSlotSubset &other) {
	_spritesIndex = other._spritesIndex;
	_frameNumber = other._frameNumber;
	_position = other._position;
	_depth = other._depth;
	_scale = other._scale;
}

/*------------------------------------------------------------------------*/

SpriteSlots::SpriteSlots(MADSEngine *vm) : _vm(vm) {
	SpriteSlot::_vm = vm;
}

void SpriteSlots::reset(bool flag) {
	_vm->_game->_scene._textDisplay.clear();

	if (flag)
		_vm->_game->_scene._sprites.clear();

	Common::Array<SpriteSlot>::clear();
	push_back(SpriteSlot(ST_FULL_SCREEN_REFRESH, -1));
}

/**
* Releases any sprites used by the player
*/
void SpriteSlots::releasePlayerSprites() {
	Player &player = _vm->_game->_player;

	if (player._spritesLoaded && player._numSprites > 0) {
		int spriteEnd = player._spritesStart + player._numSprites - 1;
		do {
			deleteEntry(spriteEnd);
		} while (--spriteEnd >= player._spritesStart);
	}
}

void SpriteSlots::deleteEntry(int index) {
	remove_at(index);
}

void SpriteSlots::setDirtyAreas() {
	Scene &scene = _vm->_game->_scene;

	for (uint i = 0; i < size(); ++i) {
		if ((*this)[i]._spriteType >= ST_NONE) {
			scene._dirtyAreas[i].setSpriteSlot(&(*this)[i]);

			scene._dirtyAreas[i]._textActive = ((*this)[i]._spriteType <= ST_NONE) ? 0 : 1;
			(*this)[i]._spriteType = ST_NONE;
		}
	}
}

void SpriteSlots::fullRefresh(bool clearAll) {
	if (clearAll)
		Common::Array<SpriteSlot>::clear();

	push_back(SpriteSlot(ST_FULL_SCREEN_REFRESH, -1));
}

void SpriteSlots::deleteTimer(int seqIndex) {
	for (uint idx = 0; idx < size(); ++idx) {
		if ((*this)[idx]._seqIndex == seqIndex) {
			remove_at(idx);
			return;
		}
	}
}

int SpriteSlots::add() {
	SpriteSlot ss;
	push_back(ss);
	return size() - 1;
}

void SpriteSlots::drawBackground() {
	Scene &scene = _vm->_game->_scene;

	// Initial draw loop for any active sprites in the background
	for (uint i = 0; i < size(); ++i) {
		SpriteSlot &spriteSlot = (*this)[i];
		DirtyArea &dirtyArea = scene._dirtyAreas[i];

		if (spriteSlot._spriteType >= ST_NONE) {
			// Foreground sprite, so we can ignore it
			dirtyArea._active = false;
		} else {
			dirtyArea._active = true;
			dirtyArea.setSpriteSlot(&spriteSlot);

			if (spriteSlot._spriteType == ST_BACKGROUND) {
				// Background object, so need to draw it
				assert(spriteSlot._frameNumber > 0);
				SpriteAsset *asset = scene._sprites[spriteSlot._spritesIndex];
				MSprite *frame = asset->getFrame(spriteSlot._frameNumber - 1);

				Common::Point pt = spriteSlot._position;
				if (spriteSlot._scale != -1) {
					// Adjust the drawing position
					pt.x -= frame->w / 2;
					pt.y -= frame->h / 2;
				}

				if (spriteSlot._depth <= 1) {
					asset->draw(&scene._backgroundSurface, spriteSlot._frameNumber, pt);
				} else if (scene._depthStyle == 0) {
					asset->depthDraw(&scene._backgroundSurface, &scene._depthSurface, spriteSlot._frameNumber,
						pt, spriteSlot._depth);
				} else {
					error("Unsupported depth style");
				}
			}
		}
	}

	// Mark any remaning sprite slot dirty areas as inactive
	for (uint i = size(); i < SPRITE_SLOTS_MAX_SIZE; ++i)
		scene._dirtyAreas[i]._active = false;

	// Flag any active text display
	for (uint i = 0; i < scene._textDisplay.size(); ++i) {
		TextDisplay &textDisplay = scene._textDisplay[i];
		DirtyArea &dirtyArea = scene._dirtyAreas[i + SPRITE_SLOTS_MAX_SIZE];

		if (textDisplay._expire >= 0 || !textDisplay._active) {
			dirtyArea._active = false;
		} else {
			dirtyArea._active = true;
			dirtyArea.setTextDisplay(&textDisplay);
		}
	}
}

void SpriteSlots::drawForeground(MSurface *s) {
	DepthList depthList;
	Scene &scene = _vm->_game->_scene;

	// Get a list of sprite object depths for active objects
	for (uint i = 0; i < size(); ++i) {
		SpriteSlot &spriteSlot = (*this)[i];
		if (spriteSlot._spriteType >= ST_NONE) {
			DepthEntry rec(16 - spriteSlot._depth, i);
			depthList.push_back(rec);
		}
	}

	// Sort the list in order of the depth
	Common::sort(depthList.begin(), depthList.end(), sortHelper);

	// Loop through each of the objects
	DepthList::iterator i;
	for (i = depthList.begin(); i != depthList.end(); ++i) {
		DepthEntry &de = *i;
		SpriteSlot &slot = (*this)[de.index];
		assert(slot._spritesIndex < (int)scene._sprites.size());
		SpriteAsset &spriteSet = *scene._sprites[slot._spritesIndex];

		// Get the sprite frame
		int frameNumber = slot._frameNumber & 0x7fff;
		bool flipped = (slot._frameNumber & 0x8000) != 0;

		assert(frameNumber > 0);
		MSprite *sprite = spriteSet.getFrame(frameNumber - 1);

		MSurface *spr = sprite;
		if (flipped) {
			// Create a flipped copy of the sprite temporarily
			spr = sprite->flipHorizontal();
		}

		if ((slot._scale < 100) && (slot._scale != -1)) {
			// Minimalised drawing
			s->copyFrom(spr, slot._position, slot._depth, &scene._depthSurface, 
				slot._scale, sprite->getTransparencyIndex());
		} else {
			int xp, yp;

			if (slot._scale == -1) {
				xp = slot._position.x - scene._posAdjust.x;
				yp = slot._position.y - scene._posAdjust.y;
			} else {
				xp = slot._position.x - (spr->w / 2) - scene._posAdjust.x;
				yp = slot._position.y - spr->h - scene._posAdjust.y + 1;
			}

			if (slot._depth > 1) {
				// Draw the frame with depth processing
				s->copyFrom(spr, Common::Point(xp, yp), slot._depth, &scene._depthSurface, 
					100, sprite->getTransparencyIndex());
			} else {
				// No depth, so simply draw the image
				spr->copyTo(s, Common::Point(xp, yp), sprite->getTransparencyIndex());
			}
		}

		// Free sprite if it was a flipped one
		if (flipped)
			delete spr;
	}
}

void SpriteSlots::cleanUp() {
	for (int i = (int)size() - 1; i >= 0; --i) {
		if ((*this)[i]._spriteType < ST_NONE)
			remove_at(i);
	}
}

/*------------------------------------------------------------------------*/

SpriteSets::~SpriteSets() {
	clear();
}

int SpriteSets::add(SpriteAsset *asset, int idx) {
	if (!idx)
		idx = size();

	if (idx >= (int)size())
		resize(idx + 1);
	delete (*this)[idx];
	(*this)[idx] = asset;

	return idx;
}

int SpriteSets::addSprites(const Common::String &resName, int flags) {
	return add(new SpriteAsset(_vm, resName, flags));
}

void SpriteSets::clear() {
	for (uint i = 0; i < size(); ++i)
		delete (*this)[i];

	Common::Array<SpriteAsset *>::clear();
}

/*------------------------------------------------------------------------*/

ImageInterEntry::ImageInterEntry() {
	_field0 = 0;
	_field2 = 0;
	_field3 = 0;
	_field4 = 0;
	_field6 = 0;
	_field8 = 0;
}

/*------------------------------------------------------------------------*/

void ImageInterEntries::add(int v1, int v2) {
	ImageInterEntry ie;
	ie._field0 = -2;
	ie._field2 = -1;

	push_back(ie);
}

void ImageInterEntries::add(int v1, int v2, int v3, int v4) {
	assert(size() < 50);

	ImageInterEntry ie;
	ie._field0 = -3;
	ie._field2 = 201;
	ie._field6 = v1;
	ie._field8 = v2;
	ie._field4 = v3;
	ie._field3 = v4;

	push_back(ie);
}

ImageInterEntry &ImageInterEntries::add() {
	resize(size() + 1);
	return (*this)[size() - 1];
}


void ImageInterEntries::call(int v1, int v2) {
	debug("TODO: ImageInterEntries::call");
}

} // End of namespace MADS
