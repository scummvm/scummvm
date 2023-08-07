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

#ifndef M4_BURGER_ROOMS_SECTION9_ROOM904_H
#define M4_BURGER_ROOMS_SECTION9_ROOM904_H

#include "m4/burger/rooms/room.h"
#include "m4/gui/gui_dialog.h"

namespace M4 {
namespace Burger {
namespace Rooms {

class Room904 : public Rooms::Room {
private:
	TextScrn *_screen1 = nullptr;
	TextScrn *_screen2 = nullptr;
	int _numSections = 0;
	int _fontHeight = 0;
	int _totalWidth = 0;
	int _totalHeight = 0;
	int _x1 = 0, _y1 = 0, _x2 = 0, _y2 = 0;
	int _currentSection = -1;

	/**
	 * Initial credits setup
	 */
	void creditsSetup();

	/**
	 * Returns the number of sections in the credits array
	 */
	size_t getCreditsSectionsCount() const;

	/**
	 * Returns the maximum width of any credits section header
	 */
	size_t getMaxCreditsHeaderWidth() const;

	/**
	 * Text items callback
	 */
	void updateCredits(TextItem *textItem, TextScrn *textScrn);

	/**
	 * Return the starting index in the credits of a given section
	 */
	int getCreditsSectionLine(int sectionNum) const;

	/**
	 * Return the first string of a given section
	 */
	const char *getCreditsSectionString(int sectionNum) const;

	/**
	 * Returns the number of lines in a given section
	 */
	int getCreditsSectionLines(int sectionNum) const;

	/**
	 * Return the maximum width of a specific credits section
	 */
	size_t getCreditsSectionWidth(int sectionNum) const;

	/**
	 * Returns a given text line within a credits section
	 */
	const char *getLineInCreditsSection(int sectionNum, int lineNum) const;

	/**
	 * Text items callback
	 */
	static void creditsCallback(TextItem *textItem, TextScrn *textScrn);

	/**
	 * Plays a random sound
	 */
	static void playRandomSound(int trigger, int channel);

public:
	Room904() : Rooms::Room() {}
	~Room904() override {}

	void preload() override;
	void init() override;
	void shutdown() override;
	void daemon() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
