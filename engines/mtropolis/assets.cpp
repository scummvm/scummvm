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

#include "mtropolis/assets.h"
#include "mtropolis/asset_factory.h"

#include "graphics/surface.h"

#include "audio/audiostream.h"

#include "common/endian.h"

namespace MTropolis {

Asset::Asset() : _assetID(0) {
}

Asset::~Asset() {
}

uint32 Asset::getAssetID() const {
	return _assetID;
}

bool ColorTableAsset::load(AssetLoaderContext &context, const Data::ColorTableAsset &data) {
	_assetID = data.assetID;
	for (int i = 0; i < 256; i++) {
		if (!_colors[i].load(data.colors[i]))
			return false;
	}

	return true;
}

AssetType ColorTableAsset::getAssetType() const {
	return kAssetTypeColorTable;
}


CachedAudio::CachedAudio() {
}

bool CachedAudio::loadFromStream(const AudioMetadata &metadata, Common::ReadStream *stream, size_t size) {
	_data.resize(size);
	if (size > 0) {
		stream->read(&_data[0], size);
		if (stream->err())
			return false;

		if (metadata.encoding == AudioMetadata::kEncodingUncompressed && metadata.bitsPerSample == 16) {
			int16 *samples = reinterpret_cast<int16 *>(&_data[0]);
			size_t numSamples = _data.size() / 2;

			if (metadata.isBigEndian) {
				for (size_t i = 0; i < numSamples; i++)
					samples[i] = FROM_BE_16(samples[i]);
			} else {
				for (size_t i = 0; i < numSamples; i++)
					samples[i] = FROM_LE_16(samples[i]);
			}
		}

		return true;
	}
	return true;
}

const void *CachedAudio::getData() const {
	if (_data.size() == 0)
		return nullptr;
	return &_data[0];
}

size_t CachedAudio::getSize() const {
	return _data.size();
}

size_t CachedAudio::getNumSamples(const AudioMetadata &metadata) const {
	switch (metadata.encoding) {
	case AudioMetadata::kEncodingMace6:
		return _data.size() * 6 / metadata.channels;
	case AudioMetadata::kEncodingMace3:
		return _data.size() * 3 / metadata.channels;
	case AudioMetadata::kEncodingUncompressed:
		return _data.size() / (metadata.channels * metadata.bitsPerSample / 8);
	default:
		return 0;
	}
}

CachedMToon::CachedMToon() : _isRLETemporalCompressed(false) {
}

bool CachedMToon::loadFromStream(const Common::SharedPtr<MToonMetadata> &metadata, Common::ReadStream *stream, size_t size) {
	_metadata = metadata;

	Common::Array<uint8> data;
	data.resize(size);
	if (size > 0) {
		stream->read(&data[0], size);
		if (stream->err())
			return false;
	}

	if (metadata->codecID == kMToonRLECodecID) {
		loadRLEFrames(data);
		uint16 firstFrameWidth = metadata->frames[0].rect.getWidth();
		uint16 firstFrameHeight = metadata->frames[0].rect.getHeight();

		_isRLETemporalCompressed = true;
		for (size_t i = 0; i < metadata->frames.size(); i++) {
			const MToonMetadata::FrameDef &frame = metadata->frames[i];
			if (frame.rect.getWidth() != firstFrameWidth || frame.rect.getHeight() != firstFrameHeight) {
				_isRLETemporalCompressed = false;
				break;
			}
		}
	}

	if (!_isRLETemporalCompressed)
		decompressFrames(data);

	return true;
}

void CachedMToon::decompressFrames(const Common::Array<uint8> &data) {
	size_t numFrames = _metadata->frames.size();

	_decompressedFrames.resize(numFrames);
	_optimizedFrames.resize(numFrames);

	for (size_t i = 0; i < numFrames; i++) {
		if (_metadata->codecID == kMToonRLECodecID) {
			decompressRLEFrame(i);
		} else {
			loadUncompressedFrame(data, i);
		}
	}

	_dataRLE8.clear();
	_dataRLE16.clear();
	_dataRLE32.clear();
}

template<class TFrame, class TNumber, uint32 TLiteralMask, uint32 TTransparentRowSkipMask>
static bool decompressMToonRLE(const TFrame &frame, Graphics::Surface &surface, bool isBottomUp) {
	assert(sizeof(TNumber) == surface.format.bytesPerPixel);

	const Common::Array<TNumber> &coefsArray = frame.data;

	size_t size = coefsArray.size();
	if (size == 0)
		return false;

	const TNumber *coefs = &coefsArray[0];

	size_t x = 0;
	size_t y = 0;
	size_t w = surface.w;
	size_t h = surface.h;

	if (w != frame.width || h != frame.height)
		return false;

	TNumber *rowData = static_cast<TNumber *>(surface.getBasePtr(0, isBottomUp ? (h - 1) : 0));

	for (;;) {
		if (size == 0)
			return false;
		const TNumber rleCode = coefs[0];
		coefs++;
		size--;

		size_t remainingInRow = w - x;

		if (rleCode == 0) {
			if (size == 0)
				return false;
			const TNumber transparentCountCode = coefs[0];
			coefs++;
			size--;

			if (transparentCountCode & TTransparentRowSkipMask) {
				// Vertical skip
				y += (transparentCountCode - TTransparentRowSkipMask);
				x = 0;
				if (y < h) {
					rowData = static_cast<TNumber *>(surface.getBasePtr(0, isBottomUp ? (h - 1 - y) : y));
					continue;
				} else {
					break;
				}
			} else {
				// Horizontal skip
				const size_t horizontalSkip = transparentCountCode;
				if (horizontalSkip > remainingInRow)
					return false;
				x += horizontalSkip;
			}
		} else if (rleCode & TLiteralMask) {
			// Literals
			const size_t numLiterals = (rleCode - TLiteralMask);
			if (numLiterals > size || numLiterals > remainingInRow)
				return false;
			memcpy(rowData + x, coefs, sizeof(TNumber) * numLiterals);
			coefs += numLiterals;
			size -= numLiterals;
			x += numLiterals;
		} else {
			// Literals
			const size_t numCopies = rleCode;
			if (numCopies > remainingInRow || size == 0)
				return false;
			const TNumber repeatedValue = coefs[0];
			for (size_t i = 0; i < numCopies; i++)
				rowData[x + i] = repeatedValue;
			coefs++;
			size--;
			x += numCopies;
		}

		if (x == w) {
			y++;
			x = 0;
			if (y < h) {
				rowData = static_cast<TNumber *>(surface.getBasePtr(0, isBottomUp ? (h - 1 - y) : y));
				continue;
			} else {
				break;
			}
		}
	}

	return true;
}

void CachedMToon::decompressRLEFrameToImage(size_t frameIndex, Graphics::Surface &surface) {
	assert(surface.format == _rleOptimizedFormat);

	const MToonMetadata::FrameDef &frameDef = _metadata->frames[frameIndex];

	int32 originX = frameDef.rect.left;
	int32 originY = frameDef.rect.top;

	bool isBottomUp = (_metadata->imageFormat == MToonMetadata::kImageFormatWindows);

	bool decompressedOK = false;
	if (_rleOptimizedFormat.bytesPerPixel == 4) {
		decompressedOK = decompressMToonRLE<Rle32Frame, uint32, 0x80000000u, 0x80000000u>(_dataRLE32[frameIndex], surface, isBottomUp);
	} else if (_rleOptimizedFormat.bytesPerPixel == 2) {
		decompressedOK = decompressMToonRLE<Rle16Frame, uint16, 0x8000u, 0x8000u>(_dataRLE16[frameIndex], surface, isBottomUp);
	} else if (_rleOptimizedFormat.bytesPerPixel == 1) {
		decompressedOK = decompressMToonRLE<Rle8Frame, uint8, 0x80u, 0u>(_dataRLE8[frameIndex], surface, isBottomUp);
	} else
		error("Unknown mToon encoding");

	if (!decompressedOK)
		warning("mToon RLE frame decompression failed");
}

void CachedMToon::loadRLEFrames(const Common::Array<uint8> &data) {
	size_t numFrames = _metadata->frames.size();
	uint16 bpp = _metadata->bitsPerPixel;

	if (bpp == 8)
		_dataRLE8.resize(numFrames);
	else if (bpp == 16)
		_dataRLE16.resize(numFrames);
	else
		error("Unsupported RLE encoding");

	for (size_t i = 0; i < numFrames; i++) {
		const MToonMetadata::FrameDef &frameDef = _metadata->frames[i];

		RleFrame *rleFrame = nullptr;
		if (bpp == 8)
			rleFrame = &_dataRLE8[i];
		else if (bpp == 16)
			rleFrame = &_dataRLE16[i];

		size_t baseOffset = frameDef.dataOffset;

		uint32 headerInts[5];
		for (size_t hi = 0; hi < 5; hi++) {
			uint32 unpacked = 0;
			for (size_t b = 0; b < 4; b++)
				unpacked = (unpacked << 8) + data[baseOffset + hi * 4 + b];
			headerInts[hi] = unpacked;
		}

		rleFrame->isKeyframe = (headerInts[0] == kMToonRLEKeyframePrefix);
		if (headerInts[1] == 0x01000001) {
			if (bpp != 8)
				error("Unknown mToon encoding");
		} else if (headerInts[1] == 0x01000002) {
			if (bpp != 16)
				error("Unknown mToon encoding");
		} else
			error("Unknown mToon encoding");

		rleFrame->version = headerInts[1];
		rleFrame->width = headerInts[2];
		rleFrame->height = headerInts[3];

		uint32 frameDataSize = headerInts[4];

		if (frameDataSize > 0) {
			if (bpp == 8) {
				_dataRLE8[i].data.resize(frameDataSize);
				memcpy(&_dataRLE8[i].data[0], &data[baseOffset + 20], frameDataSize);
			} else if (bpp == 16) {
				// RLE16 data size excludes the header
				frameDataSize -= 20;

				uint32 numDWords = frameDataSize / 2;
				_dataRLE16[i].data.resize(numDWords);
				memcpy(&_dataRLE16[i].data[0], &data[baseOffset + 20], numDWords * 2);

				uint16 *i16 = &_dataRLE16[i].data[0];
				if (_metadata->imageFormat == MToonMetadata::kImageFormatWindows) {
					for (size_t swapIndex = 0; swapIndex < numDWords; swapIndex++)
						i16[swapIndex] = FROM_LE_16(i16[swapIndex]);
				} else if (_metadata->imageFormat == MToonMetadata::kImageFormatMac) {
					for (size_t swapIndex = 0; swapIndex < numDWords; swapIndex++)
						i16[swapIndex] = FROM_BE_16(i16[swapIndex]);
				}
			} else
				error("Unknown mToon encoding");
		}
	}

	if (bpp == 8)
		_rleInternalFormat = Graphics::PixelFormat::createFormatCLUT8();
	else if (bpp == 16)
		_rleInternalFormat = Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 15);
	else
		error("Unknown mToon encoding");

	_rleOptimizedFormat = _rleInternalFormat;
}

void CachedMToon::decompressRLEFrame(size_t frameIndex) {
	Common::SharedPtr<Graphics::Surface> surface(new Graphics::Surface());

	RleFrame *frame = nullptr;
	if (_rleInternalFormat.bytesPerPixel == 1)
		frame = &_dataRLE8[frameIndex];
	else if (_rleInternalFormat.bytesPerPixel == 2)
		frame = &_dataRLE16[frameIndex];
	else
		error("Unknown mToon encoding");

	surface->create(frame->width, frame->height, _rleInternalFormat);

	decompressRLEFrameToImage(frameIndex, *surface);

	this->_decompressedFrames[frameIndex] = surface;
}

void CachedMToon::loadUncompressedFrame(const Common::Array<uint8> &data, size_t frameIndex) {
	const MToonMetadata::FrameDef &frameDef = _metadata->frames[frameIndex];
	uint16 stride = frameDef.decompressedBytesPerRow;

	uint16 bpp = _metadata->bitsPerPixel;

	Common::SharedPtr<Graphics::Surface> surface(new Graphics::Surface());
	Graphics::PixelFormat pixFmt;

	if (bpp == 1 || bpp == 2 || bpp == 4 || bpp == 8)
		pixFmt = Graphics::PixelFormat::createFormatCLUT8();
	else if (bpp == 16)
		pixFmt = Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 15);
	else if (bpp == 32)
		pixFmt = Graphics::PixelFormat(4, 8, 8, 8, 0, 0, 8, 16, 24);
	else
		error("Unknown mToon encoding");

	size_t w = frameDef.rect.getWidth();
	size_t h = frameDef.rect.getHeight();

	surface->create(w, h, pixFmt);

	for (size_t row = 0; row < h; row++) {
		const uint8 *inData = &data[frameDef.dataOffset + row * stride];
		void *outDataUntyped = nullptr;

		if (_metadata->imageFormat == MToonMetadata::kImageFormatWindows)
			outDataUntyped = surface->getBasePtr(0, h - 1 - row);
		else if (_metadata->imageFormat == MToonMetadata::kImageFormatMac)
			outDataUntyped = surface->getBasePtr(0, row);
		else
			error("Unimplemented mToon uncompressed image layout");

		if (bpp == 1) {
			for (size_t col = 0; col < w; col++)
				static_cast<uint8 *>(outDataUntyped)[col] = (inData[col / 8] >> (7 - (col % 8))) & 1;
		} else if (bpp == 2) {
			for (size_t col = 0; col < w; col++)
				static_cast<uint8 *>(outDataUntyped)[col] = (inData[col / 4] >> (6 - (col % 4) * 2)) & 3;
		} else if (bpp == 4) {
			for (size_t col = 0; col < w; col++)
				static_cast<uint8 *>(outDataUntyped)[col] = (inData[col / 2] >> (4 - (col % 2) * 4)) & 15;
		} else if (bpp == 8) {
			for (size_t col = 0; col < w; col++)
				static_cast<uint8 *>(outDataUntyped)[col] = inData[col];
		} else if (bpp == 16) {
			if (_metadata->imageFormat == MToonMetadata::kImageFormatMac) {
				for (size_t col = 0; col < w; col++)
					static_cast<uint16 *>(outDataUntyped)[col] = (inData[col * 2] << 8) | (inData[col * 2 + 1]);
			} else if (_metadata->imageFormat == MToonMetadata::kImageFormatWindows) {
				for (size_t col = 0; col < w; col++)
					static_cast<uint16 *>(outDataUntyped)[col] = (inData[col * 2 + 1] << 8) | (inData[col * 2]);
			}
		} else if (bpp == 32) {
			for (size_t col = 0; col < w; col++)
				static_cast<uint32 *>(outDataUntyped)[col] = (0xff000000 | (inData[col * 4]) | (inData[col * 4 + 1] << 8) | (inData[col * 4 + 2] << 16));
		}
	}

	_decompressedFrames[frameIndex] = surface;
}

template<class TSrcFrame, class TSrcNumber, uint32 TSrcLiteralMask, uint32 TSrcTransparentSkipMask, class TDestFrame, class TDestNumber, uint32 TDestLiteralMask, uint32 TDestTransparentSkipMask>
void CachedMToon::rleReformat(const TSrcFrame &srcFrame, const Graphics::PixelFormat &srcFormatRef, TDestFrame &destFrame, const Graphics::PixelFormat &destFormatRef) {
	const Graphics::PixelFormat srcFormat = srcFormatRef;
	const Graphics::PixelFormat destFormat = destFormatRef;

	size_t offset = 0;
	destFrame.data.resize(srcFrame.data.size());
	destFrame.width = srcFrame.width;
	destFrame.height = srcFrame.height;
	destFrame.isKeyframe = srcFrame.isKeyframe;
	destFrame.version = srcFrame.version;

	while (offset < srcFrame.data.size()) {
		uint32 rleCodeOffset = offset;
		const uint32 rleCode = srcFrame.data[rleCodeOffset];
		if (rleCode == 0) {
			destFrame.data[offset] = 0;
			offset++;

			uint32 numTransparentCode = srcFrame.data[offset];
			if (numTransparentCode & TSrcTransparentSkipMask)
				destFrame.data[offset] = (numTransparentCode - TSrcTransparentSkipMask) + TDestTransparentSkipMask;
			else
				destFrame.data[offset] = numTransparentCode;
			offset++;
		} else if (rleCode & TSrcLiteralMask) {
			uint32 numLiterals = rleCode - TSrcLiteralMask;

			destFrame.data[offset] = numLiterals + TDestLiteralMask;
			offset++;

			while (numLiterals) {
				uint8 a, r, g, b;
				srcFormat.colorToARGB(srcFrame.data[offset], a, r, g, b);
				destFrame.data[offset] = destFormat.ARGBToColor(a, r, g, b);
				offset++;
				numLiterals--;
			}
		} else {
			destFrame.data[offset] = rleCode;
			offset++;

			uint8 a, r, g, b;
			srcFormat.colorToARGB(srcFrame.data[offset], a, r, g, b);
			destFrame.data[offset] = destFormat.ARGBToColor(a, r, g, b);
			offset++;
		}
	}
}

void CachedMToon::optimize(Runtime *runtime) {
	Graphics::PixelFormat renderFmt = runtime->getRenderPixelFormat();
	if (_isRLETemporalCompressed)
		optimizeRLE(renderFmt);
	else
		optimizeNonTemporal(renderFmt);
}

void CachedMToon::optimizeNonTemporal(const Graphics::PixelFormat &targetFormatRef) {
	const Graphics::PixelFormat targetFormat = targetFormatRef;

	_optimizedFrames.resize(_decompressedFrames.size());

	for (size_t i = 0; i < _decompressedFrames.size(); i++) {
		Common::SharedPtr<Graphics::Surface> srcSurface = _decompressedFrames[i];
		Common::SharedPtr<Graphics::Surface> &optimizedSurfRef = _optimizedFrames[i];

		// FIXME: Aggregate these checks and merge into a single format field
		if (optimizedSurfRef == nullptr || optimizedSurfRef->format != targetFormat) {
			if (targetFormat.bytesPerPixel > 1 && srcSurface->format.bytesPerPixel > 1) {
				if (targetFormat.bytesPerPixel == srcSurface->format.bytesPerPixel) {
					srcSurface->convertToInPlace(targetFormat);
					optimizedSurfRef = srcSurface;
				} else {
					optimizedSurfRef.reset();
					optimizedSurfRef.reset(srcSurface->convertTo(targetFormat));
				}
			} else {
				optimizedSurfRef = srcSurface;
			}
		}
	}
}

void CachedMToon::optimizeRLE(const Graphics::PixelFormat &targetFormatRef) {
	const Graphics::PixelFormat targetFormat = targetFormatRef;

	if (targetFormat == _rleOptimizedFormat)
		return;

	if (_rleInternalFormat.bytesPerPixel != 2 && _rleInternalFormat.bytesPerPixel != 4)
		return;	// Can't optimize

	size_t numFrames = _metadata->frames.size();
	for (size_t i = 0; i < numFrames; i++) {
		if (_rleInternalFormat.bytesPerPixel == 2) {
			if (targetFormat.bytesPerPixel == 4)
				rleReformat<Rle16Frame, uint16, 0x8000u, 0x8000u, Rle32Frame, uint32, 0x80000000u, 0x80000000u>(_dataRLE16[i], _rleInternalFormat, _dataRLE32[i], targetFormat);
			else if (targetFormat.bytesPerPixel == 2)
				rleReformat<Rle16Frame, uint16, 0x8000u, 0x8000u, Rle16Frame, uint16, 0x8000u, 0x8000u>(_dataRLE16[i], _rleInternalFormat, _dataRLE16[i], targetFormat);
		} else if (_rleInternalFormat.bytesPerPixel == 4) {
			if (targetFormat.bytesPerPixel == 4)
				rleReformat<Rle32Frame, uint32, 0x80000000u, 0x80000000u, Rle32Frame, uint32, 0x80000000u, 0x80000000u>(_dataRLE32[i], _rleInternalFormat, _dataRLE32[i], targetFormat);
			else if (targetFormat.bytesPerPixel == 2)
				rleReformat<Rle32Frame, uint32, 0x80000000u, 0x80000000u, Rle16Frame, uint16, 0x8000u, 0x8000u>(_dataRLE32[i], _rleInternalFormat, _dataRLE16[i], targetFormat);
		}
	}

	if (_rleInternalFormat.bytesPerPixel == targetFormat.bytesPerPixel)
		_rleInternalFormat = targetFormat;

	_rleOptimizedFormat = targetFormat;
}

void CachedMToon::getOrRenderFrame(uint32 prevFrame, uint32 targetFrame, Common::SharedPtr<Graphics::Surface>& surface) const {
	if (!_isRLETemporalCompressed) {
		surface = _optimizedFrames[targetFrame];
	} else if (_metadata->codecID == kMToonRLECodecID) {
		uint32 firstFrameToRender = 0;
		uint32 backStopFrame = 0;

		if (surface && surface->format != _rleOptimizedFormat)
			surface.reset();

		if (surface != nullptr) {
			if (prevFrame == targetFrame)
				return;
			if (prevFrame < targetFrame)
				backStopFrame = prevFrame;
		}

		firstFrameToRender = targetFrame;
		while (firstFrameToRender > backStopFrame) {
			if (_metadata->frames[firstFrameToRender].isKeyFrame)
				break;
			firstFrameToRender--;
		}

		if (!surface || surface->format != _rleOptimizedFormat) {
			surface.reset(new Graphics::Surface());
			surface->create(_metadata->rect.getWidth(), _metadata->rect.getHeight(), _rleOptimizedFormat);
		}

		bool isBottomUp = (_metadata->imageFormat == MToonMetadata::kImageFormatWindows);

		for (size_t i = firstFrameToRender; i <= targetFrame; i++) {
			if (_rleOptimizedFormat.bytesPerPixel == 1)
				decompressMToonRLE<Rle8Frame, uint8, 0x80u, 0>(_dataRLE8[i], *surface, isBottomUp);
			else if (_rleOptimizedFormat.bytesPerPixel == 2)
				decompressMToonRLE<Rle16Frame, uint16, 0x8000u, 0x8000u>(_dataRLE16[i], *surface, isBottomUp);
			else if (_rleOptimizedFormat.bytesPerPixel == 4)
				decompressMToonRLE<Rle32Frame, uint32, 0x80000000u, 0x80000000u>(_dataRLE32[i], *surface, isBottomUp);
		}
	}
}

const Common::SharedPtr<MToonMetadata>& CachedMToon::getMetadata() const {
	return _metadata;
}

bool AudioAsset::load(AssetLoaderContext &context, const Data::AudioAsset &data) {
	_assetID = data.assetID;

	_metadata.reset(new AudioMetadata());
	_metadata->sampleRate = data.sampleRate1;
	_metadata->bitsPerSample = data.bitsPerSample;

	_streamIndex = context.streamIndex;

	switch (data.encoding1) {
	case 0:
		_metadata->encoding = AudioMetadata::kEncodingUncompressed;
		break;
	case 3:
		_metadata->encoding = AudioMetadata::kEncodingMace3;
		break;
	case 4:
		_metadata->encoding = AudioMetadata::kEncodingMace6;
		break;
	default:
		return false;
	}

	_metadata->channels = data.channels;
	// Hours Minutes Seconds Hundredths -> msec
	// Maximum is 0x37a4f52e so this fits in 30 bits
	_metadata->durationMSec = ((((data.codedDuration[0] * 60u) + data.codedDuration[1]) * 60u + data.codedDuration[2]) * 100u + data.codedDuration[3]) * 10u;
	_filePosition = data.filePosition;
	_size = data.size;
	_metadata->cuePoints.resize(data.cuePoints.size());
	_metadata->isBigEndian = data.isBigEndian;

	for (size_t i = 0; i < data.cuePoints.size(); i++) {
		_metadata->cuePoints[i].cuePointID = data.cuePoints[i].cuePointID;
		_metadata->cuePoints[i].position = data.cuePoints[i].position;
	}

	return true;
}

AssetType AudioAsset::getAssetType() const {
	return kAssetTypeAudio;
}

size_t AudioAsset::getStreamIndex() const {
	return _streamIndex;
}

const Common::SharedPtr<AudioMetadata> &AudioAsset::getMetadata() const {
	return _metadata;
}


const Common::SharedPtr<CachedAudio> &AudioAsset::loadAndCacheAudio(Runtime *runtime) {
	if (_audioCache)
		return _audioCache;

	size_t streamIndex = getStreamIndex();
	int segmentIndex = runtime->getProject()->getSegmentForStreamIndex(streamIndex);
	runtime->getProject()->openSegmentStream(segmentIndex);
	Common::SeekableReadStream *stream = runtime->getProject()->getStreamForSegment(segmentIndex);

	if (!stream || !stream->seek(_filePosition)) {
		warning("Audio asset failed to load, couldn't seek to position");
		return _audioCache;
	}

	Common::SharedPtr<CachedAudio> audio(new CachedAudio());
	if (!audio->loadFromStream(*_metadata, stream, _size)) {
		warning("Audio asset failed to load, couldn't read data");
		return _audioCache;
	}

	_audioCache.reset();
	_audioCache = audio;

	return _audioCache;
}

bool MovieAsset::load(AssetLoaderContext &context, const Data::MovieAsset &data) {
	_assetID = data.assetID;
	_moovAtomPos = data.moovAtomPos;
	_movieDataPos = data.movieDataPos;
	_movieDataSize = data.movieDataSize;
	_extFileName = data.extFileName;
	_streamIndex = context.streamIndex;

	return true;
}

AssetType MovieAsset::getAssetType() const {
	return kAssetTypeMovie;
}

uint32 MovieAsset::getMovieDataPos() const {
	return _movieDataPos;
}

uint32 MovieAsset::getMoovAtomPos() const {
	return _moovAtomPos;
}

uint32 MovieAsset::getMovieDataSize() const {
	return _movieDataSize;
}


const Common::String &MovieAsset::getExtFileName() const {
	return _extFileName;
}

size_t MovieAsset::getStreamIndex() const {
	return _streamIndex;
}


CachedImage::CachedImage() : _colorDepth(kColorDepthModeInvalid), _isOptimized(false) {
}

void CachedImage::resetSurface(ColorDepthMode colorDepth, const Common::SharedPtr<Graphics::Surface> &surface) {
	_optimizedSurface.reset();
	_isOptimized = false;

	_colorDepth = colorDepth;
	_surface = surface;
}

const Common::SharedPtr<Graphics::Surface> &CachedImage::optimize(Runtime *runtime) {
	ColorDepthMode renderDepth = runtime->getRealColorDepth();
	const Graphics::PixelFormat &renderFmt = runtime->getRenderPixelFormat();

	if (renderDepth != _colorDepth) {
		size_t w = _surface->w;
		size_t h = _surface->h;

		if (renderDepth == kColorDepthMode16Bit && _colorDepth == kColorDepthMode32Bit) {
			_optimizedSurface.reset(new Graphics::Surface());
			_optimizedSurface->create(w, h, renderFmt);
			Render::convert32To16(*_optimizedSurface, *_surface);
		} else if (renderDepth == kColorDepthMode32Bit && _colorDepth == kColorDepthMode16Bit) {
			_optimizedSurface.reset(new Graphics::Surface());
			_optimizedSurface->create(w, h, renderFmt);
			Render::convert16To32(*_optimizedSurface, *_surface);
		} else {
			_optimizedSurface = _surface;	// Can't optimize
		}
	} else {
		_surface->convertToInPlace(renderFmt, nullptr);
		_optimizedSurface = _surface;
	}

	return _optimizedSurface;
}

ImageAsset::ImageAsset() {
}

ImageAsset::~ImageAsset() {
}

bool ImageAsset::load(AssetLoaderContext &context, const Data::ImageAsset &data) {
	_assetID = data.assetID;
	if (!_rect.load(data.rect1))
		return false;
	_filePosition = data.filePosition;
	_size = data.size;
	_streamIndex = context.streamIndex;

	switch (data.bitsPerPixel) {
	case 1:
		_colorDepth = kColorDepthMode1Bit;
		break;
	case 2:
		_colorDepth = kColorDepthMode2Bit;
		break;
	case 4:
		_colorDepth = kColorDepthMode4Bit;
		break;
	case 8:
		_colorDepth = kColorDepthMode8Bit;
		break;
	case 16:
		_colorDepth = kColorDepthMode16Bit;
		break;
	case 32:
		_colorDepth = kColorDepthMode32Bit;
		break;
	default:
		return false;
	}

	if (data.haveMacPart)
		_imageFormat = kImageFormatMac;
	else if (data.haveWinPart)
		_imageFormat = kImageFormatWindows;
	else
		return false;

	return true;
}

AssetType ImageAsset::getAssetType() const {
	return kAssetTypeImage;
}

const Rect16& ImageAsset::getRect() const {
	return _rect;
}

ColorDepthMode ImageAsset::getColorDepth() const {
	return _colorDepth;
}

uint32 ImageAsset::getFilePosition() const {
	return _filePosition;
}

uint32 ImageAsset::getSize() const {
	return _size;
}

size_t ImageAsset::getStreamIndex() const {
	return _streamIndex;
}

ImageAsset::ImageFormat ImageAsset::getImageFormat() const {
	return _imageFormat;
}

const Common::SharedPtr<CachedImage> &ImageAsset::loadAndCacheImage(Runtime *runtime) {
	if (_imageCache)
		return _imageCache;

	ColorDepthMode renderDepth = runtime->getRealColorDepth();

	size_t streamIndex = getStreamIndex();
	int segmentIndex = runtime->getProject()->getSegmentForStreamIndex(streamIndex);
	runtime->getProject()->openSegmentStream(segmentIndex);
	Common::SeekableReadStream *stream = runtime->getProject()->getStreamForSegment(segmentIndex);

	if (!stream || !stream->seek(getFilePosition())) {
		warning("Image element failed to load");
		return _imageCache;
	}

	size_t bytesPerRow = 0;

	Rect16 imageRect = getRect();
	int width = imageRect.right - imageRect.left;
	int height = imageRect.bottom - imageRect.top;

	if (width <= 0 || height < 0) {
		warning("Image asset has invalid size");
		return _imageCache;
	}

	Graphics::PixelFormat pixelFmt;
	switch (getColorDepth()) {
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
		bytesPerRow = (width * 2 + 3) / 4 * 4;
		pixelFmt = Graphics::createPixelFormat<1555>();
		break;
	case kColorDepthMode32Bit:
		bytesPerRow = width * 4;
		pixelFmt = Graphics::createPixelFormat<8888>();
		break;
	default:
		warning("Image asset has an unrecognizable pixel format");
		return _imageCache;
	}

	Common::Array<uint8> rowBuffer;
	rowBuffer.resize(bytesPerRow);

	ImageAsset::ImageFormat imageFormat = getImageFormat();
	bool bottomUp = (imageFormat == ImageAsset::kImageFormatWindows);
	bool isBigEndian = (imageFormat == ImageAsset::kImageFormatMac);

	Common::SharedPtr<Graphics::Surface> imageSurface;
	imageSurface.reset(new Graphics::Surface());
	imageSurface->create(width, height, pixelFmt);

	for (int inRow = 0; inRow < height; inRow++) {
		int outRow = bottomUp ? (height - 1 - inRow) : inRow;

		stream->read(&rowBuffer[0], bytesPerRow);
		const uint8 *inRowBytes = &rowBuffer[0];

		void *outBase = imageSurface->getBasePtr(0, outRow);

		switch (getColorDepth()) {
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
						uint8 r = inRowBytes[x * 4 + 1];
						uint8 g = inRowBytes[x * 4 + 2];
						uint8 b = inRowBytes[x * 4 + 3];
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

	_imageCache.reset(new CachedImage());
	_imageCache->resetSurface(renderDepth, imageSurface);

	return _imageCache;
}

bool MToonAsset::load(AssetLoaderContext &context, const Data::MToonAsset &data) {
	_streamIndex = context.streamIndex;
	_assetID = data.assetID;

	_metadata.reset(new MToonMetadata());

	if (data.haveMacPart)
		_metadata->imageFormat = MToonMetadata::kImageFormatMac;
	else if (data.haveWinPart)
		_metadata->imageFormat = MToonMetadata::kImageFormatWindows;
	else
		return false;

	_frameDataPosition = data.frameDataPosition;
	_sizeOfFrameData = data.sizeOfFrameData;

	if (!_metadata->rect.load(data.rect))
		return false;

	_metadata->bitsPerPixel = data.bitsPerPixel;
	_metadata->codecID = data.codecID;

	_metadata->frames.resize(data.frames.size());
	for (size_t i = 0; i < data.frames.size(); i++) {
		if (!_metadata->frames[i].load(context, data.frames[i]))
			return false;
	}

	_metadata->frameRanges.resize(data.frameRangesPart.frameRanges.size());
	for (size_t i = 0; i < data.frameRangesPart.frameRanges.size(); i++) {
		if (!_metadata->frameRanges[i].load(context, data.frameRangesPart.frameRanges[i]))
			return false;
	}

	_metadata->codecData = data.codecData;

	return true;
}

AssetType MToonAsset::getAssetType() const {
	return kAssetTypeMToon;
}

const Common::SharedPtr<CachedMToon> &MToonAsset::loadAndCacheMToon(Runtime *runtime) {
	if (_cachedMToon)
		return _cachedMToon;

	Common::SharedPtr<CachedMToon> cachedMToon(new CachedMToon());

	size_t streamIndex = _streamIndex;
	int segmentIndex = runtime->getProject()->getSegmentForStreamIndex(streamIndex);
	runtime->getProject()->openSegmentStream(segmentIndex);
	Common::SeekableReadStream *stream = runtime->getProject()->getStreamForSegment(segmentIndex);

	if (!stream || !stream->seek(_frameDataPosition)) {
		warning("Couldn't seek stream to mToon data");
		return _cachedMToon;
	}

	if (!cachedMToon->loadFromStream(_metadata, stream, _sizeOfFrameData)) {
		warning("mToon data failed to load");
		return _cachedMToon;
	}

	_cachedMToon = cachedMToon;

	return _cachedMToon;
}

bool MToonMetadata::FrameDef::load(AssetLoaderContext &context, const Data::MToonAsset::FrameDef &data) {
	compressedSize = data.compressedSize;
	dataOffset = data.dataOffset;
	decompressedBytesPerRow = data.decompressedBytesPerRow;
	decompressedSize = data.decompressedSize;
	isKeyFrame = (data.keyframeFlag != 0);
	if (!rect.load(data.rect1))
		return false;

	return true;
}

bool MToonMetadata::FrameRangeDef::load(AssetLoaderContext &context, const Data::MToonAsset::FrameRangeDef &data) {
	name = data.name;
	startFrame = data.startFrame;
	endFrame = data.endFrame;

	return true;
}

bool TextAsset::load(AssetLoaderContext &context, const Data::TextAsset &data) {
	_assetID = data.assetID;

	switch (data.alignment) {
	case Data::kTextAlignmentCodeLeft:
		_alignment = kTextAlignmentLeft;
		break;
	case Data::kTextAlignmentCodeRight:
		_alignment = kTextAlignmentRight;
		break;
	case Data::kTextAlignmentCodeCenter:
		_alignment = kTextAlignmentCenter;
		break;
	default:
		return false;
	};

	_isBitmap = ((data.isBitmap & 1) != 0);

	if (_isBitmap) {
		if (!_bitmapRect.load(data.bitmapRect))
			return false;

		_bitmapData.reset(new Graphics::Surface());

		uint16 width = _bitmapRect.getWidth();
		uint16 height = _bitmapRect.getHeight();

		uint16 pitch = (data.pitchBigEndian[0] << 8) + data.pitchBigEndian[1];

		if (static_cast<uint32>(pitch * height) != data.bitmapSize) {
			// Pitch is normally aligned to 4 bytes, so if this fails, maybe compute it that way?
			warning("Pre-rendered text bitmap pitch didn't compute to bitmap size correctly, maybe it's wrong?");
			return false;
		}

		if (pitch * 8 < width) {
			warning("Pre-rendered text pitch is too small");
			return false;
		}

		_bitmapData->create(width, height, Graphics::PixelFormat::createFormatCLUT8());

		for (int row = 0; row < height; row++) {
			uint8 *outRow = static_cast<uint8 *>(_bitmapData->getBasePtr(0, row));
			const uint8 *inRow = &data.bitmapData[row * pitch];
			for (int col = 0; col < width; col++) {
				int bit = ((inRow[col / 8] >> (7 - (col & 7))) & 1);
				outRow[col] = bit;
			}
		}
	} else {
		_bitmapRect = Rect16::create(0, 0, 0, 0);

		_stringData = data.text;

		for (size_t i = 0; i < data.macFormattingSpans.size(); i++) {
			const Data::TextAsset::MacFormattingSpan &inSpan = data.macFormattingSpans[i];
			MacFormattingSpan fmtSpan;
			fmtSpan.formatting = MacFontFormatting(inSpan.fontID, inSpan.fontFlags, inSpan.size);
			fmtSpan.spanStart = inSpan.spanStart;

			_macFormattingSpans.push_back(fmtSpan);
		}
	}

	return true;
}

AssetType TextAsset::getAssetType() const {
	return kAssetTypeText;
}

bool TextAsset::isBitmap() const {
	return _isBitmap;
}

const Common::SharedPtr<Graphics::Surface>& TextAsset::getBitmapSurface() const {
	return _bitmapData;
}

const Common::String& TextAsset::getString() const {
	return _stringData;
}

const Common::Array<MacFormattingSpan> &TextAsset::getMacFormattingSpans() const {
	return _macFormattingSpans;
}

} // End of namespace MTropolis
