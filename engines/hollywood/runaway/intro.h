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

#ifndef HOLLYWOOD_RUNAWAY_INTRO_H
#define HOLLYWOOD_RUNAWAY_INTRO_H

#include "hollywood/runaway/audio.h"
#include "hollywood/runaway/graphics.h"

namespace Hollywood {

// The demo's startup sequence: animated Pendulo logo, then the story card.
// The engine supplies its pause-aware clock and handles input/presentation.
class RunawayIntro {
public:
	explicit RunawayIntro(Audio::Mixer *mixer) : _audio(mixer) {}
	bool load();
	void start(uint32 now);
	void skip(uint32 now);
	void update(uint32 now);
	void draw();
	bool finished() const { return _phase == kFinished; }
	const Graphics::ManagedSurface &screen() const { return _screen; }

private:
	enum Phase { kLogo, kLogoAudio, kStoryFadeIn, kStory, kStoryFadeOut, kFinished };
	void startStory(uint32 now);

	RunawayAudio _audio;
	RunawayAnimation _logo[3];
	Graphics::ManagedSurface _story, _screen;
	Phase _phase = kFinished;
	uint32 _phaseStarted = 0;
	uint32 _storyFadeDuration = 0;
	int _logoFrames[3] = { -1, -1, -1 };
	uint _brightness = 255;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_INTRO_H
