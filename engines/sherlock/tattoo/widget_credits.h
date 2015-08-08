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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SHERLOCK_TATTOO_CREDITS_H
#define SHERLOCK_TATTOO_CREDITS_H

#include "common/array.h"
#include "common/rect.h"

namespace Sherlock {

	class SherlockEngine;

namespace Tattoo {

struct CreditLine {
	Common::Point _position;
	int _xOffset;
	int _width;
	Common::String _line, _line2;

	CreditLine(const Common::String &line, const Common::Point &pt, int width) :
		_line(line), _position(pt), _width(width), _xOffset(0) {}
};

class WidgetCredits {
private:
	SherlockEngine *_vm;
	Common::Array<CreditLine> _creditLines;
	int _creditSpeed;
	bool _creditsActive;
public:
	WidgetCredits(SherlockEngine *vm);

	/**
	 * Returns true if the credits are active
	 */
	bool active() const { return _creditsActive; }

	/**
	 * Initialize and load credit data for display
	 */
	void initCredits();

	/**
	 * Closes down credits display
	 */
	void close();

	/**
	 * Draw credits on the screen
	 */
	void drawCredits();

	/**
	 * Blit the drawn credits to the screen
	 */
	void blitCredits();

	/**
	 * Erase any area of the screen covered by credits
	 */
	void eraseCredits();
};

} // End of namespace Tattoo

} // End of namespace Sherlock

#endif
