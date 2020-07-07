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

#ifndef PETKA_OBJECT_BG_H
#define PETKA_OBJECT_BG_H

#include "petka/objects/object.h"

namespace Petka {

// Linear Interpolation
struct Perspective {
	double f0;
	double k; // seems to be (f1 - f0) always in files
	int y0;
	int y1;
	double f1;
};

class QObjectBG : public QMessageObject {
public:
	QObjectBG();
	void processMessage(const QMessage &msg) override;
	void draw() override;
	void goTo();
	void setEntrance(const Common::String &name);
	void play(int id, int type) override {}

	void readInisData(Common::INIFile &names, Common::INIFile &cast, Common::INIFile *bgs) override;

public:
	bool _showMap;
	int _fxId;
	int _musicId;
	Perspective _persp;
};

} // End of namespace Petka

#endif
