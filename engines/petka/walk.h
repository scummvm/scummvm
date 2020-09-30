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

#ifndef PETKA_WALK_H
#define PETKA_WALK_H

#include "common/rect.h"

namespace Petka {

struct Point {
	int x;
	int y;

	Point() : x(0), y(0) {}
	Point(int x_, int y_) : x(x_), y(y_) {}
	Point(Common::Point p) : x(p.x), y(p.y) {}
	bool operator==(const Point &p) { return x == p.x && y == p.y; }
	bool operator!=(const Point &p) { return !(*this == p); }
};

struct DBLPoint {
	double x;
	double y;

	DBLPoint() : x(0.0), y(0.0) {}
	DBLPoint(double x1, double y1) : x(x1), y(y1) {}
	DBLPoint(Point p) : x(p.x), y(p.y) {}

	bool operator==(const DBLPoint &p) { return x == p.x && y == p.y; }
};

class Walk {
public:
	Walk(int id);
	~Walk();

	void init(Point start, Point end);

	void clearBackground();
	void setBackground(Common::String name);

	void reset();

	Common::Point currPos();

	int getSpriteId();

	int commonPoint(int, int);

	int readWayFile(const Common::String &name, int **, int **);

	int sub_422EA0(Point p1, Point p2);
	int sub_423350();
	Common::Point sub_4234B0();
	bool sub_423570(int, int);

	int sub_423600(Point p);
	int sub_423970(int, int);
	int sub_423A30(int, int);

	static double angle(Point p1, Point p2, Point p3);
	static double angle(DBLPoint p1, DBLPoint p2, DBLPoint p3);

	static int sub_423E00(Point p1, Point p2, Point p3, Point p4, Point &p5);

	bool sub_424160(Point *p1, Point *p2);
	bool sub_424230(Point *p1);
	int sub_424320(Point *p1, Point *p2);

	int moveInside(Point *p);
	DBLPoint sub_424610(Point p1, int x, int y, Point p4);

public:
	char field_4;
	char field_5;
	char field_6;
	char field_7;
	int *_leg1[8];
	int *_leg2[8];
	int *_off1[8];
	int *_off2[8];
	int *_offleg1[8];
	int *_offleg2[8];
	int *_bkg3_4;
	int *_bkg3_5;
	double field_D0[8];
	int waysSizes[8];
	Point *_bkg3_3;
	int field_134;
	double field_138;
	double field_140;
	int resId;
	int field_14C;
	double field_150;
	double field_158;
	double currX;
	double currY;
	double field_170;
	double field_178;
	char field_180;
	char field_181;
	char field_182;
	char field_183;
	char field_184;
	char field_185;
	char field_186;
	char field_187;
	char field_188;
	char field_189;
	char field_18A;
	char field_18B;
	char field_18C;
	char field_18D;
	char field_18E;
	char field_18F;
	int field_190;
	int field_194;
	double field_198;
	int destX;
	int destY;
	int _bkg1Count;
	Point *_bkg1;
	int _bkg2Count;
	int _bkg3Count;
	int *_bkg3_1;
	int **_bkg3_2;
	Point *_bkg2;
	char field_1C4;
	char field_1C5;
	char field_1C6;
	char field_1C7;
};

} // End of namespace Petka

#endif