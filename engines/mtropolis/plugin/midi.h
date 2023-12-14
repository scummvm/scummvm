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

#ifndef MTROPOLIS_PLUGIN_MIDI_H
#define MTROPOLIS_PLUGIN_MIDI_H

#include "mtropolis/modifier_factory.h"
#include "mtropolis/modifiers.h"
#include "mtropolis/plugin/midi_data.h"
#include "mtropolis/runtime.h"

class MidiDriver;

namespace MTropolis {

class Runtime;

namespace Midi {

class MidiPlugIn;
class MidiFilePlayer;
class MidiNotePlayer;
class MultiMidiPlayer;
class MidiCombinerSource;

class MidiModifier : public Modifier {
public:
	MidiModifier();
	~MidiModifier();

	bool load(const PlugInModifierLoaderContext &context, const Data::Midi::MidiModifier &data);

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
	uint8 _volume; // We need this always available because scripts will try to set it and then read it even in single note mode

	Common::SharedPtr<Data::Midi::MidiModifier::EmbeddedFile> _embeddedFile;

	uint16 _mutedTracks;

	MidiPlugIn *_plugIn;
	MidiFilePlayer *_filePlayer;
	MidiNotePlayer *_notePlayer;
};

class MidiPlugIn : public MTropolis::PlugIn {
public:
	explicit MidiPlugIn(bool useDynamicMidi);
	~MidiPlugIn();

	void registerModifiers(IPlugInModifierRegistrar *registrar) const override;

	MultiMidiPlayer *getMidi() const;

private:
	PlugInModifierFactory<MidiModifier, Data::Midi::MidiModifier> _midiModifierFactory;

	Common::SharedPtr<MultiMidiPlayer> _midi;
};

} // End of namespace Midi

} // End of namespace MTropolis

#endif
