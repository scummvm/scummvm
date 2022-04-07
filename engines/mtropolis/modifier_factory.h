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

#ifndef MTROPOLIS_MODIFIER_FACTORY_H
#define MTROPOLIS_MODIFIER_FACTORY_H

#include "mtropolis/data.h"
#include "mtropolis/runtime.h"

namespace MTropolis {

struct ModifierLoaderContext {
	ChildLoaderStack *childLoaderStack;
};

struct IModifierFactory {
	virtual Common::SharedPtr<Modifier> createModifier(ModifierLoaderContext &context, const Data::DataObject &dataObject) = 0;
};

IModifierFactory *getModifierFactoryForDataObjectType(Data::DataObjectTypes::DataObjectType dataObjectType);

} // End of namespace MTropolis

#endif
