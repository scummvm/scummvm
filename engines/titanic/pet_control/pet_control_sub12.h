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

#ifndef TITANIC_PET_CONTROL_SUB12_H
#define TITANIC_PET_CONTROL_SUB12_H

#include "titanic/simple_file.h"
#include "titanic/screen_manager.h"

namespace Titanic {

class CPetControlSub12 {
	struct ArrayEntry {
		CString _string1;
		CString _string2;
		CString _string3;
	};
private:
	Common::Array<ArrayEntry> _array;
	CString _lines;
	bool _stringsMerged;
	Rect _bounds;
	int _field30;
	int _lineCount;
	int _field38;
	int _field3C;
	int _field40;
	int _field44;
	int _backR;
	int _backG;
	int _backB;
	int _field54;
	int _field58;
	int _field5C;
	int _field60;
	int _field64;
	int _field68;
	int _field6C;
	bool _hasBorder;
	int _field74;
	int _field78;
	int _field7C;
private:
	void setupArrays(int count);

	void freeArrays();

	void setArrayStr2(uint idx, int val1, int val2, int val3);

	/**
	 * Merges the strings in the strings array
	 */
	void mergeStrings();
public:
	CPetControlSub12(int count = 10);

	/**
	 * Set up the control
	 */
	void setup();

	/**
	 * Load the data for the control
	 */
	void load(SimpleFile *file, int param);

	/**
	 * Set the bounds for the control
	 */
	void setBounds(const Rect &bounds) { _bounds = bounds; }

	/**
	 * Sets the flag for whether to draw a frame border around the control
	 */
	void setHasBorder(bool val) { _hasBorder = val; }

	/**
	 * Draw the control
	 */
	void draw(CScreenManager *screenManager);

};

} // End of namespace Titanic

#endif /* TITANIC_PET_CONTROL_SUB12_H */
