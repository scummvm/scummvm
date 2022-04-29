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

#include "mtropolis/miniscript.h"

#include "audio/mididrv.h"
#include "audio/midiplayer.h"
#include "audio/midiparser.h"


namespace MTropolis {

namespace Standard {

class MidiPlayer : public Audio::MidiPlayer {
public:
	explicit MidiPlayer(int8 source);
	~MidiPlayer();

	void playFile(const void *data, size_t size);
	int8 getSource() const;

private:
	int8 _source;
	bool _isInitialized;
};

MidiPlayer::MidiPlayer(int8 source) : _isInitialized(false), _source(source) {
	MidiDriver::DeviceHandle deviceHdl = MidiDriver::detectDevice(MDT_MIDI);
	if (!deviceHdl)
		return;

	_driver = MidiDriver::createMidi(deviceHdl);
	if (!_driver)
		return;

	if (_driver->open() != 0) {
		_driver->close();
		delete _driver;
		_driver = nullptr;
		return;
	}

	_driver->setTimerCallback(static_cast<Audio::MidiPlayer *>(this), &timerCallback);

	_isInitialized = true;
}

MidiPlayer::~MidiPlayer() {
	stop();

	if (_parser)
		delete _parser;
}

void MidiPlayer::playFile(const void *data, size_t size) {
	Common::StackLock lock(_mutex);

	stop();

	_parser = MidiParser::createParser_SMF();
	_parser->setMidiDriver(this);
	_parser->setTimerRate(_driver->getBaseTempo());

	// FIXME: MIDI API shouldn't need mutable void input...
	_parser->loadMusic(static_cast<byte *>(const_cast<void *>(data)), size);
	_parser->setTrack(0);
	_parser->startPlaying();

	resume();
}

int8 MidiPlayer::getSource() const {
	return _source;
}

bool CursorModifier::load(const PlugInModifierLoaderContext &context, const Data::Standard::CursorModifier &data) {
	if (!_applyWhen.load(data.applyWhen) || !_removeWhen.load(data.removeWhen))
		return false;
	_cursorID = data.cursorID;

	return true;
}

Common::SharedPtr<Modifier> CursorModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new CursorModifier(*this));
}

bool STransCtModifier::load(const PlugInModifierLoaderContext& context, const Data::Standard::STransCtModifier& data) {
	return true;
}

Common::SharedPtr<Modifier> STransCtModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new STransCtModifier(*this));
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

Common::SharedPtr<Modifier> MediaCueMessengerModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new MediaCueMessengerModifier(*this));
}

ObjectReferenceVariableModifier::ObjectReferenceVariableModifier() : _isResolved(false) {
}

bool ObjectReferenceVariableModifier::load(const PlugInModifierLoaderContext &context, const Data::Standard::ObjectReferenceVariableModifier &data) {
	if (data.setToSourceParentWhen.type != Data::PlugInTypeTaggedValue::kEvent)
		return false;

	_setToSourceParentWhen.load(data.setToSourceParentWhen.value.asEvent);

	if (data.objectPath.type != Data::PlugInTypeTaggedValue::kString)
		return false;

	_objectPath = data.objectPath.str;
	_isResolved = false;

	return true;
}

bool ObjectReferenceVariableModifier::setValue(const DynamicValue &value) {
	if (value.getType() == DynamicValueTypes::kNull) {
		_object.reset();
		_objectPath.clear();
		_isResolved = true;
	} else if (value.getType() == DynamicValueTypes::kObject) {
		_object = value.getObject();
		_objectPath.clear();
		_isResolved = true;
	} else {
		return false;
	}

	return true;
}

void ObjectReferenceVariableModifier::getValue(DynamicValue &dest) const {
	if (_isResolved) {
		if (_object.expired())
			dest.clear();
		else
			dest.setObject(_object);
	} else {
		error("Resolving default objects from variable modifiers is not implemented!");
		dest.clear();
	}
}

Common::SharedPtr<Modifier> ObjectReferenceVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new ObjectReferenceVariableModifier(*this));
}

MidiModifier::MidiModifier() : _plugIn(nullptr) {
}

MidiModifier::~MidiModifier() {
}

bool MidiModifier::load(const PlugInModifierLoaderContext &context, const Data::Standard::MidiModifier &data) {
	_plugIn = static_cast<StandardPlugIn *>(context.plugIn);

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

bool MidiModifier::respondsToEvent(const Event &evt) const {
	return _executeWhen.respondsTo(evt) || _terminateWhen.respondsTo(evt);
}

VThreadState MidiModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_executeWhen.respondsTo(msg->getEvent())) {
		if (_mode == kModeFile) {
			if (_embeddedFile) {
				_plugIn->getMidi()->playFile(&_embeddedFile->contents[0], _embeddedFile->contents.size());
			}
		}

	}

	return kVThreadReturn;
}

Common::SharedPtr<Modifier> MidiModifier::shallowClone() const {
	Common::SharedPtr<MidiModifier> clone(new MidiModifier(*this));

	clone->_isActive = false;

	return clone;
}

ListVariableModifier::ListVariableModifier() : _list(new DynamicList()) {
}

bool ListVariableModifier::load(const PlugInModifierLoaderContext &context, const Data::Standard::ListVariableModifier &data) {
	if (!data.havePersistentData || data.numValues == 0)
		return true;	// If the list is empty then we don't care, the actual value type is irrelevant because it can be reassigned

	DynamicValueTypes::DynamicValueType expectedType = DynamicValueTypes::kInvalid;
	switch (data.contentsType) {
	case Data::Standard::ListVariableModifier::kContentsTypeInteger:
		expectedType = DynamicValueTypes::kInteger;
		break;
	case Data::Standard::ListVariableModifier::kContentsTypePoint:
		expectedType = DynamicValueTypes::kPoint;
		break;
	case Data::Standard::ListVariableModifier::kContentsTypeRange:
		expectedType = DynamicValueTypes::kIntegerRange;
		break;
	case Data::Standard::ListVariableModifier::kContentsTypeFloat:
		expectedType = DynamicValueTypes::kFloat;
		break;
	case Data::Standard::ListVariableModifier::kContentsTypeString:
		expectedType = DynamicValueTypes::kString;
		break;
	case Data::Standard::ListVariableModifier::kContentsTypeObject:
		if (data.persistentValuesGarbled) {
			// Ignore and let the game fix it
			return true;
		} else {
			warning("Object reference lists are not implemented");
			return false;
		}
		break;
	case Data::Standard::ListVariableModifier::kContentsTypeVector:
		expectedType = DynamicValueTypes::kVector;
		break;
	case Data::Standard::ListVariableModifier::kContentsTypeBoolean:
		expectedType = DynamicValueTypes::kBoolean;
		break;
	default:
		warning("Unknown list data type");
		return false;
	}

	for (size_t i = 0; i < data.numValues; i++) {
		DynamicValue dynValue;
		if (!dynValue.load(data.values[i]))
			return false;

		if (dynValue.getType() != expectedType) {
			warning("List mod initialization element had the wrong type");
			return false;
		}

		if (!_list->setAtIndex(i, dynValue)) {
			warning("Failed to initialize list modifier, value was rejected");
			return false;
		}
	}

	return true;
}

bool ListVariableModifier::setValue(const DynamicValue &value) {
	if (value.getType() == DynamicValueTypes::kList)
		_list = value.getList()->clone();
	else
		return false;

	return true;
}

void ListVariableModifier::getValue(DynamicValue &dest) const {
	dest.setList(_list);
}


ListVariableModifier::ListVariableModifier(const ListVariableModifier &other) {
	if (other._list)
		_list = other._list->clone();
}

Common::SharedPtr<Modifier> ListVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new ListVariableModifier(*this));
}

bool SysInfoModifier::load(const PlugInModifierLoaderContext &context, const Data::Standard::SysInfoModifier &data) {
	return true;
}

bool SysInfoModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "bitdepth") {
		ColorDepthMode colorDepth = thread->getRuntime()->getFakeColorDepth();
		switch (colorDepth) {
		case kColorDepthMode1Bit:
			result.setInt(1);
			break;
		case kColorDepthMode2Bit:
			result.setInt(2);
			break;
		case kColorDepthMode4Bit:
			result.setInt(4);
			break;
		case kColorDepthMode8Bit:
			result.setInt(8);
			break;
		case kColorDepthMode16Bit:
			result.setInt(16);
			break;
		case kColorDepthMode32Bit:
			result.setInt(32);
			break;
		default:
			return false;
		}

		return true;
	} else if (attrib == "screensize") {
		uint16 width, height;
		thread->getRuntime()->getDisplayResolution(width, height);
		result.setPoint(Point16::create(width, height));
		return true;
	}

	return false;
}


Common::SharedPtr<Modifier> SysInfoModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new SysInfoModifier(*this));
}

StandardPlugInHacks::StandardPlugInHacks() : allowGarbledListModData(false) {
}

StandardPlugIn::StandardPlugIn()
	: _cursorModifierFactory(this)
	, _sTransCtModifierFactory(this)
	, _mediaCueModifierFactory(this)
	, _objRefVarModifierFactory(this)
	, _midiModifierFactory(this)
	, _listVarModifierFactory(this)
	, _sysInfoModifierFactory(this)
	, _lastAllocatedSourceID(0) {
	_midi.reset(new MidiPlayer(0));
}

StandardPlugIn::~StandardPlugIn() {
}

void StandardPlugIn::registerModifiers(IPlugInModifierRegistrar *registrar) const {
	registrar->registerPlugInModifier("CursorMod", &_cursorModifierFactory);
	registrar->registerPlugInModifier("STransCt", &_sTransCtModifierFactory);
	registrar->registerPlugInModifier("MediaCue", &_mediaCueModifierFactory);
	registrar->registerPlugInModifier("ObjRefP", &_objRefVarModifierFactory);
	registrar->registerPlugInModifier("MIDIModf", &_midiModifierFactory);
	registrar->registerPlugInModifier("ListMod", &_listVarModifierFactory);
	registrar->registerPlugInModifier("SysInfo", &_sysInfoModifierFactory);
}

const StandardPlugInHacks &StandardPlugIn::getHacks() const {
	return _hacks;
}

StandardPlugInHacks& StandardPlugIn::getHacks() {
	return _hacks;
}

MidiPlayer *StandardPlugIn::getMidi() const {
	return _midi.get();
}

int8 StandardPlugIn::allocateMidiSource() {
	if (_deallocatedSources.size() > 0) {
		int8 src = _deallocatedSources.back();
		_deallocatedSources.pop_back();
		return src;
	}

	if (_lastAllocatedSourceID == INT8_MAX)
		error("Ran out of MIDI sources");

	_lastAllocatedSourceID++;
	return _lastAllocatedSourceID;
}

void StandardPlugIn::deallocateMidiSource(int8 source) {
	_deallocatedSources.push_back(source);
}

} // End of namespace Standard

namespace PlugIns {

Common::SharedPtr<PlugIn> createStandard() {
	return Common::SharedPtr<PlugIn>(new Standard::StandardPlugIn());
}

} // End of namespace MTropolis

} // End of namespace MTropolis
