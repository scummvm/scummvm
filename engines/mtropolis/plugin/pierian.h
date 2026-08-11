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

#ifndef MTROPOLIS_PLUGIN_PIERIAN_H
#define MTROPOLIS_PLUGIN_PIERIAN_H

#include "mtropolis/modifiers.h"
#include "mtropolis/modifier_factory.h"
#include "mtropolis/runtime.h"
#include "mtropolis/plugin/pierian_data.h"

namespace MTropolis {

namespace Pierian {

class FlattenModifier : public Modifier {
public:
	FlattenModifier();
	~FlattenModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Pierian::FlattenModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Flatten Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class SaveFileModifier : public Modifier {
public:
	SaveFileModifier();
	~SaveFileModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Pierian::SaveFileModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "SaveFile Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class WasteModifier : public Modifier {
public:
	WasteModifier();
	~WasteModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Pierian::WasteModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;

	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Waste Modifier"; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

class PierianPlugIn : public MTropolis::PlugIn {
public:
	PierianPlugIn();
	~PierianPlugIn();

	void registerModifiers(IPlugInModifierRegistrar *registrar) const override;

private:
	PlugInModifierFactory<FlattenModifier, Data::Pierian::FlattenModifier> _flattenModifierFactory;
	PlugInModifierFactory<SaveFileModifier, Data::Pierian::SaveFileModifier> _saveFileModifierFactory;
	PlugInModifierFactory<WasteModifier, Data::Pierian::WasteModifier> _wasteModifierFactory;
};

} // End of namespace Pierian

} // End of namespace MTropolis

#endif
