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

#include "common/scummsys.h"
#include "math/angle.h"
#include "zvision/zvision.h"

#ifndef ZVISION_VOLUME_MANAGER
#define ZVISION_VOLUME_MANAGER

namespace ZVision {

enum volumeScaling {
	kVolumeLinear,
	kVolumePowerLaw,
	kVolumeParabolic,
	kVolumeCubic,
	kVolumeQuartic,
	kVolumeLogPower,
	kVolumeLogAmplitude
};

class VolumeManager {
public:
	VolumeManager(ZVision *engine, volumeScaling mode);
	~VolumeManager() {};
	volumeScaling getMode() const {
		return _mode;
	}
	void setMode(volumeScaling mode) {
		_mode = mode;
	}
	uint8 convert(uint8 inputValue);
	uint8 convert(uint8 inputValue, volumeScaling &mode);
	uint8 convert(uint8 inputValue, Math::Angle azimuth, uint8 directionality = 255);
	uint8 convert(uint8 inputValue, volumeScaling &mode, Math::Angle azimuth, uint8 directionality = 255);
#if defined(USE_MPEG2) && defined(USE_A52)
	double getVobAmplification(Common::String fileName) const;
#endif

private:
	uint _scriptScale = 100; // Z-Vision scripts internally use a volume scale of 0-100; ScummVM uses a scale of 0-255.
	volumeScaling _mode = kVolumeLinear;
};

} // End of namespace ZVision

#endif
