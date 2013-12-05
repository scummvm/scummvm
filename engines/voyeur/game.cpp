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

#include "voyeur/game.h"
#include "voyeur/voyeur.h"

namespace Voyeur {

IntData::IntData() {
	_field9 = false;
	_flipWait = false;
	_hasPalette = false;
	field16 = 0;
	field1A = 0;
	field1E = 0;
	field22 = 0;
	field24 = 0;
	field26 = 0;
	field2A = 0;
	field38 = 0;
	field3B = 0;
	field3D = 0;
	_palStartIndex = 0;
	_palEndIndex = 0;
	_palette = NULL;
}

void IntData::audioInit() {

}

/*------------------------------------------------------------------------*/

Game::Game() {
	_iForceDeath = -1;
}

void Game::doTransitionCard(const Common::String &time, const Common::String &location) {
	_vm->_graphicsManager.setColor(128, 16, 16, 16);
	_vm->_graphicsManager.setColor(224, 220, 220, 220);
	_vm->_eventsManager._intPtr.field38 = true;
	_vm->_eventsManager._intPtr._hasPalette = true;

	(*_vm->_graphicsManager._vPort)->setupViewPort();
	(*_vm->_graphicsManager._vPort)->fillPic(128);
	_vm->_graphicsManager.flipPage();
	_vm->_eventsManager.sWaitFlip();

	(*_vm->_graphicsManager._vPort)->_parent->_flags |= DISPFLAG_8;
	_vm->_graphicsManager.flipPage();
	_vm->_eventsManager.sWaitFlip();
	(*_vm->_graphicsManager._vPort)->fillPic(128);

	FontInfoResource &fi = *_vm->_graphicsManager._fontPtr;
	fi._curFont = _vm->_bVoy->boltEntry(257)._fontResource;
	fi._foreColor = 224;
	fi._fontSaveBack = 0;
	fi._pos = Common::Point(0, 116);
	fi._justify = ALIGN_CENTRE;
	fi._justifyWidth = 384;
	fi._justifyHeight = 120;

	(*_vm->_graphicsManager._vPort)->drawText(time);
	
	if (!location.empty()) {
		fi._pos = Common::Point(0, 138);
		fi._justify = ALIGN_CENTRE;
		fi._justifyWidth = 384;
		fi._justifyHeight = 140;

		(*_vm->_graphicsManager._vPort)->drawText(location);
	}

	(*_vm->_graphicsManager._vPort)->_parent->_flags |= DISPFLAG_8;
	_vm->_graphicsManager.flipPage();
	_vm->_eventsManager.sWaitFlip();
}

void Game::addVideoEventStart() {

}

void Game::playStamp() {

}

} // End of namespace Voyeur
