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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_SETTINGS_H
#define RIPPER_SETTINGS_H

#include "common/scummsys.h"
#include "common/array.h"

namespace Audio {
class Mixer;
}

namespace Ripper {

class RipperSettings {
public:
	enum Slider {
		kMasterVolume = 0,
		kAmbientVolume,
		kSfxVolume,
		kVideoVolume,
		kBrightness,
		kColor,
		kContrast,
		kTint,
		kSliderCount
	};

	struct Descriptor {
		int minimum;
		int maximum;
		int step;
		int defaultValue;
	};

	explicit RipperSettings(Audio::Mixer *mixer);

	void load();
	void save();
	void resetDefaults();
	void setValue(Slider slider, int value);
	int getValue(Slider slider) const { return _values[slider]; }
	uint getFilledTickCount(Slider slider) const;
	void applyVideoPalette(byte *palette, uint colorCount, bool rememberSource = true);
	bool restoreVideoPalette();

	static const Descriptor &getDescriptor(Slider slider);

private:
	void applyAudioVolumes();
	static int mixerToPercent(int volume);
	static int percentToMixer(int volume);

	Audio::Mixer *_mixer;
	int _values[kSliderCount];
	Common::Array<byte> _videoPaletteSource;
};

} // End of namespace Ripper

#endif // RIPPER_SETTINGS_H
