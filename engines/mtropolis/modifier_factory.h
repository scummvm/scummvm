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
	explicit ModifierLoaderContext(ChildLoaderStack *childLoaderStack);

	ChildLoaderStack *childLoaderStack;
};

struct IModifierFactory {
	virtual Common::SharedPtr<Modifier> createModifier(ModifierLoaderContext &context, const Data::DataObject &dataObject) const = 0;
};

struct IPlugInModifierFactory {
	virtual Common::SharedPtr<Modifier> createModifier(ModifierLoaderContext &context, const Data::PlugInModifier &plugInModifierData) const = 0;
};

struct IPlugInModifierFactoryAndDataFactory : public IPlugInModifierFactory, public Data::IPlugInModifierDataFactory {
};

// Helper classes for plug-in modifier loaders
struct PlugInModifierLoaderContext {
	PlugInModifierLoaderContext(ModifierLoaderContext &modifierLoaderContext, const Data::PlugInModifier &plugInModifierData, PlugIn *plugIn);

	ModifierLoaderContext &modifierLoaderContext;
	const Data::PlugInModifier &plugInModifierData;
	PlugIn *plugIn;
};

template<typename TModifier, typename TModifierData>
class PlugInModifierFactory : public IPlugInModifierFactoryAndDataFactory {
public:
	explicit PlugInModifierFactory(PlugIn *plugIn);

	Common::SharedPtr<Modifier> createModifier(ModifierLoaderContext &context, const Data::PlugInModifier &plugInModifierData) const override;
	Common::SharedPtr<Data::PlugInModifierData> createModifierData() const override;
	PlugIn &getPlugIn() const override;

private:
	PlugIn &_plugIn;
};

template<typename TModifier, typename TModifierData>
PlugInModifierFactory<TModifier, TModifierData>::PlugInModifierFactory(PlugIn *plugIn) : _plugIn(*plugIn) {
}

template<typename TModifier, typename TModifierData>
Common::SharedPtr<Modifier> PlugInModifierFactory<TModifier, TModifierData>::createModifier(ModifierLoaderContext &context, const Data::PlugInModifier &plugInModifierData) const {
	Common::SharedPtr<TModifier> modifier(new TModifier());

	PlugInModifierLoaderContext plugInContext(context, plugInModifierData, &_plugIn);

	Modifier *downcastMod = static_cast<Modifier *>(modifier.get());
	if (!downcastMod->loadPlugInHeader(plugInContext) || !modifier->load(plugInContext, static_cast<const TModifierData &>(*plugInModifierData.plugInData.get())))
		modifier.reset();
	else {
		if (downcastMod->getName().empty())
			downcastMod->setName(downcastMod->getDefaultName());
		downcastMod->setSelfReference(modifier);
	}

	return Common::SharedPtr<Modifier>(modifier);
}

template<typename TModifier, typename TModifierData>
Common::SharedPtr<Data::PlugInModifierData> PlugInModifierFactory<TModifier, TModifierData>::createModifierData() const {
	return Common::SharedPtr<Data::PlugInModifierData>(new TModifierData());
}

template<typename TModifier, typename TModifierData>
PlugIn& PlugInModifierFactory<TModifier, TModifierData>::getPlugIn() const {
	return _plugIn;
}

IModifierFactory *getModifierFactoryForDataObjectType(Data::DataObjectTypes::DataObjectType dataObjectType);

} // End of namespace MTropolis

#endif
