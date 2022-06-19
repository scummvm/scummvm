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

ModifierLoaderContext::ModifierLoaderContext(ChildLoaderStack *mlc_childLoaderStack) : childLoaderStack(mlc_childLoaderStack) {}

PlugInModifierLoaderContext::PlugInModifierLoaderContext(ModifierLoaderContext &pmlc_modifierLoaderContext, const Data::PlugInModifier &pmlc_plugInModifierData, PlugIn *pmlc_plugIn)
	: modifierLoaderContext(pmlc_modifierLoaderContext), plugInModifierData(pmlc_plugInModifierData), plugIn(pmlc_plugIn) {
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
	else {
		Modifier *downcastMod = modifier.get();
		if (downcastMod->getName().empty())
			downcastMod->setName(downcastMod->getDefaultName());
		downcastMod->setSelfReference(modifier);
	}

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
	case Data::DataObjectTypes::kSaveAndRestoreModifier:
		return ModifierFactory<SaveAndRestoreModifier, Data::SaveAndRestoreModifier>::getInstance();
	case Data::DataObjectTypes::kAliasModifier:
		return ModifierFactory<AliasModifier, Data::AliasModifier>::getInstance();
	case Data::DataObjectTypes::kChangeSceneModifier:
		return ModifierFactory<ChangeSceneModifier, Data::ChangeSceneModifier>::getInstance();
	case Data::DataObjectTypes::kSoundEffectModifier:
		return ModifierFactory<SoundEffectModifier, Data::SoundEffectModifier>::getInstance();
	case Data::DataObjectTypes::kPathMotionModifierV2:
		return ModifierFactory<PathMotionModifierV2, Data::PathMotionModifierV2>::getInstance();
	case Data::DataObjectTypes::kDragMotionModifier:
		return ModifierFactory<DragMotionModifier, Data::DragMotionModifier>::getInstance();
	case Data::DataObjectTypes::kVectorMotionModifier:
		return ModifierFactory<VectorMotionModifier, Data::VectorMotionModifier>::getInstance();
	case Data::DataObjectTypes::kSceneTransitionModifier:
		return ModifierFactory<SceneTransitionModifier, Data::SceneTransitionModifier>::getInstance();
	case Data::DataObjectTypes::kElementTransitionModifier:
		return ModifierFactory<ElementTransitionModifier, Data::ElementTransitionModifier>::getInstance();
	case Data::DataObjectTypes::kIfMessengerModifier:
		return ModifierFactory<IfMessengerModifier, Data::IfMessengerModifier>::getInstance();
	case Data::DataObjectTypes::kTimerMessengerModifier:
		return ModifierFactory<TimerMessengerModifier, Data::TimerMessengerModifier>::getInstance();
	case Data::DataObjectTypes::kBoundaryDetectionMessengerModifier:
		return ModifierFactory<BoundaryDetectionMessengerModifier, Data::BoundaryDetectionMessengerModifier>::getInstance();
	case Data::DataObjectTypes::kCollisionDetectionMessengerModifier:
		return ModifierFactory<CollisionDetectionMessengerModifier, Data::CollisionDetectionMessengerModifier>::getInstance();
	case Data::DataObjectTypes::kKeyboardMessengerModifier:
		return ModifierFactory<KeyboardMessengerModifier, Data::KeyboardMessengerModifier>::getInstance();
	case Data::DataObjectTypes::kTextStyleModifier:
		return ModifierFactory<TextStyleModifier, Data::TextStyleModifier>::getInstance();
	case Data::DataObjectTypes::kGraphicModifier:
		return ModifierFactory<GraphicModifier, Data::GraphicModifier>::getInstance();
	case Data::DataObjectTypes::kMessengerModifier:
		return ModifierFactory<MessengerModifier, Data::MessengerModifier>::getInstance();
	case Data::DataObjectTypes::kSetModifier:
		return ModifierFactory<SetModifier, Data::SetModifier>::getInstance();
	case Data::DataObjectTypes::kCompoundVariableModifier:
		return ModifierFactory<CompoundVariableModifier, Data::CompoundVariableModifier>::getInstance();
	case Data::DataObjectTypes::kBooleanVariableModifier:
		return ModifierFactory<BooleanVariableModifier, Data::BooleanVariableModifier>::getInstance();
	case Data::DataObjectTypes::kIntegerVariableModifier:
		return ModifierFactory<IntegerVariableModifier, Data::IntegerVariableModifier>::getInstance();
	case Data::DataObjectTypes::kIntegerRangeVariableModifier:
		return ModifierFactory<IntegerRangeVariableModifier, Data::IntegerRangeVariableModifier>::getInstance();
	case Data::DataObjectTypes::kVectorVariableModifier:
		return ModifierFactory<VectorVariableModifier, Data::VectorVariableModifier>::getInstance();
	case Data::DataObjectTypes::kPointVariableModifier:
		return ModifierFactory<PointVariableModifier, Data::PointVariableModifier>::getInstance();
	case Data::DataObjectTypes::kFloatingPointVariableModifier:
		return ModifierFactory<FloatingPointVariableModifier, Data::FloatingPointVariableModifier>::getInstance();
	case Data::DataObjectTypes::kStringVariableModifier:
		return ModifierFactory<StringVariableModifier, Data::StringVariableModifier>::getInstance();

	default:
		warning("No modifier factory for type %x", static_cast<int>(dataObjectType));
		return nullptr;
	}
}

} // End of namespace MTropolis
