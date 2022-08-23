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

#include "common/random.h"
#include "common/config-manager.h"
#include "common/file.h"

#include "audio/mididrv.h"
#include "audio/midiplayer.h"
#include "audio/midiparser.h"
#include "audio/midiparser_smf.h"

#include "mtropolis/miniscript.h"
#include "mtropolis/plugin/standard.h"
#include "mtropolis/plugins.h"

#include "mtropolis/miniscript.h"

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
	explicit MidiFilePlayerImpl(const Common::SharedPtr<MidiCombinerSource> &outputDriver, const Common::SharedPtr<Data::Standard::MidiModifier::EmbeddedFile> &file, uint32 baseTempo, bool hasTempoOverride, double tempoOverride, uint8 volume, bool loop, uint16 mutedTracks);
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
	Common::SharedPtr<Data::Standard::MidiModifier::EmbeddedFile> _file;
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
	//uint8 _program;

	bool _initialized;
};

class MultiMidiPlayer : public Audio::MidiPlayer {
public:
	explicit MultiMidiPlayer(bool useDynamicMidiMixer);
	~MultiMidiPlayer();

	MidiFilePlayer *createFilePlayer(const Common::SharedPtr<Data::Standard::MidiModifier::EmbeddedFile> &file, bool hasTempoOverride, double tempoOverride, uint8 volume, bool loop, uint16 mutedTracks);
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

MidiFilePlayerImpl::MidiFilePlayerImpl(const Common::SharedPtr<MidiCombinerSource> &outputDriver, const Common::SharedPtr<Data::Standard::MidiModifier::EmbeddedFile> &file, uint32 baseTempo, bool hasTempoOverride, double tempo, uint8 volume, bool loop, uint16 mutedTracks)
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
	: _timerRate(timerRate), _durationRemaining(0), _outputDriver(outputDriver), _channel(0), _note(0), /* _program(0), */_initialized(false), _volume(100) {
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

	// GM volume scale to linear is x^4 so we need the 4th root of the volume, and need to rescale it from 0-100 to 0-0x3f80
	// = 0x3f80 / sqrt(sqrt(100))
	const double volumeMultiplier = 5140.5985643697174420974013458299;

	if (volume > 100)
		volume = 100;
	uint16 hpVolume = static_cast<uint16>(floor(sqrt(sqrt(volume)) * volumeMultiplier));

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
		void softReset();	// Executes changes corresponding to Reset All Controllers message

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
		uint16 _root4MasterVolume;
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
	assert(_combiner == nullptr);	// Call detach first!
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
	src._root4MasterVolume = static_cast<uint16>(floor(sqrt(sqrt(volume)) * 16400.0));

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
	ch._noteOffCounter = 0;	// All sound is off so this can be recycled quickly
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
		// To modulate the volume linearly, we must multiply the volume by the 4th root
		// of the volume.
		uint32 effectiveValueScaled = static_cast<uint32>(srcState._root4MasterVolume) * static_cast<uint32>(effectiveValue);
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

MidiCombinerDynamic::SourceState::SourceState() : _isAllocated(false), _root4MasterVolume(0xffffu) {
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

MidiFilePlayer *MultiMidiPlayer::createFilePlayer(const Common::SharedPtr<Data::Standard::MidiModifier::EmbeddedFile> &file, bool hasTempoOverride, double tempoOverride, uint8 volume, bool loop, uint16 mutedTracks) {
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

CursorModifier::CursorModifier() : _cursorID(0) {
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
		// This doesn't call "disable" because the behavior is actually different.
		// Disabling a cursor modifier doesn't seem to remove it.
		runtime->clearModifierCursorOverride();
	}
	return kVThreadReturn;
}

void CursorModifier::disable(Runtime *runtime) {
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

const char *CursorModifier::getDefaultName() const {
	return "Cursor Modifier";
}

STransCtModifier::STransCtModifier() : _transitionType(0), _transitionDirection(0), _steps(0), _duration(0), _fullScreen(false) {
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

bool STransCtModifier::respondsToEvent(const Event &evt) const {
	return _enableWhen.respondsTo(evt) || _disableWhen.respondsTo(evt);
}

VThreadState STransCtModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_enableWhen.respondsTo(msg->getEvent())) {
		SceneTransitionEffect effect;
		effect._duration = _duration / 10;
		effect._steps = _steps;

		if (SceneTransitionTypes::loadFromData(effect._transitionType, _transitionType) && SceneTransitionDirections::loadFromData(effect._transitionDirection, _transitionDirection)) {
			// Duration doesn't seem to affect wipe transitions for some reason.
			// In Obsidian, this mostly effects 180-degree turns.
			// Good place to test this is in the corners of the Bureau library, where it's 0,
			// but some cases where it is set (e.g. the Spider control room) have the same duration anyway.
			if (effect._transitionType == SceneTransitionTypes::kWipe)
				effect._duration = 500;

			runtime->setSceneTransitionEffect(false, &effect);
		} else {
			warning("Source-scene transition had invalid data");
		}
	}
	if (_disableWhen.respondsTo(msg->getEvent()))
		disable(runtime);

	return kVThreadReturn;
}

void STransCtModifier::disable(Runtime *runtime) {
	runtime->setSceneTransitionEffect(false, nullptr);
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
}


Common::SharedPtr<Modifier> STransCtModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new STransCtModifier(*this));
}

const char *STransCtModifier::getDefaultName() const {
	return "STransCt";	// Probably wrong
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

MediaCueMessengerModifier::CueSourceUnion::CueSourceUnion() : asUnset(0) {
}

MediaCueMessengerModifier::CueSourceUnion::~CueSourceUnion() {
}

template<class T, T (MediaCueMessengerModifier::CueSourceUnion::*TMember)>
void MediaCueMessengerModifier::CueSourceUnion::construct(const T &value) {
	T *field = &(this->*TMember);
	new (field) T(value);
}

template<class T, T (MediaCueMessengerModifier::CueSourceUnion::*TMember)>
void MediaCueMessengerModifier::CueSourceUnion::destruct() {
	T *field = &(this->*TMember);
	field->~T();
}

MediaCueMessengerModifier::MediaCueMessengerModifier() : _isActive(false), _cueSourceType(kCueSourceInvalid) {
	_mediaCue.sourceModifier = this;
}

MediaCueMessengerModifier::~MediaCueMessengerModifier() {
	switch (_cueSourceType) {
	case kCueSourceInteger:
		_cueSource.destruct<int32, &CueSourceUnion::asInt>();
		break;
	case kCueSourceIntegerRange:
		_cueSource.destruct<IntRange, &CueSourceUnion::asIntRange>();
		break;
	case kCueSourceVariableReference:
		_cueSource.destruct<uint32, &CueSourceUnion::asVarRefGUID>();
		break;
	case kCueSourceLabel:
		_cueSource.destruct<Label, &CueSourceUnion::asLabel>();
		break;
	default:
		_cueSource.destruct<uint64, &CueSourceUnion::asUnset>();
		break;
	}
}

bool MediaCueMessengerModifier::load(const PlugInModifierLoaderContext &context, const Data::Standard::MediaCueMessengerModifier &data) {
	if (data.enableWhen.type != Data::PlugInTypeTaggedValue::kEvent)
		return false;

	if (!_enableWhen.load(data.enableWhen.value.asEvent))
		return false;

	if (data.disableWhen.type != Data::PlugInTypeTaggedValue::kEvent)
		return false;

	if (!_disableWhen.load(data.disableWhen.value.asEvent))
		return false;

	if (data.triggerTiming.type != Data::PlugInTypeTaggedValue::kInteger)
		return false;

	_mediaCue.triggerTiming = static_cast<MediaCueState::TriggerTiming>(data.triggerTiming.value.asInt);

	if (data.nonStandardMessageFlags.type != Data::PlugInTypeTaggedValue::kInteger)
		return false;

	int32 msgFlags = data.nonStandardMessageFlags.value.asInt;

	MessageFlags messageFlags;
	messageFlags.immediate = ((msgFlags & Data::Standard::MediaCueMessengerModifier::kMessageFlagImmediate) != 0);
	messageFlags.cascade = ((msgFlags & Data::Standard::MediaCueMessengerModifier::kMessageFlagCascade) != 0);
	messageFlags.relay = ((msgFlags & Data::Standard::MediaCueMessengerModifier::kMessageFlagRelay) != 0);
	if (!_mediaCue.send.load(data.sendEvent, messageFlags, data.with, data.destination))
		return false;

	switch (data.executeAt.type) {
	case Data::PlugInTypeTaggedValue::kInteger:
		_cueSourceType = kCueSourceInteger;
		_cueSource.asInt = data.executeAt.value.asInt;
		break;
	case Data::PlugInTypeTaggedValue::kIntegerRange:
		_cueSourceType = kCueSourceIntegerRange;
		if (!_cueSource.asIntRange.load(data.executeAt.value.asIntRange))
			return false;
		break;
	case Data::PlugInTypeTaggedValue::kVariableReference:
		_cueSourceType = kCueSourceVariableReference;
		_cueSource.asVarRefGUID = data.executeAt.value.asVarRefGUID;
		break;
	case Data::PlugInTypeTaggedValue::kLabel:
		_cueSourceType = kCueSourceLabel;
		if (!_cueSource.asLabel.load(data.executeAt.value.asLabel))
			return false;
		break;
	default:
		return false;
	}

	return true;
}

bool MediaCueMessengerModifier::respondsToEvent(const Event &evt) const {
	return _enableWhen.respondsTo(evt) || _disableWhen.respondsTo(evt);
}

VThreadState MediaCueMessengerModifier::consumeMessage(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (_enableWhen.respondsTo(msg->getEvent())) {
		Structural *owner = findStructuralOwner();
		if (owner && owner->isElement()) {

			Element *element = static_cast<Element *>(owner);

			switch (_cueSourceType) {
			case kCueSourceInteger:
				_mediaCue.minTime = _mediaCue.maxTime = _cueSource.asInt;
				break;
			case kCueSourceIntegerRange:
				_mediaCue.minTime = _cueSource.asIntRange.min;
				_mediaCue.maxTime = _cueSource.asIntRange.max;
				break;
			case kCueSourceLabel: {
					int32 resolved = 0;
					if (element->resolveMediaMarkerLabel(_cueSource.asLabel, resolved))
						_mediaCue.minTime = _mediaCue.maxTime = resolved;
					else {
						warning("Failed to resolve media cue marker label");
						return kVThreadError;
					}
				} break;
			case kCueSourceVariableReference: {
					Modifier *modifier = _cueSourceModifier.lock().get();
					if (!modifier->isVariable()) {
						warning("Media cue source variable couldn't be resolved");
						return kVThreadReturn;
					}

					DynamicValue value;
					static_cast<VariableModifier *>(modifier)->varGetValue(nullptr, value);

					switch (value.getType()) {
					case DynamicValueTypes::kInteger:
						_mediaCue.minTime = _mediaCue.maxTime = value.getInt();
						break;
					case DynamicValueTypes::kIntegerRange:
						_mediaCue.minTime = value.getIntRange().min;
						_mediaCue.maxTime = value.getIntRange().max;
						break;
					case DynamicValueTypes::kFloat:
						_mediaCue.minTime = _mediaCue.maxTime = static_cast<int32>(round(value.getFloat()));
						break;
					default:
						warning("Media cue variable was not a usable type");
						return kVThreadError;
					}

				} break;
			default:
				assert(false);	// Something wasn't handled in the loader
				return kVThreadReturn;
			}

			element->addMediaCue(&_mediaCue);
			_isActive = true;
		}
	}
	if (_disableWhen.respondsTo(msg->getEvent())) {
		disable(runtime);
	}

	return kVThreadReturn;
}

void MediaCueMessengerModifier::disable(Runtime *runtime) {
	if (_isActive) {
		Structural *owner = findStructuralOwner();
		if (owner && owner->isElement())
			static_cast<Element *>(owner)->removeMediaCue(&_mediaCue);

		_isActive = false;
	}
}

Common::SharedPtr<Modifier> MediaCueMessengerModifier::shallowClone() const {
	Common::SharedPtr<MediaCueMessengerModifier> clone(new MediaCueMessengerModifier(*this));
	clone->_isActive = false;
	clone->_mediaCue.sourceModifier = clone.get();
	clone->_mediaCue.incomingData = DynamicValue();
	return clone;
}

const char *MediaCueMessengerModifier::getDefaultName() const {
	return "Media Cue Messenger";
}

void MediaCueMessengerModifier::linkInternalReferences(ObjectLinkingScope *scope) {
	if (_cueSourceType == kCueSourceVariableReference) {
		Common::WeakPtr<RuntimeObject> obj = scope->resolve(_cueSource.asVarRefGUID);
		RuntimeObject *objPtr = obj.lock().get();
		if (objPtr && objPtr->isModifier())
			_cueSourceModifier = obj.staticCast<Modifier>();
	}

	_mediaCue.send.linkInternalReferences(scope);
}

void MediaCueMessengerModifier::visitInternalReferences(IStructuralReferenceVisitor *visitor) {
	visitor->visitWeakModifierRef(_cueSourceModifier);
	_mediaCue.send.visitInternalReferences(visitor);
}

ObjectReferenceVariableModifier::ObjectReferenceVariableModifier() {
}

bool ObjectReferenceVariableModifier::load(const PlugInModifierLoaderContext &context, const Data::Standard::ObjectReferenceVariableModifier &data) {
	if (data.setToSourceParentWhen.type != Data::PlugInTypeTaggedValue::kEvent)
		return false;

	if (!_setToSourceParentWhen.load(data.setToSourceParentWhen.value.asEvent))
		return false;

	if (data.objectPath.type == Data::PlugInTypeTaggedValue::kString)
		_objectPath = data.objectPath.str;
	else if (data.objectPath.type != Data::PlugInTypeTaggedValue::kNull)
		return false;

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
			resolve(thread->getRuntime());

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
		result.pod.ptrOrOffset = 0;
		result.pod.objectRef = this;
		result.pod.ifc = DynamicValueWriteInterfaceGlue<ObjectWriteInterface>::getInstance();
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

#ifdef MTROPOLIS_DEBUG_ENABLE
void ObjectReferenceVariableModifier::debugInspect(IDebugInspectionReport *report) const {
	VariableModifier::debugInspect(report);

	report->declareDynamic("path", _objectPath);
	report->declareDynamic("fullPath", _fullPath);
}
#endif

Common::SharedPtr<Modifier> ObjectReferenceVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new ObjectReferenceVariableModifier(*this));
}

const char *ObjectReferenceVariableModifier::getDefaultName() const {
	return "Object Reference Variable";
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

MiniscriptInstructionOutcome ObjectReferenceVariableModifier::scriptObjectRefAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, const Common::String &attrib) {
	resolve(thread->getRuntime());

	if (_object.object.expired()) {
		thread->error("Attempted to reference an attribute of an object variable object, but the reference is dead");
		return kMiniscriptInstructionOutcomeFailed;
	}

	return _object.object.lock()->writeRefAttribute(thread, proxy, attrib);
}

MiniscriptInstructionOutcome ObjectReferenceVariableModifier::scriptObjectRefAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, const Common::String &attrib, const DynamicValue &index) {
	resolve(thread->getRuntime());

	if (_object.object.expired()) {
		thread->error("Attempted to reference an attribute of an object variable object, but the reference is dead");
		return kMiniscriptInstructionOutcomeFailed;
	}

	return _object.object.lock()->writeRefAttributeIndexed(thread, proxy, attrib, index);
}

void ObjectReferenceVariableModifier::resolve(Runtime *runtime) {
	if (!_object.object.expired())
		return;

	_fullPath.clear();
	_object.reset();

	if (_objectPath.size() == 0)
		return;

	if (_objectPath[0] == '/')
		resolveAbsolutePath(runtime);
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

void ObjectReferenceVariableModifier::resolveAbsolutePath(Runtime *runtime) {
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

	size_t prefixEnd = 0;

	bool foundPrefix = false;

	if (runtime->getHacks().ignoreMismatchedProjectNameInObjectLookups) {
		size_t slashOffset = _objectPath.findFirstOf('/', 1);
		if (slashOffset != Common::String::npos) {
			prefixEnd = slashOffset;
			foundPrefix = true;
		}
	} else {
		Common::String projectPrefixes[2] = {
			"/" + static_cast<Structural *>(project)->getName(),
			"/<project>"};

		for (const Common::String &prefix : projectPrefixes) {
			if (_objectPath.size() >= prefix.size() && caseInsensitiveEqual(_objectPath.substr(0, prefix.size()), prefix)) {
				prefixEnd = prefix.size();
				foundPrefix = true;
				break;
			}
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

MiniscriptInstructionOutcome ObjectReferenceVariableModifier::ObjectWriteInterface::write(MiniscriptThread *thread, const DynamicValue &value, void *objectRef, uintptr ptrOrOffset) {
	return static_cast<ObjectReferenceVariableModifier *>(objectRef)->scriptSetObject(thread, value);
}

MiniscriptInstructionOutcome ObjectReferenceVariableModifier::ObjectWriteInterface::refAttrib(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib) {
	return static_cast<ObjectReferenceVariableModifier *>(objectRef)->scriptObjectRefAttrib(thread, proxy, attrib);
}

MiniscriptInstructionOutcome ObjectReferenceVariableModifier::ObjectWriteInterface::refAttribIndexed(MiniscriptThread *thread, DynamicValueWriteProxy &proxy, void *objectRef, uintptr ptrOrOffset, const Common::String &attrib, const DynamicValue &index) {
	return static_cast<ObjectReferenceVariableModifier *>(objectRef)->scriptObjectRefAttribIndexed(thread, proxy, attrib, index);
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

bool ObjectReferenceVariableModifier::SaveLoad::loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) {
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

bool MidiModifier::load(const PlugInModifierLoaderContext &context, const Data::Standard::MidiModifier &data) {
	_plugIn = static_cast<StandardPlugIn *>(context.plugIn);

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

		if (data.embeddedFadeIn.type != Data::PlugInTypeTaggedValue::kFloat
			|| data.embeddedFadeOut.type != Data::PlugInTypeTaggedValue::kFloat
			|| data.embeddedTempo.type != Data::PlugInTypeTaggedValue::kFloat)
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
		DynamicValueWriteFuncHelper<MidiModifier, &MidiModifier::scriptSetVolume>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "notevelocity") {
		DynamicValueWriteFuncHelper<MidiModifier, &MidiModifier::scriptSetNoteVelocity>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "noteduration") {
		DynamicValueWriteFuncHelper<MidiModifier, &MidiModifier::scriptSetNoteDuration>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "notenum") {
		DynamicValueWriteFuncHelper<MidiModifier, &MidiModifier::scriptSetNoteNum>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "loop") {
		DynamicValueWriteFuncHelper<MidiModifier, &MidiModifier::scriptSetLoop>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "playnote") {
		DynamicValueWriteFuncHelper<MidiModifier, &MidiModifier::scriptSetPlayNote>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "tempo") {
		DynamicValueWriteFuncHelper<MidiModifier, &MidiModifier::scriptSetTempo>::create(this, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "mutetrack") {
		DynamicValueWriteFuncHelper<MidiModifier, &MidiModifier::scriptSetMuteTrack>::create(this, result);
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

ListVariableModifier::ListVariableModifier() : _list(new DynamicList()), _preferredContentType(DynamicValueTypes::kInteger) {
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

	_preferredContentType = expectedType;

	return true;
}

Common::SharedPtr<ModifierSaveLoad> ListVariableModifier::getSaveLoad() {
	return Common::SharedPtr<ModifierSaveLoad>(new SaveLoad(this));
}

bool ListVariableModifier::varSetValue(MiniscriptThread *thread, const DynamicValue &value) {
	if (value.getType() == DynamicValueTypes::kList)
		_list = value.getList()->clone();
	else {
		if (!_list)
			_list.reset(new DynamicList());
		return _list->setAtIndex(0, value);
	}

	return true;
}

void ListVariableModifier::varGetValue(MiniscriptThread *thread, DynamicValue &dest) const {
	dest.setList(_list);
}

bool ListVariableModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "count") {
		result.setInt(_list->getSize());
		return true;
	} else if (attrib == "random") {
		if (_list->getSize() == 0)
			return false;

		size_t index = thread->getRuntime()->getRandom()->getRandomNumber(_list->getSize() - 1);
		return _list->getAtIndex(index, result);
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

MiniscriptInstructionOutcome ListVariableModifier::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	if (attrib == "count") {
		DynamicValueWriteFuncHelper<ListVariableModifier, &ListVariableModifier::scriptSetCount>::create(this, writeProxy);
		return kMiniscriptInstructionOutcomeContinue;
	}

	return VariableModifier::writeRefAttribute(thread, writeProxy, attrib);
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

#ifdef MTROPOLIS_DEBUG_ENABLE
void ListVariableModifier::debugInspect(IDebugInspectionReport *report) const {
	VariableModifier::debugInspect(report);

	size_t listSize = _list->getSize();

	for (size_t i = 0; i < listSize; i++) {
		int cardinal = i + 1;
		switch (_list->getType()) {
		case DynamicValueTypes::kInteger:
			report->declareLoose(Common::String::format("[%i] = %i", cardinal, _list->getInt()[i]));
			break;
		case DynamicValueTypes::kFloat:
			report->declareLoose(Common::String::format("[%i] = %g", cardinal, _list->getFloat()[i]));
			break;
		case DynamicValueTypes::kPoint:
			report->declareLoose(Common::String::format("[%i] = ", cardinal) + pointToString(_list->getPoint()[i]));
			break;
		case DynamicValueTypes::kIntegerRange:
			report->declareLoose(Common::String::format("[%i] = ", cardinal) + _list->getIntRange()[i].toString());
			break;
		case DynamicValueTypes::kBoolean:
			report->declareLoose(Common::String::format("[%i] = %s", cardinal, _list->getBool()[i] ? "true" : "false"));
			break;
		case DynamicValueTypes::kVector:
			report->declareLoose(Common::String::format("[%i] = ", cardinal) + _list->getVector()[i].toString());
			break;
		case DynamicValueTypes::kLabel:
			report->declareLoose(Common::String::format("[%i] = Label?", cardinal));
			break;
		case DynamicValueTypes::kEvent:
			report->declareLoose(Common::String::format("[%i] = Event?", cardinal));
			break;
		case DynamicValueTypes::kVariableReference:
			report->declareLoose(Common::String::format("[%i] = VarRef?", cardinal));
			break;
		case DynamicValueTypes::kIncomingData:
			report->declareLoose(Common::String::format("[%i] = IncomingData??", cardinal));
			break;
		case DynamicValueTypes::kString:
			report->declareLoose(Common::String::format("[%i] = ", cardinal) + _list->getString()[i]);
			break;
		case DynamicValueTypes::kList:
			report->declareLoose(Common::String::format("[%i] = List", cardinal));
			break;
		case DynamicValueTypes::kObject:
			report->declareLoose(Common::String::format("[%i] = Object?", cardinal));
			break;
		default:
			report->declareLoose(Common::String::format("[%i] = <BAD TYPE>", cardinal));
			break;
		}
	}
}
#endif

ListVariableModifier::ListVariableModifier(const ListVariableModifier &other) : _preferredContentType(DynamicValueTypes::kNull) {
	if (other._list)
		_list = other._list->clone();
}

MiniscriptInstructionOutcome ListVariableModifier::scriptSetCount(MiniscriptThread *thread, const DynamicValue &value) {
	int32 asInteger = 0;
	if (!value.roundToInt(asInteger)) {
		thread->error("Tried to set a list variable count to something other than an integer");
		return kMiniscriptInstructionOutcomeFailed;
	}

	if (asInteger < 0) {
		thread->error("Tried to set a list variable count to a negative value");
		return kMiniscriptInstructionOutcomeFailed;
	}

	size_t newSize = asInteger;
	if (newSize > _list->getSize()) {
		if (_list->getSize() == 0) {
			thread->error("Restoring an empty list by setting its count isn't implemented");
			return kMiniscriptInstructionOutcomeFailed;
		}

		_list->expandToMinimumSize(newSize);
	} else if (newSize < _list->getSize()) {
		_list->truncateToSize(newSize);
	}

	return kMiniscriptInstructionOutcomeContinue;
}

Common::SharedPtr<Modifier> ListVariableModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new ListVariableModifier(*this));
}

const char *ListVariableModifier::getDefaultName() const {
	return "List Variable";
}

ListVariableModifier::SaveLoad::SaveLoad(ListVariableModifier *modifier) : _modifier(modifier), _list(_modifier->_list) {
}

void ListVariableModifier::SaveLoad::commitLoad() const {
	_modifier->_list = _list;
}

void ListVariableModifier::SaveLoad::saveInternal(Common::WriteStream *stream) const {
	recursiveWriteList(_list.get(), stream);
}

bool ListVariableModifier::SaveLoad::loadInternal(Common::ReadStream *stream, uint32 saveFileVersion) {
	Common::SharedPtr<DynamicList> list = recursiveReadList(stream);
	if (list) {
		_list = list;
		return true;
	} else {
		return false;
	}
}

void ListVariableModifier::SaveLoad::recursiveWriteList(DynamicList *list, Common::WriteStream *stream) {
	stream->writeUint32BE(list->getType());
	stream->writeUint32BE(list->getSize());

	size_t listSize = list->getSize();
	for (size_t i = 0; i < listSize; i++) {
		switch (list->getType()) {
		case DynamicValueTypes::kInteger:
			stream->writeSint32BE(list->getInt()[i]);
			break;
		case DynamicValueTypes::kPoint: {
				const Common::Point &pt = list->getPoint()[i];
				stream->writeSint16BE(pt.x);
				stream->writeSint16BE(pt.y);
			}
			break;
		case DynamicValueTypes::kIntegerRange: {
				const IntRange &range = list->getIntRange()[i];
				stream->writeSint32BE(range.min);
				stream->writeSint32BE(range.max);
			} break;
		case DynamicValueTypes::kFloat:
			stream->writeDoubleBE(list->getFloat()[i]);
			break;
		case DynamicValueTypes::kString: {
				const Common::String &str = list->getString()[i];
				stream->writeUint32BE(str.size());
				stream->writeString(str);
			} break;
		case DynamicValueTypes::kVector: {
				const AngleMagVector &vec = list->getVector()[i];
				stream->writeDoubleBE(vec.angleDegrees);
				stream->writeDoubleBE(vec.magnitude);
			} break;
		case DynamicValueTypes::kBoolean:
			stream->writeByte(list->getBool()[i] ? 1 : 0);
			break;
		case DynamicValueTypes::kList:
			recursiveWriteList(list->getList()[i].get(), stream);
			break;
		default:
			error("Can't figure out how to write a saved variable");
			break;
		}
	}
}

Common::SharedPtr<DynamicList> ListVariableModifier::SaveLoad::recursiveReadList(Common::ReadStream *stream) {
	Common::SharedPtr<DynamicList> list;
	list.reset(new DynamicList());

	uint32 typeCode = stream->readUint32BE();
	uint32 size = stream->readUint32BE();

	if (stream->err())
		return nullptr;

	for (size_t i = 0; i < size; i++) {
		DynamicValue val;

		switch (typeCode) {
		case DynamicValueTypes::kInteger: {
				int32 i32 = stream->readSint32BE();
				val.setInt(i32);
			} break;
		case DynamicValueTypes::kPoint: {
				Common::Point pt;
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
					return nullptr;

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
				vec.angleDegrees = stream->readDoubleBE();
				vec.magnitude = stream->readDoubleBE();
				val.setVector(vec);
			} break;
		case DynamicValueTypes::kBoolean: {
				byte b = stream->readByte();
				val.setBool(b != 0);
			} break;
		case DynamicValueTypes::kList: {
				Common::SharedPtr<DynamicList> childList = recursiveReadList(stream);
				if (!childList)
					return nullptr;
				val.setList(childList);
			} break;
		default:
			error("Can't figure out how to write a saved variable");
			break;
		}

		if (stream->err())
			return nullptr;

		list->setAtIndex(i, val);
	}

	return list;
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

		Common::Point hacksSize = thread->getRuntime()->getHacks().reportDisplaySize;
		if (hacksSize.x != 0)
			width = hacksSize.x;
		if (hacksSize.y != 0)
			height = hacksSize.y;

		result.setPoint(Common::Point(width, height));
		return true;
	}

	return false;
}


Common::SharedPtr<Modifier> SysInfoModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new SysInfoModifier(*this));
}

const char *SysInfoModifier::getDefaultName() const {
	return "SysInfo Modifier";
}

StandardPlugInHacks::StandardPlugInHacks() : allowGarbledListModData(false) {
}

StandardPlugIn::StandardPlugIn(bool useDynamicMidi)
	: _cursorModifierFactory(this)
	, _sTransCtModifierFactory(this)
	, _mediaCueModifierFactory(this)
	, _objRefVarModifierFactory(this)
	, _midiModifierFactory(this)
	, _listVarModifierFactory(this)
	, _sysInfoModifierFactory(this) {
	_midi.reset(new MultiMidiPlayer(useDynamicMidi));
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

StandardPlugInHacks &StandardPlugIn::getHacks() {
	return _hacks;
}

MultiMidiPlayer *StandardPlugIn::getMidi() const {
	return _midi.get();
}

} // End of namespace Standard

namespace PlugIns {

Common::SharedPtr<PlugIn> createStandard() {
	const bool useDynamicMidi = ConfMan.getBool("mtropolis_mod_dynamic_midi");

	return Common::SharedPtr<PlugIn>(new Standard::StandardPlugIn(useDynamicMidi));
}

} // End of namespace MTropolis

} // End of namespace MTropolis
