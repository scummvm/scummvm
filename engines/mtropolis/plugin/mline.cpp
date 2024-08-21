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

#include "mtropolis/plugin/mline.h"
#include "mtropolis/plugins.h"

namespace MTropolis {

namespace MLine {

MLineLauncherModifier::MLineLauncherModifier() {
}

MLineLauncherModifier::~MLineLauncherModifier() {
}

bool MLineLauncherModifier::load(const PlugInModifierLoaderContext &context, const Data::MLine::MLineLauncherModifier &data) {
	return true;
}

bool MLineLauncherModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState MLineLauncherModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void MLineLauncherModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void MLineLauncherModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> MLineLauncherModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new MLineLauncherModifier(*this));
}

const char *MLineLauncherModifier::getDefaultName() const {
	return "mLine Launcher Modifier"; // ???
}

MLinePlugIn::MLinePlugIn()
	: _mlineLauncherModifierFactory(this) {
}

MLinePlugIn::~MLinePlugIn() {
}

void MLinePlugIn::registerModifiers(IPlugInModifierRegistrar *registrar) const {
	//URLaunch plugin by mLine
	registrar->registerPlugInModifier("mLine_launcher", &_mlineLauncherModifierFactory);
}

} // End of namespace MLine

namespace PlugIns {

Common::SharedPtr<PlugIn> createMLine() {
	return Common::SharedPtr<PlugIn>(new MLine::MLinePlugIn());
}

} // End of namespace PlugIns

} // End of namespace MTropolis
