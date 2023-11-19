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

#include "common/formats/formatinfo.h"

#include "common/translation.h"
#include "common/util.h"

#include "image/bmp.h"
#include "image/png.h"

namespace Common {

namespace FormatInfo {

struct FormatTypeInfo {
	const char *_shortExtension;	///< Extension limited to 3 characters
	const char *_longExtension;     ///< Extension with no length limit
};

const FormatTypeInfo g_formatTypeInfo[kCount] = {
	{nullptr, nullptr},

	// Image formats
	{"bmp", nullptr},
	{"png", nullptr},
	{"jpg", nullptr},
	{"pct", "pict"},

	// Binary formats
	{"application/octet-stream", nullptr},
};

ImageFormatCharacteristics::ImageFormatCharacteristics()
	: _lossiness(kLossinessUnknown), _supportAlpha(false), _supportPalette(false), _maxBitDepth(0) {
}

ImageSaveProperties::ImageSaveProperties() : _qualityLevel(75) {
}

bool getImageFormatCharacteristics(FormatID format, ImageFormatCharacteristics &outCharacteristics) {
	outCharacteristics = ImageFormatCharacteristics();

	switch (format) {
	case kPICT:
		outCharacteristics._supportPalette = true;
		outCharacteristics._maxBitDepth = 24;
		outCharacteristics._lossiness = kLossinessEither;
		break;
	case kBMP:
		outCharacteristics._supportPalette = true;
		outCharacteristics._maxBitDepth = 24;
		outCharacteristics._lossiness = kLossinessLossless;
		break;
	case kPNG:
		outCharacteristics._supportPalette = true;
		outCharacteristics._supportAlpha = true;
		outCharacteristics._maxBitDepth = 32;
		outCharacteristics._lossiness = kLossinessLossless;
		break;
	case kJPEG:
		outCharacteristics._maxBitDepth = 24;
		outCharacteristics._lossiness = kLossinessLossy;
		break;
	default:
		return false;
	}

	return true;
}

const char *getFormatExtension(Common::FormatInfo::FormatID format, bool limitTo3Characters) {
	const FormatTypeInfo &typeInfo = g_formatTypeInfo[format];

	if (!limitTo3Characters && typeInfo._longExtension != nullptr)
		return typeInfo._longExtension;

	return typeInfo._shortExtension;
}

Common::U32String getFormatSaveDescription(Common::FormatInfo::FormatID format) {
	switch (format) {
	case kBMP:
		return _("Bitmap Image File");
	case kPNG:
		return _("PNG Image File");
	case kJPEG:
		return _("JPEG Image File");
	case kPICT:
		return _("QuickDraw PICT Image File");
	case kUntypedBinary:
	default:
		return _("Data File");
	}
}

#ifdef USE_PNG
static bool savePNG(Common::WriteStream &stream, const Graphics::Surface &surf, const byte *palette, const ImageSaveProperties &props) {
	return Image::writePNG(stream, surf, palette);
}
#endif

static bool saveBMP(Common::WriteStream &stream, const Graphics::Surface &surf, const byte *palette, const ImageSaveProperties &props) {
	return Image::writeBMP(stream, surf, palette);
}

ImageSaveCallback_t getImageSaveFunction(FormatID format) {
	switch (format) {
#ifdef USE_PNG
	case kPNG:
		return savePNG;
#endif
	case kBMP:
		return saveBMP;
	default:
		return nullptr;
	}
}

FormatSupportLevel getFormatSupportLevel(Common::FormatInfo::FormatID format) {
	switch (format) {
	case kPICT:
		return kFormatSupportLevelAvoid;
	case kPNG:
	case kJPEG:
		return kFormatSupportLevelPreferred;
	default:
		return kFormatSupportLevelDefault;
	}
}

} // End of namespace FormatInfo

} // End of namespace Common
