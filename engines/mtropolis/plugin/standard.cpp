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

class MultiMidiPlayer;

// I guess this follows QuickTime quirks, but basically, mTropolis pipes multiple inputs to a single
// output device, and is totally cool with multiple devices stomping each other.
//
// Obsidian actually has a timer that plays a MIDI file that fires AllNoteOff on every channel every
// 30 seconds, presumably to work around stuck notes, along with workarounds for the workaround,
// i.e. the intro sequence has a silent part exactly 30 seconds in timed to sync up with the mute.
//
// NOTE: Due to SharedPtr not currently being atomic, MidiFilePlayers MUST BE DESTROYED ON THE
// MAIN THREAD so there is no contention over the file refcount.
class MidiFilePlayer {
public:
	virtual ~MidiFilePlayer();
};

// This extends MidiDriver_BASE because we need to intercept commands to modulate the volume
// separately for each input.
class MidiFilePlayerImpl : public MidiFilePlayer, public MidiDriver_BASE {
public:
	explicit MidiFilePlayerImpl(MidiDriver_BASE *outputDriver, const Common::SharedPtr<Data::Standard::MidiModifier::EmbeddedFile> &file, uint32 baseTempo, uint8 volume);
	~MidiFilePlayerImpl();

	// Do not call any of these directly since they're not thread-safe, expose them via MultiMidiPlayer
	void stop();
	void play();
	void pause();
	void resume();
	void setVolume(uint8 volume);
	void detach();
	void onTimer();

private:
	void send(uint32 b) override;

	Common::SharedPtr<Data::Standard::MidiModifier::EmbeddedFile> _file;
	Common::SharedPtr<MidiParser> _parser;
	MidiDriver_BASE *_outputDriver;
	uint8 _volume;
};

class MultiMidiPlayer : public Audio::MidiPlayer {
public:
	MultiMidiPlayer();
	~MultiMidiPlayer();

	MidiFilePlayer *createFilePlayer(const Common::SharedPtr<Data::Standard::MidiModifier::EmbeddedFile> &file, uint8 volume);
	void deleteFilePlayer(MidiFilePlayer *player);

	void setPlayerVolume(MidiFilePlayer *player, uint8 volume);
	void stopPlayer(MidiFilePlayer *player);
	void playPlayer(MidiFilePlayer *player);
	void pausePlayer(MidiFilePlayer *player);
	void resumePlayer(MidiFilePlayer *player);

	uint32 getBaseTempo() const;

	void send(uint32 b) override;

private:
	void onTimer() override;

	static void timerCallback(void *refCon);

	Common::Mutex _mutex;
	Common::Array<Common::SharedPtr<MidiFilePlayerImpl> > _players;
};

MidiFilePlayer::~MidiFilePlayer() {
}

MidiFilePlayerImpl::MidiFilePlayerImpl(MidiDriver_BASE *outputDriver, const Common::SharedPtr<Data::Standard::MidiModifier::EmbeddedFile> &file, uint32 baseTempo, uint8 volume)
	: _file(file), _outputDriver(outputDriver), _parser(nullptr) {
	Common::SharedPtr<MidiParser> parser(MidiParser::createParser_SMF());

	if (file->contents.size() != 0 && parser->loadMusic(&file->contents[0], file->contents.size())) {
		parser->setTrack(0);
		parser->startPlaying();
		parser->setMidiDriver(this);
		parser->setTimerRate(baseTempo);

		_parser = parser;
	}
}

MidiFilePlayerImpl::~MidiFilePlayerImpl() {
	assert(!_parser);	// Call detach first!
}

void MidiFilePlayerImpl::stop() {
	_parser->stopPlaying();
}

void MidiFilePlayerImpl::play() {
	_parser->startPlaying();
}

void MidiFilePlayerImpl::pause() {
	_parser->pausePlaying();
}

void MidiFilePlayerImpl::resume() {
	_parser->resumePlaying();
}

void MidiFilePlayerImpl::setVolume(uint8 volume) {
	_volume = volume;
}

void MidiFilePlayerImpl::detach() {
	if (_parser) {
		_parser->setMidiDriver(nullptr);
		_parser.reset();
	}
}

void MidiFilePlayerImpl::onTimer() {
	if (_parser)
		_parser->onTimer();
}

void MidiFilePlayerImpl::send(uint32 b) {
	byte command = (b & 0xF0);

	if (command == MIDI_COMMAND_NOTE_ON || command == MIDI_COMMAND_NOTE_OFF) {
		byte velocity = (b >> 16) & 0xFF;
		velocity = (velocity * _volume * 257 + 256) >> 16;
		b = (b & 0xff00ffff) | (velocity << 16);
	}

	_outputDriver->send(b);
}

MultiMidiPlayer::MultiMidiPlayer() {
	createDriver(MDT_MIDI | MDT_PREFER_GM);

	if (_driver->open() != 0) {
		_driver->close();
		delete _driver;
		_driver = nullptr;
		return;
	}

	_driver->setTimerCallback(this, &timerCallback);
}

MultiMidiPlayer::~MultiMidiPlayer() {
	Common::StackLock lock(_mutex);

}

void MultiMidiPlayer::timerCallback(void *refCon) {
	static_cast<MultiMidiPlayer *>(refCon)->onTimer();
}

void MultiMidiPlayer::onTimer() {
	Common::StackLock lock(_mutex);

	for (const Common::SharedPtr<MidiFilePlayerImpl> &player : _players)
		player->onTimer();
}


MidiFilePlayer *MultiMidiPlayer::createFilePlayer(const Common::SharedPtr<Data::Standard::MidiModifier::EmbeddedFile> &file, uint8 volume) {
	Common::SharedPtr<MidiFilePlayerImpl> filePlayer(new MidiFilePlayerImpl(this, file, getBaseTempo(), volume));

	{
		Common::StackLock lock(_mutex);
		_players.push_back(filePlayer);
	}

	return filePlayer.get();
}

void MultiMidiPlayer::deleteFilePlayer(MidiFilePlayer *player) {
	Common::SharedPtr<MidiFilePlayerImpl> ref;

	for (Common::Array<Common::SharedPtr<MidiFilePlayerImpl> >::iterator it = _players.begin(), itEnd = _players.end(); it != itEnd; ++it) {
		if (it->get() == player) {
			{
				Common::StackLock lock(_mutex);
				ref = *it;
				_players.erase(it);
				ref->stop();
			}
			break;
		}
	}

	if (ref)
		ref->detach();
}

void MultiMidiPlayer::setPlayerVolume(MidiFilePlayer *player, uint8 volume) {
	Common::StackLock lock(_mutex);
	static_cast<MidiFilePlayerImpl *>(player)->setVolume(volume);
}

void MultiMidiPlayer::stopPlayer(MidiFilePlayer *player) {
	Common::StackLock lock(_mutex);
	static_cast<MidiFilePlayerImpl *>(player)->stop();
}

void MultiMidiPlayer::playPlayer(MidiFilePlayer *player) {
	Common::StackLock lock(_mutex);
	static_cast<MidiFilePlayerImpl *>(player)->play();
}

void MultiMidiPlayer::pausePlayer(MidiFilePlayer *player) {
	Common::StackLock lock(_mutex);
	static_cast<MidiFilePlayerImpl *>(player)->pause();
}

void MultiMidiPlayer::resumePlayer(MidiFilePlayer *player) {
	Common::StackLock lock(_mutex);
	static_cast<MidiFilePlayerImpl *>(player)->resume();
}

uint32 MultiMidiPlayer::getBaseTempo() const {
	return _driver->getBaseTempo();
}

void MultiMidiPlayer::send(uint32 b) {
	_driver->send(b);
}

CursorModifier::CursorModifier() {
}

bool CursorModifier::respondsToEvent(const Event &evt) const {
	return _applyWhen.respondsTo(evt) || _removeWhen.respondsTo(evt);
}

VThreadState CursorModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	// As with mTropolis, this doesn't support stacking cursors
 	if (_applyWhen.respondsTo(msg->getEvent())) {
		runtime->setModifierCursorOverride(_cursorID);
	}
	if (_removeWhen.respondsTo(msg->getEvent())) {
		runtime->clearModifierCursorOverride();
	}
	return kVThreadReturn;
}

bool CursorModifier::load(const PlugInModifierLoaderContext &context, const Data::Standard::CursorModifier &data) {
	if (!_applyWhen.load(data.applyWhen) || !_removeWhen.load(data.removeWhen))
		return false;
	_cursorID = data.cursorID;

	return true;
}

Common::SharedPtr<Modifier> CursorModifier::shallowClone() const {
	Common::SharedPtr<CursorModifier> clone(new CursorModifier(*this));
	return clone;
}

bool STransCtModifier::load(const PlugInModifierLoaderContext &context, const Data::Standard::STransCtModifier &data) {
	if (data.enableWhen.type != Data::PlugInTypeTaggedValue::kEvent ||
		data.disableWhen.type != Data::PlugInTypeTaggedValue::kEvent ||
		data.transitionType.type != Data::PlugInTypeTaggedValue::kInteger ||
		data.transitionDirection.type != Data::PlugInTypeTaggedValue::kInteger ||
		data.steps.type != Data::PlugInTypeTaggedValue::kInteger ||
		data.duration.type != Data::PlugInTypeTaggedValue::kInteger ||
		data.fullScreen.type != Data::PlugInTypeTaggedValue::kBoolean)
		return false;

	if (!_enableWhen.load(data.enableWhen.value.asEvent) || !_disableWhen.load(data.disableWhen.value.asEvent))
		return false;

	_transitionType = data.transitionType.value.asInt;
	_transitionDirection = data.transitionDirection.value.asInt;
	_steps = data.steps.value.asInt;
	_duration = data.duration.value.asInt;
	_fullScreen = data.fullScreen.value.asBoolean;

	return true;
}

bool STransCtModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "rate") {
		if (_duration <= (kMaxDuration / 100))
			result.setInt(100);
		else if (_duration >= kMaxDuration)
			result.setInt(1);
		else
			result.setInt((kMaxDuration + (_duration / 2)) / _duration);
		return true;
	} else if (attrib == "steps") {
		result.setInt(_steps);
		return true;
	}

	return Modifier::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome STransCtModifier::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "rate") {
		DynamicValueWriteFuncHelper<STransCtModifier, &STransCtModifier::scriptSetRate>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "steps") {
		DynamicValueWriteFuncHelper<STransCtModifier, &STransCtModifier::scriptSetSteps>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return Modifier::writeRefAttribute(thread, result, attrib);
;
}


Common::SharedPtr<Modifier> STransCtModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new STransCtModifier(*this));
}

MiniscriptInstructionOutcome STransCtModifier::scriptSetRate(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger))
		return kMiniscriptInstructionOutcomeFailed;

	if (asInteger < 1)
		asInteger = 1;
	else if (asInteger > 100)
		asInteger = 100;

	if (asInteger == 100)
		_duration = 0;
	else
		_duration = kMaxDuration / asInteger;

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome STransCtModifier::scriptSetSteps(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger))
		return kMiniscriptInstructionOutcomeFailed;

	if (asInteger < 4)
		asInteger = 4;
	else if (asInteger > 256)
		asInteger = 100;

	_steps = asInteger;

	return kMiniscriptInstructionOutcomeContinue;
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

ObjectReferenceVariableModifier::ObjectReferenceVariableModifier() {
}

bool ObjectReferenceVariableModifier::load(const PlugInModifierLoaderContext &context, const Data::Standard::ObjectReferenceVariableModifier &data) {
	if (data.setToSourceParentWhen.type != Data::PlugInTypeTaggedValue::kEvent)
		return false;

	_setToSourceParentWhen.load(data.setToSourceParentWhen.value.asEvent);

	if (data.objectPath.type != Data::PlugInTypeTaggedValue::kString)
		return false;

	_objectPath = data.objectPath.str;
	_object.reset();

	return true;
}

Common::SharedPtr<ModifierSaveLoad> ObjectReferenceVariableModifier::getSaveLoad() {
	return Common::SharedPtr<ModifierSaveLoad>(new SaveLoad(this));
}

// Object reference variables are somewhat unusual in that they don't store a simple value,
// they instead have "object" and "path" attributes AND as a value, they resolve to the
// modifier itself.
bool ObjectReferenceVariableModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "path") {
		result.setString(_objectPath);
		return true;
	}
	if (attrib == "object") {
		if (_object.object.expired())
			resolve();

		if (_object.object.expired())
			result.clear();
		else
			result.setObject(_object);
		return true;
	}

	return VariableModifier::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome ObjectReferenceVariableModifier::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "path") {
		DynamicValueWriteFuncHelper<ObjectReferenceVariableModifier, &ObjectReferenceVariableModifier::scriptSetPath>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}
	if (attrib == "object") {
		DynamicValueWriteFuncHelper<ObjectReferenceVariableModifier, &ObjectReferenceVariableModifier::scriptSetObject>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return VariableModifier::writeRefAttribute(thread, result, attrib);
}

bool ObjectReferenceVariableModifier::varSetValue(MiniscriptThread *thread, const DynamicValue &value) {
	// Somewhat strangely, setting an object reference variable to something sets the path or object,
	// but getting the variable returns the modifier
	switch (value.getType()) {
	case DynamicValueTypes::kNull:
	case DynamicValueTypes::kObject:
		return scriptSetObject(thread, value) == kMiniscriptInstructionOutcomeContinue;
	case DynamicValueTypes::kString:
		return scriptSetPath(thread, value) == kMiniscriptInstructionOutcomeContinue;
	default:
		return false;
	}
}

void ObjectReferenceVariableModifier::varGetValue(MiniscriptThread *thread, DynamicValue &dest) const {
	dest.setObject(this->getSelfReference());
}

Common::SharedPtr<Modifier> ObjectReferenceVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new ObjectReferenceVariableModifier(*this));
}

MiniscriptInstructionOutcome ObjectReferenceVariableModifier::scriptSetPath(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kString)
		return kMiniscriptInstructionOutcomeFailed;

	_objectPath = value.getString();
	_object.reset();

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome ObjectReferenceVariableModifier::scriptSetObject(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() == DynamicValueTypes::kNull) {
		_object.reset();
		_objectPath.clear();
		_fullPath.clear();

		return kMiniscriptInstructionOutcomeContinue;
	} else if (value.getType() == DynamicValueTypes::kObject) {
		Common::SharedPtr<RuntimeObject> obj = value.getObject().object.lock();
		if (!obj)
			return scriptSetObject(thread, DynamicValue());

		if (!computeObjectPath(obj.get(), _fullPath))
			return scriptSetObject(thread, DynamicValue());

		_objectPath = _fullPath;
		_object.object = obj;

		return kMiniscriptInstructionOutcomeContinue;
	} else
		return kMiniscriptInstructionOutcomeFailed;
}

void ObjectReferenceVariableModifier::resolve() {
	if (!_object.object.expired())
		return;

	_fullPath.clear();
	_object.reset();

	if (_objectPath.size() == 0)
		return;

	if (_objectPath[0] == '/')
		resolveAbsolutePath();
	else if (_objectPath[0] == '.')
		resolveRelativePath(this, _objectPath, 0);
	else
		warning("Object reference variable had an unknown path format");

	if (!_object.object.expired()) {
		if (!computeObjectPath(_object.object.lock().get(), _fullPath)) {
			_object.reset();
		}
	}
}

void ObjectReferenceVariableModifier::resolveRelativePath(RuntimeObject *obj, const Common::String &path, size_t startPos) {
	bool haveNextLevel = true;
	size_t nextLevelPos = startPos;

	while (haveNextLevel) {
		startPos = nextLevelPos;
		size_t endPos = path.find('/', startPos);
		if (endPos == Common::String::npos) {
			haveNextLevel = false;
			endPos = path.size();
		} else {
			nextLevelPos = endPos + 1;
		}

		Common::String levelName = path.substr(startPos, endPos - startPos);

		// This is technically more forgiving than mTropolis, which only allows ".." chains at the start of the path
		// Adjust this if it turns out to be a problem...
		if (levelName == "..") {
			obj = getObjectParent(obj);
			if (obj == nullptr)
				return;
		}

		const Common::Array<Common::SharedPtr<Modifier> > *modifierChildren = nullptr;
		const Common::Array<Common::SharedPtr<Structural> > *structuralChildren = nullptr;

		if (obj->isStructural()) {
			Structural *structural = static_cast<Structural *>(obj);
			modifierChildren = &structural->getModifiers();
			structuralChildren = &structural->getChildren();
		} else if (obj->isModifier()) {
			Modifier *modifier = static_cast<Modifier *>(obj);
			IModifierContainer *childContainer = modifier->getChildContainer();
			if (childContainer)
				modifierChildren = &childContainer->getModifiers();
		}

		bool foundMatch = false;
		if (modifierChildren) {
			for (const Common::SharedPtr<Modifier> &modifier : *modifierChildren) {
				if (caseInsensitiveEqual(levelName, modifier->getName())) {
					foundMatch = true;
					obj = modifier.get();
					break;
				}
			}
		}
		if (structuralChildren && !foundMatch) {
			for (const Common::SharedPtr<Structural> &structural : *structuralChildren) {
				if (caseInsensitiveEqual(levelName, structural->getName())) {
					foundMatch = true;
					obj = structural.get();
					break;
				}
			}
		}

		if (!foundMatch)
			return;
	}

	_object.object = obj->getSelfReference();
}

void ObjectReferenceVariableModifier::resolveAbsolutePath() {
	assert(_objectPath[0] == '/');

	RuntimeObject *project = this;
	for (;;) {
		RuntimeObject *parent = getObjectParent(project);
		if (!parent)
			break;
		project = parent;
	}

	if (!project->isProject())
		return; // Some sort of detached object

	Common::String projectPrefixes[2] = {
		"/" + static_cast<Structural *>(project)->getName(),
		"/<project>"};

	size_t prefixEnd = 0;

	bool foundPrefix = false;
	for (const Common::String &prefix : projectPrefixes) {
		if (_objectPath.size() >= prefix.size() && caseInsensitiveEqual(_objectPath.substr(0, prefix.size()), prefix)) {
			prefixEnd = prefix.size();
			foundPrefix = true;
			break;
		}
	}

	if (!foundPrefix)
		return;

	// If the object path is longer, then there must be a slash separator, otherwise this doesn't match the project
	if (prefixEnd == _objectPath.size()) {
		_object = ObjectReference(project->getSelfReference());
		return;
	}

	if (_objectPath[prefixEnd] != '/')
		return;

	return resolveRelativePath(project, _objectPath, prefixEnd + 1);
}

bool ObjectReferenceVariableModifier::computeObjectPath(RuntimeObject *obj, Common::String &outPath) {
	Common::String pathForThis = "/";

	if (obj->isStructural()) {
		Structural *structural = static_cast<Structural *>(obj);
		pathForThis += structural->getName();
	} else if (obj->isModifier()) {
		Modifier *modifier = static_cast<Modifier *>(obj);
		pathForThis += modifier->getName();
	}

	RuntimeObject *parent = getObjectParent(obj);

	if (parent) {
		Common::String pathForParent;
		if (!computeObjectPath(parent, pathForParent))
			return false;

		outPath = pathForParent + pathForThis;
	} else
		outPath = pathForThis;

	return true;
}

RuntimeObject *ObjectReferenceVariableModifier::getObjectParent(RuntimeObject *obj) {
	if (obj->isStructural()) {
		Structural *structural = static_cast<Structural *>(obj);
		return structural->getParent();
	} else if (obj->isModifier()) {
		Modifier *modifier = static_cast<Modifier *>(obj);
		return modifier->getParent().lock().get();
	}
	return nullptr;
}

ObjectReferenceVariableModifier::SaveLoad::SaveLoad(ObjectReferenceVariableModifier *modifier) : _modifier(modifier) {
	_objectPath = _modifier->_objectPath;
}

void ObjectReferenceVariableModifier::SaveLoad::commitLoad() const {
	_modifier->_object.reset();
	_modifier->_fullPath.clear();
	_modifier->_objectPath = _objectPath;
}

void ObjectReferenceVariableModifier::SaveLoad::saveInternal(Common::WriteStream *stream) const {
	stream->writeUint32BE(_objectPath.size());
	stream->writeString(_objectPath);
}

bool ObjectReferenceVariableModifier::SaveLoad::loadInternal(Common::ReadStream *stream) {
	uint32 stringLen = stream->readUint32BE();
	if (stream->err())
		return false;

	_objectPath.clear();

	if (stringLen) {
		Common::Array<char> strChars;
		strChars.resize(stringLen);
		stream->read(&strChars[0], stringLen);
		if (stream->err())
			return false;

		_objectPath = Common::String(&strChars[0], stringLen);
	}

	return true;
}

MidiModifier::MidiModifier() : _plugIn(nullptr), _filePlayer(nullptr) {
}

MidiModifier::~MidiModifier() {
	if (_filePlayer)
		_plugIn->getMidi()->deleteFilePlayer(_filePlayer);
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
		_volume = data.modeSpecific.embedded.volume;

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

		_volume = 100;
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
				debug(2, "MIDI (%x '%s'): Playing embedded file", getStaticGUID(), getName().c_str());
				if (!_filePlayer)
					_filePlayer = _plugIn->getMidi()->createFilePlayer(_embeddedFile, _volume * 255 / 100);
				_plugIn->getMidi()->playPlayer(_filePlayer);
			} else {
				debug(2, "MIDI (%x '%s'): Digested execute event but don't have anything to play", getStaticGUID(), getName().c_str());
			}
		}
	}

	return kVThreadReturn;
}

bool MidiModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "volume") {
		result.setInt(_volume);
		return true;
	}

	return Modifier::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome MidiModifier::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "volume") {
		DynamicValueWriteFuncHelper<MidiModifier, &MidiModifier::scriptSetVolume>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "notevelocity") {
		DynamicValueWriteFuncHelper<MidiModifier, &MidiModifier::scriptSetNoteVelocity>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return Modifier::writeRefAttribute(thread, result, attrib);
}

Common::SharedPtr<Modifier> MidiModifier::shallowClone() const {
	Common::SharedPtr<MidiModifier> clone(new MidiModifier(*this));

	clone->_isActive = false;

	return clone;
}

MiniscriptInstructionOutcome MidiModifier::scriptSetVolume(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger))
		return kMiniscriptInstructionOutcomeFailed;

	if (asInteger < 0)
		asInteger = 0;
	else if (asInteger > 100)
		asInteger = 100;

	_volume = asInteger;

	if (_mode == kModeFile) {
		debug(2, "MIDI (%x '%s'): Changing volume to %i", getStaticGUID(), getName().c_str(), _volume);
		if (_filePlayer)
			_plugIn->getMidi()->setPlayerVolume(_filePlayer, _volume * 255 / 100);
	}

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome MidiModifier::scriptSetNoteVelocity(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger))
		return kMiniscriptInstructionOutcomeFailed;

	if (asInteger < 0)
		asInteger = 0;
	else if (asInteger > 127)
		asInteger = 127;

	if (_mode == kModeSingleNote) {
		debug(2, "MIDI (%x '%s'): Changing note velocity to %i", getStaticGUID(), getName().c_str(), asInteger);
		_modeSpecific.singleNote.velocity = asInteger;
	}

	return kMiniscriptInstructionOutcomeContinue;
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

Common::SharedPtr<ModifierSaveLoad> ListVariableModifier::getSaveLoad() {
	return Common::SharedPtr<ModifierSaveLoad>(new SaveLoad(this));
}

bool ListVariableModifier::varSetValue(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() == DynamicValueTypes::kList)
		_list = value.getList()->clone();
	else
		return false;

	return true;
}

void ListVariableModifier::varGetValue(MiniscriptThread *thread, DynamicValue &dest) const {
	dest.setList(_list);
}

bool ListVariableModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "count") {
		result.setInt(_list->getSize());
		return true;
	}

	return Modifier::readAttribute(thread, result, attrib);
}

bool ListVariableModifier::readAttributeIndexed(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib, const DynamicValue &index) {
	if (attrib == "value") {
		size_t realIndex = 0;
		return _list->dynamicValueToIndex(realIndex, index) && _list->getAtIndex(realIndex, result);
	}
	return Modifier::readAttributeIndexed(thread, result, attrib, index);
}

MiniscriptInstructionOutcome ListVariableModifier::writeRefAttributeIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib, const DynamicValue &index) {
	if (attrib == "value") {
		size_t realIndex = 0;
		if (!_list->dynamicValueToIndex(realIndex, index))
			return kMiniscriptInstructionOutcomeFailed;

		_list->createWriteProxyForIndex(realIndex, writeProxy);
		writeProxy.containerList = _list;
		return kMiniscriptInstructionOutcomeContinue;
	}
	return kMiniscriptInstructionOutcomeFailed;
}

ListVariableModifier::ListVariableModifier(const ListVariableModifier &other) {
	if (other._list)
		_list = other._list->clone();
}

Common::SharedPtr<Modifier> ListVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new ListVariableModifier(*this));
}

ListVariableModifier::SaveLoad::SaveLoad(ListVariableModifier *modifier) : _modifier(modifier), _list(new DynamicList()) {
}

void ListVariableModifier::SaveLoad::commitLoad() const {
	_modifier->_list = _list;
}

void ListVariableModifier::SaveLoad::saveInternal(Common::WriteStream *stream) const {
	stream->writeUint32BE(_list->getType());
	stream->writeUint32BE(_list->getSize());

	size_t listSize = _list->getSize();
	for (size_t i = 0; i < listSize; i++) {
		switch (_list->getType()) {
		case DynamicValueTypes::kInteger:
			stream->writeSint32BE(_list->getInt()[i]);
			break;
		case DynamicValueTypes::kPoint: {
				const Point16 &pt = _list->getPoint()[i];
				stream->writeSint16BE(pt.x);
				stream->writeSint16BE(pt.y);
			}
			break;
		case DynamicValueTypes::kIntegerRange: {
				const IntRange &range = _list->getIntRange()[i];
				stream->writeSint32BE(range.min);
				stream->writeSint32BE(range.max);
			} break;
		case DynamicValueTypes::kFloat:
			stream->writeDoubleBE(_list->getFloat()[i]);
			break;
		case DynamicValueTypes::kString: {
				const Common::String &str = _list->getString()[i];
				stream->writeUint32BE(str.size());
				stream->writeString(str);
			} break;
		case DynamicValueTypes::kVector: {
				const AngleMagVector &vec = _list->getVector()[i];
				stream->writeDoubleBE(vec.angleRadians);
				stream->writeDoubleBE(vec.magnitude);
			} break;
		case DynamicValueTypes::kBoolean:
			stream->writeByte(_list->getBool()[i] ? 1 : 0);
			break;
		default:
			error("Can't figure out how to write a saved variable");
			break;
		}
	}
}

bool ListVariableModifier::SaveLoad::loadInternal(Common::ReadStream *stream) {
	uint32 typeCode = stream->readUint32BE();
	uint32 size = stream->readUint32BE();

	if (stream->err())
		return false;

	for (size_t i = 0; i < size; i++) {
		DynamicValue val;

		switch (typeCode) {
		case DynamicValueTypes::kInteger: {
				int32 i32 = stream->readSint32BE();
				val.setInt(i32);
			} break;
		case DynamicValueTypes::kPoint: {
				Point16 pt;
				pt.x = stream->readSint16BE();
				pt.y = stream->readSint16BE();
				val.setPoint(pt);
			} break;
		case DynamicValueTypes::kIntegerRange: {
				IntRange range;
				range.min = stream->readSint32BE();
				range.max = stream->readSint32BE();
				val.setIntRange(range);
			} break;
		case DynamicValueTypes::kFloat: {
				double f;
				f = stream->readDoubleBE();
				val.setFloat(f);
			} break;
		case DynamicValueTypes::kString: {
			uint32 strLen = stream->readUint32BE();
			if (stream->err())
				return false;

				Common::String str;
				if (strLen > 0) {
					Common::Array<char> chars;
					chars.resize(strLen);
					stream->read(&chars[0], strLen);
					str = Common::String(&chars[0], strLen);
				}
				val.setString(str);
			} break;
		case DynamicValueTypes::kVector: {
				AngleMagVector vec;
				vec.angleRadians = stream->readDoubleBE();
				vec.magnitude = stream->readDoubleBE();
				val.setVector(vec);
			} break;
		case DynamicValueTypes::kBoolean: {
				byte b = stream->readByte();
				val.setBool(b != 0);
			} break;
		default:
			error("Can't figure out how to write a saved variable");
			break;
		}

		if (stream->err())
			return false;

		_list->setAtIndex(i, val);
	}

	return true;
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
	_midi.reset(new MultiMidiPlayer());
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

MultiMidiPlayer *StandardPlugIn::getMidi() const {
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
