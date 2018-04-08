/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_VISUAL_VISUAL_H
#define STARK_VISUAL_VISUAL_H

#include "common/scummsys.h"

namespace Stark {

class Visual {
public:
	enum VisualType {
		kImageXMG       = 2,
		kRendered       = 3,
		kImageText      = 4,
		kSmackerStream  = 5,
		kActor          = 6,
		kSmackerFMV     = 7,
		kEffectFish     = 8,
		kEffectBubbles  = 9,
		kEffectFirefly  = 10,
		kExplodingImage = 100,
		kFlashingImage = 101
	};

	explicit Visual(VisualType type) : _type(type) {}
	virtual ~Visual() {}

	/**
	 * Returns the visual if it has the same type as the template argument
	 */
	template <class T>
	T *get();

private:
	VisualType _type;
};

template<class T>
T *Visual::get() {
	if (_type != T::TYPE) {
		return nullptr;
	}

	return (T *) this;
}

} // End of namespace Stark

#endif // STARK_VISUAL_VISUAL_H
