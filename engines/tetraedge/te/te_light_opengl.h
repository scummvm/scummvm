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

#ifndef TETRAEDGE_TE_TE_LIGHT_OPENGL_H
#define TETRAEDGE_TE_TE_LIGHT_OPENGL_H

#if defined(USE_OPENGL_GAME)

#include "tetraedge/te/te_light.h"

namespace Tetraedge {

class TeCamera;

class TeLightOpenGL : public TeLight {
public:
	TeLightOpenGL();

	void disable(uint lightno) override;
	void enable(uint lightno) override;

	void draw(TeCamera &camera) override;

	void update(uint lightno) override;
	static void updateGlobal();

	static void disableAll();
	static void enableAll();

};

} // end namespace Tetraedge

#endif // USE_OPENGL

#endif // TETRAEDGE_TE_TE_LIGHT_OPENGL_H
