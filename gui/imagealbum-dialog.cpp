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

#include "gui/imagealbum-dialog.h"

#include "graphics/palette.h"

#include "gui/dialog.h"
#include "gui/filebrowser-dialog.h"
#include "gui/gui-manager.h"
#include "gui/ThemeEval.h"
#include "gui/widget.h"

#include "gui/widgets/scrollcontainer.h"

#include "image/bmp.h"
#include "image/png.h"

#include "common/dialogs.h"
#include "common/savefile.h"
#include "common/stream.h"
#include "common/translation.h"

namespace GUI {

ImageAlbumImageSupplier::~ImageAlbumImageSupplier() {
}

class ImageAlbumDialog : public Dialog {
public:
	ImageAlbumDialog(const Common::U32String &title, ImageAlbumImageSupplier *imageSupplier, uint initialSlot);
	~ImageAlbumDialog();

	void open() override;
	void close() override;

	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;

private:
	ImageAlbumDialog() = delete;
	ImageAlbumDialog(const ImageAlbumDialog &) = delete;

	void changeToSlot(uint slot);
	void saveImageInSlot(uint slot);

	ImageAlbumImageSupplier *_imageSupplier;
	uint _currentSlot;
	uint _numSlots;

	ButtonWidget *_prevButton;
	ButtonWidget *_nextButton;
	ButtonWidget *_saveButton;

	StaticTextWidget *_imageNumberLabel;

	ContainerWidget *_imageContainer;
	GraphicsWidget *_imageGraphic;

	bool _canAlwaysSaveImage;

	enum {
		kPrevCmd = 'PREV',
		kNextCmd = 'NEXT',
		kSaveCmd = 'SAVE',
	};
};


ImageAlbumDialog::ImageAlbumDialog(const Common::U32String &title, ImageAlbumImageSupplier *imageSupplier, uint initialSlot)
	: Dialog("ImageAlbum"), _imageSupplier(imageSupplier), _currentSlot(initialSlot), _numSlots(0), _imageGraphic(nullptr), _canAlwaysSaveImage(false) {

	_backgroundType = ThemeEngine::kDialogBackgroundSpecial;

	_numSlots = imageSupplier->getNumSlots();

	assert(_numSlots > 0);

	new StaticTextWidget(this, "ImageAlbum.Title", title);

	if (initialSlot >= _numSlots)
		initialSlot = _numSlots - 1;

	// Buttons
	if (_numSlots > 1) {
		_prevButton = new ButtonWidget(this, "ImageAlbum.Prev", _("Prev"), Common::U32String(), kPrevCmd);
		_nextButton = new ButtonWidget(this, "ImageAlbum.Next", _("Next"), Common::U32String(), kNextCmd);
		_imageNumberLabel = new StaticTextWidget(this, "ImageAlbum.ImageNumber", Common::U32String());
	} else {
		_prevButton = nullptr;
		_nextButton = nullptr;
		_imageNumberLabel = nullptr;
	}

	_saveButton = nullptr;


	bool canSaveAnyFormat = false;
	for (uint fmtID = Common::FormatInfo::kFirstImageFormat; fmtID <= Common::FormatInfo::kLastImageFormat; fmtID++) {
		Common::FormatInfo::FormatID format = static_cast<Common::FormatInfo::FormatID>(fmtID);

		if (Common::FormatInfo::getFormatSupportLevel(format) > Common::FormatInfo::kFormatSupportLevelNone) {
			canSaveAnyFormat = true;
			if (Common::FormatInfo::getImageSaveFunction(format) != nullptr) {
				_canAlwaysSaveImage = true;
				break;
			}
		}
	}

	if (canSaveAnyFormat) {
		_saveButton = new ButtonWidget(this, "ImageAlbum.Save", _("Save Image..."), Common::U32String(), kSaveCmd);
		_saveButton->setEnabled(!_canAlwaysSaveImage);
	}

	new ButtonWidget(this, "ImageAlbum.Close", _("Close"), Common::U32String(), kCloseCmd);

	_imageContainer = new ContainerWidget(this, "ImageAlbum.ImageContainer");

	_imageGraphic = nullptr;
}

ImageAlbumDialog::~ImageAlbumDialog() {
}

void ImageAlbumDialog::open() {
	Dialog::open();

	changeToSlot(_currentSlot);
}

void ImageAlbumDialog::changeToSlot(uint slot) {
	bool canSaveImage = _canAlwaysSaveImage;

	if (_imageGraphic) {
		_imageContainer->removeWidget(_imageGraphic);
		delete _imageGraphic;
		_imageGraphic = nullptr;
	}

	Common::Rect graphicRect = Common::Rect(0, 0, _imageContainer->getWidth(), _imageContainer->getHeight());

	int inset = g_gui.xmlEval()->getVar("Globals.ImageAlbum.ImageInset", 0);
	graphicRect.grow(-inset);

	if (graphicRect.isValidRect()) {
		uint32 graphicRectWidth = graphicRect.width();
		uint32 graphicRectHeight = graphicRect.height();

		const Graphics::Surface *surf = nullptr;
		bool hasPalette = false;
		Graphics::Palette palette(256);
		ImageAlbumImageMetadata metadata;

		if (_imageSupplier->loadImageSlot(slot, surf, hasPalette, palette, metadata)) {
			if (!canSaveImage) {
				// If we can't always save the image (meaning we don't have an image write-out function) then see if we can
				// at least save this file in its native format.
				Common::FormatInfo::FormatID format = Common::FormatInfo::kFormatUnknown;
				if (_imageSupplier->getFileFormatForImageSlot(slot, format)) {
					if (Common::FormatInfo::getFormatSupportLevel(format) > Common::FormatInfo::kFormatSupportLevelNone)
						canSaveImage = true;
				}
			}

			uint32 imageWidth = surf->w;
			uint32 imageHeight = surf->h;

			uint32 scaledWidth = graphicRectWidth;
			uint32 scaledHeight = graphicRectHeight;

			bool needs90Rotate = (metadata._viewTransformation == kImageAlbumViewTransformationRotate90CCW || metadata._viewTransformation == kImageAlbumViewTransformationRotate90CW);

			uint32 imageRotatedWidth = imageWidth;
			uint32 imageRotatedHeight = imageHeight;
			if (needs90Rotate) {
				imageRotatedWidth = imageHeight;
				imageRotatedHeight = imageWidth;
			}

			// if (imageRotatedWidth / imageRotatedHeight > graphicRectWidth / graphicRectHeight)
			if (imageRotatedWidth * graphicRectHeight >= graphicRectWidth * imageRotatedHeight) {
				// Image aspect ratio is wider than the graphic space, or same
				scaledWidth = graphicRectWidth;
				scaledHeight = imageRotatedHeight * graphicRectWidth / imageRotatedWidth;
			} else {
				// Image aspect ratio is taller than the graphic space
				scaledWidth = imageRotatedWidth * graphicRectHeight / imageRotatedHeight;
				scaledHeight = graphicRectHeight;
			}

			if (scaledWidth < 1)
				scaledWidth = 1;
			if (scaledHeight < 1)
				scaledHeight = 1;

			Graphics::ManagedSurface rescaledGraphic;
			rescaledGraphic.create(scaledWidth, scaledHeight, surf->format);
			if (hasPalette)
				rescaledGraphic.setPalette(palette, 0, 256);

			if (needs90Rotate) {
				bool isClockwise = metadata._viewTransformation == kImageAlbumViewTransformationRotate90CW;

				for (uint32 destX = 0; destX < scaledWidth; destX++) {
					uint32 srcY = destX * imageHeight / scaledWidth;
					if (isClockwise)
						srcY = imageHeight - 1 - srcY;

					for (uint32 destY = 0; destY < scaledHeight; destY++) {
						uint32 srcX = destY * imageWidth / scaledHeight;

						if (!isClockwise)
							srcX = imageWidth - 1 - srcX;

						rescaledGraphic.setPixel(destX, destY, surf->getPixel(srcX, srcY));
					}
				}
			} else if (metadata._viewTransformation == kImageAlbumViewTransformationRotate180) {
				for (uint32 destX = 0; destX < scaledWidth; destX++) {
					uint32 srcX = (imageWidth - 1 - (destX * imageWidth / scaledWidth));

					for (uint32 destY = 0; destY < scaledHeight; destY++) {
						uint32 srcY = (imageHeight - 1 - (destY * imageHeight / scaledHeight));

						rescaledGraphic.setPixel(destX, destY, surf->getPixel(srcX, srcY));
					}
				}
			} else {
				rescaledGraphic.blitFrom(*surf, Common::Rect(0, 0, imageWidth, imageHeight), Common::Rect(0, 0, scaledWidth, scaledHeight));
			}

			_imageSupplier->releaseImageSlot(slot);

			if (rescaledGraphic.format.bytesPerPixel == 1)
				rescaledGraphic.convertToInPlace(Graphics::createPixelFormat<888>(), palette.data, 0, 256);

			int32 xCoord = (static_cast<int32>(_imageContainer->getWidth()) - static_cast<int32>(scaledWidth)) / 2u;
			int32 yCoord = (static_cast<int32>(_imageContainer->getHeight()) - static_cast<int32>(scaledHeight)) / 2u;

			_imageGraphic = new GraphicsWidget(_imageContainer, xCoord, yCoord, xCoord + static_cast<int32>(scaledWidth), yCoord + static_cast<int32>(scaledHeight));

			_imageGraphic->setGfx(&rescaledGraphic, false);

			if (_numSlots > 1) {
				_imageNumberLabel->setLabel(Common::U32String::format(_("%u of %u"), static_cast<uint>(slot + 1u), _numSlots));
				_prevButton->setEnabled(slot > 0);
				_nextButton->setEnabled(slot < _numSlots - 1u);
			}

			_currentSlot = slot;
		} else {
			warning("Image album failed to retrieve slot %u", slot);
		}
	}

	if (_saveButton)
		_saveButton->setEnabled(canSaveImage);
}

void ImageAlbumDialog::saveImageInSlot(uint slot) {
	Common::U32String defaultFileName = _imageSupplier->getDefaultFileNameForSlot(slot);

	Common::FormatInfo::FormatID nativeFormat = Common::FormatInfo::kFormatUnknown;

	Common::U32String fileExt;
	Common::U32String fileDesc;

	bool hasExtension = 0;
	uint extensionPos = 0;
	for (uint i = 0; i < defaultFileName.size(); i++) {
		if (defaultFileName[i] == '.') {
			hasExtension = true;
			extensionPos = i;
		}
	}
	Common::SaveFileManager *saveFileManager = g_system->getSavefileManager();

	Common::FormatInfo::FormatSupportLevel bestFormatSupportLevel = Common::FormatInfo::kFormatSupportLevelNone;
	Common::FormatInfo::FormatID bestFormat = Common::FormatInfo::kFormatUnknown;
	bool bestFormatIsLossy = true;

	// Find the best format we can write the image as
	for (uint fmtID = Common::FormatInfo::kFirstImageFormat; fmtID <= Common::FormatInfo::kLastImageFormat; fmtID++) {
		Common::FormatInfo::FormatID candidateFormat = static_cast<Common::FormatInfo::FormatID>(fmtID);

		if (!Common::FormatInfo::getImageSaveFunction(candidateFormat))
			continue;

		Common::FormatInfo::FormatSupportLevel supportLevel = Common::FormatInfo::getFormatSupportLevel(candidateFormat);

		bool formatIsLossy = false;
		Common::FormatInfo::ImageFormatCharacteristics characteristics;
		if (Common::FormatInfo::getImageFormatCharacteristics(candidateFormat, characteristics))
			formatIsLossy = (characteristics._lossiness == Common::FormatInfo::kLossinessLossy);

		bool isBetter = false;

		// If the best format we have chosen is lossy, and this is a lossless format that is at least supported, it is better
		// If this format is the same lossiness, but is better-supported, it is better
		if (bestFormatIsLossy && !formatIsLossy && supportLevel >= Common::FormatInfo::kFormatSupportLevelSupported)
			isBetter = true;
		else if (bestFormatIsLossy == formatIsLossy && supportLevel > bestFormatSupportLevel)
			isBetter = true;
		else if (bestFormat == Common::FormatInfo::kFormatUnknown && supportLevel >= Common::FormatInfo::kFormatSupportLevelNone)
			isBetter = true;

		if (isBetter) {
			bestFormatSupportLevel = supportLevel;
			bestFormat = candidateFormat;
			bestFormatIsLossy = formatIsLossy;
		}
	}

	assert(bestFormat != Common::FormatInfo::kFormatUnknown);

	if (_imageSupplier->getFileFormatForImageSlot(slot, nativeFormat)) {
		if (nativeFormat != bestFormat) {
			Common::FormatInfo::FormatSupportLevel nativeSupportLevel = Common::FormatInfo::getFormatSupportLevel(nativeFormat);

			if (nativeSupportLevel > Common::FormatInfo::kFormatSupportLevelNone) {
				bool nativeFormatIsLossy = false;

				Common::FormatInfo::ImageFormatCharacteristics characteristics;
				if (Common::FormatInfo::getImageFormatCharacteristics(nativeFormat, characteristics))
					nativeFormatIsLossy = (characteristics._lossiness == Common::FormatInfo::kLossinessLossy);

				// If the native format is lossy and is at least supported, prefer using it directly, otherwise only use it if it has a higher support level
				if ((nativeFormatIsLossy && nativeSupportLevel >= Common::FormatInfo::kFormatSupportLevelSupported) || nativeSupportLevel >= bestFormatSupportLevel) {
					bestFormat = nativeFormat;
					bestFormatSupportLevel = nativeSupportLevel;
				}
			}
		}
	}

	// This shouldn't be possible, the Save button should not be visible unless there is either a saveable format,
	// or the file's native format is saveable, and in either of those circumstances, a format should have been selected by this point.
	assert(bestFormatSupportLevel > Common::FormatInfo::kFormatSupportLevelNone);

	bool needsConversion = false;

	if (nativeFormat == bestFormat) {
		// Save in the preferred format
		if (hasExtension)
			fileExt = defaultFileName.substr(extensionPos + 1);
	} else {
		// Save in the preferred format
		needsConversion = true;

		fileExt = Common::U32String(Common::FormatInfo::getFormatExtension(bestFormat, true));

		if (hasExtension)
			defaultFileName = defaultFileName.substr(0, extensionPos) + Common::U32String(".") + fileExt;
	}

	fileDesc = Common::FormatInfo::getFormatSaveDescription(bestFormat);

	Common::U32String title = _("Save Image");

	if (needsConversion) {
		const Graphics::Surface *surf = nullptr;
		bool hasPalette = false;
		Graphics::Palette palette(256);
		ImageAlbumImageMetadata metadata;

		if (_imageSupplier->loadImageSlot(slot, surf, hasPalette, palette, metadata)) {
			Common::ScopedPtr<Common::SeekableWriteStream> writeStream;

			GUI::FileBrowserDialog browser(title.encode(Common::kUtf8).c_str(), fileExt.encode(Common::kUtf8).c_str(), GUI::kFBModeSave, nullptr, defaultFileName.encode(Common::kUtf8).c_str());

			if (browser.runModal() > 0) {
				Common::String path = browser.getResult();
				writeStream.reset(saveFileManager->openForSaving(path, false));
				if (writeStream) {
					assert(writeStream);

					Common::FormatInfo::ImageSaveCallback_t saveCallback = Common::FormatInfo::getImageSaveFunction(bestFormat);
					assert(saveCallback);

					Common::FormatInfo::ImageSaveProperties saveProps;
					saveCallback(*writeStream, *surf, hasPalette ? palette.data : nullptr, saveProps);
				} else {
					warning("Failed to open image output stream");
				}
			}

			_imageSupplier->releaseImageSlot(slot);
		}
	} else {
		Common::ScopedPtr<Common::SeekableReadStream> readStream;
		readStream.reset(_imageSupplier->createReadStreamForSlot(slot));
		if (!readStream) {
			warning("Failed to open input stream for slot %u", slot);
			return;
		}

		Common::ScopedPtr<Common::SeekableWriteStream> writeStream;

		GUI::FileBrowserDialog browser(title.encode(Common::kUtf8).c_str(), fileExt.encode(Common::kUtf8).c_str(), GUI::kFBModeSave, nullptr, defaultFileName.encode(Common::kUtf8).c_str());

		if (browser.runModal() > 0) {
			Common::String path = browser.getResult();
			writeStream.reset(saveFileManager->openForSaving(path, false));

			if (writeStream) {
				assert(writeStream);

				byte copyBuffer[2048];
				uint32 bytesRead = readStream->read(copyBuffer, sizeof(copyBuffer));
				while (bytesRead) {
					writeStream->write(copyBuffer, bytesRead);
					bytesRead = readStream->read(copyBuffer, sizeof(copyBuffer));
				}
			} else {
				warning("Failed to open image output stream");
			}
		}
	}
}

void ImageAlbumDialog::close() {
	Dialog::close();
}

void ImageAlbumDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kPrevCmd:
		if (_currentSlot > 0)
			changeToSlot(_currentSlot - 1);
		break;
	case kNextCmd:
		if (_currentSlot < _numSlots - 1)
			changeToSlot(_currentSlot + 1);
		break;
	case kSaveCmd:
		saveImageInSlot(_currentSlot);
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

GUI::Dialog *createImageAlbumDialog(const Common::U32String &title, ImageAlbumImageSupplier *imageSupplier, uint initialSlot) {
	return new ImageAlbumDialog(title, imageSupplier, initialSlot);
}

} // End of namespace GUI
