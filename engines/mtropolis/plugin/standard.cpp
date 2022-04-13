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

#include "mtropolis/plugin/standard.h"
#include "mtropolis/plugins.h"


namespace MTropolis {

namespace Standard {

bool CursorModifier::load(const PlugInModifierLoaderContext &context, const Data::Standard::CursorModifier &data) {
	if (!_applyWhen.load(data.applyWhen) || !_removeWhen.load(data.removeWhen))
		return false;
	_cursorID = data.cursorID;

	return true;
}

bool STransCtModifier::load(const PlugInModifierLoaderContext& context, const Data::Standard::STransCtModifier& data) {
	return true;
}

bool MediaCueMessengerModifier::load(const PlugInModifierLoaderContext& context, const Data::Standard::MediaCueMessengerModifier& data) {
	if (data.enableWhen.type != Data::PlugInTypeTaggedValue::kEvent)
		return false;

	_enableWhen.load(data.enableWhen.value.asEvent);

	if (data.disableWhen.type != Data::PlugInTypeTaggedValue::kEvent)
		return false;

	_disableWhen.load(data.disableWhen.value.asEvent);

	if (data.triggerTiming.type != Data::PlugInTypeTaggedValue::kInteger)
		return false;

	_triggerTiming = static_cast<TriggerTiming>(data.triggerTiming.value.asInt);

	if (data.nonStandardMessageFlags.type != Data::PlugInTypeTaggedValue::kInteger)
		return false;

	int32 msgFlags = data.nonStandardMessageFlags.value.asInt;

	MessageFlags messageFlags;
	messageFlags.immediate = ((msgFlags & Data::Standard::MediaCueMessengerModifier::kMessageFlagImmediate) != 0);
	messageFlags.cascade = ((msgFlags & Data::Standard::MediaCueMessengerModifier::kMessageFlagCascade) != 0);
	messageFlags.relay = ((msgFlags & Data::Standard::MediaCueMessengerModifier::kMessageFlagRelay) != 0);
	if (!_send.load(data.sendEvent, messageFlags, data.with, data.destination))
		return false;

	return true;
}

bool ObjectReferenceVariableModifier::load(const PlugInModifierLoaderContext &context, const Data::Standard::ObjectReferenceVariableModifier &data) {
	if (data.setToSourceParentWhen.type != Data::PlugInTypeTaggedValue::kEvent)
		return false;

	_setToSourceParentWhen.load(data.setToSourceParentWhen.value.asEvent);

	if (data.objectPath.type != Data::PlugInTypeTaggedValue::kString)
		return false;

	_objectPath = data.objectPath.str;

	return true;
}

bool MidiModifier::load(const PlugInModifierLoaderContext &context, const Data::Standard::MidiModifier &data) {
	if (data.executeWhen.type != Data::PlugInTypeTaggedValue::kEvent)
		return false;

	_executeWhen.load(data.executeWhen.value.asEvent);
	if (data.terminateWhen.type != Data::PlugInTypeTaggedValue::kEvent)
		return false;

	_terminateWhen.load(data.terminateWhen.value.asEvent);

	if (data.embeddedFlag) {
		_mode = kModeFile;
		_embeddedFile = data.embeddedFile;

		_modeSpecific.file.loop = (data.modeSpecific.embedded.loop != 0);
		_modeSpecific.file.overrideTempo = (data.modeSpecific.embedded.overrideTempo != 0);
		_modeSpecific.file.volume = (data.modeSpecific.embedded.volume != 0);

		if (data.embeddedFadeIn.type != Data::PlugInTypeTaggedValue::kFloat
			|| data.embeddedFadeOut.type != Data::PlugInTypeTaggedValue::kFloat
			|| data.embeddedTempo.type != Data::PlugInTypeTaggedValue::kFloat)
			return false;

		_modeSpecific.file.fadeIn = data.embeddedFadeIn.value.asFloat.toDouble();
		_modeSpecific.file.fadeOut = data.embeddedFadeOut.value.asFloat.toDouble();
		_modeSpecific.file.tempo = data.embeddedTempo.value.asFloat.toDouble();
	} else {
		_mode = kModeSingleNote;

		if (data.singleNoteDuration.type != Data::PlugInTypeTaggedValue::kFloat)
			return false;

		_modeSpecific.singleNote.channel = data.modeSpecific.singleNote.channel;
		_modeSpecific.singleNote.note = data.modeSpecific.singleNote.note;
		_modeSpecific.singleNote.velocity = data.modeSpecific.singleNote.velocity;
		_modeSpecific.singleNote.program = data.modeSpecific.singleNote.program;
		_modeSpecific.singleNote.duration = data.singleNoteDuration.value.asFloat.toDouble();
	}

	return true;
}

StandardPlugIn::StandardPlugIn()
	: _cursorModifierFactory(this)
	, _sTransCtModifierFactory(this)
	, _mediaCueModifierFactory(this)
	, _objRefVarModifierFactory(this)
	, _midiModifierFactory(this) {
}

void StandardPlugIn::registerModifiers(IPlugInModifierRegistrar *registrar) const {
	registrar->registerPlugInModifier("CursorMod", &_cursorModifierFactory);
	registrar->registerPlugInModifier("STransCt", &_sTransCtModifierFactory);
	registrar->registerPlugInModifier("MediaCue", &_mediaCueModifierFactory);
	registrar->registerPlugInModifier("ObjRefP", &_objRefVarModifierFactory);
	registrar->registerPlugInModifier("MIDIModf", &_midiModifierFactory);
}

} // End of namespace Standard

namespace PlugIns {

Common::SharedPtr<PlugIn> createStandard() {
	return Common::SharedPtr<PlugIn>(new Standard::StandardPlugIn());
}

} // End of namespace MTropolis

} // End of namespace MTropolis
