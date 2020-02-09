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

#include "titanic/star_control/base_stars.h"
#include "common/rect.h"

#ifndef TITANIC_STAR_REF_H
#define TITANIC_STAR_REF_H

namespace Titanic {

class CCamera;
class CSurfaceArea;

class CBaseStarRef {
protected:
	CBaseStars *_stars;
public:
	CBaseStarRef(CBaseStars *stars) : _stars(stars) {}
	CBaseStarRef() : _stars(nullptr) {}
	virtual ~CBaseStarRef() {}

	void process(CSurfaceArea *surface, CCamera *camera);

	virtual bool check(const Common::Point &pt, int index) { return false; }
};

class CStarRef1 : public CBaseStarRef {
private:
	Common::Point _position;
public:
	int _index;
public:
	CStarRef1(CBaseStars *stars, const Common::Point &pt) :
		CBaseStarRef(stars), _position(pt), _index(-1) {}
	~CStarRef1() override {}

	bool check(const Common::Point &pt, int index) override;
};

class CStarRefArray : public CBaseStarRef {
private:
	Common::Array<CStarPosition> *_positions;
public:
	int _index;
public:
	CStarRefArray(CBaseStars *stars, Common::Array<CStarPosition> *positions) :
		CBaseStarRef(stars), _positions(positions), _index(0) {}
	~CStarRefArray() override {}

	bool check(const Common::Point &pt, int index) override;
};

class CStarRef3 : public CBaseStarRef {
public:
	int _index;
public:
	CStarRef3(CBaseStars *stars) :CBaseStarRef(stars), _index(0) {}
	~CStarRef3() override {}

	bool check(const Common::Point &pt, int index) override;
};

} // End of namespace Titanic

#endif /* TITANIC_STAR_REF_H */
