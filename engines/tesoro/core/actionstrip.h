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

#ifndef TESORO_CORE_ACTIONSTRIP_H
#define TESORO_CORE_ACTIONSTRIP_H

#include "tesoro/core/interactive.h"

namespace Tesoro {

class World;

/**
 * Object kind 11: the verb action strip at the top of the screen. The interface level also holds a group of the
 * verb pictures (ICONOS), a highlight square (CUADRADO) and the pointer (CURSOR).
 */
class ActionStrip : public InteractiveObject {
public:
	void init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now, World &host);

	bool acceptsPointer() override;
	int showHoverText() override;
	int clearHoverText() override;
	int setVerbAndClick(int verb) override;

	/** The group of the verb pictures (ICONOS) and the highlight square, which move with the strip. */
	Object *group() const { return _group; }
	Object *square() const { return _square; }

protected:
	void onAttach() override;

private:
	bool isPicture(const Object *object) const;
	void movePictures(int32 dy);

	Object *_group = nullptr;		// ICONOS
	Object *_pointer = nullptr;		// CURSOR
	Object *_square = nullptr;		// CUADRADO
	int _slot = 0;
	Object *_hovered = nullptr;
};

} // End of namespace Tesoro

#endif
