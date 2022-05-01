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

#include "mtropolis/elements.h"
#include "mtropolis/assets.h"
#include "mtropolis/element_factory.h"
#include "mtropolis/render.h"

#include "video/video_decoder.h"
#include "video/qt_decoder.h"

#include "common/substream.h"
#include "graphics/managed_surface.h"

namespace MTropolis {

GraphicElement::GraphicElement() : _cacheBitmap(false) {
}

GraphicElement::~GraphicElement() {
}

bool GraphicElement::load(ElementLoaderContext &context, const Data::GraphicElement &data) {
	if (!loadCommon(data.name, data.guid, data.rect1, data.elementFlags, data.layer, data.streamLocator, data.sectionID))
		return false;

	_cacheBitmap = ((data.elementFlags & Data::ElementFlags::kCacheBitmap) != 0);

	return true;
}

void GraphicElement::render(Window *window) {
	// todo
}

MovieElement::MovieElement()
	: _cacheBitmap(false), _paused(false), _reversed(false), _haveFiredAtFirstCel(false), _haveFiredAtLastCel(false)
	, _loop(false), _alternate(false), _playEveryFrame(false), _assetID(0), _runtime(nullptr) {
}

MovieElement::~MovieElement() {
	if (_unloadSignaller)
		_unloadSignaller->removeReceiver(this);
	if (_postRenderSignaller)
		_postRenderSignaller->removeReceiver(this);
}

bool MovieElement::load(ElementLoaderContext &context, const Data::MovieElement &data) {
	if (!loadCommon(data.name, data.guid, data.rect1, data.elementFlags, data.layer, data.streamLocator, data.sectionID))
		return false;

	_cacheBitmap = ((data.elementFlags & Data::ElementFlags::kCacheBitmap) != 0);
	_paused = ((data.elementFlags & Data::ElementFlags::kPaused) != 0);
	_loop = ((data.animationFlags & Data::AnimationFlags::kLoop) != 0);
	_alternate = ((data.animationFlags & Data::AnimationFlags::kAlternate) != 0);
	_playEveryFrame = ((data.animationFlags & Data::AnimationFlags::kPlayEveryFrame) != 0);
	_assetID = data.assetID;

	_runtime = context.runtime;

	return true;
}

bool MovieElement::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "paused") {
		result.setBool(_paused);
		return true;
	}

	return VisualElement::readAttribute(thread, result, attrib);
}

bool MovieElement::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {

	if (attrib == "paused") {
		writeProxy = DynamicValueWriteFuncHelper<MovieElement, &MovieElement::scriptSetPaused>::create(this);
		return true;
	}
	
	return VisualElement::writeRefAttribute(thread, writeProxy, attrib);
}

VThreadState MovieElement::consumeCommand(Runtime *runtime, const Common::SharedPtr<MessageProperties> &msg) {
	if (Event::create(EventIDs::kPlay, 0).respondsTo(msg->getEvent())) {
		StartPlayingTaskData *startPlayingTaskData = runtime->getVThread().pushTask("MovieElement::startPlayingTask", this, &MovieElement::startPlayingTask);
		startPlayingTaskData->runtime = runtime;

		ChangeFlagTaskData *becomeVisibleTaskData = runtime->getVThread().pushTask("MovieElement::changeVisibilityTask", static_cast<VisualElement *>(this), &MovieElement::changeVisibilityTask);
		becomeVisibleTaskData->desiredFlag = true;
		becomeVisibleTaskData->runtime = runtime;
	}

	return kVThreadReturn;
}


void MovieElement::activate() {
	Project *project = _runtime->getProject();
	Common::SharedPtr<Asset> asset = project->getAssetByID(_assetID).lock();

	if (!asset) {
		warning("Movie element references asset %i but the asset isn't loaded!", _assetID);
		return;
	}

	if (asset->getAssetType() != kAssetTypeMovie) {
		warning("Movie element assigned an asset that isn't a movie");
		return;
	}

	MovieAsset *movieAsset = static_cast<MovieAsset *>(asset.get());
	size_t streamIndex = movieAsset->getStreamIndex();
	int segmentIndex = project->getSegmentForStreamIndex(streamIndex);
	project->openSegmentStream(segmentIndex);
	Common::SeekableReadStream *stream = project->getStreamForSegment(segmentIndex);

	if (!stream) {
		warning("Movie element stream could not be opened");
		return;
	}

	Video::QuickTimeDecoder *qtDecoder = new Video::QuickTimeDecoder();
	qtDecoder->setChunkBeginOffset(movieAsset->getMovieDataPos());

	_videoDecoder.reset(qtDecoder);

	Common::SafeSeekableSubReadStream *movieDataStream = new Common::SafeSeekableSubReadStream(stream, movieAsset->getMovieDataPos(), movieAsset->getMovieDataPos() + movieAsset->getMovieDataSize(), DisposeAfterUse::NO);

	if (!_videoDecoder->loadStream(movieDataStream))
		_videoDecoder.reset();

	_unloadSignaller = project->notifyOnSegmentUnload(segmentIndex, this);
	_postRenderSignaller = project->notifyOnPostRender(this);
}

void MovieElement::deactivate() {
	if (_unloadSignaller) {
		_unloadSignaller->removeReceiver(this);
		_unloadSignaller.reset();
	}
	if (_postRenderSignaller) {
		_postRenderSignaller->removeReceiver(this);
		_postRenderSignaller.reset();
	}

	_videoDecoder.reset();
}

void MovieElement::render(Window *window) {
	const Graphics::Surface *videoSurface = nullptr;
	while (_videoDecoder->needsUpdate()) {
		videoSurface = _videoDecoder->decodeNextFrame();
		if (_playEveryFrame)
			break;
	}

	if (videoSurface) {
		Graphics::ManagedSurface *target = window->getSurface().get();
		Common::Rect srcRect(0, 0, videoSurface->w, videoSurface->h);
		Common::Rect destRect(_rect.left, _rect.top, _rect.right, _rect.bottom);
		target->blitFrom(*videoSurface, srcRect, destRect);
	}
}

void MovieElement::onPostRender(Runtime *runtime, Project *project) {
	if (_videoDecoder) {
		if (_videoDecoder->isPlaying() && _videoDecoder->endOfVideo()) {
			if (_alternate) {
				_reversed = !_reversed;
				if (!_videoDecoder->setReverse(_reversed)) {
					warning("Failed to reverse video decoder, disabling it");
					_videoDecoder.reset();
				}
			} else {
				_videoDecoder->stop();
			}

			Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event::create(_reversed ? EventIDs::kAtFirstCel : EventIDs::kAtLastCel, 0), DynamicValue(), getSelfReference()));
			Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
			runtime->queueMessage(dispatch);
		}
	}
}

bool MovieElement::scriptSetPaused(const DynamicValue& dest) {
	if (dest.getType() == DynamicValueTypes::kBoolean) {
		_paused = dest.getBool();
		return true;
	}
	return false;
}

void MovieElement::onSegmentUnloaded(int segmentIndex) {
	_videoDecoder.reset();
}

VThreadState MovieElement::startPlayingTask(const StartPlayingTaskData &taskData) {
	if (_videoDecoder && !_videoDecoder->isPlaying()) {
		EventIDs::EventID eventToSend = EventIDs::kPlay;
		if (_paused) {
			_paused = false;
			eventToSend = EventIDs::kUnpause;
		}

		_videoDecoder->start();

		Common::SharedPtr<MessageProperties> msgProps(new MessageProperties(Event::create(eventToSend, 0), DynamicValue(), getSelfReference()));
		Common::SharedPtr<MessageDispatch> dispatch(new MessageDispatch(msgProps, this, false, true, false));
		taskData.runtime->sendMessageOnVThread(dispatch);
	}

	return kVThreadReturn;
}


ImageElement::ImageElement() : _cacheBitmap(false), _runtime(nullptr) {
}

ImageElement::~ImageElement() {
}

bool ImageElement::load(ElementLoaderContext &context, const Data::ImageElement &data) {
	if (!VisualElement::loadCommon(data.name, data.guid, data.rect1, data.elementFlags, data.layer, data.streamLocator, data.sectionID))
		return false;

	_cacheBitmap = ((data.elementFlags & Data::ElementFlags::kCacheBitmap) != 0);
	_runtime = context.runtime;
	_assetID = data.imageAssetID;

	return true;
}

bool ImageElement::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	return VisualElement::readAttribute(thread, result, attrib);
}

bool ImageElement::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	return VisualElement::writeRefAttribute(thread, writeProxy, attrib);
}

void ImageElement::activate() {
	Project *project = _runtime->getProject();
	Common::SharedPtr<Asset> asset = project->getAssetByID(_assetID).lock();

	if (!asset) {
		warning("Image element references asset %i but the asset isn't loaded!", _assetID);
		return;
	}

	if (asset->getAssetType() != kAssetTypeImage) {
		warning("Image element assigned an asset that isn't an image");
		return;
	}

	ImageAsset *imageAsset = static_cast<ImageAsset *>(asset.get());
	size_t streamIndex = imageAsset->getStreamIndex();
	int segmentIndex = project->getSegmentForStreamIndex(streamIndex);
	project->openSegmentStream(segmentIndex);
	Common::SeekableReadStream *stream = project->getStreamForSegment(segmentIndex);

	if (!stream->seek(imageAsset->getFilePosition())) {
		warning("Image element failed to load");
		return;
	}

	size_t bytesPerRow = 0;

	Rect16 imageRect = imageAsset->getRect();
	int width = imageRect.right - imageRect.left;
	int height = imageRect.bottom - imageRect.top;

	if (width <= 0 || height < 0) {
		warning("Image asset has invalid size");
		return;
	}

	Graphics::PixelFormat pixelFmt;
	switch (imageAsset->getColorDepth()) {
	case kColorDepthMode1Bit:
		bytesPerRow = (width + 7) / 8;
		pixelFmt = Graphics::PixelFormat::createFormatCLUT8();
		break;
	case kColorDepthMode2Bit:
		bytesPerRow = (width + 3) / 4;
		pixelFmt = Graphics::PixelFormat::createFormatCLUT8();
		break;
	case kColorDepthMode4Bit:
		bytesPerRow = (width + 1) / 2;
		pixelFmt = Graphics::PixelFormat::createFormatCLUT8();
		break;
	case kColorDepthMode8Bit:
		bytesPerRow = width;
		pixelFmt = Graphics::PixelFormat::createFormatCLUT8();
		break;
	case kColorDepthMode16Bit:
		bytesPerRow = width * 2;
		pixelFmt = Graphics::createPixelFormat<1555>();
		break;
	case kColorDepthMode32Bit:
		bytesPerRow = width * 4;
		pixelFmt = Graphics::createPixelFormat<8888>();
		break;
	default:
		warning("Image asset has an unrecognizable pixel format");
		return;
	}

	// If this is the same mode as the render target, then copy the exact mode
	// so blits go faster
	if (imageAsset->getColorDepth() == _runtime->getRealColorDepth()) {
		pixelFmt = _runtime->getRenderPixelFormat();
	}

	Common::Array<uint8> rowBuffer;
	rowBuffer.resize(bytesPerRow);

	ImageAsset::ImageFormat imageFormat = imageAsset->getImageFormat();
	bool bottomUp = (imageFormat == ImageAsset::kImageFormatWindows);
	bool isBigEndian = (imageFormat == ImageAsset::kImageFormatMac);

	_imageSurface.reset(new Graphics::Surface());
	_imageSurface->create(width, height, pixelFmt);

	for (int inRow = 0; inRow < height; inRow++) {
		int outRow = bottomUp ? (height - 1 - inRow) : inRow;

		stream->read(&rowBuffer[0], bytesPerRow);
		const uint8 *inRowBytes = &rowBuffer[0];

		void *outBase = _imageSurface->getBasePtr(0, outRow);

		switch (imageAsset->getColorDepth()) {
		case kColorDepthMode1Bit: {
				for (int x = 0; x < width; x++) {
					int bit = (inRowBytes[x / 8] >> (7 - (x % 8))) & 1;
					static_cast<uint8 *>(outBase)[x] = bit;
				}
			} break;
		case kColorDepthMode2Bit: {
				for (int x = 0; x < width; x++) {
					int bit = (inRowBytes[x / 4] >> (3 - (x % 4))) & 3;
					static_cast<uint8 *>(outBase)[x] = bit;
				}
			} break;
		case kColorDepthMode4Bit: {
				for (int x = 0; x < width; x++) {
					int bit = (inRowBytes[x / 2] >> (1 - (x % 2))) & 15;
					static_cast<uint8 *>(outBase)[x] = bit;
				}
			} break;
		case kColorDepthMode8Bit:
			memcpy(outBase, inRowBytes, width);
			break;
		case kColorDepthMode16Bit: {
				if (isBigEndian) {
					for (int x = 0; x < width; x++) {
						uint16 packedPixel = inRowBytes[x * 2 + 1] + (inRowBytes[x * 2 + 0] << 8);
						int r = ((packedPixel >> 10) & 0x1f);
						int g = ((packedPixel >> 5) & 0x1f);
						int b = (packedPixel & 0x1f);

						uint16 repacked = (1 << pixelFmt.aShift) | (r << pixelFmt.rShift) | (g << pixelFmt.gShift) | (b << pixelFmt.bShift);
						static_cast<uint16 *>(outBase)[x] = repacked;
					}
				} else {
					for (int x = 0; x < width; x++) {
						uint16 packedPixel = inRowBytes[x * 2 + 0] + (inRowBytes[x * 2 + 1] << 8);
						int r = ((packedPixel >> 10) & 0x1f);
						int g = ((packedPixel >> 5) & 0x1f);
						int b = (packedPixel & 0x1f);

						uint16 repacked = (1 << pixelFmt.aShift) | (r << pixelFmt.rShift) | (g << pixelFmt.gShift) | (b << pixelFmt.bShift);
						static_cast<uint16 *>(outBase)[x] = repacked;
					}
				}
			} break;
		case kColorDepthMode32Bit: {
				if (imageFormat == ImageAsset::kImageFormatMac) {
					for (int x = 0; x < width; x++) {
						uint8 r = inRowBytes[x * 4 + 0];
						uint8 g = inRowBytes[x * 4 + 1];
						uint8 b = inRowBytes[x * 4 + 2];
						uint32 repacked = (255 << pixelFmt.aShift) | (r << pixelFmt.rShift) | (g << pixelFmt.gShift) | (b << pixelFmt.bShift);
						static_cast<uint32 *>(outBase)[x] = repacked;
					}
				} else if (imageFormat == ImageAsset::kImageFormatWindows) {
					for (int x = 0; x < width; x++) {
						uint8 r = inRowBytes[x * 4 + 2];
						uint8 g = inRowBytes[x * 4 + 1];
						uint8 b = inRowBytes[x * 4 + 0];
						uint32 repacked = (255 << pixelFmt.aShift) | (r << pixelFmt.rShift) | (g << pixelFmt.gShift) | (b << pixelFmt.bShift);
						static_cast<uint32 *>(outBase)[x] = repacked;
					}
				}
			} break;
		default:
			break;
		}
	}
}

void ImageElement::deactivate() {
	_imageSurface.reset();
}

void ImageElement::render(Window *window) {
	if (_imageSurface) {
		Common::Rect srcRect(_imageSurface->w, _imageSurface->h);
		Common::Rect destRect(_rect.left, _rect.top, _rect.right, _rect.bottom);
		window->getSurface()->blitFrom(*_imageSurface, srcRect, destRect);
	}
}

TextLabelElement::TextLabelElement() : _needsRender(false), _isBitmap(false) {
}

TextLabelElement::~TextLabelElement() {
}

bool TextLabelElement::load(ElementLoaderContext &context, const Data::TextLabelElement &data) {
	if (!loadCommon(data.name, data.guid, data.rect1, data.elementFlags, data.layer, 0, data.sectionID))
		return false;

	_cacheBitmap = ((data.elementFlags & Data::ElementFlags::kCacheBitmap) != 0);
	_assetID = data.assetID;
	_runtime = context.runtime;

	return true;
}

bool TextLabelElement::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	return VisualElement::readAttribute(thread, result, attrib);
}

bool TextLabelElement::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	return VisualElement::writeRefAttribute(thread, writeProxy, attrib);
}

void TextLabelElement::activate() {
	Project *project = _runtime->getProject();
	Common::SharedPtr<Asset> asset = project->getAssetByID(_assetID).lock();

	if (!asset) {
		warning("Text element references asset %i but the asset isn't loaded!", _assetID);
		return;
	}

	if (asset->getAssetType() != kAssetTypeText) {
		warning("Text element assigned an asset that isn't text");
		return;
	}

	TextAsset *textAsset = static_cast<TextAsset *>(asset.get());

	if (textAsset->isBitmap()) {
		_renderedText = textAsset->getBitmapSurface();
		_needsRender = false;
	} else {
		_needsRender = true;
		_text = textAsset->getString();
		_macFormattingSpans = textAsset->getMacFormattingSpans();
	}
}

void TextLabelElement::deactivate() {
}


void TextLabelElement::render(Window *window) {
}

SoundElement::SoundElement() {
}

SoundElement::~SoundElement() {
}

bool SoundElement::load(ElementLoaderContext &context, const Data::SoundElement &data) {
	if (!NonVisualElement::loadCommon(data.name, data.guid, data.elementFlags))
		return false;

	_paused = ((data.soundFlags & Data::SoundElement::kPaused) != 0);
	_leftVolume = data.leftVolume;
	_rightVolume = data.rightVolume;
	_balance = data.balance;
	_assetID = data.assetID;
	_runtime = context.runtime;

	return true;
}

bool SoundElement::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	return NonVisualElement::readAttribute(thread, result, attrib);
}

bool SoundElement::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &writeProxy, const Common::String &attrib) {
	return NonVisualElement::writeRefAttribute(thread, writeProxy, attrib);
}

void SoundElement::activate() {
}

void SoundElement::deactivate() {
}

} // End of namespace MTropolis
