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

TeScrollingLayout::TeScrollingLayout() : _contentLayout(nullptr),
	_enclose(true), _mouseControl(true), _autoScrollLoop(-1), _autoScrollDelay(1500),
	_autoScrollAnimation1Enabled(true), _autoScrollAnimation2Enabled(true),
	_autoScrollAnimation1Speed(0.1), _autoScrollAnimation2Speed(0.1),
	_autoScrollAnimation1Delay(1000), _autoScrollAnimation2Delay(1000)
{
}

void TeScrollingLayout::setContentLayout(TeLayout *layout) {
	if (_contentLayout) {
		removeChild(_contentLayout);
	}
	_contentLayout = layout;
	if (layout) {
		_contentLayoutUserPos = layout->userPosition();
		// TODO: original seems to call addChildBefore(layout, this) which doesn't make sense?
		addChild(_contentLayout);
	}
}

void TeScrollingLayout::resetScrollPosition() {
	if (!_contentLayout)
		return;
	warning("TODO: Implement TeScrollingLayout::resetScrollPosition");
}

void TeScrollingLayout::playAutoScroll() {
	warning("TODO: Implement TeScrollingLayout::playAutoScroll");
}

// TODO: Add more functions here.

} // end namespace Tetraedge
