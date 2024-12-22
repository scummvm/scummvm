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


#include "common/system.h"
#include "sci/graphics/drivers/gfxdriver_intern.h"

namespace Sci {

class SCI1_VGAGreyScaleDriver final : public GfxDefaultDriver {
public:
	SCI1_VGAGreyScaleDriver(bool rgbRendering);
	~SCI1_VGAGreyScaleDriver() override;
	void setPalette(const byte *colors, uint start, uint num, bool update, const PaletteMod *palMods, const byte *palModMapping) override;
	static bool validateMode(Common::Platform p) { return (p == Common::kPlatformDOS || p == Common::kPlatformWindows) && checkDriver(&_driverFile, 1); }
private:
	byte *_greyScalePalette;
	static const char *_driverFile;
};

SCI1_VGAGreyScaleDriver::SCI1_VGAGreyScaleDriver(bool rgbRendering) : GfxDefaultDriver(320, 200, false, rgbRendering), _greyScalePalette(nullptr) {
	_greyScalePalette = new byte[_numColors * 3]();
}

SCI1_VGAGreyScaleDriver::~SCI1_VGAGreyScaleDriver() {
	delete[] _greyScalePalette;
}

void SCI1_VGAGreyScaleDriver::setPalette(const byte *colors, uint start, uint num, bool update, const PaletteMod *palMods, const byte *palModMapping) {
	GFXDRV_ASSERT_READY;
	byte *d = _greyScalePalette;
	for (uint i = 0; i < num; ++i) {
		// In the driver files I inspected there were never any other color distributions than this.
		// So I guess it is safe to hardcode that instead of loading it from the driver file.
		d[0] = d[1] = d[2] = (colors[0] * 77 + colors[1] * 150 + colors[2] * 28) >> 8;
		colors += 3;
		d += 3;
	}

	GfxDefaultDriver::setPalette(_greyScalePalette, start, num, update, palMods, palModMapping);
}

const char *SCI1_VGAGreyScaleDriver::_driverFile = "VGA320BW.DRV";

SCI_GFXDRV_VALIDATE_IMPL(SCI1_VGAGreyScale)

GfxDriver *SCI1_VGAGreyScaleDriver_create(int rgbRendering, ...) {
	return new SCI1_VGAGreyScaleDriver(rgbRendering != 0);
}

} // End of namespace Sci
