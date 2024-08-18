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

#include "mtropolis/plugin/thereware.h"
#include "mtropolis/plugins.h"

namespace MTropolis {

namespace Thereware {

RotatorModifier::RotatorModifier() {
}

RotatorModifier::~RotatorModifier() {
}

bool RotatorModifier::load(const PlugInModifierLoaderContext &context, const Data::Thereware::RotatorModifier &data) {
	return true;
}

bool RotatorModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState RotatorModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void RotatorModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void RotatorModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> RotatorModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new RotatorModifier(*this));
}

const char *RotatorModifier::getDefaultName() const {
	return "Rotator Modifier"; // ???
}

TrackerModifier::TrackerModifier() {
}

TrackerModifier::~TrackerModifier() {
}

bool TrackerModifier::load(const PlugInModifierLoaderContext &context, const Data::Thereware::TrackerModifier &data) {
	return true;
}

bool TrackerModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState TrackerModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void TrackerModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void TrackerModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> TrackerModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new TrackerModifier(*this));
}

const char *TrackerModifier::getDefaultName() const {
	return "Tracker Modifier"; // ???
}

DoubleClickModifier::DoubleClickModifier() {
}

DoubleClickModifier::~DoubleClickModifier() {
}

bool DoubleClickModifier::load(const PlugInModifierLoaderContext &context, const Data::Thereware::DoubleClickModifier &data) {
	return true;
}

bool DoubleClickModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState DoubleClickModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void DoubleClickModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void DoubleClickModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> DoubleClickModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new DoubleClickModifier(*this));
}

const char *DoubleClickModifier::getDefaultName() const {
	return "DoubleClick Modifier"; // ???
}

MouseTrapModifier::MouseTrapModifier() {
}

MouseTrapModifier::~MouseTrapModifier() {
}

bool MouseTrapModifier::load(const PlugInModifierLoaderContext &context, const Data::Thereware::MouseTrapModifier &data) {
	return true;
}

bool MouseTrapModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState MouseTrapModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void MouseTrapModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void MouseTrapModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> MouseTrapModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new MouseTrapModifier(*this));
}

const char *MouseTrapModifier::getDefaultName() const {
	return "MouseTrap Modifier"; // ???
}

WrapAroundModifier::WrapAroundModifier() {
}

WrapAroundModifier::~WrapAroundModifier() {
}

bool WrapAroundModifier::load(const PlugInModifierLoaderContext &context, const Data::Thereware::WrapAroundModifier &data) {
	return true;
}

bool WrapAroundModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState WrapAroundModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void WrapAroundModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void WrapAroundModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> WrapAroundModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new WrapAroundModifier(*this));
}

const char *WrapAroundModifier::getDefaultName() const {
	return "WrapAround Modifier"; // ???
}

EasyScrollerModifier::EasyScrollerModifier() {
}

EasyScrollerModifier::~EasyScrollerModifier() {
}

bool EasyScrollerModifier::load(const PlugInModifierLoaderContext &context, const Data::Thereware::EasyScrollerModifier &data) {
	return true;
}

bool EasyScrollerModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState EasyScrollerModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void EasyScrollerModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void EasyScrollerModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> EasyScrollerModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new EasyScrollerModifier(*this));
}

const char *EasyScrollerModifier::getDefaultName() const {
	return "EasyScroller Modifier"; // ???
}

GoThereModifier::GoThereModifier() {
}

GoThereModifier::~GoThereModifier() {
}

bool GoThereModifier::load(const PlugInModifierLoaderContext &context, const Data::Thereware::GoThereModifier &data) {
	return true;
}

bool GoThereModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState GoThereModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void GoThereModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void GoThereModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> GoThereModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new GoThereModifier(*this));
}

const char *GoThereModifier::getDefaultName() const {
	return "GoThere Modifier"; // ???
}

RandomizerModifier::RandomizerModifier() {
}

RandomizerModifier::~RandomizerModifier() {
}

bool RandomizerModifier::load(const PlugInModifierLoaderContext &context, const Data::Thereware::RandomizerModifier &data) {
	return true;
}

bool RandomizerModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState RandomizerModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void RandomizerModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void RandomizerModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> RandomizerModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new RandomizerModifier(*this));
}

const char *RandomizerModifier::getDefaultName() const {
	return "Randomizer Modifier"; // ???
}

TimeLoopModifier::TimeLoopModifier() {
}

TimeLoopModifier::~TimeLoopModifier() {
}

bool TimeLoopModifier::load(const PlugInModifierLoaderContext &context, const Data::Thereware::TimeLoopModifier &data) {
	return true;
}

bool TimeLoopModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState TimeLoopModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void TimeLoopModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void TimeLoopModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> TimeLoopModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new TimeLoopModifier(*this));
}

const char *TimeLoopModifier::getDefaultName() const {
	return "TimeLoop Modifier"; // ???
}

ConductorModifier::ConductorModifier() {
}

ConductorModifier::~ConductorModifier() {
}

bool ConductorModifier::load(const PlugInModifierLoaderContext &context, const Data::Thereware::ConductorModifier &data) {
	return true;
}

bool ConductorModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState ConductorModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void ConductorModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void ConductorModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> ConductorModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new ConductorModifier(*this));
}

const char *ConductorModifier::getDefaultName() const {
	return "Conductor Modifier"; // ???
}

AlphaMaticModifier::AlphaMaticModifier() {
}

AlphaMaticModifier::~AlphaMaticModifier() {
}

bool AlphaMaticModifier::load(const PlugInModifierLoaderContext &context, const Data::Thereware::AlphaMaticModifier &data) {
	return true;
}

bool AlphaMaticModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState AlphaMaticModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void AlphaMaticModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void AlphaMaticModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> AlphaMaticModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new AlphaMaticModifier(*this));
}

const char *AlphaMaticModifier::getDefaultName() const {
	return "AlphaMatic Modifier"; // ???
}

TherewarePlugIn::TherewarePlugIn()
	: _conductorModifierFactory(this)
	, _alphaMaticModifierFactory(this)
	, _rotatorModifierFactory(this)
	, _trackerModifierFactory(this)
	, _doubleClickModifierFactory(this)
	, _mouseTrapModifierFactory(this)
	, _wrapAroundModifierFactory(this)
	, _easyScrollerModifierFactory(this)
	, _goThereModifierFactory(this)
	, _randomizerModifierFactory(this)
	, _timeLoopModifierFactory(this) {
}

TherewarePlugIn::~TherewarePlugIn() {
}

void TherewarePlugIn::registerModifiers(IPlugInModifierRegistrar *registrar) const {
	//Alpha Kit
	registrar->registerPlugInModifier("AlphaMatic", &_alphaMaticModifierFactory);

	//Rotator Kit
	registrar->registerPlugInModifier("Rotator", &_rotatorModifierFactory);
	registrar->registerPlugInModifier("Tracker", &_trackerModifierFactory);

	//Mercury Kit
	registrar->registerPlugInModifier("DoubleClick", &_doubleClickModifierFactory);
	registrar->registerPlugInModifier("MouseTrap", &_mouseTrapModifierFactory);
	registrar->registerPlugInModifier("TimeLoop", &_timeLoopModifierFactory);

	//WrapAround Kit
	registrar->registerPlugInModifier("WrapAround", &_wrapAroundModifierFactory);
	registrar->registerPlugInModifier("EasyScroller", &_easyScrollerModifierFactory);

	//FixIt Kit
	registrar->registerPlugInModifier("GoThere", &_goThereModifierFactory);

	//Quick Kit
	registrar->registerPlugInModifier("Randomizer", &_randomizerModifierFactory);
	registrar->registerPlugInModifier("Conductor", &_conductorModifierFactory);
}

} // End of namespace Thereware

namespace PlugIns {

Common::SharedPtr<PlugIn> createThereware() {
	return Common::SharedPtr<PlugIn>(new Thereware::TherewarePlugIn());
}

} // End of namespace PlugIns

} // End of namespace MTropolis
