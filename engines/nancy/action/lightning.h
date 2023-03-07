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

#ifndef NANCY_ACTION_LIGHTNING_H
#define NANCY_ACTION_LIGHTNING_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

class LightningOn : public ActionRecord {
public:
	enum LightningState { kStartPulse, kPulse, kThunder };

	LightningOn() = default;
	virtual ~LightningOn();

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	void handlePulse(bool on);
	void handleThunder();

	LightningState _lightningState = kStartPulse;

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

protected:
	Common::String getRecordTypeName() const override { return "LightningOn"; }
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_LIGHTNING_H
