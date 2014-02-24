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
#include "mads/scene.h"

namespace MADS {

Scene::Scene() {
	_sectionNum = 1;
	_sectionNum2 = -1;
	_priorSectionNum = 0;
	_priorSceneId = 0;
	_nextSceneId = 0;
	_currentSceneId = 0;
}

void Scene::clearSprites(bool flag) {
	for (int i = 0; i < TEXT_DISPLAY_COUNT; ++i)
		_textDisplay[i]._active = false;

	if (flag)
		_spriteList.clear();

	_spriteSlots.clear();
	_spriteSlots.push_back(SpriteSlot(ST_FULL_SCREEN_REFRESH, -1));
}

/*------------------------------------------------------------------------*/

SpriteSlot::SpriteSlot() {
	_spriteType = ST_NONE;
	_seqIndex = 0;
	_spriteListIndex = 0;
	_frameNumber = 0;
	_depth = 0;
	_scale = 0;
}

SpriteSlot::SpriteSlot(SpriteType type, int seqIndex) {
	_spriteType = type;
	_seqIndex = seqIndex;
	_spriteListIndex = 0;
	_frameNumber = 0;
	_depth = 0;
	_scale = 0;
}

/*------------------------------------------------------------------------*/

TextDisplay::TextDisplay() {
	_active = false;
	_spacing = 0;
	_expire = 0;
	_col1 = _col2 = 0;
}

} // End of namespace MADS
