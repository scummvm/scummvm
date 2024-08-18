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

#ifndef MTROPOLIS_PLUGIN_THEREWARE_H
#define MTROPOLIS_PLUGIN_THEREWARE_H

#include "mtropolis/modifiers.h"
#include "mtropolis/modifier_factory.h"
#include "mtropolis/runtime.h"
#include "mtropolis/plugin/thereware_data.h"

namespace MTropolis {

namespace Thereware {

class ConductorModifier : public Modifier {
public:
	ConductorModifier();
	~ConductorModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Thereware::ConductorModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Conductor Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class AlphaMaticModifier : public Modifier {
public:
	AlphaMaticModifier();
	~AlphaMaticModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Thereware::AlphaMaticModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "AlphaMatic Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class RotatorModifier : public Modifier {
public:
	RotatorModifier();
	~RotatorModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Thereware::RotatorModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Rotator Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class TrackerModifier : public Modifier {
public:
	TrackerModifier();
	~TrackerModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Thereware::TrackerModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Tracker Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class MouseTrapModifier : public Modifier {
public:
	MouseTrapModifier();
	~MouseTrapModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Thereware::MouseTrapModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "MouseTrap Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class DoubleClickModifier : public Modifier {
public:
	DoubleClickModifier();
	~DoubleClickModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Thereware::DoubleClickModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "DoubleClick Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class WrapAroundModifier : public Modifier {
public:
	WrapAroundModifier();
	~WrapAroundModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Thereware::WrapAroundModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "WrapAround Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class EasyScrollerModifier : public Modifier {
public:
	EasyScrollerModifier();
	~EasyScrollerModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Thereware::EasyScrollerModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "EasyScroller Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class GoThereModifier : public Modifier {
public:
	GoThereModifier();
	~GoThereModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Thereware::GoThereModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "GoThere Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class RandomizerModifier : public Modifier {
public:
	RandomizerModifier();
	~RandomizerModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Thereware::RandomizerModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Randomizer Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class TimeLoopModifier : public Modifier {
public:
	TimeLoopModifier();
	~TimeLoopModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Thereware::TimeLoopModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "TimeLoop Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};


class TherewarePlugIn : public MTropolis::PlugIn {
public:
	TherewarePlugIn();
	~TherewarePlugIn();

	void registerModifiers(IPlugInModifierRegistrar *registrar) const override;

private:
	PlugInModifierFactory<ConductorModifier, Data::Thereware::ConductorModifier> _conductorModifierFactory;
	PlugInModifierFactory<AlphaMaticModifier, Data::Thereware::AlphaMaticModifier> _alphaMaticModifierFactory;
	PlugInModifierFactory<RotatorModifier, Data::Thereware::RotatorModifier> _rotatorModifierFactory;
	PlugInModifierFactory<TrackerModifier, Data::Thereware::TrackerModifier> _trackerModifierFactory;
	PlugInModifierFactory<DoubleClickModifier, Data::Thereware::DoubleClickModifier> _doubleClickModifierFactory;
	PlugInModifierFactory<MouseTrapModifier, Data::Thereware::MouseTrapModifier> _mouseTrapModifierFactory;
	PlugInModifierFactory<WrapAroundModifier, Data::Thereware::WrapAroundModifier> _wrapAroundModifierFactory;
	PlugInModifierFactory<EasyScrollerModifier, Data::Thereware::EasyScrollerModifier> _easyScrollerModifierFactory;
	PlugInModifierFactory<GoThereModifier, Data::Thereware::GoThereModifier> _goThereModifierFactory;
	PlugInModifierFactory<RandomizerModifier, Data::Thereware::RandomizerModifier> _randomizerModifierFactory;
	PlugInModifierFactory<TimeLoopModifier, Data::Thereware::TimeLoopModifier> _timeLoopModifierFactory;
};

} // End of namespace Thereware

} // End of namespace MTropolis

#endif
