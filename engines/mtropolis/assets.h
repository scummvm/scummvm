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

#ifndef MTROPOLIS_ASSETS_H
#define MTROPOLIS_ASSETS_H

#include "mtropolis/data.h"
#include "mtropolis/runtime.h"
#include "mtropolis/render.h"

namespace MTropolis {

struct AssetLoaderContext;

class ColorTableAsset : public Asset {
public:
	bool load(AssetLoaderContext &context, const Data::ColorTableAsset &data);
	AssetType getAssetType() const override;

private:
	ColorRGB8 _colors[256];
};

class AudioAsset : public Asset {
public:
	struct CuePoint {
		uint32 position;
		uint32 cuePointID;
	};

	enum Encoding {
		kEncodingUncompressed,
		kEncodingMace3,
		kEncodingMace6,
	};

	bool load(AssetLoaderContext &context, const Data::AudioAsset &data);
	AssetType getAssetType() const override;

private:
	uint16 _sampleRate;
	uint8 _bitsPerSample;
	Encoding _encoding;
	uint8 _channels;
	uint32 _durationMSec;
	uint32 _filePosition;
	uint32 _size;

	Common::Array<CuePoint> _cuePoints;
};

class MovieAsset : public Asset {
public:
	bool load(AssetLoaderContext &context, const Data::MovieAsset &data);
	AssetType getAssetType() const override;

	uint32 getMovieDataPos() const;
	uint32 getMoovAtomPos() const;
	uint32 getMovieDataSize() const;

	const Common::String &getExtFileName() const;
	size_t getStreamIndex() const;

private:
	uint32 _movieDataPos;
	uint32 _moovAtomPos;
	uint32 _movieDataSize;

	Common::String _extFileName;
	size_t _streamIndex;
};

class ImageAsset : public Asset {
public:
	bool load(AssetLoaderContext &context, const Data::ImageAsset &data);
	AssetType getAssetType() const override;

	enum ImageFormat {
		kImageFormatMac,
		kImageFormatWindows,
	};

	const Rect16 &getRect() const;
	ColorDepthMode getColorDepth() const;
	uint32 getFilePosition() const;
	uint32 getSize() const;
	size_t getStreamIndex() const;
	ImageFormat getImageFormat() const;

private:
	Rect16 _rect;
	ColorDepthMode _colorDepth;
	uint32 _filePosition;
	uint32 _size;
	size_t _streamIndex;
	ImageFormat _imageFormat;
};

class TextAsset : public Asset {
public:
	bool load(AssetLoaderContext &context, const Data::TextAsset &data);
	AssetType getAssetType() const override;

	bool isBitmap() const;
	const Common::SharedPtr<Graphics::Surface> &getBitmapSurface() const;
	const Common::String &getString() const;
	const Common::Array<MacFormattingSpan> &getMacFormattingSpans() const;

private:
	Rect16 _bitmapRect;
	TextAlignment _alignment;
	bool _isBitmap;

	Common::SharedPtr<Graphics::Surface> _bitmapData;
	Common::String _stringData;

	Common::Array<MacFormattingSpan> _macFormattingSpans;
};

} // End of namespace MTropolis

#endif
