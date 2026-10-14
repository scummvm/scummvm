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

#ifndef HOLLYWOOD_RUNAWAY_PRESENTATION_H
#define HOLLYWOOD_RUNAWAY_PRESENTATION_H

#include "common/random.h"
#include "common/language.h"
#include "common/ustr.h"
#include "hollywood/runaway/animation.h"
#include "hollywood/runaway/audio.h"
#include "hollywood/runaway/graphics.h"
#include "hollywood/scene_text_store.h"

namespace Hollywood {

class RunawayCredits {
public:
	bool load();
	void start(uint32 now) { _started = _now = now; }
	void update(uint32 now) { _now = now; }
	bool finished() const;
	void draw(Graphics::ManagedSurface &screen) const;

private:
	Graphics::ManagedSurface _roll, _card;
	uint32 _started = 0, _now = 0;
};

class RunawayInterrogation {
public:
	explicit RunawayInterrogation(Audio::Mixer *mixer) : _audio(mixer), _random("runawayInterrogation") {}
	bool load();
	void start(uint32 now);
	void update(uint32 now);
	void skipSpeech();
	void stop();
	void draw(Graphics::ManagedSurface &screen) const;
	bool finished() const { return _phrase >= _phraseCount; }
	bool speaking() const { return _row != 0; }
	const Common::U32String &subtitle() const { return _subtitle; }

private:
	void nextPhrase();
	void showSpeechRow();
	RunawayAudio _audio;
	Common::RandomSource _random;
	RunawayLipSync _lipSync;
	SceneTextStore _text;
	Graphics::ManagedSurface _background;
	RunawayAnimation _actors[3];
	uint _frames[3] = {};
	bool _idle[3] = {};
	byte _speakers[4] = {}, _phrase = 0, _phraseCount = 0;
	uint16 _row = 0, _lastRow = 0, _sample = 0;
	uint32 _now = 0, _deadline = 0, _lastTick[3] = {};
	Common::U32String _subtitle;
};

class RunawayEnding {
public:
	RunawayEnding(Audio::Mixer *mixer, Common::Language language) : _audio(mixer), _random("runawayEnding"), _language(language) {}
	bool load();
	void start(uint32 now);
	void update(uint32 now);
	void skip(uint32 now);
	void draw(Graphics::ManagedSurface &screen) const;
	bool finished() const { return _stage == kFinished; }

private:
	enum Stage { kOutside, kSlideshow, kCard, kFinished };
	void nextSlide(uint32 now);
	RunawayAudio _audio;
	Common::RandomSource _random;
	Common::Language _language;
	Graphics::ManagedSurface _outside, _card, _slide, _demoLogo;
	RunawayAnimation _layers[7];
	Common::Array<byte> _occlusion[2];
	uint _frames[7] = {};
	bool _active[7] = {};
	Stage _stage = kOutside;
	uint _screenshot = 0;
	uint32 _started = 0, _now = 0, _lastTick = 0;
	uint32 _fadeStarted = 0, _fadeDuration = 0, _musicUntil = 0;
};

} // End of namespace Hollywood

#endif
