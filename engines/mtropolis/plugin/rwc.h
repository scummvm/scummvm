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

#ifndef MTROPOLIS_PLUGIN_RWC_H
#define MTROPOLIS_PLUGIN_RWC_H

#include "mtropolis/modifiers.h"
#include "mtropolis/modifier_factory.h"
#include "mtropolis/runtime.h"
#include "mtropolis/plugin/rwc_data.h"

namespace MTropolis {

namespace RWC {

class ThighBlasterModifier : public Modifier {
public:
	ThighBlasterModifier();
	~ThighBlasterModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::RWC::ThighBlasterModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "ThighBlaster Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class RWCPlugIn : public MTropolis::PlugIn {
public:
	RWCPlugIn();
	~RWCPlugIn();

	void registerModifiers(IPlugInModifierRegistrar *registrar) const override;

private:
	PlugInModifierFactory<ThighBlasterModifier, Data::RWC::ThighBlasterModifier> _thighBlasterModifierFactory;
};

} // End of namespace RWC

} // End of namespace MTropolis

#endif
