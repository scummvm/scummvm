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

#include "mtropolis/plugin/spqr.h"
#include "mtropolis/plugins.h"

namespace MTropolis {

namespace SPQR {

FadeModifier::FadeModifier() {
}

FadeModifier::~FadeModifier() {
}

bool FadeModifier::load(const PlugInModifierLoaderContext &context, const Data::SPQR::FadeModifier &data) {
	return true;
}


void FadeModifier::disable(Runtime *runtime) {
}

Common::SharedPtr<Modifier> FadeModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new FadeModifier(*this));
}

const char *FadeModifier::getDefaultName() const {
	return "Fade Modifier"; // ???
}

SPQRPlugIn::SPQRPlugIn()
	: _fadeModifierFactory(this) {
}

void SPQRPlugIn::registerModifiers(IPlugInModifierRegistrar *registrar) const {
	registrar->registerPlugInModifier("fade", &_fadeModifierFactory);
}

} // namespace SPQR

namespace PlugIns {

Common::SharedPtr<PlugIn> createSPQR() {
	return Common::SharedPtr<PlugIn>(new SPQR::SPQRPlugIn());
}

} // End of namespace PlugIns

} // End of namespace MTropolis
