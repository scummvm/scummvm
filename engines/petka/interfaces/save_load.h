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

#ifndef PETKA_SAVE_LOAD_H
#define PETKA_SAVE_LOAD_H

#include "petka/interfaces/interface.h"

namespace Petka {

enum {
	kSaveMode,
	kLoadMode
};

class InterfaceSaveLoad : public SubInterface {
public:
	InterfaceSaveLoad();

	void start(int id) override;

	bool loadMode() { return _loadMode; }

	void onLeftButtonDown(Common::Point p) override;
	void onRightButtonDown(Common::Point p) override;
	void onMouseMove(Common::Point p) override;

	static void saveScreen();

private:
	int findSaveLoadRectIndex(Common::Point p);

private:
	bool _loadMode;
	uint _page;
	Common::Rect _saveRects[6];
	Common::Rect _nextPageRect;
	Common::Rect _prevPageRect;
};

} // End of namespace Petka

#endif
