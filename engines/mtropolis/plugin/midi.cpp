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

#include "common/config-manager.h"
#include "common/file.h"
#include "common/random.h"

#include "audio/mididrv.h"
#include "audio/midiparser.h"
#include "audio/midiparser_smf.h"
#include "audio/midiplayer.h"

#include "mtropolis/miniscript.h"
#include "mtropolis/plugin/midi.h"
#include "mtropolis/plugins.h"

#include "mtropolis/miniscript.h"

namespace MTropolis {

namespace Midi {

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

class MidiNotePlayer {
public:
	virtual ~MidiNotePlayer();
};

class MidiCombinerSource : public MidiDriver_BASE {
public:
	virtual ~MidiCombinerSource();

	// Do not call this directly, it's not thread-safe, expose via MultiMidiPlayer
	virtual void setVolume(uint8 volume) = 0;
	virtual void detach() = 0;
};

MidiCombinerSource::~MidiCombinerSource() {
}

class MidiCombiner {
public:
	virtual ~MidiCombiner();

	virtual Common::SharedPtr<MidiCombinerSource> createSource() = 0;
};

MidiCombiner::~MidiCombiner() {
}

class MidiParser_MTropolis : public MidiParser_SMF {
public:
	MidiParser_MTropolis(bool hasTempoOverride, double tempoOverride, uint16 mutedTracks);

	void setTempo(uint32 tempo) override;

	void setTempoOverride(double tempoOverride);
	void setMutedTracks(uint16 mutedTracks);

protected:
	bool processEvent(const EventInfo &info, bool fireEvents) override;

private:
	double _tempoOverride;
	uint16 _mutedTracks;
	bool _hasTempoOverride;
};

MidiParser_MTropolis::MidiParser_MTropolis(bool hasTempoOverride, double tempoOverride, uint16 mutedTracks)
	: _hasTempoOverride(hasTempoOverride), _tempoOverride(tempoOverride), _mutedTracks(mutedTracks) {
}

void MidiParser_MTropolis::setTempo(uint32 tempo) {
	if (_hasTempoOverride)
		return;

	MidiParser_SMF::setTempo(tempo);
}

void MidiParser_MTropolis::setTempoOverride(double tempoOverride) {
	_hasTempoOverride = true;

	if (tempoOverride < 1.0)
		tempoOverride = 1.0;

	_tempoOverride = tempoOverride;

	uint32 convertedTempo = static_cast<uint32>(60000000.0 / tempoOverride);

	MidiParser_SMF::setTempo(convertedTempo);
}

void MidiParser_MTropolis::setMutedTracks(uint16 mutedTracks) {
	_mutedTracks = mutedTracks;
}

bool MidiParser_MTropolis::processEvent(const EventInfo &info, bool fireEvents) {
	if ((info.event & 0xf0) == MidiDriver_BASE::MIDI_COMMAND_NOTE_ON) {
		int track = _noteChannelToTrack[info.event & 0xf];
		if (track >= 0 && (_mutedTracks & (1 << track)))
			return true;
	}

	return MidiParser_SMF::processEvent(info, fireEvents);
}

class MidiFilePlayerImpl : public MidiFilePlayer {
public:
	explicit MidiFilePlayerImpl(const Common::SharedPtr<MidiCombinerSource> &outputDriver, const Common::SharedPtr<Data::Midi::MidiModifier::EmbeddedFile> &file, uint32 baseTempo, bool hasTempoOverride, double tempoOverride, uint8 volume, bool loop, uint16 mutedTracks);
	~MidiFilePlayerImpl();

	// Do not call any of these directly since they're not thread-safe, expose them via MultiMidiPlayer
	void stop();
	void play();
	void pause();
	void resume();
	void setVolume(uint8 volume);
	void setLoop(bool loop);
	void setTempoOverride(double tempo);
	void setMutedTracks(uint16 mutedTracks);
	void detach();
	void onTimer();

private:
	Common::SharedPtr<Data::Midi::MidiModifier::EmbeddedFile> _file;
	Common::SharedPtr<MidiParser_MTropolis> _parser;
	Common::SharedPtr<MidiCombinerSource> _outputDriver;
	uint16 _mutedTracks;
	bool _loop;
};

class MidiNotePlayerImpl : public MidiNotePlayer {
public:
	explicit MidiNotePlayerImpl(const Common::SharedPtr<MidiCombinerSource> &outputDriver, uint32 timerRate);
	~MidiNotePlayerImpl();

	// Do not call any of these directly since they're not thread-safe, expose them via MultiMidiPlayer
	void onTimer();
	void play(uint8 volume, uint8 channel, uint8 program, uint8 note, uint8 velocity, double duration);
	void stop();
	void detach();

private:
	Common::SharedPtr<MidiCombinerSource> _outputDriver;
	uint64 _durationRemaining;
	uint32 _timerRate;
	uint8 _channel;
	uint8 _note;
	uint8 _volume;
	// uint8 _program;

	bool _initialized;
};

class MultiMidiPlayer : public Audio::MidiPlayer {
public:
	explicit MultiMidiPlayer(bool useDynamicMidiMixer);
	~MultiMidiPlayer();

	MidiFilePlayer *createFilePlayer(const Common::SharedPtr<Data::Midi::MidiModifier::EmbeddedFile> &file, bool hasTempoOverride, double tempoOverride, uint8 volume, bool loop, uint16 mutedTracks);
	MidiNotePlayer *createNotePlayer();
	void deleteFilePlayer(MidiFilePlayer *player);
	void deleteNotePlayer(MidiNotePlayer *player);

	Common::SharedPtr<MidiCombinerSource> createSource();

	void setPlayerVolume(MidiFilePlayer *player, uint8 volume);
	void setPlayerLoop(MidiFilePlayer *player, bool loop);
	void setPlayerTempo(MidiFilePlayer *player, double tempo);
	void setPlayerMutedTracks(MidiFilePlayer *player, uint16 mutedTracks);
	void stopPlayer(MidiFilePlayer *player);
	void playPlayer(MidiFilePlayer *player);
	void pausePlayer(MidiFilePlayer *player);
	void resumePlayer(MidiFilePlayer *player);

	void playNote(MidiNotePlayer *player, uint8 volume, uint8 channel, uint8 program, uint8 note, uint8 velocity, double duration);
	void stopNote(MidiNotePlayer *player);

	uint32 getBaseTempo() const;

	void send(uint32 b) override;

private:
	void onTimer() override;

	static void timerCallback(void *refCon);

	Common::Mutex _mutex;
	Common::Array<Common::SharedPtr<MidiFilePlayerImpl> > _filePlayers;
	Common::Array<Common::SharedPtr<MidiNotePlayerImpl> > _notePlayers;
	Common::SharedPtr<MidiCombiner> _combiner;
};

MidiFilePlayer::~MidiFilePlayer() {
}

MidiNotePlayer::~MidiNotePlayer() {
}

MidiFilePlayerImpl::MidiFilePlayerImpl(const Common::SharedPtr<MidiCombinerSource> &outputDriver, const Common::SharedPtr<Data::Midi::MidiModifier::EmbeddedFile> &file, uint32 baseTempo, bool hasTempoOverride, double tempo, uint8 volume, bool loop, uint16 mutedTracks)
	: _file(file), _outputDriver(outputDriver), _parser(nullptr), _loop(loop), _mutedTracks(mutedTracks) {
	Common::SharedPtr<MidiParser_MTropolis> parser(new MidiParser_MTropolis(hasTempoOverride, tempo, mutedTracks));

	if (file->contents.size() != 0 && parser->loadMusic(&file->contents[0], file->contents.size())) {
		_parser = parser;

		parser->setTrack(0);
		parser->startPlaying();
		parser->setMidiDriver(outputDriver.get());
		parser->setTimerRate(baseTempo);
		parser->property(MidiParser::mpAutoLoop, loop ? 1 : 0);
	}
}

MidiFilePlayerImpl::~MidiFilePlayerImpl() {
	assert(!_parser); // Call detach first!
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
	_outputDriver->setVolume(volume);
}

void MidiFilePlayerImpl::setMutedTracks(uint16 mutedTracks) {
	_mutedTracks = mutedTracks;
	_parser->setMutedTracks(mutedTracks);
}

void MidiFilePlayerImpl::setLoop(bool loop) {
	_loop = loop;
	_parser->property(MidiParser::mpAutoLoop, loop ? 1 : 0);
}

void MidiFilePlayerImpl::setTempoOverride(double tempo) {
	_parser->setTempoOverride(tempo);
}

void MidiFilePlayerImpl::detach() {
	if (_parser) {
		_parser->setMidiDriver(nullptr);
		_parser.reset();
	}

	if (_outputDriver) {
		_outputDriver->detach();
		_outputDriver.reset();
	}
}

void MidiFilePlayerImpl::onTimer() {
	if (_parser)
		_parser->onTimer();
}

MidiNotePlayerImpl::MidiNotePlayerImpl(const Common::SharedPtr<MidiCombinerSource> &outputDriver, uint32 timerRate)
	: _timerRate(timerRate), _durationRemaining(0), _outputDriver(outputDriver), _channel(0), _note(0), /* _program(0), */ _initialized(false), _volume(100) {
}

MidiNotePlayerImpl::~MidiNotePlayerImpl() {
}

void MidiNotePlayerImpl::onTimer() {
	if (_durationRemaining > 0) {
		if (_durationRemaining <= _timerRate) {
			stop();
			assert(_durationRemaining == 0);
		} else {
			_durationRemaining -= _timerRate;
		}
	}
}

void MidiNotePlayerImpl::play(uint8 volume, uint8 channel, uint8 program, uint8 note, uint8 velocity, double duration) {
	if (duration < 0.000001)
		return;

	if (_durationRemaining)
		stop();

	_initialized = true;

	_durationRemaining = static_cast<uint64>(duration * 1000000);
	_channel = channel;
	_note = note;
	_volume = volume;

	// GM volume scale to linear is x^2 so we need the square root of the volume, and need to rescale it from 0-100 to 0-0x3f80
	// = 0x3f80 / sqrt(100)
	const double volumeMultiplier = 1625.6;

	if (volume > 100)
		volume = 100;
	uint16 hpVolume = static_cast<uint16>(floor(sqrt(volume) * volumeMultiplier));

	_outputDriver->send(MidiDriver_BASE::MIDI_COMMAND_PROGRAM_CHANGE | _channel, program, 0);
	_outputDriver->send(MidiDriver_BASE::MIDI_COMMAND_CONTROL_CHANGE | _channel, MidiDriver_BASE::MIDI_CONTROLLER_EXPRESSION, 127);
	_outputDriver->send(MidiDriver_BASE::MIDI_COMMAND_CONTROL_CHANGE | _channel, MidiDriver_BASE::MIDI_CONTROLLER_REVERB, 0);
	_outputDriver->send(MidiDriver_BASE::MIDI_COMMAND_CONTROL_CHANGE | _channel, MidiDriver_BASE::MIDI_CONTROLLER_VOLUME, (hpVolume >> 7) & 0x7f);
	_outputDriver->send(MidiDriver_BASE::MIDI_COMMAND_CONTROL_CHANGE | _channel, MidiDriver_BASE::MIDI_CONTROLLER_VOLUME + 32, hpVolume & 0x7f);
	_outputDriver->send(MidiDriver_BASE::MIDI_COMMAND_NOTE_ON | _channel, note, velocity);
}

void MidiNotePlayerImpl::stop() {
	if (!_durationRemaining)
		return;

	_durationRemaining = 0;
	_outputDriver->send(MidiDriver_BASE::MIDI_COMMAND_NOTE_OFF | _channel, _note, 0);
}

void MidiNotePlayerImpl::detach() {
	if (_outputDriver) {
		if (_durationRemaining)
			stop();

		_outputDriver->detach();
		_outputDriver.reset();
	}
}

// Simple combiner - Behaves "QuickTime-like" and all commands are passed through directly.
// This applies volume by modulating note velocity.
class MidiCombinerSimple;

class MidiCombinerSourceSimple : public MidiCombinerSource {
public:
	explicit MidiCombinerSourceSimple(MidiCombinerSimple *combiner);

	void setVolume(uint8 volume) override;
	void detach() override;
	void send(uint32 b) override;

private:
	MidiCombinerSimple *_combiner;
	uint8 _volume;
};

class MidiCombinerSimple : public MidiCombiner {
public:
	explicit MidiCombinerSimple(MidiDriver_BASE *outputDriver);

	Common::SharedPtr<MidiCombinerSource> createSource() override;

	void send(uint32 b);

private:
	MidiDriver_BASE *_outputDriver;
};

MidiCombinerSourceSimple::MidiCombinerSourceSimple(MidiCombinerSimple *combiner) : _combiner(combiner), _volume(255) {
}

void MidiCombinerSourceSimple::setVolume(uint8 volume) {
	_volume = volume;
}

void MidiCombinerSourceSimple::detach() {
}

void MidiCombinerSourceSimple::send(uint32 b) {
	byte command = (b & 0xF0);

	if (command == MIDI_COMMAND_NOTE_ON || command == MIDI_COMMAND_NOTE_OFF) {
		byte velocity = (b >> 16) & 0xFF;
		velocity = (velocity * _volume * 257 + 256) >> 16;
		b = (b & 0xff00ffff) | (velocity << 16);
	}

	_combiner->send(b);
}

MidiCombinerSimple::MidiCombinerSimple(MidiDriver_BASE *outputDriver) : _outputDriver(outputDriver) {
}

Common::SharedPtr<MidiCombinerSource> MidiCombinerSimple::createSource() {
	return Common::SharedPtr<MidiCombinerSource>(new MidiCombinerSourceSimple(this));
}

void MidiCombinerSimple::send(uint32 b) {
	_outputDriver->send(b);
}

// Dynamic combiner - Dynamic channel allocation, accurate volume control
class MidiCombinerDynamic;

class MidiCombinerSourceDynamic : public MidiCombinerSource {
public:
	MidiCombinerSourceDynamic(MidiCombinerDynamic *combiner, uint sourceID);
	~MidiCombinerSourceDynamic();

	void setVolume(uint8 volume) override;
	void send(uint32 b) override;

	void detach() override;

private:
	MidiCombinerDynamic *_combiner;
	uint _sourceID;
};

class MidiCombinerDynamic : public MidiCombiner {
public:
	MidiCombinerDynamic(MidiDriver_BASE *outputDriver);

	Common::SharedPtr<MidiCombinerSource> createSource() override;

	void deallocateSource(uint sourceID);

	void setSourceVolume(uint sourceID, uint8 volume);
	void sendFromSource(uint sourceID, uint32 b);
	void sendFromSource(uint sourceID, uint8 cmd, uint8 channel, uint8 param1, uint8 param2);

private:
	static const uint kMSBMask = 0x3f80u;
	static const uint kLSBMask = 0x7fu;
	static const uint kLRControllerStart = 64;
	static const uint kSostenutoOnThreshold = 64;
	static const uint kSustainOnThreshold = 64;

	enum DataEntryState {
		kDataEntryStateNone,
		kDataEntryStateRPN,
		kDataEntryStateNRPN,
	};

	struct MidiChannelState {
		MidiChannelState();

		void reset();
		void softReset(); // Executes changes corresponding to Reset All Controllers message

		uint16 _program;
		uint16 _aftertouch;
		uint16 _pitchBend;
		uint16 _rpnNumber;
		uint16 _nrpnNumber;
		DataEntryState _dataEntryState;

		uint16 _hrControllers[32];
		uint8 _lrControllers[32];

		uint16 _registeredParams[5];
	};

	struct SourceChannelState {
		SourceChannelState();
		void reset();

		MidiChannelState _midiChannelState;
	};

	struct SourceState {
		SourceState();

		void allocate();
		void deallocate();

		SourceChannelState _sourceChannelState[MidiDriver_BASE::MIDI_CHANNEL_COUNT];
		uint16 _sqrtMasterVolume;
		bool _isAllocated;
	};

	struct OutputChannelState {
		OutputChannelState();

		bool _hasSource;
		bool _volumeIsAmbiguous;
		uint _sourceID;
		uint _channelID;
		uint _noteOffCounter;

		MidiChannelState _midiChannelState;

		uint _numActiveNotes;
	};

	struct MidiActiveNote {
		uint8 _outputChannel;
		uint16 _tone;
		bool _affectedBySostenuto;

		// If either of these are set, then the note is off, but is sustained by a pedal
		bool _isSustainedBySustain;
		bool _isSustainedBySostenuto;
	};

	void doNoteOn(uint sourceID, uint8 channel, uint8 param1, uint8 param2);
	void doNoteOff(uint sourceID, uint8 channel, uint8 param1, uint8 param2);
	void doPolyphonicAftertouch(uint sourceID, uint8 channel, uint8 param1, uint8 param2);
	void doControlChange(uint sourceID, uint8 channel, uint8 param1, uint8 param2);
	void doProgramChange(uint sourceID, uint8 channel, uint8 param1, uint8 param2);
	void doChannelAftertouch(uint sourceID, uint8 channel, uint8 param1, uint8 param2);
	void doPitchBend(uint sourceID, uint8 channel, uint8 param1, uint8 param2);

	void doHighRangeControlChange(uint sourceID, uint8 channel, uint8 hrParam, uint16 value);
	void doLowRangeControlChange(uint sourceID, uint8 channel, uint8 lrParam, uint8 value);

	void doDataEntry(uint sourceID, uint8 channel, int16 existingValueMask, int16 offset);
	void doChannelMode(uint sourceID, uint8 channel, uint8 param1, uint8 param2);
	void doAllNotesOff(uint sourceID, uint8 channel, uint8 param2);
	void doAllSoundOff(uint sourceID, uint8 channel, uint8 param2);
	void doResetAllControllers(uint sourceID, uint8 channel, uint8 param2);

	void sendToOutput(uint8 command, uint8 channel, uint8 param1, uint8 param2);

	void syncSourceConfiguration(uint outputChannel, OutputChannelState &outChState, const SourceState &sourceState, const SourceChannelState &sourceChState);
	void syncSourceHRController(uint outputChannel, OutputChannelState &outChState, const SourceState &sourceState, const SourceChannelState &sourceChState, uint hrController);
	void syncSourceLRController(uint outputChannel, OutputChannelState &outChState, const SourceChannelState &sourceChState, uint lrController);
	void syncSourceRegisteredParam(uint outputChannel, OutputChannelState &outChState, const SourceChannelState &sourceChState, uint rpn);

	void tryCleanUpUnsustainedNote(uint noteIndex);

	Common::Array<SourceState> _sources;
	Common::Array<MidiActiveNote> _notes;
	OutputChannelState _outputChannels[MidiDriver_BASE::MIDI_CHANNEL_COUNT];
	uint _noteOffCounter;

	MidiDriver_BASE *_outputDriver;

	Common::SharedPtr<Common::DumpFile> _dumpFile;
	int _eventCounter;
};

MidiCombinerSourceDynamic::MidiCombinerSourceDynamic(MidiCombinerDynamic *combiner, uint sourceID) : _combiner(combiner), _sourceID(sourceID) {
}

MidiCombinerSourceDynamic::~MidiCombinerSourceDynamic() {
	assert(_combiner == nullptr); // Call detach first!
}

void MidiCombinerSourceDynamic::detach() {
	_combiner->deallocateSource(_sourceID);
	_combiner = nullptr;
}

void MidiCombinerSourceDynamic::setVolume(uint8 volume) {
	_combiner->setSourceVolume(_sourceID, volume);
}

void MidiCombinerSourceDynamic::send(uint32 b) {
	_combiner->sendFromSource(_sourceID, b);
}

MidiCombinerDynamic::MidiCombinerDynamic(MidiDriver_BASE *outputDriver) : _outputDriver(outputDriver), _noteOffCounter(1) {
#if 0
	_dumpFile.reset(new Common::DumpFile());
	_dumpFile->open("mididump.csv");
	_dumpFile->writeString("event\ttime\tchannel\tcmd\tparam1\tparam2\n");
#endif

	_eventCounter = 0;
}

Common::SharedPtr<MidiCombinerSource> MidiCombinerDynamic::createSource() {
	uint sourceID = _sources.size();

	for (uint i = 0; i < _sources.size(); i++) {
		if (!_sources[i]._isAllocated) {
			sourceID = i;
			break;
		}
	}

	if (sourceID == _sources.size())
		_sources.push_back(SourceState());

	_sources[sourceID].allocate();

	return Common::SharedPtr<MidiCombinerSource>(new MidiCombinerSourceDynamic(this, sourceID));
}

void MidiCombinerDynamic::deallocateSource(uint sourceID) {
	for (uint i = 0; i < ARRAYSIZE(_outputChannels); i++) {
		OutputChannelState &ch = _outputChannels[i];
		if (!ch._hasSource || ch._sourceID != sourceID)
			continue;

		// Stop any outputs and release sustain
		sendFromSource(sourceID, MidiDriver_BASE::MIDI_COMMAND_CONTROL_CHANGE, i, MidiDriver_BASE::MIDI_CONTROLLER_SUSTAIN, 0);
		sendFromSource(sourceID, MidiDriver_BASE::MIDI_COMMAND_CONTROL_CHANGE, i, MidiDriver_BASE::MIDI_CONTROLLER_SOSTENUTO, 0);
		sendFromSource(sourceID, MidiDriver_BASE::MIDI_COMMAND_CONTROL_CHANGE, i, MidiDriver_BASE::MIDI_CONTROLLER_ALL_NOTES_OFF, 0);

		ch._hasSource = false;
		assert(ch._numActiveNotes == 0);
	}

	_sources[sourceID].deallocate();
}

void MidiCombinerDynamic::setSourceVolume(uint sourceID, uint8 volume) {
	SourceState &src = _sources[sourceID];
	//src._root4MasterVolume = static_cast<uint16>(floor(sqrt(sqrt(volume)) * 16400.0));
	src._sqrtMasterVolume = static_cast<uint16>(floor(sqrt(volume) * 4104.0));

	for (uint i = 0; i < ARRAYSIZE(_outputChannels); i++) {
		OutputChannelState &ch = _outputChannels[i];
		if (!ch._hasSource || ch._sourceID != sourceID)
			continue;

		// Synchronize volume control
		syncSourceHRController(i, ch, src, src._sourceChannelState[ch._channelID], MidiDriver_BASE::MIDI_CONTROLLER_VOLUME);
	}
}

void MidiCombinerDynamic::sendFromSource(uint sourceID, uint32 b) {
	uint8 cmd = static_cast<uint8>(b & 0xf0);
	uint8 channel = static_cast<uint8>(b & 0x0f);
	uint8 param1 = static_cast<uint8>((b >> 8) & 0xff);
	uint8 param2 = static_cast<uint8>((b >> 16) & 0xff);

	sendFromSource(sourceID, cmd, channel, param1, param2);
}

void MidiCombinerDynamic::sendFromSource(uint sourceID, uint8 cmd, uint8 channel, uint8 param1, uint8 param2) {
	switch (cmd) {
	case MidiDriver_BASE::MIDI_COMMAND_NOTE_ON:
		doNoteOn(sourceID, channel, param1, param2);
		break;
	case MidiDriver_BASE::MIDI_COMMAND_NOTE_OFF:
		doNoteOff(sourceID, channel, param1, param2);
		break;
	case MidiDriver_BASE::MIDI_COMMAND_POLYPHONIC_AFTERTOUCH:
		doPolyphonicAftertouch(sourceID, channel, param1, param2);
		break;
	case MidiDriver_BASE::MIDI_COMMAND_CONTROL_CHANGE:
		doControlChange(sourceID, channel, param1, param2);
		break;
	case MidiDriver_BASE::MIDI_COMMAND_PROGRAM_CHANGE:
		doProgramChange(sourceID, channel, param1, param2);
		break;
	case MidiDriver_BASE::MIDI_COMMAND_CHANNEL_AFTERTOUCH:
		doChannelAftertouch(sourceID, channel, param1, param2);
		break;
	case MidiDriver_BASE::MIDI_COMMAND_PITCH_BEND:
		doPitchBend(sourceID, channel, param1, param2);
		break;
	case MidiDriver_BASE::MIDI_COMMAND_SYSTEM:
		break;
	}
}

void MidiCombinerDynamic::doNoteOn(uint sourceID, uint8 channel, uint8 param1, uint8 param2) {
	uint outputChannel = 0;

	if (channel == MidiDriver_BASE::MIDI_RHYTHM_CHANNEL) {
		outputChannel = MidiDriver_BASE::MIDI_RHYTHM_CHANNEL;
	} else {
		bool foundChannel = false;

		// Find an existing exactly-matching channel
		for (uint i = 0; i < ARRAYSIZE(_outputChannels); i++) {
			OutputChannelState &ch = _outputChannels[i];
			if (ch._hasSource && ch._sourceID == sourceID && ch._channelID == channel) {
				foundChannel = true;
				outputChannel = i;
				break;
			}
		}

		if (!foundChannel) {
			// Find an inactive channel
			for (uint i = 0; i < ARRAYSIZE(_outputChannels); i++) {
				if (i == MidiDriver_BASE::MIDI_RHYTHM_CHANNEL)
					continue;

				if (!_outputChannels[i]._hasSource) {
					foundChannel = true;
					outputChannel = i;
					break;
				}
			}
		}

		if (!foundChannel) {
			uint bestOffCounter = 0xffffffffu;

			// Find the channel that went quiet the longest time ago
			for (uint i = 0; i < ARRAYSIZE(_outputChannels); i++) {
				if (i == MidiDriver_BASE::MIDI_RHYTHM_CHANNEL)
					continue;

				if (_outputChannels[i]._numActiveNotes == 0 && _outputChannels[i]._noteOffCounter < bestOffCounter) {
					foundChannel = true;
					outputChannel = i;
					bestOffCounter = _outputChannels[i]._noteOffCounter;
				}
			}
		}

		// All eligible channels are playing already
		if (!foundChannel)
			return;
	}

	OutputChannelState &ch = _outputChannels[outputChannel];

	if (!ch._hasSource || ch._sourceID != sourceID || ch._channelID != channel) {
		ch._sourceID = sourceID;
		ch._channelID = channel;
		ch._hasSource = true;

		const SourceState &sourceState = _sources[sourceID];
		syncSourceConfiguration(outputChannel, ch, sourceState, sourceState._sourceChannelState[channel]);
	}

	sendToOutput(MidiDriver_BASE::MIDI_COMMAND_NOTE_ON, outputChannel, param1, param2);

	MidiActiveNote note;
	note._outputChannel = outputChannel;
	note._tone = param1;
	note._affectedBySostenuto = (ch._midiChannelState._lrControllers[MidiDriver_BASE::MIDI_CONTROLLER_SOSTENUTO - kLRControllerStart] >= kSostenutoOnThreshold);
	note._isSustainedBySostenuto = false;
	note._isSustainedBySustain = false;
	_notes.push_back(note);

	ch._numActiveNotes++;
}

void MidiCombinerDynamic::doNoteOff(uint sourceID, uint8 channel, uint8 param1, uint8 param2) {
	for (uint i = 0; i < ARRAYSIZE(_outputChannels); i++) {
		OutputChannelState &ch = _outputChannels[i];

		if (ch._hasSource && ch._sourceID == sourceID && ch._channelID == channel) {
			sendToOutput(MidiDriver_BASE::MIDI_COMMAND_NOTE_OFF, i, param1, param2);

			for (uint ani = 0; ani < _notes.size(); ani++) {
				MidiActiveNote &note = _notes[ani];
				if (note._outputChannel == i && note._tone == param1 && !note._isSustainedBySostenuto && !note._isSustainedBySustain) {
					if (ch._midiChannelState._lrControllers[MidiDriver_BASE::MIDI_CONTROLLER_SUSTAIN - kLRControllerStart] >= kSustainOnThreshold)
						note._isSustainedBySustain = true;

					if (note._affectedBySostenuto && ch._midiChannelState._lrControllers[MidiDriver_BASE::MIDI_CONTROLLER_SOSTENUTO - kLRControllerStart] >= kSostenutoOnThreshold)
						note._isSustainedBySostenuto = true;

					tryCleanUpUnsustainedNote(ani);
					break;
				}
			}

			break;
		}
	}
}

void MidiCombinerDynamic::doPolyphonicAftertouch(uint sourceID, uint8 channel, uint8 param1, uint8 param2) {
	for (uint i = 0; i < ARRAYSIZE(_outputChannels); i++) {
		const OutputChannelState &ch = _outputChannels[i];

		if (ch._hasSource && ch._sourceID == sourceID && ch._channelID == channel) {
			sendToOutput(MidiDriver_BASE::MIDI_COMMAND_POLYPHONIC_AFTERTOUCH, i, param1, param2);
			break;
		}
	}
}

void MidiCombinerDynamic::doControlChange(uint sourceID, uint8 channel, uint8 param1, uint8 param2) {
	SourceState &src = _sources[sourceID];
	SourceChannelState &sch = src._sourceChannelState[channel];

	if (param1 == MidiDriver_BASE::MIDI_CONTROLLER_DATA_ENTRY_MSB) {
		doDataEntry(sourceID, channel, kLSBMask, param2 << 7);
		return;
	} else if (param1 == MidiDriver_BASE::MIDI_CONTROLLER_DATA_ENTRY_LSB) {
		doDataEntry(sourceID, channel, kMSBMask, param2);
		return;
	} else if (param1 < 32) {
		uint16 ctrl = ((sch._midiChannelState._hrControllers[param1] & kLSBMask) | ((param2 & 0x7f)) << 7);
		doHighRangeControlChange(sourceID, channel, param1, ctrl);
		return;
	} else if (param1 < 64) {
		uint16 ctrl = ((sch._midiChannelState._hrControllers[param1 - 32] & kMSBMask) | (param2 & 0x7f));
		doHighRangeControlChange(sourceID, channel, param1 - 32, ctrl);
		return;
	} else if (param1 < 96) {
		doLowRangeControlChange(sourceID, channel, param1 - 64, param2);
		return;
	}

	switch (param1) {
	case 96:
		// Data increment
		doDataEntry(sourceID, channel, 0x3fff, 1);
		break;
	case 97:
		// Data decrement
		doDataEntry(sourceID, channel, 0x3fff, -1);
		break;
	case 98:
		// NRPN LSB
		sch._midiChannelState._nrpnNumber = ((sch._midiChannelState._nrpnNumber & kMSBMask) | (param2 & 0x7f));
		sch._midiChannelState._dataEntryState = kDataEntryStateNRPN;
		break;
	case 99:
		// NRPN MSB
		sch._midiChannelState._nrpnNumber = ((sch._midiChannelState._nrpnNumber & kLSBMask) | ((param2 & 0x7f) << 7));
		sch._midiChannelState._dataEntryState = kDataEntryStateNRPN;
		break;
	case 100:
		// RPN LSB
		sch._midiChannelState._rpnNumber = ((sch._midiChannelState._rpnNumber & kMSBMask) | (param2 & 0x7f));
		sch._midiChannelState._dataEntryState = kDataEntryStateRPN;
		break;
	case 101:
		// RPN MSB
		sch._midiChannelState._rpnNumber = ((sch._midiChannelState._rpnNumber & kLSBMask) | ((param2 & 0x7f) << 7));
		sch._midiChannelState._dataEntryState = kDataEntryStateRPN;
		break;
	default:
		if (param1 >= 120 && param1 < 128)
			doChannelMode(sourceID, channel, param1, param2);
		break;
	};
}

void MidiCombinerDynamic::doProgramChange(uint sourceID, uint8 channel, uint8 param1, uint8 param2) {
	for (uint i = 0; i < ARRAYSIZE(_outputChannels); i++) {
		OutputChannelState &ch = _outputChannels[i];

		if (ch._hasSource && ch._sourceID == sourceID && ch._channelID == channel) {
			sendToOutput(MidiDriver_BASE::MIDI_COMMAND_PROGRAM_CHANGE, i, param1, param2);
			ch._midiChannelState._program = param1;
			break;
		}
	}

	_sources[sourceID]._sourceChannelState[channel]._midiChannelState._program = param1;
}

void MidiCombinerDynamic::doChannelAftertouch(uint sourceID, uint8 channel, uint8 param1, uint8 param2) {
	for (uint i = 0; i < ARRAYSIZE(_outputChannels); i++) {
		OutputChannelState &ch = _outputChannels[i];

		if (ch._hasSource && ch._sourceID == sourceID && ch._channelID == channel) {
			sendToOutput(MidiDriver_BASE::MIDI_COMMAND_CHANNEL_AFTERTOUCH, i, param1, param2);
			ch._midiChannelState._aftertouch = param1;
			break;
		}
	}
}

void MidiCombinerDynamic::doPitchBend(uint sourceID, uint8 channel, uint8 param1, uint8 param2) {
	uint16 pitchBend = (param1 & 0x7f) | ((param2 & 0x7f) << 7);
	for (uint i = 0; i < ARRAYSIZE(_outputChannels); i++) {
		OutputChannelState &ch = _outputChannels[i];

		if (ch._hasSource && ch._sourceID == sourceID && ch._channelID == channel) {
			sendToOutput(MidiDriver_BASE::MIDI_COMMAND_PITCH_BEND, i, param1, param2);
			ch._midiChannelState._pitchBend = pitchBend;
			break;
		}
	}

	_sources[sourceID]._sourceChannelState[channel]._midiChannelState._pitchBend = pitchBend;
}

void MidiCombinerDynamic::doHighRangeControlChange(uint sourceID, uint8 channel, uint8 hrParam, uint16 value) {
	SourceState &src = _sources[sourceID];
	SourceChannelState &srcCh = src._sourceChannelState[channel];
	srcCh._midiChannelState._hrControllers[hrParam] = value;

	for (uint i = 0; i < ARRAYSIZE(_outputChannels); i++) {
		OutputChannelState &ch = _outputChannels[i];

		if (ch._hasSource && ch._sourceID == sourceID && ch._channelID == channel) {
			syncSourceHRController(i, ch, src, srcCh, hrParam);
			break;
		}
	}
}

void MidiCombinerDynamic::doLowRangeControlChange(uint sourceID, uint8 channel, uint8 lrParam, uint8 value) {
	SourceChannelState &srcCh = _sources[sourceID]._sourceChannelState[channel];
	srcCh._midiChannelState._lrControllers[lrParam] = value;

	for (uint i = 0; i < ARRAYSIZE(_outputChannels); i++) {
		OutputChannelState &ch = _outputChannels[i];

		if (ch._hasSource && ch._sourceID == sourceID && ch._channelID == channel) {
			if (lrParam == MidiDriver_BASE::MIDI_CONTROLLER_SUSTAIN - kLRControllerStart && value < kSustainOnThreshold) {
				for (uint rni = _notes.size(); rni > 0; rni--) {
					uint noteIndex = rni - 1;
					MidiActiveNote &note = _notes[noteIndex];
					if (note._isSustainedBySustain) {
						note._isSustainedBySustain = false;
						tryCleanUpUnsustainedNote(noteIndex);
					}
				}
			} else if (lrParam == MidiDriver_BASE::MIDI_CONTROLLER_SOSTENUTO - kLRControllerStart && value < kSostenutoOnThreshold) {
				for (uint rni = _notes.size(); rni > 0; rni--) {
					uint noteIndex = rni - 1;
					MidiActiveNote &note = _notes[noteIndex];
					if (note._isSustainedBySostenuto) {
						note._isSustainedBySostenuto = false;
						tryCleanUpUnsustainedNote(noteIndex);
					}
				}
			}

			syncSourceLRController(i, ch, srcCh, lrParam);
			break;
		}
	}
}

void MidiCombinerDynamic::doDataEntry(uint sourceID, uint8 channel, int16 existingValueMask, int16 offset) {
	SourceChannelState &srcCh = _sources[sourceID]._sourceChannelState[channel];

	if (srcCh._midiChannelState._dataEntryState == kDataEntryStateRPN && srcCh._midiChannelState._rpnNumber < ARRAYSIZE(srcCh._midiChannelState._registeredParams)) {
		int32 rp = srcCh._midiChannelState._registeredParams[srcCh._midiChannelState._rpnNumber];
		rp &= existingValueMask;
		rp += offset;

		srcCh._midiChannelState._registeredParams[srcCh._midiChannelState._rpnNumber] = (rp & existingValueMask) + offset;

		for (uint i = 0; i < ARRAYSIZE(_outputChannels); i++) {
			OutputChannelState &ch = _outputChannels[i];

			if (ch._hasSource && ch._sourceID == sourceID && ch._channelID == channel) {
				syncSourceRegisteredParam(i, ch, srcCh, srcCh._midiChannelState._rpnNumber);
				break;
			}
		}
	}
}

void MidiCombinerDynamic::doChannelMode(uint sourceID, uint8 channel, uint8 param1, uint8 param2) {
	// Remap omni/poly/mono modes to all notes off, since we don't do anything with omni/poly
	switch (param1) {
	case MidiDriver_BASE::MIDI_CONTROLLER_OMNI_OFF:
	case MidiDriver_BASE::MIDI_CONTROLLER_OMNI_ON:
	case MidiDriver_BASE::MIDI_CONTROLLER_MONO_ON:
	case MidiDriver_BASE::MIDI_CONTROLLER_POLY_ON:
	case MidiDriver_BASE::MIDI_CONTROLLER_ALL_NOTES_OFF:
		doAllNotesOff(sourceID, channel, param2);
		break;
	case MidiDriver_BASE::MIDI_CONTROLLER_ALL_SOUND_OFF:
		doAllSoundOff(sourceID, channel, param2);
		break;
	case MidiDriver_BASE::MIDI_CONTROLLER_RESET_ALL_CONTROLLERS:
		doResetAllControllers(sourceID, channel, param2);
		break;
	case 122: // Local control (ignore)
	default:
		break;
	}
}

void MidiCombinerDynamic::doAllNotesOff(uint sourceID, uint8 channel, uint8 param2) {
	uint outputChannel = 0;
	bool foundChannel = false;

	for (uint i = 0; i < ARRAYSIZE(_outputChannels); i++) {
		OutputChannelState &ch = _outputChannels[i];
		if (ch._hasSource && ch._sourceID == sourceID && ch._channelID == channel) {
			foundChannel = true;
			outputChannel = i;
			break;
		}
	}

	if (!foundChannel)
		return;

	OutputChannelState &ch = _outputChannels[outputChannel];

	bool sustainOn = (ch._midiChannelState._lrControllers[MidiDriver_BASE::MIDI_CONTROLLER_SUSTAIN - kLRControllerStart] >= kSustainOnThreshold);
	bool sostenutoOn = (ch._midiChannelState._lrControllers[MidiDriver_BASE::MIDI_CONTROLLER_SOSTENUTO - kLRControllerStart] >= kSostenutoOnThreshold);

	for (uint rni = _notes.size(); rni > 0; rni--) {
		uint noteIndex = rni - 1;
		MidiActiveNote &note = _notes[noteIndex];
		if (note._outputChannel == outputChannel) {
			if (note._affectedBySostenuto && sostenutoOn)
				note._isSustainedBySostenuto = true;
			if (sustainOn)
				note._isSustainedBySustain = true;

			tryCleanUpUnsustainedNote(noteIndex);
		}
	}

	sendToOutput(MidiDriver_BASE::MIDI_COMMAND_CONTROL_CHANGE, outputChannel, MidiDriver_BASE::MIDI_CONTROLLER_ALL_NOTES_OFF, param2);
}

void MidiCombinerDynamic::doAllSoundOff(uint sourceID, uint8 channel, uint8 param2) {
	uint outputChannel = 0;
	bool foundChannel = false;

	for (uint i = 0; i < ARRAYSIZE(_outputChannels); i++) {
		OutputChannelState &ch = _outputChannels[i];
		if (ch._hasSource && ch._sourceID == sourceID && ch._channelID == channel) {
			foundChannel = true;
			outputChannel = i;
			break;
		}
	}

	if (!foundChannel)
		return;

	OutputChannelState &ch = _outputChannels[outputChannel];

	for (uint rni = _notes.size(); rni > 0; rni--) {
		uint noteIndex = rni - 1;
		MidiActiveNote &note = _notes[noteIndex];
		if (note._outputChannel == outputChannel) {
			note._isSustainedBySostenuto = false;
			note._isSustainedBySustain = false;

			tryCleanUpUnsustainedNote(noteIndex);
		}
	}

	sendToOutput(MidiDriver_BASE::MIDI_COMMAND_CONTROL_CHANGE, outputChannel, MidiDriver_BASE::MIDI_CONTROLLER_ALL_SOUND_OFF, param2);
	ch._noteOffCounter = 0; // All sound is off so this can be recycled quickly
}

void MidiCombinerDynamic::doResetAllControllers(uint sourceID, uint8 channel, uint8 param2) {
	SourceChannelState &srcCh = _sources[sourceID]._sourceChannelState[channel];

	srcCh._midiChannelState.softReset();

	uint outputChannel = 0;
	bool foundChannel = false;

	for (uint i = 0; i < ARRAYSIZE(_outputChannels); i++) {
		OutputChannelState &ch = _outputChannels[i];
		if (ch._hasSource && ch._sourceID == sourceID && ch._channelID == channel) {
			foundChannel = true;
			outputChannel = i;
			break;
		}
	}

	if (!foundChannel)
		return;

	OutputChannelState &ch = _outputChannels[outputChannel];
	ch._midiChannelState.softReset();

	// Release all sustained notes
	for (uint rni = _notes.size(); rni > 0; rni--) {
		uint noteIndex = rni - 1;
		MidiActiveNote &note = _notes[noteIndex];
		if (note._outputChannel == outputChannel) {
			if (note._isSustainedBySostenuto || note._isSustainedBySustain) {
				note._isSustainedBySostenuto = false;
				note._isSustainedBySustain = false;
				tryCleanUpUnsustainedNote(noteIndex);
			}
		}
	}

	sendToOutput(MidiDriver_BASE::MIDI_COMMAND_CONTROL_CHANGE, outputChannel, MidiDriver_BASE::MIDI_CONTROLLER_RESET_ALL_CONTROLLERS, 0);
}

void MidiCombinerDynamic::sendToOutput(uint8 command, uint8 channel, uint8 param1, uint8 param2) {
	uint32 output = static_cast<uint32>(command) | static_cast<uint32>(channel) | static_cast<uint32>(param1 << 8) | static_cast<uint32>(param2 << 16);

	if (_dumpFile) {
		const int timestamp = g_system->getMillis(true);

		const char *cmdName = "Unknown Command";
		switch (command) {
		case MidiDriver_BASE::MIDI_COMMAND_CHANNEL_AFTERTOUCH:
			cmdName = "ChannelAftertouch";
			break;
		case MidiDriver_BASE::MIDI_COMMAND_NOTE_OFF:
			cmdName = "NoteOff";
			break;
		case MidiDriver_BASE::MIDI_COMMAND_NOTE_ON:
			cmdName = "NoteOn";
			break;
		case MidiDriver_BASE::MIDI_COMMAND_POLYPHONIC_AFTERTOUCH:
			cmdName = "PolyAftertouch";
			break;
		case MidiDriver_BASE::MIDI_COMMAND_CONTROL_CHANGE:
			cmdName = "ControlChange";
			break;
		case MidiDriver_BASE::MIDI_COMMAND_PROGRAM_CHANGE:
			cmdName = "ProgramChange";
			break;
		case MidiDriver_BASE::MIDI_COMMAND_PITCH_BEND:
			cmdName = "PitchBend";
			break;
		case MidiDriver_BASE::MIDI_COMMAND_SYSTEM:
			cmdName = "System";
			break;
		default:
			cmdName = "Unknown";
		}

		if (command == MidiDriver_BASE::MIDI_COMMAND_CONTROL_CHANGE) {
			Common::String ctrlName = "Unknown";

			switch (param1) {
			case MidiDriver_BASE::MIDI_CONTROLLER_BANK_SELECT_MSB:
				ctrlName = "BankSelect";
				break;
			case MidiDriver_BASE::MIDI_CONTROLLER_MODULATION:
				ctrlName = "Modulation";
				break;
			case MidiDriver_BASE::MIDI_CONTROLLER_DATA_ENTRY_MSB:
				ctrlName = "DataEntryMSB";
				break;
			case MidiDriver_BASE::MIDI_CONTROLLER_VOLUME:
				ctrlName = "VolumeMSB";
				break;
			case MidiDriver_BASE::MIDI_CONTROLLER_VOLUME + 32:
				ctrlName = "VolumeLSB";
				break;
			case MidiDriver_BASE::MIDI_CONTROLLER_BALANCE:
				ctrlName = "Balance";
				break;
			case MidiDriver_BASE::MIDI_CONTROLLER_PANNING:
				ctrlName = "Panning";
				break;
			case MidiDriver_BASE::MIDI_CONTROLLER_EXPRESSION:
				ctrlName = "Expression";
				break;
			case MidiDriver_BASE::MIDI_CONTROLLER_BANK_SELECT_LSB:
				ctrlName = "BankSelectLSB";
				break;
			case MidiDriver_BASE::MIDI_CONTROLLER_DATA_ENTRY_LSB:
				ctrlName = "DataEntryLSB";
				break;
			case MidiDriver_BASE::MIDI_CONTROLLER_SUSTAIN:
				ctrlName = "Sustain";
				break;
			case MidiDriver_BASE::MIDI_CONTROLLER_PORTAMENTO:
				ctrlName = "Portamento";
				break;
			case MidiDriver_BASE::MIDI_CONTROLLER_SOSTENUTO:
				ctrlName = "Sostenuto";
				break;
			case MidiDriver_BASE::MIDI_CONTROLLER_SOFT:
				ctrlName = "Soft";
				break;

			case MidiDriver_BASE::MIDI_CONTROLLER_REVERB:
				ctrlName = "Reverb";
				break;
			case MidiDriver_BASE::MIDI_CONTROLLER_CHORUS:
				ctrlName = "Chorus";
				break;
			case MidiDriver_BASE::MIDI_CONTROLLER_RPN_LSB:
				ctrlName = "RPNLSB";
				break;
			case MidiDriver_BASE::MIDI_CONTROLLER_RPN_MSB:
				ctrlName = "RPNMSB";
				break;
			case MidiDriver_BASE::MIDI_CONTROLLER_ALL_SOUND_OFF:
				ctrlName = "AllSoundOff";
				break;
			case MidiDriver_BASE::MIDI_CONTROLLER_RESET_ALL_CONTROLLERS:
				ctrlName = "ResetAllControllers";
				break;
			case MidiDriver_BASE::MIDI_CONTROLLER_ALL_NOTES_OFF:
				ctrlName = "AllNotesOff";
				break;
			case MidiDriver_BASE::MIDI_CONTROLLER_OMNI_ON:
				ctrlName = "OmniOn";
				break;
			case MidiDriver_BASE::MIDI_CONTROLLER_OMNI_OFF:
				ctrlName = "OmniOff";
				break;
			case MidiDriver_BASE::MIDI_CONTROLLER_MONO_ON:
				ctrlName = "MonoOn";
				break;
			case MidiDriver_BASE::MIDI_CONTROLLER_POLY_ON:
				ctrlName = "PolyOn";
				break;
			default:
				ctrlName = Common::String::format("Unknown%02x", static_cast<int>(param1));
			}

			_dumpFile->writeString(Common::String::format("%i\t%i\t%i\t%s\t%s\t%i\n", _eventCounter, timestamp, static_cast<int>(channel), cmdName, ctrlName.c_str(), static_cast<int>(param2)));
		} else
			_dumpFile->writeString(Common::String::format("%i\t%i\t%i\t%s\t%i\t%i\n", _eventCounter, timestamp, static_cast<int>(channel), cmdName, static_cast<int>(param1), static_cast<int>(param2)));

		_eventCounter++;
	}

	_outputDriver->send(output);
}

void MidiCombinerDynamic::syncSourceConfiguration(uint outputChannel, OutputChannelState &outChState, const SourceState &srcState, const SourceChannelState &sourceChState) {
	const MidiChannelState &srcMidiChState = sourceChState._midiChannelState;
	MidiChannelState &outState = outChState._midiChannelState;

	if (outState._program != srcMidiChState._program) {
		outState._program = srcMidiChState._program;
		sendToOutput(MidiDriver_BASE::MIDI_COMMAND_PROGRAM_CHANGE, outputChannel, srcMidiChState._program, 0);
	}

	if (outState._aftertouch != srcMidiChState._aftertouch) {
		outState._aftertouch = srcMidiChState._aftertouch;
		sendToOutput(MidiDriver_BASE::MIDI_COMMAND_CHANNEL_AFTERTOUCH, outputChannel, srcMidiChState._aftertouch, 0);
	}

	if (outState._pitchBend != srcMidiChState._pitchBend) {
		outState._pitchBend = srcMidiChState._pitchBend;
		sendToOutput(MidiDriver_BASE::MIDI_COMMAND_PITCH_BEND, outputChannel, (srcMidiChState._pitchBend & kLSBMask), (srcMidiChState._pitchBend & kMSBMask) >> 7);
	}

	for (uint i = 0; i < ARRAYSIZE(srcMidiChState._hrControllers); i++)
		syncSourceHRController(outputChannel, outChState, srcState, sourceChState, i);

	for (uint i = 0; i < ARRAYSIZE(srcMidiChState._lrControllers); i++)
		syncSourceLRController(outputChannel, outChState, sourceChState, i);

	for (uint i = 0; i < ARRAYSIZE(srcMidiChState._registeredParams); i++)
		syncSourceRegisteredParam(outputChannel, outChState, sourceChState, i);
}

void MidiCombinerDynamic::syncSourceHRController(uint outputChannel, OutputChannelState &outChState, const SourceState &srcState, const SourceChannelState &sourceChState, uint hrController) {
	const MidiChannelState &srcMidiChState = sourceChState._midiChannelState;
	MidiChannelState &outState = outChState._midiChannelState;

	uint16 effectiveValue = srcMidiChState._hrControllers[hrController];

	if (hrController == MidiDriver_BASE::MIDI_CONTROLLER_VOLUME) {
		// GM volume to gain control is 40*log10(V/127)
		// This means linear scale is (volume/0x3f80)^4
		// To modulate the volume linearly, we must multiply the volume by the square root
		// of the volume.
		uint32 effectiveValueScaled = static_cast<uint32>(srcState._sqrtMasterVolume) * static_cast<uint32>(effectiveValue);
		effectiveValueScaled += (effectiveValueScaled >> 16) + 1u;
		effectiveValue = static_cast<uint16>(effectiveValueScaled >> 16);
	}

	if (outState._hrControllers[hrController] == effectiveValue)
		return;

	uint16 deltaBits = (outState._hrControllers[hrController] ^ effectiveValue);

	if (deltaBits & kMSBMask)
		sendToOutput(MidiDriver_BASE::MIDI_COMMAND_CONTROL_CHANGE, outputChannel, hrController, (effectiveValue & kMSBMask) >> 7);
	if (deltaBits & kLSBMask)
		sendToOutput(MidiDriver_BASE::MIDI_COMMAND_CONTROL_CHANGE, outputChannel, hrController + 32, effectiveValue & kLSBMask);

	outState._hrControllers[hrController] = effectiveValue;
}

void MidiCombinerDynamic::syncSourceLRController(uint outputChannel, OutputChannelState &outChState, const SourceChannelState &sourceChState, uint lrController) {
	const MidiChannelState &srcState = sourceChState._midiChannelState;
	MidiChannelState &outState = outChState._midiChannelState;

	if (outState._lrControllers[lrController] == srcState._lrControllers[lrController])
		return;

	sendToOutput(MidiDriver_BASE::MIDI_COMMAND_CONTROL_CHANGE, outputChannel, lrController + kLRControllerStart, srcState._lrControllers[lrController] & kLSBMask);

	outState._lrControllers[lrController] = srcState._lrControllers[lrController];
}

void MidiCombinerDynamic::syncSourceRegisteredParam(uint outputChannel, OutputChannelState &outChState, const SourceChannelState &sourceChState, uint rpn) {
	const MidiChannelState &srcState = sourceChState._midiChannelState;
	MidiChannelState &outState = outChState._midiChannelState;

	if (outState._registeredParams[rpn] == srcState._registeredParams[rpn])
		return;

	outState._registeredParams[rpn] = srcState._registeredParams[rpn];

	if (outState._dataEntryState != kDataEntryStateRPN || outState._rpnNumber != srcState._rpnNumber) {
		outState._dataEntryState = kDataEntryStateRPN;
		outState._rpnNumber = srcState._rpnNumber;
		sendToOutput(MidiDriver_BASE::MIDI_COMMAND_CONTROL_CHANGE, outputChannel, MidiDriver_BASE::MIDI_CONTROLLER_RPN_LSB, rpn & kLSBMask);
		sendToOutput(MidiDriver_BASE::MIDI_COMMAND_CONTROL_CHANGE, outputChannel, MidiDriver_BASE::MIDI_CONTROLLER_RPN_MSB, (rpn & kMSBMask) >> 7);
	}

	sendToOutput(MidiDriver_BASE::MIDI_COMMAND_CONTROL_CHANGE, outputChannel, MidiDriver_BASE::MIDI_CONTROLLER_DATA_ENTRY_LSB, srcState._registeredParams[rpn] & kLSBMask);
	sendToOutput(MidiDriver_BASE::MIDI_COMMAND_CONTROL_CHANGE, outputChannel, MidiDriver_BASE::MIDI_CONTROLLER_DATA_ENTRY_MSB, (srcState._registeredParams[rpn] & kMSBMask) >> 7);
}

void MidiCombinerDynamic::tryCleanUpUnsustainedNote(uint noteIndex) {
	MidiActiveNote &note = _notes[noteIndex];

	if (!note._isSustainedBySostenuto && !note._isSustainedBySustain) {
		OutputChannelState &outCh = _outputChannels[note._outputChannel];
		assert(outCh._numActiveNotes > 0);
		outCh._numActiveNotes--;
		if (!outCh._numActiveNotes)
			outCh._noteOffCounter = _noteOffCounter++;

		_notes.remove_at(noteIndex);
	}
}

MidiCombinerDynamic::MidiChannelState::MidiChannelState() {
	reset();
}

void MidiCombinerDynamic::MidiChannelState::reset() {
	_program = 0;
	_aftertouch = 0;
	_pitchBend = 0x2000;

	for (uint i = 0; i < ARRAYSIZE(_hrControllers); i++)
		_hrControllers[i] = 0;
	for (uint i = 0; i < ARRAYSIZE(_lrControllers); i++)
		_lrControllers[i] = 0;
	for (uint i = 0; i < ARRAYSIZE(_registeredParams); i++)
		_registeredParams[i] = 0;

	_hrControllers[MidiDriver_BASE::MIDI_CONTROLLER_BALANCE] = (64 << 7);
	_hrControllers[MidiDriver_BASE::MIDI_CONTROLLER_PANNING] = (64 << 7);
	_hrControllers[MidiDriver_BASE::MIDI_CONTROLLER_VOLUME] = (127 << 7);

	_dataEntryState = kDataEntryStateNone;
	_rpnNumber = 0;
	_nrpnNumber = 0;
}

void MidiCombinerDynamic::MidiChannelState::softReset() {
	_hrControllers[MidiDriver_BASE::MIDI_CONTROLLER_MODULATION] = 0;
	_lrControllers[MidiDriver_BASE::MIDI_CONTROLLER_SUSTAIN - kLRControllerStart] = 0;
	_lrControllers[MidiDriver_BASE::MIDI_CONTROLLER_PORTAMENTO - kLRControllerStart] = 0;
	_lrControllers[MidiDriver_BASE::MIDI_CONTROLLER_SOSTENUTO - kLRControllerStart] = 0;
	_lrControllers[MidiDriver_BASE::MIDI_CONTROLLER_SOFT - kLRControllerStart] = 0;
	_dataEntryState = kDataEntryStateNone;
	_rpnNumber = 0;
	_nrpnNumber = 0;
	_aftertouch = 0;
	_hrControllers[MidiDriver_BASE::MIDI_CONTROLLER_EXPRESSION] = (127 << 7);
	_pitchBend = (64 << 7);
}

MidiCombinerDynamic::SourceChannelState::SourceChannelState() {
	reset();
}

void MidiCombinerDynamic::SourceChannelState::reset() {
}

MidiCombinerDynamic::SourceState::SourceState() : _isAllocated(false), _sqrtMasterVolume(0xffffu) {
}

void MidiCombinerDynamic::SourceState::allocate() {
	_isAllocated = true;
}

void MidiCombinerDynamic::SourceState::deallocate() {
	_isAllocated = false;
}

MidiCombinerDynamic::OutputChannelState::OutputChannelState() : _sourceID(0), _volumeIsAmbiguous(true), _channelID(0), _hasSource(false), _noteOffCounter(0), _numActiveNotes(0) {
}

MultiMidiPlayer::MultiMidiPlayer(bool dynamicMidiMixer) {
	if (dynamicMidiMixer)
		_combiner.reset(new MidiCombinerDynamic(this));
	else
		_combiner.reset(new MidiCombinerSimple(this));

	createDriver(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);

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
	_filePlayers.clear();
	_notePlayers.clear();
}

void MultiMidiPlayer::timerCallback(void *refCon) {
	static_cast<MultiMidiPlayer *>(refCon)->onTimer();
}

void MultiMidiPlayer::onTimer() {
	Common::StackLock lock(_mutex);

	for (const Common::SharedPtr<MidiFilePlayerImpl> &player : _filePlayers)
		player->onTimer();

	for (const Common::SharedPtr<MidiNotePlayerImpl> &player : _notePlayers)
		player->onTimer();
}

MidiFilePlayer *MultiMidiPlayer::createFilePlayer(const Common::SharedPtr<Data::Midi::MidiModifier::EmbeddedFile> &file, bool hasTempoOverride, double tempoOverride, uint8 volume, bool loop, uint16 mutedTracks) {
	Common::SharedPtr<MidiCombinerSource> combinerSource = createSource();
	Common::SharedPtr<MidiFilePlayerImpl> filePlayer(new MidiFilePlayerImpl(combinerSource, file, getBaseTempo(), hasTempoOverride, tempoOverride, volume, loop, mutedTracks));

	{
		Common::StackLock lock(_mutex);
		combinerSource->setVolume(volume);
		_filePlayers.push_back(filePlayer);
	}

	return filePlayer.get();
}

MidiNotePlayer *MultiMidiPlayer::createNotePlayer() {
	Common::SharedPtr<MidiCombinerSource> combinerSource = createSource();
	Common::SharedPtr<MidiNotePlayerImpl> notePlayer(new MidiNotePlayerImpl(combinerSource, getBaseTempo()));

	{
		Common::StackLock lock(_mutex);
		_notePlayers.push_back(notePlayer);
	}

	return notePlayer.get();
}

Common::SharedPtr<MidiCombinerSource> MultiMidiPlayer::createSource() {
	Common::StackLock lock(_mutex);
	return _combiner->createSource();
}

void MultiMidiPlayer::deleteFilePlayer(MidiFilePlayer *player) {
	Common::SharedPtr<MidiFilePlayerImpl> ref;

	for (Common::Array<Common::SharedPtr<MidiFilePlayerImpl> >::iterator it = _filePlayers.begin(), itEnd = _filePlayers.end(); it != itEnd; ++it) {
		if (it->get() == player) {
			{
				Common::StackLock lock(_mutex);
				ref = *it;
				_filePlayers.erase(it);
				ref->stop();
			}
			break;
		}
	}

	if (ref)
		ref->detach();
}

void MultiMidiPlayer::deleteNotePlayer(MidiNotePlayer *player) {
	Common::SharedPtr<MidiNotePlayerImpl> ref;

	for (Common::Array<Common::SharedPtr<MidiNotePlayerImpl> >::iterator it = _notePlayers.begin(), itEnd = _notePlayers.end(); it != itEnd; ++it) {
		if (it->get() == player) {
			{
				Common::StackLock lock(_mutex);
				ref = *it;
				_notePlayers.erase(it);
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

void MultiMidiPlayer::setPlayerLoop(MidiFilePlayer *player, bool loop) {
	Common::StackLock lock(_mutex);
	static_cast<MidiFilePlayerImpl *>(player)->setLoop(loop);
}

void MultiMidiPlayer::setPlayerTempo(MidiFilePlayer *player, double tempo) {
	Common::StackLock lock(_mutex);
	static_cast<MidiFilePlayerImpl *>(player)->setTempoOverride(tempo);
}

void MultiMidiPlayer::setPlayerMutedTracks(MidiFilePlayer *player, uint16 mutedTracks) {
	Common::StackLock lock(_mutex);
	static_cast<MidiFilePlayerImpl *>(player)->setMutedTracks(mutedTracks);
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

void MultiMidiPlayer::playNote(MidiNotePlayer *player, uint8 volume, uint8 channel, uint8 program, uint8 note, uint8 velocity, double duration) {
	Common::StackLock lock(_mutex);
	static_cast<MidiNotePlayerImpl *>(player)->play(volume, channel, program, note, velocity, duration);
}

void MultiMidiPlayer::stopNote(MidiNotePlayer *player) {
	Common::StackLock lock(_mutex);
	static_cast<MidiNotePlayerImpl *>(player)->stop();
}

uint32 MultiMidiPlayer::getBaseTempo() const {
	if (_driver)
		return _driver->getBaseTempo();
	return 1;
}

void MultiMidiPlayer::send(uint32 b) {
	if (_driver)
		_driver->send(b);
}

MidiModifier::MidiModifier() : _mode(kModeFile), _volume(100), _mutedTracks(0), /* _singleNoteChannel(0), _singleNoteNote(0), */
							   _plugIn(nullptr), _filePlayer(nullptr), _notePlayer(nullptr) /*, _runtime(nullptr) */ {

	memset(&this->_modeSpecific, 0, sizeof(this->_modeSpecific));
}

MidiModifier::~MidiModifier() {
	if (_filePlayer)
		_plugIn->getMidi()->deleteFilePlayer(_filePlayer);

	if (_notePlayer)
		_plugIn->getMidi()->deleteNotePlayer(_notePlayer);
}

bool MidiModifier::load(const PlugInModifierLoaderContext &context, const Data::Midi::MidiModifier &data) {
	_plugIn = static_cast<MidiPlugIn *>(context.plugIn);

	if (data.executeWhen.type != Data::PlugInTypeTaggedValue::kEvent)
		return false;

	if (!_executeWhen.load(data.executeWhen.value.asEvent))
		return false;

	if (data.terminateWhen.type != Data::PlugInTypeTaggedValue::kEvent)
		return false;

	if (!_terminateWhen.load(data.terminateWhen.value.asEvent))
		return false;

	if (data.embeddedFlag) {
		_mode = kModeFile;
		_embeddedFile = data.embeddedFile;

		_modeSpecific.file.loop = (data.modeSpecific.embedded.loop != 0);
		_modeSpecific.file.overrideTempo = (data.modeSpecific.embedded.overrideTempo != 0);
		_volume = data.modeSpecific.embedded.volume;

		if (data.embeddedFadeIn.type != Data::PlugInTypeTaggedValue::kFloat || data.embeddedFadeOut.type != Data::PlugInTypeTaggedValue::kFloat || data.embeddedTempo.type != Data::PlugInTypeTaggedValue::kFloat)
			return false;

		_modeSpecific.file.fadeIn = data.embeddedFadeIn.value.asFloat.toXPFloat().toDouble();
		_modeSpecific.file.fadeOut = data.embeddedFadeOut.value.asFloat.toXPFloat().toDouble();
		_modeSpecific.file.tempo = data.embeddedTempo.value.asFloat.toXPFloat().toDouble();
	} else {
		_mode = kModeSingleNote;

		if (data.singleNoteDuration.type != Data::PlugInTypeTaggedValue::kFloat)
			return false;

		_modeSpecific.singleNote.channel = data.modeSpecific.singleNote.channel;
		_modeSpecific.singleNote.note = data.modeSpecific.singleNote.note;
		_modeSpecific.singleNote.velocity = data.modeSpecific.singleNote.velocity;
		_modeSpecific.singleNote.program = data.modeSpecific.singleNote.program;
		_modeSpecific.singleNote.duration = data.singleNoteDuration.value.asFloat.toXPFloat().toDouble();

		_volume = 100;
	}

	return true;
}

bool MidiModifier::respondsToEvent(const Event &evt) const {
	return _executeWhen.respondsTo(evt) || _terminateWhen.respondsTo(evt);
}

VThreadState MidiModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_executeWhen.respondsTo(msg->getEvent())) {
		const SubtitleTables &subtitleTables = runtime->getProject()->getSubtitles();
		if (subtitleTables.modifierMapping) {
			const Common::String *subSetIDPtr = subtitleTables.modifierMapping->findSubtitleSetForModifierGUID(getStaticGUID());
			if (subSetIDPtr) {
				// Don't currently support anything except play-on-start subs for MIDI
				SubtitlePlayer subtitlePlayer(runtime, *subSetIDPtr, subtitleTables);
				subtitlePlayer.update(0, 1);
			}
		}

		if (_mode == kModeFile) {
			if (_embeddedFile) {
				debug(2, "MIDI (%x '%s'): Playing embedded file", getStaticGUID(), getName().c_str());

				const double tempo = _modeSpecific.file.overrideTempo ? _modeSpecific.file.tempo : 120.0;
				if (!_filePlayer)
					_filePlayer = _plugIn->getMidi()->createFilePlayer(_embeddedFile, _modeSpecific.file.overrideTempo, tempo, getBoostedVolume(runtime) * 255 / 100, _modeSpecific.file.loop, _mutedTracks);
				_plugIn->getMidi()->playPlayer(_filePlayer);
			} else {
				debug(2, "MIDI (%x '%s'): Digested execute event but don't have anything to play", getStaticGUID(), getName().c_str());
			}
		} else if (_mode == kModeSingleNote) {
			playSingleNote();
		}
	}
	if (_terminateWhen.respondsTo(msg->getEvent())) {
		disable(runtime);
	}

	return kVThreadReturn;
}

void MidiModifier::disable(Runtime *runtime) {
	if (_filePlayer) {
		_plugIn->getMidi()->deleteFilePlayer(_filePlayer);
		_filePlayer = nullptr;
	}
	if (_notePlayer) {
		_plugIn->getMidi()->deleteNotePlayer(_notePlayer);
		_notePlayer = nullptr;
	}
}

void MidiModifier::playSingleNote() {
	if (!_notePlayer)
		_notePlayer = _plugIn->getMidi()->createNotePlayer();
	_plugIn->getMidi()->playNote(_notePlayer, _volume, _modeSpecific.singleNote.channel, _modeSpecific.singleNote.program, _modeSpecific.singleNote.note, _modeSpecific.singleNote.velocity, _modeSpecific.singleNote.duration);
}

void MidiModifier::stopSingleNote() {
	if (_notePlayer)
		_plugIn->getMidi()->stopNote(_notePlayer);
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
		DynamicValueWriteFuncHelper<MidiModifier, &MidiModifier::scriptSetVolume, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "notevelocity") {
		DynamicValueWriteFuncHelper<MidiModifier, &MidiModifier::scriptSetNoteVelocity, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "noteduration") {
		DynamicValueWriteFuncHelper<MidiModifier, &MidiModifier::scriptSetNoteDuration, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "notenum") {
		DynamicValueWriteFuncHelper<MidiModifier, &MidiModifier::scriptSetNoteNum, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "loop") {
		DynamicValueWriteFuncHelper<MidiModifier, &MidiModifier::scriptSetLoop, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "playnote") {
		DynamicValueWriteFuncHelper<MidiModifier, &MidiModifier::scriptSetPlayNote, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "tempo") {
		DynamicValueWriteFuncHelper<MidiModifier, &MidiModifier::scriptSetTempo, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "mutetrack") {
		DynamicValueWriteFuncHelper<MidiModifier, &MidiModifier::scriptSetMuteTrack, true>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return Modifier::writeRefAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome MidiModifier::writeRefAttributeIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib, const DynamicValue &index) {
	if (attrib == "mutetrack") {
		int32 asInteger = 0;
		if (!index.roundToInt(asInteger) || asInteger < 1) {
			thread->error("Invalid index for mutetrack");
			return kMiniscriptInstructionOutcomeFailed;
		}

		result.pod.objectRef = this;
		result.pod.ptrOrOffset = static_cast<uintptr>(asInteger) - 1;
		result.pod.ifc = DynamicValueWriteInterfaceGlue<MuteTrackProxyInterface>::getInstance();

		return kMiniscriptInstructionOutcomeContinue;
	}

	return Modifier::writeRefAttributeIndexed(thread, result, attrib, index);
}

uint MidiModifier::getBoostedVolume(Runtime *runtime) const {
	uint boostedVolume = (_volume * runtime->getHacks().midiVolumeScale) >> 8;
	if (boostedVolume > 100)
		boostedVolume = 100;
	return boostedVolume;
}

Common::SharedPtr<Modifier> MidiModifier::shallowClone() const {
	Common::SharedPtr<MidiModifier> clone(new MidiModifier(*this));

	clone->_notePlayer = nullptr;
	clone->_filePlayer = nullptr;

	return clone;
}

const char *MidiModifier::getDefaultName() const {
	return "MIDI Modifier";
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
			_plugIn->getMidi()->setPlayerVolume(_filePlayer, getBoostedVolume(thread->getRuntime()) * 255 / 100);
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

MiniscriptInstructionOutcome MidiModifier::scriptSetNoteDuration(MiniscriptThread *thread, const DynamicValue &value) {
	double asDouble = 0.0;
	if (value.getType() == DynamicValueTypes::kFloat) {
		asDouble = value.getFloat();
	} else {
		DynamicValue converted;
		if (!value.convertToType(DynamicValueTypes::kFloat, converted))
			return kMiniscriptInstructionOutcomeFailed;
		asDouble = converted.getFloat();
	}

	if (_mode == kModeSingleNote) {
		debug(2, "MIDI (%x '%s'): Changing note duration to %g", getStaticGUID(), getName().c_str(), asDouble);
		_modeSpecific.singleNote.duration = asDouble;
	}

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome MidiModifier::scriptSetNoteNum(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger))
		return kMiniscriptInstructionOutcomeFailed;

	if (asInteger < 0)
		asInteger = 0;
	else if (asInteger > 255)
		asInteger = 255;

	if (_mode == kModeSingleNote) {
		debug(2, "MIDI (%x '%s'): Changing note number to %i", getStaticGUID(), getName().c_str(), asInteger);
		_modeSpecific.singleNote.note = asInteger;
	}

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome MidiModifier::scriptSetLoop(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kBoolean)
		return kMiniscriptInstructionOutcomeFailed;

	if (_mode == kModeFile) {
		const bool loop = value.getBool();

		debug(2, "MIDI (%x '%s'): Changing loop state to %s", getStaticGUID(), getName().c_str(), loop ? "true" : "false");
		if (_modeSpecific.file.loop != loop) {
			_modeSpecific.file.loop = loop;

			if (_filePlayer)
				_plugIn->getMidi()->setPlayerLoop(_filePlayer, loop);
		}
	}

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome MidiModifier::scriptSetTempo(MiniscriptThread *thread, const DynamicValue &value) {
	double tempo = 0.0;
	if (value.getType() == DynamicValueTypes::kInteger)
		tempo = value.getInt();
	else if (value.getType() == DynamicValueTypes::kFloat)
		tempo = value.getFloat();
	else
		return kMiniscriptInstructionOutcomeFailed;

	if (_mode == kModeFile) {
		debug(2, "MIDI (%x '%s'): Changing tempo to %g", getStaticGUID(), getName().c_str(), tempo);

		if (_filePlayer)
			_plugIn->getMidi()->setPlayerTempo(_filePlayer, tempo);
	}

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome MidiModifier::scriptSetPlayNote(MiniscriptThread *thread, const DynamicValue &value) {
	if (miniscriptEvaluateTruth(value))
		playSingleNote();
	else
		stopSingleNote();

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome MidiModifier::scriptSetMuteTrack(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() != DynamicValueTypes::kBoolean) {
		thread->error("Invalid type for mutetrack");
		return kMiniscriptInstructionOutcomeFailed;
	}

	uint16 mutedTracks = value.getBool() ? 0xffffu : 0u;

	if (mutedTracks != _mutedTracks) {
		_mutedTracks = mutedTracks;

		if (_filePlayer)
			_plugIn->getMidi()->setPlayerMutedTracks(_filePlayer, mutedTracks);
	}

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome MidiModifier::scriptSetMuteTrackIndexed(MiniscriptThread *thread, size_t trackIndex, bool muted) {
	if (trackIndex >= 16) {
		thread->error("Invalid track index for mutetrack");
		return kMiniscriptInstructionOutcomeFailed;
	}

	uint16 mutedTracks = _mutedTracks;
	uint16 trackMask = 1 << trackIndex;

	if (muted)
		mutedTracks |= trackMask;
	else
		mutedTracks -= (mutedTracks & trackMask);

	if (mutedTracks != _mutedTracks) {
		_mutedTracks = mutedTracks;

		if (_filePlayer)
			_plugIn->getMidi()->setPlayerMutedTracks(_filePlayer, mutedTracks);
	}

	return kMiniscriptInstructionOutcomeContinue;
}

MiniscriptInstructionOutcome MidiModifier::MuteTrackProxyInterface::write(MiniscriptThread *thread, const DynamicValue &value, void *objectRef, uintptr ptrOrOffset) {
	if (value.getType() != DynamicValueTypes::kBoolean) {
		thread->error("Invalid type for mutetrack");
		return kMiniscriptInstructionOutcomeFailed;
	}

	return static_cast<MidiModifier *>(objectRef)->scriptSetMuteTrackIndexed(thread, ptrOrOffset, value.getBool());
}

MiniscriptInstructionOutcome MidiModifier::MuteTrackProxyInterface::refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib) {
	return kMiniscriptInstructionOutcomeFailed;
}

MiniscriptInstructionOutcome MidiModifier::MuteTrackProxyInterface::refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index) {
	return kMiniscriptInstructionOutcomeFailed;
}

MidiPlugIn::MidiPlugIn(bool useDynamicMidi)
	: _midiModifierFactory(this) {
	_midi.reset(new MultiMidiPlayer(useDynamicMidi));
}

MidiPlugIn::~MidiPlugIn() {
}

void MidiPlugIn::registerModifiers(IPlugInModifierRegistrar *registrar) const {
	registrar->registerPlugInModifier("MIDIModf", &_midiModifierFactory);
}

MultiMidiPlayer *MidiPlugIn::getMidi() const {
	return _midi.get();
}

} // End of namespace Midi

namespace PlugIns {

Common::SharedPtr<PlugIn> createMIDI() {
	const bool useDynamicMidi = ConfMan.getBool("mtropolis_mod_dynamic_midi");

	return Common::SharedPtr<PlugIn>(new Midi::MidiPlugIn(useDynamicMidi));
}

} // namespace PlugIns

} // End of namespace MTropolis
