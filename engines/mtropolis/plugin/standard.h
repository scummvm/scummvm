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

#ifndef MTROPOLIS_PLUGIN_STANDARD_H
#define MTROPOLIS_PLUGIN_STANDARD_H

#include "mtropolis/modifiers.h"
#include "mtropolis/modifier_factory.h"
#include "mtropolis/runtime.h"
#include "mtropolis/plugin/standard_data.h"

class MidiDriver;

namespace MTropolis {

class Runtime;

namespace Standard {

class StandardPlugIn;
class MidiFilePlayer;
class MidiNotePlayer;
class MultiMidiPlayer;
class MidiCombinerSource;

class CursorModifier : public Modifier {
public:
	CursorModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::CursorModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Cursor Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Event _applyWhen;
	Event _removeWhen;
	uint32 _cursorID;
};

// This appears to be basically a duplicate of scene transition modifier, except unlike that,
// its parameters are controllable via script, and the duration scaling appears to be different
// (probably 600000 max rate instead of 6000000)
class STransCtModifier : public Modifier {
public:
	static const int32 kMaxDuration = 600000;

	STransCtModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::STransCtModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "STransCt Scene Transition Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	MiniscriptInstructionOutcome scriptSetRate(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetSteps(MiniscriptThread *thread, const DynamicValue &value);

	Event _enableWhen;
	Event _disableWhen;

	int32 _transitionType;
	int32 _transitionDirection;
	int32 _steps;
	int32 _duration;
	bool _fullScreen;
};

class MediaCueMessengerModifier : public Modifier {
public:
	MediaCueMessengerModifier();
	~MediaCueMessengerModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::MediaCueMessengerModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Media Cue Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	enum CueSourceType {
		kCueSourceInteger,
		kCueSourceIntegerRange,
		kCueSourceVariableReference,
		kCueSourceLabel,

		kCueSourceInvalid = -1,
	};

	union CueSourceUnion {
		CueSourceUnion();
		~CueSourceUnion();

		int32 asInt;
		IntRange asIntRange;
		uint32 asVarRefGUID;
		Label asLabel;
		uint64 asUnset;

		template<class T, T (CueSourceUnion::*TMember)>
		void construct(const T &value);

		template<class T, T (CueSourceUnion::*TMember)>
		void destruct();
	};

	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	void linkInternalReferences(ObjectLinkingScope *scope) override;
	void visitInternalReferences(IStructuralReferenceVisitor *visitor) override;

	CueSourceType _cueSourceType;
	CueSourceUnion _cueSource;

	Common::WeakPtr<Modifier> _cueSourceModifier;

	Event _enableWhen;
	Event _disableWhen;

	MediaCueState _mediaCue;
	bool _isActive;
};

class ObjectReferenceVariableModifier : public VariableModifier {
public:
	ObjectReferenceVariableModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::ObjectReferenceVariableModifier &data);

	Common::SharedPtr<ModifierSaveLoad> getSaveLoad() override;

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) override;

	bool varSetValue(MiniscriptThread *thread, const DynamicValue &value) override;
	void varGetValue(MiniscriptThread *thread, DynamicValue &dest) const override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "Object Reference Variable Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	struct ObjectWriteInterface {
		static MiniscriptInstructionOutcome write(MiniscriptThread *thread, const DynamicValue &dest, void *objectRef, uintptr ptrOrOffset);
		static MiniscriptInstructionOutcome refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib);
		static MiniscriptInstructionOutcome refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index);
	};

	class SaveLoad : public ModifierSaveLoad {
	public:
		explicit SaveLoad(ObjectReferenceVariableModifier *modifier);

	private:
		void commitLoad() const override;
		void saveInternal(Common::WriteStream *stream) const override;
		bool loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) override;

		ObjectReferenceVariableModifier *_modifier;
		Common::String _objectPath;
	};

	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	MiniscriptInstructionOutcome scriptSetPath(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetObject(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptObjectRefAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, const Common::String &attrib);
	MiniscriptInstructionOutcome scriptObjectRefAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, const Common::String &attrib, const DynamicValue &index);

	void resolve(Runtime *runtime);
	void resolveRelativePath(RuntimeObject *obj, const Common::String &path, size_t startPos);
	void resolveAbsolutePath(Runtime *runtime);

	static bool computeObjectPath(RuntimeObject *obj, Common::String &outPath);
	static RuntimeObject *getObjectParent(RuntimeObject *obj);

	Event _setToSourceParentWhen;

	mutable ObjectReference _object;
	Common::String _objectPath;
	Common::String _fullPath;
};

class MidiModifier : public Modifier {
public:
	MidiModifier();
	~MidiModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::MidiModifier &data);

	bool respondsToEvent(const Event &evt) const override;
	VThreadState consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) override;
	void disable(Runtime *runtime) override;

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttributeIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib, const DynamicValue &index) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "MIDI Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
#endif

private:
	struct MuteTrackProxyInterface {
		static MiniscriptInstructionOutcome write(MiniscriptThread *thread, const DynamicValue &dest, void *objectRef, uintptr ptrOrOffset);
		static MiniscriptInstructionOutcome refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib);
		static MiniscriptInstructionOutcome refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index);
	};

	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	MiniscriptInstructionOutcome scriptSetVolume(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetNoteVelocity(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetNoteDuration(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetNoteNum(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetLoop(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetPlayNote(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetTempo(MiniscriptThread *thread, const DynamicValue &value);
	MiniscriptInstructionOutcome scriptSetMuteTrack(MiniscriptThread *thread, const DynamicValue &value);

	MiniscriptInstructionOutcome scriptSetMuteTrackIndexed(MiniscriptThread *thread, size_t trackIndex, bool muted);

	uint getBoostedVolume(Runtime *runtime) const;

	void playSingleNote();
	void stopSingleNote();

	struct FilePart {
		bool loop;
		bool overrideTempo;
		double tempo;
		double fadeIn;
		double fadeOut;
	};

	struct SingleNotePart {
		uint8 channel;
		uint8 note;
		uint8 velocity;
		uint8 program;
		double duration;
	};

	union ModeSpecificUnion {
		FilePart file;
		SingleNotePart singleNote;
	};

	enum Mode {
		kModeFile,
		kModeSingleNote,
	};

	Event _executeWhen;
	Event _terminateWhen;

	Mode _mode;
	ModeSpecificUnion _modeSpecific;
	uint8 _volume;	// We need this always available because scripts will try to set it and then read it even in single note mode

	Common::SharedPtr<Data::Standard::MidiModifier::EmbeddedFile> _embeddedFile;

	uint16 _mutedTracks;

	StandardPlugIn *_plugIn;
	MidiFilePlayer *_filePlayer;
	MidiNotePlayer *_notePlayer;
};

class ListVariableModifier : public VariableModifier {
public:
	ListVariableModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::ListVariableModifier &data);

	Common::SharedPtr<ModifierSaveLoad> getSaveLoad() override;

	bool varSetValue(MiniscriptThread *thread, const DynamicValue &value) override;
	void varGetValue(MiniscriptThread *thread, DynamicValue &dest) const override;

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;
	bool readAttributeIndexed(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib, const DynamicValue &index) override;
	MiniscriptInstructionOutcome writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) override;
	MiniscriptInstructionOutcome writeRefAttributeIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib, const DynamicValue &index) override;

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "List Variable Modifier"; }
	SupportStatus debugGetSupportStatus() const override { return kSupportStatusDone; }
	void debugInspect(IDebugInspectionReport *report) const override;
#endif

private:
	class SaveLoad : public ModifierSaveLoad {
	public:
		explicit SaveLoad(ListVariableModifier *modifier);

	private:
		void commitLoad() const override;
		void saveInternal(Common::WriteStream *stream) const override;
		bool loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) override;

		static void recursiveWriteList(DynamicList *list, Common::WriteStream *stream);
		static Common::SharedPtr<DynamicList> recursiveReadList(Common::ReadStream *stream);

		ListVariableModifier *_modifier;
		Common::SharedPtr<DynamicList> _list;
	};

	ListVariableModifier(const ListVariableModifier &other);
	ListVariableModifier &operator=(const ListVariableModifier &other);

	MiniscriptInstructionOutcome scriptSetCount(MiniscriptThread *thread, const DynamicValue &value);

	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;

	Common::SharedPtr<DynamicList> _list;
	DynamicValueTypes::DynamicValueType _preferredContentType;
};

class SysInfoModifier : public Modifier {
public:
	bool load(const PlugInModifierLoaderContext &context, const Data::Standard::SysInfoModifier &data);

	bool readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) override;

	void disable(Runtime *runtime) override {}

#ifdef MTROPOLIS_DEBUG_ENABLE
	const char *debugGetTypeName() const override { return "System Info Modifier"; }
#endif

private:
	Common::SharedPtr<Modifier> shallowClone() const override;
	const char *getDefaultName() const override;
};

struct StandardPlugInHacks {
	StandardPlugInHacks();

	// If list mod values are illegible, just ignore them and flag it as garbled.
	// Necessary to load object 00788ab9 (olL437Check) in Obsidian, which is supposed to be a list of
	// 4 lists that are 3-size each, in the persistent data, but actually contains 4 identical values
	// that appear to be garbage.
	bool allowGarbledListModData;
};

class StandardPlugIn : public MTropolis::PlugIn {
public:
	explicit StandardPlugIn(bool useDynamicMidi);
	~StandardPlugIn();

	void registerModifiers(IPlugInModifierRegistrar *registrar) const override;

	const StandardPlugInHacks &getHacks() const;
	StandardPlugInHacks &getHacks();

	MultiMidiPlayer *getMidi() const;

private:
	PlugInModifierFactory<CursorModifier, Data::Standard::CursorModifier> _cursorModifierFactory;
	PlugInModifierFactory<STransCtModifier, Data::Standard::STransCtModifier> _sTransCtModifierFactory;
	PlugInModifierFactory<MediaCueMessengerModifier, Data::Standard::MediaCueMessengerModifier> _mediaCueModifierFactory;
	PlugInModifierFactory<ObjectReferenceVariableModifier, Data::Standard::ObjectReferenceVariableModifier> _objRefVarModifierFactory;
	PlugInModifierFactory<MidiModifier, Data::Standard::MidiModifier> _midiModifierFactory;
	PlugInModifierFactory<ListVariableModifier, Data::Standard::ListVariableModifier> _listVarModifierFactory;
	PlugInModifierFactory<SysInfoModifier, Data::Standard::SysInfoModifier> _sysInfoModifierFactory;

	Common::SharedPtr<MultiMidiPlayer> _midi;
	StandardPlugInHacks _hacks;
};

} // End of namespace Standard

} // End of namespace MTropolis

#endif
