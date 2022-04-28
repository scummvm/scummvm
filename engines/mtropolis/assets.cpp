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


} // End of namespace MTropolis
