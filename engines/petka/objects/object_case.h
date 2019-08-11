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

#ifndef PETKA_OBJECT_CASE_H
#define PETKA_OBJECT_CASE_H

#include "petka/objects/object.h"

namespace Petka {

class QObjectCase : public QObject {
public:
	QObjectCase();
	void update(int time) override;
	void show(bool v) override;
	bool isInPoint(int x, int y) override;
	void onMouseMove(int x, int y) override;
	void onClick(int x, int y) override;
	void fillWithItems();

	void addItem(int id);
	void removeItem(int id);
	void setInv(int id1, int id2);

private:
	Common::Array<int> _items;
	uint _clickedObjIndex;
	int _itemIndex;
};

} // End of namespace Petka

#endif
