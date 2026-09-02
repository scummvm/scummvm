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

#ifndef MOHAWK_ZOOMBINI_PAGES_TRANSITION_BASE_H
#define MOHAWK_ZOOMBINI_PAGES_TRANSITION_BASE_H

#include "mohawk/zoombini_page.h"

namespace Mohawk {

/**
 * Base class for non-puzzle transition pages.
 *
 * Transition pages display the route movement or startup sequence between
 * interactive pages. They accept the common skip input, close their features
 * through the normal page lifecycle, and do not own puzzle chance state.
 */
class ZoombiniTransition : public ZoombiniPage {
public:
	/** Create a transition page for @p pageType. */
	ZoombiniTransition(MohawkEngine_Zoombini *vm, ZoombiniPageType pageType);
	/** Release transition features and page-owned resources. */
	~ZoombiniTransition() override;

	/** Stop transition playback and release its active features. */
	void close() override;

	/** Handle a click that advances or skips the transition. */
	ZmbEventHandleResult onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Handle keyboard advance/skip input for the transition. */
	ZmbEventHandleResult onKeyDown(const Common::KeyState &kbd, bool kbdRepeat) override;
};

} // End of namespace Mohawk

#endif
