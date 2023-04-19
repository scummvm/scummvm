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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_MISC_SPECIALEFFECT_H
#define NANCY_MISC_SPECIALEFFECT_H

#include "engines/nancy/time.h"
#include "engines/nancy/renderobject.h"

namespace Nancy {

struct SPEC;

namespace Misc {

class SpecialEffect : public RenderObject {
public:
	static const byte kSceneChangeFadeOutToBlack 	= 1;
	static const byte kSceneChangeFadeCrossDissolve = 1;

	SpecialEffect(byte type, uint16 fadeToBlackTime, uint16 frameTime) :
		RenderObject(16),
		_type(type),
		_fadeToBlackTime(fadeToBlackTime),
		_frameTime(frameTime) {}
	virtual ~SpecialEffect() {}

	void init() override;
	void updateGraphics() override;

	void onSceneChange();
	void afterSceneChange();

	bool isDone() const;
	bool isInitialized() const { return _initialized; }

protected:
	bool _initialized = false;

	Time _nextFrameTime;
	Time _fadeToBlackEndTime;

	Graphics::ManagedSurface _fadeFrom;
	Graphics::ManagedSurface _fadeTo;

	byte _type = 1;
	uint16 _fadeToBlackTime = 0;
	uint16 _frameTime = 0;

	uint _currentFrame = 0;
	uint _numFrames = 0;
	SPEC *_specialEffectData = nullptr;
};

} // End of namespace Misc
} // End of namespace Nancy

#endif // NANCY_MISC_SPECIALEFFECT_H
