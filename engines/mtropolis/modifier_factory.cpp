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

#include "mtropolis/modifiers.h"
#include "mtropolis/modifier_factory.h"

namespace MTropolis {

ModifierLoaderContext::ModifierLoaderContext(ChildLoaderStack *childLoaderStack) : childLoaderStack(childLoaderStack) {}

PlugInModifierLoaderContext::PlugInModifierLoaderContext(ModifierLoaderContext &modifierLoaderContext, const Data::PlugInModifier &plugInModifierData, PlugIn *plugIn)
	: modifierLoaderContext(modifierLoaderContext), plugInModifierData(plugInModifierData), plugIn(plugIn) {
}

template<typename TModifier, typename TModifierData>
class ModifierFactory : public IModifierFactory {
public:
	Common::SharedPtr<Modifier> createModifier(ModifierLoaderContext &context, const Data::DataObject &dataObject) const override;
	static IModifierFactory *getInstance();

private:
	static ModifierFactory<TModifier, TModifierData> _instance;
};

template<typename TModifier, typename TModifierData>
Common::SharedPtr<Modifier> ModifierFactory<TModifier, TModifierData>::createModifier(ModifierLoaderContext &context, const Data::DataObject &dataObject) const {
	Common::SharedPtr<TModifier> modifier(new TModifier());

	if (!modifier->load(context, static_cast<const TModifierData &>(dataObject)))
		modifier.reset();

	return Common::SharedPtr<Modifier>(modifier);
}

template<typename TModifier, typename TModifierData>
IModifierFactory *ModifierFactory<TModifier, TModifierData>::getInstance() {
	return &_instance;
}

template<typename TModifier, typename TModifierData>
ModifierFactory<TModifier, TModifierData> ModifierFactory<TModifier, TModifierData>::_instance;


IModifierFactory *getModifierFactoryForDataObjectType(const Data::DataObjectTypes::DataObjectType dataObjectType) {
	switch (dataObjectType) {
	case Data::DataObjectTypes::kBehaviorModifier:
		return ModifierFactory<BehaviorModifier, Data::BehaviorModifier>::getInstance();
	case Data::DataObjectTypes::kMiniscriptModifier:
		return ModifierFactory<MiniscriptModifier, Data::MiniscriptModifier>::getInstance();
	case Data::DataObjectTypes::kIfMessengerModifier:
		return ModifierFactory<IfMessengerModifier, Data::IfMessengerModifier>::getInstance();
	case Data::DataObjectTypes::kKeyboardMessengerModifier:
		return ModifierFactory<KeyboardMessengerModifier, Data::KeyboardMessengerModifier>::getInstance();
	case Data::DataObjectTypes::kGraphicModifier:
		return ModifierFactory<GraphicModifier, Data::GraphicModifier>::getInstance();
	case Data::DataObjectTypes::kMessengerModifier:
		return ModifierFactory<MessengerModifier, Data::MessengerModifier>::getInstance();
	case Data::DataObjectTypes::kBooleanVariableModifier:
		return ModifierFactory<BooleanVariableModifier, Data::BooleanVariableModifier>::getInstance();
	case Data::DataObjectTypes::kPointVariableModifier:
		return ModifierFactory<PointVariableModifier, Data::PointVariableModifier>::getInstance();
	default:
		return nullptr;
	}
}

} // End of namespace MTropolis
