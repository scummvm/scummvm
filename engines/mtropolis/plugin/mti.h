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

#ifndef MTROPOLIS_PLUGIN_MTI_H
#define MTROPOLIS_PLUGIN_MTI_H

#include "mtropolis/modifier_factory.h"
#include "mtropolis/modifiers.h"
#include "mtropolis/plugin/mti_data.h"
#include "mtropolis/runtime.h"

namespace MTropolis {

namespace MTI {

class MTIPlugIn;

class PanningModifier : public Modifier {
public:
	PanningModifier();
	~PanningModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::MTI::PanningModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Panning Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class ShanghaiModifier : public Modifier {
public:
	ShanghaiModifier();
	~ShanghaiModifier();

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

	bool load(const PlugInModifierLoaderContext &context, const Data::MTI::ShanghaiModifier &data);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Shanghai Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class PrintModifier : public Modifier {
public:
	PrintModifier();
	~PrintModifier();

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

	bool load(const PlugInModifierLoaderContext &context, const Data::MTI::PrintModifier &data);

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Print Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Event _executeWhen;
	Common::String _filePath;
};


class MTIPlugIn : public MTropolis::PlugIn {
public:
	MTIPlugIn();

	void registerModifiers(IPlugInModifierRegistrar *registrar) const override;

private:
	PlugInModifierFactory<PanningModifier, Data::MTI::PanningModifier> _panningModifierFactory;
	PlugInModifierFactory<ShanghaiModifier, Data::MTI::ShanghaiModifier> _shanghaiModifierFactory;
	PlugInModifierFactory<PrintModifier, Data::MTI::PrintModifier> _printModifierFactory;
};

} // End of namespace MTI

} // End of namespace MTropolis

#endif
