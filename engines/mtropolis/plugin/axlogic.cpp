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

#include "mtropolis/plugin/axlogic.h"
#include "mtropolis/plugins.h"

namespace MTropolis {

namespace AXLogic {

AlienWriterModifier::AlienWriterModifier() {
}

AlienWriterModifier::~AlienWriterModifier() {
}

bool AlienWriterModifier::load(const PlugInModifierLoaderContext &context, const Data::AXLogic::AlienWriterModifier &data) {
	return true;
}

bool AlienWriterModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState AlienWriterModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void AlienWriterModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void AlienWriterModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> AlienWriterModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new AlienWriterModifier(*this));
}

const char *AlienWriterModifier::getDefaultName() const {
	return "AlienWriter Modifier"; // ???
}

AXLogicPlugIn::AXLogicPlugIn()
	: _alienWriterModifierFactory(this) {
}

AXLogicPlugIn::~AXLogicPlugIn() {
}

void AXLogicPlugIn::registerModifiers(IPlugInModifierRegistrar *registrar) const {
	registrar->registerPlugInModifier("AlienWriter", &_alienWriterModifierFactory);
}

} // End of namespace AXLogic

namespace PlugIns {

Common::SharedPtr<PlugIn> createAXLogic() {
	return Common::SharedPtr<PlugIn>(new AXLogic::AXLogicPlugIn());
}

} // End of namespace PlugIns

} // End of namespace MTropolis
