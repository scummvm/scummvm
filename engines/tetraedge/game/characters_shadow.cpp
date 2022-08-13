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

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/characters_shadow.h"
#include "tetraedge/te/te_renderer.h"

namespace Tetraedge {

CharactersShadow::CharactersShadow() {
}

void CharactersShadow::create(InGameScene *scene) {
	error("TODO: Implement me");
}

void CharactersShadow::createTexture(InGameScene *scene) {
	error("TODO: Implement me");
}

void CharactersShadow::destroy() {
	TeRenderer *renderer = g_engine->getRenderer();
	renderer->disableTexture();
	//glBindTexture(GL_TEXTURE_2D, 0);
	//glDeleteTextures(1, (uint *)this);
	error("TODO: Finish implementation here");
}

void CharactersShadow::draw(InGameScene *scene) {
	error("TODO: Implement me");
}

} // end namespace Tetraedge
