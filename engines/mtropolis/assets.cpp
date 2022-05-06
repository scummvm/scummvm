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

bool AudioAsset::load(AssetLoaderContext &context, const Data::AudioAsset &data) {
	_assetID = data.assetID;
	_sampleRate = data.sampleRate1;
	_bitsPerSample = data.bitsPerSample;

	switch (data.encoding1) {
	case 0:
		_encoding = kEncodingUncompressed;
		break;
	case 3:
		_encoding = kEncodingMace3;
		break;
	case 4:
		_encoding = kEncodingMace6;
		break;
	default:
		return false;
	}

	_channels = data.channels;
	// Hours Minutes Seconds Hundredths -> msec
	// Maximum is 0x37a4f52e so this fits in 30 bits
	_durationMSec = ((((data.codedDuration[0] * 60u) + data.codedDuration[1]) * 60u + data.codedDuration[2]) * 100u + data.codedDuration[3]) * 10u;
	_filePosition = data.filePosition;
	_size = data.size;
	_cuePoints.resize(data.cuePoints.size());

	for (size_t i = 0; i < _cuePoints.size(); i++) {
		_cuePoints[i].cuePointID = data.cuePoints[i].cuePointID;
		_cuePoints[i].position = data.cuePoints[i].position;
	}

	return true;
}

AssetType AudioAsset::getAssetType() const {
	return kAssetTypeAudio;
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

bool MToonAsset::load(AssetLoaderContext &context, const Data::MToonAsset &data) {
	if (data.haveMacPart)
		_imageFormat = kImageFormatMac;
	else if (data.haveWinPart)
		_imageFormat = kImageFormatWindows;
	else
		return false;

	_frameDataPosition = data.frameDataPosition;
	_sizeOfFrameData = data.sizeOfFrameData;

	if (!_rect.load(data.rect))
		return false;

	_bitsPerPixel = data.bitsPerPixel;
	_codecID = data.codecID;

	_frames.resize(data.frames.size());
	for (size_t i = 0; i < _frames.size(); i++) {
		if (!_frames[i].load(context, data.frames[i]))
			return false;
	}

	_frameRanges.resize(data.frameRangesPart.frameRanges.size());
	for (size_t i = 0; i < _frameRanges.size(); i++) {
		if (!_frameRanges[i].load(context, data.frameRangesPart.frameRanges[i]))
			return false;
	}

	_codecData = data.codecData;
}

AssetType MToonAsset::getAssetType() const {
	return kAssetTypeMToon;
}

bool MToonAsset::FrameDef::load(AssetLoaderContext &context, const Data::MToonAsset::FrameDef &data) {
	compressedSize = data.compressedSize;
	dataOffset = data.dataOffset;
	decompressedBytesPerRow = data.decompressedBytesPerRow;
	decompressedSize = data.decompressedSize;
	isKeyFrame = (data.keyframeFlag != 0);
	if (!rect.load(data.rect1))
		return false;

	return true;
}

bool MToonAsset::FrameRangeDef::load(AssetLoaderContext &context, const Data::MToonAsset::FrameRangeDef &data) {
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
		uint16 pitch = (data.pitchBigEndian[0] << 8) + data.pitchBigEndian[1];
		uint16 width = _bitmapRect.getWidth();
		uint16 height = _bitmapRect.getHeight();
		if (static_cast<uint32>(pitch * width) != data.bitmapSize) {
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
