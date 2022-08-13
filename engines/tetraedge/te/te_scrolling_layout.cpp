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

#include "tetraedge/te/te_scrolling_layout.h"

namespace Tetraedge {

TeScrollingLayout::TeScrollingLayout() : _autoScrollDelay(0), _contentLayout(nullptr) {
}

void TeScrollingLayout::setContentLayout(TeLayout *layout) {
	if (_contentLayout) {
		removeChild(_contentLayout);
	}
	_contentLayout = layout;
	if (layout) {
		_contentLayoutUserPos = layout->userPosition();
		// TODO: original seems to call addChildBefore(layout this) which doesn't make sense?
		addChild(_contentLayout);
	}
}


// TODO: Add more functions here.

} // end namespace Tetraedge
