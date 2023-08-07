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

#ifndef NANCY_MISC_LIGHTNING_H
#define NANCY_MISC_LIGHTNING_H

#include "engines/nancy/commontypes.h"

namespace Nancy {
namespace Misc {

// Special class that handles The Vampire Diaries' lightning screen effect.
// Activated by the LightningOn action record as well as at the endgame section
class Lightning {
public:
	enum LightningState { kBegin, kStartPulse, kPulse, kThunder, kNotRunning };

	void beginLightning(int16 distance, uint16 pulseTime, int16 rgbPercent);
	void endLightning();

	void run();

private:
	void handlePulse(bool on);
	void handleThunder();

	bool _isRunning = false;
	LightningState _state = kNotRunning;

	int16 _minRGBPercent = 0;
	int16 _maxRGBPercent = 0;

	int16 _minInterPulseDelay = 0;
	int16 _maxInterPulseDelay = 0;

	int16 _minPulseLength = 5;
	int16 _maxPulseLength = 0;

	int16 _minSoundStartDelay = 0;
	int16 _maxSoundStartDelay = 0;

	uint32 _nextStateTime = 0;
	uint32 _nextSoundTime0 = 0;
	uint32 _nextSoundTime1 = 0;

	int _nextSoundToPlay = 0;

	Common::Array<RenderObject *> _viewportObjs;
	Common::Array<byte *> _viewportObjOriginalPalettes;
};

} // End of namespace Misc
} // End of namespace Nancy

#endif // NANCY_MISC_LIGHTNING_H
