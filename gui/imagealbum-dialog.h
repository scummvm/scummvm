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

#ifndef IMAGEALBUM_DIALOG_H
#define IMAGEALBUM_DIALOG_H

#include "common/formats/formatinfo.h"
#include "common/str.h"
#include "common/rational.h"

namespace Common {

class SeekableReadStream;

} // End of namespace Common

namespace Graphics {

class Palette;
struct Surface;

} // End of namespace Graphics

namespace GUI {

class Dialog;

enum ImageAlbumImageOrientation {
	kImageAlbumImageOrientationUnspecified,

	kImageAlbumImageOrientationLandscape,
	kImageAlbumImageOrientationPortrait,
};

enum ImageAlbumViewTransformation {
	kImageAlbumViewTransformationNone,
	kImageAlbumViewTransformationRotate90CCW,
	kImageAlbumViewTransformationRotate90CW,
	kImageAlbumViewTransformationRotate180,
};

struct ImageAlbumImageMetadata {
	ImageAlbumImageMetadata() : _orientation(kImageAlbumImageOrientationUnspecified), _viewTransformation(kImageAlbumViewTransformationNone), _hdpi(72, 1), _vdpi(72, 1) {}

	ImageAlbumViewTransformation _viewTransformation;	///< Transformation required to present the image at its normal intended viewing orientation
	ImageAlbumImageOrientation _orientation;			///< Orientation of the image after view transformation
	Common::Rational _hdpi;								///< Horizontal DPI of the image
	Common::Rational _vdpi;								///< Vertical DPI of the image
};

/**
 * @brief Interface that supplies images to the image album dialog.
 */
class ImageAlbumImageSupplier {
public:
	virtual ~ImageAlbumImageSupplier();

	/**
	 * @brief Loads and returns the image for a specified slot
	 * @param slot                    The image slot to load
	 * @param outSurface              An outputted pointer to a surface containing the image data
	 * @param outHasPalette           An outputted boolean containing true if the image has a palette and false if not
	 * @param outPalette              Outputted palette colors if the image has a palette
	 * @param outMetadata             Outputted metadata for the image
	 * @return True if the image loaded successfully, false if it failed
	 */
	virtual bool loadImageSlot(uint slot, const Graphics::Surface *&outSurface, bool &outHasPalette, Graphics::Palette &outPalette, ImageAlbumImageMetadata &outMetadata) = 0;

	/**
	 * @brief Releases any resources for an image loaded with loadImageSlot
	 * @param slot The image slot to release
	 */
	virtual void releaseImageSlot(uint slot) = 0;

	/**
	 * Returns the file format of the image in the specified image slot, if it's capable of being loaded as raw file data.
	 * 
	 * @param slot       The image slot to load
	 * @param outFormat  A reference to a file format ID to set to the file format
	 * @return true if the slot is loadable as raw data and has a MIME type available, false if not
	 */
	virtual bool getFileFormatForImageSlot(uint slot, Common::FormatInfo::FormatID &outFormat) const = 0;

	/**
	 * @brief Opens a slot as a read stream containing raw file data.
	 * @param slot The image slot to load
	 * @return The created read stream, if it could be created, or nullptr if it failed
	 */
	virtual Common::SeekableReadStream *createReadStreamForSlot(uint slot) = 0;

	/**
	 * @brief Returns the number of image slots
	 * @return The number of slots
	 */
	virtual uint getNumSlots() const = 0;

	/**
	 * @brief Returns the default filename, including extension, for the specified slot
	 * @return The filename of the slot without an extension
	 */
	virtual Common::U32String getDefaultFileNameForSlot(uint slot) const = 0;
};

Dialog *createImageAlbumDialog(const Common::U32String &title, ImageAlbumImageSupplier *imageSupplier, uint initialSlot);

} // End of namespace GUI

#endif
