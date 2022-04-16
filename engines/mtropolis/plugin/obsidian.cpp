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

#include "mtropolis/plugin/obsidian.h"
#include "mtropolis/plugins.h"

namespace MTropolis {

namespace Obsidian {

bool MovementModifier::load(const PlugInModifierLoaderContext &context, const Data::Obsidian::MovementModifier &data) {
	return true;
}

Common::SharedPtr<Modifier> MovementModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new MovementModifier(*this));
}

bool RectShiftModifier::load(const PlugInModifierLoaderContext &context, const Data::Obsidian::RectShiftModifier &data) {
	return true;
}

Common::SharedPtr<Modifier> RectShiftModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new RectShiftModifier(*this));
}

bool TextWorkModifier::load(const PlugInModifierLoaderContext &context, const Data::Obsidian::TextWorkModifier &data) {
	return true;
}

Common::SharedPtr<Modifier> TextWorkModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new TextWorkModifier(*this));
}

ObsidianPlugIn::ObsidianPlugIn() : _movementModifierFactory(this), _rectShiftModifierFactory(this), _textWorkModifierFactory(this) {
}

void ObsidianPlugIn::registerModifiers(IPlugInModifierRegistrar *registrar) const {
	registrar->registerPlugInModifier("Movement", &_movementModifierFactory);
	registrar->registerPlugInModifier("rectshift", &_rectShiftModifierFactory);
	registrar->registerPlugInModifier("TextWork", &_textWorkModifierFactory);
}

} // End of namespace ObsidianPlugIn

namespace PlugIns {

Common::SharedPtr<PlugIn> createObsidian() {
	return Common::SharedPtr<PlugIn>(new Obsidian::ObsidianPlugIn());
}

} // End of namespace PlugIns

} // End of namespace MTropolis
