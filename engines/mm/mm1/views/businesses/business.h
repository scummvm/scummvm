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

#ifndef MM1_VIEWS_BUSINESSES_BUSINESS_H
#define MM1_VIEWS_BUSINESSES_BUSINESS_H

#include "mm/mm1/views/text_view.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Businesses {

class Business : public TextView {
private:
	int _timeoutCtr;
protected:
	Common::String _modeString;
protected:
	/**
	 * Clears the bottom part of the window and
	 * displays a message
	 */
	void displayMessage(const Common::String &msg);

	/**
	 * Move text position to the next line
	 */
	void newLine();

	/**
	 * Leave the business
	 */
	void leave();

	/**
	 * Subtract gold
	 */
public:
	Business(const Common::String &name);
	virtual ~Business() {}

	/**
	 * Draws the initial display for the business
	 */
	void draw() override;

	bool tick() override;
};

} // namespace Businesses
} // namespace Views
} // namespace MM1
} // namespace MM

#endif
