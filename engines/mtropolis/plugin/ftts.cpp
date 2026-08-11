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

#include "mtropolis/plugin/ftts.h"
#include "mtropolis/plugins.h"

namespace MTropolis {

namespace FTTS {

MotionModifier::MotionModifier() {
}

MotionModifier::~MotionModifier() {
}

bool MotionModifier::load(const PlugInModifierLoaderContext &context, const Data::FTTS::MotionModifier &data) {
	return true;
}

bool MotionModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState MotionModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void MotionModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void MotionModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> MotionModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new MotionModifier(*this));
}

const char *MotionModifier::getDefaultName() const {
	return "Motion Modifier"; // ???
}

SparkleModifier::SparkleModifier() {
}

SparkleModifier::~SparkleModifier() {
}

bool SparkleModifier::load(const PlugInModifierLoaderContext &context, const Data::FTTS::SparkleModifier &data) {
	return true;
}

bool SparkleModifier::respondsToEvent(const Event &evt) const {
	return false;
}

VThreadState SparkleModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	return kVThreadReturn;
}

void SparkleModifier::disable(Runtime *runtime) {
}

#ifdef MTROPOLIS_DEBUG_ENABLE
void SparkleModifier::debugInspect(IDebugInspectionReport *report) const {
	Modifier::debugInspect(report);
}
#endif

Common::SharedPtr<Modifier> SparkleModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new SparkleModifier(*this));
}

const char *SparkleModifier::getDefaultName() const {
	return "Sparkle Modifier"; // ???
}

FTTSPlugIn::FTTSPlugIn()
	: _motionModifierFactory(this)
	, _sparkleModifierFactory(this) {
}

FTTSPlugIn::~FTTSPlugIn() {
}

void FTTSPlugIn::registerModifiers(IPlugInModifierRegistrar *registrar) const {
	registrar->registerPlugInModifier("Motion", &_motionModifierFactory);
	registrar->registerPlugInModifier("Sparkle", &_sparkleModifierFactory);
}

} // End of namespace FTTS

namespace PlugIns {

Common::SharedPtr<PlugIn> createFTTS() {
	return Common::SharedPtr<PlugIn>(new FTTS::FTTSPlugIn());
}

} // End of namespace PlugIns

} // End of namespace MTropolis
