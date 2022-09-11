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

#include "graphics/managed_surface.h"

#include "mtropolis/plugin/mti.h"
#include "mtropolis/plugins.h"

#include "mtropolis/miniscript.h"

namespace MTropolis {

namespace MTI {

PanningModifier::PanningModifier() {
}

PanningModifier::~PanningModifier() {
}

bool PanningModifier::load(const PlugInModifierLoaderContext &context, const Data::MTI::PanningModifier &data) {
	return true;
}

bool PanningModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState PanningModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void PanningModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void PanningModifier::debugInspect(IDebugInspectionReport *report) const {
}
#endif

Common::SharedPtr<Modifier> PanningModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new PanningModifier(*this));
}

const char *PanningModifier::getDefaultName() const {
	return "Panning Modifier";	// ???
}

ShanghaiModifier::ShanghaiModifier() {
}

ShanghaiModifier::~ShanghaiModifier() {
}

bool ShanghaiModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState ShanghaiModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void ShanghaiModifier::disable(Runtime *runtime) {
}

bool ShanghaiModifier::load(const PlugInModifierLoaderContext &context, const Data::MTI::ShanghaiModifier &data) {
	return true;
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void ShanghaiModifier::debugInspect(IDebugInspectionReport *report) const {
}
#endif

Common::SharedPtr<Modifier> ShanghaiModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new ShanghaiModifier(*this));
}

const char *ShanghaiModifier::getDefaultName() const {
	return "Shanghai Modifier";	// ???
}

MTIPlugIn::MTIPlugIn()
	: _panningModifierFactory(this), _shanghaiModifierFactory(this) {
}

void MTIPlugIn::registerModifiers(IPlugInModifierRegistrar *registrar) const {
	registrar->registerPlugInModifier("panning", &_panningModifierFactory);
	registrar->registerPlugInModifier("Shanghai", &_shanghaiModifierFactory);
}


} // namespace MTI

namespace PlugIns {

Common::SharedPtr<PlugIn> createMTI() {
	return Common::SharedPtr<PlugIn>(new MTI::MTIPlugIn());
}

} // End of namespace PlugIns

} // End of namespace MTropolis
