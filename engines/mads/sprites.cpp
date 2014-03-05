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
#include "mads/graphics.h"
#include "mads/msurface.h"
#include "mads/sprites.h"

namespace MADS {

enum {
	kEndOfLine   = 0,
	kEndOfSprite = 1,
	kMarker = 2
};

MSprite::MSprite(): MSurface() {
	_encoding = 0;
}

MSprite::MSprite(Common::SeekableReadStream *source, const Common::Point &offset, 
		int widthVal, int heightVal, bool decodeRle, uint8 encodingVal)
		: MSurface(widthVal, heightVal), 
		_encoding(encodingVal), _offset(offset) {

	// Load the sprite data
	loadSprite(source);
}

MSprite::~MSprite() {
}


// TODO: The sprite outlines (pixel value 0xFD) are not shown
void MSprite::loadSprite(Common::SeekableReadStream *source) {
	byte *outp, *lineStart;
	bool newLine = false;

	outp = getData();
	lineStart = getData();

	while (1) {
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
						*outp++ = (pixel == 0xFD) ? 0 : pixel;
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
						*outp++ = (pixel == 0xFD) ? 0 : pixel;
				} else {
					*outp++ = (cmd2 == 0xFD) ? 0 : cmd2;
				}
			}
		}
	}
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

/*------------------------------------------------------------------------*/

SpriteSlots::SpriteSlots(MADSEngine *vm) : _vm(vm) {
	SpriteSlot::_vm = vm;
}

void SpriteSlots::clear(bool flag) {
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

void SpriteSlots::fullRefresh(bool clearAll) {
	if (clearAll)
		Common::Array<SpriteSlot>::clear();

	push_back(SpriteSlot(ST_FULL_SCREEN_REFRESH, -1));
}

void SpriteSlots::drawBackground() {
	Scene &scene = _vm->_game->_scene;

	// Initial draw loop for any active sprites in the background
	for (uint i = 0; i < scene._spriteSlots.size(); ++i) {
		if (scene._spriteSlots[i]._spriteType >= ST_NONE) {
			scene._dirtyAreas[i]._active = false;
		}
		else {
			scene._dirtyAreas[i]._active = true;
			scene._dirtyAreas[i].setSpriteSlot(&scene._spriteSlots[i]);

			SpriteAsset *asset = scene._sprites[scene._spriteSlots[i]._spritesIndex];
			MSprite *frame = asset->getFrame(scene._spriteSlots[i]._frameNumber);

			if (scene._spriteSlots[i]._spriteType == ST_BACKGROUND) {
				Common::Point pt = scene._spriteSlots[i]._position;
				if (scene._spriteSlots[i]._scale != -1) {
					// Adjust the drawing position
					pt.x -= frame->w / 2;
					pt.y -= frame->h / 2;
				}

				if (scene._spriteSlots[i]._depth <= 1) {
					asset->draw(&scene._backgroundSurface, scene._spriteSlots[i]._frameNumber, pt);
				}
				else if (scene._depthStyle == 0) {
					asset->depthDraw(&scene._backgroundSurface, &scene._depthSurface, scene._spriteSlots[i]._frameNumber,
						pt, scene._spriteSlots[i]._depth);
				} else {
					error("Unsupported depth style");
				}
			}
		}
	}

	// Mark any remaning dirty areas as inactive
	for (uint i = scene._spriteSlots.size(); i < 50; ++i)
		scene._dirtyAreas[i]._active = false;

	// Flag any active text display
	for (uint i = 50; i < scene._textDisplay.size(); ++i) {
		TextDisplay &textDisplay = scene._textDisplay[i - 50];
		if (scene._textDisplay[i]._expire >= 0 || !textDisplay._active) {
			scene._dirtyAreas[i]._active = false;
		} else {
			scene._dirtyAreas[i]._active = true;
			scene._dirtyAreas[i].setTextDisplay(&textDisplay);
		}
	}
}

/*------------------------------------------------------------------------*/

int SpriteSets::add(SpriteAsset *asset, int idx) {
	if (!idx)
		idx = size();

	if (idx >= (int)(size() + 1))
		resize(idx + 1);
	delete (*this)[idx];
	(*this)[idx] = asset;

	return idx;
}

/*------------------------------------------------------------------------*/

} // End of namespace MADS
