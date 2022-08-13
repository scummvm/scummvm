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

#ifndef TETRAEDGE_TE_TE_LIGHT_H
#define TETRAEDGE_TE_TE_LIGHT_H

#include "tetraedge/te/te_vector3f32.h"
#include "tetraedge/te/te_vector2f32.h"
#include "tetraedge/te/te_color.h"

namespace Tetraedge {

enum TeLightType {
	Type0 = 0,
	Type1 = 1,
	Type2 = 2  // uses quadratic attenuation?
};

class TeCamera;

class TeLight {
public:
	TeLight();

	TeVector3f32 directionVector() const;
	void disable(uint lightno);
	void enable(uint lightno);
	static void enableAll();
	static void disableAll();

	void draw(TeCamera &camera);

	void transformDirPoint(TeVector3f32 &pt1,TeVector3f32 &pt2);
	void transformSpotPoint(TeVector3f32 &pt1);

	void update(uint lightno);
	void updateGlobal();

private:
	TeVector3f32 _position3d;
	TeVector2f32 _positionRadial;

	TeColor col1;
	TeColor col2;
	TeColor col3;
	// TODO add private members

	enum TeLightType _type;

	static TeColor _globalAmbientColor;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_LIGHT_H
