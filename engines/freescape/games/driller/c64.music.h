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

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/softsynth/sid.h"
#include "common/debug.h"

namespace Freescape {

class DrillerSIDPlayer : public Audio::AudioStream {

	// --- Voice State Structure ---
	struct VoiceState {
		// Pointers & Indices
		const uint8_t *trackDataPtr;   // Pointer to current track data array
		uint8_t trackIndex;            // Index within trackDataPtr
		const uint8_t *patternDataPtr; // Pointer to current pattern data array
		uint8_t patternIndex;          // Index within patternDataPtr
		uint8_t instrumentIndex;       // Current instrument (0-21, scaled by 8 for lookup)

		// Playback Control & Tempo
		int8_t delayCounter;  // Counts down frames for note duration (maps to voiceX_ctrl2)
		uint8_t noteDuration; // Duration set by FD xx (maps to voiceX_something+2)
		uint8_t gateMask;     // Control gating/retriggering (maps to control3 behavior)

		// Note & Frequency
		uint8_t currentNote;     // Current raw note value (0-95)
		uint8_t portaTargetNote; // Target note for portamento
		uint16_t currentFreq;    // Current frequency being sent to SID
		uint16_t baseFreq;       // Note frequency without effects
		uint16_t targetFreq;     // Used for portamento target

		// Pulse Width
		uint16_t pulseWidth; // Current pulse width
		// Placeholder for PWM effects if needed later

		// ADSR
		uint8_t attackDecay;    // SID Attack / Decay register value
		uint8_t sustainRelease; // SID Sustain / Release register value

		// Effects State
		uint8_t effect;                  // Current active effect (0:None, 1:Arpeggio, 2:Vibrato, 3:Portamento Up, 4:Portamento Down, 5: PWM LFO?)
		uint8_t arpeggioIndex;           // Index in arpeggio table
		uint8_t arpeggioSpeed;           // Counter divisor for arpeggio step
		uint8_t arpeggioCounter;         // Counter for arpeggio step
		uint8_t arpeggioNoteOffsetIndex; // 0, 1, 2 for arpeggio notes

		int16_t vibratoDepth;         // Depth for vibrato effect
		uint8_t vibratoSpeed;         // Speed/delay for vibrato effect
		uint8_t vibratoDelay;         // Counter for vibrato step
		uint8_t vibratoDirection;     // 0: up, 1: down
		int16_t vibratoCurrentOffset; // Current frequency offset for vibrato

		int16_t portaSpeed; // Speed for portamento effect (positive for up, negative for down)

		// Hard Restart / Buzz Effect (from L1005, possibly instrument related)
		uint8_t hardRestartValue;   // Value from instrument table (a1+5)
		uint8_t hardRestartDelay;   // Counter for delay phase (voiceX_whatever+3)
		uint8_t hardRestartCounter; // Counter for frequency change phase (voiceX_whatever+4)
		bool hardRestartActive;     // Is the effect currently running?

		// From disassembly variables (mapping might need refinement)
		// voice1_whatever: 0CCE[5] - effect state? (arp, vib, porta)
		uint8_t whatever0; // 0CCE - Vibrato state? (0=off, 1=active, 3-4=sweep?)
		uint8_t whatever1; // 0CCF - Arpeggio state? (0=off, 1=active)
		uint8_t whatever2; // 0CD0 - Portamento type (0=off, 1=down(FB), 2=up(FC), 3=down H(FB), 4=up H(FC))?
		uint8_t whatever3; // 0CD1 - Hard restart delay counter
		uint8_t whatever4; // 0CD2 - Hard restart step counter

		// voice1_whatever2: 0CD4 - Vibrato direction toggle?
		uint8_t whatever2_vibDirToggle;

		// voice1_something: 0CE3[3] - Porta speed?, Note duration
		uint16_t portaStepRaw; // 0CE3/4 - Raw value from FB/FC command
		// uint8_t noteDuration // 0CE5 - Covered above

		// voice1_something_else: 0CE7[3] - PW Low, PW High? (ADSR in disassembly?) - Needs clarification
		uint8_t something_else[3]; // Re-add this array as it's used in the code logic

		// voice1_ctrl0: 0CF8 - ADSR lower nibble (Sustain/Release)
		uint8_t ctrl0; // Re-add this as it's used in the code logic

		// voice1_ctrl1: 0CF9 - Arpeggio table index / Arp speed upper nibble
		uint8_t arpTableIndex;
		uint8_t arpSpeedHiNibble;

		// voice1_ctrl2: 0CFE - Note delay counter (covered by delayCounter)

		// voice1_stuff: 0D14[7] - Current freq, base freq, hard restart freq store? Arp counter?
		uint16_t stuff_freq_porta_vib;    // 0D14/15 - Current frequency including porta/vib
		uint16_t stuff_freq_base;         // 0D16/17 - Base frequency of the note
		uint16_t stuff_freq_hard_restart; // 0D18/19 - Frequency stored during hard restart buzz
		uint8_t stuff_arp_counter;        // 0D1A - Arpeggio counter (0..speed-1)
		uint8_t stuff_arp_note_index;     // 0D1B - Index into arp notes (0, 1, 2)

		// voice1_things: 0D29[7] - Vibrato state/params
		uint8_t things_vib_state;        // 0D29 - Vibrato state (0=down1, 1=up, 2=down2, 3=sweepdown1, 4=sweepup)
		uint16_t things_vib_depth;       // 0D2A/2B - Vibrato depth
		uint8_t things_vib_delay_reload; // 0D2C - Vibrato delay reload value
		uint8_t things_vib_delay_ctr;    // 0D2D - Vibrato delay counter
		// 0D2E/F unused?
		uint8_t currentNoteSlideTarget; // 0D30 - Last played note (used for slide target?)

		// voice1_two_ctr: 0D3E - Glide down timer? (Instrument related)
		uint8_t glideDownTimer;

		// Temp values during processing
		uint8_t waveform; // Current waveform for SID
		bool keyOn;       // Current key state (attack vs release)
		bool sync;        // Sync bit state
		bool ringMod;     // Ring mod bit state

		// Pulse width parts matching something_else (if it maps to PW)
		uint8_t pwLo() const { return something_else[0]; } // Example mapping
		uint8_t pwHi() const { return something_else[2]; } // Example mapping
		void setPwLo(uint8_t val) { something_else[0] = val; }
		void setPwHi(uint8_t val) { something_else[2] = val; }

		void reset() {
			trackDataPtr = nullptr;
			trackIndex = 0;
			patternDataPtr = nullptr;
			patternIndex = 0;
			instrumentIndex = 0;
			delayCounter = 0;
			noteDuration = 0;
			gateMask = 0xFF;
			currentNote = 0;
			portaTargetNote = 0;
			currentFreq = 0;
			baseFreq = 0;
			targetFreq = 0;
			pulseWidth = 0;
			attackDecay = 0;
			sustainRelease = 0;
			effect = 0;
			arpeggioIndex = 0;
			arpeggioSpeed = 0;
			arpeggioCounter = 0;
			arpeggioNoteOffsetIndex = 0;
			vibratoDepth = 0;
			vibratoSpeed = 0;
			vibratoDelay = 0;
			vibratoDirection = 0;
			vibratoCurrentOffset = 0;
			portaSpeed = 0;
			hardRestartValue = 0;
			hardRestartDelay = 0;
			hardRestartCounter = 0;
			hardRestartActive = false;
			waveform = 0x10; // Default to triangle?
			keyOn = false;
			sync = false;
			ringMod = false;

			// Reset mapped vars
			whatever0 = 0;
			whatever1 = 0;
			whatever2 = 0;
			whatever3 = 0;
			whatever4 = 0;
			whatever2_vibDirToggle = 0;
			portaStepRaw = 0;
			memset(something_else, 0, sizeof(something_else)); // Reset the array
			ctrl0 = 0;                                         // Reset the added member
			arpTableIndex = 0;
			arpSpeedHiNibble = 0;
			stuff_freq_porta_vib = 0;
			stuff_freq_base = 0;
			stuff_freq_hard_restart = 0;
			stuff_arp_counter = 0;
			stuff_arp_note_index = 0;
			things_vib_state = 0;
			things_vib_depth = 0;
			things_vib_delay_reload = 0;
			things_vib_delay_ctr = 0;
			currentNoteSlideTarget = 0;
			glideDownTimer = 0;
		}
	};

	// --- Member Variables ---
	Resid::SID *_sid;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle; // Changed from pointer
	int _sampleRate;
	float _cyclesPerSample;
	double _cycleCounter;

	// Player State
	enum PlayState { STOPPED,
					 PLAYING,
					 CHANGING_TUNE };
	PlayState _playState;
	uint8_t _targetTuneIndex; // Tune index requested via startMusic

	// Global Timing
	uint8_t _globalTempo;      // Tempo value for current tune (0xD10)
	int8_t _globalTempoCounter; // Frame counter for tempo (0xD12), signed to handle < 0 check
	uint8_t _framePhase;       // Tracks which voice is being processed (0, 7, 14)

	// Voice States
	VoiceState _voiceState[3];

	// Internal helpers
	uint8_t _tempControl3; // Temporary storage for gate mask (0xD13)
	// uint8_t _tempControl1; // Temp storage from instrument data (0xD11)

public:
	DrillerSIDPlayer(Audio::Mixer *mixer);
	~DrillerSIDPlayer();
	void startMusic(int tuneIndex = 1);
	void stopMusic();

	int readBuffer(int16 *buffer, const int numSamples) override;

	bool isStereo() const override { return false; }
	bool endOfData() const override { return false; }
	int getRate() const override { return _sampleRate; }

private:
	void SID_Write(int reg, uint8_t data);
	void initSID();
	void playFrame();
	void handleChangeTune(int tuneIndex);
	void handleResetVoices();
	void playVoice(int voiceIndex);
	void applyNote(VoiceState &v, int sidOffset, const uint8_t *instA0, const uint8_t *instA1, int voiceIndex);
	void applyContinuousEffects(VoiceState &v, int sidOffset, const uint8_t *instA0, const uint8_t *instA1);
	void applyHardRestart(VoiceState &v, int sidOffset, const uint8_t *instA0, const uint8_t *instA1);
};

} // namespace Freescape
