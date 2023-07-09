#pragma once
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

#ifndef COMMON_FORMATS_FORMATINFO_H
#define COMMON_FORMATS_FORMATINFO_H

#include "common/str.h"
#include "image/bmp.h"

namespace Graphics {

struct Surface;

} // End of namespace Graphics

namespace Common {

namespace FormatInfo {

enum Lossiness {
	kLossinessUnknown,

	kLossinessLossless,
	kLossinessLossy,
	kLossinessEither,
};


/**
 * Values representing support for saving or sharing file types on the system.
 */
enum FormatSupportLevel {
	kFormatSupportLevelNone,      ///< Not supported at all
	kFormatSupportLevelAvoid,     ///< Can be saved/loaded but most likely unsupported even by third-party tools
	kFormatSupportLevelDefault,   ///< Not natively supported by the operating system, support by third-party tools is either available or unknown
	kFormatSupportLevelSupported, ///< Well-supported by the operating system
	kFormatSupportLevelPreferred, ///< Supported by operating system, preferred over other formats if lossless conversion is possible
};

enum FormatID {
	kFormatUnknown,

	// Image formats
	kBMP,	kFirstImageFormat = kBMP,
	kPNG,
	kJPEG,
	kPICT,	// Macintosh PICT

	// Binary formats
	kUntypedBinary,	kFirstBinaryFormat = kUntypedBinary,

	// End of list
	kCount,

	kLastImageFormat = kFirstBinaryFormat - 1,
	kLastBinaryFormat = kCount - 1,
};

struct ImageFormatCharacteristics {
	ImageFormatCharacteristics();

	Lossiness _lossiness;
	bool _supportAlpha;
	bool _supportPalette;
	uint _maxBitDepth;
};

struct ImageSaveProperties {
	ImageSaveProperties();

	uint8 _qualityLevel;
};

typedef bool (*ImageSaveCallback_t)(Common::WriteStream &stream, const Graphics::Surface &surf, const byte *palette, const ImageSaveProperties &saveProperties);

/**
 * Returns the default file extension for a format
 *
 * @param format              The file format
 * @param limitTo3Characters  If true, limit the extension to 3 characters
 * @return 
*/
const char *getFormatExtension(Common::FormatInfo::FormatID format, bool limitTo3Characters);

Common::U32String getFormatSaveDescription(Common::FormatInfo::FormatID format);

ImageSaveCallback_t getImageSaveFunction(FormatID format);


bool getImageFormatCharacteristics(FormatID format, ImageFormatCharacteristics &outCharacteristics);

/**
 * Returns the OS's level of support of a file format.
 */
FormatSupportLevel getFormatSupportLevel(Common::FormatInfo::FormatID format);

} // End of namespace FormatInfo

} // End of namespace Common

#endif
