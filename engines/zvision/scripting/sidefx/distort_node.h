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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ZVISION_DISTORT_NODE_H
#define ZVISION_DISTORT_NODE_H

#include "zvision/scripting/sidefx.h"

namespace ZVision {

class ZVision;

class DistortNode : public SideFX {
public:
	DistortNode(ZVision *engine, uint32 key, int16 speed, float st_angl, float en_angl, float st_lin, float en_lin);
	~DistortNode();

	bool process(uint32 deltaTimeInMillis);

private:
	int16 _speed;
	float _st_angl;
	float _en_angl;
	float _st_lin;
	float _en_lin;

	float _frmSpeed;
	float _diff_angl;
	float _diff_lin;
	bool _incr;
	int16 _frames;

	float _curFrame;

	float _angle;
	float _linScale;

private:
	void setParams(float angl, float linScale);
};

} // End of namespace ZVision

#endif
