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

#ifndef PETKA_OBJECT_STAR_H
#define PETKA_OBJECT_STAR_H

#include "petka/objects/object.h"

namespace Petka {

class QObjectStar : public QObject {
public:
	QObjectStar();
	void update(int time) override {}
	bool isInPoint(Common::Point p) override;
	void onMouseMove(Common::Point p) override;
	void onClick(Common::Point p) override;
	void setPos(Common::Point p, bool center) override;

private:
	uint findButtonIndex(int16 x, int16 y) const;

private:
	Common::Rect _buttonRects[6];
};

} // End of namespace Petka

#endif
