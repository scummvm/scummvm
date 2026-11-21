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

#ifndef HOLLYWOOD_RUNAWAY_SEQUENCE_H
#define HOLLYWOOD_RUNAWAY_SEQUENCE_H

#include "common/random.h"
#include "engines/engine.h"
#include "hollywood/resource.h"
#include "hollywood/runaway/audio.h"
#include "hollywood/runaway/graphics.h"
#include "hollywood/runaway/speech.h"
#include "hollywood/runaway/video.h"
#include "hollywood/scene_text_store.h"

namespace Hollywood {

// Retail films interleave movie ranges, separately stored talking portraits,
// and speech/effects. The scene supplies the original sequence of cues.
class RunawaySequence {
public:
	enum Operation {
		kOpen,        // Movie number; optional file suffix (defaults to .001).
		kPlay,        // First/last frame, Playback mode.
		kStill,       // Frame; second argument selects the separate GOG stills movie.
		kImage,       // A 1024x600 image from RESOURCE.000.
		kTalk,        // Cue, variation, portrait chunk + 1 (0 = movie still), offscreen speaker.
		kSpeech,      // Cue, first/last variation, global cue; continues alongside the movie.
		kWaitSpeech,  // Optional portrait and offscreen speaker, as in kTalk.
		kWaitInput,
		kMusic,       // Cue, volume, loop.
		kMusicVolume, // Change volume without restarting the current music.
		kEffect,      // Cue, volume, channel, loop.
		kWaitEffect,  // Maximum wait in milliseconds (channel 0); second argument blanks the screen.
		kStopEffects, // Optional channel + 1; zero stops all effects.
		kDelay,       // Milliseconds.
		kEnd
	};
	enum Playback { kOnce, kLoopDuringSpeech, kIdleDuringSpeech, kUntilSpeechEnds };
	struct Command {
		constexpr Command(Operation op, uint16 first = 0, uint16 second = 0, byte third = 0, byte fourth = 0) :
			operation(op), a(first), b(second), c(third), d(fourth) {}
		Operation operation;
		uint16 a, b;
		byte c, d;
	};
	explicit RunawaySequence(RunawayAudio &audio) : _audio(audio), _random("runawaySequence"), _speech(audio, _random) {}
	void start(const Command *commands, const SceneTextStore &text, uint32 now, char chapter = 'B', bool keepEffects = false);
	void setBackground(const Graphics::ManagedSurface &background, const Common::Rect &destination);
	void update(uint32 now);
	void skip();
	void skipSpeech();
	void draw(Graphics::ManagedSurface &screen);
	bool active() const { return _commands != nullptr; }
	bool skipped() const { return _skipFilm; }
	const Common::U32String &subtitle() const { return _speech.subtitle(); }
	byte speechFrame() const { return _speech.frame(_now); }

private:
	void advance();
	void speak(uint cue, uint variation, uint lastVariation = 0, bool global = false);
	void nextLine();
	void finishPreparation();
	void cacheRevisitedFrames();
	void prefetch();

	RunawayAudio &_audio;
	Common::RandomSource _random;
	RunawaySpeech _speech;
	RunawayVideo _video, _stills;
	bool _useStill = false;
	RunawayFilmPortrait _portraits;
	Graphics::ManagedSurface _background;
	Common::Rect _destination;
	uint _portrait = 0;
	char _chapter = 'B';
	const SceneTextStore *_text = nullptr;
	const Command *_commands = nullptr;
	uint _index = 0;
	uint _movie = 0;
	uint32 _now = 0, _started = 0;
	uint32 _repeatAt = 0;
	bool _entered = false, _showPortrait = false, _silentPortrait = false;
	bool _skipFilm = false;
	PauseToken _preparePause;
	bool _skipSpeechPending = false;
	bool _keepEffects = false;
	int _listenerX = 512;
	struct Line {
		Common::U32String text;
		uint16 sample;
		bool global;
	};
	Common::Array<Line> _lines;
	uint _line = 0;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SEQUENCE_H
