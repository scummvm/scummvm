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

#ifndef TETRAEDGE_GAME_CHARACTERS_SHADOW_TINYGL_H
#define TETRAEDGE_GAME_CHARACTERS_SHADOW_TINYGL_H

#if defined(USE_TINYGL)

#include "tetraedge/game/characters_shadow.h"

namespace Tetraedge {

class InGameScene;

class CharactersShadowTinyGL : public CharactersShadow {
public:
	CharactersShadowTinyGL() {};

	void draw(InGameScene *scene) override;

protected:
	virtual void createInternal() override;
	virtual void createTextureInternal(InGameScene *scene) override;
	virtual void deleteTexture() override;

};

} // end namespace Tetraedge

#endif // USE_TINYGL

#endif // TETRAEDGE_GAME_CHARACTERS_SHADOW_TINYGL_H
