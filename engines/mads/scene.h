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

#ifndef MADS_SCENE_H
#define MADS_SCENE_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"

namespace MADS {

enum SpriteType {
	ST_NONE = 0, ST_FOREGROUND = 1, ST_BACKGROUND = -4, 
	ST_FULL_SCREEN_REFRESH = -2, ST_EXPIRED = -1
};

class SpriteSlot {
public:
	SpriteType _spriteType;
	int _seqIndex;
	int _spriteListIndex;
	int _frameNumber;
	Common::Point _position;
	int _depth;
	int _scale;
public:
	SpriteSlot();
	SpriteSlot(SpriteType type, int seqIndex);
};

class TextDisplay {
public:
	bool _active;
	int _spacing;
	Common::Rect _bounds;
	int _expire;
	int _col1;
	int _col2;
	Common::String _fontName;
	Common::String _msg;

	TextDisplay();
};

#define SPRITE_COUNT 50
#define TEXT_DISPLAY_COUNT 40

class Scene {
public:
	int _priorSectionNum;
	int _sectionNum;
	int _sectionNum2;
	int _priorSceneId;
	int _nextSceneId;
	int _currentSceneId;
	TextDisplay _textDisplay[TEXT_DISPLAY_COUNT];
	Common::Array<SpriteSlot> _spriteSlots;
	Common::Array<int> _spriteList;
	int _spriteListIndex;

	/**
	 * Constructor
	 */
	Scene();

	/**
	 * Initialise the sprite data
	 * @param flag		Also reset sprite list
	 */
	void clearSprites(bool flag);
};

} // End of namespace MADS

#endif /* MADS_SCENE_H */
