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

#include "mtropolis/plugin/pierian.h"
#include "mtropolis/plugins.h"

namespace MTropolis {

namespace Pierian {

FlattenModifier::FlattenModifier() {
}

FlattenModifier::~FlattenModifier() {
}

bool FlattenModifier::load(const PlugInModifierLoaderContext &context, const Data::Pierian::FlattenModifier &data) {
	return true;
}

bool FlattenModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState FlattenModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void FlattenModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void FlattenModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> FlattenModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new FlattenModifier(*this));
}

const char *FlattenModifier::getDefaultName() const {
	return "Flatten Modifier"; // ???
}

SaveFileModifier::SaveFileModifier() {
}

SaveFileModifier::~SaveFileModifier() {
}

bool SaveFileModifier::load(const PlugInModifierLoaderContext &context, const Data::Pierian::SaveFileModifier &data) {
	return true;
}

bool SaveFileModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState SaveFileModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void SaveFileModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void SaveFileModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> SaveFileModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new SaveFileModifier(*this));
}

const char *SaveFileModifier::getDefaultName() const {
	return "mLine Launcher Modifier"; // ???
}

WasteModifier::WasteModifier() {
}

WasteModifier::~WasteModifier() {
}

bool WasteModifier::load(const PlugInModifierLoaderContext &context, const Data::Pierian::WasteModifier &data) {
	return true;
}

bool WasteModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState WasteModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void WasteModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void WasteModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> WasteModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new WasteModifier(*this));
}

const char *WasteModifier::getDefaultName() const {
	return "mLine Launcher Modifier"; // ???
}

PierianPlugIn::PierianPlugIn()
	: _flattenModifierFactory(this)
	, _saveFileModifierFactory(this)
	, _wasteModifierFactory(this) {
}

PierianPlugIn::~PierianPlugIn() {
}

void PierianPlugIn::registerModifiers(IPlugInModifierRegistrar *registrar) const {
	registrar->registerPlugInModifier("FlattenMod", &_flattenModifierFactory);
	registrar->registerPlugInModifier("SaveFileMod", &_saveFileModifierFactory);
	registrar->registerPlugInModifier("WasteMod", &_wasteModifierFactory);
}

} // End of namespace Pierian

namespace PlugIns {

Common::SharedPtr<PlugIn> createPierian() {
	return Common::SharedPtr<PlugIn>(new Pierian::PierianPlugIn());
}

} // End of namespace PlugIns

} // End of namespace MTropolis
