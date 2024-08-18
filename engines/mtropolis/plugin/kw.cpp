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

#include "mtropolis/plugin/kw.h"
#include "mtropolis/plugins.h"

namespace MTropolis {

namespace KW {

StrUtilModifier::StrUtilModifier() {
}

StrUtilModifier::~StrUtilModifier() {
}

bool StrUtilModifier::load(const PlugInModifierLoaderContext &context, const Data::KW::StrUtilModifier &data) {
	return true;
}

bool StrUtilModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState StrUtilModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void StrUtilModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void StrUtilModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> StrUtilModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new StrUtilModifier(*this));
}

const char *StrUtilModifier::getDefaultName() const {
	return "StrUtil Modifier"; // ???
}

KnowWonderPlugIn::KnowWonderPlugIn()
	: _strUtilModifierFactory(this) {
}

KnowWonderPlugIn::~KnowWonderPlugIn() {
}

void KnowWonderPlugIn::registerModifiers(IPlugInModifierRegistrar *registrar) const {
	registrar->registerPlugInModifier("StrUtil", &_strUtilModifierFactory);
}

} // End of namespace KW

namespace PlugIns {

Common::SharedPtr<PlugIn> createKnowWonder() {
	return Common::SharedPtr<PlugIn>(new KW::KnowWonderPlugIn());
}

} // End of namespace PlugIns

} // End of namespace MTropolis
